//
// graphics.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <algorithm>
#include <atlbase.h>
#include <d2d1.h>
#include <wincodec.h>
#include <sol/sol.hpp>

class MapperEngine;

namespace graphics{
    bool initialize_grahics();
    void terminate_graphics();
    void create_lua_env(MapperEngine& engine, sol::state& lua);

    //============================================================================================
    // color: representation of color
    //============================================================================================
    class color{
    protected:
        float r = 0.f;
        float g = 0.f;
        float b = 0.f;
        float a = 1.0f;

        template <typename T>
        static constexpr T clip(T value){
            if (value < static_cast<T>(0)){
                return static_cast<T>(0);
            }else if (value > static_cast<T>(1)){
                return static_cast<T>(1);
            }
            return value;
        }
    public:
        color() = default;
        color(float red, float green, float blue, float alpha = 1.0f): 
            r(clip(red)), g(clip(green)), b(clip(blue)), a(clip(alpha)){}
        color(sol::object value, float alpha = 1.0f);
        color(const color& src){*this = src;}
        ~color(){
        }

        color& operator = (const color& src){
            r = src.r;
            g = src.g;
            b = src.b;
            a = src.a;
            return *this;
        }

        float red() const{return r;}
        float green() const{return g;}
        float blue() const{return b;}
        float alpha() const{return a;}

        operator COLORREF (){return RGB(r * 255, g * 255, b * 255);}
        operator D3DCOLORVALUE (){return D3DCOLORVALUE{r, g, b, a};}
    };

    //============================================================================================
    // bitmap: WIC based bitmap
    //============================================================================================
    class bitmap{
    protected:
    };

    //============================================================================================
    // render_target: abstruction of Windows  render target
    //============================================================================================
    class render_target{
    public:
        enum class rendering_method{cpu, gpu};
        static std::unique_ptr<render_target> create_render_target(int width, int height, rendering_method method);

        virtual operator ID2D1RenderTarget* () const = 0;
        ID2D1RenderTarget* operator ->() const {return static_cast<ID2D1RenderTarget*>(*this);}
    };
}