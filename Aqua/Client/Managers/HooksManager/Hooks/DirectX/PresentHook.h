#pragma once
#include <ImGuiUtils.h>
#include <MiSans.h>
#include <Windows.h>
#include <imfx.h>
#include <impl/imgui_impl_dx11.h>
#include <impl/imgui_impl_win32.h>

#include "../../../../../Utils/RenderUtil.h"
#include "../../../../Client.h"
#include "../FuncHook.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam,
                                                             LPARAM lParam);

class PresentHook : public FuncHook {
   private:
    using present_t = HRESULT(__thiscall*)(IDXGISwapChain3*, UINT, UINT);
    static inline present_t oPresent;

    static inline WNDPROC originalWndProc = nullptr;
    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if(ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;
        return CallWindowProc(originalWndProc, hWnd, msg, wParam, lParam);
    }

    static HRESULT presentCallback(IDXGISwapChain3* swapChain, UINT syncInterval, UINT flags) {
        if(!Client::isInitialized())
            return oPresent(swapChain, syncInterval, flags);

        static HWND window = (HWND)FindWindowA(nullptr, (LPCSTR) "Minecraft");

        RECT rect;
        GetWindowRect(window, &rect);


        ID3D12Device* d3d12Device = nullptr;
        ID3D11Device* d3d11Device = nullptr;


        if(SUCCEEDED(swapChain->GetDevice(IID_PPV_ARGS(&d3d12Device)))) {
            static_cast<ID3D12Device5*>(d3d12Device)->RemoveDevice();
            return oPresent(swapChain, syncInterval, flags);
        } else if(SUCCEEDED(swapChain->GetDevice(IID_PPV_ARGS(&d3d11Device)))) {

            static ID3D11DeviceContext* ppContext = nullptr;
            if(!ppContext) {
                d3d11Device->GetImmediateContext(&ppContext);
            }

            static bool initContext = false;
            if(!initContext) {
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO();
                io.IniFilename = nullptr;  

                ImGui_ImplWin32_Init(window);
                ImGui_ImplDX11_Init(d3d11Device, ppContext);

                if(window && !originalWndProc) {
                    originalWndProc =
                        (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
                }

                initContext = true;
            }


            ID3D11Texture2D* pBackBuffer = nullptr;
            swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

            ID3D11RenderTargetView* mainRenderTargetView = nullptr;
            if(pBackBuffer) {
                d3d11Device->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
                pBackBuffer->Release();  
            }


            IDXGISurface* dxgiBackBuffer = nullptr;
            if(SUCCEEDED(swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer)))) {
                dxgiBackBuffer->Release();  // ★ 修复点：用完必须释放
            }

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

  
            ImGuiUtils::setDrawList(ImGui::GetBackgroundDrawList());

            static CustomFont* customFontMod = ModuleManager::getModule<CustomFont>();
            if(customFontMod) {
                ImGuiUtils::setFontSize(customFontMod->fontSize);
            }


            static ClickGui* clickGuiMod = ModuleManager::getModule<ClickGui>();
            if(clickGuiMod) {
                clickGuiMod->render(ImGui::GetBackgroundDrawList());
            }
            ModuleManager::onImGuiRender(ImGui::GetBackgroundDrawList());
            Notifications::Render(ImGui::GetBackgroundDrawList());

            ImGui::EndFrame();
            ImGui::Render();

 
            if(mainRenderTargetView) {
                ppContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
                mainRenderTargetView->Release();
            }

            d3d11Device->Release();
        }

        return oPresent(swapChain, syncInterval, flags);
    }

   public:
    PresentHook() {
        OriginFunc = (void*)&oPresent;
        func = (void*)&presentCallback;
    }
};