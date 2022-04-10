//
// graphics.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "graphics.h"

#include <stdexcept>

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