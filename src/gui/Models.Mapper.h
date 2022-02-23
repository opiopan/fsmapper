//
// Models.Mapper.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#pragma once
#include "Models.Mapper.g.h"

#include <mutex>
#include <condition_variable>
#include <thread>
#include "mappercore.h"

namespace winrt::gui::Models::implementation{
    struct Mapper : MapperT<Mapper>{
        Mapper();
        virtual ~Mapper();

        hstring ScriptPath();
        void ScriptPath(hstring const& value);
        winrt::gui::Models::MapperStatus Status();
        winrt::gui::Models::Simulators ActiveSim();
        hstring AircraftName();

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

        winrt::hstring script_path;
        gui::Models::MapperStatus status {gui::Models::MapperStatus::stop};
        gui::Models::Simulators active_sim {gui::Models::Simulators::none};
        winrt::hstring aircraft_name;

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
        static bool message_callback(MapperHandle mapper, MCONSOLE_MESSAGE_TYPE type, const char*msg, size_t len);
        static bool enum_device_callback(MapperHandle mapper, void* context, const char* devtype, const char* devname);
        static bool enum_captured_window_callback(MapperHandle mapper, void* context, CAPTURED_WINDOW_DEF* cwdef);

        Windows::Foundation::IAsyncOperation<int32_t> scheduler_proc();
    };
}

namespace winrt::gui::Models::factory_implementation
{
    struct Mapper : MapperT<Mapper, implementation::Mapper>{
    };
}
