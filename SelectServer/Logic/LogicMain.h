#pragma once

#include <memory>
#include <map>

#include "../Network/Network.h"
#include "../../Common/Protocol/Errno.h"

namespace NetLib
{
	class INetwork;
}

namespace LogicLib
{
	class MainLogic
	{
	private:
		using Packet_Type = decltype(NetLib::RecvPacketInfo::PacketType);
		using Packet_Procedure = void(MainLogic::*)(const NetLib::RecvPacketInfo&);
		using StringType = Packet_Base::StringType;
		using SerialType = decltype(NetLib::RecvPacketInfo::SessionSerial);
		static const auto INVALID_SERIAL = NetLib::INetwork::INVALID_SERIAL;

		struct User
		{
			SerialType Serial{ INVALID_SERIAL };
			StringType Name;
			USER_STATE State{ USER_STATE::NOT_LOGIN };
			SerialType RoomNumber{ INVALID_SERIAL };
		};

		struct Room
		{
			SerialType	RoomNumber{ INVALID_SERIAL };
			User*		User1{ nullptr };
			bool		Ready1{ false };
			User*		User2{ nullptr };
			bool		Ready2{ false };
			ROOM_STATE	RoomState{ ROOM_STATE::NONE };
		};

	public:
		MainLogic();
		~MainLogic();

		ERROR_CODE Init();

		void Run();

		void Stop();

	private:
		void Release();

		std::vector<User> GetLobbyUsers() const;
		void EnterRoom(Room&, User&);
		void LeaveRoom(Room&, User&);
		bool CreateRoom(SerialType);
		bool DestroyRoom(Room&);

	private:
		void ProcessPacket(const NetLib::RecvPacketInfo& packet);
		void Process_PingPong(const NetLib::RecvPacketInfo& packet);
		void Process_SessionClose(const NetLib::RecvPacketInfo& packet);

		void Process_RegisterId(const NetLib::RecvPacketInfo& packet);
		void Process_ChatAll(const NetLib::RecvPacketInfo& packet);
		void Process_GetRoomList(const NetLib::RecvPacketInfo& packet);
		void Process_EnterRoom(const NetLib::RecvPacketInfo& packet);
		void Process_LeaveRoom(const NetLib::RecvPacketInfo& packet);
		void Process_CreateRoom(const NetLib::RecvPacketInfo& packet);
		void Process_ReadyRoom(const NetLib::RecvPacketInfo& packet);

		void Process_StartGame(const NetLib::RecvPacketInfo& packet);
		void Process_SubmitRoundData(const NetLib::RecvPacketInfo& packet);
		void Process_EndGame(const NetLib::RecvPacketInfo& packet);

	private:
		bool isRun{ false };
		
		std::unique_ptr<NetLib::INetwork> pNetwork;

		std::map<Packet_Type, Packet_Procedure> packetProcedures;

		std::map<SerialType, User> users;
		std::map<StringType, SerialType> usedNames;
		std::map<SerialType, Room> rooms;
	};
}