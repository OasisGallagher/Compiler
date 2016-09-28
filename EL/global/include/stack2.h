#pragma once

#include <vector>

template <class Ty>
class Stack {
public:
	typedef std::vector<Ty> container_type;
	typedef typename container_type::value_type value_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::reference reference;
	typedef typename container_type::const_reference const_reference;

public:
	void push(const value_type& val) {
		cont_.push_back(val);
	}

	reference top(int offset = 0) {
		int size = (int)cont_.size();
		return cont_[size - offset - 1];
	}

	const_reference top(int offset = 0) const {
		int size = (int)cont_.size();
		return cont_[size - offset - 1];
	}

	void pop(int count = 1) {
		for (; count > 0; --count) {
			cont_.pop_back();
		}
	}

private:
	container_type cont_;
};
