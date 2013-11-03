// Copyright 2013 Lars Pensjö
//
// Lplog is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3.
//
// Lplog is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Lplog.  If not, see <http://www.gnu.org/licenses/>.
//

#ifdef _WIN32
	#undef __STRICT_ANSI__ // Needed for "struct stat" in MinGW.
	#include <sys/stat.h>
	#include <windows.h>
	#include <Shlobj.h>
#endif // _WIN32
#ifdef __linux__
   #include <sys/stat.h>
   #include <sys/types.h>
#endif // _linux
#include <gtk/gtk.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "SaveFile.h"

using std::string;

#ifdef _WIN32
string to_string(int val) {
	std::stringstream ss;
	ss << val;
	return ss.str();
}
int stoi(const string &str) {
	return atoi(str.c_str());
}
#else
using std::to_string;
using std::stoi;
#endif

const string sStringPrefix = "STR-";
const string sNumberPrefix = "NUM-";
const string sPatternPrefix = "PAT-";
const string sCommentPrefix = "#";

void SaveFile::Read() {
	const string path = GetPath();
	std::ifstream input(path);
	if (!input.is_open()) {
		g_debug("SaveFile::Read Failed to open '%s'", path.c_str());
		return;
	}
	g_debug("SaveFile::Read from %s", path.c_str());
	while(!input.eof()) {
		string line;
		std::getline(input, line);
		string::size_type p = line.find(sCommentPrefix);
		if (p == 0)
			continue;
		if (p != string::npos)
			line = line.substr(0, p-1);
		if (line.size() == 0)
			continue;
		p = line.find(':');
		if (p == string::npos) {
			g_debug("SaveFile::Read illegal line '%s'", line.c_str());
			continue;
		}
		string key = line.substr(0, p);
		string val = line.substr(p+1);
		if (key.substr(0, sPatternPrefix.size()) == sPatternPrefix) {
			const string patterName = key.substr(sPatternPrefix.size());
			gsize size;
			guchar *p = g_base64_decode(patterName.c_str(), &size);
			g_debug("SaveFile::Read pattern '%s' is '%s'", (const char *)p, val.c_str());
			mPatterns[(const char *)p] = val;
			g_free(p);
		} else {
			g_debug("SaveFile::Read key '%s' is '%s'", key.c_str(), val.c_str());
			mData[key] = val;
		}
	}
}

void SaveFile::Write() {
	const string path = GetPath();
	std::ofstream output(path);
	if (!output.is_open()) {
		g_debug("SaveFile::Write Failed to open '%s'", path.c_str());
		return;
	}
	g_debug("SaveFile::Write Saving to %s", path.c_str());
	output << sCommentPrefix << std::endl;
	output << sCommentPrefix << " Options file for LPlog" << std::endl;
	output << sCommentPrefix << std::endl;
	for (auto pair : mData) {
		if (pair.second == "")
			continue; // Ignore empty values
		output << pair.first << ":" << pair.second << std::endl;
	}
	for (auto pair : mPatterns) {
		if (pair.second == "")
			continue; // Ignore empty values
		const char *key = g_base64_encode((const guchar*)pair.first.c_str(), pair.first.size());
		output << sPatternPrefix << key << ":" << pair.second << std::endl;
		g_free((gpointer)key);
	}
}

void SaveFile::SetStringOption(const std::string &key, const std::string&val) {
	if (!IsValidKey(key)) {
		g_debug("SaveFile::SetStringOption Invalid key %s", key.c_str());
		return;
	}
	if (!IsValidValue(val)) {
		g_debug("SaveFile::SetStringOption Invalid value %s", val.c_str());
		return;
	}
	mData[sStringPrefix + key] = val;
}

string SaveFile::GetStringOption(const string &id, const std::string &def) {
	if (!IsValidKey(id)) {
		g_debug("SaveFile::GetStringOption Invalid key %s", id.c_str());
		return "";
	}
	string key = sStringPrefix + id;
	auto it = mData.find(key);
	if (it == mData.end())
		mData[key] = def;
	return mData[key];
}

void SaveFile::SetIntOption(const std::string &key, int val) {
	if (!IsValidKey(key)) {
		g_debug("SaveFile::SetIntOption Invalid key %s", key.c_str());
		return;
	}
	mData[sNumberPrefix + key] = to_string(val);
}

int SaveFile::GetIntOption(const std::string &id, int def) {
	if (!IsValidKey(id)) {
		g_debug("SaveFile::GetIntOption Invalid key %s", id.c_str());
		return 0;
	}
	string key = sNumberPrefix + id;
	auto it = mData.find(key);
	if (it == mData.end())
		mData[key] = to_string(def);
	return stoi(mData[key]);
}

string SaveFile::GetPath() const {
	string dataDir; // The directory where the client can save data
	string optionsFilename;
#if defined(__linux__)
	dataDir = getenv("HOME") + string("/.config");
	mkdir(dataDir.c_str(), 0700); // Make sure it exists, ignore result
	optionsFilename = dataDir + "/" + mFileName + ".ini";
#elif defined(_WIN32)
	TCHAR home[MAX_PATH];
	HRESULT res = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, 0, 0, home);
	if (res == S_OK) {
		dataDir = string(home) + "\\lplog";
		struct _stat st;
		if (_stat(dataDir.c_str(),&st) != 0) {
			bool ok = CreateDirectory(dataDir.c_str(), NULL);
			g_debug("Created application folder '%s' [%s]", dataDir.c_str(), ok?"SUCCESS":"FAIL");
		}
		optionsFilename = dataDir + "\\" + mFileName + ".ini";
	}
#else
	error Neither linux nor windows
#endif
	return optionsFilename;
}

bool SaveFile::IsValidKey(const std::string &key) {
	if (!IsValidValue(key))
		return false;
	if (key.find(':') != string::npos)
		return false;
	return true;
}

bool SaveFile::IsValidValue(const std::string &val) {
	if (val.find('\n') != string::npos)
		return false;
	if (val.find('\r') != string::npos)
		return false;
	return true;
}

void SaveFile::SetPattern(const std::string &id, const std::string&val)
{
	if (val == "")
		mPatterns.erase(id);
	else
		mPatterns[id] = val;
}

std::string SaveFile::GetPattern(const std::string &id, const std::string &def)
{
	auto it = mPatterns.find(id);
	if (it == mPatterns.end())
		return def;
	return it->second;
}

void SaveFile::IteratePatterns(std::function<void (const std::string &name, const std::string &value)> f) {
	for (auto &pair : mPatterns) {
		f(pair.first, pair.second);
	}
}
