#include "aimbot.hpp"
#include <Windows.h>
#include <chrono>
#include <thread>
#include "../../../helpers/math.hpp"
#include "../../globals/globals.hpp"
#include "../../../helpers/world.hpp"
#include "../../../helpers/tween.hpp"
using namespace std::chrono_literals;

void stellar::cheats::aimbot() {
    bool locked = false;
    Player target{};
    Part body_part{};
    Vector3 resolved_velocity{};

    // Cache for better performance
    static auto last_velocity_update = std::chrono::steady_clock::now();
    static Vector3 cached_velocity{};

    std::thread velocity_calc([&]() {
        while (true) {
            if (!globals::is_focused || !globals::aimbot || !target.self || !globals::auto_resolve || !target.static_root_part.self) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Increased sleep for better performance
                continue;
            }

            auto current_time = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_velocity_update).count() > 100) {
                Vector3 old_position = target.static_root_part.position();
                std::this_thread::sleep_for(std::chrono::milliseconds(85));
                Vector3 new_position = target.static_root_part.position();

                resolved_velocity = (new_position - old_position) / 0.05f;
                last_velocity_update = current_time;
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
        });

    while (true) {
        if (!globals::is_focused || !globals::aimbot) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Reduced from 1ms
            if (locked || globals::aimbot_saved_player.self || body_part.self) {
                globals::aimbot_key.enabled = false;
                locked = false;
                globals::aimbot_saved_player = {};
                body_part = {};
            }
            continue;
        }

        globals::aimbot_key.update();

        if (!globals::aimbot_key.enabled) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            if (locked || globals::aimbot_saved_player.self || body_part.self) {
                globals::aimbot_key.enabled = false;
                locked = false;
                globals::aimbot_saved_player = {};
                body_part = {};
            }
            continue;
        }

        if (locked && globals::aimbot_saved_player.self)
            target = globals::aimbot_saved_player;
        else if (!locked) {
            target = get_closest_player();
            locked = true;
        }

        if (!target.self || !target.static_character.self || (globals::ko_check && target.ko_path.value())) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            if (locked || globals::aimbot_saved_player.self || body_part.self) {
                globals::aimbot_key.enabled = false;
                locked = false;
                globals::aimbot_saved_player = {};
                body_part = {};
            }
            continue;
        }

        if (!body_part.self) {
            switch (globals::aim_part) {
            case 0:
                body_part = target.static_head;
                break;
            case 1:
                body_part = target.static_character.find_first_child<Part>("UpperTorso");
                break;
            case 2:
                body_part = target.static_root_part;
                break;
            case 3:
                body_part = target.static_character.find_first_child<Part>("LowerTorso");
                break;
            default:
                body_part = target.static_head;
                break;
            }
        }

        Vector3 part_pos_3d = body_part.position();
        if (part_pos_3d == Vector3{ 0, 0, 0 }) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            if (locked || globals::aimbot_saved_player.self || body_part.self) {
                globals::aimbot_key.enabled = false;
                locked = false;
                globals::aimbot_saved_player = {};
                body_part = {};
            }
            continue;
        }

        if (globals::prediction) {
            Vector3 velocity = globals::auto_resolve ? resolved_velocity : target.static_root_part.velocity();

            float prediction_x = std::clamp(globals::separate_predictions ? globals::prediction_x : globals::main_prediction, 0.1f, 10.0f);
            float prediction_y = std::clamp(globals::separate_predictions ? globals::prediction_y : globals::main_prediction, 0.1f, 10.0f);
            float prediction_z = std::clamp(globals::separate_predictions ? globals::prediction_x : globals::main_prediction, 0.1f, 10.0f);

            if (globals::distance_prediction) {
                Player local_player = player_list.front();

                if (local_player.self && local_player.static_root_part.self) {
                    Vector3 local_player_position = local_player.static_root_part.position();

                    float distance = calculate_distance(local_player_position, part_pos_3d);

                    if (distance <= globals::close_distance) {
                        prediction_x = std::clamp(globals::separate_predictions ? globals::close_prediction_x : globals::close_prediction, 0.1f, 10.0f);
                        prediction_y = std::clamp(globals::separate_predictions ? globals::close_prediction_y : globals::close_prediction, 0.1f, 10.0f);
                        prediction_z = std::clamp(globals::separate_predictions ? globals::close_prediction_x : globals::close_prediction, 0.1f, 10.0f);
                    }
                    else if (distance <= globals::mid_distance) {
                        prediction_x = std::clamp(globals::separate_predictions ? globals::mid_prediction_x : globals::mid_prediction, 0.1f, 10.0f);
                        prediction_y = std::clamp(globals::separate_predictions ? globals::mid_prediction_y : globals::mid_prediction, 0.1f, 10.0f);
                        prediction_z = std::clamp(globals::separate_predictions ? globals::mid_prediction_x : globals::mid_prediction, 0.1f, 10.0f);
                    }
                    else {
                        prediction_x = std::clamp(globals::separate_predictions ? globals::far_prediction_x : globals::far_prediction, 0.1f, 10.0f);
                        prediction_y = std::clamp(globals::separate_predictions ? globals::far_prediction_y : globals::far_prediction, 0.1f, 10.0f);
                        prediction_z = std::clamp(globals::separate_predictions ? globals::far_prediction_x : globals::far_prediction, 0.1f, 10.0f);
                    }
                }
            }

            if (globals::prediction_method == 0)
                velocity = velocity / Vector3{ prediction_x, prediction_y, prediction_z };
            else
                velocity = velocity * Vector3{ prediction_x, prediction_y, prediction_z };

            part_pos_3d = part_pos_3d + velocity;
        }

        auto [part_pos_2d, on_screen] = world_to_screen(part_pos_3d);

        if (!on_screen) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }


        POINT cursor_point;
        GetCursorPos(&cursor_point);
        ScreenToClient((HWND)globals::game_handle, &cursor_point);

        float distance = calculate_distance(part_pos_2d, { static_cast<float>(cursor_point.x), static_cast<float>(cursor_point.y) });

        if (!globals::disable_outside_fov || distance <= globals::fov) {
            if (globals::aimbot_type == 0) {
                Vector2 relative = { 0, 0 };

                relative.x = (part_pos_2d.x - cursor_point.x) * globals::sensitivity / (globals::smoothness ? globals::smoothness_x : 1);
                relative.y = (part_pos_2d.y - cursor_point.y) * globals::sensitivity / (globals::smoothness ? globals::smoothness_y : 1);

                INPUT input{};
                input.mi.time = 0;
                input.type = INPUT_MOUSE;
                input.mi.mouseData = 0;
                input.mi.dx = static_cast<LONG>(relative.x);
                input.mi.dy = static_cast<LONG>(relative.y);
                input.mi.dwFlags = MOUSEEVENTF_MOVE;
                SendInput(1, &input, sizeof(input));

                locked = true;
                globals::aimbot_saved_player = target;

                std::this_thread::sleep_for(std::chrono::milliseconds(2)); // Slightly increased
            }
            else if (globals::aimbot_type == 1) {
                Camera camera = globals::workspace.get_camera();
                Vector3 camera_pos = camera.position();
                Matrix3 camera_rot = camera.rotation();

                if (target.self && target.static_character.self) {
                    Matrix3 hit_matrix = look_at_to_matrix(camera_pos, part_pos_3d);
                    if (globals::smoothness)
                        hit_matrix = lerp_matrix(camera_rot, hit_matrix, globals::smoothness_xy / 700, tween::back);
                    camera.set_rotation(hit_matrix);

                    locked = true;
                    globals::aimbot_saved_player = target;

                    auto start = std::chrono::high_resolution_clock::now();
                    auto end = start + std::chrono::microseconds(150);
                    while (std::chrono::high_resolution_clock::now() < end) {}
                }
            }
        }
    }
}