#pragma once
#include "parser.h"

class LRImpl;
class LRTable;

class LRParser : public Parser {
public:
	LRParser();
	~LRParser();

public:
	virtual std::string ToString() const;

protected:
	virtual bool ParseGrammars(Syntaxer& syntaxer, Environment* env);
	virtual void Clear();

private:
	void CreateFirstSets();
	void CreateFollowSets();

private:
	LRImpl* impl_;
	LRTable* lrTable_;
};
