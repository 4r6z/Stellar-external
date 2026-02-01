#pragma once

#ifndef NUMBER_VALUE_HPP
#define NUMBER_VALUE_HPP

#include "instance.hpp"

class NumberValue : public Instance {
public:
	double value() {
		return read<double>(this->self + stellar::offsets::value);
	}
};

#endif