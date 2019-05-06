#pragma once

#include <string>
#include <thread>
#include <map>

#include "Socket.h"


#define IP "127.0.0.1"
#define BUF_SIZE 256

class Framework
{
private:
	using Packet_Type = decltype(Packet_Base::Packet_Header::Type);
	using Packet_Procedure = void(Framework::*)(const char*, int);

public:
	Framework();
	~Framework();

	bool Initialize();
	void Run();
	bool IsEnded() { return isEnded; }

	void ProcessPacket(const char* packet, int size);

private:
	void Request_RegisterId();
	void Request_LobbyCommand();

	void Process_RegisterId(const char* packet, int size);
	void Process_ChatAll(const char* packet, int size);

private:
	Socket							socket;
	std::unique_ptr<std::thread>	socketThread;
	bool	isEnded{ true };
	char	sendBuf[BUF_SIZE];
	char	recvBuf[BUF_SIZE];

	std::string		userName;
	USER_STATE		userState{ USER_STATE::NOT_LOGIN };
	int				roomNumber{ 0 };

	std::map<Packet_Type, Packet_Procedure> packetProcedures;
};

