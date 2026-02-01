#pragma once

#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include "../../wrappers/datamodel.hpp"
#include "../../wrappers/visual_engine.hpp"
#include "../../wrappers/workspace.hpp"
#include "../../wrappers/keybind.hpp"

namespace globals {
	extern DataModel game;
	extern VisualEngine visual_engine;
	extern Workspace workspace;

	extern std::uint64_t game_id;

	extern bool is_focused;
	extern HANDLE game_handle;
	extern HANDLE foreground;

	//////////////////////////////////////////

	extern Player aimbot_saved_player;

	extern Player triggerbot_saved_player;
	extern Vector3 triggerbot_position;

	extern bool anti_lock;

	extern bool aimbot;
	extern bool silent;
	extern bool disable_outside_fov;
	extern bool ko_check;
	extern bool crew_check;
	extern bool knife_check;
	extern float sensitivity;

	extern bool auto_resolve;
	extern float velocity_threshold;

	extern bool aimbot_range_enable;
	extern int aimbot_range;

	extern int aimbot_type;
	extern int aim_part;

	extern bool prediction;
	extern bool distance_prediction;
	extern bool separate_predictions;
	extern int prediction_method;

	extern float close_distance;
	extern float mid_distance;

	extern float main_prediction;
	extern float close_prediction;
	extern float mid_prediction;
	extern float far_prediction;

	extern float prediction_x;
	extern float prediction_y;

	extern float close_prediction_x;
	extern float close_prediction_y;

	extern float mid_prediction_x;
	extern float mid_prediction_y;

	extern float far_prediction_x;
	extern float far_prediction_y;

	extern bool smoothness;
	extern int smooth_type;
	extern float smoothness_x;
	extern float smoothness_y;
	extern float smoothness_xy;

	extern bool shake;
	extern int shake_type;
	extern float shake_x;
	extern float shake_y;
	extern float shake_xy;

	extern bool vsync;
	extern bool streamproof;

	extern bool esp;
	extern bool skeleton;

	extern bool box;
	extern int box_type;
	extern float box_color[3];

	extern bool armor_bar;
	extern bool health_bar;

	extern bool fov_enabled;
	extern float fov_color[3];
	extern bool chams;
	extern int fov;

	extern bool deadzone;
	extern float deadzone_color[3];
	extern float deadzone_fill_color[3];

	extern bool crosshair;

	extern bool speed_enabled;
	extern float speed_value;
	extern int speed_type;

	extern bool triggerbot;
	extern bool triggerbot_shoot_on_aimbot;
	extern bool triggerbot_visualize;

	extern bool triggerbot_prediction;
	extern float triggerbot_prediction_x;
	extern float triggerbot_prediction_y;

	extern bool triggerbot_distance_prediction;
	extern float triggerbot_close_prediction_x;
	extern float triggerbot_close_prediction_y;

	extern float triggerbot_mid_prediction_x;
	extern float triggerbot_mid_prediction_y;

	extern float triggerbot_far_prediction_x;
	extern float triggerbot_far_prediction_y;

	extern float triggerbot_close_distance;
	extern float triggerbot_mid_distance;

	extern float triggerbot_hitbox_x;
	extern float triggerbot_hitbox_y;

	extern CKeybind aimbot_key;
	extern CKeybind triggerbot_key;
	extern CKeybind speed_key;
}

#endif
