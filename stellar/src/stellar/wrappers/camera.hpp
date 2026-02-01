#pragma once

#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "../helpers/math.hpp"

class Camera : public Instance {
public:
	void set_rotation(Matrix3 rotation) {
		write<Matrix3>(this->self + stellar::offsets::camera_rotation, rotation);
	}

	Vector3 position() {
		return read<Vector3>(this->self + stellar::offsets::camera_pos);
	}

	Matrix3 rotation() {
		return read<Matrix3>(this->self + stellar::offsets::camera_rotation);
	}
};

#endif