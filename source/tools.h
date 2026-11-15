#pragma once

#include <GarrysMod/FactoryLoader.hpp>
#include <GarrysMod/Lua/LuaInterface.h>
#include <GarrysMod/Lua/LuaShared.h>

namespace {
	static GarrysMod::Lua::ILuaShared* lua_shared = nullptr;
}

namespace clsp {
	namespace tools {
		inline GarrysMod::Lua::ILuaShared* GetShared()
		{
			if (lua_shared != nullptr)
				return lua_shared;

			static SourceSDK::FactoryLoader loader("lua_shared");
			lua_shared = loader.GetInterface<GarrysMod::Lua::ILuaShared>(GMOD_LUASHARED_INTERFACE);

			return lua_shared;
		}

		inline GarrysMod::Lua::ILuaInterface* GetState(unsigned char state)
		{
			return lua_shared != nullptr ? lua_shared->GetLuaInterface(state) : nullptr;
		}
	} // namespace tools
} // namespace clsp