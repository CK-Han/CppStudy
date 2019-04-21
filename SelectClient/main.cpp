#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <WS2tcpip.h>
#include <string>

#include "../Common/Protocol/PacketBase.h"
#include "../Common/Protocol/protocol.h"
#include "../Common/Util/stream.h"
#include "../Common/Protocol/Errno.h"
#include "../Common/Protocol/Define.h"


#define IP "127.0.0.1"
#define BUF_SIZE 256

int main()
{
	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	auto clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		return false;
	}

	SOCKADDR_IN serverAddr;
	::ZeroMemory(&serverAddr, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, IP, &(serverAddr.sin_addr.S_un.S_addr));

	int retVal = WSAConnect(clientSocket, (sockaddr *)&serverAddr, sizeof(serverAddr), nullptr, nullptr, nullptr, nullptr);
	if (SOCKET_ERROR == retVal)
	{
		return false;
	}

	char sendBuf[BUF_SIZE];
	char recvBuf[BUF_SIZE];
	Packet_PingPong packet;
	
	std::string input;
	while (input != "x") {
		ZeroMemory(sendBuf, BUF_SIZE);
		std::cin >> input;
		packet.Message = input;
		
		StreamWriter out(sendBuf, BUF_SIZE);
		packet.Serialize(out);
		auto sendedSize = send(clientSocket, (const char*)out.GetBuffer(), out.GetStreamSize(), 0);

		ZeroMemory(recvBuf, BUF_SIZE);
		auto recvSize = recv(clientSocket, recvBuf, BUF_SIZE, 0);
		StreamReader in(recvBuf, BUF_SIZE);
		packet.Deserialize(in);

		std::cout << "server echo : " << packet.Message << std::endl;
	}

	WSACleanup();
}