#pragma once
#include <string_view>
#include <string>
#include <cstring>

#pragma warning (disable : 4477)
#pragma warning (disable : 6328)
#pragma warning (disable : 4996)

enum {
    NET_MESSAGE_TYPE_NONE,
    NET_MESSAGE_TYPE_HELLO,
    NET_MESSAGE_TYPE_TEXT,
    NET_MESSAGE_TYPE_GAME_MSG,
    NET_MESSAGE_TYPE_GAME_RAW,
    NET_MESSAGE_TYPE_TRACK
};

enum ePlatformID
{
    PLATFORM_ID_UNKNOWN = -1,
    PLATFORM_ID_WINDOWS,
    PLATFORM_ID_IOS, //iPhone/iPad etc
    PLATFORM_ID_OSX,
    PLATFORM_ID_LINUX,
    PLATFORM_ID_ANDROID,
    PLATFORM_ID_WINDOWS_MOBILE, //yeah, right.  Doesn't look like we'll be porting here anytime soon.
    PLATFORM_ID_WEBOS,
    PLATFORM_ID_BBX, //RIM Playbook
    PLATFORM_ID_FLASH,
    PLATFORM_ID_HTML5, //javascript output via emscripten for web

    //new platforms will be added above here.  Don't count on PLATFORM_ID_COUNT not changing!
    PLATFORM_ID_COUNT
};

enum {
    NET_PACKET_STATE,
    NET_PACKET_CALL_FUNCTION,
    NET_PACKET_UPDATE_STATUS,
    NET_PACKET_TILE_CHANGE_REQUEST,
    NET_PACKET_SEND_MAP_DATA,
    NET_PACKET_SEND_TILE_UPDATE_DATA,
    NET_PACKET_SEND_TILE_UPDATE_DATA_MULTIPLE,
    NET_PACKET_TILE_ACTIVATE_REQUEST,
    NET_PACKET_TILE_APPLY_DAMAGE,
    NET_PACKET_SEND_INVENTORY_STATE,
    NET_PACKET_ITEM_ACTIVATE_REQUEST,
    NET_PACKET_ITEM_ACTIVATE_OBJECT_REQUEST,
    NET_PACKET_SEND_TILE_TREE_STATE,
    NET_PACKET_MODIFY_ITEM_INVENTORY,
    NET_PACKET_ITEM_CHANGE_OBJECT,
    NET_PACKET_SEND_LOCK,
    NET_PACKET_SEND_ITEM_DATABASE_DATA,
    NET_PACKET_SEND_PARTICLE_EFFECT,
    NET_PACKET_SET_ICON_STATE,
    NET_PACKET_ITEM_EFFECT,
    NET_PACKET_SET_CHARACTER_STATE,
    NET_PACKET_PING_REPLY,
    NET_PACKET_PING_REQUEST,
    NET_PACKET_GOT_PUNCHED,
    NET_PACKET_APP_CHECK_RESPONSE,
    NET_PACKET_APP_INTEGRITY_FAIL,
    NET_PACKET_DISCONNECT,
    NET_PACKET_BATTLE_JOIN,
    NET_PACKET_BATTLE_EVEN,
    NET_PACKET_USE_DOOR,
    NET_PACKET_SEND_PARENTAL,
    NET_PACKET_GONE_FISHIN,
    NET_PACKET_STEAM,
    NET_PACKET_PET_BATTLE,
    NET_PACKET_NPC,
    NET_PACKET_SPECIAL,
    NET_PACKET_SEND_PARTICLE_EFFECT_V2,
    NET_PACKET_ACTIVE_ARROW_TO_ITEM,
    NET_PACKET_SELECT_TILE_INDEX
};

struct TankPacketStruct 
{
#pragma pack (push,1)
    uint8_t packetType = 0;
    uint8_t padding1 = 0, padding2 = 0, padding3 = 0;
    int NetID = 0;
    int secondaryNetID = 0;
    int characterState = 0;
    float padding4 = 0;
    int value = 0;
    float x = 0, y = 0;
    int XSpeed = 0, YSpeed = 0;
    int padding5 = 0;
    int punchX = 0, punchY = 0;
    uint32_t extDataSize = 0;
#pragma pack (pop)
};

struct TankInfo {
    char requestedName[16] = { 0 };
    char tankIDName[24] = { 0 };
    char tankIDPass[24] = { 0 };

    char rid[16] = { 0 },
        gid[16] = { 0 },
        sid[16] = { 0 },
        aid[16] = { 0 },
        vid[16] = { 0 },
        country[3] = "us";

    uint8_t mac[6] = { 0 };

    inline bool SetRequestedName(std::string_view strv)
    {
        if (strv.length() >= sizeof(requestedName))
            return false;

        memcpy(requestedName, strv.data(), strv.length());
        return true;
    }

    inline bool SetTankIDName(std::string_view strv)
    {
        if (strv.length() >= sizeof(tankIDName)) // leaves space for null char.
            return false;

        memcpy(tankIDName, strv.data(), strv.length());
        return true;
    }

    inline bool SetTankIDPass(std::string_view strv)
    {
        if (strv.length() >= sizeof(tankIDPass))
            return false;

        memcpy(tankIDPass, strv.data(), strv.length());
        return true;
    }

    void RandomizeMAC();
    inline const std::string GetMAC()
    {
        char str[18] = { 0 };

        if (sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) == 17)
        {
            return std::string(str);
        }

        return "";
    }

    inline bool SetCountry(std::string_view strv)
    {
        if (strv.length() >= 3)
            return false;

        memcpy(country, strv.data(), 2);
        return true;
    }
    

    inline const bool SetMAC(std::string_view strv)
    {
        if (strv.length() != ((sizeof(mac) * 2) + 5)) // format: xx:xx:xx:xx:xx:xx
            return false;

        return sscanf(strv.data(), "%02x:%02x:%02x:%02x:%02x:%02x",
            &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == sizeof(mac);
    }

    inline const bool SetUUID(uint8_t* p, std::string_view strv) 
    {
        if (strv.length() != sizeof(rid) * 2) // rid (16 bytes) should be same as all other UUID's.
            return false; // however in string form it's supposed to be 32 bytes (due to 1 taking up a single char each time)

        return sscanf(strv.data(), "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", //"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
            &p[0], &p[1], &p[2], &p[3], &p[4], &p[5], &p[6], &p[7],
            &p[8], &p[9], &p[10], &p[11], &p[12], &p[13], &p[14], &p[15]) == sizeof(rid);
    }


    inline const std::string GetAnyUUID(uint8_t* p)
    {
        static const char hexchars[] = "0123456789ABCDEF";
        std::string result;
        result.reserve(32);

        for (int i = 0; i < 16; i++)
        {
            int b = (int)p[i];
            char hex[3];

            hex[0] = hexchars[b >> 4];
            hex[1] = hexchars[b & 0xF];
            hex[2] = 0;

            result.append(hex);
        }

        return result;
    }

    inline const bool HasGrowID() { return tankIDName[0] != '\0'; }

    float game_version = 3.98f;
    uint32_t userID = 0;
    int platformID = 0, token = 0, lmode = 0;
    const int protocol = 164;
    int player_age = 22;
    std::string meta = "", doorID = "", uuid_token;

    const std::string MakeLogon();
};