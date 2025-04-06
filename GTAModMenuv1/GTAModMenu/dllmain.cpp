// dx11hook.cpp
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <vector>
#include <thread>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "MinHook.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "libMinHook.x64.lib")

using PresentFn = HRESULT(__stdcall*)(IDXGISwapChain*, UINT, UINT);
PresentFn oPresent = nullptr;

ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
HWND g_hWnd = nullptr;
WNDPROC oWndProc = nullptr;
bool showMenu = true;
bool initialized = false;

// Cheats toggles
bool godMode = false;
bool infiniteAmmo = false;
bool sprintSpeed = false;
bool wantedZero = false;

// World pointer (padrão Steam)
uintptr_t WorldPTR_STEAM = 0x23D6330; // Certifique-se de ajustar para a versão correta do GTA V

uintptr_t ReadPointerChain(uintptr_t base, const std::vector<unsigned int>& offsets) {
    uintptr_t addr = *(uintptr_t*)base;
    for (size_t i = 0; i < offsets.size() - 1; ++i)
        addr = *(uintptr_t*)(addr + offsets[i]);
    return addr + offsets.back();
}

void WriteBool(uintptr_t addr, bool val) { *(bool*)addr = val; }
void WriteFloat(uintptr_t addr, float val) { *(float*)addr = val; }
void WriteInt(uintptr_t addr, int val) { *(int*)addr = val; }

void ApplyCheats() {
    if (godMode) {
        auto addr = ReadPointerChain(WorldPTR_STEAM, { 0x08, 0x189 });
        if (addr) WriteBool(addr, true);
    }
    if (wantedZero) {
        auto addr = ReadPointerChain(WorldPTR_STEAM, { 0x08, 0x10B8, 0x7F8 });
        if (addr) WriteInt(addr, 0);
    }
    if (sprintSpeed) {
        auto addr = ReadPointerChain(WorldPTR_STEAM, { 0x08, 0x10B8, 0x14C });
        if (addr) WriteFloat(addr, 5.0f);
    }
    if (infiniteAmmo) {
        auto addr = ReadPointerChain(WorldPTR_STEAM, { 0x08, 0x10C8, 0x20, 0x60, 0x8, 0x0, 0x18 });
        if (addr) WriteInt(addr, 999);
    }
}

LRESULT CALLBACK HookWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    ImGui_ImplWin32_Init(g_hWnd);
    oWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)HookWndProc);
    return TRUE;
    return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!initialized) {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_pd3dDevice))) {
            g_pd3dDevice->GetImmediateContext(&g_pd3dDeviceContext);

            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            g_hWnd = sd.OutputWindow;
            oWndProc = (WNDPROC)SetWindowLongPtr(g_hWnd, GWLP_WNDPROC, (LONG_PTR)HookWndProc);

            ID3D11Texture2D* pBackBuffer = nullptr;
            pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
            pBackBuffer->Release();

            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(g_hWnd);
            ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

            initialized = true;
        }
    }

    if (GetAsyncKeyState(VK_F11) & 1)
        showMenu = !showMenu;

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (showMenu) {
        ImGui::Begin("GTA V Mod Menu");

        ImGui::Text("Ative os cheats desejados:");
        ImGui::Checkbox("God Mode", &godMode);
        ImGui::Checkbox("Infinite Ammo", &infiniteAmmo);
        ImGui::Checkbox("Sprint Speed x5", &sprintSpeed);
        ImGui::Checkbox("Wanted Level 0", &wantedZero);

        if (ImGui::Button("Aplicar Cheats")) {
            ApplyCheats();
        }

        ImGui::End();
    }

    ImGui::Render();
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return oPresent(pSwapChain, SyncInterval, Flags);
}

void* GetSwapChainVTable() {
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L,
        GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
        "TempWindow", NULL
    };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(wc.lpszClassName, "", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, wc.hInstance, NULL);

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    ID3D11Device* pDevice = nullptr;
    ID3D11DeviceContext* pContext = nullptr;
    IDXGISwapChain* pSwapChain = nullptr;

    if (SUCCEEDED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0,
        D3D11_SDK_VERSION, &sd, &pSwapChain, &pDevice, NULL, &pContext))) {

        void** pVTable = *reinterpret_cast<void***>(pSwapChain);
        oPresent = (PresentFn)pVTable[8];
        pSwapChain->Release();
        pDevice->Release();
        pContext->Release();
        DestroyWindow(hwnd);
        UnregisterClass(wc.lpszClassName, wc.hInstance);

        return pVTable;
    }

    return nullptr;
}

DWORD WINAPI MainThread(LPVOID) {
    void** pSwapChainVTable = (void**)GetSwapChainVTable();
    if (!pSwapChainVTable) return 0;

    if (MH_Initialize() != MH_OK) return 0;
    if (MH_CreateHook(pSwapChainVTable[8], &hkPresent, reinterpret_cast<void**>(&oPresent)) != MH_OK) return 0;
    if (MH_EnableHook(pSwapChainVTable[8]) != MH_OK) return 0;
    Sleep(15000); // espera 15 segundos após injetar
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
