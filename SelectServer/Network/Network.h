#pragma once

#define FD_SETSIZE 5096 // http://blog.naver.com/znfgkro1/220175848048

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <queue>

#include "INetwork.h"

namespace NetLib
{
	struct SelectClient
	{
		using SERIAL_TYPE = INetwork::SERIAL_TYPE;
		using SIZE_TYPE = INetwork::SIZE_TYPE;
		using PACKET_TYPE = INetwork::PACKET_TYPE;
		using PACKET_SIZE = INetwork::PACKET_SIZE;
		static const int MAX_IP_LENGTH = 32;
		
		SERIAL_TYPE			Serial{ INetwork::INVALID_SERIAL };
		SOCKET				ClientSocket{ INVALID_SOCKET };
		bool				IsConnected{ false };
		char				IP[MAX_IP_LENGTH] = { 0, };

		SIZE_TYPE			SavedRecvSize{ 0 };
		char				RecvBuf[Packet_Base::MAX_BUF_SIZE]{ 0, };
		SIZE_TYPE			SavedSendSize{ 0 };
		char				SendBuf[Packet_Base::MAX_BUF_SIZE]{ 0, };

		void Clear()
		{
			Serial = (std::numeric_limits<size_t>::max)();
			ClientSocket = INVALID_SOCKET;
			IsConnected = false;
			ZeroMemory(IP, sizeof(IP));
			SavedRecvSize = 0;
			ZeroMemory(RecvBuf, sizeof(RecvBuf));
			SavedSendSize = 0;
			ZeroMemory(SendBuf, sizeof(SendBuf));
		}
	};

	class SelectServer : public INetwork
	{
	public:
		using SERIAL_TYPE = SelectClient::SERIAL_TYPE;
		using SIZE_TYPE = SelectClient::SIZE_TYPE;
		using PACKET_TYPE = SelectClient::PACKET_TYPE;
		using PACKET_SIZE = SelectClient::PACKET_SIZE;

		static const unsigned int	MAX_CLIENT_COUNT = 2000;

		enum class SOCKET_CLOSE_CASE : unsigned short
		{
			SESSION_POOL_EMPTY = 1,
			SELECT_ERROR = 2,
			SOCKET_RECV_ERROR = 3,
			SOCKET_RECV_BUFFER_PROCESS_ERROR = 4,
			SOCKET_SEND_ERROR = 5,
			FORCING_CLOSE = 6,
		};

	public:
		NET_ERROR_CODE Initialize() override;
		void Run() override;
		void Close() override;

		NET_ERROR_CODE SendData(SelectServer::SERIAL_TYPE sessionSerial, StreamWriter& out);
		RecvPacketInfo GetPacketInfo();
	
	protected:
		NET_ERROR_CODE InitServerSocket();
		NET_ERROR_CODE BindListen(short port, int backLogCount);

		bool					IsValidSerial(SERIAL_TYPE);
		SERIAL_TYPE				AllocSessionSerial();
		void					ReleaseSessionSerial(SERIAL_TYPE);

		NET_ERROR_CODE			ProcessNewSession();
		void					CloseSession(const SOCKET_CLOSE_CASE closeCase, const SOCKET sockFD, SERIAL_TYPE sessionSerial);

		void					RunCheckSelectClients(FD_SET& readSet, FD_SET& writeSet);
		bool					RunProcessReceive(SERIAL_TYPE sessionSerial, const SOCKET fd, FD_SET& readSet);
		NET_ERROR_CODE			RecvSocket(SERIAL_TYPE sessionSerial);
		NET_ERROR_CODE			RecvBufferProcess(SERIAL_TYPE sessionSerial);
		void					AddPacketQueue(SERIAL_TYPE sessionSerial, PACKET_TYPE type, PACKET_SIZE size, char* pData);

		void					RunProcessWrite(SERIAL_TYPE sessionSerial, const SOCKET fd, FD_SET& writeSet);
		NET_ERROR_CODE			FlushSendBuff(SERIAL_TYPE sessionSerial);
		NET_ERROR_CODE			SendSocket(const SOCKET fd, const char* buf, SIZE_TYPE size);

		

	protected:
		SOCKET serverSocket{ INVALID_SOCKET };
		FD_SET readFds{ 0 };

		std::vector<SelectClient> clientSessions;
		std::queue<SERIAL_TYPE> validSessionSerials;;

		std::queue<RecvPacketInfo> packetQueue;

		size_t connectedSessionCount{ 0 };
	};
}