#pragma once
#include <vector>
#include "growtopia/CallbackManager.h"
#include <mutex>
#include "EBotNet.h"

using namespace std;
// Global list of EBotClients:

extern mutex g_mutex;
extern CallbackManager* g_callbackManager;

// Thread-safe version of enet_address_set_host (uses gethostbyname internally, not thread-safe on all platforms it seems.)
extern int ENetAddressSetHost(ENetAddress* addr, const char* host);

extern vector<string> g_logs;

extern vector<EBotClient*> g_eBots;

extern string FormatStr(const char* fmt, ...);

extern void LogMsg(const string&);

extern void FlushLogs();

extern size_t AddEBot(EBotClient* eBot);

extern bool RemoveEBot(EBotClient* eBot);