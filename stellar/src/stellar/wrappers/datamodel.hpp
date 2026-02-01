#pragma once

#ifndef DATAMODEL_HPP
#define DATAMODEL_HPP

#include "instance.hpp"
#include "service.hpp"

class DataModel : public Instance {
public:
	Service players;

	std::uint64_t game_id() {
		return read<std::uint64_t>(self + stellar::offsets::GameId);
	}
};

#endif