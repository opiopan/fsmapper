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
        template <typename Func>
        auto update_viwport_image(Func renderer){
            auto target = begin_draw();
            renderer(target);
            end_draw();
        }

    protected:
        virtual ID2D1RenderTarget* begin_draw() = 0;
        virtual void end_draw() = 0;
    };

    std::unique_ptr<viewport_target> create_viewport_target(HWND hwnd, UINT width, UINT height);
}
