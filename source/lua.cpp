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
	static void InitLuaClasses(GarrysMod::Lua::ILuaInterface* LUA)
	{
		InitLuaClassesHook.GetTrampoline<InitLuaClasses_t>()(LUA);
		g_ServerPlugin.LuaInit(LUA);
	}

	static Detouring::Hook CLuaInterface_ShutdownHook;
	static void CLuaInterface_Shutdown(GarrysMod::Lua::ILuaInterface* LUA)
	{
		CLuaInterface_ShutdownHook.GetTrampoline<CLuaInterface_Shutdown_t>()(LUA);

		if (g_ServerPlugin.m_pLua == LUA)
			g_ServerPlugin.LuaShutdown(LUA);
	}
}

void clsp::lua::Initialize()
{
	SourceSDK::ModuleLoader server("server");
	SourceSDK::ModuleLoader lua_shared("lua_shared");
	SymbolFinder sym;

	const Symbol _InitLuaClasses = GetPlatformSymbol(InitLuaClasses_Sym);
	const Symbol _CLuaInterface_Shutdown = GetPlatformSymbol(CLuaInterface_Shutdown_Sym);

	InitLuaClassesHook.Create(sym.Resolve(server.GetModule(), _InitLuaClasses.name.c_str(), _InitLuaClasses.length), reinterpret_cast<void*>(InitLuaClasses));
	CLuaInterface_ShutdownHook.Create(sym.Resolve(lua_shared.GetModule(), _CLuaInterface_Shutdown.name.c_str(), _CLuaInterface_Shutdown.length), reinterpret_cast<void*>(CLuaInterface_Shutdown));

	if (!InitLuaClassesHook.Enable())
		Warning("#datapack: InitLuaClasses hook failed to enable!\n");
	if (!CLuaInterface_ShutdownHook.Enable())
		Warning("#datapack: CLuaInterface::Shutdown hook failed to enable!\n");
}