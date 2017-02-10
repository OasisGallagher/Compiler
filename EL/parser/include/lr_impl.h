#pragma once
#include "matrix.h"

struct Environment;

class LRGotoTable;
class LRActionTable;
class FirstSetTable;
class GrammarContainer;

enum LRActionType {
	LRActionError,
	LRActionShift,
	LRActionReduce,
	LRActionAccept,
};

struct LRAction {
	LRActionType type;
	int parameter;

	bool operator == (const LRAction& other) const;
	bool operator != (const LRAction& other) const;

	std::string ToString(const GrammarContainer& grammars) const;
};

class LRImpl {
public:
	virtual ~LRImpl() { }
	
public:
	virtual std::string ToString() const = 0;

	virtual bool Parse(LRActionTable& actionTable, LRGotoTable& gotoTable) = 0;
	virtual void Setup(Environment* env, FirstSetTable* firstSet) = 0;
};
