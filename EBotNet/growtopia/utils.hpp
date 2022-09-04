#pragma once
#include <cstring>
#include <map>
#include <vector>
#include <chrono>
#include <enet/enet.h>
#include <algorithm>
#include <mutex>
#include <random>
#include "TankProtocol.h"

static std::random_device seeder;
namespace utils 
{
	using namespace std;

	const vector<string> explode(const string&, const char);
	ENetPacket* ENetSend(ENetPeer*, void*, size_t, int flags = 1);
	ENetPacket* ENetSend(ENetPeer*, ENetPacket*);
	ENetPacket* SendPacket(ENetPeer*, int, const string&); // if want to broadcast, host shouldn't be NULL
	ENetPacket* SendPacketRaw(ENetPeer*, TankPacketStruct*, int flags = 1);

	std::string GetMetaFromGrowtopiaHTTP();

    const inline bool isInside(int circle_x, int circle_y, 
                   int rad, int x, int y) 
    { 
    // Compare radius of circle with distance  
    // of its center from given point 
        if ((x - circle_x) * (x - circle_x) + 
            (y - circle_y) * (y - circle_y) <= rad * rad) 
            return true; 
        else
            return false; 
    }

	const inline uint64_t MTE(int min, int max)
	{
		static thread_local std::mt19937_64 mte_gen(seeder());

		std::uniform_int_distribution<int64_t> dis(min, max);
		return dis(mte_gen);
	}

	inline void RemoveNonNumeric(std::string& text)
	{
		text.erase(remove_if(text.begin(), text.end(), (int(*)(int))isalpha), text.end());
	}

	inline int64_t GetTimeMs()
	{
		return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
	}

	inline const string gen_random(const int len) {
		string tmp_s;
		static constexpr char alphanum[] =
			"0123456789"
			"ABCDEF";

		tmp_s.reserve(len);

		for (int i = 0; i < len; ++i)
			tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

		tmp_s[0] = '0';
		return tmp_s;
	}

	inline const string gen_random_digits(const int len) {
		string tmp_s;
		static constexpr char alphanum[] =
			"0123456789";

		tmp_s.reserve(len);

		for (int i = 0; i < len; ++i)
			tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

		tmp_s[0] = '0';
		return tmp_s;
	}

	inline const string gen_random2(const int len)
	{
		string tmp_s;
		static constexpr char alphanum[] =
			"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

		tmp_s.reserve(len);

		for (int i = 0; i < len; ++i)
			tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

		return tmp_s;
	}

	inline uint32_t RTHash(uint8_t* str, int len)
	{
		if (!str) return 0;

		uint8_t* n = (unsigned char*)str;
		uint32_t acc = 0x55555555;

		if (len == 0)
		{
			while (*n)
				acc = (acc >> 27) + (acc << 5) + *n++;
		}
		else
		{
			for (int i = 0; i < len; i++)
			{
				acc = (acc >> 27) + (acc << 5) + *n++;
			}
		}
		return acc;
	}

	vector<string_view> StringTokenize(const string_view& strv, char seperator = '|');

	class TextScanner
	{
	public:
		TextScanner(string_view strv) { Read(strv); }
		TextScanner() {} // empty constructor, used when just want to write to "scanner".
		
		string_view GetByIndex(size_t i, size_t j);
		int GetEntryCount(const char*);
		void Read(const string_view& strv);

		inline vector<string_view> GetRows(int i)
		{
			return i >= table.size() ? vector<string_view>() : table[i];
		}

		string_view Scan(const char*, int valuePos = 1, int offset = 0) noexcept;

	private:
		vector<vector<string_view>> table;
	};

	inline string_view GetTextFromPacket(char* data, uint32_t dataLen)
	{
		if (dataLen < 5 || !data)
			return string_view();

		data[dataLen - 1] = '\0';
		return string_view(data, dataLen);
	}

}