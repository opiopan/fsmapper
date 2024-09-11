local observer = {
    observers = {}
}

local common = require('fsmapper/common')

observer.base_observer = {
    new = function (id, epsilon)
        local self = common.instantiate(observer.base_observer)
        self.id = id
        self.epsilon = epsilon or 0
        self.is_enabled = false
        self.is_dirty = true
        return self
    end,

    update = function (self)
        if not self.is_enabled then return false end
        local value = self:reflect_filter(self:raw_value())
        if value ~= nil then
            local value_type = type(value)
            local current_value_type = type(self.value)
            if value_type == 'number' then
                if current_value_type ~= value_type or math.abs(self.value - value) > self.epsilon or self.is_dirty then
                    self.value = value
                    self.is_dirty = true
                end
            elseif value_type == 'string' then
                if current_value_type ~= value_type or self.value ~= value or self.is_dirty then
                    self.value = value
                    self.is_dirty = true
                end
            end
            return self.is_dirty
        end
        return self.is_dirty and self.value ~= nil
    end,

    reflect_filter = function(self, value)
        local filter_type = type(self.filter)
        if (value ~= nil) then
            if filter_type == 'nil' then
                return value
            elseif filter_type == 'table' then
                return self.filter[value] and value
            elseif filter_type == 'function' then
                local result, return_value = pcall(self.filter, value)
                if result then
                    return return_value
                else
                   log.write('FSMAPPER.LUA', log.ERROR, 'An error occured during executing a cunk set as a filter for a observer: ' .. return_value)
                end
            end
        end
    end,
    
    float_msg_fmt = fsmapper.utils.struct('c1I3c1I3f'),
    string_msg_fmt = fsmapper.utils.struct('c1I3:4c1I3S2'),
    generate_notification_message = function (self)
        self.is_dirty = false
        local value_type = type(self.value)
        if value_type == 'number' then
            return self.float_msg_fmt:pack('O', self.float_msg_size, 'N', self.id, self.value)
        elseif value_type == 'string' then
            return self.string_msg_fmt:pack('O', 'S', self.id, self.value)
        end
    end,
}
observer.base_observer.float_msg_size = observer.base_observer.float_msg_fmt:packsize() - 4

observer.argument_value_observer = {
    new = function (id, arg_number, epsilon)
        local self = common.instantiate(observer.argument_value_observer, observer.base_observer, id, epsilon)
        self.arg_number = arg_number
        return self
    end,

    raw_value = function (self)
        return GetDevice(0):get_argument_value(self.arg_number)
    end,
}

observer.indication_text_observer = {
    new = function (id, indicator_id, epsilon)
        local self = common.instantiate(observer.indication_text_observer, observer.base_observer, id, epsilon)
        self.indicator_id = indicator_id
        return self
    end,

    raw_value = function (self)
        return list_indication(self.indicator_id)
    end,
}

observer.chunk_value_observer = {
    new = function (id, chunk_text, epsilon)
        local self = common.instantiate(observer.chunk_value_observer, observer.base_observer, id, epsilon)
        local chunk, error = loadstring(chunk_text)
        if error then
            log.write('FSMAPPER.LUA', log.ERROR, 'An error occured while parcing a chunk to generate observation value: ' .. error)
        else
            self.chunk = common.configure_fenv(chunk)
        end
        return self
    end,

    raw_value = function (self)
        local result, return_value = pcall(self.chunk)
        if result then
            return return_value
        else
            log.write('FSMAPPER.LUA', log.ERROR, 'An error occured during executing a cunk set as a filter for a observer: ' .. error)
        end
    end,
}

observer.observer_list = {
    new = function ()
        local self = common.instantiate(observer.observer_list)
        self.observers ={}
        return self
    end,
    
    O_fmt = fsmapper.utils.struct('c1I3'),
    A_sub_fmt = fsmapper.utils.struct('I4f'),
    I_sub_fmt = fsmapper.utils.struct('I4f'),
    C_sub_fmt = fsmapper.utils.struct('f'),
    F_sub_fmt = fsmapper.utils.struct('f'),
    G_sub_fmt = fsmapper.utils.struct('z'),
    H_sub_fmt = fsmapper.utils.struct('z'),
    manipulate_observer = function (self, cmd)
        local type, id = self.O_fmt:unpack(cmd)
        if type == 'A' then
            local arg_number, epsilon = self.A_sub_fmt:unpack(cmd, self.O_fmt:packsize() + 1)
            if epsilon then
                self.observers[id] = observer.argument_value_observer.new(id, arg_number, epsilon)
                fsmapper.log("Registered an argument value observer for " .. arg_number .. " as id=" .. id)
            end
        elseif type == 'I' then
            local indicator_id, epsilon = self.I_sub_fmt:unpack(cmd, self.O_fmt:packsize() + 1)
            if epsilon then
                self.observers[id] = observer.indication_text_observer.new(id, indicator_id, epsilon)
                fsmapper.log("Registered an indication text observer for " .. indicator_id .. " as id=" .. id)
            end
        elseif type == 'C' then
            local epsilon = self.C_sub_fmt:unpack(cmd, self.O_fmt:packsize() + 1)
            local chunk = cmd:sub(self.O_fmt:packsize() + self.C_sub_fmt:packsize() + 1)
            if chunk:len() > 0 then
                self.observers[id] = observer.chunk_value_observer.new(id, chunk, epsilon)
                fsmapper.log("Registered a chunk value observer as id=" .. id)
            end
        elseif type == 'F' then
            local value = self.F_sub_fmt:unpack(cmd, self.O_fmt:packsize() + 1)
            if value then
                local ob = self.observers[id]
                if ob then
                    ob.filter = ob.filter or {}
                    ob.filter[value] = true
                    fsmapper.log("Added a numeric filter to the observer: id=" .. id .. " value=" .. value)
                end
            end
        elseif type == 'G' then
            local value = self.G_sub_fmt:unpack(cmd, self.O_fmt:packsize() + 1)
            if value then
                local ob = ob.observers[id]
                if ob then
                    ob.filter = self.filter or {}
                    ob.filter[value] = true
                    fsmapper.log("Added a string filter to the observer: id=" .. id .. " value=" .. value)
                end
            end
        elseif type == 'H' then
            local value = self.H_sub_fmt:unpack(cmd, self.O_fmt:packsize() + 1)
            if value then
                local ob = self.observers[id]
                if ob then
                    local chunk, error = loadstring(value)
                    if error then
                        log.write('FSMAPPER.LUA', log.ERROR, 'An error occured while parcing a chunk for observer filter: ' .. error)
                        ob.filter = {}
                    else
                        ob.filter= common.configure_fenv(chunk)
                        fsmapper.log("Added a chunk filter to the observer: id=" .. id)
                    end
                end
            end
        elseif type == 'E' then
            local ob = self.observers[id]
            if ob then
                ob.is_enabled = true
                ob:update()
                fsmapper.log("Enabled an observer: id=" .. id)
            end
        else
            fsmapper.log(string.format('Unknown observer manipulation subcommand: subcmd=%q id=%q', type, id))
        end
    end,

    clear = function (self)
        self.observers = {}
        fsmapper.log("Cleared all observer")
    end,

    refresh = function (self, now, connection)
        for _, ob in pairs(self.observers) do
            if ob:update() then
                connection:send(ob:generate_notification_message())
                fsmapper.log("Send observed data: id=" .. ob.id)
            end
        end
    end,
}

return observer