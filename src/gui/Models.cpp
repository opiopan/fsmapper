//
// Models.Mapper.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#include "pch.h"
#include "Models.h"
#include "Models.Mapper.g.cpp"
#include "Models.Device.g.cpp"
#include "Models.MappingsStat.g.cpp"
#include "Models.View.g.cpp"
#include "Models.Viewport.g.cpp"
#include "Models.CapturedWindow.g.cpp"

#include "config.hpp"
#include "encoding.hpp"

#include <memory>
#include <vector>

using namespace winrt::gui::Models;

static constexpr auto property_script_path   = 1 << 0;
static constexpr auto property_status        = 1 << 1;
static constexpr auto property_active_sim    = 1 << 2;
static constexpr auto property_aircraft_name = 1 << 3;
static constexpr auto property_mappings_info = 1 << 4;
static constexpr auto property_devices       = 1 << 5;
static constexpr auto property_viewport_mode = 1 << 6;
static constexpr auto property_viewports     = 1 << 7;

static const wchar_t* property_names[] = {
    L"ScriptPath",
    L"Status",
    L"ActiveSim",
    L"AircraftName",
    L"MappingsInfo",
    L"Devices",
    L"ViewportMode",
    L"Viewports",
    nullptr
};


namespace winrt::gui::Models::implementation{
    using enum_device_context = std::vector<std::pair<std::string, std::string>>;
    struct enum_viewport_context {
        Mapper::ViewportCollection viewports;
        hstring viewport_name;
        Mapper::ViewCollection views;
        tools::utf8_to_utf16_translator translator;
    };

    //============================================================================================
    // Object constructor / destructor
    //============================================================================================
    Mapper::Mapper(){
        script_path = fsmapper::app_config.get_script_path().c_str();
        mapper = mapper_init(event_callback, message_callback, this);
        viewports = winrt::single_threaded_vector<gui::Models::Viewport>();
        devices = winrt::single_threaded_vector<gui::Models::Device>();
        mappings_info = winrt::make<winrt::gui::Models::implementation::MappingsStat>();

        //-----------------------------------------------------------------------------------------
        // coroutine to issue property change events
        //-----------------------------------------------------------------------------------------
        scheduler = scheduler_proc();

        //-----------------------------------------------------------------------------------------
        // start scripting thread
        //-----------------------------------------------------------------------------------------
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
                mapper = mapper_init(event_callback, message_callback, this);
                status = result ? MapperStatus::stop : MapperStatus::error;
                active_sim = gui::Models::Simulators::none;
                aircraft_name = L"";
                dirty_properties |= property_status | property_active_sim | property_aircraft_name |
                                    property_mappings_info | property_viewports | property_devices;
                cv.notify_all();
            }
        }));
        if (fsmapper::app_config.get_is_starting_script_at_start_up()){
            RunScript();
        }
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
                if (dirty_properties & property_mappings_info){
                    auto&& stat = ::mapper_getMappingsStat(mapper);
                    mappings_info.Primery(stat.num_primery);
                    mappings_info.Secondary(stat.num_secondary);
                    mappings_info.Viewports(stat.num_for_viewports);
                    mappings_info.Views(stat.num_for_views);
                }

                if (dirty_properties & property_devices){
                    enum_device_context data;
                    mapper_enumDevices(mapper, enum_device_callback, &data);
                    devices.Clear();
                    tools::utf8_to_utf16_translator cname, dname;
                    for (auto& entry : data) {
                        cname = entry.first.c_str();
                        dname = entry.second.c_str();
                        auto device = winrt::make<gui::Models::implementation::Device>(hstring(cname), hstring(dname));
                        devices.Append(device);
                    }
                }

                if (dirty_properties & property_viewports){
                    viewports.Clear();
                    enum_viewport_context ctx;
                    ctx.viewports = viewports;
                    ctx.views = nullptr;
                    mapper_enumViewport(mapper, enum_viewport_callback, &ctx);
                    if (ctx.views != nullptr && ctx.views.Size() > 0) {
                        auto viewport = winrt::make<gui::Models::implementation::Viewport>(ctx.viewport_name, ctx.views);
                        viewports.Append(viewport);
                    }
                }

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

    winrt::gui::Models::ViewportStatus Mapper::ViewportMode(){
        std::lock_guard lock{mutex};
        return viewport_mode;
    }

    Mapper::ViewportCollection Mapper::Viewports(){
        std::lock_guard lock{mutex};
        return viewports;
    }

    Mapper::DeviceCollection Mapper::Devices(){
        std::lock_guard lock{mutex};
        return devices;
    }

    winrt::gui::Models::MappingsStat Mapper::MappingsInfo(){
        std::lock_guard lock{mutex};
        return mappings_info;
    }

    winrt::Microsoft::UI::Xaml::Media::Imaging::SoftwareBitmapSource Mapper::NullWindowImage(){
        return null_window_image;
    }

    //============================================================================================
    // Funcions exported as runtime class
    //============================================================================================
    void Mapper::RunScript(){
        std::lock_guard lock{mutex};
        if (status != MapperStatus::running){
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
        }else if (event == MEV_CHANGE_DEVICES){
            dirty_properties |= property_devices;
            cv.notify_all();
        }else if (event == MEV_CHANGE_MAPPINGS){
            dirty_properties |= property_mappings_info;
            cv.notify_all();
        }else if (event == MEV_CHANGE_VIEWPORTS){
            dirty_properties |= property_viewports;
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
        auto list = reinterpret_cast<enum_device_context*>(context);
        list->emplace_back(devtype, devname);
        return true;
    }

    bool Mapper::enum_viewport_callback(MapperHandle mapper, void* context_addr, VIEWPORT_DEF* vpdef){
        auto context = reinterpret_cast<enum_viewport_context*>(context_addr);
        if (vpdef->viewid == 0) {
            if (!context->viewport_name.empty()) {
                auto viewport = winrt::make<gui::Models::implementation::Viewport>(context->viewport_name, context->views);
                context->viewports.Append(viewport);
            }
            context->translator.translate(vpdef->viewport_name);
            context->viewport_name = hstring(context->translator);
            context->views = winrt::single_threaded_vector<gui::Models::View>();
        }
        context->translator.translate(vpdef->view_name);
        auto view = winrt::make<gui::Models::implementation::View>(vpdef->viewid, hstring(context->translator));
        context->views.Append(view);
        return true;
    }

    bool Mapper::enum_captured_window_callback(MapperHandle mapper, void* context, CAPTURED_WINDOW_DEF* cwdef){
        return true;
    }
}
