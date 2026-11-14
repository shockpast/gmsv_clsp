#include <GarrysMod/InterfacePointers.hpp>
#include <GarrysMod/ModuleLoader.hpp>
#include <scanning/symbolfinder.hpp>

#include <networkstringtabledefs.h>
#include <tier0/dbg.h>
#include <tier1/bitbuf.h>
#include <eiface.h>

#include "picosha2/picosha2.h"

#include "sdk/datapack.h"
#include "datapack.h"
#include "symbols.h"

static GModDataPack* g_pDataPack;
static IVEngineServer* g_pServer = InterfacePointers::VEngineServer();

void DataPack::Initialize()
{
	// hooks must access our class somehow
	m_pSingleton = this;

	SourceSDK::ModuleLoader server("server");
	SymbolFinder sym;

	m_hAddOrUpdateFileHook.Create(sym.Resolve(server.GetModule(), DataPack_AddOrUpdateFile_Sym.name.c_str()), reinterpret_cast<void*>(AddOrUpdateFile));
	m_hAddOrUpdateFileHook.Enable();
	m_hSendFileToClientHook.Create(sym.Resolve(server.GetModule(), DataPack_SendFileToClient_Sym.name.c_str()), reinterpret_cast<void*>(SendFileToClient));
	m_hSendFileToClientHook.Enable();
	m_hOnFilesRequestedHook.Create(sym.Resolve(server.GetModule(), DataPack_OnFilesRequested_Sym.name.c_str()), reinterpret_cast<void*>(OnFilesRequested));
	m_hOnFilesRequestedHook.Enable();
}

void DataPack::SendLuaFile(ProcessedFile *file, int clientId, int fileId)
{
	size_t bytes = 3 + file->compressed.GetWritten() + 14;
	std::vector<uint32_t> buffer((bytes + sizeof(std::uint32_t) - 1) / sizeof(std::uint32_t));

	bf_write message(buffer.data(), static_cast<int>(bytes));
	message.WriteByte(/* GarrysMod::NetworkMessage::LuaFileDownload */4);
	message.WriteUBitLong(fileId, 16);
	message.WriteBytes(file->compressed.GetBase(), file->compressed.GetWritten());

	if (message.IsOverflowed())
	{
		DevWarning("Failed to send a .lua file to client, message is overflowed!\n");
		return;
	}

	g_pServer->GMOD_SendToClient(clientId, message.GetData(), message.GetNumBitsWritten());
}

void DataPack::ProcessFile(FileTask& task, ProcessedFile& result)
{
	result.fileId = std::move(task.fileId);
	result.name = std::move(task.name);
	result.contents = std::move(task.data);

	picosha2::hash256(result.contents.c_str(), result.contents.c_str() + result.contents.length() + 1, result.hash.begin(), result.hash.end());

	// write a checksum header for client to validate if file changed
	auto& compressed = result.compressed.Get();
	compressed.Write(result.hash.data(), result.hash.size());

	// write compressed contents of file
	Bootil::Compression::LZMA::Compress(result.contents.data(), result.contents.length() + 1, compressed, 9);
}

bool DataPack::CommitFile(ProcessedFile&& file)
{
	if (!g_pDataPack || !g_pDataPack->m_pClientLuaFiles)
		return false;

	const int fileId = file.fileId;

	if (fileId <= 0 || static_cast<size_t>(fileId) >= MAX_LUA_FILES)
		return false;

	g_pDataPack->m_pClientLuaFiles->SetStringUserData(fileId, static_cast<int>(file.hash.size()), file.hash.data());
	m_pFiles[fileId] = std::make_unique<ProcessedFile>(std::move(file));

	return true;
}

bool DataPack::HasPendingFiles()
{
	std::lock_guard<std::mutex> lock(m_pendingMutex);
	return !m_pendingQueue.empty();
}

void DataPack::StartProcessing()
{
	if (m_bProcessing.load() || m_bCompleted.load())
		return;

	std::vector<FileTask> tasks;

	{
		std::lock_guard<std::mutex> lock(this->m_pendingMutex);

		tasks = std::move(this->m_pendingQueue);
		this->m_pendingQueue.clear();
	}

	if (tasks.empty())
		return;

	const unsigned int hardwareThreads = std::thread::hardware_concurrency();

	const size_t threadCount = std::min(tasks.size(), static_cast<size_t>(hardwareThreads > 1 ? hardwareThreads - 1 : 1));
	const size_t chunkSize = (tasks.size() + threadCount - 1) / threadCount;

	this->m_workers.reserve(threadCount);
	this->m_activeQueue = std::move(tasks);

	this->m_bCompleted.store(false);
	this->m_bProcessing.store(true);
	this->m_iActiveWorkers.store(threadCount);

	for (size_t i = 0; i < threadCount; ++i)
	{
		const size_t begin = i * chunkSize;
		const size_t end   = std::min(begin + chunkSize, this->m_activeQueue.size());

		try
		{
			m_workers.emplace_back([this, begin, end]() -> void {
				try
				{
					for (size_t j = begin; j < end; ++j)
					{
						ProcessedFile result;
						ProcessFile(this->m_activeQueue[j], result);

						std::lock_guard<std::mutex> lock(this->m_completedMutex);
						m_completedQueue.emplace_back(std::move(result));
					}
				}
				catch (...)
				{
					// * :shrug:
				}

				if (m_iActiveWorkers.fetch_sub(1) == 1)
				{
					m_bProcessing.store(false);
					m_bCompleted.store(true);
				}
			});
		}
		catch (...)
		{
			if (m_iActiveWorkers.fetch_sub(1) == 1)
			{
				m_bProcessing.store(false);
				m_bCompleted.store(true);
			}
		}
	}

	if (this->m_iActiveWorkers.load() == 0)
	{
		this->m_bProcessing.store(false);
		this->m_bCompleted.store(true);
	}
}

void DataPack::CollectResults()
{
	if (!HasCompleted())
		return;

	for (auto& worker : m_workers)
		if (worker.joinable())
			worker.join();

	m_workers.clear();

	std::vector<ProcessedFile> completed;

	{
		std::lock_guard<std::mutex> lock(this->m_completedMutex);

		completed = std::move(this->m_completedQueue);
		this->m_completedQueue.clear();
	}

	size_t committed = 0;

	for (auto& file : completed)
	{
		if (CommitFile(std::move(file)))
			++committed;
	}

	this->m_activeQueue.clear();
	this->m_bCompleted.store(false);

	Msg("*cdatapack: commited %llu/%llu files\n", static_cast<unsigned long long>(committed), static_cast<unsigned long long>(completed.size()));
}

void DataPack::SendFileToClient(GModDataPack* datapack /* ecx */, int clientId, int fileId)
{
	if (datapack == nullptr)
	{
		DevWarning("GModDataPack is missing.\n");
		return;
	}

	INetworkStringTable* clientLuaFiles = datapack->m_pClientLuaFiles;

	if (clientLuaFiles == nullptr)
	{
		DevWarning("client_lua_files stringtable is missing.\n");
		return;
	}

	DataPack* self = m_pSingleton;

	if (self == nullptr)
	{
		Error("DataPack singleton doesn't exist at the moment of function call.\n");
		return;
	}

	// * fileId == 0          - reserved for search paths
	// * fileId  < 0          - simply don't exist
	// * fileId >= numstrings - more than exists in client_lua_files stringtable
	if (fileId <= 0 || fileId >= clientLuaFiles->GetNumStrings())
	{
		DevWarning("Client tried to access file that is either out-of-bounds or reserved.\n");
		return;
	}

	ProcessedFile* file = self->m_pFiles[fileId].get();

	if (file == nullptr)
	{
		DevWarning("Client tried to access file that is not in cache.\n");
		return;
	}

	self->SendLuaFile(file, clientId, fileId);
}

void DataPack::AddOrUpdateFile(GModDataPack* datapack /* ecx */, GarrysMod::Lua::File* file, bool recompress)
{
	if (g_pDataPack == nullptr)
		g_pDataPack = datapack;

	DataPack* self = m_pSingleton;

	if (self == nullptr)
	{
		Error("DataPack singleton doesn't exist at the moment of function call.\n");
		return;
	}

	if (file == nullptr)
	{
		DevWarning("File doesn't exist but was provided for add?!\n");
		return;
	}

	INetworkStringTable* table = datapack->m_pClientLuaFiles;

	int fileId = table->FindStringIndex(file->name.c_str());

	if (fileId == INVALID_STRING_INDEX)
		fileId = table->AddString(true, file->name.c_str());

	if (fileId == INVALID_STRING_INDEX)
	{
		DevWarning("*cdatapack: failed to register %s\n", file->name.c_str());
		return;
	}

	if (fileId <= 0 || static_cast<size_t>(fileId) >= INVALID_STRING_INDEX)
	{
		DevWarning("*cdatapack: invalid file id for %s\n", file->name.c_str());
		return;
	}

	FileTask task;
	task.fileId = fileId;
	task.name = file->name;
	task.data = file->contents;

	{
		std::lock_guard<std::mutex> lock(self->m_pendingMutex);
		self->m_pendingQueue.emplace_back(std::move(task));
	}

	DevMsg(2, "Added %s into CDataPack file queue.\n", file->name.c_str());
}

void DataPack::OnFilesRequested(GModDataPack* datapack /* ecx */, int clientId, bf_read* message, int bits)
{
	if (!datapack || !datapack->m_pClientLuaFiles || !message)
		return;

	if (bits <= 0 || (bits & 0xF) != 0)
		return;

	const int count = bits / 16;
	const int numStrings = datapack->m_pClientLuaFiles->GetNumStrings();

	Msg("*cdatapack: count = %i\n", count);

	if (count > numStrings)
		return;

	std::vector<int> fileIds;
	fileIds.reserve(count);

	for (int i = 0; i < count; ++i)
	{
		const unsigned int fileId = message->ReadUBitLong(16);

		if (message->IsOverflowed())
			return;

		if (fileId == 0 || fileId >= static_cast<unsigned int>(numStrings))
			continue;

		fileIds.emplace_back(static_cast<int>(fileId));
	}

	if (message->IsOverflowed())
		return;

	std::sort(fileIds.begin(), fileIds.end());
	fileIds.erase(std::unique(fileIds.begin(), fileIds.end()), fileIds.end());

	DataPack* self = m_pSingleton;

	if (!self)
		return;

	for (const int fileId : fileIds)
		SendFileToClient(datapack, clientId, fileId);
}