#pragma once
#include "token_define.h"

class Tokens {
public:
	static int Size();
	static const char* Text(int index);
	static ScannerTokenType Type(int index);

private:
	static Tokens instance;

private:
	Tokens();
};
