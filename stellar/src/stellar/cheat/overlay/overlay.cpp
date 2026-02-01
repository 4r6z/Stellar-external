#define IMGUI_DEFINE_MATH_OPERATORS

#include "overlay.hpp"
#include "imgui/imgui_impl_win32.h"

#include "imgui/imgui_impl_dx11.h"
#include <dwmapi.h>

#include "../../wrappers/keybind.hpp"
#include <thread>
#include <filesystem>
#include <windows.h>
#include <chrono>
#include <ctime>
#include <d3d11.h>
#include <tchar.h>
#include <vector>
#include <string>

#include "custommenu/imgui_combo.hpp"
#include "custommenu/fonts.h"
#include "custommenu/settings.h"
#include "custommenu/images.h"

using namespace ImGui;

DWORD picker_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview;

#include "imgui/imgui.h"
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <set>
#include "../features/esp/esp.hpp"
#include "../globals/globals.hpp"

#include <D3DX11.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

#ifndef _MIN_MUL
#define _MIN_MUL 0.05
#define _MAX_MUL 0.2
#endif

#ifndef _MIN_DIV
#define _MIN_DIV 3.00
#define _MAX_DIV 10.00
#endif

ID3D11Device* stellar::overlay::d3d11_device = nullptr;
ID3D11DeviceContext* stellar::overlay::d3d11_device_context = nullptr;
IDXGISwapChain* stellar::overlay::dxgi_swap_chain = nullptr;
ID3D11RenderTargetView* stellar::overlay::d3d11_render_target_view = nullptr;

static const char* combo_items_4[4] = { ("Head"), ("UpperTorso"), ("HumanoidRootPart"), ("LowerTorso") };
static const char* macro_items[2] = { ("First Person"), ("I & O Keys") };
static const char* box_items_3[5] = { ("2D Box"), ("2D Dynamic Box"), ("3D Box"), ("3D Dynamic Box"), ("Chams") };
static const char* shake_items[2] = { ("X / Y"), ("Multiplication") };
static const char* pred_items[2] = { ("Division"), ("Multiplication") };
static const char* fovitems[2] = { ("Normal Fov"), ("LGBTQSENSE") };
static const char* head_dotx[2] = { ("Circle"), ("Filled") };
static const char* thread_type[2] = { ("Milliseconds"), ("Microseconds") };
static const char* smooth_typessex[5] = { ("Linear"), ("Bounce"), ("Elastic"), ("Quadratic"), ("Decay") };
static const char* aim_items[2] = { ("Mouse"), ("Camera") };

static const char* prediction_methods[2] = { ("X / Y"), ("Multiplication") };

static const char* hit_sounds[4] = { ("Neverlose"), ("Skeet"), ("Fuck"), ("Senpai") };

bool Keybind(CKeybind* keybind, const ImVec2& size_arg = ImVec2(0, 0), bool clicked = false, ImGuiButtonFlags flags = 0)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;
	// SetCursorPosX(window->Size.x - 14 - size_arg.x);
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(keybind->get_name().c_str());
	const ImVec2 label_size = ImGui::CalcTextSize(keybind->get_name().c_str(), NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) &&
		style.FramePadding.y <
		window->DC.CurrLineTextBaseOffset)  // Try to vertically align buttons that are smaller/have no padding so that
		// text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = ImGui::CalcItemSize(
		size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(size, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	if (g.CurrentItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

	bool value_changed = false;
	int key = keybind->key;

	auto io = ImGui::GetIO();

	std::string name = keybind->get_key_name();

	if (keybind->waiting_for_input)
		name = "...";

	if (ImGui::GetIO().MouseClicked[0] && hovered)
	{
		if (g.ActiveId == id)
		{
			keybind->waiting_for_input = true;
		}
	}
	else if (ImGui::GetIO().MouseClicked[1] && hovered)
	{
		ImGui::OpenPopup(keybind->get_name().c_str());
	}
	else if (ImGui::GetIO().MouseClicked[0] && !hovered)
	{
		if (g.ActiveId == id)
			ImGui::ClearActiveID();
	}

	if (keybind->waiting_for_input)
	{
		if (ImGui::GetIO().MouseClicked[0] && !hovered)
		{
			keybind->key = VK_LBUTTON;

			ImGui::ClearActiveID();
			keybind->waiting_for_input = false;
		}
		else
		{
			if (keybind->set_key())
			{
				ImGui::ClearActiveID();
				keybind->waiting_for_input = false;
			}
		}
	}

	// Render
	ImVec4 textcolor = ImLerp(ImVec4(201 / 255.f, 204 / 255.f, 210 / 255.f, 1.f), ImVec4(1.0f, 1.0f, 1.0f, 1.f), 1.f);

	window->DrawList->AddRectFilled(bb.Min, bb.Max, ImColor(33 / 255.0f, 33 / 255.0f, 33 / 255.0f, 1.f), 2.f);
	//window->DrawList->AddRect( bb.Min, bb.Max, ImColor( 0.f, 0.f, 0.f, 1.f ) );

	window->DrawList->AddText(
		bb.Min +
		ImVec2(
			size_arg.x / 2 - ImGui::CalcTextSize(name.c_str()).x / 2,
			size_arg.y / 2 - ImGui::CalcTextSize(name.c_str()).y / 2),
		ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)),
		name.c_str());

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar;
	//ImGui::SetNextWindowPos( pos + ImVec2( 0, size_arg.y - 1 ) );
	//ImGui::SetNextWindowSize( ImVec2( size_arg.x, 47 * 1.f ) );

	{
		if (ImGui::BeginPopup(keybind->get_name().c_str(), 0))
		{

			{
				{
					ImGui::BeginGroup();
					{
						if (ImGui::Selectable("HOLD", keybind->type == CKeybind::HOLD))
							keybind->type = CKeybind::HOLD;
						if (ImGui::Selectable("TOGGLE", keybind->type == CKeybind::TOGGLE))
							keybind->type = CKeybind::TOGGLE;
					}
					ImGui::EndGroup();
				}
			}

			ImGui::EndPopup();
		}
	}

	return pressed;
}

bool stellar::overlay::fullsc(HWND windowHandle) {
	MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
	if (GetMonitorInfo(MonitorFromWindow(windowHandle, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
	{
		RECT windowRect;
		if (GetWindowRect(windowHandle, &windowRect))
		{
			return windowRect.left == monitorInfo.rcMonitor.left
				&& windowRect.right == monitorInfo.rcMonitor.right
				&& windowRect.top == monitorInfo.rcMonitor.top
				&& windowRect.bottom == monitorInfo.rcMonitor.bottom;
		}
	}
}

bool stellar::overlay::init = false;

bool stellar::overlay::render() {
	ImGui_ImplWin32_EnableDpiAwareness();

	WNDCLASSEX wc;
	wc.cbClsExtra = NULL;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.cbWndExtra = NULL;
	wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpfnWndProc = window_proc;
	wc.lpszClassName = TEXT("stellarontop");
	wc.lpszMenuName = nullptr;
	wc.style = CS_VREDRAW | CS_HREDRAW;

	RegisterClassEx(&wc);
	const HWND hw = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE, wc.lpszClassName, TEXT("stellar"),
		WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, wc.hInstance, nullptr);

	SetLayeredWindowAttributes(hw, 0, 255, LWA_ALPHA);
	const MARGINS margin = { -1 };
	DwmExtendFrameIntoClientArea(hw, &margin);

	if (!create_device_d3d(hw)) {
		cleanup_device_d3d();
		UnregisterClass(wc.lpszClassName, wc.hInstance);
		return false;
	}

	ShowWindow(hw, SW_SHOW);
	UpdateWindow(hw);

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui::GetIO().IniFilename = nullptr;

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4 originalButtonColor = style.Colors[ImGuiCol_Button];

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	RECT screen_rect;
	GetWindowRect(GetDesktopWindow(), &screen_rect);
	auto x = float(screen_rect.right - 640) / 2.f;
	auto y = float(screen_rect.bottom - 520) / 2.f;

	ImFontConfig cfg;

	fonts::inter_font = io.Fonts->AddFontFromMemoryTTF(&inter, sizeof inter, 17, NULL, io.Fonts->GetGlyphRangesCyrillic());
	fonts::inter_font_b = io.Fonts->AddFontFromMemoryTTF(&inter, sizeof inter, 18.5f, NULL, io.Fonts->GetGlyphRangesCyrillic());
	fonts::inter_bold_font = io.Fonts->AddFontFromMemoryTTF(&inter_bold, sizeof inter_bold, 20, NULL, io.Fonts->GetGlyphRangesCyrillic());
	fonts::inter_bold_font2 = io.Fonts->AddFontFromMemoryTTF(&inter_bold, sizeof inter_bold, 17, NULL, io.Fonts->GetGlyphRangesCyrillic());
	fonts::inter_bold_font3 = io.Fonts->AddFontFromMemoryTTF(&inter_bold, sizeof inter_bold, 18, NULL, io.Fonts->GetGlyphRangesCyrillic());
	fonts::inter_bold_font4 = io.Fonts->AddFontFromMemoryTTF(&inter_bold, sizeof inter_bold, 16, NULL, io.Fonts->GetGlyphRangesCyrillic());
	fonts::combo_icon_font = io.Fonts->AddFontFromMemoryTTF(&combo_icon, sizeof combo_icon, 15, NULL, io.Fonts->GetGlyphRangesCyrillic());
	fonts::weapon_font = io.Fonts->AddFontFromMemoryTTF(&weapon, sizeof weapon, 15, NULL, io.Fonts->GetGlyphRangesCyrillic());

	D3DX11_IMAGE_LOAD_INFO image; ID3DX11ThreadPump* pump{ nullptr };

	if (pictures::logo_img == nullptr) D3DX11CreateShaderResourceViewFromMemory(d3d11_device, &logo, sizeof(logo), &image, pump, &pictures::logo_img, 0);
	if (pictures::aim_img == nullptr) D3DX11CreateShaderResourceViewFromMemory(d3d11_device, &aim, sizeof(aim), &image, pump, &pictures::aim_img, 0);
	if (pictures::misc_img == nullptr) D3DX11CreateShaderResourceViewFromMemory(d3d11_device, &other, sizeof(other), &image, pump, &pictures::misc_img, 0);
	if (pictures::visual_img == nullptr) D3DX11CreateShaderResourceViewFromMemory(d3d11_device, &visual, sizeof(visual), &image, pump, &pictures::visual_img, 0);

	if (pictures::trigger_img == nullptr) D3DX11CreateShaderResourceViewFromMemory(d3d11_device, &trigger, sizeof(trigger), &image, pump, &pictures::trigger_img, 0);

	if (pictures::pen_img == nullptr) D3DX11CreateShaderResourceViewFromMemory(d3d11_device, &pen, sizeof(pen), &image, pump, &pictures::pen_img, 0);
	if (pictures::world_img == nullptr) D3DX11CreateShaderResourceViewFromMemory(d3d11_device, &world, sizeof(world), &image, pump, &pictures::world_img, 0);
	if (pictures::settings_img == nullptr) D3DX11CreateShaderResourceViewFromMemory(d3d11_device, setting, sizeof(setting), &image, pump, &pictures::settings_img, 0);
	if (pictures::keyboard_img == nullptr) D3DX11CreateShaderResourceViewFromMemory(d3d11_device, keyboard, sizeof(keyboard), &image, pump, &pictures::keyboard_img, 0);
	if (pictures::input_img == nullptr) D3DX11CreateShaderResourceViewFromMemory(d3d11_device, inputimg, sizeof(inputimg), &image, pump, &pictures::input_img, 0);
	if (pictures::wat_logo_img == nullptr) D3DX11CreateShaderResourceViewFromMemory(d3d11_device, wat, sizeof(wat), &image, pump, &pictures::wat_logo_img, 0);
	if (pictures::fps_img == nullptr) D3DX11CreateShaderResourceViewFromMemory(d3d11_device, fps, sizeof(fps), &image, pump, &pictures::fps_img, 0);
	if (pictures::player_img == nullptr) D3DX11CreateShaderResourceViewFromMemory(d3d11_device, player, sizeof(player), &image, pump, &pictures::player_img, 0);
	if (pictures::time_img == nullptr) D3DX11CreateShaderResourceViewFromMemory(d3d11_device, timse, sizeof(timse), &image, pump, &pictures::time_img, 0);

	ImGui_ImplWin32_Init(hw);
	ImGui_ImplDX11_Init(d3d11_device, d3d11_device_context);

	const ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	init = true;

	bool draw = false;
	bool done = false;
	bool has_initialized = false;

	while (!done) {
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}

		if (done) 
			break;

		move_window(hw);

		if (GetAsyncKeyState(VK_INSERT) & 1)
			draw = !draw;

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		{
			if (globals::is_focused || GetForegroundWindow() == hw)
			{
				ImGuiStyle* style = &ImGui::GetStyle();

				style->WindowPadding = ImVec2(0, 0);
				style->ItemSpacing = ImVec2(10, 10);
				style->WindowBorderSize = 0;
				style->ScrollbarSize = 8.f;

				ImGui::PushStyleColor(ImGuiCol_WindowShadow, ImVec4(0, 0, 0, 0.0f));

				ImGui::Begin("overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);
				{
					stellar::cheats::esp();
					ImGui::End();
				}
				ImGui::PopStyleColor();


				if (draw) {
					ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Once);
					ImGui::SetNextWindowSize(settings::size_menu);
					ImGui::Begin("menuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);

					double _pred_min = globals::prediction_method == 0 ? _MIN_DIV : _MIN_MUL;
					double _pred_max = globals::prediction_method == 0 ? _MAX_DIV : _MAX_MUL;

					const auto& pos = ImGui::GetWindowPos();
					const auto& draw_list = ImGui::GetWindowDrawList();
					ImGuiStyle* style = &ImGui::GetStyle();

					{
						style->Colors[ImGuiCol_WindowBg] = colors::menu::window_bg;
						style->Colors[ImGuiCol_Border] = colors::menu::border;

						style->ItemSpacing = ImVec2(0, 5);
						style->WindowPadding = ImVec2(0, 0);
						style->WindowRounding = 8.f;
					}

					{
						ImGui::SetCursorPos(ImVec2(10, 10));
						ImGui::BeginChild("General Tabs", ImVec2(620, 60), true, ImGuiWindowFlags_NoBackground);

						const auto& pos = ImGui::GetWindowPos();
						const auto& draw_list = ImGui::GetWindowDrawList();

						ImGui::GetStyle().AntiAliasedLines = true;
						ImGui::GetStyle().AntiAliasedLinesUseTex = true;
						ImGui::GetStyle().AntiAliasedFill = true;

						draw_list->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + 620, pos.y + 60), ImGui::GetColorU32(menu::general_child), 10.f);
						draw_list->AddImage(pictures::logo_img, ImVec2(pos.x + 2, pos.y + 5), ImVec2(pos.x + 52, pos.y + 58), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255));

						draw_list->AddText(fonts::inter_bold_font, 20.f, ImVec2(pos.x + 56, pos.y + 20), ImColor(255, 255, 255), "STELLAR");

						draw_list->AddRectFilledMultiColor(ImVec2(pos.x + 144, pos.y + 15), ImVec2(pos.x + 145.5f, pos.y + 30), ImGui::GetColorU32(colors::accent_color, 0), ImGui::GetColorU32(colors::accent_color, 0), ImGui::GetColorU32(colors::accent_color), ImGui::GetColorU32(colors::accent_color));
						draw_list->AddRectFilledMultiColor(ImVec2(pos.x + 144, pos.y + 30), ImVec2(pos.x + 145.5f, pos.y + 45), ImGui::GetColorU32(colors::accent_color), ImGui::GetColorU32(colors::accent_color), ImGui::GetColorU32(colors::accent_color, 0), ImGui::GetColorU32(colors::accent_color, 0));

						{
							{
								if (misc::tab_count == 1)
									misc::tab_width = 92;
								else if (misc::tab_count == 2)
									misc::tab_width = 96;
								else if (misc::tab_count == 3)
									misc::tab_width = 92;
								else if (misc::tab_count == 4)
									misc::tab_width = 91;
							}
							misc::anim_tab = ImLerp(misc::anim_tab, (float)(misc::tab_count * misc::tab_width), ImGui::GetIO().DeltaTime * 15.f);

							ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(pos.x + 175 + misc::anim_tab, pos.y + 57), ImVec2(pos.x + 235 + misc::anim_tab, pos.y + 60), ImColor(colors::accent_color), 10, ImDrawCornerFlags_Top);
							ImGui::GetWindowDrawList()->AddShadowRect(ImVec2(pos.x + 175 + misc::anim_tab, pos.y + 57), ImVec2(pos.x + 235 + misc::anim_tab, pos.y + 60), ImGui::GetColorU32(colors::accent_color), 10.f, ImVec2(0, 0), ImDrawCornerFlags_Top, 10.f);

							ImGui::SetCursorPos(ImVec2(155, 12));
							ImGui::BeginGroup(); {
								if (ImGui::Tab("Aimbot", pictures::aim_img, ImVec2(87, 40), 0 == misc::tab_count))
									misc::tab_count = 0;

								ImGui::SameLine();

								if (ImGui::Tab("Visuals", pictures::visual_img, ImVec2(93, 40), 1 == misc::tab_count))
									misc::tab_count = 1;

								ImGui::SameLine();

								if (ImGui::Tab("Movement", pictures::player_img, ImVec2(100, 40), 2 == misc::tab_count))
									misc::tab_count = 2;

								ImGui::SameLine();

								if (ImGui::Tab("Triggerbot", pictures::trigger_img, ImVec2(100, 40), 3 == misc::tab_count))
									misc::tab_count = 3;

								ImGui::SameLine();

								if (ImGui::Tab("Config", pictures::misc_img, ImVec2(95, 40), 4 == misc::tab_count))
									misc::tab_count = 4;
							}
							ImGui::EndGroup();
						}
						ImGui::EndChild();
					}

					{
						misc::alpha_child = ImLerp(misc::alpha_child, (misc::tab_count == misc::active_tab_count) ? 1.f : 0.f, 15.f * ImGui::GetIO().DeltaTime);
						if (misc::alpha_child < 0.01f && misc::child_add < 0.01f) misc::active_tab_count = misc::tab_count;

						ImGui::SetCursorPos(ImVec2(10, 80));
						ImGui::BeginChild("Main", ImVec2(725, 440), true, ImGuiWindowFlags_NoBackground);

						ImGui::SetCursorPos(ImVec2(0, 100 - (misc::alpha_child * 100)));
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, misc::alpha_child * style->Alpha);

						if (misc::active_tab_count == 0)
						{
							ImGui::BeginGroup();
							{
								ImGui::BeginChildCustom(pictures::aim_img, "Aimbot", ImVec2(304, 540), false, ImGuiWindowFlags_NoScrollWithMouse);
								{
									ImGui::Checkbox("Enabled", &globals::aimbot);

									Keybind(&globals::aimbot_key, ImVec2(60, 20), "Keybind");

									combo::Combo("Type", &globals::aimbot_type, aim_items, IM_ARRAYSIZE(aim_items), 2);

									combo::Combo("BodyPart", &globals::aim_part, combo_items_4, IM_ARRAYSIZE(combo_items_4), 4);
									//combo::Combo("Easing Style", &globals::smooth_type, smooth_typessex, IM_ARRAYSIZE(smooth_typessex), 5);
									ImGui::Checkbox("AntiLock", &globals::anti_lock);


									ImGui::Checkbox("Prediction", &globals::prediction);
									ImGui::Checkbox("Smoothness", &globals::smoothness);
									//ImGui::Checkbox("Shake", &globals::shake);
									ImGui::Checkbox("Auto Resolver", &globals::auto_resolve);
									ImGui::Checkbox("Disable outside of FOV", &globals::disable_outside_fov);
									ImGui::Checkbox("VSync", &globals::vsync);
								}
								ImGui::EndChildCustom();

								ImGui::BeginChildCustom(pictures::aim_img, "Misc", ImVec2(304, 320), false, ImGuiWindowFlags_NoScrollWithMouse);
								{
									if (globals::shake)
										combo::Combo("Shake Type", &globals::shake_type, shake_items, IM_ARRAYSIZE(shake_items), 2);

									if (globals::aimbot_type == 0)
										ImGui::SliderFloat("Sensitivity", &globals::sensitivity, 0.5f, 2.5f, "%.1f");

									if (globals::aimbot_range_enable)
										ImGui::SliderInt("Range Value", &globals::aimbot_range, 1, 100);

									if (globals::auto_resolve)
										ImGui::SliderFloat("Velocity Threshold", &globals::velocity_threshold, 1, 100);

									if (globals::prediction) {
										//combo::Combo("Prediction Type", &globals::prediction_method, pred_items, IM_ARRAYSIZE(pred_items), 2);
										ImGui::Checkbox("Distance Based Prediction", &globals::distance_prediction);
										ImGui::Checkbox("Separate Prediction", &globals::separate_predictions);
									}
								}
								ImGui::EndChildCustom();

								ImGui::BeginChildCustom(pictures::aim_img, "Aimbot", ImVec2(304, 190), false, ImGuiWindowFlags_NoScrollWithMouse);
								{
									ImGui::SetCursorPos(ImVec2(10, 50));
									ImGui::BeginGroup(); {

									} 
									ImGui::EndGroup();
								}
								ImGui::EndChildCustom();

							} ImGui::EndGroup();

							ImGui::SameLine(0, 10.f);

							ImGui::BeginGroup();
							{
								ImGui::BeginChildCustom(pictures::aim_img, "Accuracy", ImVec2(304, 650), false, ImGuiWindowFlags_NoScrollWithMouse);
								{
									if (globals::game_id == 2788229376 || globals::game_id == 16033173781) {
										ImGui::Checkbox("K.O Check", &globals::ko_check);
										ImGui::Checkbox("Crew Check", &globals::crew_check);
									}

									static int key1 = 0;

									if (globals::prediction_method == 0) {
										if (globals::main_prediction < _MIN_DIV)
											globals::main_prediction = globals::close_prediction = globals::mid_prediction = globals::far_prediction = _MIN_DIV;

										if (globals::prediction_x < _MIN_DIV)
											globals::prediction_x = globals::close_prediction_x = globals::mid_prediction_x = globals::far_prediction_x = _MIN_DIV;

										if (globals::prediction_y < _MIN_DIV)
											globals::prediction_y = globals::close_prediction_y = globals::mid_prediction_y = globals::far_prediction_y = _MIN_DIV;
									}
									else {
										if (globals::main_prediction > _MAX_MUL)
											globals::main_prediction = globals::close_prediction = globals::mid_prediction = globals::far_prediction = _MAX_MUL;

										if (globals::prediction_x > _MAX_MUL)
											globals::prediction_x = globals::close_prediction_x = globals::mid_prediction_x = globals::far_prediction_x = _MAX_MUL;

										if (globals::prediction_y > _MAX_MUL)
											globals::prediction_y = globals::close_prediction_y = globals::mid_prediction_y = globals::far_prediction_y = _MAX_MUL;
									}


									if (globals::distance_prediction) {
										ImGui::SliderFloat("Close Distance", &globals::close_distance, 1.00f, 100.00f);

										if (globals::separate_predictions) {
											ImGui::SliderFloat("Close Prediction X", &globals::close_prediction_x, _pred_min, _pred_max, "%.0005f");
											ImGui::SliderFloat("Close Prediction Y", &globals::close_prediction_y, _pred_min, _pred_max, "%.0005f");
										}
										else
											ImGui::SliderFloat("Close Prediction", &globals::close_prediction, _pred_min, _pred_max, "%.0005f");

										ImGui::SliderFloat("Mid Distance", &globals::mid_distance, 1.00f, 100.00f, "%.1f");

										if (globals::separate_predictions) {
											ImGui::SliderFloat("Mid Prediction X", &globals::mid_prediction_x, _pred_min, _pred_max, "%.0005f");
											ImGui::SliderFloat("Mid Prediction Y", &globals::mid_prediction_y, _pred_min, _pred_max, "%.0005f");
										}
										else
											ImGui::SliderFloat("Mid Prediction", &globals::mid_prediction, _pred_min, _pred_max, "%.0005f");

										if (globals::separate_predictions) {
											ImGui::SliderFloat("Far Prediction X", &globals::far_prediction_x, _pred_min, _pred_max, "%.0005f");
											ImGui::SliderFloat("Far Prediction Y", &globals::far_prediction_y, _pred_min, _pred_max, "%.0005f");
										}
										else
											ImGui::SliderFloat("Far Prediction", &globals::far_prediction, _pred_min, _pred_max, "%.0005f");
									}
									else {
										if (globals::separate_predictions) {
											ImGui::SliderFloat("Prediction X", &globals::prediction_x, _pred_min, _pred_max, "%.4f");
											ImGui::SliderFloat("Prediction Y", &globals::prediction_y, _pred_min, _pred_max, "%.4tf");
										}
										else
											ImGui::SliderFloat("Prediction", &globals::main_prediction, _pred_min, _pred_max, "%.0005f");
									}

									if (globals::smoothness) {
										if (globals::aimbot_type == 0) {
											ImGui::SliderFloat("Smoothness X", &globals::smoothness_x, 1.0f, 100.0f, "%.1f");
											ImGui::SliderFloat("Smoothness Y", &globals::smoothness_y, 1.0f, 100.0f, "%.1f");
										} else if (globals::aimbot_type == 1)
											ImGui::SliderFloat("Smoothness", &globals::smoothness_xy, 0.1f, 1.5f, "%.4f");
									}

									if (globals::shake)
										if (globals::shake_type == 0) {
											//ImGui::SliderFloat("Shake X", &globals::shake_x, 0.1, 10.0, "%.1f");
											//ImGui::SliderFloat("Shake Y", &globals::shake_y, 0.1, 10.0, "%.1f");
										}
										//else if (globals::shake_type == 1)
											//ImGui::SliderFloat("Shake Value", &globals::shake_xy, 0.1, 10.0, "%.1f");
								}
								ImGui::EndChildCustom();
							}
							ImGui::EndGroup();
						}
						else if (misc::active_tab_count == 1) {
							ImGui::BeginGroup();
							{
								ImGui::BeginChildCustom(pictures::visual_img, "ESP", ImVec2(304, 520), false, ImGuiWindowFlags_NoScrollWithMouse);
								{
									ImGui::Checkbox("Enable ESP", &globals::esp);
									ImGui::Checkbox("Box ESP", &globals::box);
									
									if (globals::box) {
										combo::Combo("Box Type", &globals::box_type, box_items_3, IM_ARRAYSIZE(box_items_3), 4);
										ImGui::ColorEdit4("Box Color", globals::box_color, picker_flags);
									}
									
									ImGui::Checkbox("Skeleton ESP", &globals::skeleton);
									
									if (globals::game_id == 2788229376 || globals::game_id == 16033173781)
										ImGui::Checkbox("Armor Bar", &globals::armor_bar);
								}
								ImGui::EndChildCustom();
							} 
							ImGui::EndGroup();
							ImGui::SameLine(0, 10.f);
							ImGui::BeginGroup();
							{
								ImGui::BeginChildCustom(pictures::visual_img, "FOV & Visuals", ImVec2(304, 520), false, ImGuiWindowFlags_NoScrollWithMouse);
								{
									ImGui::Checkbox("FOV Circle", &globals::fov_enabled);

									if (globals::fov_enabled) {
										ImGui::ColorEdit4("FOV Color", globals::fov_color, picker_flags);
										ImGui::SliderInt("FOV Size", &globals::fov, 50, 500);
									}

									ImGui::Checkbox("Deadzone", &globals::deadzone);
									if (globals::deadzone) {
										ImGui::ColorEdit4("Deadzone Color", globals::deadzone_color, picker_flags);
										ImGui::ColorEdit4("Deadzone Fill", globals::deadzone_fill_color, picker_flags);
									}

									ImGui::Checkbox("Streamproof", &globals::streamproof);
									ImGui::Checkbox("VSync", &globals::vsync);
								}
								ImGui::EndChildCustom();
							} 
							ImGui::EndGroup();
						}
						else if (misc::active_tab_count == 2) {
							ImGui::BeginGroup();
							{
								ImGui::BeginChildCustom(pictures::player_img, "Movement", ImVec2(304, 400), false, ImGuiWindowFlags_NoScrollWithMouse);
								{
									ImGui::Checkbox("Speed Enabled", &globals::speed_enabled);
									
									Keybind(&globals::speed_key, ImVec2(60, 20), "Speed Key");
									
									static const char* speed_types[2] = { "Walkspeed", "Velocity" };
									combo::Combo("Speed Type", &globals::speed_type, speed_types, 2, 2);
									
									ImGui::SliderFloat("Speed Value", &globals::speed_value, 16.0f, 1000.0f, "%.1f");
									
									ImGui::Text("Default Speed: 16.0");
									ImGui::Text("Current Speed: %.1f", globals::speed_value);
									
									if (globals::speed_type == 0) {
										ImGui::Text("Type: Walkspeed (Fast & Aggressive)");
									} else if (globals::speed_type == 1) {
										ImGui::Text("Type: Velocity (Medium Risk)");
									}
								}
								ImGui::EndChildCustom();
							} 
							ImGui::EndGroup();
						}
						else if (misc::active_tab_count == 3) {
							ImGui::BeginGroup();
							{
								ImGui::BeginChildCustom(pictures::misc_img, "NOT READY", ImVec2(304, 400), false, ImGuiWindowFlags_NoScrollWithMouse);
								{
									ImGui::Checkbox("Enabled", &globals::triggerbot);
									ImGui::Checkbox("Knife Check", &globals::knife_check);
									ImGui::Checkbox("Aimbot Connect", &globals::triggerbot_shoot_on_aimbot);
									
									ImGui::Checkbox("Visualize Hitbox", &globals::triggerbot_visualize);
									ImGui::SliderFloat("Hitbox Size X", &globals::triggerbot_hitbox_x, 0.1, 1.5, "%.1f");

									ImGui::SliderFloat("Hitbox Size Y", &globals::triggerbot_hitbox_y, 1, 3, "%.1f");

									Keybind(&globals::triggerbot_key, ImVec2(60, 20));
								}
								ImGui::EndChildCustom();
							} 
							ImGui::EndGroup();

							ImGui::SameLine(0, 10.f);

							ImGui::BeginGroup();
							{
								ImGui::BeginChildCustom(pictures::aim_img, "Accuracy", ImVec2(304, 650), false, ImGuiWindowFlags_NoScrollWithMouse);
								{
									ImGui::Checkbox("Prediction", &globals::triggerbot_prediction);
									ImGui::Checkbox("Distance Prediction", &globals::triggerbot_distance_prediction);

									if (globals::triggerbot_prediction)
										if (globals::triggerbot_distance_prediction) {
											ImGui::SliderFloat("Close Distance", &globals::triggerbot_close_distance, 1.00f, 100.00f);
											ImGui::SliderFloat("Close Prediction X", &globals::triggerbot_close_prediction_x, _pred_min, _pred_max, "%.0005f");
											ImGui::SliderFloat("Close Prediction Y", &globals::triggerbot_close_prediction_y, _pred_min, _pred_max, "%.0005f");

											ImGui::SliderFloat("Mid Distance", &globals::triggerbot_mid_distance, 1.00f, 100.00f, "%.1f");
											ImGui::SliderFloat("Mid Prediction X", &globals::triggerbot_mid_prediction_x, _pred_min, _pred_max, "%.0005f");
											ImGui::SliderFloat("Mid Prediction Y", &globals::triggerbot_mid_prediction_y, _pred_min, _pred_max, "%.0005f");

											ImGui::SliderFloat("Far Prediction X", &globals::triggerbot_far_prediction_x, _pred_min, _pred_max, "%.0005f");
											ImGui::SliderFloat("Far Prediction Y", &globals::triggerbot_far_prediction_y, _pred_min, _pred_max, "%.0005f");
										}
										else {
											ImGui::SliderFloat("Prediction X", &globals::triggerbot_prediction_x, _pred_min, _pred_max, "%.4f");
											ImGui::SliderFloat("Prediction Y", &globals::triggerbot_prediction_y, _pred_min, _pred_max, "%.4tf");
										}
								}
								ImGui::EndChildCustom();
							}
							ImGui::EndGroup();
						}
						else if (misc::active_tab_count == 4) {
							ImGui::BeginGroup();
							{
								ImGui::BeginChildCustom(pictures::misc_img, "Config", ImVec2(304, 240), false, ImGuiWindowFlags_NoScrollWithMouse);
								{
									// Config content here
								}
								ImGui::EndChildCustom();
							} 
							ImGui::EndGroup();
						}

						ImGui::PopStyleVar();
						ImGui::Spacing();
						ImGui::EndChild();
					}
					ImGui::End();
				}

				{
					features::preview_alpha = ImClamp(features::preview_alpha + (4.f * ImGui::GetIO().DeltaTime * (features::esp_perview ? 1.f : -1.f)), 0.f, 1.f);

					ImGuiStyle* style = &ImGui::GetStyle();
					ImGuiContext& g = *GImGui;

					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, features::preview_alpha* style->Alpha);

					ImGui::SetNextWindowPos(ImVec2(x + 670, 253), ImGuiCond_Once);
					ImGui::SetNextWindowSize(settings::size_preview);

					if (features::esp_perview) {
						ImGui::Begin("Esp Preview Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus);

						const auto& pos = ImGui::GetWindowPos();
						const auto& draw_list = ImGui::GetWindowDrawList();

						{
							if (features::multi_esp[0] == true)
								draw_list->AddRect(ImVec2(pos.x + 50, pos.y + 50), ImVec2(pos.x + 250, pos.y + 350), ImGui::GetColorU32(colors::preview::rect), 4.f);

							if (features::multi_esp[1] == true) {
								draw_list->AddRectFilled(ImVec2(pos.x + 43, pos.y + 50), ImVec2(pos.x + 45, pos.y + 350), ImGui::GetColorU32(colors::accent_color), 4.f);
								draw_list->AddShadowRect(ImVec2(pos.x + 43, pos.y + 50), ImVec2(pos.x + 45, pos.y + 350), ImGui::GetColorU32(colors::accent_color), 10.f, ImVec2(0, 0), 0, 4.f);
							}

							if (features::multi_esp[2] == true) {
								draw_list->AddRectFilled(ImVec2(pos.x + 50, pos.y + 355), ImVec2(pos.x + 250, pos.y + 357), ImGui::GetColorU32(colors::accent_color), 4.f);
								draw_list->AddShadowRect(ImVec2(pos.x + 50, pos.y + 355), ImVec2(pos.x + 250, pos.y + 357), ImGui::GetColorU32(colors::accent_color), 10.f, ImVec2(0, 0), 0, 4.f);
							}

							if (features::multi_esp[3] == true)
								draw_list->AddText(fonts::inter_bold_font2, 17.f, ImVec2(pos.x + 120, pos.y + 28), ImGui::GetColorU32(colors::preview::name), "nickname");

							if (features::multi_esp[4] == true)
								draw_list->AddText(fonts::inter_bold_font2, 17.f, ImVec2(pos.x + 260, pos.y + 52), ImGui::GetColorU32(colors::preview::distance), "29m");

							if (features::multi_esp[5] == true)
								draw_list->AddText(fonts::weapon_font, 15.f, ImVec2(pos.x + 133, pos.y + 367), ImGui::GetColorU32(colors::preview::distance), "W");

							if (features::multi_esp[6] == true) {
								draw_list->AddCircleFilled(ImVec2(pos.x + 152, pos.y + 105), 11.f, ImGui::GetColorU32(colors::preview::head), 20.f);
								draw_list->AddShadowCircle(ImVec2(pos.x + 152, pos.y + 105), 11.f, ImGui::GetColorU32(colors::preview::head), 25.f, ImVec2(0, 0));
								draw_list->AddLine(ImVec2(pos.x + 152, pos.y + 135), ImVec2(pos.x + 152, pos.y + 240), ImGui::GetColorU32(colors::preview::dice));

								// left hand
								draw_list->AddLine(ImVec2(pos.x + 100, pos.y + 180), ImVec2(pos.x + 152, pos.y + 135), ImGui::GetColorU32(colors::preview::dice));

								// right hand
								draw_list->AddLine(ImVec2(pos.x + 152, pos.y + 135), ImVec2(pos.x + 204, pos.y + 180), ImGui::GetColorU32(colors::preview::dice));

								// left leg
								draw_list->AddLine(ImVec2(pos.x + 100, pos.y + 295), ImVec2(pos.x + 152, pos.y + 240), ImGui::GetColorU32(colors::preview::dice));

								// right leg
								draw_list->AddLine(ImVec2(pos.x + 152, pos.y + 240), ImVec2(pos.x + 204, pos.y + 295), ImGui::GetColorU32(colors::preview::dice));
							}
						}
						ImGui::End();
					}
					ImGui::PopStyleVar();
				}
			}

			if (globals::streamproof)
				SetWindowDisplayAffinity(hw, WDA_EXCLUDEFROMCAPTURE);
			else
				SetWindowDisplayAffinity(hw, WDA_NONE);

				//if (globals::show_auth)
			if (draw)
				SetWindowLong(hw, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW);
			else
				SetWindowLong(hw, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW);

			ImGui::EndFrame();
			ImGui::Render();

			const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
			d3d11_device_context->OMSetRenderTargets(1, &d3d11_render_target_view, nullptr);
			d3d11_device_context->ClearRenderTargetView(d3d11_render_target_view, clear_color_with_alpha);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

			if (globals::vsync)
				dxgi_swap_chain->Present(1, 0);
			else
				dxgi_swap_chain->Present(0, 0);
		}
	}

	has_initialized = false;
	init = false;

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	cleanup_device_d3d();
	DestroyWindow(hw);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
}

void stellar::overlay::move_window(HWND hw) {
	HWND target = FindWindowA(0, "Roblox");
	HWND foregroundWindow = GetForegroundWindow();

	if (target != foregroundWindow && hw != foregroundWindow)
		MoveWindow(hw, 0, 0, 0, 0, true);
	else {
		RECT rect;
		GetWindowRect(target, &rect);

		int rsize_x = rect.right - rect.left;
		int rsize_y = rect.bottom - rect.top;

		if (fullsc(target)) {
			rsize_x += 16;
			rsize_y -= 24;
		} else {
			rsize_y -= 63;
			rect.left += 8;
			rect.top += 31;
		}

		MoveWindow(hw, rect.left, rect.top, rsize_x, rsize_y, TRUE);
	}
}

bool stellar::overlay::create_device_d3d(HWND hw) {
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hw;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	const UINT create_device_flags = 0;
	D3D_FEATURE_LEVEL d3d_feature_level;
	const D3D_FEATURE_LEVEL feature_level_arr[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, create_device_flags, feature_level_arr, 2, D3D11_SDK_VERSION, &sd, &dxgi_swap_chain, &d3d11_device, &d3d_feature_level, &d3d11_device_context);

	if (res == DXGI_ERROR_UNSUPPORTED) {
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, create_device_flags, feature_level_arr, 2, D3D11_SDK_VERSION, &sd, &dxgi_swap_chain, &d3d11_device, &d3d_feature_level, &d3d11_device_context);
	}
	if (res != S_OK) {
		return false;
	}

	create_render_target();
	return true;
}

void stellar::overlay::cleanup_device_d3d() {
	cleanup_render_target();

	if (dxgi_swap_chain) {
		dxgi_swap_chain->Release();
		dxgi_swap_chain = nullptr;
	}

	if (d3d11_device_context) {
		d3d11_device_context->Release();
		d3d11_device_context = nullptr;
	}

	if (d3d11_device) {
		d3d11_device->Release();
		d3d11_device = nullptr;
	}
}

void stellar::overlay::create_render_target() {
	ID3D11Texture2D* d3d11_back_buffer;
	dxgi_swap_chain->GetBuffer(0, IID_PPV_ARGS(&d3d11_back_buffer));
	if (d3d11_back_buffer != nullptr) {
		d3d11_device->CreateRenderTargetView(d3d11_back_buffer, nullptr, &d3d11_render_target_view);
		d3d11_back_buffer->Release();
	}
}

void stellar::overlay::cleanup_render_target() {
	if (d3d11_render_target_view) {
		d3d11_render_target_view->Release();
		d3d11_render_target_view = nullptr;
	}
}

LRESULT __stdcall stellar::overlay::window_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg) {
	case WM_SIZE:
		if (d3d11_device != nullptr && wParam != SIZE_MINIMIZED) {
			cleanup_render_target();
			dxgi_swap_chain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			create_render_target();
		}
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}