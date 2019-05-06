#include "PacketBase.h"
#include <vector>

/**
	@brief		�׽�Ʈ�� ����
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
	@brief		���� �������� ���, ���� ���� ���� ó��
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
	@brief		Ŭ���̾�Ʈ -> ���� : �� �̸����� �����ϰ�ʹ�
				���� -> Ŭ���̾�Ʈ : Created�� ���� �Ǿ����� �˸�
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
	@brief		�α��� �� ���� ��ü�� ä�� ��û
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
	@brief		���� ��ü �� ����Ʈ ��û, �������� ��, ���� ������ ��
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
	@brief		Ŭ�� -> ���� : �� ������ ���� �ʹ�
				���� -> Ŭ�� : �ش������ ������ Ȯ��
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
	@brief		Ŭ�� -> ���� : �� �濡�� ������ �ʹ�
				���� -> Ŭ�� : �濡�� �������� Ȯ��
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
	@brief		���� -> Ŭ�� : �ش� �� ���� ����
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
	@brief		Ŭ�� -> ���� : ���� ����ڴ�.
				���� -> Ŭ�� : �� ���� ����, �� ��ȣ ����
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
	@brief		Ŭ�� -> ���� : ������ ���� ready �Ѵ� (�濡 2���� �����Ҷ��� ����)
				���� -> Ŭ�� : ready �� ó���Ǿ����� Ȯ��
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
	@brief		���� -> Ŭ�� : �� ���� �� ready�Ͽ� ������ �����Ұ����� �˸�
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
	@brief		���� -> Ŭ�� : �ش� ���尡 ���۵Ǿ����� �˸�
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
	@brief		Ŭ�� -> ���� : �̹� ���忡�� ���� �� �и� ���ڴ�
				���� -> Ŭ�� : ��û�� �� ó���Ǿ���
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
	@brief		���� -> Ŭ�� : �ش� ������ ��� ����
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
	@brief		���� -> Ŭ�� : ���� �¸��� ����, ����
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