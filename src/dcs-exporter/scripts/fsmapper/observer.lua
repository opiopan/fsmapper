local observer = {}
local common = require('fsmapper/common')

observer.argument_value_observer = {
    new = function (param)
        local self = common.instantiate(observer.argument_value_observer)
        local args = common.split(param, ':')
        self.id = tonumber(args[1])
        self.arg_number = tonumber(args[2])
        self.epsilon = tonumber(args[3])
        self.is_dirty = true;
        self.value = GetDevice(0):get_argument_value(self.arg_number)
        return self
    end,

    update = function (self)
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


return observer