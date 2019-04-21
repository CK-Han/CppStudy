#pragma once

#include <string>
#include <map>
#include <functional>

#include "../Util/stream.h"

/**
	@class Packet_Base
	@brief		통신을 위해 기본적인 내용이 정의된 패킷 베이스 클래스
	@details	상수 나열 - 통신에 필요한 상수들을 정의한다.
				별칭 - 패킷들이 공통적으로 사용할 기본 타입설정
				패킷 타입 관리 - 정의된 Hash를 사용, 해시값으로 타입을 구분하도록 한다.
				공통적인 직렬화 관련 동작 정의 - Begin, End 및 Stream에 정의되지 않은 동작 (컨테이너들)

	@warning	StreamWriter 및 Reader를 다루는 만큼, 예외에 신경써서 작업해야한다.
				해시 충돌을 염두해두어야 한다.

	@todo		예외 상황 처리에 대해 생각해야한다. 현재는 단순히 std::cerr에 기록한다.
*/
class Packet_Base
{
public:
	using ValueType = unsigned short;
	using StringType = std::string;
	using HashType = std::hash<StringType>;

public:
	struct Packet_Header
	{
		ValueType Size;
		ValueType Type;
	};

public:
	static const ValueType		MAX_BUF_SIZE = 4096;
	static const ValueType		HEADER_SIZE = sizeof(Packet_Header);


protected:
	class TypeAdder
	{
	public:
		TypeAdder(const StringType& packetName);
		ValueType GetType() const { return type; }

	private:
		const ValueType type;
	};


public:
	virtual void Serialize(StreamWriter&) const = 0;
	virtual void Deserialize(StreamReader&) = 0;

	virtual ~Packet_Base() {}


protected:
	void SerializeBegin(StreamWriter&, ValueType type) const;
	void SerializeEnd(StreamWriter&) const;

	void DeserializeBegin(StreamReader&, ValueType type);
	void DeserializeEnd(StreamReader&);

	void SerializeString(StreamWriter&, const StringType&, ValueType maxSize = MAX_BUF_SIZE) const;
	void DeserializeString(StreamReader&, StringType&);

private:
	static ValueType RegisterType(const StringType& packetName);

	static std::map<ValueType, StringType> registeredTypes;
};

Packet_Base::ValueType GetPacketSize(const void* buf);
Packet_Base::ValueType GetPacketType(const void* buf);