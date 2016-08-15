#pragma once
#include <map>
#include <string>
#include <fstream>

template <class Ty>
class Table {
public:
	typedef std::map<std::string, Ty*> Container;

	~Table() {
		for (Container::iterator ite = cont_.begin();
			ite != cont_.end(); ++ite) {
			delete ite->second;
		}
	}

public:
	Ty* Add(const char* text) {
		Container::iterator ite = cont_.find(text);
		Ty* ans = nullptr;

		if (ite == cont_.end()) {
			ans = new Ty(text);
			cont_[text] = ans;
		}
		else {
			ans = ite->second;
		}

		return ans;
	}

private:
	Container cont_;
};

class RefCountable {
protected:
	RefCountable();

	unsigned IncRefCount();

	unsigned DecRefCount();

private:
	unsigned refCount_;
};

class FileReader {
public:
	FileReader(const char* path);
	~FileReader();

public:
	bool ReadLine(char* buffer, size_t length);

private:
	std::ifstream ifs_;
};

