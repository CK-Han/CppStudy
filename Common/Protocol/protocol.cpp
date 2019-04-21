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

const Packet_Base::TypeAdder Packet_Login::typeAdder("Packet_Login");
void Packet_Login::Serialize(StreamWriter& out) const
{
	SerializeBegin(out, typeAdder.GetType());
	
	out << Created;
	SerializeString(out, UserName, static_cast<ValueType>(UserName.length()));

	SerializeEnd(out);
}

void Packet_Login::Deserialize(StreamReader& in)
{
	DeserializeBegin(in, typeAdder.GetType());

	in >> Created;
	DeserializeString(in, UserName);

	DeserializeEnd(in);
}

