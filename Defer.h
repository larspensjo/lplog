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

#include <functional>

// Deferred execution of a function. It will be called when the object goes out of scope.
// Always use a named object, or it will go out of scope immediately.
class Defer {
public:
	typedef std::function<void (void)> ptr;
	Defer() : mFunction(nullptr) {}
	Defer(ptr f) : mFunction(f) { }
	~Defer() { if (mFunction != nullptr) mFunction(); }
	void Set(ptr f) { mFunction = f; }
	void operator=(ptr f) { mFunction = f; }
private:
	ptr mFunction;
};
