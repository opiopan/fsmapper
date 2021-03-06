//
// graphics.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "graphics.h"

#include <stdexcept>
#include <memory>
#include <unordered_map>
#include <iomanip>
#include <limits>
#include <sol/sol.hpp>
#include <d2d1helper.h>
#include "tools.h"
#include "engine.h"
#include "fileops.h"
#include "encoding.hpp"

//============================================================================================
// initialize / deinitialize factory objects
//============================================================================================
static CComPtr<ID2D1Factory> d2d_factory;
static CComPtr<IWICImagingFactory> wic_factory;
static CComPtr<ID3D10Device1> d3d_device;

namespace graphics{
    bool initialize_grahics(){
        auto result = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &d2d_factory) == S_OK;
        result &= wic_factory.CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER) == S_OK;
        result &= D3D10CreateDevice1(
            0, // adapter
            D3D10_DRIVER_TYPE_HARDWARE,
            0, // reserved
            D3D10_CREATE_DEVICE_BGRA_SUPPORT,
            D3D10_FEATURE_LEVEL_10_0,
            D3D10_1_SDK_VERSION,
            &d3d_device) == S_OK;
        return result;
    }

    void terminate_graphics(){
        d3d_device = nullptr;
        wic_factory = nullptr;
        d2d_factory = nullptr;
    }
}

//============================================================================================
// Reder target implementation
//============================================================================================
template <typename TARGET, typename CONTENTS>
class render_target_implementation : public graphics::render_target{
protected:
    CComPtr<TARGET> target;
    CComPtr<CONTENTS> contents;
    std::unordered_map<uint32_t, CComPtr<ID2D1SolidColorBrush>> solid_color_brush_pool;

public:
    render_target_implementation() = delete;
    render_target_implementation(const render_target_implementation&) = delete;
    render_target_implementation(render_target_implementation&&) = delete;
    render_target_implementation(TARGET* target, CONTENTS* contents) : target(target), contents(contents){}
    virtual ~render_target_implementation() = default;

    operator ID2D1RenderTarget * () const override{
        return target;
    }

    ID2D1Brush* get_solid_color_brush(const graphics::color& color) override{
        auto key = color.rgba();
        if (solid_color_brush_pool.count(key)){
            return solid_color_brush_pool.at(key);
        }else{
            CComPtr<ID2D1SolidColorBrush> brush;
            target->CreateSolidColorBrush(color, &brush);
            solid_color_brush_pool[key] = brush;
            return brush;
        }
    }
};

namespace graphics{
    std::unique_ptr<render_target> render_target::create_render_target(int width, int height, rendering_method method){
        const D2D1_PIXEL_FORMAT format = D2D1::PixelFormat(
            DXGI_FORMAT_B8G8R8A8_UNORM,
            D2D1_ALPHA_MODE_PREMULTIPLIED);
        const D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            format,
            0.0f, // default dpi
            0.0f, // default dpi
            D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE);

        if (method == rendering_method::cpu){
            CComPtr<IWICBitmap> bitmap;
            wic_factory->CreateBitmap(width, height, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &bitmap);
            CComPtr<ID2D1RenderTarget> target;
            if (d2d_factory->CreateWicBitmapRenderTarget(bitmap, properties, &target) != S_OK){
                throw std::runtime_error("failed to create cpu redering environment");
            }
            return std::make_unique<render_target_implementation<ID2D1RenderTarget, IWICBitmap>>(target, bitmap);
        }else if (method == rendering_method::gpu){
            D3D10_TEXTURE2D_DESC description = {};
            description.ArraySize = 1;
            description.BindFlags = D3D10_BIND_RENDER_TARGET;
            description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
            description.Width = width;
            description.Height = height;
            description.MipLevels = 1;
            description.SampleDesc.Count = 1;
            description.MiscFlags = D3D10_RESOURCE_MISC_GDI_COMPATIBLE;

            CComPtr<ID3D10Texture2D> texture;
            d3d_device->CreateTexture2D(&description, 0, &texture);
            CComPtr<IDXGISurface> surface;
            texture.QueryInterface(&surface);
            CComPtr<ID2D1RenderTarget> target;
            if (d2d_factory->CreateDxgiSurfaceRenderTarget(surface, properties, &target) != S_OK){
                throw std::runtime_error("failed to create gpu rendering environment");
            }
            return std::make_unique<render_target_implementation<ID2D1RenderTarget, ID3D10Texture2D>>(target, texture);
        }else{
            throw std::runtime_error("invalid redering method is specified");
        }
    }
}

//============================================================================================
// representation of color
//============================================================================================
namespace graphics{
    color::color(sol::object value, float alpha){
        if (value.get_type() == sol::type::string){
            auto rgb = webcolor_to_colorref(value.as<std::string>());
            if (!rgb){
                throw MapperException("color as string must be specified with Web colors format");
            }
            r = GetRValue(*rgb) / 255.f;
            g = GetGValue(*rgb) / 255.f;
            b = GetBValue(*rgb) / 255.f;
            a = alpha;
        }else if (value.is<color&>()){
            auto rgba = value.as<std::shared_ptr<color>>();
            *this = *rgba;
        }else{
            throw MapperException("color must be specified as string or graphics.color() object");
        }
    }

    ID2D1Brush* color::brush_interface(render_target& target){
        if (this->target == target){
            return brush;
        }else{
            this->target = target;
            brush = target.get_solid_color_brush(*this);
            return brush;
        }
    }
}

//============================================================================================
// bitmap: WiC based bitmap representation
//============================================================================================
namespace graphics{
    class bitmap_source{
        IntRect rect;
        CComPtr<IWICBitmapSource> wic_bitmap{nullptr};
        ID2D1RenderTarget* last_render_target{nullptr};
        CComPtr<ID2D1Bitmap> d2d_bitmap{nullptr};

    public:
        bitmap_source() = delete;
        ~bitmap_source() = default;

        bitmap_source(int width, int height){
            CComPtr<IWICBitmap> bitmap;
            wic_factory->CreateBitmap(width, height, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, &bitmap);
            wic_bitmap = bitmap;
            rect = {0, 0, width, height};
        }

        bitmap_source(const char* sub_path){
            auto& lua = mapper_EngineInstance()->getLuaState();
            auto&& search_path = lua_safestring(lua["mapper"]["asset_path"]);
            auto&& path = fileops::find_file_in_paths(search_path.c_str(), sub_path);
            if (path){
                CComPtr<IWICBitmapDecoder> decoder;
                if (wic_factory->CreateDecoderFromFilename(path->c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder) != S_OK){
                    throw MapperException("bitmap format of specified file cannot be handled");
                }
                CComPtr<IWICBitmapFrameDecode> frame;
                if (decoder->GetFrame(0, &frame) != S_OK){
                    throw MapperException("specified bitmap file does not cantain varid image data");
                }
                CComPtr<IWICFormatConverter> converter;
                wic_factory->CreateFormatConverter(&converter);
                converter->Initialize(
                    frame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
                wic_bitmap = converter;
                UINT width, height;
                wic_bitmap->GetSize(&width, &height);
                rect = {0, 0, static_cast<int>(width), static_cast<int>(height)};
            }else{
                throw MapperException("specified file does not found");
            }
        }

        int width() const {return rect.width;}
        int height() const {return rect.height;}

        operator CComPtr<IWICBitmap> ()const {
            CComPtr<IWICBitmap> bitmap;
            auto rc = wic_bitmap->QueryInterface(IID_PPV_ARGS(&bitmap));
            if (rc != S_OK){
                throw MapperException("bitmap object is not modifiable");
            }
            return bitmap;
        }

        ID2D1Bitmap* get_d2d_bitmap(ID2D1RenderTarget* target){
            if (target != last_render_target){
                d2d_bitmap = nullptr;
                target->CreateBitmapFromWicBitmap(wic_bitmap, &d2d_bitmap);
                last_render_target = target;
            }
            return d2d_bitmap;
        }
    };

    void bitmap::lua_set_origin(sol::variadic_args va){
        lua_c_interface(*mapper_EngineInstance(), "graphics.bitmap:set_origin", [this, &va]{
            auto x = lua_safevalue<float>(va[0]);
            auto y = lua_safevalue<float>(va[1]);
            if (va[0].get_type() == sol::type::table){
                sol::table def = va[0];
                x = lua_safevalue<float>(def["x"]);
                y = lua_safevalue<float>(def["y"]);
            }
            if (x && y){
                this->set_origin({*x, *y});
            }else{
                throw MapperException("invalid argument");
            }
        });
    }

    std::shared_ptr<bitmap> bitmap::create_partial_bitmap(sol::variadic_args va) const{
        return lua_c_interface(*mapper_EngineInstance(), "bitmap::create_partial_bitmap", [this, &va]{
            auto x = lua_safevalue<float>(va[0]);
            auto y = lua_safevalue<float>(va[1]);
            auto width = lua_safevalue<float>(va[2]);
            auto height = lua_safevalue<float>(va[3]);
            if (x && y && width && height){
                return std::make_shared<bitmap>(source, FloatRect(*x, *y, *width, *height));
            }else{
                throw MapperException("invalid argument");
            }
        });
    }

    std::unique_ptr<render_target> bitmap::create_render_target() const{
        const D2D1_PIXEL_FORMAT format = D2D1::PixelFormat(
            DXGI_FORMAT_B8G8R8A8_UNORM,
            D2D1_ALPHA_MODE_PREMULTIPLIED);
        const D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            format,
            0.0f, 0.0f // default dpi
        );
        auto&& target_bitmap = source->operator CComPtr<IWICBitmap>();
        CComPtr<ID2D1RenderTarget>target;
        if (d2d_factory->CreateWicBitmapRenderTarget(target_bitmap, properties, &target) != S_OK){
            throw std::runtime_error("failed to create cpu redering environment");
        }
        return std::make_unique<render_target_implementation<ID2D1RenderTarget, IWICBitmap>>(target, target_bitmap);
    }

    void bitmap::draw(const render_target& target, const FloatRect& dest_rect){
        auto shift = origin;
        shift.x *= dest_rect.width / rect.width;
        shift.y *= dest_rect.height / rect.height;
        auto drect = dest_rect - shift;
        auto bitmap = source->get_d2d_bitmap(target);
        target->DrawBitmap(bitmap, drect, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, rect);
    }

    void bitmap::draw(const render_target& target, const FloatPoint& offset, float scale_x, float scale_y, float angle){
        auto&& matrix = D2D1::Matrix3x2F::Translation(-origin.x, -origin.y)
                        * D2D1::Matrix3x2F::Scale(scale_x, scale_y)
                        * D2D1::Matrix3x2F::Rotation(angle)
                        * D2D1::Matrix3x2F::Translation(offset.x, offset.y);
        target->SetTransform(matrix);
        FloatRect drect{0.f, 0.f, rect.width, rect.height};
        auto bitmap = source->get_d2d_bitmap(target);
        target->DrawBitmap(bitmap, drect, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, rect);
        target->SetTransform(D2D1::Matrix3x2F::Identity());
    }
}

//============================================================================================
// font:
//============================================================================================
namespace graphics{
    std::shared_ptr<font> as_font(sol::object& obj){
        std::shared_ptr<font> font;
        if (obj.is<bitmap_font>()){
            font = obj.as<std::shared_ptr<bitmap_font>>();
        }
        return font;
    }

    void bitmap_font::add_glyph(int code_point, const std::shared_ptr<bitmap>& glyph){
        if (code_point >= code_point_min && code_point <= code_point_max ){
            glyphs[code_point - code_point_min] = glyph;
        }
    }

    void bitmap_font::add_glyph_lua(sol::variadic_args args){
        lua_c_interface(*mapper_EngineInstance(), "graphics.bitmap_font:add_glyph", [this, &args]{
            std::string code_point;
            sol::object may_be_bitmap;
            sol::object arg0 = args[0];
            if (arg0.get_type() == sol::type::string){
                code_point = std::move(lua_safestring(arg0));
                may_be_bitmap = args[1];
            }else if (arg0.get_type() == sol::type::table){
                sol::table def = arg0;
                code_point = std::move(lua_safestring(def["code_point"]));
                may_be_bitmap = def["bitmap"];
            }else{
                throw MapperException("invalid parameters");
            }

            if (code_point.size() != 1 || code_point.c_str()[0] < code_point_min || code_point.c_str()[1] > code_point_max){
                throw MapperException("string that represents code point is not correct, "
                                      "it must be one charactor and must be within the range of ASCII code");
            }
            if (!may_be_bitmap.is<bitmap&>()){
                throw MapperException("bitmap parameter is not specified or specified value is not bitmap object");
            }
            auto bitmap = may_be_bitmap.as<std::shared_ptr<graphics::bitmap>>();
            add_glyph(code_point.c_str()[0], bitmap);
        });
    }

    FloatRect bitmap_font::draw_string(const render_target& target, const char* string, const FloatPoint& pos, float scale){
        FloatRect rect{pos.x, pos.y, 0.f, 0.f};
        for (const char* code = string; *code; code++){
            if (*code >= code_point_min && *code <= code_point_max){
                const auto& glyph = glyphs[*code - code_point_min];
                if (glyph){
                    FloatRect target_rect{
                        rect.x + rect.width,
                        rect.y, 
                        glyph->get_width() * scale,
                        glyph->get_height() * scale
                    };
                    glyph->draw(target, target_rect);
                    auto out_rect = target_rect - glyph->get_origin();
                    rect += out_rect;
                }
            }
        }
        return rect;
    }
}

//============================================================================================
// rendering_context: access point to render graphics from Lua script
//============================================================================================
namespace graphics{
    rendering_context::rendering_context(const bitmap& bitmap) : rect(bitmap.get_rect_in_source()), origin(bitmap.get_origin()){
        target_entity = bitmap.create_render_target();
        target = target_entity.get();
        (*target)->BeginDraw();
    }

    rendering_context::~rendering_context(){
        finish_rendering();
    }

    void rendering_context::finish_rendering(){
        if (target_entity){
            (*target)->EndDraw();
            target_entity = nullptr;
        }
        target = nullptr;
        brush = nullptr;
        font = nullptr;
    }

    void rendering_context::translate_to_context_coordinate(FloatPoint& point){
        point.x = point.x * this->scale + this->rect.x;
        point.y = point.y * this->scale + this->rect.y;
    }

    void rendering_context::translate_to_context_coordinate(FloatRect& rect){
        rect.x = rect.x * this->scale + this->rect.x;
        rect.y = rect.y * this->scale + this->rect.y;
        rect.width *= this->scale;
        rect.height *= this->scale;
    }

    void rendering_context::set_brush(sol::object brush){
        lua_c_interface(*mapper_EngineInstance(), "graphics.rendering_context:set_brush", [this, &brush]{
            if (brush.get_type() == sol::type::lua_nil){
                this->brush = nullptr;
            }else if (brush.is<graphics::brush&>()){
                this->brush = brush.as<std::shared_ptr<graphics::brush>>();
            }else if (brush.is<graphics::color&>()){
                this->brush = brush.as<std::shared_ptr<graphics::color>>();
            }else{
                throw MapperException("specified value is not brush object");
            }
        });
    }

    void rendering_context::set_font(sol::object font){
        lua_c_interface(*mapper_EngineInstance(), "graphics.rendering_context:set_font", [this, &font]{
            if (font.get_type() == sol::type::lua_nil){
                this->font = nullptr;
            }else{
                auto newfont = as_font(font);
                if (newfont){
                    this->font = newfont;
                }else{
                    throw MapperException("specified value is not font object");
                }
            }
        });
    }


    void rendering_context::draw_bitmap(sol::variadic_args args){
        lua_c_interface(*mapper_EngineInstance(), "graphics.rendering_context:draw_bitmap", [this, &args]{
            std::shared_ptr<bitmap> bitmap;
            std::optional<float> x;
            std::optional<float> y;
            std::optional<float> width;
            std::optional<float> height;
            std::optional<float> angle;
            FloatRect drect{0.f, 0.f, 0.f, 0.f};
            sol::object arg0 = args[0];
            if (arg0.is<graphics::bitmap&>()){
                bitmap = arg0.as<std::shared_ptr<graphics::bitmap>>();
                x = lua_safevalue<float>(args[1]);
                y = lua_safevalue<float>(args[2]);
                width = lua_safevalue<float>(args[3]);
                height = lua_safevalue<float>(args[4]);
                angle = lua_safevalue<float>(args[5]);
            }else if (arg0.get_type() == sol::type::table){
                auto def = arg0.as<sol::table>();
                sol::object bm = def["bitmap"];
                if (!bm.is<graphics::bitmap&>()){
                    throw MapperException("no bitmap is specified");
                }
                bitmap = bm.as<std::shared_ptr<graphics::bitmap>>();
                x = lua_safevalue<float>(def["x"]);
                y = lua_safevalue<float>(def["y"]);
                width = lua_safevalue<float>(def["width"]);
                height = lua_safevalue<float>(def["height"]);
                angle = lua_safevalue<float>(def["angle"]);
            }else{
                throw MapperException("invalid parameters");
            }

            drect.width = bitmap->get_width() * scale;
            drect.height = bitmap->get_height() * scale;
            if (x && y){
                drect.x = *x * scale;
                drect.y = *y * scale;
            }
            if (width && height){
                drect.width = *width * scale;
                drect.height = *height * scale;
            }
            drect.x += this->rect.x;
            drect.y += this->rect.y;
            bitmap->draw(*target, {drect.x, drect.y},
                            drect.width / bitmap->get_width(), drect.height / bitmap->get_height(),
                            angle ? *angle : 0.f);
            // bitmap->draw(*target, drect);
        });
    }

    void rendering_context::fill_rectangle(sol::object x, sol::object y, sol::object width, sol::object height){
        lua_c_interface(*mapper_EngineInstance(), "graphics.rendering_context:fill_rectangle", [this, &x, &y, &width, &height]{
            auto vx = lua_safevalue<float>(x);
            auto vy = lua_safevalue<float>(y);
            auto vwidth = lua_safevalue<float>(width);
            auto vheight = lua_safevalue<float>(height);
            if (!vx || !vy || !vwidth || !vheight){
                throw MapperException("invarid argument");
            }
            if (this->brush){
                FloatRect rect{*vx, *vy, *vwidth, *vheight};
                rect.x = rect.x * this->scale + this->rect.x;
                rect.y = rect.y * this->scale + this->rect.y;
                rect.width *= this->scale;
                rect.height *= this->scale;
                (*target)->FillRectangle(rect, brush->brush_interface(*target));
            }
        });
    }

    void rendering_context::draw_string(sol::variadic_args args){
        lua_c_interface(*mapper_EngineInstance(), "graphics.rendering_context:draw_string", [this, &args]{
            std::string string;
            std::optional<float> x;
            std::optional<float> y;
            sol::object arg0 = args[0];
            if (arg0.get_type() == sol::type::string){
                string = std::move(lua_safestring(arg0));
                x = lua_safevalue<float>(args[1]);
                y = lua_safevalue<float>(args[2]);
            }else if (arg0.get_type() == sol::type::table){
                sol::table def = arg0;
                string = std::move(lua_safestring(def["string"]));
                x = lua_safevalue<float>(def["x"]);
                y = lua_safevalue<float>(def["y"]);
            }else{
                throw MapperException("invalid parameters");
            }
            FloatPoint point{0.f, 0.f};
            if (x && y){
                point.x = *x;
                point.y = *y;
            }
            draw_string_native(string.c_str(), point);
        });
    }

    void rendering_context::draw_number(sol::variadic_args args){
        lua_c_interface(*mapper_EngineInstance(), "graphics.rendering_context:draw_number", [this, &args]{
            std::optional<double> value;
            std::optional<float> x;
            std::optional<float> y;
            std::optional<int> fraction_precision;
            std::optional<int> precision;
            std::optional<bool> leading_zero;

            sol::object arg0 = args[0];
            if (arg0.get_type() == sol::type::number){
                value = lua_safevalue<double>(arg0);
                x = lua_safevalue<float>(args[1]);
                y = lua_safevalue<float>(args[2]);
            }else if (arg0.get_type() == sol::type::table){
                sol::table def = arg0;
                value = lua_safevalue<double>(def["value"]);
                x = lua_safevalue<float>(def["x"]);
                y = lua_safevalue<float>(def["y"]);
                fraction_precision = lua_safevalue<int>(def["fraction_precision"]);
                precision = lua_safevalue<int>(def["precision"]);
                leading_zero = lua_safevalue<bool>(def["leading_zero"]);
            }else{
                throw MapperException("invalid parameters");
            }
            if (!value){
                throw MapperException("no numeric value is specified");
            }
            if (precision && fraction_precision && precision <= fraction_precision){
                throw MapperException("precition parameter must be larger than fraction_precision parameter");
            }
            if (precision && *precision <= 0){
                throw MapperException("precition parameter must be grater than 0");
            }
            if (fraction_precision && *fraction_precision < 0){
                throw MapperException("precition parameter must be grater than 0 or 0");
            }

            std::ostringstream os;
            if (precision && fraction_precision){
                if (*fraction_precision > 0){
                    os << std::setprecision(*fraction_precision) << std::fixed << std::setw(*precision + 1);
                }else{
                    os << std::setprecision(0) << std::fixed << std::setw(*precision);
                }
            }else if (precision){
                os << std::setprecision(*precision);
            }else if (fraction_precision){
                os << std::setprecision(*fraction_precision) << std::fixed;
            }
            if (leading_zero && *leading_zero){
                os << std::setfill('0');
            }
            os << *value;

            FloatPoint point{0.f, 0.f};
            if (x && y){
                point.x = *x;
                point.y = *y;
            }
            draw_string_native(os.str().c_str(), point);
        });
    }


    void rendering_context::draw_string_native(const char* string, const FloatPoint& point){
        auto opoint = point;
        translate_to_context_coordinate(opoint);
        font->draw_string(*target, string, opoint, this->scale);
    }
}

//============================================================================================
// create lua environment
//============================================================================================
void graphics::create_lua_env(MapperEngine& engine, sol::state& lua){
    auto table = lua.create_named_table("graphics");

    //
    // color
    //
    table.new_usertype<graphics::color>(
        "color",
        sol::call_constructor, sol::factories([&engine](sol::variadic_args va){
            return lua_c_interface(engine, "graphics.color", [&va](){
                auto type = va[0].get_type();
                if (type == sol::type::number){
                    auto r = lua_safevalue<float>(va[0]);
                    auto g = lua_safevalue<float>(va[1]);
                    auto b = lua_safevalue<float>(va[2]);
                    auto a = lua_safevalue<float>(va[3]);
                    if (r && g && b){
                        if (a){
                            return std::make_shared<graphics::color>(*r / 255.f, *g / 255.f, *b / 255.f, *a);
                        }else{
                            return std::make_shared<graphics::color>(*r / 255.f, *g / 255.f, *b / 255.f);
                        }
                    }
                }else if (type == sol::type::string){
                    sol::object color_name = va[0];
                    auto a = lua_safevalue<float>(va[1]);
                    if (a){
                        return std::make_shared<graphics::color>(color_name, *a);
                    }else{
                        return std::make_shared<graphics::color>(color_name);
                    }
                }
                throw MapperException("invalid arguments");
            });
        })
    );

    //
    // bitmap
    //
    table.new_usertype<graphics::bitmap>(
        "bitmap",
        sol::call_constructor, sol::factories([&engine](sol::variadic_args va){
            return lua_c_interface(engine, "graphics.bitmap", [&va](){
                auto type = va[0].get_type();
                if (type == sol::type::string){
                    auto&& sub_path = lua_safestring(va[0]);
                    auto source = std::make_shared<bitmap_source>(sub_path.c_str());
                    FloatRect rect{0.f, 0.f, static_cast<float>(source->width()), static_cast<float>(source->height())};
                    return std::make_shared<bitmap>(source, rect);
                }else if (type == sol::type::number){
                    auto width = lua_safevalue<int>(va[0]);
                    auto height = lua_safevalue<int>(va[1]);
                    if (width && height){
                        auto source = std::make_shared<bitmap_source>(*width, *height);
                        FloatRect rect{0.f, 0.f, static_cast<float>(*width), static_cast<float>(*height)};
                        return std::make_shared<bitmap>(source, rect);
                    }
                }
                throw MapperException("invalid arguments");
            });
        }),
        "opacity", sol::property(&bitmap::get_opacity, &bitmap::set_opacity),
        "width", sol::property(&bitmap::get_width),
        "height", sol::property(&bitmap::get_height),
        "set_origin", &bitmap::lua_set_origin,
        "create_partial_bitmap", &bitmap::create_partial_bitmap
    );

    //
    // bitmap_font
    //
    table.new_usertype<graphics::bitmap_font>(
        "bitmap_font",
        sol::call_constructor, sol::factories([]{return std::make_shared<bitmap_font>();}),
        "add_glyph", &graphics::bitmap_font::add_glyph_lua
    );

    //
    // rendering_context
    //
    table.new_usertype<graphics::rendering_context>(
        "rendering_context",
        sol::call_constructor, sol::factories([&engine](sol::object object){
            return lua_c_interface(engine, "graphics.rendering_context", [&object](){
                if (object.is<graphics::bitmap&>()){
                    auto bitmap = object.as<std::shared_ptr<graphics::bitmap>>();
                    return std::make_shared<graphics::rendering_context>(*bitmap);
                }else{
                    throw MapperException("bitmap to render is not specified");
                }
            });
        }),
        "finish_rendering", &graphics::rendering_context::finish_rendering,
        "set_brush", &graphics::rendering_context::set_brush,
        "set_font", &graphics::rendering_context::set_font,
        "draw_bitmap", &graphics::rendering_context::draw_bitmap,
        "draw_string", &graphics::rendering_context::draw_string,
        "draw_number", &graphics::rendering_context::draw_number,
        "fill_rectangle", &graphics::rendering_context::fill_rectangle
    );
}
