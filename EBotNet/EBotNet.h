#pragma once
#include <enet/enet.h>
#include "growtopia/TankProtocol.h"
#include "SocksTunnel.h"

class EBotClient;
typedef void (*EventHandlerReceive)(EBotClient*);
typedef void (*EventHandlerDisconnect)(EBotClient*);
typedef void (*EventHandlerConnect)(EBotClient*);

using namespace std;

enum 
{
	EBOT_BIT_INITIALIZED = (1 << 0),
	EBOT_BIT_INGAME = (1 << 1),
	EBOT_BIT_UPDATED_ITEMS = (1 << 2),
	EBOT_BIT_LOGON = (1 << 3)
};

#define C_MAX_STR_MEMBER_LEN 32
struct EBotData 
{
	int bits = EBOT_BIT_INITIALIZED; // Whether player is ingame, has updated items, has logon, etc.
	TankInfo detail;
	std::string currentWorld = "EXIT";
	int itemCycle = 0;
	float x = 0.f, y = 0.f;
};

class EBotClient 
{
public:
	ENetHost* GetHost();
	ENetPeer* GetPeer();
	void Service();
	void Kill();
	void Connect(bool reconnect = false);
	void Init(const char*, enet_uint16, enet_uint8 protocolRev = 0, const char* socksHost = NULL, enet_uint16 socksPort = 0);

	inline const ENetAddress GetAddress() { return m_address; }

	EBotClient(const char*, enet_uint16, enet_uint8 protocolRev = 0, const char* socksHost = NULL, enet_uint16 socksPort = 0);
	~EBotClient();

	void SendPacket(const int, const std::string&);
	ENetPacket* SendPacketRaw(TankPacketStruct* ts);

	void OnDisconnected();
	void OnReceived();
	void OnConnected();
	void OnServerMove(const std::string&, const std::string&, enet_uint16, enet_uint32, int, int, std::string uuid = "");

	constexpr ENetEvent& GetENetEvent() { return m_event; }

	inline void SetConnectCallback(EventHandlerConnect e) { OnConnectCallback = e; }
	inline void SetReceiveCallback(EventHandlerReceive e) { OnReceiveCallback = e; }
	inline void SetDisconnectCallback(EventHandlerDisconnect e) { OnDisconnectCallback = e; }

	bool UseSocks5(const char* socksHost, enet_uint16 socksPort = 1080);

	inline const std::string GetHostname()
	{
		char hostname[16];
		enet_address_get_host_ip(&m_address, hostname, 16);

		return std::string(hostname);
	}

	template<typename T>
	inline T* GetSender() { return static_cast<T*>(m_sender); }

	inline void SetSender(void* sender) { m_sender = sender; }

	EBotData m_data;
	ENetPacket* m_packet = NULL;
	SocksTunnel* m_proxy = NULL;
	SocksTunnel httpTunnel;

private:
	static int OnENetIntercept(ENetHost*, ENetEvent*);
	
	ENetHost* m_client = NULL;
	ENetEvent m_event;

	EventHandlerConnect OnConnectCallback = NULL;
	EventHandlerReceive OnReceiveCallback = NULL;
	EventHandlerDisconnect OnDisconnectCallback = NULL;

	void* m_sender = NULL;
	ENetAddress m_address;
};

class EBotTunnel 
{

};