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
#include "tools.h"

class MapperEngine;

namespace graphics{
    bool initialize_grahics();
    void terminate_graphics();
    void create_lua_env(MapperEngine& engine, sol::state& lua);

    class color;

    //============================================================================================
    // render_target: abstruction of Direct2D  render target
    //============================================================================================
    class render_target{
    public:
        enum class rendering_method{cpu, gpu};
        static std::unique_ptr<render_target> create_render_target(int width, int height, rendering_method method);

        virtual operator ID2D1RenderTarget* () const = 0;
        ID2D1RenderTarget* operator ->() const {return static_cast<ID2D1RenderTarget*>(*this);}

        virtual ID2D1Brush* get_solid_color_brush(const color& color) = 0;
    };

    //============================================================================================
    // brush: abstruct class to express brush
    //============================================================================================
    class brush{
    public:
        virtual ID2D1Brush* brush_interface(render_target& target) = 0;
    };

    //============================================================================================
    // color: representation of color
    //============================================================================================
    class color : public brush{
    protected:
        float r = 0.f;
        float g = 0.f;
        float b = 0.f;
        float a = 1.0f;
        
        ID2D1RenderTarget* target = nullptr;
        ID2D1Brush* brush = nullptr;

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
        virtual ~color() = default;

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
        void set_alpha(float alpha){a = alpha;}
        
        uint32_t rgba() const{
            return static_cast<uint32_t>(r * 255) << 24 |
                   static_cast<uint32_t>(g * 255) << 16 |
                   static_cast<uint32_t>(b * 255) << 8 |
                   static_cast<uint32_t>(a * 255);
        };

        operator COLORREF () const{return RGB(r * 255, g * 255, b * 255);}
        operator D3DCOLORVALUE () const{return D3DCOLORVALUE{r, g, b, a};}
        bool operator == (const color& rvalue) const{return this->rgba() == rvalue.rgba();}
        bool operator != (const color& rvalue) const{return this->rgba() != rvalue.rgba();}

        struct hash{
            std::size_t operator ()(const color& key) const{
                return std::hash<uint32_t>()(key.rgba());
            }
        };

        ID2D1Brush* brush_interface(render_target& target) override;
    };

    //============================================================================================
    // bitmap: WIC based bitmap
    //============================================================================================
    class bitmap_source;

    class bitmap{
    protected:
        std::shared_ptr<bitmap_source> source;
        FloatRect rect;
        FloatPoint origin;
        float opacity{1.f};

    public:
        bitmap(const std::shared_ptr<bitmap_source>& source, const FloatRect& rect, const FloatPoint& origin ={0.f, 0.f}) :
            source(source), rect(rect), origin(origin){}
        
        const FloatRect& get_rect_in_source() const {return rect;}
        float get_width() const {return rect.width;}
        float get_height() const {return rect.height;}
        const FloatPoint& get_origin() const {return origin;}
        bool set_origin(const FloatPoint& new_origin){
            if (rect.pointIsInRectangle(new_origin.x, new_origin.y)){
                origin = new_origin;
                return true;
            }else{
                return false;
            }
        }
        float get_opacity() const {return opacity;}
        void set_opacity(float opacity){this->opacity = opacity;}

        std::unique_ptr<render_target> create_render_target() const;

        void draw(const render_target& target, const FloatRect& src_rect, const FloatRect& dest_rect);
        void draw(const render_target& target, const FloatPoint& offset, float scale, float rotation = 0.f);
    };

    //============================================================================================
    // rendering_context: access point to render graphics from Lua script
    //============================================================================================
    class rendering_context {
    protected:
        render_target* target;
        std::unique_ptr<render_target> target_entity;
        FloatRect rect;
        FloatPoint origin{ 0.f, 0.f };
        float scale = 1.f;

    public:
        rendering_context() = delete;
        rendering_context(render_target& target, const FloatRect& rect, float scale) : target(&target), rect(rect), scale(scale) {}
        rendering_context(const bitmap& bitmap);
        ~rendering_context();

        void finish_rendering();
    };
}