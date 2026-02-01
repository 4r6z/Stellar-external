#include "anti_lock.hpp"
#include "../../../helpers/math.hpp"
#include "../../globals/globals.hpp"
#include "../../../helpers/world.hpp"
#include <cstdlib>
#include <cmath>
#include <chrono>
#include <thread>

void stellar::cheats::anti_lock() {
    Vector3 old_velocity{};
    Vector3 move_direction{};
    Vector3 new_position{};
    Vector3 current_position{};

    float mwmult = 100.0f;
    float smoothing_factor = 0.1f; // Adjust this value for more or less smoothing
    std::chrono::milliseconds sleep_duration(10); // Adjust this for the update frequency

    while (true) {
        std::this_thread::sleep_for(sleep_duration);

        if (!globals::is_focused || !globals::anti_lock) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        Player local_player = player_list.front();

        if (!local_player.self) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        Part humanoid_root_part = local_player.character().find_first_child<Part>("HumanoidRootPart");
        current_position = humanoid_root_part.position();
        move_direction = humanoid_root_part.move_direction();
        new_position = current_position + (move_direction * -mwmult);

        // Smooth movement towards the new position
        Vector3 smoothed_position = current_position * (1 - smoothing_factor) + new_position * smoothing_factor;
        humanoid_root_part.set_part_pos(smoothed_position);
    }
}

void main_loop() {
    while (true) {
        auto start = std::chrono::high_resolution_clock::now();
        stellar::cheats::anti_lock();
        auto end = start + std::chrono::microseconds(1000); // Adjust this value to control the loop frequency
        std::this_thread::sleep_until(end);
    }
}
