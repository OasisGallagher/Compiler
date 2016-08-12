#pragma once
#include "utilities.h"

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
	RefCountable() : refCount_(1) {
	}
	unsigned IncRefCount() {
		return ++refCount_;
	}

	unsigned DecRefCount() {
		Assert(refCount_ > 0, "can't decrease reference count");
		return --refCount_;
	}

private:
	unsigned refCount_;
};
