//
// devicemodifier.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include <optional>
#include <vector>
#include "engine.h"
#include "devicemodifier.h"

//============================================================================================
// raw modifier implementation
//============================================================================================
class RawModifier : public DeviceModifier{
    uint64_t evid;
public:
    RawModifier(const RawModifier&) = default;
    RawModifier(DeviceModifierManager& manager) : DeviceModifier(manager){};
    ~RawModifier() = default;

    virtual size_t getEventNum() const{
        return 1;
    }
    virtual Event getEvent(size_t index) const{
        return {evid, "CHANGE"};
    }

    virtual std::shared_ptr<DeviceModifier> makeInstanceFitsToUnit(const char *devname, const FSMDEVUNITDEF& unit) const{
        auto instanse = std::make_shared<RawModifier>(*this);
        std::ostringstream os;
        os << devname << ":" << unit.name << ":CHANGE";
        instanse->evid = manager.getEngine().registerEvent(os.str());
        return instanse;
    }

    virtual void processUnitValueChangeEvent(int value){
        ::Event event(evid, static_cast<int64_t>(value));
        manager.getEngine().sendEvent(std::move(event));
    }

};

//============================================================================================
// button modifier implementation
//============================================================================================
class ButtonModifier : public DeviceModifier{
protected:
    enum class Status{
        off,
        on,
        maybedouble_up_off,
        maybedouble_up_on,
        maybedouble_down_on,
        maybedouble_down_off,
    };
    Status status;
    int lastvalue;
    bool negative_polarity;
    std::optional<int> threshold_max;
    std::optional<int> threshold_min;
    std::optional<int> longpress;
    std::optional<int> doubleclick;
    bool click_on_up;
    DEVICEMOD_TIME starttime;
    std::optional<DEVICEMOD_TIME> longpress_timer;
    std::optional<DEVICEMOD_TIME> doubleclick_timer;
    std::vector<Event> events;
    struct {
        int down;
        int up;
        int singleclick;
        int doubleclick;
        int longpressed;
    }eventix;

public:
    ButtonModifier(const ButtonModifier &) = default;
    ButtonModifier(DeviceModifierManager& manager, sol::object &param);
    ~ButtonModifier();

    virtual std::shared_ptr<DeviceModifier> makeInstanceFitsToUnit(const char *devname, const FSMDEVUNITDEF &unit) const;
    virtual size_t getEventNum() const;
    virtual Event getEvent(size_t index) const;

    virtual void processUnitValueChangeEvent(int value);
    virtual void processUnitValueChangeEvent(int value, DEVICEMOD_TIME now);
    virtual void processTimerEvent(DEVICEMOD_TIME timer_time);

protected:
    enum class CoockedEvent{
        none,
        down,
        up,
    };
    CoockedEvent coockEvent(int value);
};

ButtonModifier::ButtonModifier(DeviceModifierManager& manager, sol::object &param) : 
    DeviceModifier(manager), status(Status::off), lastvalue(0), negative_polarity(false), click_on_up(true){
    if (param.get_type() == sol::type::table){
        auto table = param.as<sol::table>();
        sol::object polarity = table["polarity"];
        if (polarity.get_type() != sol::type::lua_nil){
            auto val = polarity.as<std::string>();
            if (val == "negative"){
                negative_polarity = true;
            }else if (val != "positive"){
                throw MapperException("Value of \"polarity\" parameter for button modifier is invalid. "
                                      "Only \"positive\" or \"negative\" can be specified.");
            }
        }
        auto get_number = [&table](const char* param){
            sol::object object = table[param];
            if (object.get_type() != sol::type::lua_nil){
                if (object.get_type() != sol::type::number){
                    std::ostringstream os;
                    os << "Value of \"" << param << "\" parameter for button modifier must be number.";
                    throw MapperException(std::move(os.str()));
                }
                return std::optional<int>(object.as<int>());
            }
            return std::optional<int>();
        };
        threshold_max = get_number("max_threshold");
        threshold_min = get_number("min_threshold");
        if ((threshold_max.has_value() && !threshold_min.has_value()) ||
            (!threshold_max.has_value() && threshold_min.has_value())){
            throw MapperException("One of \"max_threshold\" parameter and \"min_threshold\" parameter "
                                  "for button modifier is specified, but the other is not specified.");
        }
        if (threshold_max.has_value() && !threshold_min.has_value() && threshold_max.value() <= threshold_min.value()){
            throw MapperException("Value of \"max_threshold\" parameter for button modifier "
                                  "must be grater than value of \"min_threshold\" parammeter");
        }
        longpress = get_number("longpress");
        doubleclick = get_number("doubleclick");
        if (longpress.has_value() && doubleclick.has_value() && longpress <= doubleclick){
            throw MapperException("Value of \"longpress\" parameter for button modifier"
                                  "must be grater than value of \"doubleclick\" parameter");
        }
        sol::object click_timing = table["click_timing"];
        if (click_timing.get_type() != sol::type::lua_nil){
            auto val = click_timing.as<std::string>();
            if (val == "down"){
                click_on_up = false;
            }else if (val != "up"){
                throw MapperException("Value of \"click_timing\" parameter for button modifier is invalid. "
                                      "Only \"up\" or \"down\" can be specified.");
            }
        }
    }
}

ButtonModifier::~ButtonModifier(){
    if (doubleclick_timer.has_value()){
        manager.cancelTimer(*this, doubleclick_timer.value());
    }
    if (longpress_timer.has_value()){
        manager.cancelTimer(*this, longpress_timer.value());
    }
}

std::shared_ptr<DeviceModifier> ButtonModifier::makeInstanceFitsToUnit(const char *devname, const FSMDEVUNITDEF &unit) const{
    auto instance = std::make_shared<ButtonModifier>(*this);
    if (!instance->threshold_max.has_value()){
        instance->threshold_min = unit.minValue + (unit.maxValue - unit.minValue);
        instance->threshold_max = instance->threshold_min.value() + 1;
    }else if (instance->threshold_max.value() > unit.maxValue || instance->threshold_min < unit.minValue){
        std::ostringstream os;
        os << "\"max_threshold\" or \"min_threshold\" parameter value of the modifier applied to the unit is out of range. [device: ";
        os << devname << "] [unit: " << unit.name;
        throw MapperException(std::move(os.str()));
    }

    auto register_event = [&instance, devname, &unit](const char* evname){
        std::ostringstream os;
        os << devname << ":" << unit.name << ":" << evname;
        instance->events.push_back(Event{
            instance->manager.getEngine().registerEvent(os.str()),
            devname,
        });
    };
    instance->eventix.down = instance->events.size();
    register_event("DOWN");
    instance->eventix.up = instance->events.size();
    register_event("UP");
    if (doubleclick.has_value()){
        instance->eventix.singleclick = instance->events.size();
        register_event("SINGLECLICK");
        instance->eventix.doubleclick = instance->events.size();
        register_event("DOUBLECLICK");
    }
    if (longpress.has_value()){
        instance->eventix.longpressed = instance->events.size();
        register_event("LONGPRESSED");
    }

    return instance;
}

size_t ButtonModifier::getEventNum() const{
    return events.size();
}

ButtonModifier::Event ButtonModifier::getEvent(size_t index) const{
    return events.at(index);
}

ButtonModifier::CoockedEvent ButtonModifier::coockEvent(int value){
    auto lastvalue = this->lastvalue;
    this->lastvalue = value;
    if (lastvalue < threshold_max && value >= threshold_max){
        return negative_polarity ? CoockedEvent::up : CoockedEvent::down;
    }else if (lastvalue > threshold_min && value <= threshold_min){
        return negative_polarity ? CoockedEvent::down : CoockedEvent::up;
    }else{
        return CoockedEvent::none;
    }
}

void ButtonModifier::processUnitValueChangeEvent(int value){
    if (longpress.has_value() || doubleclick.has_value()){
        manager.delegateEventProcessing(*this, value);
    }else{
        auto event = coockEvent(value);
        uint64_t evid;
        if (event == CoockedEvent::down){
            evid = events[eventix.down].id;
        }else if (event == CoockedEvent::up){
            evid = events[eventix.up].id;
        }else{
            return;
        }
        manager.getEngine().sendEvent(std::move(::Event(evid)));
    }
}

void ButtonModifier::processUnitValueChangeEvent(int value, DEVICEMOD_TIME now){
    auto event = coockEvent(value);
    if (event == CoockedEvent::none){
        return;
    }
    
    if (status == Status::off && event == CoockedEvent::down){
        starttime = now;
        manager.getEngine().sendEvent(std::move(::Event(events[eventix.down].id)));
        if (longpress.has_value()){
            longpress_timer = manager.addTimer(*this, starttime + DEVICEMOD_MILLISEC(longpress.value()));
        }
        if (doubleclick.has_value() && !click_on_up){
            status = Status::maybedouble_down_on;
            doubleclick_timer = manager.addTimer(*this, starttime + DEVICEMOD_MILLISEC(doubleclick.value()));
        }else{
            status = Status::on;
        }
    }else if (status == Status::on && event == CoockedEvent::up){
        manager.getEngine().sendEvent(std::move(::Event(events[eventix.up].id)));
        if (longpress_timer.has_value()){
            manager.cancelTimer(*this, longpress_timer.value());
            longpress_timer = std::nullopt;
        }
        if (doubleclick.has_value() && click_on_up){
            if (now < starttime + DEVICEMOD_MILLISEC(doubleclick.value())){
                status = Status::maybedouble_up_off;
                doubleclick_timer = manager.addTimer(*this, starttime + DEVICEMOD_MILLISEC(doubleclick.value()));
            }else{
                manager.getEngine().sendEvent(std::move(::Event(events[eventix.singleclick].id)));
                status = Status::off;
            }
        }else{
            status = Status::off;
        }
    }else if (status == Status::maybedouble_up_off && event == CoockedEvent::down){
        manager.getEngine().sendEvent(std::move(::Event(events[eventix.down].id)));
        if (longpress.has_value()){
            longpress_timer = manager.addTimer(*this, starttime + DEVICEMOD_MILLISEC(longpress.value()));
        }
        status = Status::maybedouble_up_on;
    }else if (status == Status::maybedouble_up_on && event == CoockedEvent::up){
        manager.getEngine().sendEvent(std::move(::Event(events[eventix.up].id)));
        manager.getEngine().sendEvent(std::move(::Event(events[eventix.doubleclick].id)));
        if (doubleclick_timer.has_value()){
            manager.cancelTimer(*this, doubleclick_timer.value());
            doubleclick_timer = std::nullopt;
        }
        if (longpress_timer.has_value()){
            manager.cancelTimer(*this, longpress_timer.value());
            longpress_timer = std::nullopt;
        }
        status = Status::off;
    }
    else if (status == Status::maybedouble_down_on && event == CoockedEvent::up){
        manager.getEngine().sendEvent(std::move(::Event(events[eventix.up].id)));
        if (longpress_timer.has_value()){
            manager.cancelTimer(*this, longpress_timer.value());
            longpress_timer = std::nullopt;
        }
        status = Status::maybedouble_down_off;
    }else if (status == Status::maybedouble_down_off && event == CoockedEvent::down){
        manager.getEngine().sendEvent(std::move(::Event(events[eventix.down].id)));
        manager.getEngine().sendEvent(std::move(::Event(events[eventix.doubleclick].id)));
        if (doubleclick_timer.has_value()){
            manager.cancelTimer(*this, doubleclick_timer.value());
            doubleclick_timer = std::nullopt;
        }
        starttime = now;
        status = Status::on;
        if (longpress.has_value()){
            manager.addTimer(*this, starttime + DEVICEMOD_MILLISEC(longpress.value()));
        }
    }
};

void ButtonModifier::processTimerEvent(DEVICEMOD_TIME timer_time){
    if (doubleclick_timer.has_value() && timer_time == doubleclick_timer.value()){
        manager.getEngine().sendEvent(std::move(::Event(events[eventix.singleclick].id)));
        doubleclick_timer = std::nullopt;
        if (status == Status::maybedouble_down_off || status == Status::maybedouble_up_off){
            status = Status::off;
        }else if (status == Status::maybedouble_down_on || status == Status::maybedouble_up_on){
            status = Status::on;
        }
    }else if (longpress_timer.has_value() && timer_time == longpress_timer.value()){
        manager.getEngine().sendEvent(std::move(::Event(events[eventix.longpressed].id)));
        longpress_timer = std::nullopt;
        status = Status::off;
    }
};

//============================================================================================
// increment/decrement modifier implementation
//============================================================================================
class IncDecModifier : public DeviceModifier{
protected:
    bool is_absolute;
    int last_value;
    uint64_t evid_increment;
    uint64_t evid_decrement;

public:
    IncDecModifier(const IncDecModifier&) = default;
    IncDecModifier(DeviceModifierManager& manager) : DeviceModifier(manager){};
    ~IncDecModifier() = default;

    virtual std::shared_ptr<DeviceModifier> makeInstanceFitsToUnit(const char *devname, const FSMDEVUNITDEF& unit) const{
        auto instanse = std::make_shared<IncDecModifier>(*this);
        instanse->is_absolute = unit.type == FSMDU_TYPE_ABSOLUTE;
        instanse->last_value = unit.minValue + (unit.maxValue - unit.minValue) / 2;

        auto new_event_id = [this, devname, &unit](const char* evname){
            std::ostringstream os;
            os << devname << ":" << unit.name << ":" << evname;
            return this->manager.getEngine().registerEvent(os.str());
        };
        instanse->evid_decrement = new_event_id("INCREMENT");
        instanse->evid_decrement = new_event_id("DECTREMENT");

        return instanse;
    }

    virtual size_t getEventNum() const{
        return 2;
    }
    virtual Event getEvent(size_t index) const{
        if (index == 0){
            return {evid_increment, "INCREMENT"};
        }else{
            return {evid_decrement, "DECREMENT"};
        }
    }

    virtual void processUnitValueChangeEvent(int value){
        auto delta = is_absolute ? value - last_value : value;
        last_value = value;
        if (delta > 0){
            manager.getEngine().sendEvent(std::move(::Event(evid_increment, static_cast<int64_t>(delta))));
        }else if (delta < 0){
            manager.getEngine().sendEvent(std::move(::Event(evid_decrement, static_cast<int64_t>(-delta))));
        }
    }

};

//============================================================================================
// Interpret modifier rule definition
//============================================================================================
DeviceModifierRule&& DeviceModifierManager::makeRule(sol::object &def){
    DeviceModifierRule rule;
    rule.raw = std::make_shared<RawModifier>(*this);
    if (def.get_type() == sol::type::table){
        auto table = def.as<sol::table>();
        for (int i = 1; i <= table.size(); i++){
            sol::object item = table[i];
            if (item.get_type() != sol::type::table){
                auto itemdef = item.as<sol::table>();
                std::string modtype = itemdef["modtype"];
                sol::object modparam = itemdef["modparam"];
                std::shared_ptr<DeviceModifier> modifier;
                if (modtype == "raw"){
                    modifier = std::make_shared<RawModifier>(*this);
                }else if (modtype == "button"){
                    modifier = std::make_shared<ButtonModifier>(*this, modparam);
                }else if (modtype == "incdec"){
                    modifier = std::make_shared<IncDecModifier>(*this);
                }else{
                    throw MapperException("\"modtype\" parameter is invalid or that parameter is not specified");
                }
                std::string classname = itemdef["class"];
                std::string name = itemdef["name"];
                static const std::map<std::string, FSMDEVUNIT_VALTYPE> classname_dic{
                    {"absolute", FSMDU_TYPE_ABSOLUTE},
                    {"relative", FSMDU_TYPE_RELATIVE},
                    {"binary", FSMDU_TYPE_BINARY},
                };
                if (classname != ""){
                    if (classname_dic.count(classname) > 0){
                        rule.classRule.try_emplace(classname_dic.at(classname), modifier);
                    }else{
                        throw MapperException("The value of \"class\" parameter is invarid. "
                                              "This parameter can be \"binary\", \"absolute\", or \"relative\".");
                    }
                }else if (name != ""){
                    rule.unitRule.try_emplace(std::move(name), modifier);
                }else{
                    throw MapperException("There is no specification what the modifiere apply to. "
                                          "Spcecify \"class\" parameter or \"name\" parameter.");
                }
            }
        }
    }
    return std::move(rule);
}

//============================================================================================
// Deferred event processing mechanism
//============================================================================================
DeviceModifierManager::DeviceModifierManager(MapperEngine &engine) : engine(engine), status(Status::running){
    scheduler = std::thread([this](){
        std::unique_lock lock(mutex);
        while (true){
            auto now = DEVICEMOD_CLOCK::now();
            if (event_queue.size() > 0){
                auto item = event_queue.front();
                event_queue.pop();
                lock.unlock();
                item.modifier.processUnitValueChangeEvent(item.value, now);
                lock.lock();
            }else if (timers.size() > 0){
                auto most_recent = timers.begin();
                if (most_recent->first >= now){
                    auto target_time = most_recent->first;
                    auto& modifier = most_recent->second;
                    timers.erase(most_recent);
                    lock.unlock();
                    modifier.processTimerEvent(target_time);
                    lock.lock();
                }
            }else{
                auto event_queue_size = event_queue.size();
                auto timer_num = timers.size();
                auto condition = [this, event_queue_size, timer_num](){
                    return event_queue.size() > event_queue_size || timers.size() > timer_num;
                };
                if (timer_num > 0){
                    auto now = DEVICEMOD_CLOCK::now();
                    auto target = timers.begin()->first;
                    if (now < target){
                        cv.wait_until(lock, target, condition);
                    }
                }else{
                    cv.wait(lock, condition);
                }
                if (status != Status::running){
                    status = Status::stop;
                    cv.notify_all();
                    break;
                }
            }
        }
    });
}

DeviceModifierManager::~DeviceModifierManager(){
    stop();
    scheduler.join();
}

void DeviceModifierManager::stop(){
    std::lock_guard lock(mutex);
    if (status == Status::running){
        status = Status::stopping;
    }
}

void DeviceModifierManager::delegateEventProcessing(DeviceModifier &modifier, int value){
    std::lock_guard lock(mutex);
    event_queue.push({modifier, value});
    cv.notify_all();
}

DEVICEMOD_TIME DeviceModifierManager::addTimer(DeviceModifier &modifier, DEVICEMOD_TIME at){
    std::lock_guard lock(mutex);
    while (timers.count(at)){
        at += DEVICEMOD_MILLISEC(1);
    }
    timers.emplace(at, modifier);
    cv.notify_all();
    return at;
}

void DeviceModifierManager::cancelTimer(DeviceModifier &modifier, DEVICEMOD_TIME at){
    std::lock_guard lock(mutex);
    timers.erase(at);
    cv.notify_all();
}
