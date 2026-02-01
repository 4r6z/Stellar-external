#pragma once

#ifndef VISUAL_ENGINE_HPP
#define VISUAL_ENGINE_HPP

#include "../helpers/math.hpp"
#include "../stellar.hpp"
#include "../driver/driver.hpp"

#include <cstdint>

class VisualEngine {
public:
	std::uint64_t self;

	Vector2 dimensions() {
		if (!self)
			return Vector2();

		return read<Vector2>(self + stellar::offsets::dimensions);
	};

	Matrix4 view_matrix() {
		if (!self)
			return Matrix4();

		return read<Matrix4>(self + stellar::offsets::viewmatrix);
	};
};

#endif