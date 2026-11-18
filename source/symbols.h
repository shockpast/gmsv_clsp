#pragma once

#include <vector>

#include <GarrysMod/Symbol.hpp>
#include <Platform.hpp>

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
const std::vector<Symbol> DataPack_SendFileToClient_Sym = {
  Symbol::FromName("_ZN12GModDataPack16SendFileToClientEii"),
  Symbol::FromName("_ZN12GModDataPack16SendFileToClientEii"),
};
const std::vector<Symbol> DataPack_AddOrUpdateFile_Sym = {
  Symbol::FromName("_ZN12GModDataPack15AddOrUpdateFileEP7LuaFileb"),
  Symbol::FromName("_ZN12GModDataPack15AddOrUpdateFileEP7LuaFileb"),
};
const std::vector<Symbol> DataPack_OnFilesRequested_Sym = {
  Symbol::FromName("_ZN12GModDataPack16OnFilesRequestedEiP7bf_readi"),
  Symbol::FromSignature("\x55\x48\x63\xC6\x48\x8D\x04\x87"),
};

/* lua.cpp/lua.h */
using InitLuaClasses_t = void(*)(GarrysMod::Lua::ILuaInterface*);
using CLuaInterface_Shutdown_t = void(*)(GarrysMod::Lua::ILuaInterface*);

const std::vector<Symbol> InitLuaClasses_Sym = {
  Symbol::FromName("_Z14InitLuaClassesP13ILuaInterface"),
  Symbol::FromSignature("\x48\x8B\x05****\x48\x85\xC0\x74*\x8B\x50*\x85\xD2\x7E*\x55\x48\x89\xE5\x53\x31\xDB\x48\x83\xEC*\x66\x90\x48\x8B*\x48\x8B\x3C\xD8\xE8"),
};
const std::vector<Symbol> CLuaInterface_Shutdown_Sym = {
  Symbol::FromName("_ZN13CLuaInterface8ShutdownEv"),
  Symbol::FromSignature("\x55\x48\x89\xE5\x41\x55\x41\x54\x49\x89\xFC\x53\x4D\x8D\xAC\x24"),
};

/* multi-platform symbol resolver */
#ifdef SYSTEM_LINUX
  #if ARCHITECTURE_IS_X86
    #define SYMBOL_ID 0
  #else
    #define SYMBOL_ID 1
  #endif
#endif

inline const Symbol GetPlatformSymbol(std::vector<Symbol> symbols)
{
  return symbols[SYMBOL_ID];
}