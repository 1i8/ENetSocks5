#include <cstdarg>
#include <iostream>
#include "EBotList.h"

CallbackManager* g_callbackManager;
vector<EBotClient*> g_eBots;
vector<string> g_logs;
mutex g_mutex;
mutex _logmut, _listmut;

int ENetAddressSetHost(ENetAddress* addr, const char* host)
{
	lock_guard<mutex> lock(g_mutex);
	return enet_address_set_host(addr, host);
}

string FormatStr(const char* fmt, ...)
{
	char buf[8192];
	va_list arg;
	va_start(arg, fmt);
	vsnprintf(buf, sizeof(buf) - 1, fmt, arg);
	va_end(arg);
	return std::string(buf);
}

void LogMsg(const string& log)
{
	lock_guard<mutex> lock(_logmut);
	g_logs.push_back(log);
}

void FlushLogs()
{
	lock_guard<mutex> lock(_logmut);

	for (const string& log : g_logs)
		cout << log << '\n';

	g_logs.clear();
}

size_t AddEBot(EBotClient* eBot) 
{
	lock_guard<mutex> lock(_listmut);
	g_eBots.push_back(eBot);
	return g_eBots.size();
}

bool RemoveEBot(EBotClient* eBot) 
{
	lock_guard<mutex> lock(_listmut);
	bool state = false;
	auto it = find(g_eBots.begin(), g_eBots.end(), eBot);

	state = it != g_eBots.end();

	if (state)
		g_eBots.erase(it);

	delete eBot;
	return state;
}