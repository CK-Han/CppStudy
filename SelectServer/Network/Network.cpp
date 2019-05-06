#include "Network.h"

namespace NetLib
{
	NET_ERROR_CODE SelectServer::Initialize()
	{
		WSADATA	wsadata;
		WSAStartup(MAKEWORD(2, 2), &wsadata);

		auto initSocketRet = InitServerSocket();
		if (initSocketRet != NET_ERROR_CODE::NONE) {
			return initSocketRet;
		}

		auto bindListenRet = BindListen(PORT, 10);
		if (bindListenRet != NET_ERROR_CODE::NONE) {
			return bindListenRet;
		}

		FD_ZERO(&readFds);
		FD_SET(serverSocket, &readFds);

		clientSessions.clear();
		clientSessions.resize(MAX_CLIENT_COUNT);

		for (auto i = 0; i < MAX_CLIENT_COUNT; ++i) {
			validSessionSerials.push(i);
		}

		return NET_ERROR_CODE::NONE;
	}

	void SelectServer::Run()
	{
		auto read_set = readFds;
		auto write_set = readFds;

		timeval timeout{ 0, 1000 }; //tv_sec, tv_usec
		auto selectResult = select(0, &read_set, &write_set, 0, &timeout);
		if (selectResult == SOCKET_ERROR) {
			// auto errCode = WSAGetLastError();
			return;
		}

		if (selectResult == 0) {
			//timeout
			return;
		}

		// Accept
		if (FD_ISSET(serverSocket, &read_set))
		{
			ProcessNewSession();
		}

		// 세션 통신 확인
		RunCheckSelectClients(read_set, write_set);
	}

	void SelectServer::Close()
	{
		WSACleanup();
	}

	NET_ERROR_CODE SelectServer::SendData(SelectServer::SERIAL_TYPE sessionSerial, StreamWriter& out)
	{
		auto& session = clientSessions[sessionSerial];

		auto savedSize = session.SavedSendSize;
		auto totalSize = out.GetStreamSize();
		auto bufPtr = out.GetBuffer();

		if ((savedSize + totalSize) > Packet_Base::MAX_BUF_SIZE) {
			return NET_ERROR_CODE::CLIENT_SEND_BUFFER_FULL;
		}

		std::memcpy(&(session.SendBuf[savedSize]), bufPtr, totalSize);
		session.SavedSendSize += totalSize;

		return NET_ERROR_CODE::NONE;
	}

	RecvPacketInfo SelectServer::GetPacketInfo()
	{
		RecvPacketInfo packetInfo;

		if (packetQueue.empty() == false)
		{
			packetInfo = packetQueue.front();
			packetQueue.pop();
		}

		return packetInfo;
	}

	NET_ERROR_CODE SelectServer::ProcessNewSession()
	{
		auto tryCount = 0; // 너무 많이 accept를 시도하지 않도록 한다.

		do {
			++tryCount;

			SOCKADDR_IN clientAddr;
			auto addrSize = static_cast<int>(sizeof(clientAddr));
			auto clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &addrSize);
			
			if (clientSocket == INVALID_SOCKET) {
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					return NET_ERROR_CODE::ACCEPT_API_WSAEWOULDBLOCK;
				}

				return NET_ERROR_CODE::ACCEPT_API_ERROR;
			}

			auto newSessionSerial = AllocSessionSerial();
			if (IsValidSerial(newSessionSerial) == FALSE)
			{
				// 더 이상 수용할 수 없으므로 바로 짜른다.
				CloseSession(SOCKET_CLOSE_CASE::SESSION_POOL_EMPTY, clientSocket, newSessionSerial);
				return NET_ERROR_CODE::ACCEPT_MAX_SESSION_COUNT;
			}

			auto& session = clientSessions[newSessionSerial];
			session.Clear();
			session.Serial = newSessionSerial;
			session.ClientSocket = clientSocket;
			session.IsConnected = true;
			inet_ntop(AF_INET, &(clientAddr.sin_addr), session.IP, SelectClient::MAX_IP_LENGTH - 1);

			//SetSockOption(client_sockfd); // 소켓버퍼 크기 조절하기
			FD_SET(clientSocket, &readFds);

			AddPacketQueue(newSessionSerial, 0, 0, nullptr); // 로직에서 세션 연결 이벤트가 필요하면, type 수정

		} while (tryCount < FD_SETSIZE);

		return NET_ERROR_CODE::NONE;
	}

	void SelectServer::RunCheckSelectClients(FD_SET& readSet, FD_SET& writeSet)
	{
		auto sessionCount = clientSessions.size();
		for (decltype(sessionCount) i = 0; i < sessionCount; ++i)
		{
			auto& session = clientSessions[i];

			if (session.IsConnected == false) {
				continue;
			}

			// check read
			auto retReceive = RunProcessReceive(session.Serial, session.ClientSocket, readSet);
			if (retReceive == false) {
				continue;
			}

			// check write
			RunProcessWrite(session.Serial, session.ClientSocket, writeSet);
		}
	}

	bool SelectServer::RunProcessReceive(SelectServer::SERIAL_TYPE sessionSerial, const SOCKET fd, FD_SET& readSet)
	{
		if (!FD_ISSET(fd, &readSet)) {
			return true;
		}

		auto ret = RecvSocket(sessionSerial);
		if (ret != NET_ERROR_CODE::NONE) {
			CloseSession(SOCKET_CLOSE_CASE::SOCKET_RECV_ERROR, fd, sessionSerial);
			return false;
		}

		ret = RecvBufferProcess(sessionSerial);
		if (ret != NET_ERROR_CODE::NONE)
		{
			CloseSession(SOCKET_CLOSE_CASE::SOCKET_RECV_BUFFER_PROCESS_ERROR, fd, sessionSerial);
			return false;
		}

		return true;
	}

	NET_ERROR_CODE SelectServer::RecvSocket(SelectServer::SERIAL_TYPE sessionSerial)
	{
		auto& session = clientSessions[sessionSerial];
		auto fd = session.ClientSocket;

		if (session.IsConnected == false) {
			return NET_ERROR_CODE::RECV_PROCESS_NOT_CONNECTED;
		}

		auto recvPos = session.SavedRecvSize;
		auto remainedSize = sizeof(session.RecvBuf) - recvPos; //RecvBuf 공간이 부족해 read하지 못한 데이터는 다음 루프에서...
		auto recvSize = recv(fd, (char*)&(session.RecvBuf[recvPos]), static_cast<int>(remainedSize), 0);

		if (recvSize == 0) {
			return NET_ERROR_CODE::RECV_REMOTE_CLOSE;
		}

		if (recvSize < 0) { // SOCKET_ERROR
			auto error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK) {
				return NET_ERROR_CODE::RECV_API_ERROR;
			}
			else {
				return NET_ERROR_CODE::NONE;
			}
		}

		session.SavedRecvSize += recvSize;
		return NET_ERROR_CODE::NONE;
	}

	NET_ERROR_CODE SelectServer::RecvBufferProcess(SelectServer::SERIAL_TYPE sessionSerial)
	{
		auto& session = clientSessions[sessionSerial];

		auto readPos = 0;
		while (session.SavedRecvSize >= Packet_Base::HEADER_SIZE) {
			auto bufPtr = &(session.RecvBuf[readPos]);
			auto packetSize = GetPacketSize(bufPtr);

			if (packetSize <= session.SavedRecvSize) {
				// 패킷 조립이 가능한 경우
				AddPacketQueue(sessionSerial, GetPacketType(bufPtr), packetSize, bufPtr);

				readPos += packetSize;
				session.SavedRecvSize -= packetSize;
			}
			else if(packetSize >= Packet_Base::MAX_BUF_SIZE) {
				return NET_ERROR_CODE::RECV_BUFFER_OVERFLOW;
			} 
			else {
				break;
			}
		}

		return NET_ERROR_CODE::NONE;
	}

	void SelectServer::AddPacketQueue(SERIAL_TYPE sessionSerial, PACKET_TYPE type, PACKET_SIZE size, char* pData)
	{
		RecvPacketInfo info;
		info.SessionSerial = sessionSerial;
		info.PacketType = type;
		info.PacketSize = size;
		info.pData = pData;

		packetQueue.push(info);
	}

	void SelectServer::RunProcessWrite(SelectServer::SERIAL_TYPE sessionSerial, const SOCKET fd, FD_SET& writeSet)
	{
		if (!FD_ISSET(fd, &writeSet)) {
			return;
		}

		auto retsend = FlushSendBuff(sessionSerial);
		if (retsend != NET_ERROR_CODE::NONE) {
			CloseSession(SOCKET_CLOSE_CASE::SOCKET_SEND_ERROR, fd, sessionSerial);
		}
	}

	NET_ERROR_CODE SelectServer::FlushSendBuff(SERIAL_TYPE sessionSerial)
	{
		auto& session = clientSessions[sessionSerial];
		if (session.IsConnected == false) {
			return NET_ERROR_CODE::CLIENT_FLUSH_SEND_BUFF_REMOTE_CLOSE;
		}

		while (session.SavedSendSize >= Packet_Base::HEADER_SIZE) {
			auto packetSize = GetPacketSize(session.SendBuf);

			if (session.SavedSendSize >= packetSize) {
				// 패킷 조립이 가능한 경우
				auto sendResult = SendSocket(session.ClientSocket, session.SendBuf, packetSize);
				if (sendResult != NET_ERROR_CODE::NONE) {
					return sendResult;
				}

				session.SavedSendSize -= packetSize;
				std::memmove(session.SendBuf, &(session.SendBuf[packetSize]), session.SavedSendSize);
			}
			else if (packetSize >= Packet_Base::MAX_BUF_SIZE) {
				return NET_ERROR_CODE::CLIENT_SEND_BUFFER_FULL;
			}
			else {
				break;
			}
		}

		return NET_ERROR_CODE::NONE;
	}

	NET_ERROR_CODE SelectServer::SendSocket(const SOCKET fd, const char* buf, SIZE_TYPE size)
	{
		auto sended = send(fd, buf, static_cast<int>(size), 0);

		if (sended <= 0) {
			return NET_ERROR_CODE::SEND_SIZE_ZERO;
		}

		return NET_ERROR_CODE::NONE;
	}

	void SelectServer::CloseSession(const SOCKET_CLOSE_CASE closeCase, const SOCKET sockFD, SelectServer::SERIAL_TYPE sessionSerial)
	{
		if (closeCase == SOCKET_CLOSE_CASE::SESSION_POOL_EMPTY) {
			closesocket(sockFD);
			FD_CLR(sockFD, &readFds);
			return;
		}

		if (IsValidSerial(sessionSerial) == FALSE) {
			return;
		}

		auto& session = clientSessions[sessionSerial];
		if (session.IsConnected == false) {
			return;
		}

		closesocket(sockFD);
		FD_CLR(sockFD, &readFds);

		session.Clear();
		--connectedSessionCount;
		ReleaseSessionSerial(sessionSerial);

		// TODO 세션 close process 전에 다시 그 세션이 사용된다면(이미 reelaseSessionSerial했으니...) 문제가 발생할것이다.
		AddPacketQueue(sessionSerial, Packet_SessionClose::typeAdder.GetType(), 0, nullptr);
	}

	bool SelectServer::IsValidSerial(SERIAL_TYPE serial)
	{
		return (0 <= serial && serial < MAX_CLIENT_COUNT);
	}

	SelectServer::SERIAL_TYPE SelectServer::AllocSessionSerial()
	{
		if (validSessionSerials.empty()) {
			return INVALID_SERIAL;
		}

		auto serial = validSessionSerials.front();
		validSessionSerials.pop();

		clientSessions[serial].Clear();
		return serial;
	}

	void SelectServer::ReleaseSessionSerial(SelectServer::SERIAL_TYPE serial)
	{
		if (IsValidSerial(serial) == false) {
			return;
		}

		validSessionSerials.push(serial);
		clientSessions[serial].Clear();
	}

	NET_ERROR_CODE SelectServer::InitServerSocket()
	{
		serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (serverSocket < 0)
		{
			return NET_ERROR_CODE::SERVER_SOCKET_CREATE_FAIL;
		}

		auto n = 1;
		if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&n, sizeof(n)) < 0)
		{
			return NET_ERROR_CODE::SERVER_SOCKET_SO_REUSEADDR_FAIL;
		}

		return NET_ERROR_CODE::NONE;
	}

	NET_ERROR_CODE SelectServer::BindListen(short port, int backlogCount)
	{
		SOCKADDR_IN server_addr;
		ZeroMemory(&server_addr, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons(port);

		if (bind(serverSocket, (SOCKADDR*)&server_addr, sizeof(server_addr)) < 0)
		{
			return NET_ERROR_CODE::SERVER_SOCKET_BIND_FAIL;
		}

		unsigned long mode = 1;
		if (ioctlsocket(serverSocket, FIONBIO, &mode) == SOCKET_ERROR)
		{
			return NET_ERROR_CODE::SERVER_SOCKET_FIONBIO_FAIL;
		}

		if (listen(serverSocket, backlogCount) == SOCKET_ERROR)
		{
			return NET_ERROR_CODE::SERVER_SOCKET_LISTEN_FAIL;
		}

		return NET_ERROR_CODE::NONE;
	}
}