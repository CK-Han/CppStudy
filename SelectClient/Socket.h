#pragma once
#pragma comment(lib, "ws2_32.lib")
#include <WS2tcpip.h>
#include <queue>
#include <mutex>
#include <memory>

#include "../Common/Protocol/PacketBase.h"
#include "../Common/Protocol/protocol.h"
#include "../Common/Util/stream.h"
#include "../Common/Protocol/Errno.h"
#include "../Common/Protocol/Define.h"

class Framework;

class Socket
{
public:
	struct SendData
	{
		const char* buf;
		int size;
	};

public:
	Socket();
	~Socket();

	bool Initialize(Framework* framework, const char* serverIP);
	bool IsRun() { return !disconnect; }
	void Run();

	void AddSendPacket(const char* packet, int size);
	void SetDisconnect(bool disconnect);

private:
	void ProcessPacket(const void* packet, int size);

	void ReadPacket();
	void SendPacket();

private:
	Framework*				framework{ nullptr };
	bool					disconnect{ true };
	SOCKET					clientSocket{ INVALID_SOCKET };
	FD_SET					readSet;
	char					recvBuf[Packet_Base::MAX_BUF_SIZE]{ 0, };
	int						savedPacketSize{ 0 };
	std::queue<SendData>	sendQueue;
	std::mutex				sendQueueLock;
};

