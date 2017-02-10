#include "debug.h"
#include "action.h"
#include "parser.h"
#include "grammar.h"
#include "lr_table.h"
#include "syntaxer.h"
#include "serializer.h"
#include "grammar_symbol.h"

#define MAX_SERIALIZABLE_CHARACTERS		256

static char intBuffer[sizeof(int)];
static char strBuffer[MAX_SERIALIZABLE_CHARACTERS];

bool Serializer::SaveEnvironment(std::ofstream& file, Environment* env) {
	if (!SaveSymbols(file, env->terminalSymbols)) {
		Debug::LogError("failed to save terminal symbols.");
		return false;
	}

	// TODO: 非终结符没有必要存储字符串, 为它们对应一个ID即可.
	if (!SaveSymbols(file, env->nonterminalSymbols)) {
		Debug::LogError("failed to save non-terminal symbols.");
		return false;
	}

	if (!SaveGrammars(file, env->grammars)) {
		Debug::LogError("failed to save grammars.");
		return false;
	}

	return true;
}

bool Serializer::LoadEnvironment(std::ifstream& file, Environment* env) {
	if (!LoadSymbols(file, env->terminalSymbols)) {
		Debug::LogError("failed to save terminal symbols.");
		return false;
	}

	if (!LoadSymbols(file, env->nonterminalSymbols)) {
		Debug::LogError("failed to save non-terminal symbols.");
		return false;
	}

	NativeSymbols::Copy(env->terminalSymbols, env->nonterminalSymbols);

	if (!LoadGrammars(file, env->terminalSymbols, env->nonterminalSymbols, env->grammars)) {
		Debug::LogError("failed to save grammars.");
		return false;
	}

	return true;
}

bool Serializer::SaveSyntaxer(std::ofstream& file, const SyntaxerSetupParameter& p) {
	if (!SaveLRTable(file, p.lrTable)) {
		Debug::LogError("failed to save lr-table.");
		return false;
	}

	return true;
}

bool Serializer::LoadSyntaxer(std::ifstream& file, SyntaxerSetupParameter& p) {
	if (!LoadLRTable(file, p.env->terminalSymbols, p.env->nonterminalSymbols, p.lrTable)) {
		Debug::LogError("failed to save lr-table.");
		return false;
	}

	return true;
}

bool Serializer::SaveSymbols(std::ofstream& file, const GrammarSymbolContainer& cont) {
	typedef std::ios::pos_type pos_type;
	pos_type oldpos = file.tellp(), newpos;
	int count = 0;
	file.seekp(oldpos + (pos_type)sizeof(count));

	for (GrammarSymbolContainer::const_iterator ite = cont.begin(); ite != cont.end(); ++ite) {
		if (NativeSymbols::IsNative(ite->second)) {
			continue;
		}

		++count;
		if (!WriteString(file, ite->first)) {
			return false;
		}
	}

	newpos = file.tellp();
	file.seekp(oldpos);
	WriteInteger(file, count);

	file.seekp(newpos);

	return true;
}

bool Serializer::LoadSymbols(std::ifstream& file, GrammarSymbolContainer& cont) {
	int count = 0;
	if (!ReadInteger(file, count)) {
		return false;
	}

	std::string text;
	for (int i = 0; i < count; ++i) {
		if (!ReadString(file, text)) {
			return false;
		}

		GrammarSymbol symbol = SymbolFactory::Create(text);
		cont.insert(std::make_pair(symbol.ToString(), symbol));
	}

	return true;
}

bool Serializer::SaveGrammars(std::ofstream& file, const GrammarContainer& cont) {
	WriteInteger(file, cont.size());

	for (GrammarContainer::const_iterator ite = cont.begin(); ite != cont.end(); ++ite) {
		const Grammar* g = *ite;
		if (!WriteString(file, g->GetLhs().ToString())) {
			return false;
		}

		const CondinateContainer& conds = g->GetCondinates();
		if (!WriteInteger(file, conds.size())) {
			return false;
		}

		for (CondinateContainer::const_iterator ite = conds.begin(); ite != conds.end(); ++ite) {
			const Condinate* c = *ite;
			if (!WriteInteger(file, c->symbols.size())) {
				return false;
			}

			for (SymbolVector::const_iterator ite2 = c->symbols.begin(); ite2 != c->symbols.end(); ++ite2) {
				if (!WriteString(file, ite2->ToString())) {
					return false;
				}
			}

			if (!WriteString(file, c->action != nullptr ? c->action->ToString() : "")) {
				return false;
			}
		}
	}

	return true;
}

bool Serializer::LoadGrammars(std::ifstream& file, GrammarSymbolContainer& terminalSymbols, GrammarSymbolContainer& nonterminalSymbols, GrammarContainer& grammars) {
	int count = 0;
	if (!ReadInteger(file, count)) {
		return false;
	}

	std::string ltext;
	for (int i = 0; i < count; ++i) {
		if (!ReadString(file, ltext)) {
			return false;
		}

		GrammarSymbol lhs = nonterminalSymbols[ltext];
		Assert(lhs != NativeSymbols::null, "can not find non-terminal symbol " + ltext);

		Grammar* grammar = new Grammar(lhs);
		grammars.push_back(grammar);

		if (!LoadCondinates(terminalSymbols, nonterminalSymbols, file, grammar)) {
			return false;
		}
	}

	return true;
}

bool Serializer::SaveLRTable(std::ofstream& file, const LRTable& table) {
	if (!SaveLRActionTable(file, table.actionTable_)) {
		return false;
	}

	return SaveLRGotoTable(file, table.gotoTable_);
}

bool Serializer::LoadLRTable(std::ifstream& file, GrammarSymbolContainer& terminalSymbols, GrammarSymbolContainer& nonterminalSymbols, LRTable& table) {
	if (!LoadLRActionTable(file, terminalSymbols, table)) {
		return false;
	}

	return LoadLRGotoTable(file, nonterminalSymbols, table);
}

bool Serializer::LoadCondinates(GrammarSymbolContainer& terminalSymbols, GrammarSymbolContainer& nonterminalSymbols, std::ifstream& file, Grammar* grammar) {
	int cn = 0;
	SymbolVector symbols;
	std::string stext, atext;

	if (!ReadInteger(file, cn)) {
		return false;
	}

	for (int i = 0; i < cn; ++i) {
		int sn = 0;
		if (!ReadInteger(file, sn)) {
			return false;
		}

		for (int j = 0; j < sn; ++j) {
			if (!ReadString(file, stext)) {
				return false;
			}

			GrammarSymbolContainer::iterator pos = terminalSymbols.find(stext);
			if (pos == terminalSymbols.end()) {
				pos = nonterminalSymbols.find(stext);
				Assert(pos != nonterminalSymbols.end(), "can not find symbol " + stext);
			}

			GrammarSymbol symbol = pos->second;
			symbols.push_back(symbol);
		}

		if (!ReadString(file, atext)) {
			return false;
		}

		grammar->AddCondinate(atext, symbols);
		symbols.clear();
	}

	return true;
}

bool Serializer::SaveLRActionTable(std::ofstream& file, const LRActionTable &actionTable) {
	WriteInteger(file, actionTable.size());

	for (LRActionTable::const_iterator ite = actionTable.begin(); ite != actionTable.end(); ++ite) {
		if (!WriteInteger(file, ite->first.first)) {
			return false;
		}

		if (!WriteString(file, ite->first.second.ToString())) {
			return false;
		}

		if (!WriteInteger(file, ite->second.type)) {
			return false;
		}

		if (!WriteInteger(file, ite->second.parameter)) {
			return false;
		}
	}

	return true;
}

bool Serializer::SaveLRGotoTable(std::ofstream& file, const LRGotoTable &gotoTable) {
	WriteInteger(file, gotoTable.size());
	for (LRGotoTable::const_iterator ite = gotoTable.begin(); ite != gotoTable.end(); ++ite) {
		if (!WriteInteger(file, ite->first.first)) {
			return false;
		}

		if (!WriteString(file, ite->first.second.ToString())) {
			return false;
		}

		if (!WriteInteger(file, ite->second)) {
			return false;
		}
	}

	return true;
}

bool Serializer::LoadLRActionTable(std::ifstream& file, GrammarSymbolContainer& terminalSymbols, LRTable &table) {
	int count = 0;
	if (!ReadInteger(file, count)) {
		return false;
	}

	std::string stext;
	for (int i = 0; i < count; ++i) {
		int from, actionType, actionParameter;
		if (!ReadInteger(file, from)) {
			return false;
		}

		if (!ReadString(file, stext)) {
			return false;
		}

		GrammarSymbol symbol = terminalSymbols[stext];
		Assert(symbol != NativeSymbols::null, "invalid terminal symbol " + stext);

		if (!ReadInteger(file, actionType)) {
			return false;
		}

		if (!ReadInteger(file, actionParameter)) {
			return false;
		}

		LRAction action = { (LRActionType)actionType, actionParameter };
		table.actionTable_.insert(from, symbol, action);
	}

	return true;
}

bool Serializer::LoadLRGotoTable(std::ifstream& file, GrammarSymbolContainer& nonterminalSymbols, LRTable &table) {
	int count = 0;
	if (!ReadInteger(file, count)) {
		return false;
	}

	std::string stext;
	for (int i = 0; i < count; ++i) {
		int from, to;
		if (!ReadInteger(file, from)) {
			return false;
		}

		if (!ReadString(file, stext)) {
			return false;
		}

		GrammarSymbol symbol = nonterminalSymbols[stext];
		Assert(symbol != NativeSymbols::null, "invalid terminal symbol " + stext);

		if (!ReadInteger(file, to)) {
			return false;
		}

		table.gotoTable_.insert(from, symbol, to);
	}

	return true;
}

bool Serializer::WriteInteger(std::ofstream& file, int x) {
	return !!file.write((char*)&x, sizeof(x));
}

bool Serializer::WriteString(std::ofstream& file, const std::string& str) {
	Assert(str.length() < MAX_SERIALIZABLE_CHARACTERS, "string length exceed.");
	int count = (int)str.length();
	return file.write((char*)&count, sizeof(count)) && file.write(str.c_str(), count);
}

bool Serializer::ReadInteger(std::ifstream& file, int& integer) {
	if (!file.read(intBuffer, sizeof(int))) {
		return false;
	}
	
	integer = *(int*)intBuffer;
	return true;
}

bool Serializer::ReadString(std::ifstream& file, std::string& str) {
	int length = 0;
	if (!ReadInteger(file, length)) {
		return false;
	}

	Assert(length < MAX_SERIALIZABLE_CHARACTERS, "token length exceed.");

	if (!file.read(strBuffer, length)) {
		return false;
	}

	strBuffer[length] = 0;
	str.assign(strBuffer);
	return true;
}
