#pragma once
#include <string>
#include <unordered_map>

enum class LookForType
{
	Exact,
	Partial
};

class FileWorker
{
public:
	FileWorker();
	virtual ~FileWorker() {}
private:
	void ChangeNames(const std::wstring& workingDir);
	void ChangeName(CFileFind& finder);
	std::wstring GetFileRelative(const std::wstring& filename, LookForType type = LookForType::Exact);
	void SearchAndAlphabetize();

	std::unordered_multimap<std::wstring, std::wstring> m_files;
	bool bFilesystemRead = false;
};

