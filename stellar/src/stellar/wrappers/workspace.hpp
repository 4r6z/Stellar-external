#pragma once

#ifndef WORKSPACE_HPP
#define WORKSPACE_HPP

#include "instance.hpp"
#include "camera.hpp"
#include "../helpers/math.hpp"

class Workspace : public Instance {
public:
	Camera get_camera() {
		return read<Camera>(this->self + stellar::offsets::camera);
	}
};

#endif