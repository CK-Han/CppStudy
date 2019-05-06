#include "protocol.h"
#include "../Util/stream.h"

const Packet_Base::TypeAdder Packet_PingPong::typeAdder("Packet_PingPong");
void Packet_PingPong::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());

	SerializeString(out, Message, static_cast<ValueType>(Message.length()));

	SerializeEnd(out);
}

void Packet_PingPong::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	DeserializeString(in, Message);

	DeserializeEnd(in);
}


const Packet_Base::TypeAdder Packet_SessionClose::typeAdder("Packet_SessionClose");
void Packet_SessionClose::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());

	out << session;

	SerializeEnd(out);
}

void Packet_SessionClose::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	in >> session;

	DeserializeEnd(in);
}


const Packet_Base::TypeAdder Packet_RegisterId::typeAdder("Packet_RegisterId");
void Packet_RegisterId::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());
	
	out << Created;
	SerializeString(out, UserName, static_cast<ValueType>(UserName.length()));

	SerializeEnd(out);
}

void Packet_RegisterId::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	in >> Created;
	DeserializeString(in, UserName);

	DeserializeEnd(in);
}


const Packet_Base::TypeAdder Packet_ChatAll::typeAdder("Packet_ChatAll");
void Packet_ChatAll::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());

	SerializeString(out, UserName, static_cast<ValueType>(UserName.length()));
	SerializeString(out, Message, static_cast<ValueType>(Message.length()));

	SerializeEnd(out);
}

void Packet_ChatAll::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	DeserializeString(in, UserName);
	DeserializeString(in, Message);

	DeserializeEnd(in);
}


const Packet_Base::TypeAdder Packet_GetRoomList::typeAdder("Packet_GetRoomList");
void Packet_GetRoomList::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());

	out << PlayedRoomCount;
	auto waitedRoomCount = WaitedRooms.size();
	out << waitedRoomCount;

	for (const auto& roomNumber : WaitedRooms) {
		out << roomNumber;
	}

	SerializeEnd(out);
}

void Packet_GetRoomList::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	in >> PlayedRoomCount;
	decltype(WaitedRooms)::size_type waitedRoomCount = 0;
	in >> waitedRoomCount;

	WaitedRooms.clear();
	WaitedRooms.resize(waitedRoomCount);

	decltype(WaitedRooms)::value_type roomNumber = 0;
	for (auto& room : WaitedRooms) {
		in >> roomNumber;
		room = roomNumber;
	}

	DeserializeEnd(in);
}


const Packet_Base::TypeAdder Packet_EnterRoom::typeAdder("Packet_EnterRoom");
void Packet_EnterRoom::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());

	SerializeString(out, User);
	out << Entered;
	out << RoomNumber;

	SerializeEnd(out);
}

void Packet_EnterRoom::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	DeserializeString(in, User);
	in >> Entered;
	in >> RoomNumber;

	DeserializeEnd(in);
}


const Packet_Base::TypeAdder Packet_LeaveRoom::typeAdder("Packet_LeaveRoom");
void Packet_LeaveRoom::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());

	SerializeString(out, User);
	out << Leaved;
	out << LeavedRoomNumber;

	SerializeEnd(out);
}

void Packet_LeaveRoom::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	DeserializeString(in, User);
	in >> Leaved;
	in >> LeavedRoomNumber;

	DeserializeEnd(in);
}


const Packet_Base::TypeAdder Packet_RoomInfo::typeAdder("Packet_RoomInfo");
void Packet_RoomInfo::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());

	out << RoomNumber;
	auto userCount = Users.size();
	out << userCount;

	for (const auto& user : Users) {
		SerializeString(out, user);
	}

	SerializeEnd(out);
}

void Packet_RoomInfo::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	in >> RoomNumber;
	decltype(Users)::size_type userCount = 0;
	in >> userCount;

	Users.clear();
	Users.resize(userCount);

	decltype(Users)::value_type userName;
	for (auto& name : Users) {
		DeserializeString(in, name);
	}

	DeserializeEnd(in);
}


const Packet_Base::TypeAdder Packet_CreateRoom::typeAdder("Packet_CreateRoom");
void Packet_CreateRoom::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());

	out << Created;
	out << RoomNumber;

	SerializeEnd(out);
}

void Packet_CreateRoom::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	in >> Created;
	in >> RoomNumber;

	DeserializeEnd(in);
}


const Packet_Base::TypeAdder Packet_ReadyRoom::typeAdder("Packet_ReadyRoom");
void Packet_ReadyRoom::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());

	out << Readyed;
	out << RoomNumber;

	SerializeEnd(out);
}

void Packet_ReadyRoom::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	in >> Readyed;
	in >> RoomNumber;

	DeserializeEnd(in);
}


const Packet_Base::TypeAdder Packet_StartGame::typeAdder("Packet_StartGame");
void Packet_StartGame::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());

	out << RoomNumber;
	auto userCount = Users.size();
	out << userCount;

	for (const auto& user : Users) {
		SerializeString(out, user);
	}

	SerializeEnd(out);
}

void Packet_StartGame::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	in >> RoomNumber;
	decltype(Users)::size_type userCount = 0;
	in >> userCount;

	Users.clear();
	Users.resize(userCount);

	decltype(Users)::value_type userName;
	for (auto& name : Users) {
		DeserializeString(in, name);
	}

	DeserializeEnd(in);
}


const Packet_Base::TypeAdder Packet_BeginRound::typeAdder("Packet_BeginRound");
void Packet_BeginRound::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());

	out << RoomNumber;
	out << Round;

	SerializeEnd(out);
}

void Packet_BeginRound::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	in >> RoomNumber;
	in >> Round;

	DeserializeEnd(in);
}

const Packet_Base::TypeAdder Packet_SubmitRoundData::typeAdder("Packet_SubmitRoundData");
void Packet_SubmitRoundData::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());

	out << RoomNumber;
	out << Round;
	out << SubmitData;

	SerializeEnd(out);
}

void Packet_SubmitRoundData::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	in >> RoomNumber;
	in >> Round;
	in >> SubmitData;

	DeserializeEnd(in);
}


const Packet_Base::TypeAdder Packet_RoundResult::typeAdder("Packet_RoundResult");
void Packet_RoundResult::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());

	out << RoomNumber;
	out << Round;
	SerializeString(out, Winner);

	auto submitCount = SubmitDatas.size();
	out << submitCount;

	for (const auto& data : SubmitDatas) {
		SerializeString(out, data.first);
		out << data.second;
	}

	auto winCount = WinCounts.size();
	out << winCount;

	for (const auto& data : WinCounts) {
		SerializeString(out, data.first);
		out << data.second;
	}

	SerializeEnd(out);
}

void Packet_RoundResult::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	in >> RoomNumber;
	in >> Round;
	DeserializeString(in, Winner);

	decltype(SubmitDatas)::size_type submitCount = 0;
	in >> submitCount;

	SubmitDatas.clear();
	SubmitDatas.resize(submitCount);

	decltype(SubmitDatas)::value_type submitData;
	for (auto& data : SubmitDatas) {
		DeserializeString(in, submitData.first);
		in >> submitData.second;
		
		data = submitData;
	}

	decltype(WinCounts)::size_type winCount = 0;
	in >> winCount;

	WinCounts.clear();
	WinCounts.resize(winCount);

	decltype(WinCounts)::value_type countData;
	for (auto& data : WinCounts) {
		DeserializeString(in, countData.first);
		in >> countData.second;

		data = countData;
	}

	DeserializeEnd(in);
}


const Packet_Base::TypeAdder Packet_EndGame::typeAdder("Packet_EndGame");
void Packet_EndGame::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());

	out << RoomNumber;
	SerializeString(out, Winner);

	SerializeEnd(out);
}

void Packet_EndGame::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	in >> RoomNumber;
	DeserializeString(in, Winner);

	DeserializeEnd(in);
}