#include <Aurie/shared.hpp>
#include <YYToolkit/Shared.hpp>
#include <map>

#define CIMGUI_API __declspec(dllexport)
#include "cimgui.cpp" // NOLINT(bugprone-suspicious-include) lmao

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include "ImGuiWrapper.h"

using namespace Aurie;
using namespace YYTK;

enum ImGuiInitState
{
    FetchingSwapchain = 0,
    CreatingContext = 1,
    CreatingContext2 = 2,
    Done = 3,
    Failed = -1
};

static AurieModule* g_module = nullptr;
static YYTKInterface* g_module_interface = nullptr;

static ImGuiInitState g_init_state = FetchingSwapchain;
static ID3D11Device* g_device = nullptr;
static ID3D11DeviceContext* g_context = nullptr;
static IDXGISwapChain* g_swapchain = nullptr;
static ID3D11RenderTargetView* g_render_target_view = nullptr;
static bool g_rendering = true;


static ImGuiWrapper g_wrapper;

void draw()
{
    for (auto cb : g_wrapper.draw_callbacks)
    {
        cb();
    }
}

void present_detour(YYTK::FWFrame& FrameContext)
{
    auto swapchain = std::get<0>(FrameContext.Arguments());
    if (g_init_state == FetchingSwapchain)
    {
        g_swapchain = swapchain;
        g_init_state = CreatingContext;
    }

    if (g_init_state == Done)
    {
        if (!g_rendering) return;
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();

        auto io = ImGui::GetIO();
        if ((io.WantCaptureMouse || io.MouseDrawCursor) && !(io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange))
        {
            static std::map<ImGuiMouseCursor, HCURSOR> Cursor = {
                {ImGuiMouseCursor_Arrow, LoadCursor(nullptr, IDC_ARROW)},
                {ImGuiMouseCursor_TextInput, LoadCursor(nullptr, IDC_IBEAM)},
                {ImGuiMouseCursor_ResizeAll, LoadCursor(nullptr, IDC_SIZEALL)},
                {ImGuiMouseCursor_ResizeNS, LoadCursor(nullptr, IDC_SIZENS)},
                {ImGuiMouseCursor_ResizeEW, LoadCursor(nullptr, IDC_SIZEWE)},
                {ImGuiMouseCursor_ResizeNESW, LoadCursor(nullptr, IDC_SIZENESW)},
                {ImGuiMouseCursor_ResizeNWSE, LoadCursor(nullptr, IDC_SIZENWSE)},
                {ImGuiMouseCursor_Hand, LoadCursor(nullptr, IDC_HAND)},
                {ImGuiMouseCursor_NotAllowed, LoadCursor(nullptr, IDC_NO)}
            };
            auto cursor = ImGui::GetMouseCursor();

            if (cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
            {
                SetCursor(nullptr);
            }
            else
            {
                SetCursor(Cursor[cursor]);
            }
        }

        draw();
        ImGui::Render();
        g_context->OMSetRenderTargets(1, &g_render_target_view, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }
}

void resize_detour(YYTK::FWResize& ResizeContext)
{
    if (g_init_state == Done)
    {
        if (g_render_target_view != nullptr)
        {
            g_context->OMSetRenderTargets(0, nullptr, nullptr);
            g_render_target_view->Release();
        }

        ResizeContext.Call();

        ID3D11Texture2D* backBuffer = nullptr;
        g_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
        g_device->CreateRenderTargetView(backBuffer, nullptr, &g_render_target_view);
        backBuffer->Release();

        g_context->OMSetRenderTargets(1, &g_render_target_view, nullptr);

        D3D11_VIEWPORT vp;
        vp.Width = static_cast<float>(std::get<2>(ResizeContext.Arguments()));
        vp.Height = static_cast<float>(std::get<3>(ResizeContext.Arguments()));
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        g_context->RSSetViewports(1, &vp);
    }
}

void wnd_proc_detour(YYTK::FWWndProc& WndProcContext)
{
    auto [hWnd, msg, wParam, lParam] = WndProcContext.Arguments();

    if (g_init_state == CreatingContext2)
    {
        if (g_swapchain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&g_device)) == S_OK)
        {
            g_device->GetImmediateContext(&g_context);

            ID3D11Texture2D* backBuffer = nullptr;
            g_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
            g_device->CreateRenderTargetView(backBuffer, nullptr, &g_render_target_view);
            backBuffer->Release();
            g_context->OMSetRenderTargets(1, &g_render_target_view, nullptr);

            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            ImGui_ImplWin32_Init(hWnd);
            ImGui_ImplDX11_Init(g_device, g_context);

            g_init_state = Done;
            g_module_interface->Print(CM_LIGHTGREEN, "UI initialized!");
        }
        else
        {
            g_module_interface->Print(CM_LIGHTRED, "Failed to get device!");
            g_init_state = Failed;
        }
    }

    if (g_init_state == CreatingContext)
    {
        g_init_state = CreatingContext2;
    }

    if (g_init_state == Done)
    {
        if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) && wParam == VK_INSERT)
        {
            g_rendering = !g_rendering;
        }
        if (!g_rendering) return;

        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        {
            WndProcContext.Override(1);
        }

        auto io = ImGui::GetIO();
        if (io.WantCaptureKeyboard || io.WantTextInput)
        {
            if (msg == WM_KEYDOWN || msg == WM_KEYUP || msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP || msg == WM_CHAR)
            {
                WndProcContext.Override(1);
            }
        }

        if (io.WantCaptureMouse)
        {
            if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP || msg == WM_LBUTTONDBLCLK ||
                msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP || msg == WM_RBUTTONDBLCLK ||
                msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP || msg == WM_MBUTTONDBLCLK ||
                msg == WM_MOUSEWHEEL || msg == WM_MOUSEHWHEEL)
            {
                WndProcContext.Override(1);
            }
        }
    }
}

EXPORTED AurieStatus ModuleInitialize(
    IN AurieModule* module,
    IN const fs::path& module_path
)
{
    g_module = module;

    auto status = ObGetInterface(
        "YYTK_Main",
        reinterpret_cast<AurieInterfaceBase*&>(g_module_interface)
    );
    if (!AurieSuccess(status)) return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

    status = g_module_interface->CreateCallback(
        g_module,
        EVENT_FRAME,
        present_detour,
        0
    );
    if (!AurieSuccess(status)) return status;

    status = g_module_interface->CreateCallback(
        g_module,
        EVENT_RESIZE,
        resize_detour,
        0
    );
    if (!AurieSuccess(status)) return status;

    status = g_module_interface->CreateCallback(
        g_module,
        EVENT_WNDPROC,
        wnd_proc_detour,
        0
    );
    if (!AurieSuccess(status)) return status;

    status = ObCreateInterface(
        g_module,
        &g_wrapper,
        "ImGuiWrapper"
    );
    if (!AurieSuccess(status)) return status;

    return AURIE_SUCCESS;
}
