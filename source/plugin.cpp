#include "api.h"
#include "lua.h"
#include <GarrysMod/FactoryLoader.hpp>
#include <GarrysMod/Lua/LuaInterface.h>
#define AutoBuffer _AutoBuffer /* hacky hack */
#include <GarrysMod/Lua/LuaShared.h>
#undef AutoBuffer

#include <tier1.h>

#include "datapack.h"
#include "plugin.h"

// required for server plugins
CServerPlugin g_ServerPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CServerPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_ServerPlugin);

CServerPlugin::CServerPlugin()
{
}

CServerPlugin::~CServerPlugin()
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is loaded, load the interface we need from the engine
//---------------------------------------------------------------------------------
bool CServerPlugin::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	g_DataPack.Initialize();
	clsp::lua::Initialize();

	ConnectTier1Libraries(&interfaceFactory, 1);

	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//---------------------------------------------------------------------------------
void CServerPlugin::Unload()
{
	g_DataPack.Shutdown();
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is paused (i.e should stop running but isn't unloaded)
//---------------------------------------------------------------------------------
void CServerPlugin::Pause()
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unpaused (i.e should start executing again)
//---------------------------------------------------------------------------------
void CServerPlugin::UnPause()
{
}

//---------------------------------------------------------------------------------
// Purpose: the name of this plugin, returned in "plugin_print" command
//---------------------------------------------------------------------------------
const char* CServerPlugin::GetPluginDescription()
{
	return "none";
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CServerPlugin::LevelInit(const char *pMapName)
{
}

//---------------------------------------------------------------------------------
// Purpose: called on level start, when the server is ready to accept client connections
//		edictCount is the number of entities in the level, clientMax is the max client count
//---------------------------------------------------------------------------------
void CServerPlugin::ServerActivate(edict_t* pEdictList, int edictCount, int clientMax)
{
	g_DataPack.ProcessQueue();
}

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------
void CServerPlugin::GameFrame(bool simulating)
{
	if (g_DataPack.HasCompleted())
		g_DataPack.CollectResults();

	if (!g_DataPack.HasCompleted() && !g_DataPack.IsProcessing() && g_DataPack.HasPendingFiles())
		g_DataPack.ProcessQueue();
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void CServerPlugin::LevelShutdown(void)
{
	g_DataPack.ResetLevel();
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------
void CServerPlugin::ClientActive(edict_t* pEntity)
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client leaves a server (or is timed out)
//---------------------------------------------------------------------------------
void CServerPlugin::ClientDisconnect(edict_t* pEntity)
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns?
//---------------------------------------------------------------------------------
void CServerPlugin::ClientPutInServer(edict_t* pEntity, char const* playername)
{
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CServerPlugin::SetCommandClient(int index)
{
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CServerPlugin::ClientSettingsChanged(edict_t* pEdict)
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client joins a server
//---------------------------------------------------------------------------------
PLUGIN_RESULT CServerPlugin::ClientConnect(bool* bAllowConnect, edict_t* pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen)
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client types in a command (only a subset of commands however, not CON_COMMAND's)
//---------------------------------------------------------------------------------
PLUGIN_RESULT CServerPlugin::ClientCommand(edict_t* pEntity, const CCommand& args)
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client is authenticated
//---------------------------------------------------------------------------------
PLUGIN_RESULT CServerPlugin::NetworkIDValidated(const char* pszUserName, const char* pszNetworkID)
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a cvar value query is finished
//---------------------------------------------------------------------------------
void CServerPlugin::OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t* pPlayerEntity, EQueryCvarValueStatus eStatus, const char* pCvarName, const char* pCvarValue)
{
}

void CServerPlugin::OnEdictAllocated(edict_t* edict)
{
}

void CServerPlugin::OnEdictFreed(const edict_t* edict)
{
}

//---------------------------------------------------------------------------------
// Purpose: called when lua is fully initialized and ready to run
//---------------------------------------------------------------------------------
bool CServerPlugin::LuaInit(GarrysMod::Lua::ILuaInterface* LUA)
{
	this->m_pLua = LUA;

	clsp::api::Initialize(this->m_pLua);
	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called when server is shutting down and lua is shutting down too
//---------------------------------------------------------------------------------
void CServerPlugin::LuaShutdown(GarrysMod::Lua::ILuaInterface* LUA)
{
	this->m_pLua = nullptr;

	clsp::api::Shutdown(this->m_pLua);
}