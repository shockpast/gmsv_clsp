#pragma once

#include <GarrysMod/Lua/LuaInterface.h>
#include <string>

#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/Lua/LuaBase.h>
#include <GarrysMod/Lua/Types.h>

struct Rule {
  std::string identifier;
  std::string route;
  int reference = -1;
};

namespace clsp {
  namespace api {
    void Initialize(GarrysMod::Lua::ILuaBase* LUA);
    void Shutdown(GarrysMod::Lua::ILuaBase* LUA);

    namespace internal {
      void Run(GarrysMod::Lua::ILuaBase* LUA, const std::string& path, std::string& contents);
    } // namespace internal
  } // namespace api
} // namespace clsp