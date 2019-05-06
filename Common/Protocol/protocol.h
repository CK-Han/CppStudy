#include "PacketBase.h"
#include <vector>

/**
	@brief		테스트용 핑퐁
*/
struct Packet_PingPong
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	StringType	Message;
};

/**
	@brief		서버 내에서만 사용, 유저 접속 종료 처리
*/
struct Packet_SessionClose
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	unsigned long long session{ 0 };
};

/**
	@brief		클라이언트 -> 서버 : 이 이름으로 접속하고싶다
				서버 -> 클라이언트 : Created로 생성 되었는지 알림
*/
struct Packet_RegisterId
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	bool			Created{ false };
	StringType		UserName;
};

/**
	@brief		로그인 후 대기실 전체에 채팅 요청
*/
struct Packet_ChatAll
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	StringType		UserName;
	StringType		Message;
};

/**
	@brief		서버 전체 방 리스트 요청, 진행중인 방, 입장 가능한 방
*/
struct Packet_GetRoomList
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	ValueType					PlayedRoomCount{ 0 };
	std::vector<ValueType>		WaitedRooms;
};

/**
	@brief		클라 -> 서버 : 이 방으로 들어가고 싶다
				서버 -> 클라 : 해당방으로 들어갔음을 확인
*/
struct Packet_EnterRoom
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	StringType		User;
	bool			Entered{ false };
	ValueType		RoomNumber{ 0 };
};

/**
	@brief		클라 -> 서버 : 이 방에서 나가고 싶다
				서버 -> 클라 : 방에서 나갔는지 확인
*/
struct Packet_LeaveRoom
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	StringType		User;
	bool			Leaved{ false };
	ValueType		LeavedRoomNumber{ 0 };
};

/**
	@brief		서버 -> 클라 : 해당 방 정보 전달
*/
struct Packet_RoomInfo
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	ValueType					RoomNumber{ 0 };
	std::vector<StringType>		Users;
};

/**
	@brief		클라 -> 서버 : 방을 만들겠다.
				서버 -> 클라 : 방 생성 여부, 방 번호 전달
*/
struct Packet_CreateRoom
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	bool			Created{ false };
	ValueType		RoomNumber{ 0 };
};

/**
	@brief		클라 -> 서버 : 게임을 위해 ready 한다 (방에 2명이 존재할때만 가능)
				서버 -> 클라 : ready 잘 처리되었는지 확인
*/
struct Packet_ReadyRoom
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	bool			Readyed{ false };
	ValueType		RoomNumber{ 0 };
};

/**
	@brief		서버 -> 클라 : 두 명이 다 ready하여 게임을 시작할것임을 알림
*/
struct Packet_StartGame
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	ValueType					RoomNumber{ 0 };
	std::vector<StringType>		Users;
};

/**
	@brief		서버 -> 클라 : 해당 라운드가 시작되었음을 알림
*/
struct Packet_BeginRound
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	ValueType		RoomNumber{ 0 };
	ValueType		Round{ 0 };
};

/**
	@brief		클라 -> 서버 : 이번 라운드에는 내가 이 패를 내겠다
				서버 -> 클라 : 요청이 잘 처리되었다
*/
struct Packet_SubmitRoundData
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	ValueType		RoomNumber{ 0 };
	ValueType		Round{ 0 };
	ValueType		SubmitData{ 0 };
};

/**
	@brief		서버 -> 클라 : 해당 라운드의 결과 전송
*/
struct Packet_RoundResult
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	ValueType		RoomNumber{ 0 };
	ValueType		Round{ 0 };
	StringType		Winner;
	std::vector<std::pair<StringType/*name*/, ValueType/*SubmitData*/>> SubmitDatas;
	std::vector<std::pair<StringType/*name*/, ValueType/*winCount*/>> WinCounts;
};

/**
	@brief		서버 -> 클라 : 최종 승리자 선발, 전달
*/
struct Packet_EndGame
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	ValueType		RoomNumber{ 0 };
	StringType		Winner;
};