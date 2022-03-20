//
// Models.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#pragma once
#include "Models.Device.g.h"
#include "Models.MappingsStat.g.h"
#include "Models.View.g.h"
#include "Models.Viewport.g.h"
#include "Models.CapturedWindow.g.h"
#include "Models.Mapper.g.h"

#include <mutex>
#include <condition_variable>
#include <thread>
#include "mappercore.h"

//============================================================================================
// Device
//============================================================================================
namespace winrt::gui::Models::implementation{
    struct Device : DeviceT<Device>{
        Device(hstring const& cname, hstring const& dname) : class_name(cname), device_name(dname){}
        hstring ClassName(){return class_name;}
        hstring DeviceName(){return device_name;}
    protected:
        hstring class_name;
        hstring device_name;
    };
}

namespace winrt::gui::Models::factory_implementation{
    struct Device : DeviceT<Device, implementation::Device>{
    };
}

//============================================================================================
// MappingStat
//============================================================================================
namespace winrt::gui::Models::implementation{
    struct MappingsStat : MappingsStatT<MappingsStat>{
        MappingsStat() = default;

        int32_t Primery(){return primery;}
        void Primery(int32_t value){primery = value;}
        int32_t Secondary(){return secondary;}
        void Secondary(int32_t value){secondary = value;}
        int32_t Viewports(){return viewports;}
        void Viewports(int32_t value){viewports = value;}
        int32_t Views(){return views;}
        void Views(int32_t value){views = value;}
        
    protected:
        int32_t primery{0};
        int32_t secondary{0};
        int32_t viewports{0};
        int32_t views{0};
    };
}
namespace winrt::gui::Models::factory_implementation{
    struct MappingsStat : MappingsStatT<MappingsStat, implementation::MappingsStat>{
    };
}

//============================================================================================
// View
//============================================================================================
namespace winrt::gui::Models::implementation{
    struct View : ViewT<View>{
        View(int32_t id, hstring const& name) : id(id), name(name){}

        int32_t Id(){return id;}
        hstring Name(){return name;}

    protected:
        int32_t id;
        hstring name;
    };
}
namespace winrt::gui::Models::factory_implementation{
    struct View : ViewT<View, implementation::View>{
    };
}

//============================================================================================
// Viewport
//============================================================================================
namespace winrt::gui::Models::implementation{
    struct Viewport : ViewportT<Viewport>{
        using ViewList = winrt::Windows::Foundation::Collections::IVector<winrt::gui::Models::View>;
        Viewport(hstring const& name, winrt::Windows::Foundation::IInspectable const& views) :
            name(name), views(views.as<ViewList>()){}

        hstring Name(){return name;}
        ViewList Views(){return views;}

    protected:
        hstring name;
        ViewList views;
    };
}
namespace winrt::gui::Models::factory_implementation{
    struct Viewport : ViewportT<Viewport, implementation::Viewport>
    {
    };
}

//============================================================================================
// CapturedWindow
//============================================================================================
namespace winrt::gui::Models::implementation
{
    struct CapturedWindow : CapturedWindowT<CapturedWindow>{
        CapturedWindow() = delete;
        CapturedWindow(winrt::Windows::Foundation::IInspectable const& mapper,
                       uint32_t cwid, hstring const& name, hstring const& description) : 
            mapper(winrt::make_weak(mapper.as<winrt::gui::Models::Mapper>())),
            cwid(cwid), name(name), description(description),
            image(mapper.as<winrt::gui::Models::Mapper>().NullWindowImage()){
            update_status_string();
        }

        uint32_t Cwid(){return cwid;}
        hstring Name(){return name;}
        hstring Description(){return description;}
        hstring StatusString(){return status_string;}
        bool IsCaptured(){return is_captured;}
        winrt::Microsoft::UI::Xaml::Media::ImageSource Image(){return image;}

        winrt::Windows::Foundation::IAsyncAction ToggleCapture(
            winrt::Windows::Foundation::IInspectable sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs args);

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler){
            return property_changed.add(handler);
        }
        void PropertyChanged(winrt::event_token const& token) noexcept{
            return property_changed.remove(token);
        }

    protected:
        winrt::weak_ref<winrt::gui::Models::Mapper> mapper;
        uint32_t cwid;
        hstring name;
        hstring description;
        hstring status_string;
        bool is_captured {false};
        winrt::Microsoft::UI::Xaml::Media::ImageSource image;
        winrt::Microsoft::UI::WindowId window_id;

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> property_changed;

        template <typename T>
        void update_property(T& variable, const T& value, const wchar_t* name){
            if (variable != value){
                variable = value;
                property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{name});
            }
        }

        template <typename T>
        void update_property(T& variable, T&& value, const wchar_t* name){
            if (variable != value){
                variable = std::move(value);
                property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{name});
            }
        }

        void update_property(const wchar_t* nm){
            property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{nm});
        }

        void update_status_string();
    };
}
namespace winrt::gui::Models::factory_implementation{
    struct CapturedWindow : CapturedWindowT<CapturedWindow, implementation::CapturedWindow>{
    };
}

//============================================================================================
// Mapper
//============================================================================================
namespace winrt::gui::Models::implementation{
    struct Mapper : MapperT<Mapper>{
        using DeviceCollection = winrt::Windows::Foundation::Collections::IVector<winrt::gui::Models::Device>;
        using ViewCollection = winrt::Windows::Foundation::Collections::IVector<winrt::gui::Models::View>;
        using ViewportCollection = winrt::Windows::Foundation::Collections::IVector<winrt::gui::Models::Viewport>;
        using CapturedWindowCollection = winrt::Windows::Foundation::Collections::IObservableVector<winrt::gui::Models::CapturedWindow>;

        Mapper();
        virtual ~Mapper();

        hstring ScriptPath();
        void ScriptPath(hstring const& value);
        winrt::gui::Models::MapperStatus Status();
        winrt::gui::Models::Simulators ActiveSim();
        hstring AircraftName();
        winrt::gui::Models::ViewportStatus ViewportMode();
        ViewportCollection Viewports();
        CapturedWindowCollection CapturedWindows();
        DeviceCollection Devices();
        winrt::gui::Models::MappingsStat MappingsInfo();

        winrt::Microsoft::UI::Xaml::Media::ImageSource NullWindowImage();

        void RunScript();
        void StopScript();
        void CaptureWindow(uint32_t Cwid, uint64_t hWnd);
        void ReleaseWindow(uint32_t Cwid);
        void StartViewports();
        void StopViewports();

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

    protected:
        winrt::apartment_context ui_thread;
        std::mutex mutex;
        std::condition_variable cv;
        bool should_stop {false};
        MapperHandle mapper {nullptr};
        MapperToolsHandle mapper_tools {nullptr};
        uint32_t dirty_properties {0};

        Windows::Foundation::IAsyncOperation<int32_t> scheduler;
        std::thread script_runner;
        std::thread mapper_observer;

        winrt::hstring script_path;
        gui::Models::MapperStatus status {gui::Models::MapperStatus::stop};
        gui::Models::Simulators active_sim {gui::Models::Simulators::none};
        winrt::hstring aircraft_name;
        winrt::gui::Models::ViewportStatus viewport_mode{winrt::gui::Models::ViewportStatus::none};
        ViewportCollection viewports {nullptr};
        CapturedWindowCollection captured_windows{nullptr};
        DeviceCollection devices {nullptr};
        winrt::gui::Models::MappingsStat mappings_info{nullptr};

		winrt::Microsoft::UI::Xaml::Media::ImageSource null_window_image{ nullptr };

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> property_changed;

        template <typename LOCK, typename T>
        void update_property(LOCK& lock, T& variable, const T& value, const wchar_t* name){
            if (variable != value){
                variable = value;
                lock.unlock();
                property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{name});
                lock.lock();
            }
        }

        template <typename LOCK, typename T>
        void update_property(LOCK& lock, T& variable, T&& value, const wchar_t* name){
            if (variable != value){
                variable = std::move(value);
                lock.unlock();
                property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{name});
                lock.lock();
            }
        }

        static bool event_callback(MapperHandle mapper, MAPPER_EVENT event, int64_t data);
        bool proc_event(MAPPER_EVENT event, int64_t data);
        static bool message_callback(MapperHandle mapper, MCONSOLE_MESSAGE_TYPE type, const char*msg, size_t len);
        bool proc_message(MCONSOLE_MESSAGE_TYPE type, const char*msg, size_t len);
        static bool enum_device_callback(MapperHandle mapper, void* context, const char* devtype, const char* devname);
        static bool enum_viewport_callback(MapperHandle mapper, void* context, VIEWPORT_DEF* vpdef);
        static bool enum_captured_window_callback(MapperHandle mapper, void* context, CAPTURED_WINDOW_DEF* cwdef);

        Windows::Foundation::IAsyncOperation<int32_t> scheduler_proc();
    };
}

namespace winrt::gui::Models::factory_implementation{
    struct Mapper : MapperT<Mapper, implementation::Mapper>{
    };
}
