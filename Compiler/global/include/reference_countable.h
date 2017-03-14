#pragma once

class RefCount {
public:
	RefCount();
	RefCount(const RefCount& other);

	~RefCount();

public:
	bool Reattach(const RefCount& other);
	bool IsOnly() const;

private:
	RefCount& operator = (const RefCount& other);

private:
	int* count_;
};

#define IMPLEMENT_REFERENCE_COUNTABLE(Wrapper, Impl)	\
	public: \
		Wrapper(Impl* impl) : ptr_(impl) { } \
		~Wrapper() { \
			if (counter_.IsOnly()) { \
				delete ptr_; \
			} \
		} \
		Wrapper& operator = (const Wrapper& other) { \
			if (counter_.Reattach(other.counter_)) { \
				delete ptr_; \
										} \
			ptr_ = other.ptr_; \
			return *this;\
		} \
	private: \
		RefCount counter_; \
		Impl* ptr_
