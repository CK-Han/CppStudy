#include "LogicMain.h"

#include <iostream>

using NET_ERROR_CODE = NetLib::NET_ERROR_CODE;

namespace LogicLib
{
	MainLogic::MainLogic()
	{

	}

	MainLogic::~MainLogic()
	{
		Release();
	}

	void MainLogic::Release()
	{
		if (pNetwork) {
			pNetwork->Close();
		}
	}

	ERROR_CODE MainLogic::Init()
	{
		pNetwork = std::make_unique<NetLib::SelectServer>();
		auto result = pNetwork->Initialize();

		if (result != NET_ERROR_CODE::NONE)
		{
			return ERROR_CODE::MAIN_INIT_NETWORK_INIT_FAIL;
		}

		packetProcedures.insert(std::make_pair(Packet_PingPong::typeAdder.GetType(), &MainLogic::Process_PingPong));
		packetProcedures.insert(std::make_pair(Packet_SessionClose::typeAdder.GetType(), &MainLogic::Process_SessionClose));
		packetProcedures.insert(std::make_pair(Packet_RegisterId::typeAdder.GetType(), &MainLogic::Process_RegisterId));
		packetProcedures.insert(std::make_pair(Packet_ChatAll::typeAdder.GetType(), &MainLogic::Process_ChatAll));
		packetProcedures.insert(std::make_pair(Packet_GetRoomList::typeAdder.GetType(), &MainLogic::Process_GetRoomList));
		packetProcedures.insert(std::make_pair(Packet_EnterRoom::typeAdder.GetType(), &MainLogic::Process_EnterRoom));
		packetProcedures.insert(std::make_pair(Packet_LeaveRoom::typeAdder.GetType(), &MainLogic::Process_LeaveRoom));
		packetProcedures.insert(std::make_pair(Packet_CreateRoom::typeAdder.GetType(), &MainLogic::Process_CreateRoom));
		
		isRun = true;
		return ERROR_CODE::NONE;
	}

	void MainLogic::Run()
	{
		while (isRun)
		{
			pNetwork->Run();

			while (true)
			{
				auto packetInfo = pNetwork->GetPacketInfo();

				if (packetInfo.PacketType == 0)
				{
					break;
				}
				else
				{
					ProcessPacket(packetInfo);
				}
			}
		}
	}

	void MainLogic::Stop()
	{
		isRun = false;
	}

	void MainLogic::ProcessPacket(const NetLib::RecvPacketInfo& packet)
	{
		if (packet.PacketType == 0) {
			return;
		}

		auto procedure = packetProcedures.find(packet.PacketType);
		if (procedure == packetProcedures.end()) {
			return;
		}

		(this->*packetProcedures[packet.PacketType])(packet);
	}

	void MainLogic::Process_PingPong(const NetLib::RecvPacketInfo& packet)
	{
		StreamReader inStream(packet.pData, packet.PacketSize);
		Packet_PingPong in;
		in.Deserialize(inStream);

		std::cout << "clients: " << in.Message << std::endl;

		Packet_PingPong out;
		out.Message = in.Message;
		char buf[Packet_Base::MAX_BUF_SIZE] = { 0, };
		StreamWriter outStream(buf, sizeof(buf));
		out.Serialize(outStream);
		
		pNetwork->SendData(packet.SessionSerial, outStream);
	}

	void MainLogic::Process_SessionClose(const NetLib::RecvPacketInfo& packet)
	{
		auto session = packet.SessionSerial;
		auto userIter = users.find(session);
		if (userIter == users.end()) {
			return;
		}
		
		if (userIter->second.RoomNumber != INVALID_SERIAL) {
			auto roomIter = rooms.find(userIter->second.RoomNumber);
			if (roomIter != rooms.end()) {
				LeaveRoom(roomIter->second, userIter->second);
			}
		}

		auto name = userIter->second.Name;
		users.erase(userIter);
		
		auto userNameIter = usedNames.find(name);
		if (userNameIter == usedNames.end()) {
			return;
		}
		usedNames.erase(userNameIter);
	}

	void MainLogic::Process_RegisterId(const NetLib::RecvPacketInfo& packet)
	{
		StreamReader inStream(packet.pData, packet.PacketSize);
		Packet_RegisterId in;
		in.Deserialize(inStream);
		
		auto insertPair = usedNames.insert(std::make_pair(in.UserName, packet.SessionSerial));
		bool created = insertPair.second;

		if (created) {
			User user;
			user.Serial = packet.SessionSerial;
			user.Name = in.UserName;
			user.State = USER_STATE::LOBBY;

			auto userInsertResult = users.insert(std::make_pair(packet.SessionSerial, user));
			if (userInsertResult.second == false) {
				// error
			}
		}

		Packet_RegisterId out;
		out.Created = created;
		out.UserName = in.UserName;
		char buf[Packet_Base::MAX_BUF_SIZE] = { 0, };
		StreamWriter outStream(buf, sizeof(buf));
		out.Serialize(outStream);

		pNetwork->SendData(packet.SessionSerial, outStream);

		// 대기실에 들어왔다는 노티는 필요 없겠지?
	}

	void MainLogic::Process_ChatAll(const NetLib::RecvPacketInfo& packet)
	{
		StreamReader inStream(packet.pData, packet.PacketSize);
		Packet_ChatAll in;
		in.Deserialize(inStream);

		if (in.Message.empty()) {
			return;
		}

		char buf[Packet_Base::MAX_BUF_SIZE] = { 0, };
		StreamWriter outStream(buf, sizeof(buf));
		in.Serialize(outStream);

		// 대기실에 있는 유저들에게만 보내야함
		auto lobbyUsers = GetLobbyUsers();
		for(const auto& user : lobbyUsers) {
			pNetwork->SendData(user.Serial, outStream);
		}
	}

	void MainLogic::Process_GetRoomList(const NetLib::RecvPacketInfo& packet)
	{
		Packet_GetRoomList outPacket;
		outPacket.WaitedRooms.reserve(rooms.size());
		for (const auto& room : rooms) {
			switch (room.second.RoomState) {
			case ROOM_STATE::CREATED_WAIT:
				outPacket.WaitedRooms.push_back(room.second.RoomNumber);
				break;

			case ROOM_STATE::STARTED:
				++outPacket.PlayedRoomCount;
				break;

			default:
				break;
			}
		}

		char buf[Packet_Base::MAX_BUF_SIZE] = { 0, };
		StreamWriter outStream(buf, sizeof(buf));
		outPacket.Serialize(outStream);

		pNetwork->SendData(packet.SessionSerial, outStream);
	}

	void MainLogic::Process_EnterRoom(const NetLib::RecvPacketInfo& packet)
	{
		StreamReader inStream(packet.pData, packet.PacketSize);
		Packet_EnterRoom in;
		in.Deserialize(inStream);

		bool entered = true;
		auto enterUserIter = users.find(packet.SessionSerial);
		if (enterUserIter == users.end()) {
			//오류 처리
			return;
		}

		auto roomIter = rooms.find(in.RoomNumber);
		if (roomIter == rooms.end() ||
			roomIter->second.RoomState != ROOM_STATE::CREATED_WAIT) {
			entered = false;
		}

		auto roomMasterSerial = INVALID_SERIAL;
		if (entered) {
			roomMasterSerial = roomIter->second.User1->Serial;
			EnterRoom(roomIter->second, enterUserIter->second);
		}

		Packet_EnterRoom outEnterRoom;
		outEnterRoom.User = enterUserIter->second.Name;
		outEnterRoom.Entered = entered;
		outEnterRoom.RoomNumber = in.RoomNumber;
		char buf[Packet_Base::MAX_BUF_SIZE] = { 0, };
		StreamWriter outEnterRoomStream(buf, sizeof(buf));
		outEnterRoom.Serialize(outEnterRoomStream);

		pNetwork->SendData(packet.SessionSerial, outEnterRoomStream);

		//기존에 방장인 유저에게 노티 + enter 유저에게 방장 정보 전달
		if (roomMasterSerial != INVALID_SERIAL) {
			pNetwork->SendData(roomMasterSerial, outEnterRoomStream);

			Packet_RoomInfo outRoomInfo;
			outRoomInfo.RoomNumber = in.RoomNumber;
			outRoomInfo.Users.push_back(roomIter->second.User1->Name);
			outRoomInfo.Users.push_back(roomIter->second.User2->Name);
			char buf[Packet_Base::MAX_BUF_SIZE] = { 0, };
			StreamWriter outRoomInfoStream(buf, sizeof(buf));
			outRoomInfo.Serialize(outRoomInfoStream);

			pNetwork->SendData(packet.SessionSerial, outRoomInfoStream);
		}
	}

	void MainLogic::Process_LeaveRoom(const NetLib::RecvPacketInfo& packet)
	{
		StreamReader inStream(packet.pData, packet.PacketSize);
		Packet_LeaveRoom in;
		in.Deserialize(inStream);

		bool leaved = true;
		auto leaveUserIter = users.find(packet.SessionSerial);
		if (leaveUserIter == users.end()) {
			//오류 처리
			return;
		}

		auto roomIter = rooms.find(in.LeavedRoomNumber);
		if (roomIter == rooms.end()) {
			leaved = false;
		}

		auto remainedUserSerial = INVALID_SERIAL;
		if (leaved) {
			if (roomIter->second.RoomState == ROOM_STATE::FULL) {
				remainedUserSerial = (roomIter->second.User1->Serial == packet.SessionSerial) ?
					roomIter->second.User2->Serial : packet.SessionSerial;
			}

			LeaveRoom(roomIter->second, leaveUserIter->second);
		}

		Packet_LeaveRoom out;
		out.User = leaveUserIter->second.Name;
		out.Leaved = leaved;
		out.LeavedRoomNumber = in.LeavedRoomNumber;
		char buf[Packet_Base::MAX_BUF_SIZE] = { 0, };
		StreamWriter outStream(buf, sizeof(buf));
		out.Serialize(outStream);

		pNetwork->SendData(packet.SessionSerial, outStream);

		//방에 남아있는 유저에게 노티
		if (remainedUserSerial != INVALID_SERIAL) {
			pNetwork->SendData(remainedUserSerial, outStream);
		}
	}

	void MainLogic::Process_CreateRoom(const NetLib::RecvPacketInfo& packet)
	{
		StreamReader inStream(packet.pData, packet.PacketSize);
		Packet_CreateRoom in;
		in.Deserialize(inStream);

		auto userIter = users.find(packet.SessionSerial);
		if (userIter == users.end()) {
			//오류 처리
			return;
		}

		bool created = CreateRoom(in.RoomNumber);
		if (created) {
			auto& room = rooms[in.RoomNumber];
			EnterRoom(room, userIter->second);
		}

		Packet_CreateRoom out;
		out.Created = created;
		out.RoomNumber = in.RoomNumber;
		char buf[Packet_Base::MAX_BUF_SIZE] = { 0, };
		StreamWriter outStream(buf, sizeof(buf));
		out.Serialize(outStream);

		pNetwork->SendData(packet.SessionSerial, outStream);
	}


	std::vector<MainLogic::User> MainLogic::GetLobbyUsers() const
	{
		std::vector<User> lobbyUsers;
		lobbyUsers.reserve(users.size());

		for (const auto& user : users) {
			if (user.second.State != USER_STATE::LOBBY) {
				continue;
			}

			lobbyUsers.push_back(user.second);
		}

		return lobbyUsers;
	}

	void MainLogic::EnterRoom(Room& room, User& user)
	{
		if (room.User1 != nullptr) {
			room.User2 = &user;
			room.RoomState = ROOM_STATE::FULL;
		}
		else {
			room.User1 = &user;
			room.RoomState = ROOM_STATE::CREATED_WAIT;
		}

		user.State = USER_STATE::ROOM;
		user.RoomNumber = room.RoomNumber;
	}

	void MainLogic::LeaveRoom(Room& room, User& user)
	{
		if (room.User2 == nullptr) {
			DestroyRoom(room);
		}
		else {
			// 남아있는 user를 1p로 설정
			room.User1 = room.User2;
			room.Ready1 = room.Ready2;
			room.User2 = nullptr;
			room.Ready2 = false;
			room.RoomState = ROOM_STATE::CREATED_WAIT;
		}

		user.State = USER_STATE::LOBBY;
		user.RoomNumber = INVALID_SERIAL;
	}

	bool MainLogic::CreateRoom(SerialType serial)
	{
		auto roomIter = rooms.find(serial);
		if (roomIter != rooms.end()) {
			return false;
		}

		rooms[serial].RoomNumber = serial;
		rooms[serial].RoomState = ROOM_STATE::CREATED_WAIT;
		return true;
	}

	bool MainLogic::DestroyRoom(Room& room)
	{
		rooms.erase(room.RoomNumber);

		return true;
	}
}