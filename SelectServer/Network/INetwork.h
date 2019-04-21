#pragma once

#include "../../Common/Protocol/PacketBase.h"
#include "../../Common/Protocol/protocol.h"
#include "../../Common/Util/stream.h"
#include "../../Common/Protocol/Define.h"
#include "NetErrno.h"

namespace NetLib
{
	struct RecvPacketInfo;

	class INetwork
	{
	public:
		using SERIAL_TYPE = unsigned int;
		using SIZE_TYPE = StreamBase::SizeType;
		using PACKET_TYPE = decltype(Packet_Base::Packet_Header::Type);
		using PACKET_SIZE = decltype(Packet_Base::Packet_Header::Size);
		static const SERIAL_TYPE INVALID_SERIAL = (std::numeric_limits<SERIAL_TYPE>::max)();

	public:
		virtual NET_ERROR_CODE Initialize() = 0;
		virtual void Run() = 0;
		virtual void Close() = 0;

		virtual NET_ERROR_CODE SendData(SERIAL_TYPE sessionSerial, StreamWriter& out) = 0;
		virtual RecvPacketInfo GetPacketInfo() = 0;
	};

	struct RecvPacketInfo
	{
		INetwork::SERIAL_TYPE		SessionSerial{ INetwork::INVALID_SERIAL };
		INetwork::PACKET_TYPE		PacketType{ 0 };
		INetwork::PACKET_SIZE		PacketSize{ 0 };
		char*						pData{ nullptr };
	};
}