//
// dcs.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <filesystem>
#include <sol/sol.hpp>
#include <format>
#include <algorithm>
#include <stdlib.h>
#include "dcs.h"
#include "engine.h"
#include "tools.h"

static constexpr auto connecting_interval = 1000; // in milli second

//============================================================================================
// Exporter configuration loader
//============================================================================================
struct ExporterConfig{
    std::filesystem::path config_path;
    u_short tcp_port{8544};
    u_short udp_port{8544};

    ExporterConfig(){
        std::vector<char> buf;
        buf.resize(256);
        while(true){
            if (::GetModuleFileNameA(nullptr, &buf.at(0), static_cast<DWORD>(buf.size())) < buf.size()){
                break;
            }
            buf.resize(buf.size() + 256);
        }
        config_path = &buf.at(0);
        config_path.remove_filename();
        config_path = config_path / "dcs-exporter/fsmapper_config.lua";
    }

    void parse(){
        sol::state lua;
        lua.script("fsmapper={}");
        auto result = lua.safe_script_file(config_path.generic_string(), sol::script_pass_on_error);
        if (!result.valid()){
            sol::error err = result;
            throw MapperException(err.what());
        }

        sol::table fsmapper = lua["fsmapper"];
        sol::object config_obj = fsmapper["config"];
        if (config_obj.get_type() == sol::type::table){
            sol::table config = config_obj;
            auto tcp_port_value = lua_safevalue<u_short>(config["tcp_port"]);
            if (tcp_port_value){
                tcp_port = *tcp_port_value;
            }
            auto udp_port_value = lua_safevalue<u_short>(config["udp_port"]);
            if (udp_port_value){
                udp_port = *udp_port_value;
            }
        }
    }
};

//============================================================================================
// Client socket abstraction
//============================================================================================
class client_socket{
    SOCKET sock{INVALID_SOCKET};
    sockaddr_in server_addr;
    WSAEVENT event{WSA_INVALID_EVENT};

public:
    client_socket() = delete;
    client_socket(const client_socket&) = delete;
    client_socket(client_socket&&) = delete;
    client_socket(u_short port){
        event = ::WSACreateEvent();

        addrinfo hint;
        ZeroMemory(&hint, sizeof(hint));
        hint.ai_family =AF_INET;
        hint.ai_socktype = SOCK_DGRAM;
        hint.ai_protocol = IPPROTO_TCP;
        std::ostringstream os;
        os << port;
        addrinfo *addrs;
        if (getaddrinfo("localhost", os.str().c_str(), &hint, &addrs) == 0 && addrs[0].ai_family == AF_INET){
            server_addr = *reinterpret_cast<sockaddr_in*>(addrs[0].ai_addr);
        }else{
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.S_un.S_un_b.s_b1 = 127;
            server_addr.sin_addr.S_un.S_un_b.s_b2 = 0;
            server_addr.sin_addr.S_un.S_un_b.s_b3 = 0;
            server_addr.sin_addr.S_un.S_un_b.s_b4 = 1;
            server_addr.sin_port = htons(port);
        }

        open();
    }
    ~client_socket(){
        close();
        WSACloseEvent(event);
    }

    void open(){
        sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        u_long nonblocking = 1;
        ::ioctlsocket(sock, FIONBIO, &nonblocking);
        WSAEventSelect(sock, event, FD_CONNECT);
    }

    void close(){
        if (sock != INVALID_SOCKET){
            closesocket(sock);
            sock = INVALID_SOCKET;
        }
    }

    void reopen(){
        close();
        open();
    }

    WSAEVENT get_event()const {return event;}
    DWORD get_socket_error() const {
        DWORD error{0};
        int error_len = sizeof(error);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &error_len);
        return error;
    }

    int connect(){
        WSAEventSelect(sock, event, FD_CONNECT);
        return ::connect(sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
    }

    void event_select(long event_flags){
        WSAEventSelect(sock, event, event_flags);
    }

    DWORD get_network_event(){
        WSANETWORKEVENTS nevent;
        WSAEnumNetworkEvents(sock, event, &nevent);
        return nevent.lNetworkEvents;
    }

    int receive(void* buf, int len){
        return ::recv(sock, reinterpret_cast<char*>(buf), len, 0);
    }

    int send(const void* buf, int len){
        return ::send(sock, reinterpret_cast<const char*>(buf), len, 0);
    }
};

//============================================================================================
// Sned buffer
//============================================================================================
class DCSWorldSendBuffer{
    static constexpr auto block_size = 32 * 1024;
    static constexpr auto initial_num_of_blocks = 2;

    struct buff_block {
        std::vector<char> buff;
        size_t used{0};
        buff_block(){
            buff.resize(block_size);
        }
        operator char* () {
            return &buff.at(0);
        }
        size_t remain() {
            return buff.size() - used;
        }
    };

    using buff_blocks_type = std::list<std::shared_ptr<buff_block>>;
    using buff_block_ptr = std::shared_ptr<buff_block>;

    std::mutex mutex;
    bool is_enable{false};
    buff_blocks_type buff_blocks;
    buff_blocks_type::iterator current;
    buff_block_ptr writing{nullptr};
    size_t written{0};

public:
    DCSWorldSendBuffer(){
        for (auto i = 0; i < initial_num_of_blocks; i++){
            buff_blocks.emplace_back(std::make_shared<buff_block>());
        }
        current = buff_blocks.begin();
    }

    bool is_empty(){
        std::lock_guard lock{mutex};
        return !writing && buff_blocks.begin()->operator*().used == 0;
    }

    void reset(bool mode){
        std::lock_guard lock{mutex};
        is_enable = mode;
        if (is_enable){
            if (writing){
                buff_blocks.push_back(writing);
                writing = nullptr;
            }
            for (auto& block : buff_blocks){
                block->used = 0;
            }
            current = buff_blocks.begin();
            written = 0;
        }
    }

    struct data_block{
        char* buff{nullptr};
        size_t size{0};
    };
    data_block get_writable_data(){
        std::lock_guard lock(mutex);
        if (!writing){
            if (buff_blocks.begin()->operator*().used == 0){
                return {};
            }
            if (current == buff_blocks.begin()){
                current++;
            }
            writing = *buff_blocks.begin();
            written = 0;
            buff_blocks.pop_front();
        }
        return {&writing->buff.at(0) + written, writing->used - written};
    }

    void trim(size_t size){
        std::lock_guard lock(mutex);
        if (!writing || size > writing->used - written){
            abort();
        }
        written += size;
        if (written >= writing->used){
            written = 0;
            buff_blocks.push_back(writing);
            writing = nullptr;
        }
    }

    bool insert_data(const void* data, size_t size){
        std::lock_guard lock(mutex);
        if (!is_enable){
            return false;
        }
        while (size){
            if (current->operator*().remain() == 0){
                current++;
                if (current == buff_blocks.end()){
                    buff_blocks.emplace_back(std::make_shared<buff_block>());
                    current--;
                }
            }
            auto current_buf = current->operator->();
            auto write_size = std::min(size, current_buf->remain());
            memcpy(*current_buf + current_buf->used, data, write_size);
            current_buf->used += write_size;
            size -= write_size;
        }
        return true;
    }
};


//============================================================================================
// Communicator with DCS World exportor
//============================================================================================
static DCSWorld* the_manager {nullptr};
DCSWorld::DCSWorld(SimHostManager &manager, int id): SimHostManager::Simulator(manager, id){
    schedule_event = ::WSACreateEvent();
    tx_buf = std::make_unique<DCSWorldSendBuffer>();

    scheduler = std::thread([this]{
        ExporterConfig config;
        try{
            config.parse();
        }catch (MapperException& e){
            std::ostringstream os2;
            os2 << "dcs: an error occurred while recgnizing the exporter configuration file:\n" << e.what();
            mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, os2.str());
            std::ostringstream os;
            os << "dcs: failed to recognize the exporter configuration file, the DCS World connectiviy will be limited: " << config.config_path.generic_string();
            mapper_EngineInstance()->putLog(MCONSOLE_WARNING, os.str());
            return;
        }

        std::unique_lock lock{mutex};
        client_socket client(config.tcp_port);
        std::string rx_command;
        WSAEVENT events[] {schedule_event, client.get_event()};
        auto event_num = 1;
        auto timeout = 0;
        auto on_close = [&]{
            status = STATUS::connecting;
            aircraft_name.clear();
            rx_command.clear();
            client.reopen();
            tx_buf->reset(false);
            lock.unlock();
            reportConnectivity(true, MAPPER_SIM_NONE, nullptr, nullptr);
            lock.lock();
        };

        while (true){
            if (status == STATUS::connecting){
                client.connect();
                event_num = 2;
                timeout = WSA_INFINITE;
            }else if (status == STATUS::retrying){
                event_num =1;
                timeout = connecting_interval;
            }else if (status == STATUS::connected){
                event_num = 2;
                timeout = WSA_INFINITE;
                client.event_select(FD_READ | FD_CLOSE | (tx_buf->is_empty() ? 0 : FD_WRITE));
            }
            lock.unlock();
            auto index = ::WSAWaitForMultipleEvents(event_num, events, false, timeout, false);
            lock.lock();
            if (should_stop){
                break;
            }
            if (index == WSA_WAIT_TIMEOUT){
                status = STATUS::connecting;
            }else if (index >= WSA_WAIT_EVENT_0 && index < WSA_WAIT_EVENT_0 + event_num){
                ::ResetEvent(events[index - WSA_WAIT_EVENT_0]);
                if (index == 0){
                    // process requests
                }else if (status == STATUS::connecting){
                    if (client.get_socket_error() == 0){
                        status = STATUS::connected;
                        tx_buf->reset(true);
                        mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, "dcs: connection with DCS World exporter has been established");
                        lock.unlock();
                        reportConnectivity(true, MAPPER_SIM_DCS, "dcs", nullptr);
                        lock.lock();
                    }else{
                        status = STATUS::retrying;
                    }
                }else if (status == STATUS::connected){
                    auto nevent = client.get_network_event();
                    if (nevent & FD_WRITE){
                        auto data = tx_buf->get_writable_data();
                        auto written = client.send(data.buff, data.size);
                        if (written == SOCKET_ERROR){
                            mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, "dcs: An error occurred while communicating with DCS World: send()");
                            on_close();
                        }
                        tx_buf->trim(written);
                    }
                    if (nevent & (FD_READ | FD_CLOSE)){
                        auto received = client.receive(rx_buf, sizeof(rx_buf));
                        if (received > 0){
                            process_received_data(lock, rx_buf, received, rx_command);
                        }else if (received == 0){
                            mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, "dcs: connection with DCS World exporter has been closed");
                            on_close();
                        }else{
                            mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, "dcs: An error occurred while communicating with DCS World: recv()");
                            on_close();
                        }
                    }
                }
            }
        }
    });
    
    the_manager = this;
}

DCSWorld::~DCSWorld(){
    {
        std::lock_guard lock{mutex};
        should_stop = true;
        ::SetEvent(schedule_event);
    }
    scheduler.join();
    WSACloseEvent(schedule_event);
    the_manager = nullptr;
}

//============================================================================================
// Processing received data from DCS exporter
//============================================================================================
void DCSWorld::process_received_data(std::unique_lock<std::mutex>& lock, const char *buf, int len, std::string& context){
    int from = 0;
    for (auto to = from; to < len; to++){
        if (buf[to] == '\n'){
            if (context.length()){
                context.append(buf + from, to - from);
                dispatch_received_command(lock, context.c_str(), context.length());
                context.clear();
            }else{
                dispatch_received_command(lock, buf + from, to - from);
            }
            from = to + 1;
        }
    }
    if (from < len){
        context.append(buf + from, len - from);
    }
}

void DCSWorld::dispatch_received_command(std::unique_lock<std::mutex> &lock, const char *cmd, int len){
    if (len <= 0){
        return;
    }

    if (*cmd == 'A'){
        // Aircraft Name event
        if (len > 1){
			aircraft_name.clear();
			aircraft_name.append(cmd + 1, len - 1);
			mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, std::format("dcs: Aircraft name has been received: {}", aircraft_name));
			lock.unlock();
			reportConnectivity(true, MAPPER_SIM_DCS, "dcs", aircraft_name.c_str());
			lock.lock();
        }
    }
    else if (*cmd == 'V'){
        // Version nortification
        std::string name, version, *target{&name};
        for (auto i = 1; i < len; i++){
            if (cmd[i] == ':'){
                target = &version;
            }else{
                target->push_back(cmd[i]);
            }
        }
        auto&& msg = std::format(
            "dcs: Product version information has been received:\n"
            "    Product Name    : {}\n"
            "    Product Version : {}",
            name, version);
        mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, msg);
    }
}


//============================================================================================
// Create Lua scripting environment
//============================================================================================
void DCSWorld::initLuaEnv(sol::state &lua){
}
