#pragma once

#ifndef OVERLAY_HPP
#define OVERLAY_HPP

#include <Windows.h>

#include <d3d11.h>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace stellar {
	class overlay
	{
	public:
		static bool render();
		static void move_window(HWND hw);
		static bool fullsc(HWND windowHandle);
	private:
		static bool create_device_d3d(HWND hw);
		static void cleanup_device_d3d();
		static void create_render_target();
		static void cleanup_render_target();
		static LRESULT WINAPI window_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	private:
		static ID3D11Device* d3d11_device;
		static ID3D11DeviceContext* d3d11_device_context;
		static IDXGISwapChain* dxgi_swap_chain;
		static ID3D11RenderTargetView* d3d11_render_target_view;
		static bool init;
	};
}

#endif