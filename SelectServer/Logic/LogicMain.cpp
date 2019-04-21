#include "LogicMain.h"

#include <iostream>

using NET_ERROR_CODE = NetLib::NET_ERROR_CODE;

namespace LogicLib
{
	MainLogic::MainLogic()
	{

	}

	MainLogic::~MainLogic()
	{
		Release();
	}

	void MainLogic::Release()
	{
		if (pNetwork) {
			pNetwork->Close();
		}
	}

	ERROR_CODE MainLogic::Init()
	{
		pNetwork = std::make_unique<NetLib::SelectServer>();
		auto result = pNetwork->Initialize();

		if (result != NET_ERROR_CODE::NONE)
		{
			return ERROR_CODE::MAIN_INIT_NETWORK_INIT_FAIL;
		}

		packetProcedures.insert(std::make_pair(Packet_PingPong::typeAdder.GetType(), &MainLogic::Process_PingPong));
		
		isRun = true;
		return ERROR_CODE::NONE;
	}

	void MainLogic::Run()
	{
		while (isRun)
		{
			pNetwork->Run();

			while (true)
			{
				auto packetInfo = pNetwork->GetPacketInfo();

				if (packetInfo.PacketType == 0)
				{
					break;
				}
				else
				{
					ProcessPacket(packetInfo);
				}
			}
		}
	}

	void MainLogic::Stop()
	{
		isRun = false;
	}

	void MainLogic::ProcessPacket(const NetLib::RecvPacketInfo& packet)
	{
		if (packet.PacketType == 0) {
			return;
		}

		auto procedure = packetProcedures.find(packet.PacketType);
		if (procedure == packetProcedures.end()) {
			return;
		}

		(this->*packetProcedures[packet.PacketType])(packet);
	}

	void MainLogic::Process_PingPong(const NetLib::RecvPacketInfo& packet)
	{
		StreamReader inStream(packet.pData, packet.PacketSize);
		Packet_PingPong in;
		in.Deserialize(inStream);

		std::cout << "clients: " << in.Message << std::endl;

		Packet_PingPong out;
		out.Message = in.Message;
		char buf[Packet_Base::MAX_BUF_SIZE] = { 0, };
		StreamWriter outStream(buf, sizeof(buf));
		out.Serialize(outStream);
		
		pNetwork->SendData(packet.SessionSerial, outStream);
	}
}