#pragma once
#include "../EBotList.h"
#include "../EBotNet.h"
#include "TankProtocol.h"

class MessageSender 
{
public:
	MessageSender(EBotClient*);
	~MessageSender();

	void SendJoinRequest(const string&);
	void SendEnterGame();
	void SendRefreshItemData();
	void SendLogon();
	void Kill();
	void SendInput(const string&);
	void SendDialogReturn(const vector<string>&, int, ...);

private:
	EBotClient* m_client;
};