#include "SocksTunnel.h"
#include "EBotList.h"

#pragma warning (disable : 4996)

SocksTunnel::SocksTunnel()
{
	Init();
}

SocksTunnel::~SocksTunnel()
{
	DestroySocket();
}

const bool SocksTunnel::Init()
{
	if (_sock != ENET_SOCKET_NULL)
		DestroySocket();

	_sock = enet_socket_create(ENET_SOCKET_TYPE_STREAM);

	if (_sock == ENET_SOCKET_NULL)
	{
		printf("Error during socket initialization in SocksTunnel()!!!\n");
		return false;
	}

	enet_socket_set_option(_sock, ENET_SOCKOPT_NODELAY, 1);
	return true;
}

bool SocksTunnel::Disconnect()
{
	if (_sock == ENET_SOCKET_NULL)
		return false;

	ENetBuffer buf;
	buf.dataLength = 0;
	buf.data = NULL;

	return enet_socket_send(_sock, NULL, &buf, 0) == 0;
}

bool SocksTunnel::Connect(const char* host, enet_uint16 port)
{
	if (_sock == ENET_SOCKET_NULL)
		return false;

	if (ENetAddressSetHost(&_addr, host) != 0)
		return false;

	_addr.port = port;
	return enet_socket_connect(_sock, &_addr) == 0;
}

int SocksTunnel::Receive(std::vector<enet_uint8>& bytes, int len)
{
	ENetBuffer buffer;

	buffer.data = packetData;
	buffer.dataLength = len;

	while (len > 0)
	{
		int read = _ssl ? ReadSSL(&buffer) : enet_socket_receive(_sock, NULL, &buffer, 1);

		if (read < 0)
		{
#ifdef _DEBUG
			LogMsg("Fatal error while trying to SocksTunnel::Receive!");
#endif
			return -1;
		}

		if (read == 0)
		{
			Disconnected();
			return -1;
		}

		size_t prevSize = bytes.size();
		bytes.resize(prevSize + read);
		memcpy(bytes.data() + prevSize, buffer.data, read);
	}

	return (int)bytes.size();
}

int SocksTunnel::Receive(void* data, int len)
{
	ENetBuffer buffer;

	buffer.data = data;
	buffer.dataLength = len;

	char* ptr = (char*)data;

	while (len > 0)
	{
		int read = _ssl ? ReadSSL(&buffer) : enet_socket_receive(_sock, NULL, &buffer, 1);

		if (read < 0)
			return -1;

		if (read == 0)
		{
			Disconnected();
			return -1;
		}

		ptr += read;
		len -= read;
	}

	return ptr - (char*)data;
}

bool SocksTunnel::Send(void* data, size_t dataLen)
{
	ENetBuffer buffer;
	buffer.data = data;
	buffer.dataLength = dataLen;

	enet_uint8* dataPtr = (enet_uint8*)buffer.data;

	int sent;
	while (buffer.dataLength > 0)
	{
		sent = _ssl ? WriteSSL(buffer.data, buffer.dataLength) : enet_socket_send(_sock, NULL, &buffer, 1); // keep sending until we hit the first 0, so that we know all data has been sent.

		if (sent <= 0)
			return false;

		dataPtr += sent;
		buffer.data = dataPtr;
		buffer.dataLength -= sent;
	}

	return true;
}

// supported overloads for sending:
bool SocksTunnel::Send(const std::string& s) { return Send((void*)s.c_str(), s.length()); } // Not relevant for SOCKS5, just tested this with HTTP.
bool SocksTunnel::Send(const socks5_ident_req& s) { return Send((void*)&s, 2 + s.NumberOfMethods); }

bool SocksTunnel::Send(const socks5_auth& authMsg)
{

	if (!Send((void*)&authMsg, 2))
		return false;

	if (!Send((void*)authMsg.UserName, authMsg.UserLen))
		return false;

	if (!Send((void*)&authMsg.PassLen, 1))
		return false;

	if (!Send((void*)authMsg.Password, authMsg.PassLen))
		return false;

	char resp[2];
	if (Receive(resp, 2) == -1)
		return false;

	return resp[0] == 0x1 && resp[1] == 0;
}

bool SocksTunnel::Request(const socks5_msg& in, socks5_msg& out) 
{ 
	memset(&out, 0, sizeof(out));

	if (!Send((void*)&in, 4))
		return false;

	switch (in.AddrType)
	{
	case 1:
	{
		if (!Send((void*)&(in.DestAddr.IPv4), sizeof(in_addr)))
			return false;

		break;
	}

	case 3:
	{
		if (!Send((void*)&(in.DestAddr.DomainLen), 1))
			return false;

		if (!Send((void*)in.DestAddr.Domain, in.DestAddr.DomainLen))
			return false;

		break;
	}

	case 4:
	{
		if (!Send((void*)&(in.DestAddr.IPv6), sizeof(in6_addr)))
			return false;

		break;
	}
	
	default:
		printf("SOCKS 5 requesting unknown address type %d!\n", in.AddrType);
		break;
	}

	enet_uint16 port = htons(in.DestPort);
	if (!Send(&port, sizeof(enet_uint16)))
		return false;
	
	if (Receive(&out, 4) == -1)
		return false;

	switch (out.AddrType)
	{
	case 1:
	{
		if (Receive(&(out.DestAddr.IPv4), sizeof(in_addr)) == -1)
			return false;

		break;
	}
	case 3:
	{
		if (Receive(&(out.DestAddr.DomainLen), 1) == -1)
			return false;

		if (Receive(out.DestAddr.Domain, out.DestAddr.DomainLen) == -1)
			return false;

		break;
	}
	case 4:
	{
		if (Receive(&(out.DestAddr.IPv6), sizeof(in6_addr)) == -1)
			return false;

		break;
	}

	default:
	{
		printf("SOCKS 5 bound to unknown address type %d!\n", out.AddrType);
		return false;
	}
	}

	if (Receive(&out.DestPort, sizeof(enet_uint16)) == -1)
		return false;

	return true;
}

void SocksTunnel::Disconnected()
{
#ifdef _DEBUG
	printf("Socks Disconnect.\n");
#endif
}

bool SocksTunnel::SocksConnect(const in_addr& dest, enet_uint16 port)
{
	socks5_msg req, resp;

	req.Version = 5;
	req.Cmd = 1;
	req.Reserved = 0;
	req.AddrType = 1;
	req.DestAddr.IPv4 = dest;
	req.DestPort = port;

	if (!Request(req, resp))
		return false;

	if (resp.Cmd != 0x00)
	{
		//LogMsg("SocksConnect gave error code: " + to_string((int)resp.Cmd));
		return false;
	}

	return true;
}

bool SocksTunnel::SocksBind(const in_addr& dest, enet_uint16 port)
{
	socks5_msg req, resp;
	req.Version = 5;
	req.Cmd = 2;
	req.Reserved = 0;
	req.AddrType = 1;
	req.DestAddr.IPv4 = dest;
	req.DestPort = port;

	if (!Request(req, resp))
		return false;

	if (resp.Cmd != 0x00)
	{
#ifdef _DEBUG
		printf("SOCKS v5 OpenUDP failed, error: 0x%02X\n", resp.Cmd);
#endif
		return false;
	}
	else
	{
		_openUdp.host = *(enet_uint32*)&resp.DestAddr.IPv4;
		_openUdp.port = htons(resp.DestPort);

		if (_openUdp.host == 0 && _openUdp.port != 0)
			_openUdp.host = this->_addr.host;
	}

	return true;
}

bool SocksTunnel::SocksOpenUDP(const in_addr& dest, enet_uint16 port)
{
	socks5_msg req, resp;
	req.Version = 5;
	req.Cmd = 3;
	req.Reserved = 0;
	req.AddrType = 1;
	req.DestAddr.IPv4 = dest;
	req.DestPort = port;

	if (!Request(req, resp))
		return false;

	if (resp.Cmd != 0x00)
	{
#ifdef _DEBUG
		printf("SOCKS v5 OpenUDP failed, error: 0x%02X\n", resp.Cmd);
#endif
		return false;
	}
	else
	{
		_openUdp.host = *(enet_uint32*)&resp.DestAddr.IPv4;
		_openUdp.port = htons(resp.DestPort);

		if (_openUdp.host == 0 && _openUdp.port != 0)
			_openUdp.host = this->_addr.host;
	}

	return true;
}

bool SocksTunnel::SocksLogin(const char* user, const char* pass)
{
	size_t uLen = strlen(user), pLen = strlen(pass);
	if (uLen > 255 || pLen > 255)
		return false;

	socks5_ident_req ireq;
	ireq.Version = 5;
	ireq.NumberOfMethods = 1;
	ireq.Methods[0] = uLen == 0 ? 0x00 : 0x02;
	Send(ireq);

	socks5_ident_resp iresp;
	if (Receive(&iresp, sizeof(socks5_ident_resp)) == -1)
		return false;

	if (uLen != 0)
	{
		socks5_auth auth;
		auth.Version = 5;
		auth.UserLen = uLen;
		auth.PassLen = pLen;
		memcpy(auth.UserName, user, uLen);
		memcpy(auth.Password, pass, pLen);

		if (!Send(auth))
			return false;
	}

	return iresp.Version == 0x5 && iresp.Method != 0xFF;
}

const ENetAddress& SocksTunnel::GetProxyEndpoint()
{
	return _addr;
}

const ENetAddress& SocksTunnel::GetOpenUDPRelayEndpoint()
{
	return _openUdp;
}

void SocksTunnel::DestroySocket()
{
	enet_socket_destroy(_sock);
	_sock = ENET_SOCKET_NULL;
}

void SocksTunnel::Kill()
{
	delete this;
}

void SocksTunnel::KillSSL()
{
	if (_ssl)
	{
		SSL_free(_ssl);
		_ssl = NULL;
	}
}

const bool SocksTunnel::InitSSL()
{
	// Initialize SSL:
	auto ctx = SSL_CTX_new(TLS_client_method());

	if (!ctx)
	{
		LogMsg("Failed to initialize new SSL context!");
		return false;
	}

	
	_ssl = SSL_new(ctx);
	if (!_ssl)
	{
		LogMsg("Failed to allocate new SSL!");
		SSL_CTX_free(ctx);
		return false;
	}

	SSL_set_fd(_ssl, _sock);

	const int status = SSL_connect(_ssl);
	if (status != 1)
	{
		SSL_get_error(_ssl, status);
		LogMsg("SSL_connect failed with error code: " + to_string(status));
		SSL_CTX_free(ctx);
		KillSSL();
		return false;
	}

	return true;
}

int SocksTunnel::WriteSSL(void* data, const size_t len)
{
	if (!_ssl)
		return -1;

	return SSL_write(_ssl, data, len);
}

void SocksTunnel::WriteSSL(const std::string& str)
{
	WriteSSL((void*)str.c_str(), str.length());
}

int SocksTunnel::ReadSSL(ENetBuffer* buf)
{
	int len = SSL_read(_ssl, buf->data, sizeof(packetData));

	return len;
}
