#include "EBotList.h"
#include "EBotNet.h"
#include "growtopia/utils.hpp"
#include "growtopia/MessageSender.h"
#include <thread>

#pragma warning (disable : 4996)

ENetHost* EBotClient::GetHost()
{
	return m_client;
}

ENetPeer* EBotClient::GetPeer()
{
	return m_client ? &m_client->peers[0] : NULL;
}

void EBotClient::Service()
{
	if (!m_client)
		return;

	while (enet_host_service(m_client, &m_event, 1000) > 0)
	{
		switch (m_event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			OnConnected();
			break;

		case ENET_EVENT_TYPE_RECEIVE:
			OnReceived();
			
			enet_packet_destroy(m_event.packet);
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			OnDisconnected();
			return;

		default: // ENET_EVENT_TYPE_NONE
			break;
		}
	}
}

#define USE_HTTP_TUNNEL
//#define USE_HTTPS_FOR_TUNNEL // (enable this if you want the socks5 to get server_data.php via HTTPS too!)

void EBotClient::Connect(bool reconnect)
{
	if (m_proxy)
	{
		ENetAddress eAddr = m_proxy->GetProxyEndpoint();
		in_addr addr;
		*(int*)&addr = GetHost()->relayAddress.host;

#ifdef USE_HTTP_TUNNEL
		if (reconnect)
		{
			if (!httpTunnel.Init())
				return;
		}

		if (httpTunnel.Connect(IPAddress(eAddr.host).GetAsString().c_str(), eAddr.port))
		{
			if (httpTunnel.SocksLogin())
			{
				if (httpTunnel.SocksConnect(addr, 443))
				{
#ifdef _DEBUG
					LogMsg("Sending http tunnel...");
#endif
#ifdef USE_HTTPS_FOR_TUNNEL
					if (httpTunnel.InitSSL())
					{
#endif
						httpTunnel.Send(
							"POST /growtopia/server_data.php HTTP/1.1\r\n"
							"Accept: */*\r\n"
							"Host: www.growtopia1.com\r\n"
							"Connection: close\r\n"
							"Content-Type: application/x-www-form-urlencoded\r\n"
							"Content-Length: 36\r\n\r\n"
							"User-Agent: UbiServices_SDK_2017.Final.21_ANDROID64_static\r\n"
							"version=3.99&platform=4&protocol=173\r\n");

#ifdef USE_HTTPS_FOR_TUNNEL
					}
					else
					{
						LogMsg("Could not initialize SSL for the tunnel!");
					}
#endif
				}
			}
		}
#endif
	}

	if (!(enet_host_connect(m_client, &m_address, 2, 0)))
	{
#ifdef _DEBUG
		LogMsg("Could not initiate an ENet connection!");
#endif
		enet_peer_reset(GetPeer());
	}
}

void EBotClient::Init(const char* host, enet_uint16 port, enet_uint8 protocolRev, const char* socksHost, enet_uint16 socksPort)
{
	Kill();

	if (!(m_client = enet_host_create(NULL, 1, 2, 0, 0)))
	{
		printf("Couldn't allocate an ENetHost!!!\n");
		exit(EXIT_FAILURE);
	}

	m_client->intercept = OnENetIntercept;
	m_client->checksum = enet_crc32;
	m_client->protocolRev = protocolRev;
	enet_host_compress_with_range_coder(m_client);

	ENetAddressSetHost(&m_address, host);
	m_address.port = port;

	if (socksHost)
	{
#ifdef _DEBUG
		LogMsg("SOCKS host specified, engage...");
#endif

		if (UseSocks5(socksHost, socksPort))
			m_address = m_proxy->GetOpenUDPRelayEndpoint();
		else
			Kill();
	}
}


EBotClient::EBotClient(const char* host, enet_uint16 port, enet_uint8 protocolRev, const char* socksHost, enet_uint16 socksPort)
{
	Init(host, port, protocolRev, socksHost, socksPort);
}

void EBotClient::OnDisconnected()
{
#ifdef _DEBUG
	LogMsg("Disconnected from server.");
#endif

	if (OnDisconnectCallback)
		OnDisconnectCallback(this);
}

ENetPacket* EBotClient::SendPacketRaw(TankPacketStruct* ts)
{
	return utils::SendPacketRaw(GetPeer(), ts);
}

void EBotClient::OnReceived()
{
	if (OnReceiveCallback)
		OnReceiveCallback(this);
}

void EBotClient::OnConnected()
{
	if (OnConnectCallback)
		OnConnectCallback(this);
}

void EBotClient::OnServerMove(const std::string& host, const std::string& doorID, enet_uint16 port,
	enet_uint32 userID, int token, int lmode, std::string uuid)
{
	ENetPeer* peer = GetPeer();
	if (peer->state != ENET_PEER_STATE_DISCONNECTED)
	{
		enet_host_flush(m_client);
		enet_peer_reset(peer);
	}

	m_data.detail.doorID = doorID;
	m_data.detail.userID = userID;
	m_data.detail.token = token;
	m_data.detail.lmode = lmode;
	m_data.detail.uuid_token = uuid;

	ENetAddressSetHost(&m_address, host.c_str());
	m_address.port = port;

#ifdef _DEBUG
	LogMsg(FormatStr("Moving to server %s:%d, user %d, token %d, lmode %d",
		host.c_str(), port, userID, token, lmode));
#endif
	
	Connect();
}

// only authentication-less supported as of now.
bool EBotClient::UseSocks5(const char* socksHost, enet_uint16 socksPort)
{
	if (m_proxy)
	{
		enet_host_udp_tunnel(m_client, &m_proxy->GetOpenUDPRelayEndpoint());
		return true;
	}

	m_proxy = new SocksTunnel;
	if (m_proxy->Connect(socksHost, socksPort))
	{
#ifdef _DEBUG
		LogMsg("Connected to SOCKS5 proxy, authenticating...");
#endif

		if (m_proxy->SocksLogin())
		{
			//LogMsg("Authentication successful! Sending UDP associate to obtain UDP relay server...");
			struct in_addr addr;
			*(int*)&addr = 0;

			if (m_proxy->SocksOpenUDP(addr, 0))
			{

				auto& relayAddr = m_proxy->GetOpenUDPRelayEndpoint();
#ifdef _DEBUG
				LogMsg("[VERBOSE] Relay server: " + IPAddress(relayAddr.host).GetAsString() + ":" +
					to_string(relayAddr.port));

				LogMsg("[VERBOSE] Target relay is: " + IPAddress(m_address.host).GetAsString() + ":" +
					to_string(m_address.port));
#endif
				enet_host_udp_tunnel(m_client, &m_address);

				return true;
			}
		}
		else
		{
#ifdef _DEBUG
			LogMsg("Authentication fail! Fallback to native connection...");
#endif
		}
	}

	return false;
}

EBotClient::~EBotClient()
{
	Kill();
}

void EBotClient::SendPacket(const int mType, const std::string& str)
{
	if (!m_client)
		return;

	if (!utils::SendPacket(GetPeer(), mType, str))
	{
#ifdef _DEBUG
		LogMsg("EBotNet: SendPacket failure! Are we connected to ENet?!");
#endif
	}
}

void EBotClient::Kill()
{
	if (m_client)
	{
		enet_host_flush(m_client);
		enet_host_destroy(m_client);
		m_client = NULL;
	}

	if (m_sender)
	{
		delete (MessageSender*)m_sender;
		m_sender = NULL;
	}

	if (m_packet)
	{
		enet_packet_destroy(m_packet);
		m_packet = NULL;
	}
}

int ENET_CALLBACK EBotClient::OnENetIntercept(ENetHost* host, ENetEvent* ev)
{
	//char hostname[16];
	//enet_address_get_host_ip(&host->receivedAddress, hostname, sizeof(hostname));

	//printf("Got plain UDP packet of length %lld from IPv4 %s!\n", host->receivedDataLength, hostname);

	return 0;
}
