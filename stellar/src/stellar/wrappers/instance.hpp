#pragma once

#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include "../stellar.hpp"
#include "../driver/driver.hpp"

#include <iostream>
#include <vector>
#include <cstdint>
#include <string>

static std::string readstring(std::uint64_t address) {
	if (!address)
		return std::string();

	std::string string;
	char character = 0;
	int char_size = sizeof(character);
	int offset = 0;

	string.reserve(204);

	while (offset < 200) {
		character = read<char>(address + offset);

		if (character == 0)
			break;

		offset += char_size;
		string.push_back(character);
	}

	return string;
}

static std::string readstring2(std::uint64_t string) {
	if (read<int>(string + 0x18) >= 16u)
		return readstring(read<std::uint64_t>(string));
	else
		return readstring(string);
}

class Instance {
public:
	std::uint64_t self;

	std::string name() {
		if (!self)
			return "???";

		const std::uint64_t ptr = read<std::uint64_t>(self + stellar::offsets::name);

		if (ptr)
			return readstring2(ptr);

		return "???";
	};

	std::string class_name() {
		if (!self)
			return "???";

		std::uint64_t ptr1 = read<std::uint64_t>(self + 0x18);

		if (!ptr1)
			return "???";

		std::uint64_t ptr2 = read<std::uint64_t>(ptr1 + 0x8);

		if (ptr2)
			return read<std::string>(ptr2 + 0x0);

		return "???";
	}

	template <typename T = Instance>
	std::vector<T> children() {
		std::vector<T> container;

		if (!self)
			return container;

		auto start = read<std::uint64_t>(self + stellar::offsets::children);

		if (!start)
			return container;

		auto end = read<std::uint64_t>(start + stellar::offsets::size);

		if (!end)
			return container;

		std::uint64_t instances = read<std::uint64_t>(start);

		if (!instances)
			return container;

		for (auto inst = instances; inst != end; inst += 16) {
			if (inst > 1099511627776 && inst < 3298534883328) { 
				T new_instance = read<T>(inst);

				if (!new_instance)
					continue;

				container.emplace_back(new_instance);
			}
			else
				break;
		}

		return container;
	}

	template <typename T = Instance>
	T find_first_child(std::string child_name) {
		if (!this->self)
			return T();

		std::vector<T> children_array = this->children<T>();

		if (children_array.empty())
			return T();

		T child{};

		for (T obj : children_array) {
			if (!obj.self)
				continue;

			std::string obj_name = obj.name();

			if (obj_name == "" || obj_name == "???")
				continue;

			if (obj_name == child_name) {
				child = static_cast<T>(obj);
				break;
			}
		}

		if (!child)
			return T();

		return child;
	}

	template <typename T = Instance>
	T find_first_child_which_is_a(std::string name_class) {
		if (!this->self)
			return T();

		std::vector<T> children_array = this->children<T>();

		if (children_array.empty())
			return T();

		T child{};

		for (T obj : children_array) {
			if (!obj.self)
				continue;

			std::string obj_class = obj.class_name();

			if (obj_class == "" || obj_class == "???")
				continue;

			if (obj_class == name_class) {
				child = static_cast<T>(obj);
				break;
			}
		}

		if (!child)
			return T();

		return child;
	}

	bool operator!() const {
		return this->self == 0;
	}

	bool operator==(const Instance& other) const {
		return other.self == this->self;
	}
};

#endif