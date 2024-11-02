//
// composition.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <atlbase.h>
#include <dxgi1_3.h>
#include <d2d1_2.h>
#include <d3d11_2.h>
#include <dcomp.h>

namespace composition{
    CComPtr<ID3D11Device> create_d3d_device();
    CComPtr<IDXGISwapChain1> create_swapchain(IDXGIDevice* device, UINT width, UINT height);
    CComPtr<IDXGISwapChain1> create_swapchain(UINT width, UINT height);

    class viewport_target{
    public:
        virtual ID2D1RenderTarget* get_render_target() = 0;
        ID2D1RenderTarget* operator ()(){return get_render_target();}

        virtual IDCompositionDevice* get_device() = 0;
        virtual CComPtr<IDCompositionVisual> create_visual() = 0;

        virtual void reset_visual_tree() = 0;
        virtual void add_visual(IDCompositionVisual* visual) = 0;
        virtual void commit_visual_tree(bool show_main_visual) = 0;

        virtual void present() = 0;
    };

    std::unique_ptr<viewport_target> create_viewport_target(HWND hwnd, UINT width, UINT height);
}
