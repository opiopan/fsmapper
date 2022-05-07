//
// graphics.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "graphics.h"

#include <stdexcept>
#include <memory>
#include <unordered_map>
#include <sol/sol.hpp>
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

    void bitmap::draw(const render_target& target, const FloatRect& src_rect, const FloatRect& dest_rect){
        auto srect = src_rect + origin;
        auto bitmap = source->get_d2d_bitmap(target);
        target->DrawBitmap(bitmap, dest_rect, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, srect);
    }
}

//============================================================================================
// rendering_context: access point to render graphics from Lua script
//============================================================================================
namespace graphics{
    rendering_context::rendering_context(const bitmap& bitmap) : rect(bitmap.get_rect_in_source()), origin(bitmap.get_origin()){
        target_entity = bitmap.create_render_target();
        target = target_entity.get();
    }

    rendering_context::~rendering_context(){
        finish_rendering();
    }

    void rendering_context::finish_rendering(){
        target_entity = nullptr;
        target = nullptr;
    }
}

//============================================================================================
// create lua environment
//============================================================================================
void graphics::create_lua_env(MapperEngine& engine, sol::state& lua){
    auto table = lua.create_named_table("graphics");

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
                    auto a = lua_safevalue<float>(va[1]);
                    if (a){
                        return std::make_shared<graphics::color>(va[0], *a);
                    }else{
                        return std::make_shared<graphics::color>(va[0]);
                    }
                }
                throw MapperException("invalid arguments");
            });
        })
    );

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
        "opacity", sol::property(&bitmap::get_opacity, &bitmap::set_opacity)
    );

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
        "finish_rendering", &graphics::rendering_context::finish_rendering
    );
}
