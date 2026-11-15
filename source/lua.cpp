#include <GarrysMod/Lua/LuaInterface.h>
#include <GarrysMod/ModuleLoader.hpp>

#include <detouring/hook.hpp>
#include <scanning/symbolfinder.hpp>

#include <dbg.h>

#include "lua.h"
#include "plugin.h"
#include "symbols.h"

namespace {
	static Detouring::Hook InitLuaClassesHook;
	void InitLuaClasses(GarrysMod::Lua::ILuaInterface* LUA)
	{
		g_ServerPlugin.LuaInit(LUA);
		InitLuaClassesHook.GetTrampoline<InitLuaClasses_t>()(LUA);
	}

	static Detouring::Hook CLuaInterface_ShutdownHook;
	void CLuaInterface_Shutdown(GarrysMod::Lua::ILuaInterface* LUA)
	{
		g_ServerPlugin.LuaShutdown(LUA);
		CLuaInterface_ShutdownHook.GetTrampoline<CLuaInterface_Shutdown_t>()(LUA);
	}
}

void clsp::lua::Initialize()
{
	SourceSDK::ModuleLoader server("server");
	SourceSDK::ModuleLoader lua_shared("lua_shared");
	SymbolFinder sym;

	InitLuaClassesHook.Create(sym.Resolve(server.GetModule(), InitLuaClasses_Sym.name.c_str()), reinterpret_cast<void*>(InitLuaClasses));
	CLuaInterface_ShutdownHook.Create(sym.Resolve(lua_shared.GetModule(), CLuaInterface_Shutdown_Sym.name.c_str()), reinterpret_cast<void*>(CLuaInterface_Shutdown));

	if (!InitLuaClassesHook.Enable())
		Warning("#datapack: InitLuaClasses hook failed to enable!\n");
	if (!CLuaInterface_ShutdownHook.Enable())
		Warning("#datapack: CLuaInterface::Shutdown hook failed to enable!\n");
}