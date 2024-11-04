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
#include <atomic>

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
std::optional<FloatRect> parse_rect_def(sol::object object){
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
        if (rc.width > 0 && height > 0){
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
// Captured image view element
//============================================================================================
class captured_image_imp : public capture::captured_image{
    FloatRect clip_rect{0, 0, 0, 0};
    view_utils::alignment_opt alignment;
    bool was_disposed{false};
    ViewPort* viewport{nullptr};
    const FloatRect& capture_rect;
    CComPtr<IDXGISwapChain1>& swap_chain;
    CComPtr<IDCompositionVisual> visual{nullptr};

public:
    captured_image_imp(sol::object def_object, const FloatRect& capture_rect, CComPtr<IDXGISwapChain1>& swap_chain) : 
        capture_rect(capture_rect), swap_chain(swap_chain){
        if (def_object.get_type() != sol::type::table){
            throw MapperException("The argument must be specified as a table");
        }
        sol::table def = def_object;
        auto rect = parse_rect_def(def_object);
        if (!rect){
            throw MapperException(
                "There is an error in specifying which rectangular area of the image captured by the WindowImageStreamer "
                "object this object displays. "
                "Make sure that the x, y, width, and height parameters are correctly specified.");
        }
        clip_rect = *rect;
        alignment = {def};
    }

    virtual ~captured_image_imp() = default;

    void associate_viewport(ViewPort* viewport, composition::viewport_target& target) override{
        check_disposed();
        if (this->viewport && !viewport &&  viewport != this->viewport){
            throw MapperException(
                "he CapturedImage object is already associated with a view that belongs to a "
                "different viewport than the specified view's viewport");
        }
        this->viewport = viewport;
        visual = target.create_visual();
        visual->SetContent(swap_chain);
    }

    void set_bounds(const FloatRect& bounds) override{
        // calcurate the scale and target rectangle
        check_disposed();
        FloatRect to_rect = bounds;
        float scale = 1.0;
        if (bounds.width / bounds.height >  clip_rect.width / clip_rect.height){
            scale = bounds.height / clip_rect.height;
            to_rect.width = clip_rect.width * scale;
            if (alignment.h == view_utils::horizontal_alignment::center){
                to_rect.x += (bounds.width - to_rect.width) / 2;
            }else if (alignment.h == view_utils::horizontal_alignment::right){
                to_rect.x += bounds.width - to_rect.width;
            }
        }else{
            scale = bounds.width / clip_rect.width;
            to_rect.height = clip_rect.height * scale;
            if (alignment.v == view_utils::vertical_alignment::center){
                to_rect.y += (bounds.height - to_rect.height) / 2;
            }else if (alignment.v == view_utils::vertical_alignment::bottom){
                to_rect.y += bounds.height - to_rect.height;
            }
        }

        // set the clip region to the visual
        D2D_RECT_F rect;
        rect.left = clip_rect.x - capture_rect.x;
        rect.right = rect.left + clip_rect.width;
        rect.top = clip_rect.y - capture_rect.y;
        rect.bottom = rect.top + clip_rect.height;
        visual->SetClip(rect);

        // calcurate the transform matrix then set it to visual
        auto target = viewport->get_composition_target();
        auto dcomp_device = target->get_device();
        CComPtr<IDCompositionTranslateTransform> transform_adjust_origin;
        dcomp_device->CreateTranslateTransform(&transform_adjust_origin);
        transform_adjust_origin->SetOffsetX(-(clip_rect.x - capture_rect.x));
        transform_adjust_origin->SetOffsetY(-(clip_rect.y - capture_rect.y));
        CComPtr<IDCompositionScaleTransform> transform_scale;
        dcomp_device->CreateScaleTransform(&transform_scale);
        transform_scale->SetScaleX(scale);
        transform_scale->SetScaleY(scale);
        transform_scale->SetCenterX(0.0f);
        transform_scale->SetCenterY(0.0f);
        CComPtr<IDCompositionTranslateTransform> transform_shift_to_target;
        dcomp_device->CreateTranslateTransform(&transform_shift_to_target);
        transform_shift_to_target->SetOffsetX(to_rect.x);
        transform_shift_to_target->SetOffsetY(to_rect.y);
        IDCompositionTransform* transforms[] = {
            transform_adjust_origin.operator->(),
            transform_scale.operator->(),
            transform_shift_to_target.operator->(),
        };
        CComPtr<IDCompositionTransform> transform_group;
        dcomp_device->CreateTransformGroup(transforms, std::extent<decltype(transforms)>::value, &transform_group);
        visual->SetTransform(transform_group);
    }

    IDCompositionVisual* get_visual() override{
        check_disposed();
        return visual;
    }

    void dispose() override{
        was_disposed = true;
    }

    const auto& get_clip_rect(){
        return clip_rect;
    }

protected:
    inline void check_disposed(){
        if (was_disposed){
            throw MapperException(
                "The CapturedImage object has been invalidated because mapper.reset_viewport() was called.");
        }
    }
};

//============================================================================================
// Window image streamer
//============================================================================================
class image_streamer_imp : public capture::image_streamer{
protected:
    ViewPortManager& manager;
    uint32_t id;
    std::string name{"DCS World"};
    std::vector<std::string> target_titles;
    bool was_disposed{false};
    HWND hwnd{nullptr};
    FloatRect capture_rect{0, 0, 0, 0};
    D3D11_BOX src_rect;
    std::atomic<bool> capture_item_is_valid{false};
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem capture_item{nullptr};
    winrt::Windows::Graphics::SizeInt32 capture_item_size;
    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice device{nullptr};
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool frame_pool{nullptr};
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession session{nullptr};
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::FrameArrived_revoker frame_arrived;
    CComPtr<ID3D11Device> d3d_device;
    CComPtr<IDXGISwapChain1> swap_chain;
    CComPtr<ID3D11DeviceContext> d3d_context;

    std::vector<std::shared_ptr<captured_image_imp>> view_elements;

public:
    image_streamer_imp(ViewPortManager&manager, sol::variadic_args args, uint32_t id): manager(manager), id(id){
        if (args.size() > 0){
            auto def_obj = args[0];
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
        }
        if (target_titles.size() == 0){
            target_titles.emplace_back("Digital Combat Simulator");
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
                capture_item_size = capture_item.Size();
                capture_item.Closed({this, &image_streamer_imp::on_close_target});
                capture_item_is_valid.store(true);
            }catch (...){
                mapper_EngineInstance()->putLog(MCONSOLE_WARNING, "image_streamer: The specified window cannot be used for image capture.");
                capture_item_is_valid.store(true);
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

        d3d_device = composition::create_d3d_device();
        d3d_device->GetImmediateContext(&d3d_context);
        CComPtr<IDXGIDevice> dxgi_device;
        d3d_device.QueryInterface(&dxgi_device);
        winrt::com_ptr<::IInspectable> dev{nullptr};
        CreateDirect3D11DeviceFromDXGIDevice(dxgi_device, dev.put());
        device = dev.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();

        if (capture_item && capture_item_is_valid.load()){
            src_rect.left = std::floor(capture_rect.x);
            src_rect.right = std::ceil(capture_rect.x + capture_rect.width);
            src_rect.top = std::floor(capture_rect.y);
            src_rect.bottom = std::ceil(capture_rect.y + capture_rect.height);
            src_rect.front = 0;
            src_rect.back = 1;
            swap_chain = composition::create_swapchain(dxgi_device, std::ceil(capture_rect.width), std::ceil(capture_rect.height));
            frame_pool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::CreateFreeThreaded(
                device,
                winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
                2,
                capture_item_size);
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
        for (auto& element : view_elements){
            element->dispose();
        }
        was_disposed = true;
    }

    std::shared_ptr<capture::captured_image> create_view_element(sol::object arg_object) override{
        check_disposed();
        return lua_c_interface(*mapper_EngineInstance(), "WindowImageStreamer:view_element()", [arg_object, this]{
            auto element = std::make_shared<captured_image_imp>(arg_object, capture_rect, swap_chain);
            view_elements.push_back(element);
            capture_rect += element->get_clip_rect();
            return element;
        });
    }

protected:
    inline void check_disposed(){
        if (was_disposed){
            throw MapperException(
                "The WindowImageStreamer object has been invalidated because mapper.reset_viewport() was called.");
        }
    }

    void on_frame_arrived(
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender, 
        winrt::Windows::Foundation::IInspectable const&){
        auto frame = sender.TryGetNextFrame();
        auto frameSurface = get_dxgi_interface_from_object<ID3D11Texture2D>(frame.Surface());
        
        winrt::com_ptr<ID3D11Texture2D> backBuffer;
        swap_chain->GetBuffer(0, winrt::guid_of<ID3D11Texture2D>(), backBuffer.put_void());

        d3d_context->CopySubresourceRegion(backBuffer.get(), 0, 0, 0, 0, frameSurface.get(), 0, &src_rect);

        DXGI_PRESENT_PARAMETERS present_parameters{0};
        swap_chain->Present1(1, 0, &present_parameters);
    }

    void on_close_target(
        winrt::Windows::Graphics::Capture::GraphicsCaptureItem const& sender,
        winrt::Windows::Foundation::IInspectable const&){
        capture_item_is_valid.store(false);
        mapper_EngineInstance()->notifyUpdate(MapperEngine::UPDATED_LOST_CAPTURED_WINDOW);
    }
};

namespace capture{
    std::shared_ptr<image_streamer> create_image_streamer(ViewPortManager&manager, uint32_t id, sol::variadic_args args){
        return std::make_shared<image_streamer_imp>(manager, args, id);
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
