#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/Lua/LuaShared.h>
#include <GarrysMod/ModuleLoader.hpp>
#include <GarrysMod/Symbol.hpp>
#include <dbg.h>
#include <detouring/hook.hpp>
#include <interface.h>
#include <scanning/symbolfinder.hpp>
#include <tier0/dbg.h>
#include <tier1/bitbuf.h>
#include <tier2/tier2.h>
#include <tier1.h>
#include <networkstringtabledefs.h>

#include "plugin.h"

// required for server plugins
static CServerPlugin g_ServerPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CServerPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_ServerPlugin);

// fwd-decl
class GModDataPack;
class LuaFindResult;

// constants
constexpr int MAX_LUA_FILES = 2 << 12;

// symbols
const Symbol CDataPack_SendFileToClient_Sym = Symbol::FromName("_ZN12GModDataPack16SendFileToClientEii");
const Symbol CDataPack_AddOrUpdateFile_Sym = Symbol::FromName("_ZN12GModDataPack15AddOrUpdateFileEP7LuaFileb");
const Symbol CDataPack_OnClientConnected_Sym = Symbol::FromName("_ZN12GModDataPack17OnClientConnectedEi");
const Symbol CDataPack_OnFilesRequested_Sym = Symbol::FromName("_ZN12GModDataPack16OnFilesRequestedEiP7bf_readi");

// helper classes
static SymbolFinder g_sym;

// interfaces shared between functions

// type-decl for hooks
using CDataPack_SendFileToClient_t = void(*)(GModDataPack*, int, int);
using CDataPack_AddOrUpdateFile_t = void(*)(GModDataPack*, void*, bool);
using CDataPack_OnClientConnected_t = GModDataPack*(*)(GModDataPack*, int);
using CDataPack_OnFilesRequested_t = void(*)(GModDataPack*, int, bf_read*, int);

abstract_class IGModDataPack
{
private: // i don't care about these functions
	virtual void* GetFromDatatable(const std::string&) = 0;
	virtual void* GetHashFromDatatable(const std::string&) = 0;
	virtual void* GetHashFromString(const char*, unsigned long) = 0;
	virtual void FindInDatatable(const std::string&, std::vector<LuaFindResult>&, bool) = 0;
	virtual void* FindFileInDatatable(const std::string&, bool, bool) = 0;
	virtual bool IsSingleplayer() = 0;
	virtual void v000000000000000000000000000oo0000000000000000fff000000000000000000000000000000000o0o() = 0;
	virtual bool IsValidDirectory(const std::string&) = 0;
};

class GModDataPack : public IGModDataPack
{
private: // i don't care about these functions
	void OnClientConnected(int userId);
	bool IsSinglePlayer();
	void OnFilesRequested(int userId, bf_read* message, int bits);

public:
	INetworkStringTable* m_pClientLuaFiles;
};

class CDataPack
{
public:
	CDataPack() = default;

	void Initialize()
	{
		m_pSingleton = this;

		SourceSDK::ModuleLoader server("server");

		m_hAddOrUpdateFileHook.Create(g_sym.Resolve(server.GetModule(), CDataPack_AddOrUpdateFile_Sym.name.c_str()), reinterpret_cast<void*>(AddOrUpdateFile));
		Msg("%i\n", m_hAddOrUpdateFileHook.Enable());
		m_hSendFileToClientHook.Create(g_sym.Resolve(server.GetModule(), CDataPack_SendFileToClient_Sym.name.c_str()), reinterpret_cast<void*>(SendFileToClient));
		Msg("%i\n", m_hSendFileToClientHook.Enable());
	}

private:
	void SendLuaFile(GarrysMod::Lua::File* file, int clientId, int fileId)
	{
		size_t bytes = 3 + file->compressed.m_iWritten + 4;
		std::vector<uint32_t> buffer(bytes / sizeof(uint32_t));

		bf_write message(buffer.data(), (int)bytes);
		message.WriteByte(/* GarrysMod::NetworkMessage::LuaFileDownload */4);
		message.WriteUBitLong(fileId, 16);
		message.WriteBytes(file->compressed.m_pData, file->compressed.m_iWritten);

		if (message.IsOverflowed())
		{
			Warning("Failed to send a .lua file to client, message is overflowed!\n");
			return;
		}
	}

public:
	// drop-in replacement
	static void SendFileToClient(GModDataPack* datapack /* ecx */, int clientId, int fileId)
	{
		if (datapack == nullptr )
		{
			Warning("GModDataPack is missing.\n");
			return;
		}

		INetworkStringTable* clientLuaFiles = datapack->m_pClientLuaFiles;

		if (clientLuaFiles == nullptr)
		{
			Warning("client_lua_files stringtable is missing.\n");
			return;
		}

		CDataPack* self = m_pSingleton;

		if (self == nullptr)
		{
			Error("DataPack singleton doesn't exist at the moment of function call.\n");
			return;
		}

		// * fileId == 0          - reserved for search paths
		// * fileId  < 0          - simply don't exist
		// * fileId  > numstrings - more than exists in client_lua_files stringtable
		if (fileId <= 0 || fileId > clientLuaFiles->GetNumStrings())
		{
			Warning("Client tried to access file that is either out-of-bounds or reserved.\n");
			return;
		}

		GarrysMod::Lua::File* file = self->m_pFiles[fileId];

		if (file == nullptr)
		{
			Warning("Client tried to access file that is not in cache.\n");
			return;
		}

		self->SendLuaFile(file, clientId, fileId);
	}

	// drop-in replacement
	static void AddOrUpdateFile(GModDataPack* datapack /* ecx */, GarrysMod::Lua::File* file, bool recompress)
	{
		CDataPack* self = m_pSingleton;

		if (self == nullptr)
		{
			Error("DataPack singleton doesn't exist at the moment of function call.\n");
			return;
		}

		Msg("Adding %s in CDataPack\n", file->name.c_str());
	}

private:
	GarrysMod::Lua::File* m_pFiles[MAX_LUA_FILES]{};

private:
	Detouring::Hook m_hSendFileToClientHook;
	Detouring::Hook m_hAddOrUpdateFileHook;

	inline static CDataPack* m_pSingleton;
};

static CDataPack g_dataPack;

CServerPlugin::CServerPlugin()
{
	m_iClientCommandIndex = 0;
}

CServerPlugin::~CServerPlugin()
{
}

bool CServerPlugin::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	ConnectTier1Libraries(&interfaceFactory, 1);
	g_dataPack.Initialize();

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
}

void CServerPlugin::GameFrame(bool simulating)
{
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
