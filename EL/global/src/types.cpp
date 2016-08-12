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

FileReader::FileReader(const char* path) {
	ifs_.open(path);
}

FileReader::~FileReader() {
	ifs_.close();
}

bool FileReader::ReadLine(char* buffer, size_t length) {
	return !!ifs_.getline(buffer, length);
}