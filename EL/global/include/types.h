#pragma once

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