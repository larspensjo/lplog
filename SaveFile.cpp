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
#include <windows.h>
#endif // _WIN32
#include <gtk/gtk.h>
#include <iostream>
#include <fstream>

#include "SaveFile.h"

using std::string;

const string sStringPrefix = "STR-";
const string sNumberPrefix = "NUM-";
const string sCommentPrefix = "#";
const string sRecentFileNamePrefix = "REC-";

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
		g_debug("SaveFile::Read key '%s' is '%s'", key.c_str(), val.c_str());
		mData[key] = val;
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
	mData[sNumberPrefix + key] = std::to_string(val);
}

int SaveFile::GetIntOption(const std::string &id, int def) {
	if (!IsValidKey(id)) {
		g_debug("SaveFile::GetIntOption Invalid key %s", id.c_str());
		return 0;
	}
	string key = sNumberPrefix + id;
	auto it = mData.find(key);
	if (it == mData.end())
		mData[key] = std::to_string(def);
	return std::stoi(mData[key]);
}

std::string SaveFile::GetRecentFileName(int n) {
	string key = sRecentFileNamePrefix + std::to_string(n);
	return mData[key];
}

void SaveFile::SetRecentFileName(int n, const std::string &fn) {
	string key = sRecentFileNamePrefix + std::to_string(n);
	mData[key] = fn;
}

string SaveFile::GetPath() const {
	string dataDir; // The directory where the client can save data
	string optionsFilename;
#ifdef __gnu_linux__
	dataDir = getenv("HOME");
	optionsFilename = dataDir + "/." + mFileName;
#endif
#ifdef _WIN32
	TCHAR home[MAX_PATH];
	HRESULT res = SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, 0, 0, home);
	if (res == S_OK) {
		dataDir = string(home) + "\\lplog";
		struct _stat st;
		if (_stat(dataDir,&st) != 0) {
			res = _mkdir(dataDir);
		}
		optionsFilename = dataDir + "\\" + mFileName + ".ini";
	}
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
