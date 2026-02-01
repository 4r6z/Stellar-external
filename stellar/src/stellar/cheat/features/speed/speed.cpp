#include "speed.hpp"
#include "../../globals/globals.hpp"
#include "../../threads/threads.hpp"
#include "../../../helpers/math.hpp"
#include "../../../stellar.hpp"
#include "../../../driver/driver.hpp"
#include "../../../wrappers/player.hpp"
#include <chrono>
#include <thread>
#include <vector>
#include <windows.h>

#pragma comment(lib, "winmm.lib")

void stellar::cheats::speed() {
	while (true) {
		if (player_list.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			continue;
		}

		globals::speed_key.update();
		
		if (globals::speed_enabled && globals::speed_key.enabled) {
			Player& local_player = player_list[0];
			
			if (!local_player.self || !local_player.static_humanoid.self || !local_player.static_root_part.self) {
				continue;
			}

			switch (globals::speed_type) {
			case 0: // Walkspeed - Fast and aggressive method
				{
					// Continuously set walkspeed with high iteration count for maximum effectiveness
					for (int i = 0; i < 50000; i++) {
						local_player.static_humanoid.write_walkspeed(globals::speed_value);
					}
				}
				break;
				
			case 1: // Velocity - Medium risk method
				{
					Vector3 dir = local_player.static_humanoid.get_move_dir();
					Vector3 current_vel = local_player.static_root_part.get_velocity();
					
					// Check if player is moving
					if (dir.length() > 0.1f) {
						Vector3 normalized_dir = dir.normalize();
						
						// Apply speed with high iteration count to override game resets
						for (int i = 0; i < 10000; i++) {
							local_player.static_root_part.write_velocity(Vector3{
								normalized_dir.x * globals::speed_value,
								current_vel.y, // Preserve Y velocity for jumping/falling
								normalized_dir.z * globals::speed_value
							});
						}
					}
				}
				break;
			}
		}

		// High-performance timing system (260 FPS)
		static LARGE_INTEGER frequency;
		static LARGE_INTEGER lastTime;
		static bool timeInitialized = false;

		if (!timeInitialized) {
			QueryPerformanceFrequency(&frequency);
			QueryPerformanceCounter(&lastTime);
			timeBeginPeriod(1);
			timeInitialized = true;
		}

		const double targetFrameTime = 1.0 / 260.0;

		LARGE_INTEGER currentTime;
		QueryPerformanceCounter(&currentTime);
		double elapsedSeconds = static_cast<double>(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;

		if (elapsedSeconds < targetFrameTime) {
			DWORD sleepMilliseconds = static_cast<DWORD>((targetFrameTime - elapsedSeconds) * 1000.0);
			if (sleepMilliseconds > 0) {
				Sleep(sleepMilliseconds);
			}
		}

		QueryPerformanceCounter(&lastTime);
		
		// Small sleep to prevent excessive CPU usage when speed is disabled
		if (!globals::speed_enabled || !globals::speed_key.enabled) {
			std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS when idle
		}
	}
}