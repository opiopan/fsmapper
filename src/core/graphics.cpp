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
#include "composition.h"

//============================================================================================
// initialize / deinitialize factory objects
//============================================================================================
static CComPtr<ID2D1Factory> d2d_factory;
static CComPtr<IWICImagingFactory> wic_factory;
static CComPtr<IDWriteFactory> dwrite_factory;
static CComPtr<ID3D10Device1> d3d_device;

namespace graphics{
    bool initialize_grahics(){
        auto result = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &d2d_factory) == S_OK;
        result &= wic_factory.CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER) == S_OK;
        result &= ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&dwrite_factory)) == S_OK;
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
        dwrite_factory = nullptr;
        d2d_factory = nullptr;
    }
}

//============================================================================================
// Reder target implementation
//============================================================================================
class render_target_base : public graphics::render_target{
protected:
    std::unordered_map<uint32_t, CComPtr<ID2D1SolidColorBrush>> solid_color_brush_pool;

public:
    ID2D1Brush* get_solid_color_brush(const graphics::color& color) override{
        auto key = color.rgba();
        if (solid_color_brush_pool.count(key)){
            return solid_color_brush_pool.at(key);
        }else{
            CComPtr<ID2D1SolidColorBrush> brush;
            (*this)->CreateSolidColorBrush(color, &brush);
            solid_color_brush_pool[key] = brush;
            return brush;
        }
    }
};

template <typename TARGET, typename CONTENTS>
class render_target_implementation : public render_target_base{
protected:
    CComPtr<TARGET> target;
    CComPtr<CONTENTS> contents;

public:
    render_target_implementation() = delete;
    render_target_implementation(const render_target_implementation&) = delete;
    render_target_implementation(render_target_implementation&&) = delete;
    render_target_implementation(TARGET* target, CONTENTS* contents) : target(target), contents(contents){}
    virtual ~render_target_implementation() = default;

    operator ID2D1RenderTarget * () const override{
        return target;
    }
};

class light_render_target : public render_target_base{
protected:
    CComPtr<ID2D1RenderTarget> target;

public:
    light_render_target(ID2D1RenderTarget* target_ptr){
        target = target_ptr;
    }

    operator ID2D1RenderTarget * () const override{
        return target;
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

    std::unique_ptr<render_target> render_target::create_render_target(ID2D1RenderTarget* target_ptr){
        return std::make_unique<light_render_target>(target_ptr);
    }
}

//============================================================================================
// utility function to check if a lua object can be translated to brush class pointer
//============================================================================================
namespace graphics{
    std::shared_ptr<brush> as_brush(sol::object& obj){
        if (obj.is<color>()){
            return obj.as<std::shared_ptr<color>>();
        }else if (obj.is<bitmap>()){
            return obj.as<std::shared_ptr<bitmap>>();
        }
        return nullptr;
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
// transformable: common base class for the object which has capability to transform coordinates
//============================================================================================
namespace graphics{
    void transformable::lua_set_origin_raw(sol::variadic_args va){
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
    }
}

//============================================================================================
// geometry: base class for geometrys
//============================================================================================
namespace graphics{
    void geometry::draw(const render_target& target, ID2D1Brush* brush, float width, ID2D1StrokeStyle* style,
                        const FloatPoint& offset, float scale_x, float scale_y, float angle){
        auto&& matrix = transformation(offset, scale_x, scale_y, angle);
        target->SetTransform(matrix);
        target->DrawGeometry(*this, brush, width, style);
        target->SetTransform(D2D1::Matrix3x2F::Identity());
    }

    void geometry::fill(const render_target& target, ID2D1Brush* brush, ID2D1Brush* opacity_mask,
                        const FloatPoint& offset, float scale_x, float scale_y, float angle){
        auto&& matrix = transformation(offset, scale_x, scale_y, angle);
        target->SetTransform(matrix);
        target->FillGeometry(*this, brush, opacity_mask);
        target->SetTransform(D2D1::Matrix3x2F::Identity());
    }
}

//============================================================================================
// simple_geometry: built-in geometry generator
//==========================================================================================
namespace graphics{
    class simple_geometry : public geometry{
        CComPtr<ID2D1Geometry> geometry;
    public:
        simple_geometry(ID2D1Geometry* geometry) : geometry(geometry){}
        operator ID2D1Geometry* () override{
            return geometry;
        }
        void lua_set_origin(sol::variadic_args va) override{
            lua_c_interface(*mapper_EngineInstance(), "graphics.simple_geometry:set_origin", [this, &va]{
                lua_set_origin_raw(va);
            });
        }

        static std::shared_ptr<graphics::geometry> rectangle(sol::variadic_args args){
            std::optional<float> x, y, width, height;
            sol::object arg0 = args[0];
            if (arg0.get_type() == sol::type::table){
                sol::table params = arg0;
                x = lua_safevalue<float>(params["x"]);
                y = lua_safevalue<float>(params["y"]);
                width = lua_safevalue<float>(params["width"]);
                height = lua_safevalue<float>(params["height"]);
            }else{
                x = lua_safevalue<float>(args[0]);
                y = lua_safevalue<float>(args[1]);
                width = lua_safevalue<float>(args[2]);
                height = lua_safevalue<float>(args[3]);
            }
            if (!x || !y || !width || !height){
                throw std::runtime_error("invalid argument");
            }
            FloatRect rect{*x, *y, *width, *height};
            CComPtr<ID2D1RectangleGeometry> rectangle;
            d2d_factory->CreateRectangleGeometry(rect, &rectangle);
            return std::make_shared<simple_geometry>(rectangle);
        }

        static std::shared_ptr<graphics::geometry> rounded_rectangle(sol::variadic_args args){
            std::optional<float> x, y, width, height, radius_x, radius_y;
            sol::object arg0 = args[0];
            if (arg0.get_type() == sol::type::table){
                sol::table params = arg0;
                x = lua_safevalue<float>(params["x"]);
                y = lua_safevalue<float>(params["y"]);
                width = lua_safevalue<float>(params["width"]);
                height = lua_safevalue<float>(params["height"]);
                radius_x = lua_safevalue<float>(params["radius_x"]);
                radius_y = lua_safevalue<float>(params["radius_y"]);
            }else{
                x = lua_safevalue<float>(args[0]);
                y = lua_safevalue<float>(args[1]);
                width = lua_safevalue<float>(args[2]);
                height = lua_safevalue<float>(args[3]);
                radius_x = lua_safevalue<float>(args[4]);
                radius_y = lua_safevalue<float>(args[5]);
            }
            if (!x || !y || !width || !height || !radius_x || !radius_y){
                throw std::runtime_error("invalid argument");
            }
            FloatRect rect{*x, *y, *width, *height};
            D2D1_ROUNDED_RECT rrect{rect, *radius_x, *radius_y};
            CComPtr<ID2D1RoundedRectangleGeometry> rectangle;
            d2d_factory->CreateRoundedRectangleGeometry(rrect, &rectangle);
            return std::make_shared<simple_geometry>(rectangle);
        }

        static std::shared_ptr<graphics::geometry> ellipse(sol::variadic_args args){
            std::optional<float> x, y, radius_x, radius_y;
            sol::object arg0 = args[0];
            if (arg0.get_type() == sol::type::table){
                sol::table params = arg0;
                x = lua_safevalue<float>(params["x"]);
                y = lua_safevalue<float>(params["y"]);
                radius_x = lua_safevalue<float>(params["radius_x"]);
                radius_y = lua_safevalue<float>(params["radius_y"]);
            }else{
                x = lua_safevalue<float>(args[0]);
                y = lua_safevalue<float>(args[1]);
                radius_x = lua_safevalue<float>(args[2]);
                radius_y = lua_safevalue<float>(args[3]);
            }
            if (!x || !y || !radius_x || !radius_y){
                throw std::runtime_error("invalid argument");
            }
            D2D1_ELLIPSE def{*x, *y, *radius_x, *radius_y};
            CComPtr<ID2D1EllipseGeometry> ellipse;
            d2d_factory->CreateEllipseGeometry(def, &ellipse);
            return std::make_shared<simple_geometry>(ellipse);
        }
    };
}

//============================================================================================
// path: path geometry object which encapsulates ID2D1PathGeometry
//==========================================================================================
template <>
std::optional<FloatPoint> lua_safevalue<FloatPoint>(const sol::object& object){
    if (object.get_type() == sol::type::table){
        sol::table def = object;
        auto x = lua_safevalue<float>(def[1]);
        auto y = lua_safevalue<float>(def[2]);
        if (x && y){
            return FloatPoint{*x, *y};
        }else{
            return std::nullopt;
        }
    }else{
        return std::nullopt;
    }
}

namespace graphics{
    enum class fill_mode{
        none = -1,
        winding = D2D1_FILL_MODE_WINDING,
        alternate = D2D1_FILL_MODE_ALTERNATE,
    };
}

template <>
std::optional<graphics::fill_mode> lua_safevalue(const sol::object& object){
    using rtype = std::optional<graphics::fill_mode>;
    if (object.get_type() == sol::type::lua_nil){
        return graphics::fill_mode::none;
    }else if (object.get_type() == sol::type::string){
        auto &&value = lua_safestring(object);
        return value == "none" ? rtype{graphics::fill_mode::none} :
               value == "winding" ? rtype{graphics::fill_mode::winding} :
               value == "alternate" ? rtype{graphics::fill_mode::alternate} :
               std::nullopt;
    }else{
        std::nullopt;
    }
}

namespace graphics{
    class path : public geometry{
        CComPtr<ID2D1PathGeometry> path_object{nullptr};
        CComPtr<ID2D1GeometrySink> sink{nullptr};
    public:
        path(sol::object arg){
            d2d_factory->CreatePathGeometry(&path_object);
            path_object->Open(&sink);
            if (arg.get_type() != sol::type::lua_nil){
                add_figure(arg);
            }
        }

        virtual ~path(){
            fix();
        };

        void add_figure_lua(sol::object arg){
            lua_c_interface(*mapper_EngineInstance(), "graphics.path:add_figure_lua", [this, &arg]{
                add_figure(arg);
            });
        }

        void fix(){
            if (sink){
                sink->Close();
                sink = nullptr;
            }
        }

        void lua_set_origin(sol::variadic_args va) override{
            lua_c_interface(*mapper_EngineInstance(), "graphics.path:set_origin", [this, &va]{
                lua_set_origin_raw(va);
            });
        }
        
        operator ID2D1Geometry* () override{
            fix();
            return path_object;
        }

    protected:
        void add_figure(sol::object arg){
            if (!sink){
                throw std::runtime_error("no more adding figure due to fixed path geometory, "
                                         "note that path is in fixed state once path is used to draw or to fill");
            }
            if (arg.get_type() != sol::type::table){
                throw std::runtime_error("argument of this funciton must be a table");
            }
            sol::table def = arg;
            auto&& from = lua_safevalue<FloatPoint>(def["from"]);
            auto&& fill_mode = lua_safevalue<graphics::fill_mode>(def["fill_mode"]);
            if (!from){
                throw std::runtime_error("'from' parameter is not specified or invalid format data is specified");
            }
            if (!fill_mode){
                throw std::runtime_error("the value of 'fill_mode' prameter must be 'none', 'winding', or 'alternate'");
            }
            sink->BeginFigure(*from, *fill_mode == fill_mode::none ? D2D1_FIGURE_BEGIN_HOLLOW :  D2D1_FIGURE_BEGIN_FILLED);
            if (*fill_mode != fill_mode::none){
                sink->SetFillMode(static_cast<D2D1_FILL_MODE>(*fill_mode));
            }

            auto segments = def["segments"];
            if (segments.get_type() != sol::type::table){
                throw std::runtime_error("'segments' parameter must be specified as a table");
            }
            sol::table segments_def = segments;
            auto same_as_start_point = false;
            for (auto i = 1; i <= segments_def.size(); i++){
                auto throw_error = [i](const char* msg) -> int{
                    std::ostringstream os;
                    write_ordinal_string(os, i);
                    os << " element of 'segments' parameter is invalid, " << msg;
                    throw std::runtime_error(os.str());
                };
                auto element = segments_def[i];
                if (element.get_type() != sol::type::table){
                    throw_error("that must be a table");
                }
                sol::table element_def = element;
                auto&& to = lua_safevalue<FloatPoint>(element_def["to"]);
                auto&& radius = lua_safevalue<float>(element_def["radius"]);
                auto&& direction = lua_safestring(element_def["direction"]);
                auto&& arc_type = lua_safestring(element_def["arc_type"]);
                auto&& control1 = lua_safevalue<FloatPoint>(element_def["control1"]);
                auto&& control2= lua_safevalue<FloatPoint>(element_def["control2"]);
                if (!to){
                    throw_error("'to' parameter is not found or invalid format");
                }
                same_as_start_point = *from == *to;
                if (radius || direction.size() || arc_type.size()){
                    //---------------------------------------------------------
                    // Arc element deffinition
                    //---------------------------------------------------------
                    if (!radius || direction.size() == 0){
                        throw_error("both of 'radius' parameter and 'direction' parameter must be specified correctly for arc segment definition");
                    }
                    auto direction_value = direction == "clockwise" ? D2D1_SWEEP_DIRECTION_CLOCKWISE :
                                           direction == "counter_clockwise" ? D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE :
                                           direction == "counterclockwise" ? D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE :
                                           throw_error("the value of 'direction' parameter must be either of 'clockwise' or 'counterclockwise'");
                    auto arc_type_value = arc_type == "small" ? D2D1_ARC_SIZE_SMALL :
                                          arc_type == "large" ? D2D1_ARC_SIZE_LARGE :
                                          arc_type.size() == 0 ? D2D1_ARC_SIZE_SMALL :
                                          throw_error("value of 'arc_type' parameter must be either of 'small' or 'large'");
                    sink->AddArc(D2D1::ArcSegment(
                        *to, {*radius, *radius}, 0.f, 
                        static_cast<D2D1_SWEEP_DIRECTION>(direction_value),
                        static_cast<D2D1_ARC_SIZE>(arc_type_value)
                    ));
                }else if (control1 || control2){
                    //---------------------------------------------------------
                    // Bezier element deffinition
                    //---------------------------------------------------------
                    if (!control1 || !control2){
                        throw_error("both of 'control1' parametaer and 'control2' parameter must be specified correctly for bezier segment definition");
                    }
                    sink->AddBezier(D2D1::BezierSegment(*control1, *control2, *to));
                }else{
                    //---------------------------------------------------------
                    // Line element deffinition
                    //---------------------------------------------------------
                    sink->AddLine(*to);
                }
            }
            sink->EndFigure(fill_mode != fill_mode::none || same_as_start_point ? D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
        }
    };
}

//============================================================================================
// utility function to check if a lua object can be translated to geometry class pointer
//============================================================================================
namespace graphics{
    std::shared_ptr<geometry> as_geometry(sol::object& obj){
        if (obj.is<path>()){
            return obj.as<std::shared_ptr<path>>();
        }else if (obj.is<geometry>()){
            return obj.as<std::shared_ptr<geometry>>();
        }
        return nullptr;
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
                    throw MapperException("specified bitmap file does not cantain valid image data");
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
            lua_set_origin_raw(va);
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
        auto shift = get_origin();
        shift.x *= dest_rect.width / rect.width;
        shift.y *= dest_rect.height / rect.height;
        auto drect = dest_rect - shift;
        auto bitmap = source->get_d2d_bitmap(target);
        target->DrawBitmap(bitmap, drect, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, rect);
    }

    void bitmap::draw(const render_target& target, const FloatPoint& offset, float scale_x, float scale_y, float angle){
        auto&& matrix = transformation(offset, scale_x, scale_y, angle);
        target->SetTransform(matrix);
        FloatRect drect{0.f, 0.f, rect.width, rect.height};
        auto bitmap = source->get_d2d_bitmap(target);
        target->DrawBitmap(bitmap, drect, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, rect);
        target->SetTransform(D2D1::Matrix3x2F::Identity());
    }

    ID2D1Brush* bitmap::brush_interface(render_target& target){
        if (target_for_brush == target){
            return brush;
        }else{
            target_for_brush = target;
            if (source->width() != rect.width || source->height() != rect.height){
                auto newsrc = std::make_shared<bitmap_source>(std::ceil(rect.width), std::ceil(rect.height));
                auto tmp_bitmap = std::make_unique<bitmap>(newsrc, FloatRect{0.f, 0.f, static_cast<float>(newsrc->width()), static_cast<float>(newsrc->height())});
                auto rt = tmp_bitmap->create_render_target();
                (*rt)->BeginDraw();
                (*rt)->DrawBitmap(source->get_d2d_bitmap(*rt), tmp_bitmap->get_rect_in_source(), 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, rect);
                (*rt)->EndDraw();
                source = newsrc;
                rect = {0.f, 0.f, static_cast<float>(newsrc->width()), static_cast<float>(newsrc->height())};
            }
            D2D1_BITMAP_BRUSH_PROPERTIES props{brush_extend_mode_x, brush_extend_mode_y, brush_interpolation_mode};
            target->CreateBitmapBrush(source->get_d2d_bitmap(target), props, &brush);
            return brush;
        }
    }

    static const char* extend_mode_dict[] = {"clamp", "wrap", "mirror", nullptr};

    static D2D1_EXTEND_MODE str_to_extend_mode(const char* mode){
        for (auto i = 0; extend_mode_dict[i]; i++){
            if (strcmp(mode, extend_mode_dict[i]) == 0){
                return static_cast<D2D1_EXTEND_MODE>(i);
            }
        }
        throw std::runtime_error("brush extend mode must be \"clamp\" , \"wrap\", or \"mirror\"");
    }

    const char* bitmap::get_brush_extend_mode_x(){
        return extend_mode_dict[brush_extend_mode_x];
    }

    void bitmap::set_brush_extend_mode_x(const char* mode){
        lua_c_interface(*mapper_EngineInstance(), "graphics.bitmap:set_brush_extend_mode_x", [this, &mode]{
            brush_extend_mode_x = str_to_extend_mode(mode);
        });
    }

    const char* bitmap::get_brush_extend_mode_y(){
        return extend_mode_dict[brush_extend_mode_y];
    }

    void bitmap::set_brush_extend_mode_y(const char* mode){
        lua_c_interface(*mapper_EngineInstance(), "graphics.bitmap:set_brush_extend_mode_y", [this, &mode]{
            brush_extend_mode_y = str_to_extend_mode(mode);
        });
    }

    static const char* interpolation_mode_dict[] = {"nearest_neighbor", "linear", nullptr};

    static D2D1_BITMAP_INTERPOLATION_MODE str_to_interpolation_mode(const char* mode){
        for (auto i = 0; interpolation_mode_dict[i]; i++){
            if (strcmp(mode, interpolation_mode_dict[i]) == 0){
                return static_cast<D2D1_BITMAP_INTERPOLATION_MODE>(i);
            }
        }
        throw std::runtime_error("brush interpolation mode must be \"nearest_neighbor\" or \"linear\"");
    }

    const char* bitmap::get_brush_interpolation_mode(){
        return interpolation_mode_dict[brush_interpolation_mode];
    }

    void bitmap::set_brush_interpolation_mode(const char* mode){
        lua_c_interface(*mapper_EngineInstance(), "graphics.bitmap:set_brush_interpolation_mode", [this, &mode]{
            brush_interpolation_mode = str_to_interpolation_mode(mode);
        });
    }
}

//============================================================================================
// font:
//============================================================================================
namespace graphics{
    std::shared_ptr<font> as_font(sol::object& obj){
        std::shared_ptr<font> font;
        if (obj.is<system_font>()){
            font = obj.as<std::shared_ptr<system_font>>();
        }else if (obj.is<bitmap_font>()){
            font = obj.as<std::shared_ptr<bitmap_font>>();
        }
        return font;
    }

    std::unordered_map<std::string, DWRITE_FONT_STYLE> system_font::style_map{
        {"normal", DWRITE_FONT_STYLE_NORMAL},
        {"oblique", DWRITE_FONT_STYLE_OBLIQUE},
        {"italic", DWRITE_FONT_STYLE_ITALIC},
    };

    system_font::system_font(const char* font_family, unsigned weight, const char* style, float height) : 
        font_family(font_family), weight(weight), style(style), height(height) {
        tools::utf8_to_utf16_translator utf16string{font_family};
        font_family_utf16 = utf16string;
        if (this->style.size() == 0){
            style_value = DWRITE_FONT_STYLE_NORMAL;
        }else if (style_map.count(this->style) > 0){
            style_value = style_map[this->style];
        }else{
            throw MapperException("font style must be either 'normal', 'oblique', or 'italic'");
        }
    }

    FloatRect system_font::draw_string(
        const render_target &target, const char *string, ID2D1Brush *brush, const FloatRect &rect, float scale, valign v_align, halign h_align){
        float dpi_x, dpi_y;
        target->GetDpi(&dpi_x, &dpi_y);
        float exact_height = height / dpi_y * 96 * scale;
        if (!text_format || exact_height != height_in_dpi || this->h_align != h_align || this->v_align != v_align){
            text_format = nullptr;
            height_in_dpi = exact_height;
            this->h_align = h_align;
            this->v_align = v_align;
            auto rc = dwrite_factory->CreateTextFormat(
                font_family_utf16.c_str(), nullptr,
                static_cast<DWRITE_FONT_WEIGHT>(weight), style_value, DWRITE_FONT_STRETCH_NORMAL,
                exact_height, L"en-us",
                &text_format);
            if (rc != S_OK){
                std::ostringstream os;
                os << "failed to create a DirectWrite text format object when rendering the text:\n";
                os << "    Font Family: " << font_family << "\n";
                os << "    Font Weight: " << weight << "\n";
                os << "    Font Style: " << style;
                mapper_EngineInstance()->putLog(MCONSOLE_WARNING, os.str().c_str());
            }
            text_format->SetTextAlignment(
                h_align == halign::left   ? DWRITE_TEXT_ALIGNMENT_LEADING :
                h_align == halign::center ? DWRITE_TEXT_ALIGNMENT_CENTER :
                h_align == halign::right  ? DWRITE_TEXT_ALIGNMENT_TRAILING :
                                            DWRITE_TEXT_ALIGNMENT_LEADING);
            text_format->SetParagraphAlignment(
                v_align == valign::top    ? DWRITE_PARAGRAPH_ALIGNMENT_NEAR :
                v_align == valign::center ? DWRITE_PARAGRAPH_ALIGNMENT_CENTER :
                v_align == valign::bottom ? DWRITE_PARAGRAPH_ALIGNMENT_FAR :
                                            DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        }
        tools::utf8_to_utf16_translator utf16string{string};
        if (brush){
            target->DrawText(utf16string, utf16string.size(), text_format, rect, brush);
        }
        return {};
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

            if (code_point.size() != 1 || 
                static_cast<uint8_t>(code_point.c_str()[0]) < code_point_min || 
                static_cast<uint8_t>(code_point.c_str()[1]) > code_point_max){
                throw MapperException("string that represents code point is not correct, "
                                      "it must be one charactor and must be within the range of unsigned 8bit integer");
            }
            if (!may_be_bitmap.is<bitmap&>()){
                throw MapperException("bitmap parameter is not specified or specified value is not bitmap object");
            }
            auto bitmap = may_be_bitmap.as<std::shared_ptr<graphics::bitmap>>();
            add_glyph(static_cast<uint8_t>(code_point.c_str()[0]), bitmap);
        });
    }

    FloatRect bitmap_font::draw_string(
        const render_target &target, const char *string, ID2D1Brush *brush, const FloatRect &rect, float scale, valign v_align, halign h_align){
        FloatRect orect{rect.x, rect.y, 0, 0};
        for (const uint8_t* code = reinterpret_cast<const uint8_t*>(string); *code; code++){
            if (*code >= code_point_min && *code <= code_point_max){
                const auto& glyph = glyphs[*code - code_point_min];
                if (glyph){
                    FloatRect target_rect{
                        orect.x + orect.width,
                        orect.y, 
                        glyph->get_width() * scale,
                        glyph->get_height() * scale
                    };
                    glyph->draw(target, target_rect);
                    auto out_rect = target_rect - glyph->get_origin();
                    orect += out_rect;
                }
            }
        }
        return orect;
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

    std::shared_ptr<brush> as_brush_or_nil(sol::object obj){
        if (obj.get_type() == sol::type::lua_nil){
            return nullptr;
        }else{
            auto brush_ptr = as_brush(obj);
            if (!brush_ptr){
                throw MapperException("specified value is not brush object");
            }
            return brush_ptr;
        }
    }

    void rendering_context::set_brush(sol::object brush){
        lua_c_interface(*mapper_EngineInstance(), "graphics.rendering_context:set_brush", [this, &brush]{
            this->brush = as_brush_or_nil(brush);
        });
    }

    void rendering_context::set_opacity_mask(sol::object mask){
        lua_c_interface(*mapper_EngineInstance(), "graphics.rendering_context:set_opacity_mask", [this, &mask]{
            this->opacity_mask = as_brush_or_nil(mask);
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

    void rendering_context::set_stroke_width(sol::object width){
        lua_c_interface(*mapper_EngineInstance(), "graphics.rendering_context:set_stroke_width", [this, &width]{
            if (width.get_type() == sol::type::lua_nil){
                stroke_width = 1.f;
            }else if (width.get_type() == sol::type::number){
                auto new_width = width.as<float>();
                if (new_width > 0.f){
                    stroke_width = new_width;
                }else{
                    throw MapperException("stroke width must be grater than 0");
                }
            }else{
                throw MapperException("stroke width must be number grater than 0");
            }
        });
    }

    template <typename TFunction>
    void process_geometry(sol::variadic_args args, const FloatPoint& offset, float src_scale, TFunction callback){
            std::shared_ptr<geometry> geometry;
            std::optional<float> x;
            std::optional<float> y;
            std::optional<float> angle;
            std::optional<float> scale;

            sol::object arg0 = args[0];
            geometry = as_geometry(arg0);
            if (geometry){
                x = lua_safevalue<float>(args[1]);
                y = lua_safevalue<float>(args[2]);
                angle = lua_safevalue<float>(args[3]);
                scale = lua_safevalue<float>(args[4]);
            }else if (arg0.get_type() == sol::type::table){
                sol::table def = arg0;
                sol::object geometry_obj = def["geometry"];
                geometry = as_geometry(geometry_obj);
                x = lua_safevalue<float>(def["x"]);
                y = lua_safevalue<float>(def["y"]);
                angle = lua_safevalue<float>(def["angle"]);
                scale = lua_safevalue<float>(def["scale"]);
            }else{
                throw MapperException("invalid parameters");
            }
            if (!geometry){
                throw MapperException("no geometry object is specified");
            }
            x = x ? *x : 0.f;
            y = y ? *y : 0.f;
            angle = angle? *angle : 0.f;
            scale = scale? *scale : 1.f;

            callback(geometry, FloatPoint(*x * src_scale, *y * src_scale) + offset, *angle, *scale * src_scale);
    }

    void rendering_context::draw_geometry(sol::variadic_args args){
        lua_c_interface(*mapper_EngineInstance(), "graphics.rendering_context:draw_geometry", [this, &args]{
            process_geometry(args, {this->rect.x, this->rect.y}, this->scale, [this](auto geometry, auto offset, auto angle, auto scale){
                if (this->brush){
                    geometry->draw(*target, brush->brush_interface(*target), stroke_width, nullptr, offset, scale, scale, angle);
                }
            });
        });
    }

    void rendering_context::fill_geometry(sol::variadic_args args){
        lua_c_interface(*mapper_EngineInstance(), "graphics.rendering_context:fill_geometry", [this, &args]{
            process_geometry(args, {this->rect.x, this->rect.y}, this->scale, [this](auto geometry, auto offset, auto angle, auto scale){
                if (this->brush){
                    geometry->fill(*target, brush->brush_interface(*target),
                                   opacity_mask ? opacity_mask->brush_interface(*target) : nullptr,
                                   offset, scale, scale, angle);
                }
            });
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
            std::optional<float> scale;
            FloatRect drect{0.f, 0.f, 0.f, 0.f};
            sol::object arg0 = args[0];
            if (arg0.is<graphics::bitmap&>()){
                bitmap = arg0.as<std::shared_ptr<graphics::bitmap>>();
                x = lua_safevalue<float>(args[1]);
                y = lua_safevalue<float>(args[2]);
                width = lua_safevalue<float>(args[3]);
                height = lua_safevalue<float>(args[4]);
                angle = lua_safevalue<float>(args[5]);
                scale = lua_safevalue<float>(args[6]);
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
                scale = lua_safevalue<float>(def["scale"]);
            }else{
                throw MapperException("invalid parameters");
            }

            scale = scale ? *scale * this->scale : this->scale;
            drect.width = bitmap->get_width() * *scale;
            drect.height = bitmap->get_height() * *scale;
            if (x && y){
                drect.x = *x * this->scale;
                drect.y = *y * this->scale;
            }
            if (width && height){
                drect.width = *width * *scale;
                drect.height = *height * *scale;
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
                throw MapperException("invalid argument");
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
            sol::object arg0 = args[0];
            if (arg0.get_type() == sol::type::string){
                string = std::move(lua_safestring(arg0));
            }else if (arg0.get_type() == sol::type::table){
                sol::table def = arg0;
                string = std::move(lua_safestring(def["string"]));
            }else{
                throw MapperException("invalid parameters");
            }
            FloatRect rect;
            valign v_align;
            halign h_align;
            extract_region(args, rect, v_align, h_align);
            draw_string_native(string.c_str(), rect, v_align, h_align);
        });
    }

    void rendering_context::draw_number(sol::variadic_args args){
        lua_c_interface(*mapper_EngineInstance(), "graphics.rendering_context:draw_number", [this, &args]{
            std::optional<double> value;
            std::optional<int> fraction_precision;
            std::optional<int> precision;
            std::optional<bool> leading_zero;

            sol::object arg0 = args[0];
            if (arg0.get_type() == sol::type::number){
                value = lua_safevalue<double>(arg0);
            }else if (arg0.get_type() == sol::type::table){
                sol::table def = arg0;
                value = lua_safevalue<double>(def["value"]);
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

            FloatRect rect;
            valign v_align;
            halign h_align;
            extract_region(args, rect, v_align, h_align);
            draw_string_native(os.str().c_str(), rect, v_align, h_align);
        });
    }

    void rendering_context::extract_region(const sol::variadic_args &args, FloatRect &rect, valign &v_align, halign &h_align){
        static std::unordered_map<std::string, valign> valign_map{
            {"top", valign::top},
            {"center", valign::center},
            {"bottom", valign::bottom},
        };
        static std::unordered_map<std::string, halign> halign_map{
            {"left", halign::left},
            {"center", halign::center},
            {"right", halign::right},
        };
        rect = {0.f, 0.f, std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
        v_align = valign::top;
        h_align = halign::left;
        std::optional<float> x;
        std::optional<float> y;
        std::optional<float> width;
        std::optional<float> height;
        std::string v_align_string;
        std::string h_align_string;
        sol::object arg0 = args[0];
        if (arg0.get_type() == sol::type::table){
            sol::table def = arg0;
            x = lua_safevalue<float>(def["x"]);
            y = lua_safevalue<float>(def["y"]);
            width = lua_safevalue<float>(def["width"]);
            height = lua_safevalue<float>(def["height"]);
            h_align_string = lua_safestring(def["horizontal_alignment"]);
            v_align_string = lua_safestring(def["vertical_alignment"]);
        }else{
            x = lua_safevalue<float>(args[1]);
            y = lua_safevalue<float>(args[2]);
            width = lua_safevalue<float>(args[3]);
            height = lua_safevalue<float>(args[4]);
            h_align_string = lua_safestring(args[5]);
            v_align_string = lua_safestring(args[6]);
        }
        if (x) rect.x = *x;
        if (y) rect.y = *y;
        if (width) rect.width = *width;
        if (height) rect.height = *height;
        if (v_align_string.size() > 0){
            if (valign_map.count(v_align_string)){
                v_align = valign_map[v_align_string];
            }else{
                throw MapperException("the 'vertical_alignment' parameter must be either 'top', 'center', or 'buttom'");
            }
        }
        if (h_align_string.size() > 0){
            if (halign_map.count(h_align_string)){
                h_align = halign_map[h_align_string];
            }else{
                throw MapperException("the 'horizontal_alignment' parameter must be either 'left', 'center', or 'right'");
            }
        }
    }

    void rendering_context::draw_string_native(const char* string, const FloatRect& rect, valign v_align, halign h_align){
        auto orect{rect};
        translate_to_context_coordinate(orect);
        font->draw_string(*target, string, brush ? brush->brush_interface(*target): nullptr, orect, this->scale, v_align, h_align);
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
    // simple geometies
    //
    table["rectangle"] = [&engine](sol::variadic_args args){
        return lua_c_interface(engine, "graphics.rectangle", [&args](){
            return simple_geometry::rectangle(args);
        });
    };
    table["rounded_rectangle"] = [&engine](sol::variadic_args args){
        return lua_c_interface(engine, "graphics.rounded_rectangle", [&args](){
            return simple_geometry::rounded_rectangle(args);
        });
    };
    table["ellipse"] = [&engine](sol::variadic_args args){
        return lua_c_interface(engine, "graphics.ellipse", [&args](){
            return simple_geometry::ellipse(args);
        });
    };

    //
    // path
    //
    table.new_usertype<graphics::path>(
        "path",
        sol::call_constructor, sol::factories([&engine](sol::variadic_args va){
            sol::object arg = va[0];
            return lua_c_interface(engine, "graphics.path", [arg](){
                return std::make_shared<graphics::path>(arg);
            });
        }),
        "add_figure", &path::add_figure_lua,
        "fix", &path::fix,
        "set_origin", &path::lua_set_origin
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
                    auto width = lua_safevalue<float>(va[0]);
                    auto height = lua_safevalue<float>(va[1]);
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
        "create_partial_bitmap", &bitmap::create_partial_bitmap,
        "brush_extend_mode_x", sol::property(&bitmap::get_brush_extend_mode_x, &bitmap::set_brush_extend_mode_x),
        "brush_extend_mode_y", sol::property(&bitmap::get_brush_extend_mode_y, &bitmap::set_brush_extend_mode_y),
        "brush_interpolation_mode", sol::property(&bitmap::get_brush_interpolation_mode, &bitmap::set_brush_interpolation_mode)
    );

    //
    // system_font
    //
    table.new_usertype<graphics::system_font>(
        "system_font",
        sol::call_constructor, sol::factories([&engine](sol::object arg){
            return lua_c_interface(engine, "graphics.system_font", [&arg](){
                if (arg.get_type() != sol::type::table){
                    throw MapperException("argument of this funciton must be a table");
                }
                sol::table params = arg;
                auto family_name = lua_safestring(params["family_name"]);
                auto weight = lua_safevalue<int>(params["weight"]);
                auto style = lua_safestring(params["style"]);
                auto height = lua_safevalue<float>(params["height"]);
                if (family_name.size() == 0){
                    throw MapperException("'family_name' parameter must be specified");
                }
                if (weight && (*weight < 1 || *weight > 999)){
                    throw MapperException("the value of `weight` parameter must be between 1 and 999");
                }
                if (!height){
                    throw MapperException("'height' parameter' must be specified");
                }
                return std::make_shared<system_font>(
                    family_name.c_str(), 
                    weight ? *weight : DWRITE_FONT_WEIGHT_NORMAL,
                    style.size() == 0 ? "normal" : style.c_str(),
                    *height);
            });
        })
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

        "brush", sol::property(&rendering_context::get_brush, &rendering_context::set_brush),
        "opacity_mask", sol::property(&rendering_context::get_opacity_mask, &rendering_context::set_opacity_mask),
        "font", sol::property(&rendering_context::get_font, &rendering_context::set_font),
        "stroke_width", sol::property(&rendering_context::get_stroke_width, &rendering_context::set_stroke_width),

        "finish_rendering", &graphics::rendering_context::finish_rendering,
        "set_brush", &graphics::rendering_context::set_brush,
        "set_opacity_mask", &graphics::rendering_context::set_opacity_mask,
        "set_font", &graphics::rendering_context::set_font,
        "set_stroke_width", &graphics::rendering_context::set_stroke_width,
        "draw_geometry", &graphics::rendering_context::draw_geometry,
        "fill_geometry", &graphics::rendering_context::fill_geometry,
        "draw_bitmap", &graphics::rendering_context::draw_bitmap,
        "draw_string", &graphics::rendering_context::draw_string,
        "draw_number", &graphics::rendering_context::draw_number,
        "fill_rectangle", &graphics::rendering_context::fill_rectangle
    );
}
