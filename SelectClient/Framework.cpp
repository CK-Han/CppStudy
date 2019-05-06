#include "Framework.h"
#include <iostream>

Framework::Framework()
{
}


Framework::~Framework()
{
}

bool Framework::Initialize()
{
	bool initResult = socket.Initialize(this, IP);
	if (initResult == FALSE) {
		std::cout << "소켓 연결 실패 : " << GetLastError() << std::endl;
		return false;
	}

	packetProcedures.insert(std::make_pair(Packet_RegisterId::typeAdder.GetType(), &Framework::Process_RegisterId));
	packetProcedures.insert(std::make_pair(Packet_ChatAll::typeAdder.GetType(), &Framework::Process_ChatAll));

	socketThread.reset(new std::thread([&]() {
		while (socket.IsRun()) {
			socket.Run();
		}
	}));

	isEnded = false;
	return true;
}

void Framework::Run()
{
	while (isEnded == false) {
		switch (userState) {
		case USER_STATE::NOT_LOGIN:
			Request_RegisterId();
			break;

		case USER_STATE::LOBBY:
			Request_LobbyCommand();
			break;

		case USER_STATE::ROOM:
			break;

		default:
			break;
		}
	}
}

void Framework::ProcessPacket(const char* packet, int size)
{
	if (size < Packet_Base::HEADER_SIZE) {
		return;
	}

	auto type = GetPacketType(packet);

	auto procedure = packetProcedures.find(type);
	if (procedure == packetProcedures.end()) {
		return;
	}

	(this->*packetProcedures[type])(packet, size);
}

void Framework::Request_RegisterId()
{
	if (userState != USER_STATE::NOT_LOGIN) {
		return;
	}

	std::string input;
	std::cout << "게임에서 사용하실 닉네임을 입력해주세요: ";
	std::cin >> input;

	StreamWriter out(sendBuf, BUF_SIZE);
	Packet_RegisterId packet;
	packet.UserName = input;
	packet.Serialize(out);
	socket.AddSendPacket((const char*)out.GetBuffer(), out.GetStreamSize());
	userState = USER_STATE::LOGIN_WAIT;
}

void Framework::Request_LobbyCommand()
{
	if (userState != USER_STATE::LOBBY) {
		return;
	}

	std::string input;
	std::cin >> input;

	StreamWriter out(sendBuf, BUF_SIZE);
	Packet_ChatAll packet;
	packet.UserName = userName;
	packet.Message = input;
	packet.Serialize(out);
	socket.AddSendPacket((const char*)out.GetBuffer(), out.GetStreamSize());
}

void Framework::Process_RegisterId(const char* packet, int size)
{
	StreamReader in(packet, size);
	Packet_RegisterId inPacket;
	inPacket.Deserialize(in);
	if (inPacket.Created == false) {
		std::cout << "사용할 수 없는 id 입니다." << std::endl;
		userState = USER_STATE::NOT_LOGIN;
		return;
	}

	userName = inPacket.UserName;
	std::cout << userName << " 으로 로그인했습니다." << std::endl;
	userState = USER_STATE::LOBBY;
}

void Framework::Process_ChatAll(const char* packet, int size)
{
	StreamReader in(packet, size);
	Packet_ChatAll inPacket;
	inPacket.Deserialize(in);

	std::cout << inPacket.UserName << " : " << inPacket.Message << std::endl;
}