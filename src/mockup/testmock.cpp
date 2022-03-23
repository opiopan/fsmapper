#include <iostream>
#include <mutex>
#include <functional>
#include <thread>
#include <map>
#include <unordered_map>
#include <chrono>
#include <windows.h>
#include "mappercore.h"

static constexpr auto find_interval = std::chrono::milliseconds(1000);
//static const std::string capture_target = "WIN64APP";
static const std::string capture_target = "AceApp";

class MapperInterface{
protected:
    std::mutex mutex;
    std::condition_variable cv;
    std::thread controller;
    bool should_stop = false;
    bool cw_changed = false;
    bool finding = false;
    MapperHandle mapper = nullptr;

    struct CWInfo{
        uint32_t cwid;
        std::string name;
    };
    std::map<uint32_t, CWInfo> cwinfo_list;

public:
    MapperInterface(){
        mapper = mapper_init(&MapperInterface::event_handler, &MapperInterface::console_handler, this);
        controller = std::move(std::thread([this](){
            while (true){
                std::unique_lock lock(mutex);
                auto condition = [this](){return should_stop || cw_changed;};
                if (finding){
                    cv.wait_for(lock, find_interval, condition);
                }else{
                    cv.wait(lock, condition);
                }
                if (should_stop){
                    return;
                }
                if (cw_changed){
                    cw_changed = false;
                    finding = true;
                    cwinfo_list.clear();
                    lock.unlock();
                    mapper_stopViewPort(mapper);
                    mapper_enumCapturedWindows(mapper, &MapperInterface::enum_cw_callback, nullptr);
                    lock.lock();
                }
                if (finding){
                    lock.unlock();
                    std::map<LONG, HWND> targets;
                    ::EnumWindows(&MapperInterface::enum_windows_proc, reinterpret_cast<LPARAM>(&targets));
                    if (targets.size() >= cwinfo_list.size()){
                        std::vector<HWND> hwnd_list;
                        for (auto item : targets){
                            hwnd_list.push_back(item.second);
                        }
                        auto idx = 0;
                        for (auto item : cwinfo_list){
                            mapper_captureWindow(mapper, item.second.cwid, hwnd_list[idx]);
                            idx++;
                        }
                        mapper_startViewPort(mapper);
                        lock.lock();
                        finding = false;
                    }else{
                        lock.lock();
                    }

                }
            }
        }));
    }

    ~MapperInterface(){
        {
            std::lock_guard lock(mutex);
            should_stop = true;
            cv.notify_all();
        }
        controller.join();
        mapper_terminate(mapper);
    }

    bool run(const char* script_path){
        mapper_setLogMode(mapper, MAPPER_LOG_EVENT);
        //mapper_setLogMode(mapper, 0);
        return mapper_run(mapper, script_path);
    }

protected:
    static bool event_handler(MapperHandle mapper, MAPPER_EVENT ev, int64_t data){
        auto self = reinterpret_cast<MapperInterface*>(mapper_getHostContext(mapper));
        return self->process_event(ev, data);
    }

    bool process_event(MAPPER_EVENT ev, int64_t data){
        static const std::unordered_map<MAPPER_EVENT, const char*> evnames = {
            {MEV_START_MAPPING, "MEV_START_MAPPTING"},
            {MEV_STOP_MAPPING, "MEV_STOP_MAPPTING"},
            {MEV_CHANGE_SIMCONNECTION, "MEV_CHANGE_SIMCONNECTION"},
            {MEV_CHANGE_AIRCRAFT, "MEV_CHANGE_AIRCRAFT"},
            {MEV_CHANGE_DEVICES, "MEV_CHANGE_DEVICES"},
            {MEV_CHANGE_MAPPINGS, "MEV_CHANGE_MAPPINGS"},
            {MEV_CHANGE_VJOY, "MEV_CHANGE_VJOY"},
            {MEV_READY_TO_CAPTURE_WINDOW, "MEV_READY_TO_CAPTURE"},
            {MEV_LOST_CAPTURED_WINDOW, "MEV_LOCST_CAPTURED_WINDOW"},
            {MEV_CHANGE_VIEWPORTS, "MEV_CHANGE_VIEWPORTS"},
            {MEV_START_VIEWPORTS, "MEV_START_VIEWPORTS"},
            {MEV_STOP_VIEWPORTS, "MEV_STOP_VIEWPORTS"},
            {MEV_RESET_VIEWPORTS, "MEV_RESET_VIEWPORTS"},
        };

        std::lock_guard lock(mutex);
        std::cout << "=== " << evnames.at(ev) << " ===" << std::endl;

        if (ev == MEV_READY_TO_CAPTURE_WINDOW || ev == MEV_LOST_CAPTURED_WINDOW){
            cw_changed = true;
            cv.notify_all();
        }

        return true;
    }

    static bool console_handler(MapperHandle mapper, MCONSOLE_MESSAGE_TYPE type, const char *msg, size_t len){
        auto self = reinterpret_cast<MapperInterface*>(mapper_getHostContext(mapper));
        return self->process_console_message(type, msg, len);
    }

    bool process_console_message(MCONSOLE_MESSAGE_TYPE type, const char *msg, size_t len){
        std::lock_guard lock(mutex);
        std::cout.write(msg, len);
        std::cout << std::endl;
        return true;
    }

    static bool enum_cw_callback(MapperHandle mapper, void* context, CAPTURED_WINDOW_DEF* cwdef){
        auto self = reinterpret_cast<MapperInterface*>(mapper_getHostContext(mapper));
        return self->enum_cw(cwdef);
    }

    bool enum_cw(CAPTURED_WINDOW_DEF* cwdef){
        std::lock_guard lock(mutex);
        CWInfo info = {cwdef->cwid, cwdef->name};
        cwinfo_list[cwdef->cwid] = std::move(info);
        return true;
    }

    static BOOL CALLBACK enum_windows_proc(HWND hwnd, LPARAM lParam){
        auto& targets = *reinterpret_cast<std::map<LONG, HWND>*>(lParam);
        char name[256];
        name[0] = 0;
        ::GetClassNameA(hwnd, name, sizeof(name));
        if (capture_target == name){
            auto rc = ::GetWindowTextA(hwnd, name, sizeof(name));
            if (rc == 0){
                RECT rect;
                ::GetWindowRect(hwnd, &rect);
                targets[rect.left] = hwnd;
            }
        }
        return true;
    }
};

int main(int argc, char* argv[]){
    if (argc < 2){
        std::cerr << "usage: " << argv[0] << "script-path" << std::endl;
        return 1;
    }

    MapperInterface mapper;
    return mapper.run(argv[1]);
}
