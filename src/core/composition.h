//
// composition.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <atlbase.h>
#include <dxgi1_3.h>
#include <d2d1_2.h>

namespace composition{
    CComPtr<IDXGISwapChain1> create_swapchain(UINT width, UINT height);

    class viewport_target{
    public:
        virtual ID2D1RenderTarget* get_render_target() = 0;
        ID2D1RenderTarget* operator ()(){return get_render_target();}

        virtual void present() = 0;
    };

    std::unique_ptr<viewport_target> create_viewport_target(HWND hwnd, UINT width, UINT height);
}
