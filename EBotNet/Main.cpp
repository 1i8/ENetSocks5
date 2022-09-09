// PROJECT "SATAN'S NUKE II", by DEERUX aka playingo.

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <future>
#include <thread>
#include <numeric>
#include <string>
#include <fstream>
#include <streambuf>
#include <execution>
#include "EBotList.h"
#include "EBotNet.h"
#include "SocksTunnel.h"
#include "growtopia/MessageHandler.h"
#include "growtopia/utils.hpp"
#include "httplib.h"
#include "growtopia/MessageSender.h"
#include <enet/utility.h>

#pragma warning (disable : 4996)

uint64_t mix(uint64_t a, uint64_t b, uint64_t c)
{

	a = a - b;  a = a - c;  a = a ^ (c >> 13);
	b = b - c;  b = b - a;  b = b ^ (a << 8);
	c = c - a;  c = c - b;  c = c ^ (b >> 13);
	a = a - b;  a = a - c;  a = a ^ (c >> 12);
	b = b - c;  b = b - a;  b = b ^ (a << 16);
	c = c - a;  c = c - b;  c = c ^ (b >> 5);
	a = a - b;  a = a - c;  a = a ^ (c >> 3);
	b = b - c;  b = b - a;  b = b ^ (a << 10);
	c = c - a;  c = c - b;  c = c ^ (b >> 15);
	return c;
}

void init()
{
	MessageHandler::InitCallbacks();

	if (enet_initialize() != 0)
	{
		printf("ENetInit failure!\n");
		exit(EXIT_FAILURE);
	}

	atexit(enet_deinitialize);
}

#define C_PROTOREV 2


atomic<size_t> botIDCounter{ 9999 }; // Use this as port.

static const string gthost = "growtopia1.com";

std::string loadstr(const string& path)
{
	std::ifstream t(path);
	return std::string((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());
}

void launch_bot(string gtip, enet_uint16 gtport, TankInfo tankOverride)
{
	EBotClient* pBot = new EBotClient(gtip.c_str(), gtport, C_PROTOREV/*, "your.proxy.host", ++botIDCounter*/); // Up to 32K threads
	// (32K of these are going to be UDP sockets and 32K of these TCP sockets, to keep the socks5 alive. Has custom ENet to support beyond 1024 and 
	// extreme optimizations like less frequent pinging and very fast framework and custom ENet tweaks to hold this many bots.)
	
	if (!pBot->GetHost())
	{
		LogMsg("Had to discard a client due to failure to connect.");
		delete pBot;
		return;
	}

	TankInfo& det = pBot->m_data.detail;
	det.SetTankIDName(string_view(tankOverride.tankIDName));
	det.SetTankIDPass(string_view(tankOverride.tankIDPass));
	det.meta = tankOverride.meta;

	pBot->SetDisconnectCallback(MessageHandler::ProcessDisconnect);
	pBot->SetConnectCallback(MessageHandler::ProcessConnect);
	pBot->SetReceiveCallback(MessageHandler::ProcessPacket);
	pBot->Connect();

	while (1)
	{
		pBot->Service();

		if (pBot->GetPeer()->state != ENET_PEER_STATE_CONNECTED)
			continue;

		// Can do misc. stuff in here, if wanted.
	}
}

void startup()
{
	srand(mix(clock(), time(NULL), getpid()));
	string gtip = gthost, meta;
	enet_uint16 gtport = 24000;

	httplib::Client cli("https://www.growtopia1.com");
	cli.enable_server_certificate_verification(false);
	httplib::Headers headers = {
	{ "User-Agent", "UbiServices_SDK_2019.Release.27_PC64_unicode_static" },
	{ "Accept", "/" }
	};
	auto res = cli.Post("/growtopia/server_data.php", headers, "version=3.98&protocol=164&platform=0", "application/x-www-form-urlencoded");
	if (res) {
		auto body = res.value().body;
		LogMsg("Got HTTP data: " + body);

		body.erase(remove(body.begin(), body.end(), '\r'), body.end());

		utils::TextScanner scanner(body);
		try {
			gtport = stoi(string(scanner.Scan("port")));
		}
		catch (...)
		{
			goto LABEL_FAILURE;
		}

		gtip = scanner.Scan("server");
		meta = scanner.Scan("meta");
	}
	else
	{
	LABEL_FAILURE:
		LogMsg("Had to discard HTTP due to failure contacting HTTP.");
	}

	cli.stop();

	auto s = loadstr("accs.txt");
	auto accs = utils::explode(loadstr("accs.txt"), '\n');

	LogMsg("Accounts loaded: " + to_string(accs.size()));

	// DO *NOT* USE MORE THAN 5 BOTS IF YOU DONT HAVE BOUGHT SOCKS5 PROXY from SOAX.COM or PROXYRACK.COM OR ANY OTHER PROVIDER THAT SUPPORTS SOCKS5 UDP ASSOCATION!!

	for (int i = 0; i < 1; i++)
	{
		TankInfo tInfo;
		tInfo.SetTankIDName(utils::gen_random(8)); // This should create a new pass.
		tInfo.SetTankIDPass(utils::gen_random(8));
		tInfo.meta = meta;

		thread t([=]
			{
				launch_bot(gtip, gtport, tInfo);
			});

		t.detach();
	}

	while (1)
	{
		FlushLogs();
		this_thread::sleep_for(10ms);
	}
}

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		int startPort = atoi(argv[1]);
		botIDCounter = startPort;
		printf("Started up at port %d!\n", startPort);
	}

	init();
	startup();

	return 0;
}