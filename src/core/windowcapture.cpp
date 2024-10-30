//
// windowcapture.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "windowcapture.h"
#include "composition.h"
#include "engine.h"
#include "viewport.h"

#include <optional>
#include <cmath>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.UI.Popups.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3d11.h>

#include <windows.ui.composition.interop.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.directx.direct3d11.interop.h>

extern "C" {
    HRESULT __stdcall CreateDirect3D11DeviceFromDXGIDevice(
        ::IDXGIDevice* dxgiDevice,
        ::IInspectable** graphicsDevice);

    HRESULT __stdcall CreateDirect3D11SurfaceFromDXGISurface(
        ::IDXGISurface* dgxiSurface,
        ::IInspectable** graphicsSurface);
}


//============================================================================================
// Utility functions
//============================================================================================
std::optional<FloatRect> parese_rect_def(sol::object object){
    if (object.get_type() == sol::type::table){
        sol::table def = object;
        auto x = lua_safevalue<float>(def["x"]);
        auto y = lua_safevalue<float>(def["y"]);
        auto width = lua_safevalue<float>(def["width"]);
        auto height = lua_safevalue<float>(def["height"]);
        FloatRect rc{
            x ? *x : 0, 
            y ? *y : 0,
            width ? *width : 0, 
            height ? *height : 0};
        if (rc.width != 0, height != 0){
            return rc;
        }
    }
    return std::nullopt;
}

auto create_capture_item_for_window(HWND hwnd){
    auto activation_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
    auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem item {nullptr};
    interop_factory->CreateForWindow(
        hwnd, 
        winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
        reinterpret_cast<void **>(winrt::put_abi(item))
    );
    return item;
}

template <typename T>
auto get_dxgi_interface_from_object(winrt::Windows::Foundation::IInspectable const &object){
    auto access = object.as<Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();
    winrt::com_ptr<T> result;
    winrt::check_hresult(access->GetInterface(winrt::guid_of<T>(), result.put_void()));
    return result;
}

//============================================================================================
// Window image streamer
//============================================================================================
namespace capture{
    class image_streamer_imp : public image_streamer{
    protected:
        ViewPortManager& manager;
        uint32_t id;
        std::string name{"DCS World"};
        std::vector<std::string> target_titles;
        bool was_disposed{false};
        HWND hwnd{nullptr};
        FloatRect capture_rect{0, 0, 0, 0};
        D3D11_BOX src_rect;
        winrt::Windows::Graphics::Capture::GraphicsCaptureItem capture_item{nullptr};
        winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice device{nullptr};
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool frame_pool{nullptr};
        winrt::Windows::Graphics::Capture::GraphicsCaptureSession session{nullptr};
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::FrameArrived_revoker frame_arrived;
        CComPtr<ID3D11Device> d3d_device;
        CComPtr<IDXGISwapChain1> swap_chain;
        CComPtr<ID3D11DeviceContext> d3d_context;

        uint64_t stat{0};

    public:
        image_streamer_imp(ViewPortManager&manager, sol::object def_obj, uint32_t id): manager(manager), id(id){
            if (def_obj.get_type() != sol::type::table){
                throw MapperException("function argument is not a table");
            }
            auto def = def_obj.as<sol::table>();
            auto name = lua_safevalue<std::string>(def["name"]);
            if (name && name->size() > 0){
                this->name = std::move(*name);
            }
            sol::object titles_obj = def["window_titles"];
            if (titles_obj.get_type() == sol::type::table){
                sol::table titles = titles_obj;
                for (int i = 1; i <= titles.size(); i++){
                    auto title = lua_safevalue<std::string>(titles[i]);
                    if (title && title->size() > 0){
                        target_titles.emplace_back(std::move(*title));
                    }else{
                        throw MapperException("each element of 'window_titles' have to be specified as a string");
                    }
                }
            }else if (titles_obj.valid()){
                throw MapperException("'window_titles' parameter for captured window definition must be a table");
            }
            auto title = lua_safestring(def["window_title"]);
            if (title.size() > 0){
                target_titles.emplace_back(std::move(title));
            }
            if (target_titles.size() == 0){
                target_titles.emplace_back("Digital Combat Simulator");
            }

            auto&& rect = parese_rect_def(def["capture_rect"]);
            if (rect){
                capture_rect = *rect;
            }
        }

        uint32_t get_id() const override{return id;}
        const char* get_name() const override{return name.c_str();}
        HWND get_hwnd() const override{return hwnd;}
        const std::vector<std::string>& get_target_titles() const override{return target_titles;}

        void set_hwnd(HWND handle) override{
            hwnd = handle;
            if (hwnd){
                try{
                    capture_item = create_capture_item_for_window(hwnd);
                }catch (...){
                    mapper_EngineInstance()->putLog(MCONSOLE_WARNING, "image_streamer: The specified window cannot be used for image capture.");
                    capture_item = nullptr;
                }
            }else{
                capture_item = nullptr;
            }
        }

        virtual ~image_streamer_imp(){
            stop_capture();
        }

        void start_capture() override{
            if (was_disposed || capture_rect.width <= 0 || capture_rect.height <= 0){
                return;
            }

            auto queue = winrt::Windows::System::DispatcherQueue::GetForCurrentThread();

            stat = 0;
            d3d_device = composition::create_d3d_device();
            d3d_device->GetImmediateContext(&d3d_context);
            CComPtr<IDXGIDevice> dxgi_device;
            d3d_device.QueryInterface(&dxgi_device);
            winrt::com_ptr<::IInspectable> dev{nullptr};
            CreateDirect3D11DeviceFromDXGIDevice(dxgi_device, dev.put());
            device = dev.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();

            if (capture_item){
                auto size = capture_item.Size();
                src_rect.front = 0;
                src_rect.back = 0;
                src_rect.left = std::floor(capture_rect.x);
                src_rect.right = std::ceil(capture_rect.x + capture_rect.width);
                src_rect.top = std::floor(capture_rect.y);
                src_rect.bottom = std::ceil(capture_rect.y + capture_rect.height);
                swap_chain = composition::create_swapchain(dxgi_device, std::ceil(capture_rect.width), std::ceil(capture_rect.height));
                // swap_chain = composition::create_swapchain(dxgi_device, size.Width, size.Height);
                frame_pool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::CreateFreeThreaded(
                    device,
                    winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
                    2,
                    size);
                session = frame_pool.CreateCaptureSession(capture_item);
                session.IsBorderRequired(false);
                session.IsCursorCaptureEnabled(false);
                frame_arrived = frame_pool.FrameArrived(winrt::auto_revoke, {this, &image_streamer_imp::on_frame_arrived});
                session.StartCapture();
            }else{
                swap_chain = composition::create_swapchain(dxgi_device, std::ceil(capture_rect.width), std::ceil(capture_rect.height));
            }
        }

        void stop_capture() override{
            if (frame_pool){
                frame_arrived.revoke();
                frame_pool.Close();
                session.Close();

                session = nullptr;
                frame_pool = nullptr;
            }
            swap_chain = nullptr;
            device = nullptr;
            d3d_context = nullptr;
            d3d_device = nullptr;
        }

        void dispose() override{
            stop_capture();
            was_disposed = true;
        }

    protected:
        void on_frame_arrived(
            winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender, 
            winrt::Windows::Foundation::IInspectable const&){
            stat++;
            auto frame = sender.TryGetNextFrame();
            auto frameSurface = get_dxgi_interface_from_object<ID3D11Texture2D>(frame.Surface());
            
            winrt::com_ptr<ID3D11Texture2D> backBuffer;
            swap_chain->GetBuffer(0, winrt::guid_of<ID3D11Texture2D>(), backBuffer.put_void());

            d3d_context->CopySubresourceRegion(backBuffer.get(), 0, 0, 0, 0, frameSurface.get(), 0, &src_rect);
            // d3d_context->CopyResource(backBuffer.get(), frameSurface.get());

            DXGI_PRESENT_PARAMETERS present_parameters{0};
            swap_chain->Present1(1, 0, &present_parameters);
        }
    };

    std::shared_ptr<image_streamer> create_image_streamer(ViewPortManager&manager, uint32_t id, sol::object arg){
        return std::make_shared<image_streamer_imp>(manager, arg, id);
    }
}

//============================================================================================
// Initialize Lua scripting environment
//
//  Note: 
//  The environment for image_streamer is initialized in the start up procedure of Viewport
//  manager. Refer 'viewport.cpp'
//============================================================================================
namespace capture{
    void init_scripting_env(sol::table& mapper_table){
    }
}
