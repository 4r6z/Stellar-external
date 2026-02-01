#include "globals.hpp"

namespace globals {
	DataModel game{};
	VisualEngine visual_engine{};
	Workspace workspace{};

	std::uint64_t game_id{};

	bool is_focused = false;
	HANDLE game_handle{};
	HANDLE foreground{};

	//////////////////////////////////////////

	Player aimbot_saved_player{};

	Player triggerbot_saved_player{};
	Vector3 triggerbot_position{};

	bool anti_lock = false;

	bool aimbot = false;
	bool silent = true;
	bool disable_outside_fov = false;
	bool ko_check = false;
	bool crew_check = false;
	bool knife_check = false;
	float sensitivity = 1.0f;

	bool auto_resolve = false;
	float velocity_threshold = 40.f;

	bool aimbot_range_enable = false;
	int aimbot_range = 50;

	int aimbot_type = 1;
	int aim_part = 0;

	bool prediction = false;
	bool distance_prediction = false;
	bool separate_predictions = true;
	int prediction_method = 1;

	float close_distance = 20.f;
	float mid_distance = 50.f;

	float main_prediction = 0.15f;
	float close_prediction = 0.15f;
	float mid_prediction = 0.15f;
	float far_prediction = 0.15f;

	float prediction_x = 0.15f;
	float prediction_y = 0.15f;

	float close_prediction_x = 0.15f;
	float close_prediction_y = 0.15f;

	float mid_prediction_x = 0.15f;
	float mid_prediction_y = 0.15f;

	float far_prediction_x = 0.15f;
	float far_prediction_y = 0.15f;

	bool smoothness = false;
	int smooth_type = 0;
	float smoothness_x = 0.5;
	float smoothness_y = 0.5;
	float smoothness_xy = 3;

	bool shake = false;
	int shake_type = 0;
	float shake_x = 0;
	float shake_y = 0;
	float shake_xy = 0;

	bool vsync = true;
	bool streamproof = false;

	bool esp = false;
	bool skeleton = false;

	bool box = false;
	int box_type = 0;
	bool chams = false;
	float box_color[3] = { 255, 255, 255 };

	bool armor_bar = false;
	bool health_bar = false;

	bool fov_enabled = false;
	float fov_color[3] = { 255, 255, 255 };
	int fov = 100;

	bool deadzone = false;
	float deadzone_color[3] = { 255, 255, 255 };
	float deadzone_fill_color[3] = { 255, 255, 255 };

	bool crosshair = false;

	bool speed_enabled = false;
	float speed_value = 16.0f;
	int speed_type = 0;

	bool triggerbot = false;
	bool triggerbot_shoot_on_aimbot = false;
	bool triggerbot_visualize = false;

	bool triggerbot_prediction = false;
	float triggerbot_prediction_x = 0.15f;
	float triggerbot_prediction_y = 0.15f;

	bool triggerbot_distance_prediction = false;
	float triggerbot_close_prediction_x = 0.15f;
	float triggerbot_close_prediction_y = 0.15f;

	float triggerbot_mid_prediction_x = 0.15f;
	float triggerbot_mid_prediction_y = 0.15f;

	float triggerbot_far_prediction_x = 0.15f;
	float triggerbot_far_prediction_y = 0.15f;

	float triggerbot_close_distance = 20.f;
	float triggerbot_mid_distance = 50.f;

	float triggerbot_hitbox_x = 1.f;
	float triggerbot_hitbox_y = 1.f;

	CKeybind aimbot_key{ "aimkey" };
	CKeybind triggerbot_key{ "triggerkey" };
	CKeybind speed_key{ "speedkey" };
}
