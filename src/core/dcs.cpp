//
// dcs.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
//  Commands:
//    fsmapper -> exorter:
//      P: perform clickable action
//      O: manipulate observer
//         subcommand:
//           A: create argument value observer: GetDevice():get_argument_value()
//           I: create indication text observer: list_indication()
//           C: create chunk observer
//           F: add numeric filter
//           G: add string filter
//           H: add chunk filter
//           E: enable observer
//      C: clear observed data
//
//    exporter -> fsmapper
//      V: notify DCS World version
//      A: change aircraft event
//      O: change observed data value event
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
// Transmit buffer
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
            writing->used = 0;
            buff_blocks.push_back(writing);
            writing = nullptr;
        }
    }

    bool insert_data(const void* data, size_t size){
        std::lock_guard lock(mutex);
        if (!is_enable){
            return false;
        }
        return insert_data_without_lock(data, size);
    }

    bool insert_data_without_lock(const void* data, size_t size){
        auto rc = !writing && buff_blocks.begin()->operator*().used == 0;
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
        return rc;
    }

    std::unique_lock<std::mutex> get_lock(){
        return std::unique_lock(mutex);
    }
};

using command_header = int32_t;
constexpr int32_t make_command_header(uint8_t command, int32_t length){
    return length << 8 | command;
}

//============================================================================================
// Received packet
//============================================================================================
class DCSPacket{
    char holding_buf[4];
    size_t holding_size{0};
    char cmd{'\0'};
    size_t data_length{0};
    std::vector<char> data_buf;
    size_t effective_length{0};
public:
    static constexpr auto command_length = 4;
    DCSPacket(){
        data_buf.resize(256);
    }
    
    char get_command()const{return cmd;}
    size_t get_data_length()const{return data_length;}
    size_t get_effective_length()const{return effective_length;}
    const char* get_data()const{return &data_buf.at(0);}
    operator const char* ()const{return get_data();}
    operator bool ()const{return holding_size == sizeof(holding_buf) && data_length == effective_length;}

    size_t append_data(const char* in, size_t length){
        auto skip_len{0};
        if (holding_size < sizeof(holding_buf)){
            skip_len = std::min(sizeof(holding_buf) - holding_size, length);
            memcpy(holding_buf + holding_size, in, skip_len);
            holding_size += skip_len;
            if (holding_size < sizeof(holding_buf)){
                return skip_len;
            }
            set_command();
            length -= skip_len;
        }
        length = std::min(length, data_length - effective_length);
        memcpy(&data_buf.at(0), in + skip_len, length);
        effective_length += length;
        return length + skip_len;
    }

    void clear(){
        holding_size = 0;
        cmd = '\0';
        data_length = 0;
        effective_length = 0;
    }
    
protected:
    void set_command(){
        cmd = holding_buf[0];
        union {int32_t i; char c[4];} length;
        length.i = 0;
        length.c[0] = holding_buf[1];
        length.c[1] = holding_buf[2];
        length.c[2] = holding_buf[3];
        data_length = length.i;
        effective_length = 0;
        if (data_length > data_buf.size()){
            data_buf.resize((data_length / 512 + 1) * 512);
        }
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
        DCSPacket rx_packet;
        WSAEVENT events[] {schedule_event, client.get_event()};
        auto event_num{1};
        auto timeout{0};
        auto write_is_blocked{false};
        auto on_close = [&]{
            status = STATUS::connecting;
            is_active = false;
            aircraft_name.clear();
            rx_packet.clear();
            client.reopen();
            tx_buf->reset(false);
            lock.unlock();
            reportConnectivity(false, MAPPER_SIM_NONE, nullptr, nullptr);
            lock.lock();
        };

        while (true){
            if (status == STATUS::connected){
                if (!tx_buf->is_empty() && !write_is_blocked){
                    auto data = tx_buf->get_writable_data();
                    client.event_select(FD_READ | FD_CLOSE | FD_WRITE);
                    auto written = client.send(data.buff, data.size);
                    if (written == SOCKET_ERROR){
                        if (WSAGetLastError() == EWOULDBLOCK){
                            write_is_blocked = true;
                        }else{
                            mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, "dcs: An error occurred while communicating with DCS World: send()");
                            on_close();
                        }
                    }else{
                        tx_buf->trim(written);
                        client.event_select(FD_READ | FD_CLOSE);
                    }
                    continue;
                }
                event_num = 2;
                timeout = WSA_INFINITE;
            }else if (status == STATUS::connecting){
                client.connect();
                event_num = 2;
                timeout = WSA_INFINITE;
            }else if (status == STATUS::retrying){
                event_num =1;
                timeout = connecting_interval;
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
                if (index == 0){
                    // process requests
                    ::ResetEvent(events[index - WSA_WAIT_EVENT_0]);
                }else if (status == STATUS::connected){
                    auto nevent = client.get_network_event();
                    if (nevent & FD_WRITE){
                        write_is_blocked = false;
                    }
                    if (nevent & (FD_READ | FD_CLOSE)){
                        auto received = client.receive(rx_buf, sizeof(rx_buf));
                        if (received > 0){
                            process_received_data(lock, rx_buf, received, rx_packet);
                        }else if (received == 0){
                            mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, "dcs: connection with DCS World exporter has been closed");
                            on_close();
                        }else{
                            mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, "dcs: An error occurred while communicating with DCS World: recv()");
                            on_close();
                        }
                    }
                }else if (status == STATUS::connecting){
                    if (client.get_socket_error() == 0){
                        status = STATUS::connected;
                        tx_buf->reset(true);
                        client.event_select(FD_READ | FD_CLOSE);
                        mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, "dcs: connection with DCS World exporter has been established");
                        lock.unlock();
                        reportConnectivity(true, MAPPER_SIM_DCS, "dcs", nullptr);
                        lock.lock();
                    }else{
                        status = STATUS::retrying;
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
size_t DCSWorld::process_received_data(std::unique_lock<std::mutex>& lock, char *buf, size_t len, DCSPacket& packet){
    int from = 0;
    while (from < len){
        from += packet.append_data(buf + from, len - from);
        if (packet){
            dispatch_received_command(lock, packet);
            packet.clear();
        }
    }
    return from;
}

void DCSWorld::dispatch_received_command(std::unique_lock<std::mutex> &lock, const DCSPacket& packet){
    auto cmd = packet.get_command();
    if (cmd == 'O'){
        O_command(lock, packet);
    }else if (cmd == 'A'){
        A_command(lock, packet);
    }else if (cmd == 'V'){
        V_command(lock, packet);
    }
}

void DCSWorld::V_command(std::unique_lock<std::mutex> &lock, const DCSPacket &packet){
        // Version nortification
        const struct DATA{
            uint32_t version[4];
            uint32_t produce_name_len;
            //char product_name[...];
        };
        auto data {reinterpret_cast<const DATA*>(packet.get_data())};
        std::string name{reinterpret_cast<const char*>(data + 1), data->produce_name_len};
        auto&& msg = std::format(
            "dcs: Product version information has been received:\n"
            "    Product Name    : {}\n"
            "    Product Version : {}.{}.{}.{}",
            name, data->version[0], data->version[1], data->version[2], data->version[3]);
        mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, msg);
}

void DCSWorld::A_command(std::unique_lock<std::mutex> &lock, const DCSPacket &packet){
        // Aircraft Name event
        if (packet.get_data_length()){
			aircraft_name.clear();
			aircraft_name.append(packet, packet.get_data_length());
			mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, std::format("dcs: Aircraft name has been received: {}", aircraft_name));
			lock.unlock();
			reportConnectivity(true, MAPPER_SIM_DCS, "dcs", aircraft_name.c_str());
			lock.lock();
        }
}

void DCSWorld::O_command(std::unique_lock<std::mutex> &lock, const DCSPacket &packet){
        // Observed value nortification
        auto data = packet.get_data();
        auto type = data[0];
        union {uint32_t i; char c[4];} index;
        index.i = 0;
        index.c[0] = data[1];
        index.c[1] = data[2];
        index.c[2] = data[3];
        if (index.i >= 0 && index.i < observed_data_defs.size()){
            triger_observed_data_event(index.i, type, data + 4, packet.get_data_length() - 4);
        }
}

//============================================================================================
// Handling observed data
//============================================================================================
class DCSObservedData{
    uint64_t event_id;
public:
    DCSObservedData(uint64_t event_id): event_id(event_id){}
    virtual ~DCSObservedData(){}
    uint64_t get_event_id() const{return event_id;}
    std::vector<float> numeric_filter;
    std::vector<std::string> string_filter;
    std::string chunk_filter;

    void set_filter(sol::object object){
        numeric_filter.clear();
        string_filter.clear();
        chunk_filter.clear();
        if (object.get_type() == sol::type::nil){
            return;
        }
        if (object.get_type() == sol::type::string){
            chunk_filter = object.as<const char*>();
        }else if (object.get_type() == sol::type::table){
            sol::table table = object;
            for (auto key_value : table){
                if (key_value.second.get_type() == sol::type::string){
                    string_filter.emplace_back(key_value.second.as<const char*>());
                }else if (key_value.second.get_type() == sol::type::number){
                    numeric_filter.push_back(key_value.second.as<float>());
                }else{
                    throw std::runtime_error("the values contains the table specified as 'filter' parameter must be either a number or a string");
                }
            }
        }else{
            throw std::runtime_error("the 'filter' parameter must be either a table or a string");
        }
    }

    virtual void get_register_cmd_text(std::string& buffer, uint32_t observed_data_id) = 0;

    void add_filter_sub_command(std::string& buffer, uint32_t observed_data_id){
        struct OF_CMD{
            command_header hdr;     // 'O', command length
            command_header sub_hdr; // 'F', oberver ID
            float value;
        }fcmd{
            make_command_header('O', sizeof(OF_CMD) - 4),
            make_command_header('F', observed_data_id),
            0,
        };
        for (auto value : numeric_filter){
            fcmd.value = value;
            buffer.append(reinterpret_cast<const char*>(&fcmd), sizeof(fcmd));
        }

        for (const auto& value : string_filter){
            struct OG_CMD{
                command_header hdr;     // 'O', command length
                command_header sub_hdr; // 'G', oberver ID
            }gcmd{
                make_command_header('O', sizeof(OG_CMD) - 4 + value.length() + 1),
                make_command_header('G', observed_data_id),
            };
            buffer.append(reinterpret_cast<const char*>(&gcmd), sizeof(gcmd));
            buffer.append(value.c_str(), value.length() + 1);
        }

        if (chunk_filter.length()){
            struct OG_CMD{
                command_header hdr;     // 'O', command length
                command_header sub_hdr; // 'H', oberver ID
            }gcmd{
                make_command_header('O', sizeof(OG_CMD) - 4 + chunk_filter.length() + 1),
                make_command_header('H', observed_data_id),
            };
            buffer.append(reinterpret_cast<const char*>(&gcmd), sizeof(gcmd));
            buffer.append(chunk_filter.c_str(), chunk_filter.length() + 1);
        }
    }
    void add_enable_sub_command(std::string& buffer, uint32_t observed_data_id){
        struct OE_CMD{
            command_header hdr;     // 'O', command length
            command_header sub_hdr; // 'E', oberver ID
        } cmd{
            make_command_header('O', sizeof(OE_CMD) - 4),
            make_command_header('E', observed_data_id),
        };
        buffer.append(reinterpret_cast<const char*>(&cmd), sizeof(cmd));
    }

    virtual void triger_event(int type, const char* value, size_t length){
        if (type == 'N'){
            if (length == sizeof(float)){
                mapper_EngineInstance()->sendEvent(std::move(Event(event_id, *reinterpret_cast<const float*>(value))));
            }
        }else if (type == 'S'){
            mapper_EngineInstance()->sendEvent(std::move(Event(event_id, std::string(value, length))));
        }else{
            auto evname = mapper_EngineInstance()->getEventName(event_id);
            auto&& msg = std::format("dcs: unsupported type of value has been received from DCS World as the observed data for a message '{}'", evname);
            mapper_EngineInstance()->putLog(MCONSOLE_DEBUG, msg);
        }
    }
};

class ObservedArgumentValue : public DCSObservedData{
    uint32_t arg_number;
    float epsilon;

public:
    ObservedArgumentValue(uint64_t event_id, uint32_t arg_number, float epsilon=0) : DCSObservedData(event_id), arg_number(arg_number), epsilon(epsilon){}

    void get_register_cmd_text(std::string& buffer, uint32_t observed_data_id) override{
        struct OA_CMD{
            command_header hdr;      // 'O', command length
            command_header sub_hdr;  // 'A', oberver ID
            uint32_t arg_number;
            float epsilon;
        } cmd{
            make_command_header('O', sizeof(cmd) - 4), 
            make_command_header('A', observed_data_id), 
            arg_number, epsilon,
        };

        buffer.clear();
        buffer.append(reinterpret_cast<char*>(&cmd), sizeof(cmd));
        add_filter_sub_command(buffer, observed_data_id);
        add_enable_sub_command(buffer, observed_data_id);
    }
};

void DCSWorld::sync_observed_data_definitions(std::unique_lock<std::mutex>& lock){
    bool need_to_set_event{false};
    std::string cmd;
    for (auto i = 0; i < observed_data_defs.size(); i++){
        observed_data_defs[i]->get_register_cmd_text(cmd, i);
        need_to_set_event = tx_buf->insert_data(cmd.c_str(), cmd.length()) || need_to_set_event;
    }
    if (need_to_set_event){
        ::SetEvent(schedule_event);
    }
}

void DCSWorld::triger_observed_data_event(size_t index, int type, const char* value, size_t length){
    observed_data_defs[index]->triger_event(type, value, length);
}


//============================================================================================
// Utilities for Lua functions
//============================================================================================
struct basic_args{
    uint32_t device_id{0};
    uint32_t command{0};
    uint32_t arg_nummber{0};
    float value{0};
};
enum class arg_type:int {device_id = 0, command, arg_number, value};

template <typename T, T basic_args::* Member>
bool apply_value(basic_args& result, sol::object object){
    auto value = lua_safevalue<T>(object);
    if (value){
        result.*Member = *value;
        return true;
    }else{
        return false;
    }
}

enum class args_kind{sequential, lableled};
args_kind parse_basic_args(sol::variadic_args args, const std::vector<arg_type>& rule, basic_args& result){
    struct arg_prop{
        const char* name;
        bool (*apply_value)(basic_args& result, sol::object);
    };
    static std::vector<arg_prop> arg_props {
        {"device_id", apply_value<uint32_t, &basic_args::device_id>},
        {"command", apply_value<uint32_t, &basic_args::command>},
        {"arg_number", apply_value<uint32_t, &basic_args::arg_nummber>},
        {"value", apply_value<float, &basic_args::value>},
    };

    sol::object arg0 = args[0];
    if (arg0.get_type() == sol::type::table){
        sol::table arg_table = arg0;
        for (const auto& type : rule){
            auto& prop = arg_props[static_cast<int>(type)];
            if (!prop.apply_value(result, arg_table[prop.name])){
                throw std::runtime_error(std::format("the parameter '{}' is not specified or value type is invalid", prop.name));
            }
        }
        return args_kind::lableled;
    }else{
        if (args.size() < rule.size()){
            throw std::runtime_error("not enough argument provided");
        }
        for (auto i = 0; i < rule.size(); i++){
            auto& prop = arg_props[static_cast<int>(rule[i])];
            if (!prop.apply_value(result, args[i])){
                static const char*numeral[] = {"1st", "2nd", "3rd", "4th", "5th"};
                throw std::runtime_error(std::format("the {} argument is invalid", numeral[i]));
            }
        }
        return args_kind::sequential;
    }
}

//============================================================================================
// Lua functions
//============================================================================================
struct P_CMD{
    command_header hdr;
    uint32_t device_id;
    uint32_t command;
};

void DCSWorld::lua_perform_clickable_action(sol::variadic_args args){
    static std::vector<arg_type> rule{arg_type::device_id, arg_type::command};
    basic_args result;
    if (parse_basic_args(args, rule, result) != args_kind::sequential){
        throw std::runtime_error("not enough argument provided");
    }
    auto cmd_len = sizeof(P_CMD) + sizeof(float)* (args.size() - 2) - 4;
    P_CMD cmd {make_command_header('P', cmd_len), result.device_id, result.command};
    auto need_to_set_event {false};
    {
        auto lock = std::move(tx_buf->get_lock());
        need_to_set_event = tx_buf->insert_data_without_lock(&cmd, sizeof(cmd));
        auto value_num = 0;
        for (auto i = 2; i < args.size(); i++){
            auto value = lua_safevalue<float>(args[i]);
            if (value){
                value_num++;
                tx_buf->insert_data_without_lock(&*value, sizeof(*value));
            }
        }
        if (value_num != args.size() - 2){
            throw std::runtime_error("the specified value for the clickable action is incorrect");
        }
    }
    if (need_to_set_event){
        ::SetEvent(schedule_event);
    }
}

std::shared_ptr<NativeAction::Function> DCSWorld::lua_clickable_action_performer(sol::variadic_args args){
    static std::vector<arg_type> rule{arg_type::device_id, arg_type::command};
    basic_args result;
    if (parse_basic_args(args, rule, result) != args_kind::sequential){
        throw std::runtime_error("not enough argument provided");
    }
    bool static_value = args.size() > 2;
    auto cmd_len = sizeof(P_CMD) - 4 + sizeof(float)* (static_value ? args.size() - 2 : 1);
    std::vector<char> cmd;
    cmd.resize(cmd_len + 4);
    *reinterpret_cast<P_CMD*>(&cmd[0]) = {make_command_header('P', cmd_len), result.device_id, result.command};
    std::ostringstream os;
    os << std::format("dcs.perform_clickable_action({}, {}", result.device_id, result.command);
    NativeAction::Function::ACTION_FUNCTION func;
    if (static_value){
        auto values = reinterpret_cast<float*>(reinterpret_cast<P_CMD*>(&cmd.at(0)) + 1);
        auto value_num = 0;
        for (auto i = 2; i < args.size(); i++){
            auto value = lua_safevalue<float>(args[i]);
            if (value){
                value_num++;
                values[i - 2] = *value;
                os << ", " << *value;
            }
        }
        if (value_num != args.size() - 2){
            throw std::runtime_error("the specified value for the clickable action is incorrect");
        }
        os << ")";
        func = [this, cmd = std::move(cmd)](auto& event, auto&){
            if (tx_buf->insert_data(&cmd[0], cmd.size())){
                ::SetEvent(schedule_event);
            }
        };
    }else{
        os << ", EVENT_VALUE)";
        func = [this, cmd = std::move(cmd)](auto &event, auto &){
            auto buf = const_cast<P_CMD*>(reinterpret_cast<const P_CMD*>(&cmd[0]) + 1);
            *reinterpret_cast<float *>(buf) = static_cast<float>(static_cast<double>(event));
            if (tx_buf->insert_data(&cmd[0], cmd.size())){
                ::SetEvent(schedule_event);
            }
        };
    }

    return std::make_shared<NativeAction::Function>(os.str().c_str(), func);
}

static std::string translate_to_numeral(int i){
    static std::vector<std::string> numeral = {"1st", "2nd", "3rd"};
    if (i < numeral.size()){
        return numeral[i];
    }else{
        return std::format("{}th", i + 1);
    }
}

void DCSWorld::lua_add_observed_data(sol::object arg0){
    if (arg0.get_type() != sol::type::table){
        throw std::runtime_error("invalid argument, the 1st argument must be an array of the observed data definition");
    }
    sol::table array = arg0;
    std::lock_guard lock{mutex};
    auto need_to_set_event{false};
    for (auto i = 1; i <= array.size(); i++){
        if (array[i].get_type() != sol::type::table){
            throw std::runtime_error(std::format("the {} element of the array is not a table", translate_to_numeral(i)));
        }
        sol::table def = array[i];
        auto event_id = lua_safevalue<uint64_t>(def["event"]);
        auto arg_number = lua_safevalue<int64_t>(def["arg_number"]);
        auto epsilon = lua_safevalue<double>(def["epsilon"]);

        if (arg_number){
            auto observed_data_def = std::make_unique<ObservedArgumentValue>(*event_id, *arg_number, epsilon ? *epsilon : 0);
            observed_data_def->set_filter(def["filter"]);
            auto defid = observed_data_defs.size();
            observed_data_defs.push_back(std::move(observed_data_def));
            if (is_active){
                std::string cmd;
                observed_data_defs[defid]->get_register_cmd_text(cmd, defid);
                need_to_set_event = tx_buf->insert_data(cmd.c_str(), cmd.size()) || need_to_set_event;
            }
        }
    }
    if (need_to_set_event){
        ::SetEvent(schedule_event);
    }
}

void DCSWorld::lua_clear_observed_data(){
    std::lock_guard lock{mutex};
    observed_data_defs.clear();
    struct C_CMD {
        command_header hdr;
    } cmd {
        make_command_header('C', sizeof(C_CMD) - 4),
    };
    if (tx_buf->insert_data(reinterpret_cast<char*>(&cmd), sizeof(cmd))){
        ::SetEvent(schedule_event);
    }
}

//============================================================================================
// Create Lua scripting environment
//============================================================================================
void DCSWorld::initLuaEnv(sol::state &lua){
    auto dcs = lua.create_table();

    dcs["perform_clickable_action"] = [this](sol::variadic_args args){
        lua_c_interface(*mapper_EngineInstance(), "dcs.perform_clickable_action", [this, args](){
            lua_perform_clickable_action(args);
        });
    };
    dcs["clickable_action_performer"] = [this](sol::variadic_args args){
        return lua_c_interface(*mapper_EngineInstance(), "dcs.perform_clickable_action", [this, args](){
            return lua_clickable_action_performer(args);
        });
    };
    dcs["add_observed_data"] = [this](sol::object arg0){
        lua_c_interface(*mapper_EngineInstance(), "dcs.add_observed_data", [this, arg0](){
            lua_add_observed_data(arg0);
        });
    };
    dcs["clear_observed_data"] = [this](){
        lua_c_interface(*mapper_EngineInstance(), "dcs.clear_observed_data", [this](){
            lua_clear_observed_data();
        });
    };
    
    lua["dcs"] = dcs;
}
