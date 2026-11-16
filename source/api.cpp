#include <vector>

#include <GarrysMod/Lua/Types.h>
#include <GarrysMod/Lua/LuaObject.h>

#include "api.h"
#include "glob.h"
#include "version.h"

namespace {
	std::vector<Rule> rules;

	LUA_FUNCTION(clsp_add)
	{
		LUA->CheckType(1, GarrysMod::Lua::Type::STRING);
		LUA->CheckType(2, GarrysMod::Lua::Type::STRING);
		LUA->CheckType(3, GarrysMod::Lua::Type::FUNCTION);

		const char* identifier = LUA->GetString(1);
		const char* route = LUA->GetString(2);

		// Check for duplicate identifiers.
		for (const auto& rule : rules)
		{
			if (rule.identifier == identifier)
			{
				LUA->ThrowError("Rule identifier already exists");
				return 0;
			}
		}

		// Create a Lua reference to the callback.
		LUA->Push(3);
		int reference = LUA->ReferenceCreate();

		Rule rule;
		rule.identifier = identifier;
		rule.route = route;
		rule.reference = reference;

		rules.push_back(std::move(rule));

		return 0;
	}
}

//

void clsp::api::Initialize(GarrysMod::Lua::ILuaBase* LUA)
{
	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);

	LUA->CreateTable();
		LUA->PushCFunction(clsp_add);
		LUA->SetField(-2, "add");

		LUA->PushString(VERSION);
		LUA->SetField(-2, "VERSION");
	LUA->SetField(-2, "clsp");

	LUA->Pop();
}

void clsp::api::Shutdown(GarrysMod::Lua::ILuaBase* LUA)
{
	for (const auto& rule : rules)
		if (rule.reference != -1)
			LUA->ReferenceFree(rule.reference);

	rules.clear();
}

void clsp::api::internal::Run(GarrysMod::Lua::ILuaBase* LUA, const std::string& path, std::string& contents)
{
	if (rules.empty())
		return;

	for (const auto& rule : rules)
	{
		if (rule.reference == -1)
			continue;

		if (!clsp::glob::Match(rule.route, path))
			continue;

		LUA->ReferencePush(rule.reference);
		LUA->PushString(path.c_str());
		LUA->PushString(contents.c_str());
		int rets = LUA->PCall(2, 1, 0);

		if (rets != 0)
		{
			Msg("#clsp: callback error: %s\n", LUA->GetString(-1));
			LUA->Pop();
			continue;
		}

		if (LUA->IsType(-1, GarrysMod::Lua::Type::STRING))
			contents = LUA->GetString(-1);

		LUA->Pop();
	}
}