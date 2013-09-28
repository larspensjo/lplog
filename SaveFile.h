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

#pragma once

#include <string>
#include <map>

class SaveFile
{
public:
	SaveFile(const std::string &fn) : mFileName(fn) {}
	void Read();
	void Write();

	void SetStringOption(const std::string &id, const std::string&val);
	std::string GetStringOption(const std::string &id, const std::string &def = "");

	void SetIntOption(const std::string &id, int val);
	int GetIntOption(const std::string &id, int def = 0);

	std::string GetRecentFileName(int);
	void SetRecentFileName(int, const std::string &);
private:
	const std::string mFileName;
	std::map<std::string, std::string> mData;
	std::string GetPath() const;
	static bool IsValidKey(const std::string &);
	static bool IsValidValue(const std::string &);
};
