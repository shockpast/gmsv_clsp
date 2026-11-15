#pragma once

#include <GarrysMod/Symbol.hpp>

class GModDataPack;
class bf_read;
namespace GarrysMod {
  namespace Lua {
    class File;
    class ILuaInterface;
  }
}
class File; /* GarrysMod::Lua::File */

/* datapack.cpp/datapack.h */
using DataPack_SendFileToClient_t = void(*)(GModDataPack*, int, int);
using DataPack_AddOrUpdateFile_t = void(*)(GModDataPack*, GarrysMod::Lua::File*, bool);
using DataPack_OnFilesRequested_t = void(*)(GModDataPack*, int, bf_read*, int);

const Symbol DataPack_SendFileToClient_Sym = Symbol::FromName("_ZN12GModDataPack16SendFileToClientEii");
const Symbol DataPack_AddOrUpdateFile_Sym = Symbol::FromName("_ZN12GModDataPack15AddOrUpdateFileEP7LuaFileb");
const Symbol DataPack_OnFilesRequested_Sym = Symbol::FromName("_ZN12GModDataPack16OnFilesRequestedEiP7bf_readi");

/* lua.cpp/lua.h */
using InitLuaClasses_t = void(*)(GarrysMod::Lua::ILuaInterface*);
using CLuaInterface_Shutdown_t = void(*)(GarrysMod::Lua::ILuaInterface*);

const Symbol InitLuaClasses_Sym = Symbol::FromName("_Z14InitLuaClassesP13ILuaInterface");
const Symbol CLuaInterface_Shutdown_Sym = Symbol::FromName("_ZN13CLuaInterface8ShutdownEv");