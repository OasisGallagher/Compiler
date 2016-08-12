#pragma once
#include <cassert>

#define Assert(_Expression, _Message)	\
	(void)((!!(_Expression)) || (_wassert(_CRT_WIDE(#_Expression) _CRT_WIDE(": ") _CRT_WIDE(_Message), _CRT_WIDE(__FILE__), __LINE__), 0))
