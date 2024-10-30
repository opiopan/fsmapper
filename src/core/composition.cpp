//
// composition.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <d3d11_2.h>
#include <d2d1_2helper.h>
#include <dcomp.h>
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dcomp")

#include <system_error>

#include "composition.h"
#include "engine.h"

//============================================================================================
// Utility functions
//============================================================================================
class ComAssertion{
protected:
    HRESULT hr;

public:
    ComAssertion(HRESULT hr = S_OK) {
        operator = (hr);
    }

    inline HRESULT operator = (HRESULT hr){
        this->hr = hr;
        if (hr != S_OK){
            mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, std::system_category().message(hr));
        }
        return hr;
    }

    HRESULT operator()(){return hr;}
};

static CComPtr<ID3D11Device> create_d3d_device(){
    ComAssertion hr;
    CComPtr<ID3D11Device> d3d_device;
    hr = D3D11CreateDevice(
        nullptr, // Adapter
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr, // Module
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr, 0, // Highest available feature level
        D3D11_SDK_VERSION,
        &d3d_device,
        nullptr, // Actual feature level
        nullptr
    );
    return d3d_device;
}

static CComPtr<IDXGIDevice> create_dxgi_device(){
    ComAssertion hr;
    auto d3d_device = create_d3d_device();
    CComPtr<IDXGIDevice> dxgi_device;
    hr = d3d_device.QueryInterface(&dxgi_device);
    return dxgi_device;
}

CComPtr<IDXGISwapChain1> create_swapchain(IDXGIDevice* dxgi_device, UINT width, UINT height){
    ComAssertion hr;

    CComPtr<IDXGIFactory2> dxgi_factory;
    hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, __uuidof(dxgi_factory), reinterpret_cast<void**>(dxgi_factory.operator &()));
    DXGI_SWAP_CHAIN_DESC1 description = {};
    description.Format           = DXGI_FORMAT_B8G8R8A8_UNORM;     
    description.BufferUsage      = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.SwapEffect       = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    description.BufferCount      = 2;                              
    description.SampleDesc.Count = 1;                              
    description.AlphaMode        = DXGI_ALPHA_MODE_PREMULTIPLIED;
    description.Width            = width;
    description.Height           = height;

    CComPtr<IDXGISwapChain1> swap_chain;
    hr = dxgi_factory->CreateSwapChainForComposition(dxgi_device, &description, nullptr, &swap_chain);

    return swap_chain;
}

CComPtr<ID2D1Device1> create_d2d_device(IDXGIDevice* dxgi_device){
    ComAssertion hr;
    CComPtr<ID2D1Factory2> factory;
    D2D1_FACTORY_OPTIONS const options = {D2D1_DEBUG_LEVEL_INFORMATION};
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, &factory);
    CComPtr<ID2D1Device1> device;
    hr = factory->CreateDevice(dxgi_device, &device);
    return device;
}

//============================================================================================
// Building devices
//============================================================================================
namespace composition{
    CComPtr<ID3D11Device> create_d3d_device(){
        return ::create_d3d_device();
    }

    CComPtr<IDXGISwapChain1> create_swapchain(IDXGIDevice *device, UINT width, UINT height){
        return ::create_swapchain(device, width, height);
    }

    CComPtr<IDXGISwapChain1> create_swapchain(UINT width, UINT height){
        CComPtr<IDXGIDevice> dxgi_device = create_dxgi_device();
        return ::create_swapchain(dxgi_device, width, height);
    }
}

//============================================================================================
// capsulizeing of the composition target for a viewport
//============================================================================================
namespace composition{
    class viewport_target_imp: public composition::viewport_target{
    protected:
        HWND hwnd;
        UINT width;
        UINT height;
        CComPtr<IDXGIDevice> dxgi_device;
        CComPtr<IDXGISwapChain1> swap_chain;
        CComPtr<IDCompositionDevice> dcomp_device;
        CComPtr<IDCompositionTarget> target;
        CComPtr<ID2D1Device1> d2d_device;
        CComPtr<ID2D1DeviceContext> d2d_context;
        D2D1_BITMAP_PROPERTIES1 bitmap_props{};
        CComPtr<IDCompositionVisual> root_visual;
        CComPtr<IDCompositionVisual> main_visual;

    public:
        viewport_target_imp(HWND hwnd, UINT width, UINT height) : hwnd(hwnd), width(width), height(height){
            ComAssertion hr;
            dxgi_device = create_dxgi_device();
            swap_chain = ::create_swapchain(dxgi_device, width, height);
            hr = DCompositionCreateDevice(dxgi_device, __uuidof(dcomp_device), reinterpret_cast<void**>(dcomp_device.operator&()));
            hr = dcomp_device->CreateTargetForHwnd(hwnd, true, &target);
            hr = dcomp_device->CreateVisual(&root_visual);
            hr = target->SetRoot(root_visual);
            hr = dcomp_device->Commit();
            hr = dcomp_device->CreateVisual(&main_visual);
            hr = main_visual->SetContent(swap_chain);
            d2d_device = create_d2d_device(dxgi_device);
            hr = d2d_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &d2d_context);
            bitmap_props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
            bitmap_props.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
            bitmap_props.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
            configure_d2d_context();
        }

        virtual ~viewport_target_imp(){
        }

        ID2D1RenderTarget* get_render_target() override {
            return d2d_context.operator->();
        }

        void reset_visual_tree() override{
            ComAssertion hr = root_visual->RemoveAllVisuals();
        }

        void commit_visual_tree(bool show_main_visual) override {
            ComAssertion hr;
            if (show_main_visual){
                hr = root_visual->AddVisual(main_visual, false, nullptr);
            }
            hr = dcomp_device->Commit();
        }

        void present(){
            ComAssertion hr = swap_chain->Present(1, 0);
            configure_d2d_context();
        }

    protected:
        void configure_d2d_context(){
            ComAssertion hr;
            CComPtr<IDXGISurface2> surface;
            hr = swap_chain->GetBuffer(0, __uuidof(surface), reinterpret_cast<void**>(surface.operator&()));
            CComPtr<ID2D1Bitmap1> bitmap;
            hr = d2d_context->CreateBitmapFromDxgiSurface(surface, bitmap_props, &bitmap);
            d2d_context->SetTarget(bitmap);
        }
    };

    std::unique_ptr<viewport_target> create_viewport_target(HWND hwnd, UINT width, UINT height){
        return std::make_unique<viewport_target_imp>(hwnd, width, height);
    }
}