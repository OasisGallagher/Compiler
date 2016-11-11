#pragma once
#include <map>

template <class Ty>
class Table {
public:
	typedef Ty value_type;
	typedef std::map<std::string, value_type*> container_type;

public:
	~Table() {
		for (container_type::iterator ite = cont_.begin(); ite != cont_.end(); ++ite) {
			delete ite->second;
		}
	}

public:
	value_type* Add(const std::string& text) {
		container_type::iterator ite = cont_.find(text);
		if (ite != cont_.end()) {
			return ite->second;
		}

		value_type* val = new value_type(text);
		cont_.insert(std::make_pair(text, val));

		return val;
	}

	value_type* Get(const std::string& text) {
		container_type::iterator ite = cont_.find(text);
		if (ite != cont_.end()) {
			return ite->second;
		}

		return nullptr;
	}

private:
	container_type cont_;
};

class Sym {
public:
	Sym(const std::string& text);

public:
	std::string ToString() const;

private:
	std::string value_;
};

class SymTable : public Table<Sym> {

};

class Constant {
public:
	Constant(const std::string& text);

public:
	std::string ToString() const;

private:
	int value_;
};

class ConstantTable : public Table<Constant> {

};

class Literal {
public:
	Literal(const std::string& text);

public:
	std::string ToString() const;

private:
	std::string value_;
};

class LiteralTable : public Table<Literal> {

};
