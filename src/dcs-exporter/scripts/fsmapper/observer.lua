local observer = {
    observers = {}
}

local common = require('fsmapper/common')

observer.argument_value_observer = {
    new = function (id, arg_number, epsilon)
        local self = common.instantiate(observer.argument_value_observer)
        self.id = id
        self.is_enabled = false
        self.arg_number = arg_number
        self.epsilon = epsilon
        self.is_dirty = true
        self.value = GetDevice(0):get_argument_value(self.arg_number)
        return self
    end,

    update = function (self)
        if not self.is_enabled then return false end
        local value = GetDevice(0):get_argument_value(self.arg_number)
        if math.abs(self.value - value) > self.epsilon or self.is_dirty then
            self.value = value
            self.is_dirty = true
        end
        return self.is_dirty
    end,

    message_fmt = fsmapper.utils.struct('c1I3c1I3f'),
    generate_notification_message = function (self)
        self.is_dirty = false
        return self.message_fmt:pack('O', self.message_size, 'N', self.id, self.value)
    end,
}
observer.argument_value_observer.message_size = observer.argument_value_observer.message_fmt:packsize() - 4

local O_fmt = fsmapper.utils.struct('c1I3')
local A_sub_fmt = fsmapper.utils.struct('I4f')
function observer:manipulate_observer(cmd)
    local type, id = O_fmt:unpack(cmd)
    if type == 'A' then
        local arg_number, epsilon = A_sub_fmt:unpack(cmd, O_fmt:packsize())
        if epsilon then
            self.observers[id] = self.argument_value_observer.new(id, arg_number, epsilon)
            fsmapper.log("Registered an argument value observer for " .. arg_number .. " as id=" .. id)
        end
    elseif type == 'E' then
        local ob = self.observers[id]
        if ob then
            ob.is_enabled = true
            fsmapper.log("Enabled an observer: id=" .. id)
        end
    end
end

function observer:clear()
    self.observers = {}
    fsmapper.log("Cleared all observer")
end

function observer:refresh(now, connection)
    for _, ob in pairs(self.observers) do
        if ob:update() then
            connection:send(ob:generate_notification_message())
            fsmapper.log("Send observed data for " .. ob.arg_number)
        end
    end
end

return observer