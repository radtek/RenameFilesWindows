#include "stdafx.h"
#include "ChangeFileNames.h"
#include <cctype>       // towlower
#include <filesystem>
#include <iostream>
#include <iterator>
#include <regex>
#include <vector>

using namespace std;

namespace
{
	constexpr bool bDryRun = true;
	const wstring workingDirectory = L"C:\\Christine\\travel\\2016 New Zealand\\2016_02 NZ album in process\\photos";
	const wstring readOnlyDirectory = L"C:\\Users\\christine\\Pictures\\2016\\2016_02 NZ originals\\From raja";
}

FileWorker::FileWorker()
{
	// Pick which function(s) to do
	SearchAndAlphabetize();
	//ChangeNames(workingDirectory);
}

void FileWorker::SearchAndAlphabetize()
{
	SetCurrentDirectory(readOnlyDirectory.c_str());
	wchar_t dirBuffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, dirBuffer);
	wcout << L"Current dir now " << dirBuffer << endl;

	vector<wstring> rajasFiles;
	CFileFind finder;
	BOOL found = finder.FindFile(NULL);
	while (found)
	{
		found = finder.FindNextFile();
		if (!finder.IsDots() && !finder.IsDirectory())
		{
			const wstring fileName = finder.GetFileName().GetBuffer(0);
			if (fileName.find(L".zip") == wstring::npos)
			{
				rajasFiles.push_back(finder.GetFileName().GetBuffer(0));
			}
		}
	}
	finder.Close();

	SetCurrentDirectory(workingDirectory.c_str());
	GetCurrentDirectory(MAX_PATH, dirBuffer);
	wcout << L"Current dir now " << dirBuffer << endl;
	for (const auto& r : rajasFiles)
	{
		wcout << L"raja file " << r << L"\t";
		auto fileRelative = GetFileRelative(r);
		if (fileRelative.empty())
		{
			// look up partial matches for info / this will print the results
			GetFileRelative(r, LookForType::Partial);
		}
		else
		{
			wcout << L" found at " << fileRelative;

			// TODO HERE jump in and change the name
		}
		wcout << endl;
	}
}

wstring FileWorker::GetFileRelative(const wstring& lookingForThisFile, LookForType type)
{
	if (!bFilesystemRead)
	{
		vector<wstring> folderNames;
		folderNames.push_back(L".");
		CFileFind finder;

		auto i = 0u;
		while (i < folderNames.size())
		{
			const wstring all = folderNames[i] + L"\\" + L"*.*";
			BOOL found = finder.FindFile(all.c_str());
			while (found)
			{
				found = finder.FindNextFile();
				if (!finder.IsDots())
				{
					const wstring itemName = finder.GetFileName().GetBuffer(0);
					const auto relativeName = folderNames[i] + L"\\" + itemName;
					if (finder.IsDirectory())
					{
						folderNames.push_back(relativeName);
					}
					else
					{
						m_files.insert(make_pair(itemName, relativeName));
					}
				}
			}
			finder.Close();
			i++;
		}
		bFilesystemRead = true;
	}

	wstring answer;
	if (type == LookForType::Exact)
	{
		auto numFound = m_files.count(lookingForThisFile);
		if (numFound == 0)
		{
			wcout << lookingForThisFile << L" not found\n";
		}
		else
		{
			// Set the answer to the first one found
			answer = m_files.find(lookingForThisFile)->second;
			if (numFound > 1)
			{
				wcout << L"More than one " << lookingForThisFile << L" file found!\n";
				auto rangeIters = m_files.equal_range(lookingForThisFile);
				for (auto iter = rangeIters.first; iter != rangeIters.second; ++iter)
				{
					wcout << L"\t" << iter->second << endl;
				}
			}
		}
	}
	else  // print info on partial matches
	{
		// cumbersome - make both strings the same case
		wstring lowercaseLookingFor = lookingForThisFile;
		for (auto& c : lowercaseLookingFor) { c = ::towlower(c); }
		for (const auto& a : m_files)
		{
			wstring lowercaseFileName = a.first;
			for (auto& c : lowercaseFileName) { c = ::towlower(c); }
			if (lowercaseFileName.find(lowercaseLookingFor) != wstring::npos)
			{
				wcout << L"\tPossibly " << a.second << endl;
			}
		}
	}
	return answer;
}

void FileWorker::ChangeNames(const wstring& workingDir)
{
	SetCurrentDirectory(workingDirectory.c_str());
	wchar_t dirBuffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, dirBuffer);
	wcout << L"Current dir now " << dirBuffer << endl;

	CFileFind finder;
	BOOL found = finder.FindFile(NULL);
	while (found)
	{
		found = finder.FindNextFile();
		if (!finder.IsDots() && !finder.IsDirectory())
		{
			ChangeName(finder);
		}
	}
	finder.Close();
}

void FileWorker::ChangeName(CFileFind& finder)
{
	wstring fname = finder.GetFileName().GetBuffer(0);
	wregex matchCriteria(L"^(IMG_\\d{4}\\.)(?!(Doug|Astrid))(.*)", std::regex_constants::icase);
	wsmatch matchResults;
	regex_search(fname, matchResults, matchCriteria);
	bool bTryFirst = true;
	if (matchResults.ready() && !matchResults.empty())
	{
		// could use matchResults[3].str() for the ".jpg" part, but some are capitalized and others are not
		// and I'd like them to all be the same.
		wstring newName = matchResults[1].str() + L"Doug\'s.jpg";
		wcout << L"Candidate " << fname << L" will be renamed " << newName << endl;
		if (!bDryRun)
		{
			std::experimental::filesystem::rename(fname, newName);
		}
	}
}
//wregex matchCriteria(L"^(IMG_\\d{4})\\.(?![^(Doug|Astrid)])(.*)", std::regex_constants::icase);
//wcout << L"Candidate " << fname << L" matched ";
//for (auto i : matchResults)
//{
//	wcout << L" [" << i.str() << L"]";
//}
//wcout << endl;
//wstring newName = matchResults[1].str() + L"Doug\'s." + matchResults[3].str();
