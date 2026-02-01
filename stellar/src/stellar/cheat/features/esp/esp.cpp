#include "esp.hpp"

#include "../../../wrappers/player.hpp"
#include "../../../wrappers/int_value.hpp"
#include "../../overlay/imgui/imgui.h"
#include "../../threads/threads.hpp"
#include "../../globals/globals.hpp"
#include "../../../helpers/world.hpp"
#include "../../../helpers/console.hpp"

void stellar::cheats::esp() {
	auto draw = ImGui::GetBackgroundDrawList();

	POINT cursor_point;
	GetCursorPos(&cursor_point);
	ScreenToClient((HWND)globals::game_handle, &cursor_point);

	if (globals::fov_enabled) {
		ImVec4 color = ImVec4(globals::fov_color[0], globals::fov_color[1], globals::fov_color[2], 1.0f);
		draw->AddCircle(ImVec2(cursor_point.x, cursor_point.y), globals::fov, ImGui::ColorConvertFloat4ToU32(color), 126, 3.5f);
	}


	if (globals::esp) {
		if (globals::triggerbot && globals::triggerbot_saved_player.self && globals::triggerbot_visualize) {
			Vector3 hrp_pos = globals::triggerbot_saved_player.static_root_part.position();

			if (globals::triggerbot_prediction) {
				Vector3 velocity = globals::triggerbot_saved_player.static_root_part.velocity();

				float prediction_x = globals::triggerbot_prediction_x;
				float prediction_y = globals::triggerbot_prediction_y;
				float prediction_z = globals::triggerbot_prediction_x;

				if (globals::triggerbot_distance_prediction) {
					Player local_player = player_list.front();

					if (local_player.self && local_player.static_root_part.self) {
						Vector3 local_player_position = local_player.static_root_part.position();

						float distance = calculate_distance(local_player_position, hrp_pos);

						if (distance <= globals::triggerbot_close_distance) {
							prediction_x = globals::triggerbot_close_prediction_x;
							prediction_y = globals::triggerbot_close_prediction_y;
							prediction_z = globals::triggerbot_close_prediction_x;
						}
						else if (distance <= globals::triggerbot_mid_distance) {
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

				hrp_pos = hrp_pos + velocity;
			}
			std::vector<Vector3> cube_vertices = get_corners(hrp_pos, { globals::triggerbot_hitbox_x, globals::triggerbot_hitbox_y, globals::triggerbot_hitbox_x });
			Vector2 CubeVertice1 = world_to_screen(cube_vertices[0]).position;
			Vector2 CubeVertice2 = world_to_screen(cube_vertices[1]).position;
			Vector2 CubeVertice3 = world_to_screen(cube_vertices[2]).position;
			Vector2 CubeVertice4 = world_to_screen(cube_vertices[3]).position;
			if (CubeVertice1.x != -1 && CubeVertice2.x != -1 && CubeVertice3.x != -1 && CubeVertice4.x != -1) {
				Vector2 CubeVertice5 = world_to_screen(cube_vertices[4]).position;
				Vector2 CubeVertice6 = world_to_screen(cube_vertices[5]).position;
				Vector2 CubeVertice7 = world_to_screen(cube_vertices[6]).position;
				Vector2 CubeVertice8 = world_to_screen(cube_vertices[7]).position;
				if (CubeVertice5.x != -1 && CubeVertice6.x != -1 && CubeVertice7.x != -1 && CubeVertice8.x != -1) {
					ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(0.f, 1.f, 1.f, 1.f)); 
					draw->AddLine(ImVec2(CubeVertice1.x, CubeVertice1.y), ImVec2(CubeVertice2.x, CubeVertice2.y), color, 1.5f);
					draw->AddLine(ImVec2(CubeVertice2.x, CubeVertice2.y), ImVec2(CubeVertice6.x, CubeVertice6.y), color, 1.5f);
					draw->AddLine(ImVec2(CubeVertice6.x, CubeVertice6.y), ImVec2(CubeVertice5.x, CubeVertice5.y), color, 1.5f);
					draw->AddLine(ImVec2(CubeVertice5.x, CubeVertice5.y), ImVec2(CubeVertice1.x, CubeVertice1.y), color, 1.5f);

					draw->AddLine(ImVec2(CubeVertice1.x, CubeVertice1.y), ImVec2(CubeVertice3.x, CubeVertice3.y), color, 1.5f);
					draw->AddLine(ImVec2(CubeVertice2.x, CubeVertice2.y), ImVec2(CubeVertice4.x, CubeVertice4.y), color, 1.5f);
					draw->AddLine(ImVec2(CubeVertice6.x, CubeVertice6.y), ImVec2(CubeVertice8.x, CubeVertice8.y), color, 1.5f);
					draw->AddLine(ImVec2(CubeVertice5.x, CubeVertice5.y), ImVec2(CubeVertice7.x, CubeVertice7.y), color, 1.5f);

					draw->AddLine(ImVec2(CubeVertice3.x, CubeVertice3.y), ImVec2(CubeVertice4.x, CubeVertice4.y), color, 1.5f);
					draw->AddLine(ImVec2(CubeVertice4.x, CubeVertice4.y), ImVec2(CubeVertice8.x, CubeVertice8.y), color, 1.5f);
					draw->AddLine(ImVec2(CubeVertice8.x, CubeVertice8.y), ImVec2(CubeVertice7.x, CubeVertice7.y), color, 1.5f);
					draw->AddLine(ImVec2(CubeVertice7.x, CubeVertice7.y), ImVec2(CubeVertice3.x, CubeVertice3.y), color, 1.5f);
				}
			}
		}
	}


		for (Player& player : player_list) {
			if (player.self && player.self != globals::game.players.local_player().self && player.static_character.self) {
				if (player.health <= 1 || (globals::crew_check && player_list.front().crew && player.crew == player_list.front().crew))
					continue;

				if (player.static_root_part.self) {
					Vector3 torso_pos3d = player.static_root_part.position();
					if (torso_pos3d == 0)
						continue;

					auto [torso_pos, torso_on_screen] = world_to_screen(torso_pos3d);

					if (torso_on_screen && torso_pos3d != get_local_position()) {
						Vector3 head_pos3d = player.static_head.position();
						if (head_pos3d == 0)
							continue;

						auto [head_pos, head_on_screen] = world_to_screen(head_pos3d - Vector3{ 0, 2, 0 });

						if (head_on_screen) {
							Vector3 foot_pos3d = head_pos3d - Vector3{ 0, 5, 0 };
							auto [foot_pos, foot_on_screen] = world_to_screen(foot_pos3d);

							if (foot_on_screen) {
								float height = head_pos.y - foot_pos.y;
								float width = height / 1.5f;

								if ((foot_pos.x - width) > 1800 || (torso_pos.x + width) > 1800 || foot_pos.y > 1800 || (torso_pos.y + height) > 1800)
									continue;

								if (globals::box) {
									if (globals::box_type == 0)
										draw->AddRect(ImVec2(foot_pos.x - width, foot_pos.y), ImVec2(torso_pos.x + width, torso_pos.y + height), ImGui::ColorConvertFloat4ToU32(ImVec4(globals::box_color[0], globals::box_color[1], globals::box_color[2], 1.f)), 0.f, 0, 2.f);									else if (globals::box_type == 1 || globals::box_type == 2 || globals::box_type == 3 || globals::box_type == 4) {
										Vector3 box_size{};
										if (globals::box_type == 1)
											box_size = { 2, 3.25, 0 };
										else
											box_size = { 2, 3.25, 2 };

										std::vector<Vector3> cube_vertices = get_corners(torso_pos3d, box_size);

										if (globals::box_type == 1 || globals::box_type == 3) {
											Matrix3 hrp_rotation_matrix = player.static_root_part.rotation();
											Vector3 hrp_rotation = { -hrp_rotation_matrix.data[2] / 100, -hrp_rotation_matrix.data[5] / 100, -hrp_rotation_matrix.data[8] / 100 };
											Vector3 look_direction = hrp_rotation.normalize();
											Vector3 side_vector1 = Vector3(0.0f, 1.0f, 0.0f).cross_product(look_direction);

											for (int i = 0; i < 8; ++i) {
												Vector3 relative_corner = cube_vertices[i] - torso_pos3d;
												Vector3 rotated_corner = {
													relative_corner.dot_product(side_vector1),
													relative_corner.y,
													-relative_corner.dot_product(look_direction)
												};
												cube_vertices[i] = rotated_corner + torso_pos3d;
											}
										}

										Vector2 CubeVertice1 = world_to_screen(cube_vertices[0]).position;
										Vector2 CubeVertice2 = world_to_screen(cube_vertices[1]).position;
										Vector2 CubeVertice3 = world_to_screen(cube_vertices[2]).position;
										Vector2 CubeVertice4 = world_to_screen(cube_vertices[3]).position;

										if (CubeVertice1.x != -1 && CubeVertice2.x != -1 && CubeVertice3.x != -1 && CubeVertice4.x != -1) {
											Vector2 CubeVertice5 = world_to_screen(cube_vertices[4]).position;
											Vector2 CubeVertice6 = world_to_screen(cube_vertices[5]).position;
											Vector2 CubeVertice7 = world_to_screen(cube_vertices[6]).position;
											Vector2 CubeVertice8 = world_to_screen(cube_vertices[7]).position;

											if (CubeVertice5.x != -1 && CubeVertice6.x != -1 && CubeVertice7.x != -1 && CubeVertice8.x != -1) {
												draw->AddLine(ImVec2(CubeVertice1.x, CubeVertice1.y), ImVec2(CubeVertice2.x, CubeVertice2.y), ImGui::ColorConvertFloat4ToU32(ImVec4(globals::box_color[0], globals::box_color[1], globals::box_color[2], 1.f)));
												draw->AddLine(ImVec2(CubeVertice2.x, CubeVertice2.y), ImVec2(CubeVertice6.x, CubeVertice6.y), ImGui::ColorConvertFloat4ToU32(ImVec4(globals::box_color[0], globals::box_color[1], globals::box_color[2], 1.f)));
												draw->AddLine(ImVec2(CubeVertice6.x, CubeVertice6.y), ImVec2(CubeVertice5.x, CubeVertice5.y), ImGui::ColorConvertFloat4ToU32(ImVec4(globals::box_color[0], globals::box_color[1], globals::box_color[2], 1.f)));
												draw->AddLine(ImVec2(CubeVertice5.x, CubeVertice5.y), ImVec2(CubeVertice1.x, CubeVertice1.y), ImGui::ColorConvertFloat4ToU32(ImVec4(globals::box_color[0], globals::box_color[1], globals::box_color[2], 1.f)));

												draw->AddLine(ImVec2(CubeVertice1.x, CubeVertice1.y), ImVec2(CubeVertice3.x, CubeVertice3.y), ImGui::ColorConvertFloat4ToU32(ImVec4(globals::box_color[0], globals::box_color[1], globals::box_color[2], 1.f)));
												draw->AddLine(ImVec2(CubeVertice2.x, CubeVertice2.y), ImVec2(CubeVertice4.x, CubeVertice4.y), ImGui::ColorConvertFloat4ToU32(ImVec4(globals::box_color[0], globals::box_color[1], globals::box_color[2], 1.f)));
												draw->AddLine(ImVec2(CubeVertice6.x, CubeVertice6.y), ImVec2(CubeVertice8.x, CubeVertice8.y), ImGui::ColorConvertFloat4ToU32(ImVec4(globals::box_color[0], globals::box_color[1], globals::box_color[2], 1.f)));
												draw->AddLine(ImVec2(CubeVertice5.x, CubeVertice5.y), ImVec2(CubeVertice7.x, CubeVertice7.y), ImGui::ColorConvertFloat4ToU32(ImVec4(globals::box_color[0], globals::box_color[1], globals::box_color[2], 1.f)));

												draw->AddLine(ImVec2(CubeVertice3.x, CubeVertice3.y), ImVec2(CubeVertice4.x, CubeVertice4.y), ImGui::ColorConvertFloat4ToU32(ImVec4(globals::box_color[0], globals::box_color[1], globals::box_color[2], 1.f)));
												draw->AddLine(ImVec2(CubeVertice4.x, CubeVertice4.y), ImVec2(CubeVertice8.x, CubeVertice8.y), ImGui::ColorConvertFloat4ToU32(ImVec4(globals::box_color[0], globals::box_color[1], globals::box_color[2], 1.f)));
												draw->AddLine(ImVec2(CubeVertice8.x, CubeVertice8.y), ImVec2(CubeVertice7.x, CubeVertice7.y), ImGui::ColorConvertFloat4ToU32(ImVec4(globals::box_color[0], globals::box_color[1], globals::box_color[2], 1.f)));
												draw->AddLine(ImVec2(CubeVertice7.x, CubeVertice7.y), ImVec2(CubeVertice3.x, CubeVertice3.y), ImGui::ColorConvertFloat4ToU32(ImVec4(globals::box_color[0], globals::box_color[1], globals::box_color[2], 1.f)));
											}
										}
									}
								}

								if (globals::health_bar) {
									ImVec2 bar_pos = { foot_pos.x - width + 6, foot_pos.y };
									ImVec2 bar_size = { 4, torso_pos.y + height };

									draw->AddRectFilled(bar_pos, ImVec2(bar_pos.x + bar_size.x, bar_size.y), ImColor(0, 0, 0, 150));

									if (player.health > 0 && player.health <= 100) {
										float health_ratio = player.health / 100;
										float bar_height = (bar_size.y - bar_pos.y) * health_ratio;

										draw->AddRectFilled(ImVec2(bar_pos.x + 1, bar_pos.y + 1), ImVec2(bar_pos.x + bar_size.x - 1, bar_pos.y + bar_height - 1), ImColor(0, 255, 0, 150));
									}
								}

								if (globals::armor_bar) {
									float armor = player.armor_path.value();

									ImVec2 bar_pos = { foot_pos.x - width, foot_pos.y + 6 };
									ImVec2 bar_size = { torso_pos.x + width, 4 };

									draw->AddRectFilled(bar_pos, ImVec2(bar_size.x, bar_pos.y + bar_size.y), ImColor(0, 0, 0, 150));

									if (armor > 0) {
										float armor_ratio = armor / 130;
										float bar_width = (bar_size.x - bar_pos.x) * armor_ratio;

										draw->AddRectFilled(ImVec2(bar_pos.x + 1, bar_pos.y + 1), ImVec2(bar_pos.x + bar_width - 1, bar_pos.y + bar_size.y - 1), ImColor(114, 160, 193));
									}
								}

								if (globals::skeleton && player.bones["UpperTorso"].self) {
									Vector2 upper_torso_2d = world_to_screen(player.bones["UpperTorso"].position()).position;
									Vector2 lower_torso_2d = world_to_screen(player.bones["LowerTorso"].position()).position;

									if (upper_torso_2d == -1 || lower_torso_2d == -1)
										continue;

									Vector2 left_upper_arm_2d = world_to_screen(player.bones["LeftUpperArm"].position()).position;
									Vector2 left_lower_arm_2d = world_to_screen(player.bones["LeftLowerArm"].position()).position;
									Vector2 left_hand_2d = world_to_screen(player.bones["LeftHand"].position()).position;

									if (left_hand_2d == -1 || left_lower_arm_2d == -1 || left_upper_arm_2d == -1)
										continue;

									Vector2 right_upper_arm_2d = world_to_screen(player.bones["RightUpperArm"].position()).position;
									Vector2 right_lower_arm_2d = world_to_screen(player.bones["RightLowerArm"].position()).position;
									Vector2 right_hand_2d = world_to_screen(player.bones["RightHand"].position()).position;

									if (right_hand_2d == -1 || right_lower_arm_2d == -1 || right_upper_arm_2d == -1)
										continue;

									Vector2 left_upper_leg_2d = world_to_screen(player.bones["LeftUpperLeg"].position()).position;
									Vector2 left_lower_leg_2d = world_to_screen(player.bones["LeftLowerLeg"].position()).position;
									Vector2 left_foot_2d = world_to_screen(player.bones["LeftFoot"].position()).position;

									if (left_foot_2d == -1 || left_lower_leg_2d == -1 || left_upper_leg_2d == -1)
										continue;

									Vector2 right_upper_leg_2d = world_to_screen(player.bones["RightUpperLeg"].position()).position;
									Vector2 right_lower_leg_2d = world_to_screen(player.bones["RightLowerLeg"].position()).position;
									Vector2 right_foot_2d = world_to_screen(player.bones["RightFoot"].position()).position;

									if (right_foot_2d == -1 || right_lower_leg_2d == -1 || right_upper_leg_2d == -1)
										continue;

									draw->AddLine(ImVec2(upper_torso_2d.x, upper_torso_2d.y), ImVec2(lower_torso_2d.x, lower_torso_2d.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));
									draw->AddLine(ImVec2(upper_torso_2d.x, upper_torso_2d.y), ImVec2(left_upper_arm_2d.x, left_upper_arm_2d.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));                    
									draw->AddLine(ImVec2(upper_torso_2d.x, upper_torso_2d.y), ImVec2(right_upper_arm_2d.x, right_upper_arm_2d.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));

									draw->AddLine(ImVec2(left_upper_arm_2d.x, left_upper_arm_2d.y), ImVec2(left_lower_arm_2d.x, left_lower_arm_2d.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));
									draw->AddLine(ImVec2(left_lower_arm_2d.x, left_lower_arm_2d.y), ImVec2(left_hand_2d.x, left_hand_2d.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));

									draw->AddLine(ImVec2(right_upper_arm_2d.x, right_upper_arm_2d.y), ImVec2(right_lower_arm_2d.x, right_lower_arm_2d.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));
									draw->AddLine(ImVec2(right_lower_arm_2d.x, right_lower_arm_2d.y), ImVec2(right_hand_2d.x, right_hand_2d.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));

									draw->AddLine(ImVec2(lower_torso_2d.x, lower_torso_2d.y), ImVec2(left_upper_leg_2d.x, left_upper_leg_2d.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));
									draw->AddLine(ImVec2(lower_torso_2d.x, lower_torso_2d.y), ImVec2(right_upper_leg_2d.x, right_upper_leg_2d.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));

									draw->AddLine(ImVec2(right_upper_leg_2d.x, right_upper_leg_2d.y), ImVec2(right_lower_leg_2d.x, right_lower_leg_2d.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));
									draw->AddLine(ImVec2(right_lower_leg_2d.x, right_lower_leg_2d.y), ImVec2(right_foot_2d.x, right_foot_2d.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));

									draw->AddLine(ImVec2(left_upper_leg_2d.x, left_upper_leg_2d.y), ImVec2(left_lower_leg_2d.x, left_lower_leg_2d.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));
									draw->AddLine(ImVec2(left_lower_leg_2d.x, left_lower_leg_2d.y), ImVec2(left_foot_2d.x, left_foot_2d.y), ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 1.f, 1.f)));
								}
							}
						}
					}
				}
			}
		}

		// ESP for NPCs/Bots
		for (Player& npc : npc_list) {
			if (npc.self && npc.static_character.self && npc.static_root_part.self) {
				if (npc.health <= 1)
					continue;

				Vector3 torso_pos3d = npc.static_root_part.position();
				if (torso_pos3d == 0)
					continue;

				auto [torso_pos, torso_on_screen] = world_to_screen(torso_pos3d);

				if (torso_on_screen && torso_pos3d != get_local_position()) {
					Vector3 head_pos3d = npc.static_head.position();
					if (head_pos3d == 0)
						continue;

					auto [head_pos, head_on_screen] = world_to_screen(head_pos3d - Vector3{ 0, 2, 0 });

					if (head_on_screen) {
						Vector3 foot_pos3d = head_pos3d - Vector3{ 0, 5, 0 };
						auto [foot_pos, foot_on_screen] = world_to_screen(foot_pos3d);

						if (foot_on_screen) {
							float height = head_pos.y - foot_pos.y;
							float width = height / 1.5f;

							if ((foot_pos.x - width) > 1800 || (torso_pos.x + width) > 1800 || foot_pos.y > 1800 || (torso_pos.y + height) > 1800)
								continue;

							// NPC Box ESP (using different color - red tint for NPCs)
							if (globals::box) {
								if (globals::box_type == 0) {
									ImVec4 npc_color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); // Red tint for NPCs
									draw->AddRect(ImVec2(foot_pos.x - width, foot_pos.y), ImVec2(torso_pos.x + width, torso_pos.y + height), ImGui::ColorConvertFloat4ToU32(npc_color), 0.f, 0, 2.f);
								}
							}

							// NPC Health Bar
							if (globals::health_bar) {
								float health_percentage = npc.health / 100.0f;
								ImVec2 bar_pos = ImVec2(foot_pos.x - width + 6, foot_pos.y);
								ImVec2 bar_size = ImVec2(4, torso_pos.y + height);

								draw->AddRectFilled(bar_pos, ImVec2(bar_pos.x + bar_size.x, bar_size.y), ImColor(0, 0, 0, 150));

								if (health_percentage > 0) {
									float bar_height = (bar_size.y - bar_pos.y) * health_percentage;
									draw->AddRectFilled(ImVec2(bar_pos.x + 1, bar_pos.y + 1), ImVec2(bar_pos.x + bar_size.x - 1, bar_pos.y + bar_height - 1), ImColor(255, 100, 100, 150)); // Red health for NPCs
								}
							}

							// NPC Skeleton
							if (globals::skeleton && npc.bones["UpperTorso"].self) {
								Vector2 upper_torso_2d = world_to_screen(npc.bones["UpperTorso"].position()).position;
								Vector2 lower_torso_2d = world_to_screen(npc.bones["LowerTorso"].position()).position;

								if (upper_torso_2d == -1 || lower_torso_2d == -1)
									continue;

								Vector2 left_upper_arm_2d = world_to_screen(npc.bones["LeftUpperArm"].position()).position;
								Vector2 left_lower_arm_2d = world_to_screen(npc.bones["LeftLowerArm"].position()).position;
								Vector2 left_hand_2d = world_to_screen(npc.bones["LeftHand"].position()).position;

								if (left_hand_2d == -1 || left_lower_arm_2d == -1 || left_upper_arm_2d == -1)
									continue;

								Vector2 right_upper_arm_2d = world_to_screen(npc.bones["RightUpperArm"].position()).position;
								Vector2 right_lower_arm_2d = world_to_screen(npc.bones["RightLowerArm"].position()).position;
								Vector2 right_hand_2d = world_to_screen(npc.bones["RightHand"].position()).position;

								if (right_hand_2d == -1 || right_lower_arm_2d == -1 || right_upper_arm_2d == -1)
									continue;

								Vector2 left_upper_leg_2d = world_to_screen(npc.bones["LeftUpperLeg"].position()).position;
								Vector2 left_lower_leg_2d = world_to_screen(npc.bones["LeftLowerLeg"].position()).position;
								Vector2 left_foot_2d = world_to_screen(npc.bones["LeftFoot"].position()).position;

								if (left_foot_2d == -1 || left_lower_leg_2d == -1 || left_upper_leg_2d == -1)
									continue;

								Vector2 right_upper_leg_2d = world_to_screen(npc.bones["RightUpperLeg"].position()).position;
								Vector2 right_lower_leg_2d = world_to_screen(npc.bones["RightLowerLeg"].position()).position;
								Vector2 right_foot_2d = world_to_screen(npc.bones["RightFoot"].position()).position;

								if (right_foot_2d == -1 || right_lower_leg_2d == -1 || right_upper_leg_2d == -1)
									continue;

								// Draw skeleton with red color for NPCs
								ImU32 npc_skeleton_color = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 0.3f, 0.3f, 1.f));
								
								draw->AddLine(ImVec2(upper_torso_2d.x, upper_torso_2d.y), ImVec2(lower_torso_2d.x, lower_torso_2d.y), npc_skeleton_color);
								draw->AddLine(ImVec2(upper_torso_2d.x, upper_torso_2d.y), ImVec2(left_upper_arm_2d.x, left_upper_arm_2d.y), npc_skeleton_color);                    
								draw->AddLine(ImVec2(upper_torso_2d.x, upper_torso_2d.y), ImVec2(right_upper_arm_2d.x, right_upper_arm_2d.y), npc_skeleton_color);

								draw->AddLine(ImVec2(left_upper_arm_2d.x, left_upper_arm_2d.y), ImVec2(left_lower_arm_2d.x, left_lower_arm_2d.y), npc_skeleton_color);
								draw->AddLine(ImVec2(left_lower_arm_2d.x, left_lower_arm_2d.y), ImVec2(left_hand_2d.x, left_hand_2d.y), npc_skeleton_color);

								draw->AddLine(ImVec2(right_upper_arm_2d.x, right_upper_arm_2d.y), ImVec2(right_lower_arm_2d.x, right_lower_arm_2d.y), npc_skeleton_color);
								draw->AddLine(ImVec2(right_lower_arm_2d.x, right_lower_arm_2d.y), ImVec2(right_hand_2d.x, right_hand_2d.y), npc_skeleton_color);

								draw->AddLine(ImVec2(lower_torso_2d.x, lower_torso_2d.y), ImVec2(left_upper_leg_2d.x, left_upper_leg_2d.y), npc_skeleton_color);
								draw->AddLine(ImVec2(lower_torso_2d.x, lower_torso_2d.y), ImVec2(right_upper_leg_2d.x, right_upper_leg_2d.y), npc_skeleton_color);

								draw->AddLine(ImVec2(right_upper_leg_2d.x, right_upper_leg_2d.y), ImVec2(right_lower_leg_2d.x, right_lower_leg_2d.y), npc_skeleton_color);
								draw->AddLine(ImVec2(right_lower_leg_2d.x, right_lower_leg_2d.y), ImVec2(right_foot_2d.x, right_foot_2d.y), npc_skeleton_color);

								draw->AddLine(ImVec2(left_upper_leg_2d.x, left_upper_leg_2d.y), ImVec2(left_lower_leg_2d.x, left_lower_leg_2d.y), npc_skeleton_color);
								draw->AddLine(ImVec2(left_lower_leg_2d.x, left_lower_leg_2d.y), ImVec2(left_foot_2d.x, left_foot_2d.y), npc_skeleton_color);
							}
						}
					}
				}
			}
		}
	}