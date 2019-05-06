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
	auto remainedSize = sizeof(recvBuf) - recvPos; //RecvBuf ������ ������ read���� ���� �����ʹ� ���� ��������...
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
			// ��Ŷ ������ ������ ���
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
	@brief		��Ŷ ���� �Ϸ�� Framework�� ��Ŷ ���ν����� ȣ���ϵ��� �Ѵ�.
	@details	�ܼ��ϰ� ���ڰ˻� ���� �����Ѵ�.
*/
void Socket::ProcessPacket(const void* packet, int size)
{
	framework->ProcessPacket((const char*)packet, size);
}