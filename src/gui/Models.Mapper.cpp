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
#include "Models.Message.g.cpp"

#include "App.xaml.h"
#include "config.hpp"
#include "encoding.hpp"
#include "tools.hpp"
#include "version_parser.hpp"

#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>

#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Microsoft.Graphics.Canvas.UI.Xaml.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Data.Json.h>
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")

using namespace winrt;
using namespace Windows::Foundation;
using namespace winrt::gui::Models;
using namespace std::literals::chrono_literals;

static constexpr auto property_script_path = 1 << 0;
static constexpr auto property_status                  = 1 << 1;
static constexpr auto property_active_sim              = 1 << 2;
static constexpr auto property_aircraft_name           = 1 << 3;
static constexpr auto property_mappings_info           = 1 << 4;
static constexpr auto property_devices                 = 1 << 5;
static constexpr auto property_viewport_is_active      = 1 << 6;
static constexpr auto property_viewports               = 1 << 7;
static constexpr auto property_captured_windows        = 1 << 8;
static constexpr auto property_event_msg_enabled       = 1 << 9;
static constexpr auto property_debug_msg_enabled       = 1 << 10;
static constexpr auto property_captured_window_status  = 1 << 11;

static const wchar_t* property_names[] = {
    L"ScriptPath",
    L"Status",
    L"ActiveSim",
    L"AircraftName",
    L"MappingsInfo",
    L"Devices",
    L"ViewportIsActive",
    L"Viewports",
    L"CapturedWindows",
    L"EventMessageIsEnabled",
    L"DebugMessageIsEnabled",
    L"CapturedWindowStatus",
    nullptr
};

namespace winrt::gui::Models::implementation{
    using enum_device_context = std::vector<std::pair<std::string, std::string>>;
    struct captured_window_def{
        uint32_t cwid;
        std::string name;
        std::string description;
        std::string target_class;
        std::vector<std::string> target_titles;
        captured_window_def(uint32_t cwid, const char* name, const char* description, const char* target_class):
            cwid(cwid), name(name), description(description ? description : ""), target_class(target_class) {}
    };
    using enum_captured_windows_context = std::vector<captured_window_def>;
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
        captured_windows = winrt::single_threaded_observable_vector<gui::Models::CapturedWindow>();
        devices = winrt::single_threaded_vector<gui::Models::Device>();
        mappings_info = winrt::make<winrt::gui::Models::implementation::MappingsStat>();
        messages = winrt::single_threaded_observable_vector<gui::Models::Message>();

        auto device = winrt::Microsoft::Graphics::Canvas::CanvasDevice::GetSharedDevice();
        auto source = winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasImageSource(device, 40, 30, 96);
        auto ds = source.CreateDrawingSession(winrt::Microsoft::UI::Colors::DimGray());
        ds.Close();
        null_window_image = source;

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
                set_log_mode();
                if (fsmapper::app_config.get_pre_run_script_is_valid()){
                    mapper_set_option_string(mapper, MOPT_PRE_RUN_SCRIPT, fsmapper::app_config.get_pre_run_script());
                }
                mapper_set_option_integer(mapper, MOPT_RENDERING_METHOD, fsmapper::app_config.get_rendering_method());
                mapper_set_option_boolean(mapper, MOPT_ASYNC_MESSAGE_PUMPING, fsmapper::app_config.get_use_separated_ui_thread());
                mapper_set_option_boolean(mapper, MOPT_LOGMODE, fsmapper::app_config.get_developer_log());
                mapper_set_option_string(mapper, MOPT_APP_PLUGIN_FOLDER, fsmapper::app_config.get_default_plugin_folder().string().c_str());
                mapper_set_option_string(mapper, MOPT_USER_PLUGIN_FOLDER, fsmapper::app_config.get_default_plugin_folder2().string().c_str());
                if (!fsmapper::app_config.get_plugin_folder_is_default()){
                    mapper_set_option_string(mapper, MOPT_PLUGIN_FOLDER, fsmapper::app_config.get_custom_plugin_folder().string().c_str());
                }
                mapper_set_option_integer(mapper, MOPT_STDLIB, fsmapper::app_config.get_lua_standard_libraries());
                mapper_set_option_boolean(mapper, MOPT_DCS_EXPORTER, fsmapper::app_config.get_dcs_exporter_mode() == fsmapper::config::dcs_exporter_mode::on);
                TOUCH_CONFIG tc{};
                tc.delay_mouse_emulation = fsmapper::app_config.get_touch_delay_mouse_emulation();
                tc.start_delay = fsmapper::app_config.get_touch_cursor_delay();
                tc.down_delay = fsmapper::app_config.get_touch_down_delay();
                tc.up_delay = fsmapper::app_config.get_touch_up_delay();
                tc.drag_delay = fsmapper::app_config.get_touch_drag_start_delay();
                tc.double_tap_on_drag = false;
                tc.dead_zone_for_drag_start = fsmapper::app_config.get_touch_deadzone_for_drag();
                tc.pointer_jitter = fsmapper::app_config.get_touch_pointer_jitter();
                tc.move_trigger_distance = fsmapper::app_config.get_touch_move_trigger_distance();
                tc.minimum_interval = fsmapper::app_config.get_touch_minimum_interval();
                mapper_tools_SetTouchParameters(&tc);
                lock.unlock();
                auto result = mapper_run(mapper, path);
                lock.lock();
                mapper_terminate(mapper);
                mapper = mapper_init(event_callback, message_callback, this);
                status = result ? MapperStatus::stop : MapperStatus::error;
                active_sim = gui::Models::Simulators::none;
                aircraft_name = L"";
                dirty_properties |= property_status | property_active_sim | property_aircraft_name |
                                    property_mappings_info | property_viewports | property_devices |
                                    property_captured_windows;
                cv.notify_all();
            }
        }));
        auto& cli_script_path = fsmapper::app_config.get_cli_script_path();
        if ((cli_script_path || fsmapper::app_config.get_is_starting_script_at_start_up()) &&
            fsmapper::app_config.get_script_path().c_str()[0]){
            RunScript();
        }

        //-----------------------------------------------------------------------------------------
        // coroutine to read  message
        //-----------------------------------------------------------------------------------------
        message_reader = message_reader_proc();

        //-----------------------------------------------------------------------------------------
        // run coroutine to check new release in background
        //-----------------------------------------------------------------------------------------
        check_new_release_async();
    }

    Mapper::~Mapper(){
        StopScript();
        std::unique_lock lock(mutex);
        should_stop = true;
        cv.notify_all();
        lock.unlock();
        std::unique_lock msg_lock(message_mutex);
        message_should_stop = true;
        message_cv.notify_all();
        msg_lock.unlock();
        script_runner.join();
    }

    //============================================================================================
    // Auto window capturing
    //============================================================================================
    class AutoWindowCapturerImp : public AutoWindowCapturer{
    protected:
        struct cw_attr{
            uint32_t cwid;
            std::wstring target_class;

            cw_attr(uint32_t cwid, const wchar_t* target_class): cwid(cwid), target_class(target_class){}
            cw_attr(cw_attr&& src): cwid(src.cwid), target_class(std::move(src.target_class)){}
            cw_attr& operator = (cw_attr&& src){
                cwid = src.cwid;
                target_class = std::move(src.target_class);
            }
        };
        using cw_attr_map = std::unordered_multimap <std::wstring, cw_attr>;
        using cw_attr_map_entry = std::pair<std::wstring, cw_attr>;

        winrt::apartment_context ui_thread;
        gui::Models::Mapper mapper{nullptr};
        cw_attr_map cw_attrs;
        std::mutex mutex;
        std::condition_variable cv;
        Windows::Foundation::IAsyncOperation<int32_t> obserber;
        bool should_stop{false};
        bool is_completed{false};
        static constexpr auto initial_buff_size{256};
        static constexpr auto incremental_amount{256};
        std::vector<wchar_t> buff;

    public:
        AutoWindowCapturerImp(winrt::apartment_context ui_thread, gui::Models::Mapper mapper, cw_attr_map&& cw_attrs): 
            ui_thread(ui_thread), mapper(mapper), cw_attrs(std::move(cw_attrs)){
            buff.resize(initial_buff_size);
            obserber = observe_window();
        }
        AutoWindowCapturerImp() = delete;
        virtual ~AutoWindowCapturerImp(){
            std::unique_lock lock{mutex};
            should_stop = true;
            cv.notify_all();
            cv.wait(lock, [this] {return is_completed;});
        }

        static std::unique_ptr<AutoWindowCapturer> make_auto_window_capturer(winrt::apartment_context& ui_thread, gui::Models::Mapper mapper){
            cw_attr_map cw_attrs;
            for (auto cw : mapper.CapturedWindows()){
                for (auto title : cw.TargetTitles()){
                    cw_attrs.insert(cw_attr_map_entry{title.c_str(), cw_attr{cw.Cwid(), cw.TargetClass().c_str()}});
                }
            }
            if (cw_attrs.size() > 0){
                return std::make_unique<AutoWindowCapturerImp>(ui_thread, mapper, std::move(cw_attrs));
            }else{
                return nullptr;
            }
        }

    protected:
        Windows::Foundation::IAsyncOperation<int32_t> observe_window(){
            co_await winrt::resume_background();
            std::unique_lock lock{mutex};

            bool is_captured = true;

            while (true){
                cv.wait_for(lock, is_captured ? std::chrono::seconds(0) : std::chrono::milliseconds(1000), [this]{return should_stop;});
                if (should_stop){
                    break;
                }
                is_captured = false;

                //
                // find a window that match the criteria
                //
                struct CONTEXT {
                    AutoWindowCapturerImp* self;
                    HWND target = nullptr;
                    uint32_t cwid = 0;
                } ctx;
                ctx.self = this;
                ::EnumWindows([](HWND hwnd, LPARAM lParam)->BOOL{
                    auto ctx = reinterpret_cast<CONTEXT*>(lParam);
                    auto title = ctx->self->get_text<::GetWindowTextW>(hwnd);
                    auto range = ctx->self->cw_attrs.equal_range(title);
                    for (auto it = range.first; it != range.second; it++){
                        if (it->second.target_class.size() == 0 || 
                            it->second.target_class == ctx->self->get_text<::GetClassNameW>(hwnd)){
                            ctx->target = hwnd;
                            ctx->cwid = it->second.cwid;
                            return false;
                        }
                    }
                    return true;
                }, reinterpret_cast<LPARAM>(&ctx));

                if (ctx.target){
                    //
                    // capture the found window
                    //
                    lock.unlock();
                    co_await ui_thread;
                    mapper.CaptureWindowBypassingGUI(ctx.cwid, reinterpret_cast<int64_t>(ctx.target));
                    co_await winrt::resume_background();
                    lock.lock();
                    if (should_stop){
                        break;
                    }
                    is_captured = true;

                    //
                    // maintain target list
                    //
                    for (auto it = cw_attrs.begin(); it != cw_attrs.end();){
                        auto deletee = it;
                        it++;
                        if (deletee->second.cwid == ctx.cwid){
                            cw_attrs.erase(deletee);
                        }
                    }

                    //
                    // start viewports if there is no capture target
                    //
                    if (cw_attrs.size() == 0){
                        is_completed = true;
                        cv.notify_all();
                        auto uit = ui_thread;
                        lock.unlock();
                        co_await winrt::resume_after(200ms);
                        co_await uit;
                        mapper.StartViewportsIfReady();
                        co_await winrt::resume_background();
                        co_return 0;
                    }
                }
            }

            is_completed = true;
            cv.notify_all();
            co_return 0;
        }

        template <int APIFUNC(HWND, LPWSTR, int)>
        const wchar_t* get_text(HWND hwnd){
            while (true){
                auto rc = APIFUNC(hwnd, &buff.at(0), static_cast<int>(buff.size()));
                if (rc < buff.size() - 1){
                    return &buff.at(0);
                }else{
                    buff.resize(buff.max_size() + incremental_amount);
                }
            };
        };
    };

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
                auto mask = dirty_properties;
                if (mask & property_mappings_info){
                    auto&& stat = ::mapper_getMappingsStat(mapper);
                    mappings_info.Primery(stat.num_primery);
                    mappings_info.Secondary(stat.num_secondary);
                    mappings_info.Viewports(stat.num_for_viewports);
                    mappings_info.Views(stat.num_for_views);
                }

                if (mask & property_devices){
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

                if (mask & property_viewports){
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

                if (mask & property_captured_windows){
                    window_capturer = nullptr;
                    lock.unlock();
                    mapper_stopViewPort(mapper);
                    enum_captured_windows_context cw_list;
                    mapper_enumCapturedWindows(mapper, enum_captured_window_callback, &cw_list);
                    co_await ui_thread;
                    captured_windows.Clear();
                    tools::utf8_to_utf16_translator translator;
                    for (const auto& def : cw_list) {
                        translator = def.name.c_str();
                        auto name = hstring(translator);
                        translator = def.description.c_str();
                        auto description = hstring(translator);
                        translator = def.target_class.c_str();
                        auto target_class = hstring(translator);
                        auto target_titles = single_threaded_vector<hstring>();
                        for (auto& title : def.target_titles){
                            translator = title.c_str();
                            auto wtitle = hstring(translator);
                            target_titles.Append(wtitle);
                        }
                        auto cw = winrt::make<winrt::gui::Models::implementation::CapturedWindow>(
                           *this, def.cwid, name, description, target_class, target_titles);
                        captured_windows.Append(cw);
                    }
                    auto capturer = std::move(AutoWindowCapturerImp::make_auto_window_capturer(ui_thread, *this));
                    co_await winrt::resume_background();
                    lock.lock();
                    window_capturer = std::move(capturer);
                }

                for (auto i = 0; property_names[i]; i++){
                    if (mask & (1 << i)){
                        lock.unlock();
                        co_await ui_thread;
                         property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{property_names[i]});
                        co_await winrt::resume_background();
                        lock.lock();
                    }
                }
                dirty_properties &= ~mask;
            }
        }

        co_return 0;
    }

    //============================================================================================
    // Message reader
    //============================================================================================
    Windows::Foundation::IAsyncOperation<int32_t> Mapper::message_reader_proc(){
        co_await winrt::resume_background();
        std::unique_lock lock(message_mutex);
        while (true){
            message_cv.wait(lock, [this]{return message_buffer_is_dirty || should_stop;});
            if (should_stop){
                break;
            }
            auto& buffer = message_buffer[message_buffer_current];
            message_buffer_current ^= 1;
            message_buffer_is_dirty = false;
            lock.unlock();
            co_await ui_thread;

            for (auto& message : buffer){
                messages.Append(message);
            }
            while (messages.Size() > fsmapper::app_config.get_message_buffer_size()){
                messages.RemoveAt(0);
            }

            buffer.clear();

            co_await winrt::resume_background();
            lock.lock();
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

    bool Mapper::ViewportIsActive(){
        std::lock_guard lock{mutex};
        return viewport_is_active;
    }

    Mapper::ViewportCollection Mapper::Viewports(){
        std::lock_guard lock{mutex};
        return viewports;
    }

    Mapper::CapturedWindowCollection Mapper::CapturedWindows(){
        std::lock_guard lock{mutex};
        return captured_windows;
    }

    Mapper::DeviceCollection Mapper::Devices(){
        std::lock_guard lock{mutex};
        return devices;
    }

    winrt::gui::Models::MappingsStat Mapper::MappingsInfo(){
        std::lock_guard lock{mutex};
        return mappings_info;
    }

    winrt::Microsoft::UI::Xaml::Media::ImageSource Mapper::NullWindowImage(){
        std::lock_guard lock{mutex};
        return null_window_image;
    }

    Mapper::MessageCollection Mapper::Messages(){
        std::lock_guard lock{mutex};
        return messages;
    }

    bool Mapper::EventMessageIsEnabled(){
        std::lock_guard lock{mutex};
        return event_message_is_enabled;
    }
    void Mapper::EventMessageIsEnabled(bool value){
        std::lock_guard lock{mutex};
        event_message_is_enabled = value;
        dirty_properties |= property_event_msg_enabled;
        cv.notify_all();
        set_log_mode();
    }
    bool Mapper::DebugMessageIsEnabled(){
        std::lock_guard lock{mutex};
        return debug_message_is_enabled;
    }
    void Mapper::DebugMessageIsEnabled(bool value){
        std::lock_guard lock{mutex};
        debug_message_is_enabled = value;
        dirty_properties |= property_debug_msg_enabled;
        cv.notify_all();
        set_log_mode();
    }
    
    bool Mapper::IsAvailableNewRelease(){
        std::lock_guard lock{mutex};
        return is_available_new_release;
    }

    hstring Mapper::LatestRelease(){
        std::lock_guard lock{mutex};
        return latest_release;
    }

    bool Mapper::IsIgnoredUpdate(){
        std::lock_guard lock{mutex};
        return is_ignored_update;
    }
    void Mapper::IsIgnoredUpdate(bool value){
        std::unique_lock lock{mutex};
        update_property(lock, is_ignored_update, value, L"NewRelease");
        if (value){
            tools::utf16_to_utf8_translator utf8str;
            utf8str = latest_release.c_str();
            fsmapper::app_config.set_skipped_version(utf8str);
        }
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
        window_capturer = nullptr;
        if (status == MapperStatus::running){
            mapper_stop(mapper);
        }
    }

    void Mapper::StopScriptSync(){
        StopScript();
        std::unique_lock lock(mutex);
        cv.wait(lock, [this](){return status != MapperStatus::running;});
    }

    void Mapper::CaptureWindow(uint32_t Cwid, uint64_t hWnd){
        std::lock_guard lock{mutex};
        mapper_captureWindow(mapper, Cwid, reinterpret_cast<HWND>(hWnd));
        dirty_properties |= property_captured_window_status;
        cv.notify_all();
    }

    void Mapper::ReleaseWindow(uint32_t Cwid){
        std::lock_guard lock{mutex};
        mapper_releaseWindw(mapper, Cwid);
        dirty_properties |= property_captured_window_status;
        cv.notify_all();
    }

    bool Mapper::StartViewports(){
        std::lock_guard lock{mutex};
        window_capturer = nullptr;
        return mapper_startViewPort(mapper);
    }

    bool Mapper::StopViewports(){
        std::lock_guard lock{mutex};
        return mapper_stopViewPort(mapper);
    }

    //============================================================================================
    // Funcions for auto window capturer
    //============================================================================================
    void Mapper::CaptureWindowBypassingGUI(uint32_t Cwid, uint64_t hWnd){
        for (auto cw : captured_windows){
            if (cw.Cwid() == Cwid){
                cw.CaptureWindow(hWnd);
                break;
            }
        }
    }

    void Mapper::StartViewportsIfReady(){
        for (auto cw : captured_windows){
            if (!cw.IsCaptured()){
                return;
            }
        }
        StartViewports();
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
        }else if (event == MEV_READY_TO_CAPTURE_WINDOW || event == MEV_RESET_VIEWPORTS || event == MEV_LOST_CAPTURED_WINDOW){
            viewport_is_active = false;
            dirty_properties |= (property_captured_windows | property_viewports | property_viewport_is_active);
            cv.notify_all();
        }else if (event == MEV_STOP_VIEWPORTS){
            viewport_is_active = false;
            dirty_properties |= property_viewport_is_active;
            cv.notify_all();
        }else if (event == MEV_START_VIEWPORTS){
            viewport_is_active = true;
            dirty_properties |= property_viewport_is_active;
            cv.notify_all();
        }
        return true;
    }

    bool Mapper::message_callback(MapperHandle mapper, MCONSOLE_MESSAGE_TYPE type, const char*msg, size_t len){
        auto self = reinterpret_cast<Mapper*>(mapper_getHostContext(mapper));
        return self->proc_message(type, msg, len);
    }

    bool Mapper::proc_message(MCONSOLE_MESSAGE_TYPE type, const char* msg, size_t){
        std::lock_guard lock(message_mutex);
        auto message_type = static_cast<winrt::gui::Models::MessageType>(type);
        message_translator = msg;
        auto message = winrt::make<winrt::gui::Models::implementation::Message>(message_type, hstring(message_translator));
        message_buffer[message_buffer_current].push_back(message);
        message_buffer_is_dirty = true;
        message_cv.notify_all();
        return true;
    }

    bool Mapper::enum_device_callback(MapperHandle, void* context, const char* devtype, const char* devname){
        auto list = reinterpret_cast<enum_device_context*>(context);
        list->emplace_back(devtype, devname);
        return true;
    }

    bool Mapper::enum_viewport_callback(MapperHandle, void* context_addr, VIEWPORT_DEF* vpdef){
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
        auto list = reinterpret_cast<enum_captured_windows_context*>(context);
        list->emplace_back(cwdef->cwid, cwdef->name, cwdef->description, cwdef->target_class);
        mapper_enumCapturedWindowTitles(mapper, cwdef->cwid, [](MapperHandle, void* context, const char* title){
            auto def = reinterpret_cast<captured_window_def*>(context);
            def->target_titles.emplace_back(title);
            return true;
        }, &list->back());
        return true;
    }

    //============================================================================================
    // new release handling
    //============================================================================================
    winrt::Windows::Foundation::IAsyncAction Mapper::check_new_release_async(){
        co_await winrt::resume_background();
        Windows::Web::Http::HttpClient client;
        auto current_version = utils::this_version;
        while (true){
            try{
                Uri requestUri{L"https://opiopan.github.io/fsmapper/release.json"};
                auto result = co_await client.GetStringAsync(requestUri);
                auto json = Windows::Data::Json::JsonObject::Parse(result);
                auto latest_version_string = json.GetNamedString(L"version");
                auto latest_version_uri = json.GetNamedString(L"package");
                utils::parsed_version latest_version{latest_version_string.c_str()};
                if (latest_version > current_version){
                    current_version = latest_version;
                    co_await ui_thread;
                    std::unique_lock lock{mutex};
                    latest_release_uri = latest_version_uri;
                    latest_release = latest_version_string;
                    tools::utf8_to_utf16_translator utf16str;
                    utf16str = fsmapper::app_config.get_skipped_version();
                    utils::parsed_version skipped_version{utf16str};
                    if (current_version > skipped_version){
                        is_available_new_release = true;
                        is_ignored_update = false;
                        lock.unlock();
                        update_property(L"NewRelease");
                    }else{
                        current_version = skipped_version;
                        lock.unlock();
                    }
                    co_await winrt::resume_background();
                }
            }
            catch (winrt::hresult_error const &){}

            co_await winrt::resume_after(1h);
        }
    }


    void Mapper::DownloadLatestRelease(){
        ::ShellExecuteW(nullptr, L"open", latest_release_uri.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
    }
}
