#pragma once
#include "CallbackManager.h"
#include "../EBotNet.h"
#include <mutex>

#define LOG_ACCS
namespace VariantFuncs 
{
	mutex m;
	// define callbacks here, will call with respected delay:
	inline void OnDialogRequest(CALLBACK_ARGS)
	{
		auto pClient = (EBotClient*)context;
		auto pSender = pClient->GetSender<MessageSender>();

		//pSender->SendDialogReturn({ "collectionQuests", "info_%d" }, 0, ++pClient->m_data.itemCycle);

		auto diagStr = vList->GetFuncArg(1).strVal;

		if (diagStr.find("Get a GrowID") != string::npos)
		{
			auto logonName = utils::gen_random2(8);
			auto xlog = utils::gen_random(9);

			pClient->SendPacket(2, "action|dialog_return\ndialog_name|growid_apply\nlogon|" + logonName +
				"\npassword|" + xlog + "\npassword_verify|" + xlog + "\nemail|wuvaobk@gmail.com\nbuttonClicked|\n");

#ifdef LOG_ACCS
				ofstream outfile;

				m.lock();
				outfile.open("accs.txt", std::ios_base::app); // append instead of overwrite
				outfile << logonName << "\n";

				outfile.close();
				m.unlock();
#endif
			enet_peer_disconnect_later(pClient->GetPeer(), 0);
		}

#ifdef _DEBUG
		LogMsg("[OnDialogRequest]: " + diagStr);
#endif
	}

	inline void OnSendToServer(CALLBACK_ARGS)
	{
		auto pClient = (EBotClient*)context;

		std::string mainParam = vList->GetFuncArg(4);
		LogMsg(mainParam);

		size_t tk = mainParam.find('|');

		if (tk == std::string::npos)
			return;

		std::string after = mainParam.substr(tk + 1);

		size_t tk2 = after.find('|');

		if (tk2 == std::string::npos)
			return;

		pClient->OnServerMove(mainParam.substr(0, tk), after.substr(0, tk2), vList->GetFuncArg(1).intVal,
			vList->GetFuncArg(3).intVal, vList->GetFuncArg(2).intVal, vList->GetFuncArg(5).intVal, after.substr(tk2 + 1));
	}

	inline void OnRequestWorldSelectMenu(CALLBACK_ARGS)
	{
		auto pClient = (EBotClient*)context;
		auto pSender = pClient->GetSender<MessageSender>();

		//enet_peer_disconnect_later(pClient->GetPeer(), 0);
	}
}