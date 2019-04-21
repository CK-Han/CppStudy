#include "PacketBase.h"

/**
	@brief		테스트용 핑퐁
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
	@brief		클라이언트 -> 서버 : 이 이름으로 접속하고싶다
				서버 -> 클라이언트 : Created로 생성 되었는지 알림
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

