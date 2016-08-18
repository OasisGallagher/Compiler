#pragma once
#include "debug.h"

class RefCountable {
protected:
	RefCountable() : refCount_(1) {
	}

	virtual ~RefCountable() {

	}

	unsigned IncRefCount() {
		return ++refCount_;
	}

	unsigned DecRefCount() {
		Assert(refCount_ > 0, "can't decrease reference count");
		return --refCount_;
	}

private:
	unsigned refCount_;
};
