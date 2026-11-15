#pragma once

#include <string>
#include <array>
#include <atomic>
#include <mutex>
#include <thread>

#define AutoBuffer _AutoBuffer /* hacky hack */
#include <GarrysMod/Lua/LuaShared.h>
#undef AutoBuffer
#include <detouring/hook.hpp>

#include "buffer.h"
#include "sdk/datapack.h"

struct FileTask {
	int fileId = (unsigned short )-1;

	std::string name;
	std::string data;
};

struct ProcessedFile {
	int fileId = (unsigned short )-1;

	std::string name;
	std::string contents;
	Buffer compressed;

	std::array<unsigned char, 32> hash{};
};

struct LuaFile {
  int time;
  std::string name;
  std::string source;
  std::string contents;
  Bootil::AutoBuffer compressed;
  int unk000;
  unsigned int timesLoadedServer;
  unsigned int timesLoadedClient;
};

class DataPack {
  /* functions */
public:
  DataPack() = default;
  ~DataPack();

  void Initialize();
  void Shutdown();
  void ResetLevel();
  bool IsProcessing() const { return m_bProcessing.load(); };
  bool HasCompleted() const { return !IsProcessing() && m_bCompleted.load(); };
  bool HasPendingFiles();
  void ProcessRules(std::vector<FileTask>& tasks);
  void ProcessQueue();
  void CollectResults();

  static void SendFileToClient(GModDataPack* datapack, int clientId, int fileId);
  static void AddOrUpdateFile(GModDataPack* datapack, LuaFile* file, bool recompress);
  static void OnFilesRequested(GModDataPack* datapack, int clientId, bf_read* message, int bits);

  private:
  void SendLuaFile(ProcessedFile* file, int clientId, int fileId);
  void BuildCompressed(const std::string& contents, const decltype(ProcessedFile::hash) & header, ProcessedFile& out);
  void ProcessFile(FileTask& task, ProcessedFile& out);
  bool CommitFile(ProcessedFile&& file);

  /* variables */
public:
  static constexpr size_t kMaxFiles = 2 << 12;

private:
  std::atomic<size_t> m_iActiveWorkers { 0 };
  std::atomic<bool> m_bProcessing { false };
  std::atomic<bool> m_bCompleted { false };
  std::atomic<bool> m_bStopRequested { false };

  std::unique_ptr<ProcessedFile> m_pFiles[kMaxFiles]{};

  // being currently worked on
  std::vector<FileTask> m_activeQueue;
  // just waiting to be picked up
  std::vector<FileTask> m_pendingQueue;
  // completed or failed tasks
  std::vector<ProcessedFile> m_completedQueue;

  std::mutex m_pendingMutex;
  std::mutex m_completedMutex;

  std::vector<std::thread> m_workers;

  // maybe we should store them outside class?
  Detouring::Hook m_hSendFileToClientHook;
  Detouring::Hook m_hAddOrUpdateFileHook;
  Detouring::Hook m_hOnFilesRequestedHook;

  inline static DataPack* m_pSingleton;
};

extern DataPack g_DataPack;