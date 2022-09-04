#pragma once
#include "../EBotNet.h"

namespace MessageHandler 
{
	void InitCallbacks();
	void ProcessConnect(EBotClient*);
	void ProcessDisconnect(EBotClient*);
	void ProcessPacket(EBotClient*);
}