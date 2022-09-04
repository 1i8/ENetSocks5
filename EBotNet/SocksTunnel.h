#pragma once
#include <vector>
#include <string>
#include <enet/enet.h>
#ifdef _WIN32
#include <in6addr.h>
#endif

struct IPAddress {
	char ip[16] = { 0 };

	std::string GetAsString()
	{
		return std::string(ip);
	}

	int GetAsInt()
	{
		ENetAddress addr;
		enet_address_set_host_ip(&addr, ip);

		return addr.host;
	}

	IPAddress(int ipv4)
	{
		Set(ipv4);
	}

	void SetFromStr(const std::string& ip)
	{
		ENetAddress addr;

		enet_address_set_host_ip(&addr, ip.c_str());
		Set(addr.host);
	}

	void Set(int ipv4)
	{
		ENetAddress addr;
		addr.host = ipv4;

		enet_address_get_host_ip(&addr, ip, 16);
	}
};

struct socks5_ident_req
{
    enet_uint8 Version;
    enet_uint8 NumberOfMethods;
    enet_uint8 Methods[256];
};

struct socks5_ident_resp
{
    enet_uint8 Version;
    enet_uint8 Method;
};

struct socks5_msg
{
    enet_uint8 Version;
    enet_uint8 Cmd;
    enet_uint8 Reserved;
    enet_uint8 AddrType;
    union {
        in_addr IPv4;
        in6_addr IPv6;
        struct {
            enet_uint8 DomainLen;
            char Domain[256];
        };
    } DestAddr;
    enet_uint16 DestPort; // can also be BindPort
};

struct socks5_auth
{
	enet_uint8 Version;
	enet_uint8 UserLen;
	char UserName[256];
	enet_uint8 PassLen;
	char Password[256];
};


class SocksTunnel // Basically Custom ENetHost just for universal (unix & win) TCP sending/receiving
{
public:
	SocksTunnel();
    ~SocksTunnel();
	void Init();
	bool Disconnect();
	bool Connect(const char*, enet_uint16);
	int Receive(std::vector<enet_uint8>& bytes, int len = ENET_PROTOCOL_MAXIMUM_MTU);
    int Receive(void*, int);
	bool Send(void*, size_t);
	bool Send(const std::string&);
	bool Send(const socks5_ident_req&);
	bool Send(const socks5_auth&);
    bool Request(const socks5_msg&, socks5_msg&);
    void Disconnected();
    bool SocksConnect(const in_addr&, enet_uint16);
	bool SocksBind(const in_addr&, enet_uint16);
    bool SocksOpenUDP(const in_addr&, enet_uint16);
    bool SocksLogin(const char* user = "", const char* pass = "");
	const ENetAddress& GetProxyEndpoint();
    const ENetAddress& GetOpenUDPRelayEndpoint();
	const ENetAddress& GetOpenTCPRelayEndpoint();
	void DestroySocket();
    void Kill();

private:
	enet_uint8 packetData[ENET_PROTOCOL_MAXIMUM_MTU];
	ENetSocket _sock = ENET_SOCKET_NULL;
	ENetAddress _addr;
    ENetAddress _openUdp;
	ENetAddress _openTcp;
};