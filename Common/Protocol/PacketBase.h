#pragma once

#include <string>
#include <map>
#include <functional>

#include "../Util/stream.h"

/**
	@class Packet_Base
	@brief		����� ���� �⺻���� ������ ���ǵ� ��Ŷ ���̽� Ŭ����
	@details	��� ���� - ��ſ� �ʿ��� ������� �����Ѵ�.
				��Ī - ��Ŷ���� ���������� ����� �⺻ Ÿ�Լ���
				��Ŷ Ÿ�� ���� - ���ǵ� Hash�� ���, �ؽð����� Ÿ���� �����ϵ��� �Ѵ�.
				�������� ����ȭ ���� ���� ���� - Begin, End �� Stream�� ���ǵ��� ���� ���� (�����̳ʵ�)

	@warning	StreamWriter �� Reader�� �ٷ�� ��ŭ, ���ܿ� �Ű�Ἥ �۾��ؾ��Ѵ�.
				�ؽ� �浹�� �����صξ�� �Ѵ�.

	@todo		���� ��Ȳ ó���� ���� �����ؾ��Ѵ�. ����� �ܼ��� std::cerr�� ����Ѵ�.
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