#include "../combat.h"
#include <thread>
#include <chrono>
#include <Windows.h>
#include "../../../util/console/console.h"
#include <iostream>
#include <random>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <unordered_map>
#include "../../hook.h"
#include "../../wallcheck/wallcheck.h"
#include "../../../drawing/overlay/overlay.h"

/*
 * Silent Aim Part Position Fixes:
 * 
 * 1. Fixed inconsistent partpos calculation across different target selection modes
 * 2. Added proper validation for part positions to prevent invalid coordinates
 * 3. Improved closest part calculation with better candidate filtering
 * 4. Enhanced closest point mode with dynamic bounding box calculation
 * 5. Added fallback mechanisms when part positions fail to calculate
 * 6. Fixed worldtoscreen function call (GetDimensins -> get_dimensions)
 * 7. Added screen bounds validation and clamping
 * 8. Improved prediction handling for all modes
 * 9. Added game ID detection for instance validation
 * 10. Implemented game-specific validation settings for different Roblox games
 */

#define max
#undef max
#define min
#undef min

using namespace roblox;

bool foundTarget = false;
static math::Vector2 partpos = {};
static uint64_t cachedPositionX = 0;
static uint64_t cachedPositionY = 0;
static bool dataReady = false;
static bool targetNeedsReset = false;


using namespace roblox;
inline float fastdist(const math::Vector2& a, const math::Vector2& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

bool isAutoFunctionActive() {
    return globals::bools::kill || globals::bools::autokill;
}

bool isRobloxMenuOpen() {
    try {
        // Check if Roblox menu is open by looking for Settings GUI in PlayerGui
        roblox::instance plrgui = globals::instances::localplayer.findfirstchild("PlayerGui");
        if (is_valid_address(plrgui.address)) {
            auto children = plrgui.get_children();
            for (auto& child : children) {
                std::string name = child.get_name();
                // Check for common Roblox menu GUI names
                if (name == "Settings" || name == "Menu" || name == "CoreGui" || 
                    name == "SettingsGui" || name == "MenuGui") {
                    return true;
                }
            }
        }
        
        // Also check if CoreGui menu is visible
        roblox::instance coregui = roblox::instance().read_service("CoreGui");
        if (is_valid_address(coregui.address)) {
            auto children = coregui.get_children();
            for (auto& child : children) {
                std::string name = child.get_name();
                if (name == "Settings" || name == "Menu" || name == "SettingsGui") {
                    return true;
                }
            }
        }
    } catch (...) {
        // Silent fail
    }
    return false;
}

bool shouldSilentAimBeActive() {
    // Disable silent aim when cheat overlay menu is visible
    if (overlay::visible) return false;
    
    // Disable silent aim when Roblox menu is open
    if (isRobloxMenuOpen()) return false;
    
    // Check if silent aim is enabled and keybind is active (if keybind is set)
    bool silentAimEnabled = globals::combat::silentaim;
    bool keybindActive = true; // Default to true if no keybind is set
    
    // If keybind is set (key > 0), require it to be enabled
    if (globals::combat::silentaimkeybind.key > 0) {
        keybindActive = globals::combat::silentaimkeybind.enabled;
    }
    
    return (globals::focused && silentAimEnabled && keybindActive) || isAutoFunctionActive();
}


// Helper function to get target closest to camera for silent aim
roblox::player gettargetclosesttocamerasilent() {
    // Add custom models to the player list
    std::vector<roblox::player> custom_models = GetCustomModels(base_address);
    std::vector<roblox::player> all_players = globals::instances::cachedplayers;
    all_players.insert(all_players.end(), custom_models.begin(), custom_models.end());
    const auto& players = all_players;
    
    if (players.empty()) return {};

    const bool useKnockCheck = false;
    const bool useHealthCheck = false;
    const bool useFov = globals::combat::usesfov;
    const float fovSize = globals::combat::sfovsize;
    const float fovSizeSquared = fovSize * fovSize;
    const float healthThreshold = 0.0f;
    const bool isArsenal = (globals::instances::gamename == "Arsenal");
    const std::string& localPlayerName = globals::instances::localplayer.get_name();
    const math::Vector3 cameraPos = globals::instances::camera.getPos();

    roblox::player closest = {};
    float closestDistanceSquared = 9e18f;

    for (auto player : players) {
        if (!is_valid_address(player.main.address) || !is_valid_address(player.head.address)) continue;
        if (player.name == localPlayerName) continue;

        // Arsenal-specific checks
        if (isArsenal) {
            if (useHealthCheck && player.health <= healthThreshold) continue;
            if (useKnockCheck) {
                try {
                    if (player.bodyeffects.findfirstchild("K.O").read_bool_value()) continue;
                } catch (...) { continue; }
            }
        }

        // Generic checks
        if (useHealthCheck && player.health <= healthThreshold) continue;
        if (useKnockCheck) {
            try {
                if (player.bodyeffects.findfirstchild("K.O").read_bool_value()) continue;
            } catch (...) { continue; }
        }

        // Team check
        if (globals::combat::teamcheck) {
            if (player.team.address == globals::instances::lp.team.address) continue;
        }

        // Grabbed check
        if (globals::combat::grabbedcheck) {
            try {
                if (player.bodyeffects.findfirstchild("Grabbed").read_bool_value()) continue;
            } catch (...) { continue; }
        }

        // Death check (always skip dead players during target selection)
        try {
            // Check for Dead flag in BodyEffects
            if (player.bodyeffects.findfirstchild("Dead").read_bool_value()) continue;
            
            // Check health
            if (player.health <= 0) continue;
            
            // Check if player is below ground (common death indicator)
            math::Vector3 playerPos = player.hrp.get_pos();
            if (playerPos.y < -50.0f) continue; // Below ground threshold
            
            // Check for K.O. state
            if (player.bodyeffects.findfirstchild("K.O").read_bool_value()) continue;
            
            // Check for respawn state
            if (player.bodyeffects.findfirstchild("Respawn").read_bool_value()) continue;
            
            // Arsenal-specific death check
            if (isArsenal) {
                auto nrpbs = player.main.findfirstchild("NRPBS");
                if (nrpbs.address) {
                    auto health = nrpbs.findfirstchild("Health");
                    if (health.address && health.read_double_value() == 0.0) continue;
                }
            }
        } catch (...) { continue; }

        math::Vector3 targetPos = player.hrp.get_pos();
        float distanceSquared = (cameraPos.x - targetPos.x) * (cameraPos.x - targetPos.x) + 
                               (cameraPos.y - targetPos.y) * (cameraPos.y - targetPos.y) + 
                               (cameraPos.z - targetPos.z) * (cameraPos.z - targetPos.z);

        // FOV check
        if (useFov) {
            math::Vector2 screenPos = roblox::worldtoscreen(targetPos);
            if (screenPos.x == -1.0f || screenPos.y == -1.0f) {
                continue;
            } else {
                math::Vector2 screenCenter = { 960.0f, 540.0f }; // Assuming 1920x1080, adjust as needed
                float screenDistanceSquared = (screenPos.x - screenCenter.x) * (screenPos.x - screenCenter.x) + 
                                            (screenPos.y - screenCenter.y) * (screenPos.y - screenCenter.y);
                if (screenDistanceSquared > fovSizeSquared) continue;
            }
        }

        // No wall check - treat all targets as visible
        if (distanceSquared < closestDistanceSquared) {
            closestDistanceSquared = distanceSquared;
            closest = player;
        }
    }

    // Return closest target
    return closest;
}

roblox::player gettargetclosesttomousesilent() {
    static HWND robloxWindow = nullptr;
    static auto lastWindowCheck = std::chrono::steady_clock::now();

    auto now = std::chrono::steady_clock::now();
    if (!robloxWindow || std::chrono::duration_cast<std::chrono::seconds>(now - lastWindowCheck).count() > 5) {
        robloxWindow = FindWindowA(nullptr, "Roblox");
        lastWindowCheck = now;
    }

    if (!robloxWindow) return {};

    POINT point;
    if (!GetCursorPos(&point) || !ScreenToClient(robloxWindow, &point)) {
        return {};
    }

    const math::Vector2 curpos = { static_cast<float>(point.x), static_cast<float>(point.y) };
    
    // Add custom models to the player list
    std::vector<roblox::player> custom_models = GetCustomModels(base_address);
    std::vector<roblox::player> all_players = globals::instances::cachedplayers;
    all_players.insert(all_players.end(), custom_models.begin(), custom_models.end());
    const auto& players = all_players;

    if (players.empty()) return {};

    const bool useKnockCheck = false;
    const bool useHealthCheck = false;
    const bool useWallCheck = globals::combat::wallcheck;
    const bool allowWallFallback = true; // fallback built-in; UI toggle optional
            const bool useFov = globals::combat::usesfov;
        const float fovSize = globals::combat::sfovsize;
        const float fovSizeSquared = fovSize * fovSize;
    const float healthThreshold = 0.0f;
    const bool isArsenal = (globals::instances::gamename == "Arsenal");
    const std::string& localPlayerName = globals::instances::localplayer.get_name();
    const math::Vector3 cameraPos = globals::instances::camera.getPos();

    roblox::player closest = {};
    roblox::player closestVisible = {};
    float closestDistanceSquared = 9e18f;
    float closestVisibleDistanceSquared = 9e18f;

    for (auto player : players) {
        if (!is_valid_address(player.main.address) ||
            player.name == localPlayerName ||
            player.head.address == 0) {
            continue;
        }

        // Team check - skip teammates if teamcheck is enabled
        if (globals::combat::teamcheck && globals::is_teammate(player)) {
            continue; // Skip teammate
        }

        // Grabbed check - skip players who are grabbed if enabled
        if (globals::combat::grabbedcheck && globals::is_grabbed(player)) {
            continue; // Skip grabbed player
        }

        // Death check (always skip dead players during target selection)
        try {
            // Check for Dead flag in BodyEffects
            if (player.bodyeffects.findfirstchild("Dead").read_bool_value()) continue;
            
            // Check health
            if (player.health <= 0) continue;
            
            // Check if player is below ground (common death indicator)
            math::Vector3 playerPos = player.hrp.get_pos();
            if (playerPos.y < -50.0f) continue; // Below ground threshold
            
            // Check for K.O. state
            if (player.bodyeffects.findfirstchild("K.O").read_bool_value()) continue;
            
            // Check for respawn state
            if (player.bodyeffects.findfirstchild("Respawn").read_bool_value()) continue;
            
            // Arsenal-specific death check
            if (isArsenal) {
                auto nrpbs = player.main.findfirstchild("NRPBS");
                if (nrpbs.address) {
                    auto health = nrpbs.findfirstchild("Health");
                    if (health.address && health.read_double_value() == 0.0) continue;
                }
            }
        } catch (...) { continue; }

        const math::Vector2 screenCoords = roblox::worldtoscreen(player.head.get_pos());
        if (screenCoords.x == -1.0f || screenCoords.y == -1.0f) {
            continue;
        }

        const float dx = curpos.x - screenCoords.x;
        const float dy = curpos.y - screenCoords.y;
        const float distanceSquared = dx * dx + dy * dy;

        // FOV check disabled for now - uncomment to enable FOV restriction
        // if (useFov && distanceSquared > fovSizeSquared) continue;

        if (isArsenal) {
            auto nrpbs = player.main.findfirstchild("NRPBS");
            if (nrpbs.address) {
                auto health = nrpbs.findfirstchild("Health");
                if (health.address && (health.read_double_value() == 0.0 || player.hrp.get_pos().y < 0.0f)) {
                    continue;
                }
            }
        }

        // Wall check
        bool isVisible = true;
        if (useWallCheck) {
            math::Vector3 targetPos = player.head.get_pos();
            isVisible = wallcheck::can_see(cameraPos, targetPos);
        }

        if (isVisible) {
            if (distanceSquared < closestVisibleDistanceSquared) {
                closestVisibleDistanceSquared = distanceSquared;
                closestVisible = player;
            }
        } else {
            if (distanceSquared < closestDistanceSquared) {
                closestDistanceSquared = distanceSquared;
                closest = player;
            }
        }
    }

    if (is_valid_address(closestVisible.main.address)) return closestVisible;
    if (allowWallFallback) return closest;
    return {};
}

void hooks::silentrun() {
    roblox::player target;
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL); // Reduced from HIGHEST to save CPU
    HWND robloxWindow = FindWindowA(0, "Roblox");
    const std::chrono::milliseconds sleepTime(10);
    static std::unordered_map<uintptr_t, int> walledCount; // Track consecutive walled checks per target

    while (true) {
        std::this_thread::sleep_for(std::chrono::microseconds(100)); // Increased from 47μs to reduce CPU usage
        if (globals::handlingtp) {
            dataReady = false;
            globals::instances::cachedtarget = {};
            foundTarget = false;
            targetNeedsReset = false;
            continue;
        }
        globals::combat::silentaimkeybind.update();

        if (!shouldSilentAimBeActive()) {
            dataReady = false;
            // Clear cached target when silent aim is deactivated to allow manual unlocking
            if (globals::combat::stickyaimsilent) {
                globals::instances::cachedtarget = {};
            }
            target = {};
            foundTarget = false;
            targetNeedsReset = false;
            continue;
        }

        // Dynamic aim instance search through PlayerGui children
        roblox::instance plrgui = globals::instances::localplayer.findfirstchild("PlayerGui");
        if (plrgui.address) {
            auto chdr = plrgui.get_children();
            for (auto& child : chdr) {
                auto schild = child.get_children();
                for (auto& child2 : schild) {
                    if (child2.get_name() == "Aim") {
                        globals::instances::aim = child2;
                        break;
                    }
                }
            }
        }

        if (isAutoFunctionActive()) {
            target = globals::bools::entity;
            globals::instances::cachedtarget = target;
            foundTarget = (target.head.address != 0);
        }
        else if (globals::combat::connect_to_aimbot) {
            // When connected to aimbot, get the current aimbot target directly
            // This will use the same target that the aimbot (camlock) is currently locked onto
            if (globals::combat::aimbot_locked && is_valid_address(globals::combat::aimbot_current_target.main.address)) {
                target = globals::combat::aimbot_current_target;
                foundTarget = (target.head.address != 0);
                
                // Validate the target is still alive and valid for silent aim
                if (foundTarget) {
                    try {
                        // Check if target is still alive
                        if (target.bodyeffects.findfirstchild("Dead").read_bool_value()) {
                            foundTarget = false;
                            target = {};
                        } else if (target.health <= 0) {
                            foundTarget = false;
                            target = {};
                        } else {
                            math::Vector3 targetPos = target.hrp.get_pos();
                            if (targetPos.y < -50.0f) { // Below ground threshold
                                foundTarget = false;
                                target = {};
                            }
                        }
                    } catch (...) {
                        foundTarget = false;
                        target = {};
                    }
                }
            } else {
                // No aimbot target available or aimbot not locked
                target = {};
                foundTarget = false;
            }
            // Don't update cachedtarget here to avoid interfering with aimbot
            // Don't use sticky aim when connected to aimbot - always follow aimbot's target
        }
        else {
            // Always get closest player to mouse for silent aim
            target = gettargetclosesttomousesilent();
            globals::instances::cachedlasttarget = target;
            foundTarget = (target.head.address != 0);
            globals::instances::cachedtarget = target;
        }
        // Death and knock check for current target (always unlock on death/knock)
        if (foundTarget && is_valid_address(target.main.address)) {
            try {
                // Check for Dead flag in BodyEffects
                if (target.bodyeffects.findfirstchild("Dead").read_bool_value()) {
                    foundTarget = false; // Drop target if they die
                    continue;
                }
                
                // Check health
                if (target.health <= 0) {
                    foundTarget = false; // Drop target if they die
                    continue;
                }
                
                // Check if target is below ground (common death indicator)
                math::Vector3 targetPos = target.hrp.get_pos();
                if (targetPos.y < -50.0f) { // Below ground threshold
                    foundTarget = false; // Drop target if they're below ground
                    continue;
                }
                
                // Arsenal-specific death check
                if (globals::instances::gamename == "Arsenal") {
                    auto nrpbs = target.main.findfirstchild("NRPBS");
                    if (nrpbs.address) {
                        auto health = nrpbs.findfirstchild("Health");
                        if (health.address && health.read_double_value() == 0.0) {
                            foundTarget = false; // Drop target if they die
                            continue;
                        }
                    }
                }
                
                // Check for K.O. state
                if (target.bodyeffects.findfirstchild("K.O").read_bool_value()) {
                    foundTarget = false; // Drop target if they're knocked out
                    continue;
                }
                
                // Check for respawn state
                if (target.bodyeffects.findfirstchild("Respawn").read_bool_value()) {
                    foundTarget = false; // Drop target if they're respawning
                    continue;
                }
            } catch (...) { }
        }
        
        // Wall check - only enforce if wallcheck was enabled during target selection
        // Use a more lenient approach to prevent flickering and false positives
        if (foundTarget && globals::combat::wallcheck && is_valid_address(target.head.address)) {
            math::Vector3 cameraPos = globals::instances::camera.getPos();
            
            // Check multiple body parts to reduce false positives (head might be occluded but torso visible)
            bool isVisible = false;
            bool checkedAny = false;
            
            // Try to find at least one visible body part
            std::vector<roblox::instance> partsToCheck = {
                target.head,
                target.hrp,
                target.uppertorso,
                target.lowertorso
            };
            
            for (auto& part : partsToCheck) {
                if (is_valid_address(part.address)) {
                    try {
                        math::Vector3 partPos = part.get_pos();
                        if (wallcheck::can_see(cameraPos, partPos)) {
                            isVisible = true;
                            checkedAny = true;
                            break; // Found at least one visible part
                        }
                        checkedAny = true;
                    } catch (...) {
                        // Part became invalid, continue checking other parts
                    }
                }
            }
            
            // Only drop target if we checked parts and none were visible
            // If we couldn't check any parts (all invalid), keep the target to avoid false drops
            if (checkedAny && !isVisible) {
                // Only drop if sticky aim is disabled OR if target is clearly walled
                // This prevents flickering when target briefly goes behind cover
                uintptr_t targetId = target.main.address;
                
                if (walledCount.find(targetId) == walledCount.end()) {
                    walledCount[targetId] = 0;
                }
                
                walledCount[targetId]++;
                
                // Require 3 consecutive walled checks before dropping (prevents flickering)
                if (walledCount[targetId] >= 3) {
                    foundTarget = false;
                    target = {};
                    globals::instances::cachedtarget = {};
                    walledCount.erase(targetId);
                    continue; // Break lock and skip this frame
                }
            } else {
                // Target is visible or we couldn't check, reset walled count
                if (foundTarget && is_valid_address(target.main.address)) {
                    uintptr_t targetId = target.main.address;
                    walledCount[targetId] = 0;
                }
            }
        }
        
        if (foundTarget && globals::instances::cachedtarget.head.address != 0) {
            if (isAutoFunctionActive()) {
                        }
            roblox::instance part = globals::instances::cachedtarget.head;
            
            // Enhanced target selection logic with body part selection
            if (globals::combat::closestpartsilent == 1) {
                // Choose closest body part to crosshair for silent aim
                struct Candidate { roblox::instance inst; };
                std::vector<Candidate> candidates;
                candidates.push_back({ globals::instances::cachedtarget.head });
                candidates.push_back({ globals::instances::cachedtarget.uppertorso });
                candidates.push_back({ globals::instances::cachedtarget.lowertorso });
                candidates.push_back({ globals::instances::cachedtarget.hrp });
                candidates.push_back({ globals::instances::cachedtarget.lefthand });
                candidates.push_back({ globals::instances::cachedtarget.righthand });
                candidates.push_back({ globals::instances::cachedtarget.leftfoot });
                candidates.push_back({ globals::instances::cachedtarget.rightfoot });
                POINT p; HWND rw = FindWindowA(nullptr, "Roblox"); if (rw) { GetCursorPos(&p); ScreenToClient(rw, &p); }
                math::Vector2 mouse = { (float)p.x, (float)p.y };
                float bestDist = 9e18f; roblox::instance best = part;
                
                // Filter out invalid candidates first
                std::vector<Candidate> validCandidates;
                for (auto& c : candidates) {
                    if (is_valid_address(c.inst.address)) {
                        // For sticky aim, include all valid body parts even if off screen
                        validCandidates.push_back(c);
                    }
                }
                
                // Find the closest valid part
                for (auto& c : validCandidates) {
                    math::Vector2 scr = roblox::worldtoscreen(c.inst.get_pos());
                    float dx, dy, d2;
                    
                    if (scr.x == -1.0f || scr.y == -1.0f) {
                        // Use a large distance to prioritize on-screen parts
                        scr = { 999999.0f, 999999.0f };
                        dx = mouse.x - scr.x; dy = mouse.y - scr.y; d2 = dx * dx + dy * dy;
                    } else {
                        dx = mouse.x - scr.x; dy = mouse.y - scr.y; d2 = dx * dx + dy * dy;
                    }
                    
                    if (d2 < bestDist) { bestDist = d2; best = c.inst; }
                }
                
                if (is_valid_address(best.address)) part = best;
                
                // Calculate partpos for the selected closest part
                math::Vector3 part3d = part.get_pos();
                if (globals::combat::silentpredictions) {
                    math::Vector3 velocity = part.get_velocity();
                    math::Vector3 veloVector = velocity / math::Vector3(globals::combat::silentpredictionsx, globals::combat::silentpredictionsy, globals::combat::silentpredictionsx);
                    part3d = part3d + veloVector;
                }
                partpos = roblox::worldtoscreen(part3d);
                
                // Validate the calculated partpos
                if (partpos.x == -1.0f || partpos.y == -1.0f) {
                    // If the selected part failed, try to find any valid part
                    for (auto& c : validCandidates) {
                        math::Vector3 fallbackPos = c.inst.get_pos();
                        if (globals::combat::silentpredictions) {
                            math::Vector3 velocity = c.inst.get_velocity();
                            math::Vector3 veloVector = velocity / math::Vector3(globals::combat::silentpredictionsx, globals::combat::silentpredictionsy, globals::combat::silentpredictionsx);
                            fallbackPos = fallbackPos + veloVector;
                        }
                        partpos = roblox::worldtoscreen(fallbackPos);
                        if (partpos.x != -1.0f && partpos.y != -1.0f) {
                            break;
                        }
                    }
                }
            }
            else if (globals::combat::closestpartsilent == 2) {
                // Choose closest point on the target to crosshair for silent aim
                POINT p; HWND rw = FindWindowA(nullptr, "Roblox"); if (rw) { GetCursorPos(&p); ScreenToClient(rw, &p); }
                math::Vector2 mouse = { (float)p.x, (float)p.y };
                
                // Get target's position and convert to screen coordinates
                math::Vector3 targetPos = globals::instances::cachedtarget.hrp.get_pos();
                math::Vector2 targetScreen = roblox::worldtoscreen(targetPos);
                
                // Check if target is visible on screen
                if (targetScreen.x == -1.0f || targetScreen.y == -1.0f) {
                    // Target not visible, fallback to HRP
                    math::Vector3 fallbackPos = targetPos;
                    if (globals::combat::silentpredictions) {
                        math::Vector3 velocity = globals::instances::cachedtarget.hrp.get_velocity();
                        math::Vector3 veloVector = velocity / math::Vector3(globals::combat::silentpredictionsx, globals::combat::silentpredictionsy, globals::combat::silentpredictionsx);
                        fallbackPos = fallbackPos + veloVector;
                    }
                    partpos = roblox::worldtoscreen(fallbackPos);
                } else {
                    // Calculate dynamic bounding box based on target's actual size
                    // Use multiple body parts to determine target bounds
                    std::vector<math::Vector2> bodyPartScreens;
                    std::vector<roblox::instance> bodyParts = {
                        globals::instances::cachedtarget.head,
                        globals::instances::cachedtarget.uppertorso,
                        globals::instances::cachedtarget.lowertorso,
                        globals::instances::cachedtarget.lefthand,
                        globals::instances::cachedtarget.righthand,
                        globals::instances::cachedtarget.leftfoot,
                        globals::instances::cachedtarget.rightfoot
                    };
                    
                    for (auto& bp : bodyParts) {
                        if (is_valid_address(bp.address)) {
                            math::Vector2 screenPos = roblox::worldtoscreen(bp.get_pos());
                            if (screenPos.x != -1.0f && screenPos.y != -1.0f) {
                                bodyPartScreens.push_back(screenPos);
                            }
                        }
                    }
                    
                    if (bodyPartScreens.empty()) {
                        // Fallback to simple bounding box
                        float boxWidth = 60.0f;
                        float boxHeight = 120.0f;
                        float closestX = std::max(targetScreen.x - boxWidth/2, std::min(targetScreen.x + boxWidth/2, mouse.x));
                        float closestY = std::max(targetScreen.y - boxHeight/2, std::min(targetScreen.y + boxHeight/2, mouse.y));
                        partpos = { closestX, closestY };
                    } else {
                        // Calculate actual bounding box from visible body parts
                        float minX = bodyPartScreens[0].x, maxX = bodyPartScreens[0].x;
                        float minY = bodyPartScreens[0].y, maxY = bodyPartScreens[0].y;
                        
                        for (const auto& pos : bodyPartScreens) {
                            minX = std::min(minX, pos.x);
                            maxX = std::max(maxX, pos.x);
                            minY = std::min(minY, pos.y);
                            maxY = std::max(maxY, pos.y);
                        }
                        
                        // Add some padding to the bounding box
                        float padding = 10.0f;
                        minX -= padding; maxX += padding;
                        minY -= padding; maxY += padding;
                        
                        // Find closest point on the bounding box to mouse cursor
                        float closestX = std::max(minX, std::min(maxX, mouse.x));
                        float closestY = std::max(minY, std::min(maxY, mouse.y));
                        partpos = { closestX, closestY };
                    }
                }
            }
            else {
                // Use selected body part for silent aim
                roblox::instance selectedPart = part;
                
                switch (globals::combat::silentaimpart) {
                    case 0: // Head
                        selectedPart = globals::instances::cachedtarget.head;
                        break;
                    case 1: // Upper Torso
                        selectedPart = globals::instances::cachedtarget.uppertorso;
                        break;
                    case 2: // Lower Torso
                        selectedPart = globals::instances::cachedtarget.lowertorso;
                        break;
                    case 3: // HumanoidRootPart
                        selectedPart = globals::instances::cachedtarget.hrp;
                        break;
                    case 4: // Left Hand
                        selectedPart = globals::instances::cachedtarget.lefthand;
                        break;
                    case 5: // Right Hand
                        selectedPart = globals::instances::cachedtarget.righthand;
                        break;
                    case 6: // Left Foot
                        selectedPart = globals::instances::cachedtarget.leftfoot;
                        break;
                    case 7: // Right Foot
                        selectedPart = globals::instances::cachedtarget.rightfoot;
                        break;
                    case 8: // Closest Part
                        // Choose closest body part to crosshair
                        {
                            struct Candidate { roblox::instance inst; };
                            std::vector<Candidate> candidates;
                            candidates.push_back({ globals::instances::cachedtarget.head });
                            candidates.push_back({ globals::instances::cachedtarget.uppertorso });
                            candidates.push_back({ globals::instances::cachedtarget.lowertorso });
                            candidates.push_back({ globals::instances::cachedtarget.hrp });
                            candidates.push_back({ globals::instances::cachedtarget.lefthand });
                            candidates.push_back({ globals::instances::cachedtarget.righthand });
                            candidates.push_back({ globals::instances::cachedtarget.leftfoot });
                            candidates.push_back({ globals::instances::cachedtarget.rightfoot });
                            POINT p; HWND rw = FindWindowA(nullptr, "Roblox"); if (rw) { GetCursorPos(&p); ScreenToClient(rw, &p); }
                            math::Vector2 mouse = { (float)p.x, (float)p.y };
                            float bestDist = 9e18f;
                            for (auto& c : candidates) {
                                if (!is_valid_address(c.inst.address)) continue;
                                math::Vector2 scr = roblox::worldtoscreen(c.inst.get_pos());
                                float dx, dy, d2;
                                
                                if (scr.x == -1.0f || scr.y == -1.0f) {
                                    // Use a large distance to prioritize on-screen parts
                                    scr = { 999999.0f, 999999.0f };
                                    dx = mouse.x - scr.x; dy = mouse.y - scr.y; d2 = dx * dx + dy * dy;
                                } else {
                                    dx = mouse.x - scr.x; dy = mouse.y - scr.y; d2 = dx * dx + dy * dy;
                                }
                                
                                if (d2 < bestDist) { bestDist = d2; selectedPart = c.inst; }
                            }
                        }
                        break;
                    case 9: // Random Part
                        {
                            std::vector<roblox::instance> candidates = {
                                globals::instances::cachedtarget.head,
                                globals::instances::cachedtarget.uppertorso,
                                globals::instances::cachedtarget.lowertorso,
                                globals::instances::cachedtarget.hrp,
                                globals::instances::cachedtarget.lefthand,
                                globals::instances::cachedtarget.righthand,
                                globals::instances::cachedtarget.leftfoot,
                                globals::instances::cachedtarget.rightfoot
                            };
                            // Filter out invalid candidates
                            std::vector<roblox::instance> validCandidates;
                            for (auto& c : candidates) {
                                if (is_valid_address(c.address)) {
                                    validCandidates.push_back(c);
                                }
                            }
                            if (!validCandidates.empty()) {
                                int randomIndex = rand() % validCandidates.size();
                                selectedPart = validCandidates[randomIndex];
                            }
                        }
                        break;
                }
                
                // Use the selected part
                if (is_valid_address(selectedPart.address)) {
                    part = selectedPart;
                }
                
                math::Vector3 part3d = part.get_pos();
                
                // Apply prediction for silent aim (keeping #RR EXTERNAL's prediction feature)
                math::Vector3 predictedPos = part3d;
                if (globals::combat::silentpredictions) {
                    math::Vector3 velocity = part.get_velocity();
                    math::Vector3 veloVector = velocity / math::Vector3(globals::combat::silentpredictionsx, globals::combat::silentpredictionsy, globals::combat::silentpredictionsx);
                    predictedPos = part3d + veloVector;
                }
                
                partpos = roblox::worldtoscreen(predictedPos);
            }
            
            // Ensure partpos is valid before proceeding
            if (partpos.x == -1.0f || partpos.y == -1.0f) {
                // Fallback: use target's HRP position if partpos is invalid
                math::Vector3 fallbackPos = globals::instances::cachedtarget.hrp.get_pos();
                if (globals::combat::silentpredictions) {
                    math::Vector3 velocity = globals::instances::cachedtarget.hrp.get_velocity();
                    math::Vector3 veloVector = velocity / math::Vector3(globals::combat::silentpredictionsx, globals::combat::silentpredictionsy, globals::combat::silentpredictionsx);
                    fallbackPos = fallbackPos + veloVector;
                }
                partpos = roblox::worldtoscreen(fallbackPos);
            }
            
            // Additional validation: ensure partpos is within screen bounds
            math::Vector2 dimensions = globals::instances::visualengine.get_dimensions();
            if (partpos.x < 0 || partpos.x > dimensions.x || partpos.y < 0 || partpos.y > dimensions.y) {
                // If partpos is outside screen bounds, clamp it to screen edges
                partpos.x = std::max(0.0f, std::min(partpos.x, dimensions.x));
                partpos.y = std::max(0.0f, std::min(partpos.y, dimensions.y));
            }
            
            // Final validation: ensure we have a valid part position
            if (partpos.x == -1.0f || partpos.y == -1.0f || 
                std::isnan(partpos.x) || std::isnan(partpos.y) ||
                std::isinf(partpos.x) || std::isinf(partpos.y)) {
                // Last resort: use screen center
                partpos = { dimensions.x / 2.0f, dimensions.y / 2.0f };
            }
            
            // If using closest point mode, partpos is already set above
            if (globals::combat::closestpartsilent != 2) {
                // partpos is already set above for the selected body part
            }

            
            
            globals::instances::cachedlasttarget = target;

            POINT cursorPoint;
            GetCursorPos(&cursorPoint);
            ScreenToClient(robloxWindow, &cursorPoint);

            if (globals::combat::usesfov) {
                float dx_f = static_cast<float>(cursorPoint.x) - partpos.x;
                float dy_f = static_cast<float>(cursorPoint.y) - partpos.y;
                float d2_f = dx_f * dx_f + dy_f * dy_f;
                float f2 = globals::combat::sfovsize * globals::combat::sfovsize;
                if (d2_f > f2) {
                    dataReady = false;
                    foundTarget = false;
                    continue;
                }
            }

            cachedPositionX = static_cast<uint64_t>(cursorPoint.x);
            cachedPositionY = static_cast<uint64_t>(dimensions.y - std::abs(dimensions.y - (cursorPoint.y)) - 58);
            dataReady = true;
        }
        else {
            dataReady = false;
                   }
    }
}

void hooks::silentrun2() {
    roblox::instance mouseServiceInstance;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> distrib(-100.0, 100.0);

    while (true) {
        if (globals::handlingtp) {
            continue;
        }
        if (!shouldSilentAimBeActive()) {
            if (globals::instances::cachedtarget.head.address != 0) targetNeedsReset = true;
            continue;
        }

        if (globals::instances::cachedtarget.head.address != 0 && dataReady) {
            // Always spoof mouse
            globals::instances::aim.setFramePosX(cachedPositionX);
            globals::instances::aim.setFramePosY(cachedPositionY);
            mouseServiceInstance.initialize_mouse_service(globals::instances::mouseservice);
            mouseServiceInstance.write_mouse_position(globals::instances::mouseservice, partpos.x, partpos.y);
        }
    }
}

void hooks::silent() {
    std::thread primaryThread(&hooks::silentrun);
    std::thread secondaryThread(&hooks::silentrun2);
    primaryThread.detach();
    secondaryThread.detach();
}