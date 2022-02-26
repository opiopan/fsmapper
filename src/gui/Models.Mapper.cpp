//
// Models.Mapper.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#include "pch.h"
#include "Models.Mapper.h"
#include "Models.Mapper.g.cpp"

#include "config.hpp"
#include "encoding.hpp"

using namespace winrt::gui::Models;

static constexpr auto property_script_path   = 1 << 0;
static constexpr auto property_status        = 1 << 1;
static constexpr auto property_active_sim    = 1 << 2;
static constexpr auto property_aircraft_name = 1 << 3;

static const wchar_t* property_names[] = {
    L"ScriptPath",
    L"Status",
    L"ActiveSim",
    L"AircraftName",
    nullptr
};

namespace winrt::gui::Models::implementation{
    //============================================================================================
    // Object constructor / destructor
    //============================================================================================
    Mapper::Mapper(){
        script_path = fsmapper::app_config.get_script_path().c_str();

        scheduler = scheduler_proc();
        script_runner = std::move(std::thread([this]{
            std::unique_lock lock(mutex);
            while (true){
                cv.wait(lock, [this]{return status == MapperStatus::running || should_stop;});
                if (should_stop){
                    break;
                }
                tools::utf16_to_utf8_translator path(script_path.c_str());
                lock.unlock();
                auto result = mapper_run(mapper, path);
                lock.lock();
                mapper_terminate(mapper);
                mapper = nullptr;
                status = result ? MapperStatus::stop : MapperStatus::error;
                dirty_properties |= property_status;
                cv.notify_all();
            }
        }));
    }

    Mapper::~Mapper(){
        StopScript();
        std::unique_lock lock(mutex);
        should_stop = true;
        cv.notify_all();
        lock.unlock();
        script_runner.join();
    }

    //============================================================================================
    // Asynchronous event scheduling
    //============================================================================================
    Windows::Foundation::IAsyncOperation<int32_t> Mapper::scheduler_proc(){
        co_await winrt::resume_background();
        std::unique_lock lock{mutex};

        while (true){
            cv.wait(lock, [this]{return should_stop || dirty_properties;});
            if (should_stop){
                break;
            }
            if (dirty_properties){
                for (auto i = 0; property_names[i]; i++){
                    if (dirty_properties & (1 << i)){
                        lock.unlock();
                        co_await ui_thread;
                         property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{property_names[i]});
                        co_await winrt::resume_background();
                        lock.lock();
                    }
                }
                dirty_properties = 0;
            }
        }

        co_return 0;
    }

    //============================================================================================
    // Properties of runtime class
    //============================================================================================
    hstring Mapper::ScriptPath(){
        std::lock_guard lock{mutex};
        return script_path;
    }

    void Mapper::ScriptPath(hstring const& value){
        std::unique_lock lock{ mutex };
            if (status != MapperStatus::running){
            std::filesystem::path path(value.c_str());
            fsmapper::app_config.set_script_path(std::move(path));
            update_property(lock, script_path, value, L"ScriptPath");
            update_property(lock, status, MapperStatus::stop, L"Status");
        }
    }

    winrt::gui::Models::MapperStatus Mapper::Status(){
        std::lock_guard lock{mutex};
        return status;
    }

    winrt::gui::Models::Simulators Mapper::ActiveSim(){
        std::lock_guard lock{mutex};
        return active_sim;
    }

    hstring Mapper::AircraftName(){
        std::lock_guard lock{mutex};
        return aircraft_name;
    }

    //============================================================================================
    // Funcions exported as runtime class
    //============================================================================================
    void Mapper::RunScript(){
        std::lock_guard lock{mutex};
        if (status != MapperStatus::running){
            mapper = mapper_init(event_callback, message_callback, this);
            status = MapperStatus::running;
            dirty_properties |= property_status;
            cv.notify_all();
        }
    }

    void Mapper::StopScript(){
        std::lock_guard lock{mutex};
        if (status == MapperStatus::running){
            mapper_stop(mapper);
        }
    }

    //============================================================================================
    // Event notification
    //============================================================================================
    winrt::event_token Mapper::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler){
        return property_changed.add(handler);
    }

    void Mapper::PropertyChanged(winrt::event_token const& token) noexcept{
        return property_changed.remove(token);
    }

    //============================================================================================
    // mapper core callbaack functions
    //============================================================================================
    bool Mapper::event_callback(MapperHandle mapper, MAPPER_EVENT event, int64_t data){
        auto self = reinterpret_cast<Mapper*>(mapper_getHostContext(mapper));
        return self->proc_event(event, data);
    }

    bool Mapper::proc_event(MAPPER_EVENT event, int64_t data){
        std::unique_lock lock(mutex);
        if (event == MEV_CHANGE_SIMCONNECTION){
            active_sim = static_cast<gui::Models::Simulators>(data);
            dirty_properties |= property_active_sim;
            cv.notify_all();
        }else if (event == MEV_CHANGE_AIRCRAFT){
            tools::utf8_to_utf16_translator name(reinterpret_cast<const char*>(data));
            aircraft_name = name;
            dirty_properties |= property_aircraft_name;
            cv.notify_all();
        }
        return true;
    }

    bool Mapper::message_callback(MapperHandle mapper, MCONSOLE_MESSAGE_TYPE type, const char*msg, size_t len){
        auto self = reinterpret_cast<Mapper*>(mapper_getHostContext(mapper));
        return self->proc_message(type, msg, len);
    }

    bool Mapper::proc_message(MCONSOLE_MESSAGE_TYPE type, const char*msg, size_t len){
        return true;
    }

    bool Mapper::enum_device_callback(MapperHandle mapper, void* context, const char* devtype, const char* devname){
        return true;
    }

    bool Mapper::enum_captured_window_callback(MapperHandle mapper, void* context, CAPTURED_WINDOW_DEF* cwdef){
        return true;
    }
}
