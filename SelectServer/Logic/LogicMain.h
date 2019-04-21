#pragma once

#include <memory>
#include <map>

#include "../Network/Network.h"
#include "../../Common/Protocol/Errno.h"

namespace NetLib
{
	class INetwork;
}

namespace LogicLib
{
	class MainLogic
	{
	private:
		using Packet_Type = decltype(NetLib::RecvPacketInfo::PacketType);
		using Packet_Procedure = void(MainLogic::*)(const NetLib::RecvPacketInfo&);

	public:
		MainLogic();
		~MainLogic();

		ERROR_CODE Init();

		void Run();

		void Stop();

	private:
		void Release();


		void ProcessPacket(const NetLib::RecvPacketInfo& packet);
		void Process_PingPong(const NetLib::RecvPacketInfo& packet);

	private:
		bool isRun{ false };
		
		std::unique_ptr<NetLib::INetwork> pNetwork;

		std::map<Packet_Type, Packet_Procedure> packetProcedures;
	};
}