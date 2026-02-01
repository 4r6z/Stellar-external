#pragma once

#ifndef THREADS_HPP
#define THREADS_HPP

#include <vector>
#include <thread>

#include "../../wrappers/player.hpp"

extern std::vector<int> players_val;
extern int mostfreq;
extern std::vector<Player> player_list;
extern std::vector<Player> npc_list;

int find_most_frequent(const std::vector<int>& nums);
void update_npc_cache();

template <typename T>
void sync_lists(std::vector<T>& names1, std::vector<T>& names2);

void update_thread();
void player_thread();


#endif