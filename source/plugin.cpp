#include <tier1.h>

#include "plugin.h"
#include "datapack.h"

// required for server plugins
static CServerPlugin g_ServerPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CServerPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_ServerPlugin);

CServerPlugin::CServerPlugin()
{
}

CServerPlugin::~CServerPlugin()
{
}

bool CServerPlugin::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	g_DataPack.Initialize();

	ConnectTier1Libraries(&interfaceFactory, 1);

	return true;
}

void CServerPlugin::Unload()
{
}

void CServerPlugin::Pause()
{
}

void CServerPlugin::UnPause()
{
}

const char* CServerPlugin::GetPluginDescription()
{
	return "none";
}

void CServerPlugin::LevelInit(const char *pMapName)
{
}

void CServerPlugin::ServerActivate(edict_t* pEdictList, int edictCount, int clientMax)
{
	g_DataPack.StartProcessing();
}

void CServerPlugin::GameFrame(bool simulating)
{
	if (g_DataPack.HasCompleted())
		g_DataPack.CollectResults();

	if (!g_DataPack.HasCompleted() && !g_DataPack.IsProcessing() && g_DataPack.HasPendingFiles())
		g_DataPack.StartProcessing();
}

void CServerPlugin::LevelShutdown(void)
{
}

void CServerPlugin::ClientActive(edict_t* pEntity)
{
}

void CServerPlugin::ClientDisconnect(edict_t* pEntity)
{
}

void CServerPlugin::ClientPutInServer(edict_t* pEntity, char const* playername)
{
}

void CServerPlugin::SetCommandClient(int index)
{
}

void CServerPlugin::ClientSettingsChanged(edict_t* pEdict)
{
}

PLUGIN_RESULT CServerPlugin::ClientConnect(bool* bAllowConnect, edict_t* pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen)
{
	return PLUGIN_CONTINUE;
}

PLUGIN_RESULT CServerPlugin::ClientCommand(edict_t* pEntity, const CCommand& args)
{
	return PLUGIN_CONTINUE;
}

PLUGIN_RESULT CServerPlugin::NetworkIDValidated(const char* pszUserName, const char* pszNetworkID)
{
	return PLUGIN_CONTINUE;
}

void CServerPlugin::OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t* pPlayerEntity, EQueryCvarValueStatus eStatus, const char* pCvarName, const char* pCvarValue)
{
}

void CServerPlugin::OnEdictAllocated(edict_t* edict)
{
}

void CServerPlugin::OnEdictFreed(const edict_t* edict)
{
}

bool CServerPlugin::LuaInit()
{
	return true;
}

void CServerPlugin::LuaShutdown()
{
}
