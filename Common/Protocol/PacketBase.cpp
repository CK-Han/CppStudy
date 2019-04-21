#include "PacketBase.h"

#include <iostream>
#include <algorithm>
#include <numeric>
////////////////////////////////////
/*
	Serialize
	1. out << size << type << datas;
		������ ������� ���
	2. out.GetStreamSize() ������ Size ������

	Deserialize
	1. in >> packet.size >> packet.type;
		��� ����
	2. datas�� serialize���� �ߴ� ��Ĵ�� �Ľ�
*/
///////////////////////////////////

/**
	@static		��Ŷ�� type���� �ؽ��� ���� �浹(Ȥ�� �ߺ�)�� Ȯ���ϱ� ���� map
*/
std::map<Packet_Base::ValueType, Packet_Base::StringType> Packet_Base::registeredTypes;

/**
	@brief		�Էµ� ��Ŷ Ŭ���� �̸��� �ؽ��Ͽ� ���������� ����Ѵ�.
	@return		�ؽ̵� ValueType �����̸� �浹�� ���� �ߺ��� ���� �� �ִ�.

	@todo		�ؽ� �浹 �� �ٿ�ĳ���ý��� �ߺ��� ó���ؾ� �Ѵ�.
*/
Packet_Base::ValueType Packet_Base::RegisterType(const StringType& packetName)
{
	ValueType type = static_cast<ValueType>(HashType{}(packetName));

	if (false == registeredTypes.insert(std::make_pair(type, packetName)).second)
	{
		std::cerr << "RegisterType Error - Packet Name : " << packetName << ", hash : " << type << std::endl;
		std::cerr << "Existed Packet Name : " << registeredTypes[type] << std::endl;
	}

	return type;
}

Packet_Base::TypeAdder::TypeAdder(const Packet_Base::StringType& packetName)
	: type(Packet_Base::RegisterType(packetName))
{
}

/**
	@warning		������ �� �˻�� �����ϳ� �޸� ħ���� �˻����� �ʴ´�.
*/
Packet_Base::ValueType GetPacketSize(const void* buf)
{
	if (buf == nullptr)	return 0;

	Packet_Base::ValueType size = 0;
	StreamReader stream(buf, sizeof(size));
	stream >> size;

	return size;
}

/**
	@warning		������ �� �˻�� �����ϳ� �޸� ħ���� �˻����� �ʴ´�.
*/
Packet_Base::ValueType GetPacketType(const void* buf)
{
	if (buf == nullptr)	return 0;

	Packet_Base::ValueType size = 0, type = 0;
	StreamReader stream(buf, sizeof(size) + sizeof(type));
	stream >> size >> type;

	return type;
}


/////////Protected Functions////////////
/**
	@brief		��Ŷ�� ��� �ۼ�
	@warning	ó�� �Է��ϴ� Size�� ������ ���̸�, SerializeEnd���� ��Ʈ���� ũ�⸦ �����Ѵ�.
*/
void Packet_Base::SerializeBegin(StreamWriter& out, ValueType type) const
{
	out << ValueType(0) << type;	//Size is garbage value now
}

/**
	@brief		StreamWriter�� StreamSize(cursor)���� ���� ��Ŷ�� ���� ũ�⸦ �����Ѵ�.
*/
void Packet_Base::SerializeEnd(StreamWriter& out) const
{
	auto streamSize = out.GetStreamSize();
	if (streamSize > MAX_BUF_SIZE)
	{
		std::cerr << "SerializeEnd Warning - Invalid Stream Size" << std::endl;
	}

	ValueType size = static_cast<ValueType>(streamSize);
	out.OverwriteRawData(0, &size, sizeof(size));
}


void Packet_Base::DeserializeBegin(StreamReader& in, ValueType type)
{
	ValueType size, packetType;
	in >> size >> packetType;		//header parsing

	if (size > MAX_BUF_SIZE
		|| packetType != type)
	{
		std::cerr << "DeserializeBegin Warning - invalid header value" << std::endl;
	}
}

void Packet_Base::DeserializeEnd(StreamReader&)
{
	//nothing to do
}

/**
	@brief		���ڿ� Serialize, ũ�⸦ ���� �˻��Ͽ� ���� �� ���ڿ��� �����Ѵ�.
	@details	�ִ� ���̸� ����, std::min()���� ������ ���̸� ��´�.
*/
void Packet_Base::SerializeString(StreamWriter& out, const StringType& str, ValueType maxSize) const
{
	StringType::size_type strSize
		= std::min<StringType::size_type>(str.size(), maxSize);

	out << strSize;
	out.WriteRawData(str.c_str(), strSize);
}

void Packet_Base::DeserializeString(StreamReader& in, StringType& str)
{
	StringType::size_type msgSize = 0;
	in >> msgSize;

	StringType::value_type buffer[MAX_BUF_SIZE];
	in.ReadRawData(buffer, msgSize);

	str = StringType(buffer, msgSize);
}