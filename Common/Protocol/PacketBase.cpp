#include "PacketBase.h"

#include <iostream>
#include <algorithm>
#include <numeric>
////////////////////////////////////
/*
	Serialize
	1. out << size << type << datas;
		데이터 순서대로 담기
	2. out.GetStreamSize() 값으로 Size 재조정

	Deserialize
	1. in >> packet.size >> packet.type;
		헤더 제거
	2. datas를 serialize에서 했던 방식대로 파싱
*/
///////////////////////////////////

/**
	@static		패킷의 type값을 해싱한 이후 충돌(혹은 중복)을 확인하기 위한 map
*/
std::map<Packet_Base::ValueType, Packet_Base::StringType> Packet_Base::registeredTypes;

/**
	@brief		입력된 패킷 클래스 이름을 해싱하여 관리구조에 등록한다.
	@return		해싱된 ValueType 정수이며 충돌로 인해 중복된 값일 수 있다.

	@todo		해시 충돌 및 다운캐스팅시의 중복을 처리해야 한다.
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
	@warning		버퍼의 널 검사는 진행하나 메모리 침범은 검사하지 않는다.
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
	@warning		버퍼의 널 검사는 진행하나 메모리 침범은 검사하지 않는다.
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
	@brief		패킷의 헤더 작성
	@warning	처음 입력하는 Size는 쓰레기 값이며, SerializeEnd에서 스트림의 크기를 설정한다.
*/
void Packet_Base::SerializeBegin(StreamWriter& out, ValueType type) const
{
	out << ValueType(0) << type;	//Size is garbage value now
}

/**
	@brief		StreamWriter의 StreamSize(cursor)값을 통해 패킷의 최종 크기를 설정한다.
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
	@brief		문자열 Serialize, 크기를 먼저 검사하여 복사 후 문자열을 복사한다.
	@details	최대 길이를 지정, std::min()으로 복사할 길이를 얻는다.
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