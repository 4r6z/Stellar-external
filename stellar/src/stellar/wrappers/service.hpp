#pragma once

#ifndef SERVICE_HPP
#define SERVICE_HPP

#include "player.hpp"
#include "instance.hpp"

class Service : public Instance {
public:
	int read_players() {
		return read<int>(self);
	}

	Player local_player() {
		if (!self)
			return Player();

		return read<Player>(self + stellar::offsets::local_player);
	};
};

#endif