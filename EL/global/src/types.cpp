#include "types.h"
#include "debug.h"

RefCountable::RefCountable() 
	: refCount_(1) {
}

unsigned RefCountable::IncRefCount() {
	return ++refCount_;
}

unsigned RefCountable::DecRefCount() {
	Assert(refCount_ > 0, "can't decrease reference count");
	return --refCount_;
}
