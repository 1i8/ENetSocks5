#include <cstdarg>
#include "MessageSender.h"

MessageSender::MessageSender(EBotClient* client)
{
	m_client = client;
}

MessageSender::~MessageSender()
{
	m_client->SetSender(NULL);
}

void MessageSender::SendJoinRequest(const string& name)
{
	m_client->SendPacket(3, "action|join_request\nname|" +  name + "\n");
	m_client->m_data.currentWorld = name;
}

void MessageSender::SendInput(const string& text)
{
	m_client->SendPacket(2, "action|input\n|text|" + text);
}

void MessageSender::SendDialogReturn(const vector<string>& keys, int type, ...) // int used as endmarker of the vector here, va_list would otherwise complain.
{
	size_t keysSize = keys.size();
	if (keysSize < 2)
		return;

	string extKeys;
	if (keysSize > 3)
	{
		for (size_t i = 1; i < (keysSize - 1); i++)
		{
			extKeys += keys[i];
			
			if (i < (keysSize - 2))
				extKeys += "\n";
		}
	}

	string unformatted = "action|dialog_return\ndialog_name|" + keys[0] + "\n" + extKeys + "buttonClicked|" + keys[1] + "\n";

	char buf[8192];
	va_list args;
	va_start(args, type);
	vsnprintf(buf, sizeof(buf) - 1, unformatted.c_str(), args);
	va_end(args);

	m_client->SendPacket(2, string(buf));
}


void MessageSender::SendEnterGame()
{
	m_client->SendPacket(2, "action|enter_game\n");
}

void MessageSender::SendRefreshItemData()
{
	m_client->SendPacket(2, "action|refresh_item_data\n");
}

void MessageSender::SendLogon()
{
	m_client->SendPacket(2, m_client->m_data.detail.MakeLogon());
}

void MessageSender::Kill()
{
	delete this;
}
