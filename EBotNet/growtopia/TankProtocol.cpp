#include "TankProtocol.h"
#include "../EBotList.h"
#include "../MD5.h"
#include "utils.hpp"

void TankInfo::RandomizeMAC()
{
    for (int i = 0; i < sizeof(mac); i++)
        mac[i] = utils::MTE(0, 0xFF);
}

const std::string TankInfo::MakeLogon()
{
   //auto key = md5("jFGVTi30jIf3" + to_string(time(NULL)));
   //utils::RemoveNonNumeric(key);

   //meta = "growmmfi.de_" + key;

    std::string str;

    if (HasGrowID())
    {
        str += FormatStr(
            "tankIDName|%s\n"
            "tankIDPass|%s\n",
            tankIDName, tankIDPass);
    }

    str += FormatStr(
        "requestedName|%s\n"
        "f|1\n"
        "protocol|%d\n"
        "game_version|%.2f\n"
        "fz|%d\n"
        "lmode|%d\n"
        "cbits|0\n"
        "player_age|%d\n"
        "GDPR|1\n"
        "category|_0totalPlaytime|0hash2|%d\n"
        "meta|%s\n"
        "fhash|-716928004\n"
        "rid|%s\n"
        "platformID|%d,1,1\n"
        "deviceVersion|0\n"
        "country|%s\n"
        "hash|%d\n"
        "mac|%s\n",
        requestedName, protocol, game_version, 16137536, lmode, player_age, 19999999 + rand() % 65535, meta.c_str(),
        GetAnyUUID((uint8_t*)rid).c_str(), platformID, country, 39999999 + rand() % 65535, GetMAC().c_str());

    switch (platformID)
    {
    case PLATFORM_ID_WINDOWS:
        str += FormatStr(
            "wk|%s\n"
            "zf|%d\n",
            GetAnyUUID((uint8_t*)sid).c_str(), (-19999999 - rand() % 65535));
        
        break;

    case PLATFORM_ID_OSX:
    case PLATFORM_ID_IOS:
        str += FormatStr(
            "aid|%s\n"
            "vid|%s\n",
            aid, vid);

        break;

    case PLATFORM_ID_ANDROID:
        str += FormatStr("gid|%s\n", gid);
        break;

    default:
        break;
    }

    if (userID != 0)
    {
        str += FormatStr(
            "user|%d\n"
            "token|%d\n"
            "UUIDToken|%s\n",
            userID, token, uuid_token.c_str());
    }

#ifdef _DEBUG
    LogMsg(str);
#endif
    return str;
}
