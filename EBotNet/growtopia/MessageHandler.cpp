#include <fstream>
#include <mutex>
#include <thread>
#include "../EBotList.h"
#include "MessageHandler.h"
#include "MessageSender.h"
#include "utils.hpp"
#include "VariantList.h"
#include "VariantFuncs.h"
#include "../SocksTunnel.h"

using namespace Variant;

mutex m;
void RegisterAccount(EBotClient* pClient, const std::string& btnName)
{
	auto growID = utils::gen_random2(8);

#ifdef LOG_ACCS
	ofstream outfile;

	m.lock();
	outfile.open("accs.txt", std::ios_base::app); // append instead of overwrite
	outfile << growID << "\n";

	outfile.close();
	m.unlock();
#endif

	//LogMsg("Creating account with GrowID: " + growID + "...");

	pClient->SendPacket(2, "action|dialog_return\ndialog_name|register\nusername|" + growID + "\npassword|noobs123\npasswordverify|noobs123\nemail|" + utils::gen_random2(12) + "@gmail.com\nbuttonClicked|" + btnName);
}

void MessageHandler::InitCallbacks()
{
	g_callbackManager = new CallbackManager;
	g_callbackManager->SetCallback("OnConsoleMessage", [](CALLBACK_ARGS) 
	{ 
		string str = vList->GetFuncArg(1).strVal;
		LogMsg(str);


		if (str.find("network") != string::npos || str.find("entering") != string::npos)
		{
			auto pClient = ((EBotClient*)context);
			auto ePeer = pClient->GetPeer();
			auto pSender = pClient->GetSender<MessageSender>();

			pSender->SendJoinRequest("START");
		}

		/*if (str.find("Sorry") != string::npos || str.find("System") != string::npos || str.find("OOPS") != string::npos)
		{
			auto pClient = ((EBotClient*)context);
			auto ePeer = pClient->GetPeer();

			//pClient->GetSender<MessageSender>()->SendEnterGame();
			//enet_host_flush(ePeer->host);
			enet_peer_reset(ePeer);
			((EBotClient*)context)->OnDisconnected();
		}*/

#ifdef _DEBUG
		LogMsg("[OnConsoleMessage]: " + str); 
#endif
	});
	g_callbackManager->SetCallback("OnDialogRequest", VariantFuncs::OnDialogRequest);
	g_callbackManager->SetCallback("OnSuperMainStartAcceptLogonHrdxs47254722215a", [](CALLBACK_ARGS) 
	{ 
		auto pClient = ((EBotClient*)context);
		auto ePeer = pClient->GetPeer();
		auto pSender = pClient->GetSender<MessageSender>();

		pSender->SendEnterGame();
	});
	g_callbackManager->SetCallback("OnFailedToEnterWorld", [](CALLBACK_ARGS) { std::this_thread::sleep_for(1s); });
	g_callbackManager->SetCallback("OnSendToServer", VariantFuncs::OnSendToServer);
	g_callbackManager->SetCallback("OnRequestWorldSelectMenu", VariantFuncs::OnRequestWorldSelectMenu);
}

void MessageHandler::ProcessConnect(EBotClient* pClient)
{
	pClient->SetSender(new MessageSender(pClient));
}

void MessageHandler::ProcessDisconnect(EBotClient* pClient)
{
	if (auto pSender = pClient->GetSender<MessageSender>())
		pSender->Kill();

	pClient->Connect(true);
}

void MessageHandler::ProcessPacket(EBotClient* pClient)
{
	auto pSender = pClient->GetSender<MessageSender>();
	if (!pSender)
		return;

	auto ev = pClient->GetENetEvent();
	if (!ev.packet)
	{
#ifdef _DEBUG
		LogMsg("NULL ev.packet!");
#endif
		return;
	}

	if (ev.packet->dataLength < 4)
	{
#ifdef _DEBUG
		LogMsg(FormatStr("Packet data length was %d!", ev.packet->dataLength));
#endif
		return;
	}

	int messageType = *(int*)ev.packet->data;

	switch (messageType)
	{
	case NET_MESSAGE_TYPE_HELLO:
	{
		//LogMsg("GOT HELLO!");
		auto& det = pClient->m_data.detail;

		det.SetRequestedName(utils::gen_random(8));
		det.SetUUID((uint8_t*)det.rid, utils::gen_random(32));
		det.RandomizeMAC();
		det.SetCountry("id");
		det.SetUUID((uint8_t*)det.sid, utils::gen_random(32));
		det.meta = utils::GetMetaFromGrowtopiaHTTP();
		det.player_age = rand() % 100;

		vector<enet_uint8> bytes;

		// Get the latest meta:
		pClient->httpTunnel.Receive(bytes);

		det.meta = string(bytes.begin(), bytes.end());
		size_t tk = det.meta.find("meta|");
		if (tk != string::npos)
		{
			size_t tk2 = det.meta.find("\n");
			
			if (tk2 != string::npos)
				det.meta = det.meta.substr(tk + 5, tk2);

			LogMsg("Meta: " + det.meta);
		}

		pSender->SendLogon();
		pClient->m_data.bits |= EBOT_BIT_LOGON;
		break;
	}
	case NET_MESSAGE_TYPE_TEXT:
	{
		auto text = utils::GetTextFromPacket((char*)ev.packet->data + 4, ev.packet->dataLength - 4);
#ifdef _DEBUG
		LogMsg("[TEXT]: " + string(text));
#endif
		break;
	}

	case NET_MESSAGE_TYPE_GAME_MSG:
	{
		auto text = utils::GetTextFromPacket((char*)ev.packet->data + 4, ev.packet->dataLength - 4);
#ifdef _DEBUG
		LogMsg("[GAME_MSG]: " + string(text));
#endif
		break;
	}

	case NET_MESSAGE_TYPE_GAME_RAW:
	{
		TankPacketStruct* ts = (TankPacketStruct*)(ev.packet->data + 4);

		if (ts->packetType == NET_PACKET_CALL_FUNCTION)
		{
			VariantList vList(ev.packet->data + 4);
			
			if (!g_callbackManager->Call(&vList, (void*)pClient))
			{
#ifdef _DEBUG
				LogMsg("Could not call '" + vList.GetFuncName() + "', variant callback non-existent.");
#endif
			}
		}
		else if (ts->packetType == NET_PACKET_PING_REQUEST)
		{
			
		}
		else if (ts->packetType == NET_PACKET_SEND_ITEM_DATABASE_DATA)
		{

		}
		break;
	}
	default:
		break;
	}
}