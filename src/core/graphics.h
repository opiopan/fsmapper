//
// graphics.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <atlbase.h>
#include <d2d1.h>
#include <wincodec.h>

namespace graphics{
    bool initialize_grahics();
    void terminate_graphics();

    class bitmap{
    protected:
    };

    class render_target{
    public:
        enum class rendering_method{cpu, gpu};
        static std::unique_ptr<render_target> create_render_target(int width, int height, rendering_method method);

        virtual operator ID2D1RenderTarget* () const = 0;
        ID2D1RenderTarget* operator ->() const {return static_cast<ID2D1RenderTarget*>(*this);}
    };
}