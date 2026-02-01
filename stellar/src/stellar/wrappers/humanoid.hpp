#pragma once

#ifndef HUMANOID_HPP
#define HUMANOID_HPP

#include "instance.hpp"
#include "../helpers/math.hpp"

static union convertion {
	std::uint64_t hex;
	float f;
} conv;

class Humanoid : public Instance {
public:
	float walkspeed;
	float jump_power;
	float hip_height;

	float health() {
		float last_valid_health = -1.0f;
		float health = -1.0f;

		while (true) {
			std::uint64_t first = read<std::uint64_t>(self + stellar::offsets::health);

			if (!first)
				return last_valid_health;

			std::uint64_t second = read<std::uint64_t>(first);

			conv.hex = first ^ second;
			health = conv.f;

			if (health >= 0.1f && health <= 1000.0f) {
				last_valid_health = health;
				break;
			}
			else
				health = last_valid_health;
		}

		return last_valid_health;
	};

	float max_health() {
		float last_valid_max_health = -1.0f;
		float max_health = -1.0f;

		while (true) {
			std::uint64_t first = read<std::uint64_t>(self + stellar::offsets::max_health);

			if (!first)
				return last_valid_max_health;

			std::uint64_t second = read<std::uint64_t>(first);

			conv.hex = first ^ second;
			max_health = conv.f;

			if (max_health >= 0.1f && max_health <= 1000.0f) {
				last_valid_max_health = max_health;
				break;
			}
			else
				max_health = last_valid_max_health;
		}

		return last_valid_max_health;
	};

	void set_walkspeed(float speed) {
		if (!self) return;
		
		// Write to both walkspeed offsets for compatibility and detection bypass
		write<float>(self + stellar::offsets::walkspeed, speed);
		write<float>(self + stellar::offsets::WalkSpeedCheck, speed);
	}

	float get_walkspeed() {
		if (!self) return 16.0f;
		return read<float>(self + stellar::offsets::walkspeed);
	}

	float read_walkspeed() {
		if (!self) return 16.0f;
		return read<float>(self + stellar::offsets::walkspeed);
	}

	void write_walkspeed(float speed) {
		if (!self) return;
		write<float>(self + stellar::offsets::walkspeed, speed);
		write<float>(self + stellar::offsets::WalkSpeedCheck, speed);
	}

	Vector3 get_move_dir() {
		if (!self) return Vector3();
		return read<Vector3>(self + stellar::offsets::move_direction);
	}
};

#endif