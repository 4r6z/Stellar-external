#include "threads.hpp"

#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <iostream>

#include "../../wrappers/number_value.hpp"
#include "../../wrappers/string_value.hpp"
#include "../../wrappers/service.hpp"
#include "../../helpers/console.hpp"
#include "../globals/globals.hpp"
#include "../../helpers/utils.hpp"

std::vector<int> players_val{};
int player_count = 0;
int mostfreq = 0;

std::mutex player_list_mutex;
std::vector<Player> player_list;

// NPC/Bot caching
std::mutex npc_list_mutex;
std::vector<Player> npc_list;

std::atomic<bool> loop_control(true);
std::atomic<bool> loop_running(true);
std::atomic<bool> is_restarting(false);
std::atomic<bool> has_started(false);
std::chrono::time_point<std::chrono::steady_clock> last_iteration_time;

std::thread list_thread;

std::vector<std::string> bone_names = {
     "Head", "LeftHand", "RightHand", "LeftLowerArm", "RightLowerArm",
     "LeftUpperArm", "RightUpperArm", "LeftFoot", "LeftLowerLeg",
     "UpperTorso", "LeftUpperLeg", "RightFoot", "RightLowerLeg",
     "LowerTorso", "RightUpperLeg"
};

int find_most_frequent(const std::vector<int>& nums) {
    if (nums.empty()) return -1;

    std::unordered_map<int, int> countMap;
    int maxCount = 0, mostFrequent = nums[0];

    for (int num : nums) {
        if (++countMap[num] > maxCount) {
            maxCount = countMap[num];
            mostFrequent = num;
        }
    }

    return mostFrequent;
}

template <typename T>
void sync_lists(std::vector<T>& namesList1, std::vector<T>& namesList2) {
    namesList1.erase(
        std::remove_if(
            namesList1.begin(),
            namesList1.end(),
            [&namesList2](const T& item) {
                return std::find(namesList2.begin(), namesList2.end(), item) == namesList2.end() || item.self == 0 || item.health == 0 || (is_valid_address(item.self) && !is_valid_instance(item));
            }
        ),
        namesList1.end()
    );

    for (auto it = namesList2.begin(); it != namesList2.end(); ++it) {
        auto pos = std::distance(namesList2.begin(), it);
        if (pos >= namesList1.size() || *it != namesList1[pos])
            namesList1.insert(namesList1.begin() + pos, *it);
    }
}

void update_npc_cache() {
    try {
        std::lock_guard<std::mutex> lock(npc_list_mutex);
        std::vector<Player> current_npcs;
        current_npcs.reserve(20); // Reserve space for NPCs

        // Check if Bot or Bots folder exists in workspace
        Instance bot_folder = globals::workspace.find_first_child("Bot");
        if (!bot_folder.self) {
            // Try "Bots" folder if "Bot" doesn't exist
            bot_folder = globals::workspace.find_first_child("Bots");
        }
        
        if (!bot_folder.self) {
            // Clear NPC list if neither Bot nor Bots folder exists
            npc_list.clear();
            return;
        }

        // Look for datamodel objects in the Bot folder
        for (Instance& bot_instance : bot_folder.children()) {
            if (!bot_instance.self) continue;

            // Check if this instance has a model (character-like structure)
            ModelInstance bot_model;
            
            // Try different approaches to get the model
            if (bot_instance.class_name() == "Model") {
                // Instance itself is a model
                bot_model = static_cast<ModelInstance>(bot_instance);
            } else {
                // Look for a Model child first
                bot_model = bot_instance.find_first_child<ModelInstance>("Model");
                if (!bot_model.self) {
                    // Try looking for any child that might be a character model
                    for (Instance& child : bot_instance.children()) {
                        if (child.class_name() == "Model") {
                            bot_model = static_cast<ModelInstance>(child);
                            break;
                        }
                    }
                }
                
                // If still no model found, try to use the bot instance directly as a model
                // This handles cases where bots have non-standard class names but still function as models
                if (!bot_model.self) {
                    // Force cast the instance to ModelInstance and check if it has character parts
                    ModelInstance temp_model = static_cast<ModelInstance>(bot_instance);
                    Part test_root = temp_model.find_first_child<Part>("HumanoidRootPart");
                    Part test_head = temp_model.find_first_child<Part>("Head");
                    
                    if (test_root.self || test_head.self) {
                        bot_model = temp_model;
                    }
                }
            }
            
            if (!bot_model.self) continue;

            Player npc;
            npc.self = bot_model.self;
            npc.static_character = bot_model;
            npc.static_instance = bot_instance;

            // Get basic parts for the NPC
            npc.static_root_part = bot_model.find_first_child<Part>("HumanoidRootPart");
            npc.static_head = bot_model.find_first_child<Part>("Head");
            npc.static_humanoid = bot_model.find_first_child<Humanoid>("Humanoid");

            // Skip if essential parts are missing
            if (!npc.static_root_part.self || !npc.static_head.self) {
                continue;
            }

            // Set health for NPCs
            if (npc.static_humanoid.self) {
                npc.health = npc.static_humanoid.health();
            } else {
                npc.health = 100; // Default health
            }

            // Add skeleton bones if needed
            if (globals::skeleton && npc.static_character.self) {
                for (Part& part : npc.static_character.children<Part>()) {
                    std::string part_name = part.name();
                    const auto it = std::find(bone_names.begin(), bone_names.end(), part_name);
                    if (it != bone_names.end()) {
                        npc.bones[part_name] = part;
                    }
                }
            }

            current_npcs.push_back(npc);
        }

        // Update the NPC list
        sync_lists(npc_list, current_npcs);

    } catch (const std::exception& e) {
        npc_list.clear();
    }
}

void update_thread() {
    Service players = globals::game.players;
    
    // Cache frequently used values
    static int cached_mostfreq = 0;
    static auto last_player_check = std::chrono::steady_clock::now();
    static auto last_npc_check = std::chrono::steady_clock::now();
    
    while (true) {
        auto current_time = std::chrono::steady_clock::now();
        
        // Check player count less frequently for better performance
        if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_player_check).count() > 500) {
            cached_mostfreq = players.read_players();
            last_player_check = current_time;
        }
        
        // Update NPC cache every 1 second (less frequent than players)
        if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_npc_check).count() > 1000) {
            update_npc_cache();
            last_npc_check = current_time;
        }
        
        if (cached_mostfreq != mostfreq) {
            Sleep(250); // Reduced sleep time
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(player_list_mutex);
            std::vector<Player> current_players;
            current_players.reserve(50); // Pre-allocate for better performance

            if (globals::game_id != 3233893879 && globals::game_id != 292439477) {
                for (Player& entity : players.children<Player>()) {
                    Player plr;
                    plr.self = entity.self;
                    plr.static_instance = static_cast<Instance>(entity);

                    current_players.push_back(plr);

                    Sleep(1);
                }

                sync_lists(player_list, current_players);

                for (Player& entity : player_list) {
                    ModelInstance new_character = entity.character();

                    if (!is_valid_instance(entity.static_character, new_character))
                        entity.static_character = new_character;

                    Humanoid new_humanoid = entity.static_character.find_first_child<Humanoid>("Humanoid");

                    if (!is_valid_instance(entity.static_humanoid, new_humanoid))
                        entity.static_humanoid = new_humanoid;

                    Part new_root_part = entity.static_character.find_first_child<Part>("HumanoidRootPart");

                    if (!is_valid_instance(entity.static_root_part, new_root_part))
                        entity.static_root_part = new_root_part;

                    Part new_head = entity.static_character.find_first_child<Part>("Head");

                    if (!is_valid_instance(entity.static_head, new_head))
                        entity.static_head = new_head;

                    if (globals::game_id == 2788229376 || globals::game_id == 16033173781) {
                        Instance body_effects = entity.static_character.find_first_child("BodyEffects");

                        if (body_effects.self) {
                            if (globals::armor_bar)
                                entity.armor_path = body_effects.find_first_child<IntValue>("Armor");

                            if (globals::ko_check)
                                entity.ko_path = body_effects.find_first_child<BoolValue>("K.O");
                        }

                        if (globals::crew_check) {
                            StringValue found_crew = entity.static_instance.find_first_child("DataFolder").find_first_child("Information").find_first_child<StringValue>("Crew");

                            if (found_crew.self)
                                entity.crew = found_crew.value();
                        }
                    }

                    if (globals::skeleton && entity.static_character.self) {
                        for (Part& part : entity.static_character.children<Part>()) {
                            std::string part_name = part.name();

                            const auto it = std::find(bone_names.begin(), bone_names.end(), part_name);

                            if (it != bone_names.end())
                                entity.bones[part_name] = part;
                        }
                    }

                    entity.health = 100;
                    Sleep(1);
                }
            }
            else if (globals::game_id == 3233893879) {
                for (ModelInstance& character : globals::workspace.find_first_child("Characters").children<ModelInstance>()) {
                    Player plr;
                    plr.self = character.self;
                    plr.static_character = character;

                    current_players.push_back(plr);
                }

                sync_lists(player_list, current_players);

                for (Player& entity : player_list) {
                    entity.static_root_part = entity.static_character.find_first_child<Part>("Root");
                    entity.static_head = entity.static_character.find_first_child("Body").find_first_child<Part>("Head");

                    if (entity.static_root_part.self) {
                        NumberValue health = entity.static_character.find_first_child<NumberValue>("Health");

                        if (health.self)
                            entity.health = health.value();
                    }
                    else
                        entity.health = 150;
                }
            }
            else if (globals::game_id == 292439477) {
                for (Instance& team : globals::workspace.find_first_child("Players").children()) {
                    for (ModelInstance& character : team.children<ModelInstance>()) {
                        Player plr;

                        plr.self = character.self;
                        plr.static_character = character;
                        plr.health = 100;

                        current_players.push_back(plr);
                    }
                }

                player_list = current_players;

                for (Player& entity : player_list) {
                    for (Part& part : entity.children<Part>()) {
                        std::string part_class = part.class_name();

                        if (part_class == "Part" || part_class == "MeshPart") {
                            Vector3 part_size = part.size();

                            if (part_size == 1)
                                entity.static_head = part;
                            else if (part_size == Vector3{ 2, 2, 1 })
                                entity.static_root_part = part;
                        }
                    }
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Reduced from 400ms
    }
}