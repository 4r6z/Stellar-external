#pragma once

#ifndef BOOL_VALUE_HPP
#define BOOL_VALUE_HPP

#include "instance.hpp"

class BoolValue : public Instance {
public:
	bool value() {
		return read<bool>(this->self + stellar::offsets::value);
	}
};

#endif