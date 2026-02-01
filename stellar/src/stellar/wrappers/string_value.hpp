#pragma once

#ifndef STRING_VALUE_HPP
#define STRING_VALUE_HPP

#include "instance.hpp"

class StringValue : public Instance {
public:
	const char* value() {
		return read<const char*>(this->self + stellar::offsets::value);
	}
};

#endif