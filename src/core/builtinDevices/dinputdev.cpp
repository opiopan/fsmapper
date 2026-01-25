//
// dinputdev.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <stdexcept>
#include <sstream>
#include <optional>
#include <cmath>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "dinputdev.h"
#include "tools.h"
#include "guid.hpp"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

static constexpr auto AXIS_VALUE_MAX = JOYSTICK_AXIS_VALUE_MAX;
static constexpr auto AXIS_VALUE_MIN = JOYSTICK_AXIS_VALUE_MIN;

static const auto pi = std::acos(-1.0);
static const auto pi2 = pi * 2.0;

//============================================================================================
// Device capabilities representation
//============================================================================================
class DeviceCaps{
public:
    using NameList = std::unordered_map<std::string, int>;

protected:
    struct UnitDef{
        std::string name;
        FSMDEVUNIT_VALTYPE valtype;
        int max_value;
        int min_value;
        GUID type_guid;
        DWORD type_and_id;
        int buff_offset;
        int (*unit_value)(const char* buf, int offset);
        bool map_with_mapper_event = true;
    };

    static constexpr auto AXIS_X = 0x1;
    static constexpr auto AXIS_Y = 0x2;
    static constexpr auto AXIS_Z = 0x4;
    static constexpr auto AXIS_RX = 0x8;
    static constexpr auto AXIS_RY = 0x10;
    static constexpr auto AXIS_RZ = 0x20;
    static constexpr auto AXIS_SLIDER1 = 0x40;
    static constexpr auto AXIS_SLIDER2 = 0x80;

    const NameList allowlist;
    const NameList denylist;
    uint32_t defined_axes = 0;
    std::vector<UnitDef> axes;
    std::vector<UnitDef> povs;
    std::vector<UnitDef> buttons;
    int unit_num_raw = 0;
    int unit_num = 0;
    std::unique_ptr<UnitDef* []> units_raw;
    std::unique_ptr<UnitDef* []> units;
    int buf_size = 0;
    std::unique_ptr<char []> last_data;
    std::unique_ptr<char []> current_data;

public:
    using UnitPtr = UnitDef*;

    DeviceCaps(const NameList&& allowlist, const NameList&& denylist) : 
        allowlist(std::move(allowlist)), denylist(std::move(denylist)){}
    DeviceCaps(const DeviceCaps&) = delete;
    DeviceCaps(DeviceCaps&&) = delete;
    ~DeviceCaps() = default;

    inline int get_unit_num() const {return unit_num;}
    inline const std::string& get_unit_name(int index) const {return units.get()[index]->name;}
    inline FSMDEVUNIT_VALTYPE get_unit_valtype(int index) const {return units.get()[index]->valtype;}
    inline int get_unit_max_value(int index) const {return units.get()[index]->max_value;}
    inline int get_unit_min_value(int index) const {return units.get()[index]->min_value;}

    inline UnitPtr get_unit_ptr(int index) const {return units.get()[index];}
    inline int get_unit_value(UnitPtr unit) const {return unit->unit_value(current_data.get(), unit->buff_offset);}
    inline int get_unit_last_value(UnitPtr unit) const {return unit->unit_value(last_data.get(), unit->buff_offset);}

    void add_unit(const DIDEVICEOBJECTINSTANCEA* def){
        UnitDef unit_def;
        unit_def.type_guid = def->guidType;
        unit_def.type_and_id = def->dwType;

        if (def->dwType & DIDFT_AXIS){
            if (def->guidType == GUID_Slider){
                if (!(defined_axes & AXIS_SLIDER1)){
                    defined_axes |= AXIS_SLIDER1;
                    unit_def.name = "slider1";
                }else if (!(defined_axes & AXIS_SLIDER2)){
                    defined_axes |= AXIS_SLIDER2;
                    unit_def.name = "slider2";
                }else{
                    throw std::runtime_error("unsupported device: there are sliders more than two");
                }
            }else{
                struct AxisDef{int mask; const char* name;};
                static const std::map<GUID_KEY, AxisDef> dict = {
                    {GUID_XAxis, {AXIS_X, "x"}},
                    {GUID_YAxis, {AXIS_Y, "y"}},
                    {GUID_ZAxis, {AXIS_Z, "z"}},
                    {GUID_RxAxis, {AXIS_RX, "rx"}},
                    {GUID_RyAxis, {AXIS_RY, "ry"}},
                    {GUID_RzAxis, {AXIS_RZ, "rz"}},
                };
                if (dict.count(def->guidType) == 0){
                    throw std::runtime_error("unsupported device: unknown axis type");
                }
                auto& axis = dict.at(def->guidType);
                defined_axes |= axis.mask;
                unit_def.name = axis.name;
            }
            unit_def.valtype = FSMDU_TYPE_ABSOLUTE;
            unit_def.max_value = AXIS_VALUE_MAX;
            unit_def.min_value = AXIS_VALUE_MIN;
            unit_def.unit_value = [](const char* buf, int offset) -> int{
                return *reinterpret_cast<const LONG*>(buf + offset);
            };
            if (is_valid_name(unit_def.name)){
                axes.push_back(std::move(unit_def));
            }
        }else if (def->dwType & DIDFT_POV){
            std::ostringstream os;
            os << "pov" << povs.size() + 1;
            unit_def.name = std::move(os.str());
            unit_def.valtype = FSMDU_TYPE_ABSOLUTE;
            unit_def.min_value = -1;
            unit_def.max_value = 360 * 100;
            unit_def.unit_value = [](const char* buf, int offset) -> int{
                return *reinterpret_cast<const DWORD*>(buf + offset);
            };
            if (is_valid_name(unit_def.name)){
                povs.push_back(std::move(unit_def));
            }
        }else if (def->dwType & DIDFT_BUTTON){
            std::ostringstream os;
            os << "button" << buttons.size() + 1;
            unit_def.name = std::move(os.str());
            unit_def.valtype = FSMDU_TYPE_BINARY;
            unit_def.min_value = 0;
            unit_def.max_value = 1;
            unit_def.unit_value = [](const char* buf, int offset) -> int{
                auto raw = *reinterpret_cast<const BYTE*>(buf + offset);
                return raw & 0x80 ? 1 : 0;
            };
            if (is_valid_name(unit_def.name)){
                buttons.push_back(std::move(unit_def));
            }
        }else{
            throw std::runtime_error("unsupported device: unknown object type");
        }
    }

    UnitPtr mark_unit_as_source_of_meta_unit(const char* name, bool enable_as_raw_unit){
        for (auto& unit : axes){
            if (unit.name == name){
                unit.map_with_mapper_event = enable_as_raw_unit;
                return &unit;
            }
        }
        // for (auto& unit : povs){
        //     if (unit.name == name){
        //         unit.map_with_mapper_event = enable_as_raw_unit;
        //         return &unit;
        //     }
        // }
        // for (auto& unit : buttons){
        //     if (unit.name == name){
        //         unit.map_with_mapper_event = enable_as_raw_unit;
        //         return &unit;
        //     }
        // }
        return nullptr;
    }

    void fix_definition(ComPtr<IDirectInputDevice8A>& dinput_device){
        //
        // Calculate data buffer size
        //
        auto max_unit_num = axes.size() + povs.size() + buttons.size();
        units = std::make_unique<UnitDef* []>(max_unit_num);
        units_raw = std::make_unique<UnitDef* []>(max_unit_num);
        int offset = 0;
        for (auto& unit : axes){
            unit.buff_offset = offset;
            offset += sizeof(LONG);
            units_raw.get()[unit_num_raw] = &unit;
            unit_num_raw++;
            if (unit.map_with_mapper_event){
                units.get()[unit_num] = &unit;
                unit_num++;
            }
        }
        for (auto& unit : povs){
            unit.buff_offset = offset;
            offset += sizeof(DWORD);
            units_raw.get()[unit_num_raw] = &unit;
            unit_num_raw++;
            if (unit.map_with_mapper_event){
                units.get()[unit_num] = &unit;
                unit_num++;
            }
        }
        for (auto& unit : buttons){
            unit.buff_offset = offset;
            offset += sizeof(BYTE);
            units_raw.get()[unit_num_raw] = &unit;
            unit_num_raw++;
            if (unit.map_with_mapper_event){
                units.get()[unit_num] = &unit;
                unit_num++;
            }
        }
        buf_size = ((offset + 3) / 4) * 4;
        current_data = std::make_unique<char []>(buf_size);
        last_data = std::make_unique<char []>(buf_size);

        //
        // Register data format
        //
        auto formats = std::make_unique<DIOBJECTDATAFORMAT []>(unit_num_raw);
        for (int i = 0; i < unit_num_raw; i++){
            auto unit = units_raw.get()[i];
            auto& format = formats.get()[i];
            format.pguid = nullptr;
            format.dwOfs = unit->buff_offset;
            format.dwType = unit->type_and_id;
            format.dwFlags = 0;
        }
        DIDATAFORMAT format;
        format.dwSize = sizeof(format);
        format.dwObjSize = sizeof(DIOBJECTDATAFORMAT);
        format.dwFlags = DIDF_ABSAXIS;
        format.dwDataSize = buf_size;
        format.dwNumObjs = unit_num_raw;
        format.rgodf = formats.get();
        auto hr = dinput_device->SetDataFormat(&format);
        if (FAILED(hr)){
            throw std::runtime_error("registering data format to DirectInput device failed");
        }

        //
        // Set axis mode to absolute
        //
        DIPROPDWORD axismode = {0};
        axismode.diph.dwSize = sizeof(axismode);
        axismode.diph.dwHeaderSize = sizeof(axismode.diph);
        axismode.diph.dwHow = DIPH_DEVICE;
        axismode.diph.dwObj = 0;
        axismode.dwData = DIPROPAXISMODE_ABS;
        hr = dinput_device->SetProperty(DIPROP_AXISMODE, &axismode.diph);
        if (FAILED(hr)){
            throw std::runtime_error("setting axis mode of DirectInput device failed");
        }

        //
        // Set range of each axis
        //
        for (auto& axis : axes){
            DIPROPRANGE range = {0};
            range.diph.dwSize = sizeof(range);
            range.diph.dwHeaderSize = sizeof(range.diph);
            range.diph.dwHow = DIPH_BYID;
            range.diph.dwObj = axis.type_and_id;
            range.lMax = axis.max_value;
            range.lMin = axis.min_value;
            hr = dinput_device->SetProperty(DIPROP_RANGE, &range.diph);
            if (FAILED(hr)){
                throw std::runtime_error("setting range of axis of DirectInput device failed");
            }
        }
    }

    template <typename TFunction>
    void process_event(ComPtr<IDirectInputDevice8A>& dinput_device, TFunction event_callback){
        last_data.swap(current_data);
        if (FAILED(dinput_device->GetDeviceState(buf_size, current_data.get()))){
            throw std::runtime_error("getting DirectInput device data failed");
        }
        for (int i = 0; i < unit_num; i++){
            auto unit = units.get()[i];
            auto last_val = unit->unit_value(last_data.get(), unit->buff_offset);
            auto current_val = unit->unit_value(current_data.get(), unit->buff_offset);
            if (last_val != current_val){
                event_callback(i, current_val);
            }
        }
    }

    std::string to_string(const char* device_name){
        std::ostringstream os;
        os << "\"" << device_name << "\" (" << unit_num_raw << " objects) has been opened";
        if (axes.size()){
            os << std::endl << "    " << axes.size() << " axes : ";
            for (auto& axis : axes){
                os << axis.name << " ";
            }
        }
        if (povs.size()){
            os << std::endl << "    " << povs.size() << " POVs";
        }
        if (buttons.size()){
            os << std::endl << "    " << buttons.size() << " buttons";
        }
        return std::move(os.str());
    }

protected:
    bool is_valid_name(const std::string& name){
        if (allowlist.size() > 0){
            return allowlist.count(name) > 0 ? true : false;
        }else if (denylist.size() > 0){
            return denylist.count(name) > 0 ? false : true;
        }else{
            return true;
        }
    }
};

//============================================================================================
// Virtual POV unit
//============================================================================================
struct VirtualPov{
    FSMAPPER_HANDLE mapper {nullptr};
    std::string name;
    std::string x_unit_name;
    std::string y_unit_name;
    DeviceCaps::UnitPtr x_unit {nullptr};
    DeviceCaps::UnitPtr y_unit {nullptr};
    int resolution {4};
    double angle_unit;
    double angle_allowable_error;
    double angle_allowable_error_continuous;
    bool disable_source {true};
    double th_on {0.8 * AXIS_VALUE_MAX};
    double th_off {0.3 * AXIS_VALUE_MAX};
    int last_x {0};
    int last_y {AXIS_VALUE_MIN};
    double last_theta {0.};

    VirtualPov() = delete;
    VirtualPov(const VirtualPov&) = delete;
    VirtualPov(VirtualPov&& src){
        *this = std::move(src);
    }
    VirtualPov& operator = (VirtualPov&& src){
        mapper = src.mapper;
        name = std::move(src.name);
        x_unit_name = std::move(src.x_unit_name);
        y_unit_name = std::move(src.y_unit_name);
        x_unit = src.x_unit;
        y_unit = src.y_unit;
        disable_source = src.disable_source;
        return *this;
    }

    VirtualPov( FSMAPPER_HANDLE mapper, LUAVALUE def) : mapper(mapper){
        if (luav_getType(def) != LV_TABLE){
            throw std::runtime_error("virtual POV difinition must be a table");
        }
        auto name = luav_getItemWithKey(def, "name");
        auto xaxis = luav_getItemWithKey(def, "xaxis");
        auto yaxis = luav_getItemWithKey(def, "yaxis");
        auto resolution = luav_getItemWithKey(def, "resolution");
        auto disable_source = luav_getItemWithKey(def, "disable_source");
        if (luav_getType(name) == LV_STRING){
            this->name = luav_asString(name);
        }else{
            throw std::runtime_error("name parameter must be specified as string for virtual POV definition");
        }
        if (luav_getType(xaxis) == LV_STRING && luav_getType(yaxis) == LV_STRING){
            this->x_unit_name = luav_asString(xaxis);
            this->y_unit_name = luav_asString(yaxis);
        }else{
            throw std::runtime_error("xaxis parameter and yaxis parameter must be specified as string for virtual POV definition");
        }
        if (luav_getType(resolution) == LV_NUMBER){
            this->resolution = luav_asInt(resolution);
            if (this->resolution < 1){
                throw std::runtime_error("resolution parameter for virtual POV definition must be integer grater than zero");
            }
        }else{
            throw std::runtime_error("resolution parameter must be specified as integer grater than zero for virtual POV definition");
        }
        if (!luav_isNull(disable_source)){
            if (luav_getType(disable_source) == LV_BOOL){
                this->disable_source = luav_asBool(disable_source);
            }else{
                throw std::runtime_error("disable_source parameter for virtual POV definition must be bool");
            }
        }

        angle_unit = pi2 / this->resolution;
        angle_allowable_error = angle_unit * 0.5;
        angle_allowable_error_continuous = angle_unit * 0.75;
    }

    template <typename TFunction>
    void update(DeviceCaps& caps, TFunction callback){
        auto x = caps.get_unit_value(x_unit);
        auto y = caps.get_unit_value(y_unit);
        if (x != last_x || y != last_y){
            last_x = x;
            last_y = y;
            auto r = std::sqrt(static_cast<double>(x) * static_cast<double>(x) + static_cast<double>(y) * static_cast<double>(y));
            double theta = 0.;
            if (last_theta >= 0){
                if (r < th_off){
                    theta = -1.;
                }else{
                    theta = calculate_angle(x, y);
                    if (last_theta == 0.){
                        if (theta <= angle_allowable_error || theta >= pi2 - angle_allowable_error){
                            return;
                        }
                    }else{
                        if (theta <= last_theta + angle_allowable_error_continuous && theta >= last_theta - angle_allowable_error_continuous){
                            return;
                        }
                    }
                    theta = normalize_angle(theta);
                }
            }else{
                if (r < th_on){
                    return;
                }else{
                    theta = normalize_angle(calculate_angle(x, y));
                }
            }
            callback(theta < 0. ? -1 : static_cast<int>(std::round(theta * 18000. / pi)));
            last_theta = theta;
        }
    }

protected:
    inline double calculate_angle(int x, int y){
        auto raw = std::atan2(static_cast<double>(x), static_cast<double>(-y));
        return raw < 0. ? raw + pi2 : raw;
    }

    inline double normalize_angle(double angle){
        auto factor = std::round(angle / angle_unit);
        return angle_unit * (factor == resolution ? 0 : factor);
    }
};

//============================================================================================
// Capsulize DirectInputDevice8 interface
//============================================================================================
class DirectInputDevice{
public:
    class MapperDevice{
    protected:
        DirectInputDevice& dinput_device;
        FSMDEVICE mapper_device;
    public:
        MapperDevice() = delete;
        MapperDevice(DirectInputDevice& dinput_device, FSMDEVICE mapper_device) : 
            dinput_device(dinput_device), mapper_device(mapper_device){}
        MapperDevice(const MapperDevice&) = delete;
        MapperDevice(MapperDevice&&) = delete;
        MapperDevice& operator = (const MapperDevice&) = delete;
        MapperDevice& operator = (MapperDevice&&) = delete;
        ~MapperDevice() = default;
        DirectInputDevice& get_dinput_device(){return dinput_device;}
        FSMDEVICE get_mapper_device(){return mapper_device;}
    };

protected:
    std::string name;
    GUID guid;
    FSMAPPER_HANDLE mapper;
    ComPtr<IDirectInputDevice8A> dinput_device;
    DeviceCaps device_caps;
    std::vector<VirtualPov> vpovs;
    bool running = false;
    std::unordered_map<MapperDevice*, std::unique_ptr<MapperDevice>> mapper_devices;
    WinHandle event;

    struct CallbackContext{
        DirectInputDevice& self;
        std::optional<std::runtime_error> error;
    };

public:
    DirectInputDevice() = delete;
    DirectInputDevice(const DirectInputDevice&) = delete;
    DirectInputDevice(DirectInputDevice&&) = delete;
    DirectInputDevice& operator = (const DirectInputDevice&) = delete;
    DirectInputDevice& operator = (DirectInputDevice&&) = delete;

    DirectInputDevice(const GUID& guid, const std::string& name, FSMAPPER_HANDLE mapper, IDirectInputDevice8A* dinput_device,
                      const DeviceCaps::NameList&& allowlist, const DeviceCaps::NameList&& denylist, std::vector<VirtualPov>&& vpovs) : 
        guid(guid), name(name), mapper(mapper), dinput_device(dinput_device),
        device_caps(std::move(allowlist), std::move(denylist)), vpovs(std::move(vpovs)){
        event = ::CreateEventA(nullptr, true, false, nullptr);
        CallbackContext ctx = {*this, std::nullopt};
        dinput_device->EnumObjects(enum_object_callback, &ctx, DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV);
        if (ctx.error){
            throw *ctx.error;
        }
        for (auto& vpov : this->vpovs){
            vpov.x_unit = device_caps.mark_unit_as_source_of_meta_unit(vpov.x_unit_name.c_str(), !vpov.disable_source);
            vpov.y_unit = device_caps.mark_unit_as_source_of_meta_unit(vpov.y_unit_name.c_str(), !vpov.disable_source);
            if (!vpov.x_unit || !vpov.y_unit){
                throw std::runtime_error("specified unit of x-axis or y-asis for virtual POV is not found");
            }
        }
        device_caps.fix_definition(this->dinput_device);
        if (FAILED(dinput_device->SetCooperativeLevel(nullptr, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))){
            throw std::runtime_error("setting cooperative level of DirectInput device failed");
        }
        if (FAILED(dinput_device->SetEventNotification(event))){
            throw std::runtime_error("associating a event to DirectInput device failed");
        }
        if (FAILED(dinput_device->Acquire())){
            throw std::runtime_error("acquiring DirectInput device failed");
        }

        auto&& caps_string = device_caps.to_string(name.c_str());
        fsmapper_putLog(mapper, FSMLOG_DEBUG, caps_string.c_str());
    }

    ~DirectInputDevice(){
        dinput_device->Unacquire();
    }

    const bool is_running(){return running;}
    void start(){running = true;}
    void stop(){running = false;}

    const GUID& get_guid() const {return guid;}
    const std::string& get_name() const {return name;}
    HANDLE get_event() const {return event;}
    

    MapperDevice* add_mapper_device(FSMDEVICE device_handle){
        auto device = std::make_unique<MapperDevice>(*this, device_handle);
        auto device_ptr = device.get();
        mapper_devices.emplace(device_ptr, std::move(device));
        return device_ptr;
    }

    void remove_mapper_device(MapperDevice* device){
        mapper_devices.erase(device);
    }

    size_t get_mapper_device_num(){
        return mapper_devices.size();
    }

    const DeviceCaps& get_device_caps() const{
        return device_caps;
    }

    size_t get_vpov_num() const{
        return vpovs.size();
    }

    const VirtualPov& get_vpov(size_t index) const{
        return vpovs.at(index);
    }

    void process_event(){
        device_caps.process_event(dinput_device, [this](int index, int value){
            for (auto& [key, device] : mapper_devices){
                fsmapper_raiseEvent(mapper, device->get_mapper_device(), index, value);
            }
        });
        for (auto i = 0; i < vpovs.size(); i++){
            auto& vpov = vpovs.at(i);
            vpov.update(device_caps, [this, i](int value){
                for (auto& [key, device] : mapper_devices){
                    fsmapper_raiseEvent(mapper, device->get_mapper_device(), device_caps.get_unit_num() + i, value);
                }
            });
        }
    }

protected:
    static BOOL enum_object_callback(LPCDIDEVICEOBJECTINSTANCEA lpddoi, LPVOID pvRef){
        auto ctx = static_cast<CallbackContext*>(pvRef);
        try {
            ctx->self.device_caps.add_unit(lpddoi);
            return true;
        }catch (std::runtime_error& e){
            ctx->error = std::move(e);
            return false;
        }
    }
};

//============================================================================================
// Capsulize DirectInput8 interface
//============================================================================================
class DirectInput{
protected:
    ComPtr<IDirectInput8A> dinput;
    std::vector<DIDEVICEINSTANCEA> device_ids;

public:
    DirectInput(){
        HMODULE hModule = nullptr;
        ::GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, 
            reinterpret_cast<LPCSTR>(dinput_PluginDeviceOps),
            &hModule);
        IDirectInput8A* rawptr = nullptr;
        auto hr = ::DirectInput8Create(hModule, DIRECTINPUT_VERSION, IID_IDirectInput8A, 
                                    reinterpret_cast<void**>(&rawptr), nullptr);
        if (!SUCCEEDED(hr)){
            throw std::runtime_error("failed to initialize DirectInput environment");
        }
        dinput = rawptr;

        IDirectInputDevice8A* rawdev = nullptr;
    }
    ~DirectInput() = default;
    DirectInput(const DirectInput&) = delete;
    DirectInput(DirectInput&&) = delete;
    DirectInput& operator = (const DirectInput&) = delete;
    DirectInput& operator = (DirectInput&&) = delete;

    operator IDirectInput8A* () const {return dinput;}
    IDirectInput8A* operator -> () const {return dinput;}

    void reflesh_ids(){
        device_ids.clear();
        dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, enum_devices_callback, this, DIEDFL_ALLDEVICES);
    }

    const DIDEVICEINSTANCEA* find_device_id(const char* name){
        for (auto& id : device_ids){
            if (strcmp(name, id.tszInstanceName) == 0){
                return &id;
            }
        }
        return nullptr;
    }

    const DIDEVICEINSTANCEA* find_device_id(const GUID& guid){
        for (auto& id : device_ids){
            if (::IsEqualGUID(guid, id.guidInstance)){
                return &id;
            }
        }
        return nullptr;
    }

    const DIDEVICEINSTANCEA* find_device_id(int index){
        if (index >= 0 && index < device_ids.size()){
            return &device_ids[index];
        }else{
            return nullptr;
        }
    }

    std::unique_ptr<DirectInputDevice> create_device(FSMAPPER_HANDLE mapper, const DIDEVICEINSTANCEA& device_id,
                                                     const DeviceCaps::NameList&& allowlist,
                                                     const DeviceCaps::NameList&& denylist,
                                                     std::vector<VirtualPov>&& vpovs){
        IDirectInputDevice8A* rawdev = nullptr;
        auto hr = dinput->CreateDevice(device_id.guidInstance, &rawdev, nullptr);
        if (!SUCCEEDED(hr)){
            throw std::runtime_error("failed to create a direct input devce");
        }
        return std::move(std::make_unique<DirectInputDevice>(
            device_id.guidInstance, device_id.tszInstanceName, mapper, rawdev, std::move(allowlist), std::move(denylist), std::move(vpovs)));
    }

    std::string to_string() const{
        std::ostringstream os;
        os << "detected " << device_ids.size() << " DirectInput gaming devices:";
        tools::guid guid;
        for (auto& id : device_ids){
            guid = id.guidInstance;
            os << std::endl << "    " << static_cast<const char*>(guid) << " : " << id.tszInstanceName;
        }
        return std::move(os.str());
    }

protected:
    static BOOL enum_devices_callback(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef){
        auto self = reinterpret_cast<DirectInput*>(pvRef);
        self->device_ids.push_back(*lpddi);
        return true;
    }
};

//============================================================================================
// Base context object to handle DirectInput device
//============================================================================================
class DinputDev{
protected:
    std::mutex mutex;
    WinHandle event;
    bool should_stop;
    int update_count = 1;
    std::thread reader;
    FSMAPPER_HANDLE mapper;
    DirectInput dinput;
    std::unordered_map<std::string, std::unique_ptr<DirectInputDevice>> dinput_devices;
    std::unordered_map<DirectInputDevice::MapperDevice*, DirectInputDevice::MapperDevice*> mapper_devices;
    
public:
    DinputDev(FSMAPPER_HANDLE mapper) : mapper(mapper){
        event = ::CreateEventA(nullptr, false, false, nullptr);
        dinput.reflesh_ids();
        auto&& debug_msg = dinput.to_string();
        fsmapper_putLog(mapper, FSMLOG_DEBUG, debug_msg.c_str());

        reader = std::move(std::thread([this]{
            try{
                std::unique_lock lock(mutex);
                auto eventbuf_size = 10;
                auto eventbuf = std::make_unique<HANDLE []>(eventbuf_size);
                std::vector<DirectInputDevice*> devices;
                auto last_count = 0;
                while (true){
                    //
                    // maintain event list
                    //
                    if (last_count != update_count){
                        last_count = update_count;
                        if (eventbuf_size < dinput_devices.size() + 1){
                            eventbuf_size = dinput_devices.size() + 10;
                            eventbuf = std::make_unique<HANDLE []>(eventbuf_size);
                        }
                        devices.clear();
                        auto idx = 0;
                        eventbuf.get()[idx++] = event;
                        for (auto& [k, device] : dinput_devices){
                            if (device->is_running()){
                                eventbuf.get()[idx++] = device->get_event();
                                devices.push_back(device.get());
                            }
                        }
                    }

                    //
                    // wait for devices to become read data or changing device set
                    //
                    lock.unlock();
                    auto signaled_event = ::WaitForMultipleObjects(devices.size() + 1, eventbuf.get(), false, INFINITE);
                    lock.lock();

                    //
                    // finish if stop request is received
                    //
                    if (should_stop){
                        return;
                    }

                    //
                    // do action according to each event
                    //
                    if (signaled_event == WAIT_OBJECT_0){
                        ::ResetEvent(event);
                    }else if (signaled_event > WAIT_OBJECT_0 && signaled_event <= WAIT_OBJECT_0 + devices.size()){
                        auto device = devices[signaled_event - WAIT_OBJECT_0 - 1];
                        ::ResetEvent(device->get_event());
                        device->process_event();
                    }
                }
            }catch (std::runtime_error& e){
                fsmapper_putLog(this->mapper, FSMLOG_ERROR, e.what());
                fsmapper_abort(this->mapper);
            }
        }));
    }

    ~DinputDev(){
        stop();
        reader.join();
    }

    DirectInput& get_dinput(){return dinput;}

    void stop(){
        std::unique_lock lock(mutex);
        should_stop = true;
        ::SetEvent(event);
    }

    DirectInputDevice::MapperDevice* open_device(FSMDEVICE dev_handle, const DIDEVICEINSTANCEA& device_id,
                                                 const DeviceCaps::NameList&& allowlist,
                                                 const DeviceCaps::NameList&& denylist,
                                                 std::vector<VirtualPov>&& vpovs){
        std::unique_lock lock(mutex);
        tools::guid guid(device_id.guidInstance);
        if (dinput_devices.count(static_cast<const char*>(guid)) == 0){
            dinput_devices.emplace(
                guid, std::move(dinput.create_device(mapper, device_id, std::move(allowlist), std::move(denylist), std::move(vpovs))));
        }
        auto& dinput_device = dinput_devices.at(static_cast<const char*>(guid));
        dinput_device->stop();
        auto mapper_device = dinput_device->add_mapper_device(dev_handle);
        mapper_devices.emplace(mapper_device, mapper_device);
        update_count++;
        ::SetEvent(event);
        return mapper_device;
    }

    void start_device(DirectInputDevice::MapperDevice* mapper_device){
        std::unique_lock lock(mutex);
        auto& dinput_device = mapper_device->get_dinput_device();
        dinput_device.start();
        update_count++;
        ::SetEvent(event);
    }

    void close_device(DirectInputDevice::MapperDevice* mapper_device){
        std::unique_lock lock(mutex);
        auto& dinput_device = mapper_device->get_dinput_device();
        dinput_device.remove_mapper_device(mapper_device);
        if (dinput_device.get_mapper_device_num() == 0){
            tools::guid guid(dinput_device.get_guid());
            dinput_devices.erase(static_cast<const char*>(guid));
            update_count++;
            ::SetEvent(event);
        }
    }
};

//============================================================================================
// plugin interfaces that expose to mapper core
//============================================================================================
template <typename TFunc, typename TReturn>
inline auto plugin_interface(FSMAPPER_HANDLE handle, TReturn value_in_fail, TFunc function){
    try{
        return function();
    }catch (std::runtime_error& e){
        fsmapper_putLog(handle, FSMLOG_ERROR, e.what());
        return value_in_fail;
    }
}

static bool dinputdev_init(FSMAPPER_HANDLE handle){
    return plugin_interface(handle, false, [handle](){
        auto dinputdev = new DinputDev(handle);
        fsmapper_setContext(handle, dinputdev);
        return true;
    });
}

static bool dinputdev_term(FSMAPPER_HANDLE handle){
    return plugin_interface(handle, false, [handle](){
        auto dinputdev = static_cast<DinputDev*>(fsmapper_getContext(handle));
        delete dinputdev;
        return true;
    });
}

static bool dinputdev_open(FSMAPPER_HANDLE handle, FSMDEVICE dev_handle, LUAVALUE identifier, LUAVALUE options){
    auto make_list = [](LUAVALUE value){
        DeviceCaps::NameList list;
        if (luav_getType(value) == LV_TABLE){
            int idx = 1;
            while (true){
                auto item = luav_getItemWithIndex(value, idx++);
                auto type = luav_getType(item);
                if (type == LV_STRING){
                    list.emplace(std::move(std::string(luav_asString(item))), 0);
                }else if (type == LV_NULL){
                    break;
                }
            }
        }
        return std::move(list);
    };

    return plugin_interface(handle, false, [handle, dev_handle, identifier, options, &make_list](){
        auto dinputdev = static_cast<DinputDev*>(fsmapper_getContext(handle));
        dinputdev->get_dinput().reflesh_ids();
        auto name = luav_getItemWithKey(identifier, "name");
        auto guid = luav_getItemWithKey(identifier, "guid");
        auto index = luav_getItemWithKey(identifier, "index");
        const DIDEVICEINSTANCEA* device_id{nullptr};
        if (!luav_isNull(name)){
            if (luav_getType(name) != LV_STRING){
                throw std::runtime_error("\"name\" value for direct input device identifier must be string");
            }
            device_id = dinputdev->get_dinput().find_device_id(luav_asString(name));
            if (!device_id){
                throw std::runtime_error("direct input device which specified by \"name\" identifier is not found");
            }
        }else if (!luav_isNull(guid)){
            if (luav_getType(guid) != LV_STRING){
                throw std::runtime_error("\"guid\" value for direct input device identifier must be string");
            }
            tools::guid id(luav_asString(guid));
            device_id = dinputdev->get_dinput().find_device_id(static_cast<const GUID&>(id));
            if (!device_id){
                throw std::runtime_error("direct input device which specified by \"guid\" identifier is not found");
            }
        }else if (!luav_isNull(index)){
            if (luav_getType(index) != LV_NUMBER){
                throw std::runtime_error("\"guid\" value for direct input device identifier must be numeric");
            }
            device_id = dinputdev->get_dinput().find_device_id(luav_asInt(index));
            if (!device_id){
                throw std::runtime_error("direct input device which specified by \"index\" identifier is not found");
            }
        }

        if (!device_id){
            throw std::runtime_error("no valid identifire for direct input device is specified");
        }

        auto&& denylist = make_list(luav_getItemWithKey(options, "denylist"));
        auto&& allowlist = make_list(luav_getItemWithKey(options, "allowlist"));
        if (denylist.size() > 0 && allowlist.size() > 0){
            throw std::runtime_error("both of allowlist and denylist cannot be specified");
        }
        std::vector<VirtualPov> vpovs;
        auto vpovs_def = luav_getItemWithKey(options, "vpovs");
        for (auto i = 1; true; i++){
            auto def = luav_getItemWithIndex(vpovs_def, i);
            if (luav_isNull(def)){
                break;
            }
            vpovs.emplace_back(handle, def);
        }
        auto device = dinputdev->open_device(dev_handle, *device_id, std::move(allowlist), std::move(denylist), std::move(vpovs));
        fsmapper_setContextForDevice(handle, dev_handle, device);
        return true;
    });
}

static bool dinputdev_start(FSMAPPER_HANDLE handle, FSMDEVICE dev_handle){
    return plugin_interface(handle, false, [handle, dev_handle](){
        auto dinputdev = static_cast<DinputDev*>(fsmapper_getContext(handle));
        auto device = static_cast<DirectInputDevice::MapperDevice*>(fsmapper_getContextForDevice(handle, dev_handle));
        dinputdev->start_device(device);
        return true;
    });
}

static bool dinputdev_close(FSMAPPER_HANDLE handle, FSMDEVICE dev_handle){
    return plugin_interface(handle, false, [handle, dev_handle](){
        auto dinputdev = static_cast<DinputDev*>(fsmapper_getContext(handle));
        auto device = static_cast<DirectInputDevice::MapperDevice*>(fsmapper_getContextForDevice(handle, dev_handle));
        dinputdev->close_device(device);
        return true;
    });
}

static size_t dinputdev_getUnitNum(FSMAPPER_HANDLE handle, FSMDEVICE dev_handle){
    auto device = static_cast<DirectInputDevice::MapperDevice*>(fsmapper_getContextForDevice(handle, dev_handle));
    auto& caps = device->get_dinput_device().get_device_caps();
    return caps.get_unit_num() + device->get_dinput_device().get_vpov_num();
}

static bool dinputdev_getUnitDef(FSMAPPER_HANDLE handle, FSMDEVICE dev_handle, size_t index, FSMDEVUNITDEF *def){
    return plugin_interface(handle, false, [handle, dev_handle, index, def](){
        auto device = static_cast<DirectInputDevice::MapperDevice*>(fsmapper_getContextForDevice(handle, dev_handle));
        auto& caps = device->get_dinput_device().get_device_caps();
        if (index < caps.get_unit_num()){
            def->name = caps.get_unit_name(index).c_str();
            def->direction = FSMDU_DIR_INPUT;
            def->type = caps.get_unit_valtype(index);
            def->maxValue = caps.get_unit_max_value(index);
            def->minValue = caps.get_unit_min_value(index);
        }else{
            const auto& vpos = device->get_dinput_device().get_vpov(index - caps.get_unit_num());
            def->name = vpos.name.c_str();
            def->direction = FSMDU_DIR_INPUT;
            def->type = FSMDU_TYPE_ABSOLUTE;
            def->maxValue = 360 * 100;
            def->minValue = -1;
        }
        return true;
    });
}

static bool dinputdev_sendUnitValue(FSMAPPER_HANDLE handle, FSMDEVICE dev_handle, size_t index, int value){
    fsmapper_putLog(handle, FSMLOG_ERROR, "unsupported function");
    return false;
}

static MAPPER_PLUGIN_DEVICE_OPS dinputdev_ops = {
    "dinput",
    "DirectInput gaming device",
    dinputdev_init,
    dinputdev_term,
    dinputdev_open,
    dinputdev_start,
    dinputdev_close,
    dinputdev_getUnitNum,
    dinputdev_getUnitDef,
    dinputdev_sendUnitValue,
};

MAPPER_PLUGIN_DEVICE_OPS* dinput_PluginDeviceOps = &dinputdev_ops;
