// i dont use this on growmmfi or anything its really simple and doesnt have any latest fixes or subserver system with synchronization, its literally just using multiple peers/hosts to connect.
#include "utils.hpp"
#include <sstream>
#include <iomanip>
#include <string>
#include <chrono>
#include "../httplib.h"

const std::vector<std::string> utils::explode(const std::string& s, const char c)
{
	std::string buff{ "" };
	std::vector<std::string> v;

	for (auto n : s)
	{
		if (n != c) buff += n; else
			if (n == c && buff != "") { v.push_back(buff); buff = ""; }
	}

	if (buff != "") 
		v.push_back(buff);

	return v;
}

std::vector<std::string_view> utils::StringTokenize(const std::string_view& strv, char seperator)
{
	std::vector<std::string_view> output;
	size_t first = 0;

	while (first < strv.size())
	{
		const size_t second = strv.find_first_of(seperator, first);

		if (first != second)
			output.emplace_back(strv.substr(first, second - first));

		if (second == std::string_view::npos)
			break;

		first = second + 1;
	}

	return output;
}

ENetPacket* utils::ENetSend(ENetPeer* peer, void* data, size_t dataLen, int flags)
{
	return ENetSend(peer, enet_packet_create(data, dataLen, flags));
}

ENetPacket* utils::ENetSend(ENetPeer* peer, ENetPacket* packet)
{
	if (peer)
	{
		if (enet_peer_send(peer, 0, packet) != 0)
		{
			enet_packet_destroy(packet);
			return NULL;
		}
	}

	return packet;
}

ENetPacket* utils::SendPacket(ENetPeer* peer, int messageType, const std::string& text)
{
	if (!peer)
		return NULL;

	if (peer->state != ENET_PEER_STATE_CONNECTED)
		return NULL;

	size_t textLen = text.length();
	ENetPacket* p = enet_packet_create(NULL, 5 + textLen, 1);

	*(int*)p->data = messageType;
	memcpy(p->data + 4, text.c_str(), textLen);

	return utils::ENetSend(peer, p);
}

ENetPacket* utils::SendPacketRaw(ENetPeer* peer, TankPacketStruct* raw, int flags)
{
	char packet[61];
	*(int*)packet = 4;
	memcpy(packet + 4, raw, 56);

	return utils::ENetSend(peer, packet, 61, flags);
}

std::string utils::GetMetaFromGrowtopiaHTTP()
{
	std::string meta = "EBOTNET_FAILURE!";

	httplib::Client cli("https://www.growtopia1.com");
	cli.enable_server_certificate_verification(false);
	httplib::Headers headers = {
	{ "User-Agent", "UbiServices_SDK_2019.Release.27_PC64_unicode_static" },
	{ "Accept", "/" }
	};
	auto res = cli.Post("/growtopia/server_data.php", headers, "version=3.98&protocol=164&platform=0", "application/x-www-form-urlencoded");
	if (res) {
		auto body = res.value().body;

		utils::TextScanner ts(body);
		meta = ts.Scan("meta");
	}

	return meta;
}

std::string_view utils::TextScanner::GetByIndex(size_t i, size_t j)
{
	if (i >= table.size())
		return std::string_view();

	const auto& rows = table[i];

	if (j >= rows.size())
		return std::string_view();

	return rows[j];
}

int utils::TextScanner::GetEntryCount(const char* name)
{
	int cur = 0;
	for (const auto& columns : table)
	{
		for (int i = 0; i < columns.size(); i++)
		{
			if (columns[i] == name)
				cur++;
		}
	}

	return cur;
}

void utils::TextScanner::Read(const std::string_view& strv)
{
	const auto lines = StringTokenize(strv, '\n');

	for (const auto& line : lines)
	{
		const auto rows = StringTokenize(line);

		if (rows.size() < 1)
			continue; // requires at least a name

		table.push_back(rows);
	}
}

std::string_view utils::TextScanner::Scan(const char* name, int valuePos, int offset) noexcept
{
	// Take an "offset"'s value by name 

	int cur = 0;
	for (const auto& columns : table)
	{
		const size_t colSize = columns.size();
		for (int i = 0; i < colSize; i++)
		{
			if (columns[i] == name)
			{
				if (cur != offset)
					cur++;
				else if (colSize > (i + valuePos))
					return columns[i + valuePos];
				else
					break;
			}
		}

	}

	return std::string_view();
}