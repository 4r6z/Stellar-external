#pragma once

#ifndef INT_VALUE_HPP
#define INT_VALUE_HPP

#include "instance.hpp"

class IntValue : public Instance {
public:
	float value() {
		return read<int>(this->self + stellar::offsets::value);
	}
};

#endif