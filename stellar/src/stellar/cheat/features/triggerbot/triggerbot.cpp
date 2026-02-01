#include "triggerbot.hpp"

#include <Windows.h>
#include <chrono>
#include <thread>
#include <vector>
#include <iostream> 

#include "../../../helpers/math.hpp"
#include "../../globals/globals.hpp"
#include "../../../helpers/world.hpp"

void click() {
    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}

void stellar::cheats::triggerbot() {
    bool locked = false;
    Player target{};

    Vector3 resolved_velocity{};

    std::thread velocity_calc([&]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));

            if (!globals::is_focused || !globals::triggerbot || !target.self || !globals::auto_resolve || !target.static_root_part.self) {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                continue;
            }
            Vector3 old_position = target.static_root_part.position();
            std::this_thread::sleep_for(std::chrono::milliseconds(85));
            Vector3 new_position = target.static_root_part.position();

            resolved_velocity = (new_position - old_position) / 0.02f;
        }
        });

    while (true) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));

        if (!globals::is_focused || !globals::triggerbot) {
            if (locked || globals::triggerbot_saved_player.self || target.self) {
                globals::triggerbot_key.enabled = false;
                locked = false;
                target = {};
                globals::triggerbot_saved_player = {};
            }
            continue;
        }

        if (globals::knife_check && globals::game.players.local_player().character().find_first_child("[Knife]").self != 0) {
            continue;
        }

        globals::triggerbot_key.update();

        if (!globals::triggerbot_key.enabled) {
            if (locked || globals::triggerbot_saved_player.self || target.self) {
                globals::triggerbot_key.enabled = false;
                locked = false;
                target = {};
                globals::triggerbot_saved_player = {};
            }
            continue;
        }

        if (!globals::aimbot_saved_player) {
            if (locked && globals::triggerbot_saved_player.self)
                target = globals::triggerbot_saved_player;
            else if (!locked) {
                target = get_closest_player();
                locked = true;
            }
        }
        else {
            target = globals::aimbot_saved_player;
            locked = true;
        }

        if (!target.self || !target.static_character.self || (globals::ko_check && target.ko_path.value())) {
            if (locked || globals::triggerbot_saved_player.self || target.self) {
                globals::triggerbot_key.enabled = false;
                locked = false;
                target = {};
                globals::triggerbot_saved_player = {};
            }
            continue;
        }

        globals::triggerbot_saved_player = target;
        Vector3 part_pos_3d = target.static_root_part.position();

        if (part_pos_3d == 0 || part_pos_3d == get_local_position()) {
            if (locked || globals::triggerbot_saved_player.self || target.self) {
                globals::triggerbot_key.enabled = false;
                locked = false;
                target = {};
                globals::triggerbot_saved_player = {};
            }
            continue;
        }

        if (globals::triggerbot_prediction) {
            Vector3 velocity = globals::auto_resolve ? resolved_velocity : target.static_root_part.velocity();

            float prediction_x = globals::triggerbot_prediction_x;
            float prediction_y = globals::triggerbot_prediction_y;
            float prediction_z = globals::triggerbot_prediction_x;

            if (globals::triggerbot_distance_prediction) {
                Player local_player = player_list.front();

                if (local_player.self && local_player.static_root_part.self) {
                    Vector3 local_player_position = local_player.static_root_part.position();

                    float distance = calculate_distance(local_player_position, part_pos_3d);

                    if (distance <= globals::close_distance) {
                        prediction_x = globals::triggerbot_close_prediction_x;
                        prediction_y = globals::triggerbot_close_prediction_y;
                        prediction_z = globals::triggerbot_close_prediction_x;
                    }
                    else if (distance <= globals::mid_distance) {
                        prediction_x = globals::triggerbot_mid_prediction_x;
                        prediction_y = globals::triggerbot_mid_prediction_y;
                        prediction_z = globals::triggerbot_mid_prediction_x;
                    }
                    else {
                        prediction_x = globals::triggerbot_far_prediction_x;
                        prediction_y = globals::triggerbot_far_prediction_y;
                        prediction_z = globals::triggerbot_far_prediction_x;
                    }
                }
            }

            if (globals::prediction_method == 0)
                velocity = velocity / Vector3{ prediction_x, prediction_y, prediction_z };
            else
                velocity = velocity * Vector3{ prediction_x, prediction_y, prediction_z };

            part_pos_3d = part_pos_3d + velocity;
        }

        globals::triggerbot_position = part_pos_3d;
        auto [part_pos_2d, on_screen] = world_to_screen(part_pos_3d);

        if (!on_screen) {
            continue;
        }

        POINT cursor_point;
        GetCursorPos(&cursor_point);
        ScreenToClient((HWND)globals::game_handle, &cursor_point);

        std::vector<Vector3> cube_vertices = get_corners(part_pos_3d, { globals::triggerbot_hitbox_x, globals::triggerbot_hitbox_y, globals::triggerbot_hitbox_x });

        std::vector<Vector2> cube_2d_vertices;
        bool all_vertices_on_screen = true;
        for (const auto& vertex : cube_vertices) {
            auto [screen_pos, vertex_on_screen] = world_to_screen(vertex);
            if (!vertex_on_screen) {
                all_vertices_on_screen = false;
                break;
            }
            cube_2d_vertices.push_back(screen_pos);
        }

        if (!all_vertices_on_screen) {
            continue;
        }

        std::vector<std::vector<Vector2>> faces = {
            { cube_2d_vertices[0], cube_2d_vertices[1], cube_2d_vertices[3], cube_2d_vertices[2] }, // top
            { cube_2d_vertices[4], cube_2d_vertices[5], cube_2d_vertices[7], cube_2d_vertices[6] }, // bottom
            { cube_2d_vertices[0], cube_2d_vertices[1], cube_2d_vertices[5], cube_2d_vertices[4] }, // front
            { cube_2d_vertices[2], cube_2d_vertices[3], cube_2d_vertices[7], cube_2d_vertices[6] }, // back
            { cube_2d_vertices[0], cube_2d_vertices[2], cube_2d_vertices[6], cube_2d_vertices[4] }, // left
            { cube_2d_vertices[1], cube_2d_vertices[3], cube_2d_vertices[7], cube_2d_vertices[5] }  // right
        };

        bool inside = false;
        for (const auto& face : faces) {
            if (is_point_in_polygon(face, { static_cast<float>(cursor_point.x), static_cast<float>(cursor_point.y) })) {
                inside = true;
                break;
            }
        }

        locked = true;

        if (inside && !(globals::knife_check && globals::game.players.local_player().character().find_first_child("[Knife]").self != 0)) {
            click();
        }
    }
}