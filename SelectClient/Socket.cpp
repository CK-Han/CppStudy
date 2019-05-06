#include "Socket.h"
#include "Framework.h"

Socket::Socket()
{
	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
}

Socket::~Socket()
{
	WSACleanup();
}

bool Socket::Initialize(Framework* framework, const char* serverIP)
{
	if (framework == nullptr) {
		return false;
	}

	this->framework = framework;

	if (serverIP == nullptr) {
		return false;
	}

	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		return false;
	}

	auto n = 1;
	if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&n, sizeof(n)) < 0)
	{
		return false;
	}

	unsigned long mode = 1;
	if (ioctlsocket(clientSocket, FIONBIO, &mode) == SOCKET_ERROR)
	{
		return false;
	}

	SOCKADDR_IN serverAddr;
	::ZeroMemory(&serverAddr, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, serverIP, &(serverAddr.sin_addr.S_un.S_addr));

	int retVal = connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
	if (retVal <= SOCKET_ERROR) {
		if (GetLastError() != WSAEWOULDBLOCK) {
			return false;
		}
	}

	FD_ZERO(&readSet);
	FD_SET(clientSocket, &readSet);

	disconnect = false;
	return true;
}

void Socket::Run()
{
	timeval timeout{ 0, 1000 }; //tv_sec, tv_usec
	auto selectResult = select(0, &readSet, &readSet, 0, &timeout);
	if (selectResult == SOCKET_ERROR) {
		// auto errCode = WSAGetLastError();
		return;
	}

	if (selectResult == 0) {
		//timeout
		return;
	}

	ReadPacket();
	SendPacket();
}

void Socket::ReadPacket()
{
	auto recvPos = savedPacketSize;
	auto remainedSize = sizeof(recvBuf) - recvPos; //RecvBuf 공간이 부족해 read하지 못한 데이터는 다음 루프에서...
	auto recvSize = recv(clientSocket, &(recvBuf[recvPos]), remainedSize, 0);

	if (recvSize <= 0) {
		if (GetLastError() != WSAEWOULDBLOCK) {
			disconnect = true;
		}

		return;
	}

	savedPacketSize += recvSize;

	auto readPos = 0;
	while (savedPacketSize >= Packet_Base::HEADER_SIZE) {
		auto bufPtr = &(recvBuf[readPos]);
		auto packetSize = GetPacketSize(bufPtr);

		if (packetSize <= savedPacketSize) {
			// 패킷 조립이 가능한 경우
			ProcessPacket(bufPtr, packetSize);

			readPos += packetSize;
			savedPacketSize -= packetSize;
		}
		else if (packetSize >= Packet_Base::MAX_BUF_SIZE) {
			disconnect = true;
			return;
		}
		else {
			break;
		}
	}
}

void Socket::AddSendPacket(const char* packet, int size)
{
	std::unique_lock<std::mutex> ul_sendQueue(sendQueueLock);
	SendData sendData;
	sendData.buf = packet;
	sendData.size = size;
	sendQueue.push(sendData);
}

void Socket::SetDisconnect(bool disconnect)
{
	this->disconnect = disconnect;
}

void Socket::SendPacket()
{
	std::unique_lock<std::mutex> ul_sendQueue(sendQueueLock);
	while (sendQueue.empty() == FALSE) {
		auto packet = sendQueue.front();
		sendQueue.pop();
		auto sendSize = send(clientSocket, packet.buf, packet.size, 0);
		
		if (sendSize <= 0) {
			if (GetLastError() != WSAEWOULDBLOCK) {
				disconnect = true;
			}

			break;
		}
	}
}

/**
	@brief		패킷 조립 완료로 Framework의 패킷 프로시저를 호출하도록 한다.
	@details	단순하게 인자검사 없이 전달한다.
*/
void Socket::ProcessPacket(const void* packet, int size)
{
	framework->ProcessPacket((const char*)packet, size);
}