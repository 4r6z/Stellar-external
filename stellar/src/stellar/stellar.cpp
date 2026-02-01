#include "stellar.hpp"

#include <ShlObj.h>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <TlHelp32.h>

#include "wrappers/datamodel.hpp"
#include "wrappers/instance.hpp"
#include "wrappers/service.hpp"

#include "driver/driver.hpp"

#include "helpers/console.hpp"
#include "wrappers/visual_engine.hpp"
#include "cheat/threads/threads.hpp"
#include "cheat/globals/globals.hpp"
#include "helpers/world.hpp"
#include "cheat/features/triggerbot/triggerbot.hpp"

#include "cheat/features/aimbot/aimbot.hpp"
#include "cheat/features/speed/speed.hpp"
#include "cheat/features/anti_lock/anti_lock.hpp"
#include "cheat/overlay/overlay.hpp"
#include "helpers/utils.hpp"
std::mutex update_mutex;

// Forward declaration
static std::uint64_t get_render_view_direct();

static std::wstring appdata_path() {
	wchar_t path[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)))
		return std::wstring(path);

	return L"";
}

std::vector<std::filesystem::path> get_roblox_file_logs() {
	std::vector<std::filesystem::path> roblox_log;
	std::wstring app_data_path = appdata_path();
	std::wstring roblox_log_path = app_data_path + L"\\Roblox\\logs";

	try {
		for (const auto& entry : std::filesystem::directory_iterator(roblox_log_path))
			if (entry.is_regular_file() && entry.path().extension() == ".log" && entry.path().filename().string().find("Player") != std::string::npos)
				roblox_log.push_back(entry.path());
	}
	catch (const std::exception& e) {
		COUT("Error reading Roblox logs directory: " << e.what());
	}

	return roblox_log;
}

std::filesystem::path get_latest_log() {
	auto logs = get_roblox_file_logs();

	if (logs.empty()) {
		COUT("No Roblox log files found!");
		return {};
	}

	std::sort(logs.begin(), logs.end(), [](const std::filesystem::path& a, const std::filesystem::path& b) {
		return std::filesystem::last_write_time(a) > std::filesystem::last_write_time(b);
	});

	COUT("Using log file: " << logs[0].string());
	return logs[0];
}


static std::uint64_t get_render_view() {
	COUT("Attempting to get render view using direct offsets...");
	
	// Try the direct offset method first
	std::uint64_t render_view = get_render_view_direct();
	if (render_view) {
		return render_view;
	}
	
	COUT("Direct method failed, trying log file method...");
	auto latest_log = get_latest_log();

	if (latest_log.empty()) {
		COUT("No log file available!");
		return 0;
	}

	std::ifstream rbx_log(latest_log);
	if (!rbx_log.is_open()) {
		COUT("Failed to open log file: " << latest_log.string());
		return 0;
	}

	std::string rbx_log_line;
	int line_count = 0;

	while (std::getline(rbx_log, rbx_log_line)) {
		line_count++;
		if (rbx_log_line.contains("initialize view(")) {
			COUT("Found render view on line " << line_count);
			size_t start_pos = rbx_log_line.find("initialize view(") + 16;
			size_t end_pos = rbx_log_line.find(')', start_pos);
			
			if (end_pos != std::string::npos) {
				std::string hex_str = rbx_log_line.substr(start_pos, end_pos - start_pos);
				std::uint64_t renderview = std::strtoull(hex_str.c_str(), nullptr, 16);
				COUT("Render view address: 0x" << std::hex << renderview);
				return renderview;
			}
		}
	}
	
	COUT("'initialize view(' not found in log file after " << line_count << " lines");
	return 0;
}

static std::uint64_t get_render_view_direct() {
	COUT("Using direct offset-based render view detection...");
	
	try {
		// Use the VisualEnginePointer offset to get the visual engine directly
		uintptr_t visual_engine_ptr = base_address + stellar::offsets::VisualEnginePointer;
		std::uint64_t visual_engine = read<std::uint64_t>(visual_engine_ptr);
		
		if (!visual_engine || visual_engine < 0x10000) {
			COUT("Failed to read visual engine from offset 0x" << std::hex << stellar::offsets::VisualEnginePointer);
			return 0;
		}
		
		COUT("Visual engine found at: 0x" << std::hex << visual_engine);
		
		// Now get the render view from the visual engine
		std::uint64_t render_view = read<std::uint64_t>(visual_engine + stellar::offsets::VisualEngineToDataModel1);
		
		if (!render_view || render_view < 0x10000) {
			COUT("Failed to get render view from visual engine");
			return 0;
		}
		
		COUT("Render view found at: 0x" << std::hex << render_view);
		return render_view;
	}
	catch (const std::exception& e) {
		COUT("Exception in direct render view method: " << e.what());
		return 0;
	}
}

bool stellar::init() {
    try {
        COUT("Starting Stellar initialization...");
        
        // Initialize the new driver system
        COUT("Initializing driver system...");
        mem::initialize();
        
        if (!mem::grabroblox_h()) {
            COUT("Failed to initialize new driver system!");
            std::cin.get(); // Wait for user input before closing
            return false;
        }

        COUT("New driver system initialized successfully!");

        COUT("Looking for Roblox window...");
        globals::game_handle = FindWindowA(0, "Roblox");

        if (!globals::game_handle) {
            COUT("Failed to get game handle!");
            std::cin.get(); // Wait for user input before closing
            return false;
        }

        COUT("Found Roblox! Waiting 1 second before initialization...");
        CNEWLINE;
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Reduced from 3 seconds

        COUT("Using direct offset-based initialization...");
        COUT("Base address: 0x" << std::hex << base_address);
        
        // Get Visual Engine directly using offsets
        COUT("Getting Visual Engine...");
        uintptr_t visual_engine_ptr = base_address + stellar::offsets::VisualEnginePointer;
        VisualEngine visual_engine = static_cast<VisualEngine>(read<std::uint64_t>(visual_engine_ptr));

        if (!visual_engine.self) {
            COUT("Failed to get Visual Engine from offset 0x" << std::hex << stellar::offsets::VisualEnginePointer);
            std::cin.get(); // Wait for user input before closing
            return false;
        }

        COUT("Visual Engine found: 0x" << std::hex << visual_engine.self);

        // Get DataModel using the visual engine
        COUT("Getting DataModel...");
        std::uint64_t datamodel_ptr1 = read<std::uint64_t>(visual_engine.self + stellar::offsets::VisualEngineToDataModel1);
        if (!datamodel_ptr1) {
            COUT("Failed to get DataModel pointer 1");
            std::cin.get();
            return false;
        }

        DataModel game;
        game.self = read<std::uint64_t>(datamodel_ptr1 + stellar::offsets::VisualEngineToDataModel2);

        if (!game.self) {
            COUT("Failed to get DataModel");
            std::cin.get(); // Wait for user input before closing
            return false;
        }

        COUT("DataModel found: 0x" << std::hex << game.self);

        COUT("Getting game ID...");
        globals::game_id = game.game_id();
        COUT("Game ID: " << globals::game_id);

        if (globals::game_id != 18461536252) {
            COUT("Getting Players service...");
            Service players = game.find_first_child<Service>("Players");

            if (!players.self) {
                COUT("Failed to get Player service!");
                std::cin.get(); // Wait for user input before closing
                return false;
            }

            game.players = players;

            COUT("Reading player count...");
            for (int i = 0; i < 30; i++)
                players_val.push_back(players.read_players());

            mostfreq = find_most_frequent(players_val);
            COUT("Most frequent player count: " << mostfreq);
        }

        COUT("Getting Workspace...");
        globals::workspace = game.find_first_child<Workspace>("Workspace");
        globals::visual_engine = visual_engine;
        globals::game = game;

        COUT("Starting threads...");
        std::thread(stellar::cheats::aimbot).detach();
        std::thread(stellar::cheats::speed).detach();
        std::thread(stellar::cheats::triggerbot).detach();
        std::thread(stellar::cheats::anti_lock).detach();
        std::thread(stellar::overlay::render).detach();
        std::thread(update_thread).detach();

        COUT("Initialized!");

        while (true) {
            globals::foreground = GetForegroundWindow();
            globals::is_focused = globals::foreground == globals::game_handle;

            if (!mem::process_id.load()) {
                COUT("Roblox Closed Closing Stellar");
                stellar::close();
                return false;
            }

            std::this_thread::sleep_for(std::chrono::seconds(6));
        }

        return true;
    }
    catch (const std::exception& e) {
        COUT("Exception in stellar::init(): " << e.what());
        std::cin.get(); // Wait for user input before closing
        return false;
    }
    catch (...) {
        COUT("Unknown exception in stellar::init()");
        std::cin.get(); // Wait for user input before closing
        return false;
    }
}

void stellar::close() {
    exit(-1);
}