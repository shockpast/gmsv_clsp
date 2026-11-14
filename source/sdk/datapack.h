#pragma once

#include <string>
#include <vector>

#include <platform.h>

class LuaFindResult;
class bf_read;
class INetworkStringTable;

abstract_class IGModDataPack {
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

class GModDataPack : public IGModDataPack {
private: // i don't care about these functions
	void OnClientConnected(int userId);
	bool IsSinglePlayer();
	void OnFilesRequested(int userId, bf_read* message, int bits);

public:
	INetworkStringTable* m_pClientLuaFiles;
};
