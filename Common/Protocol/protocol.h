#include "PacketBase.h"

/**
	@brief		�׽�Ʈ�� ����
*/
struct Packet_PingPong
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	StringType	Message{};
};

/**
	@brief		Ŭ���̾�Ʈ -> ���� : �� �̸����� �����ϰ�ʹ�
				���� -> Ŭ���̾�Ʈ : Created�� ���� �Ǿ����� �˸�
*/
struct Packet_Login
	: public Packet_Base
{
	static const TypeAdder typeAdder;

	void Serialize(StreamWriter&) const;
	void Deserialize(StreamReader&);

	bool			Created{ false };
	StringType		UserName{};
};

