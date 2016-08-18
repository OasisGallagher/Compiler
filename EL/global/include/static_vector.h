#pragma once
#include "debug.h"

template <class Ty>
class static_vector {
public:
	typedef Ty value_type;
	typedef Ty* iterator;
	typedef const Ty* const_iterator;

public:
	static_vector(size_t capacity) {
		size_ = 0;
		capacity_ = capacity;
		cont_ = new Ty[capacity];
	}

	~static_vector() {
		delete[] cont_;
	}

public:
	iterator begin() { return cont_; }
	iterator end() { return cont_ + size_; }

	const_iterator begin() const { return cont_; }
	const_iterator end() const { return cont_ + size_; }

	size_t size() const { return size_; }
	Ty& operator[] (size_t i) { return cont_[i]; }

	void push_back(const Ty& value) {
		Assert(size_ < capacity, "out of memory");
		cont_[size_++] = value;
	}

private:
	value_type* cont_;
	size_t size_, capacity_;
};
