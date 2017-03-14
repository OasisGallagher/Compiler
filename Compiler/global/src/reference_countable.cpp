#include "reference_countable.h"

RefCount::RefCount() {
	count_ = new int(1);
}

RefCount::RefCount(const RefCount& other) {
	count_ = other.count_;
	++*count_;
}

RefCount::~RefCount() {
	if (--*count_ == 0) {
		delete count_;
	}
}

bool RefCount::Reattach(const RefCount& other) {
	++*other.count_;
	if (--*count_ == 0) {
		delete count_;
		count_ = other.count_;
		return true;
	}

	count_ = other.count_;
	return false;
}

bool RefCount::IsOnly() const {
	return *count_ == 1;
}
