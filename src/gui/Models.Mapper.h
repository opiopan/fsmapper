//
// Models.Mapper.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#pragma once
#include "Models.Device.g.h"
#include "Models.MappingsStat.g.h"
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
namespace winrt::gui::Models::implementation
{
    struct MappingsStat : MappingsStatT<MappingsStat>
    {
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
namespace winrt::gui::Models::factory_implementation
{
    struct MappingsStat : MappingsStatT<MappingsStat, implementation::MappingsStat>
    {
    };
}

//============================================================================================
// Mapper
//============================================================================================
namespace winrt::gui::Models::implementation{
    struct Mapper : MapperT<Mapper>{
        using DeviceCollection = winrt::Windows::Foundation::Collections::IVector<winrt::gui::Models::Device>;

        Mapper();
        virtual ~Mapper();

        hstring ScriptPath();
        void ScriptPath(hstring const& value);
        winrt::gui::Models::MapperStatus Status();
        winrt::gui::Models::Simulators ActiveSim();
        hstring AircraftName();
        DeviceCollection Devices();
        winrt::gui::Models::MappingsStat MappingsInfo();

        void RunScript();
        void StopScript();

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

    protected:
        winrt::apartment_context ui_thread;
        std::mutex mutex;
        std::condition_variable cv;
        bool should_stop {false};
        MapperHandle mapper {nullptr};
        uint32_t dirty_properties {0};

        Windows::Foundation::IAsyncOperation<int32_t> scheduler;
        std::thread script_runner;
        std::thread mapper_observer;

        winrt::hstring script_path;
        gui::Models::MapperStatus status {gui::Models::MapperStatus::stop};
        gui::Models::Simulators active_sim {gui::Models::Simulators::none};
        winrt::hstring aircraft_name;
        DeviceCollection devices {nullptr};
        winrt::gui::Models::MappingsStat mappings_info{nullptr};

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
        static bool enum_captured_window_callback(MapperHandle mapper, void* context, CAPTURED_WINDOW_DEF* cwdef);

        Windows::Foundation::IAsyncOperation<int32_t> scheduler_proc();
    };
}

namespace winrt::gui::Models::factory_implementation{
    struct Mapper : MapperT<Mapper, implementation::Mapper>{
    };
}
