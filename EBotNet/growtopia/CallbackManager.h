#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include "VariantList.h"

#define CALLBACK_ARGS Variant::VariantList* vList, void* context = NULL

class CallbackManager
{
public:
	inline void SetCallback(const std::string& funcName, const std::function<void(Variant::VariantList*, void*)>& func)
	{
		m_funcMap[funcName] = func;
	}

	inline bool Call(CALLBACK_ARGS)
	{
		const auto funcName = vList->GetFuncName();

		auto it = m_funcMap.find(funcName);
		if (it == m_funcMap.end())
			return false;

		it->second(vList, context);
		return true;
	}

	inline size_t UnsetCallback(const std::string& funcName)
	{
		return m_funcMap.erase(funcName);
	}


private:
	std::unordered_map<std::string, std::function<void(Variant::VariantList*, void*)>> m_funcMap;
};