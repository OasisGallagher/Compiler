#pragma once

#include <map>
#include <string>
#include "token.h"

class LineScanner {
public:
	LineScanner();
	~LineScanner();

	void SetText(const char* text);
	ScannerTokenType GetToken(char* token);

private:
	bool GetChar(int* ch);
	void UngetChar();

	int tokenBufferIndex_;
	char* lineBuffer_;
	char* tokenBuffer_;
	char* dest_;
	char* current_;
};

class FileReader;

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

class FileScanner {
public:
	FileScanner(const char* path);
	~FileScanner();

	bool GetToken(ScannerToken* token);

private:
	ScannerTokenType GetReserveTokenType(const char* name);
private:
	FileReader* reader_;
	LineScanner lineScanner_;

	Table <Symbol >* symbols_;
	Table <NumberLiteral>* numberLiterals_;
	Table <StringLiteral>* stringLiterals_;
};
