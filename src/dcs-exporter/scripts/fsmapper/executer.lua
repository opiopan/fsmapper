local common = require('fsmapper/common')

executer = {
    new = function()
        local self = common.instantiate(executer)
        self.chunks = {}
        return self
    end,

    R_cmd_fmt = fsmapper.utils.struct('I4'),
    register_chunk = function(self, cmd_body)
        local id = self.R_cmd_fmt:unpack(cmd_body)
        local text = cmd_body:sub(self.R_cmd_fmt:packsize() + 1)
        if text:len() > 0 then
            local chunk, error = loadstring(text)
            if error then
                log.write('FSMAPPER.LUA', log.ERROR, 'An error occured while parcing a registered chunk: ' .. error)
            else
                self.chunks[id] = chunk
                fsmapper.log("Registered a chunk: id=" .. id)
            end
        end
    end,

    clear_chunk = function(self)
        fsmapper.log("Clear all registered chunk")
        self.chunks = {}
    end,

    execute_chunk = function(self, id, ...)
        local chunk = self.chunks[id]
        if chunk then
            local result, msg = pcall(chunk, ...)
            if not result then
                log.write('FSMAPPER.LUA', log.ERROR, 'An error occured during executing a registered cunk: ' .. msg)
            end
        end
    end,

    T_cmd_fmt = fsmapper.utils.struct('I4'),
    execute_chunk_without_argument = function(self, cmd_body)
        local id = self.T_cmd_fmt:unpack(cmd_body)
        if id then
            self:execute_chunk(id)
        end
    end,

    U_cmd_fmt = fsmapper.utils.struct('I4f'),
    execute_chunk_with_number = function(self, cmd_body)
        local id, argument = self.U_cmd_fmt:unpack(cmd_body)
        if argument then
            self:execute_chunk(id, argument)
        end
    end,

    V_cmd_fmt = fsmapper.utils.struct('I4'),
    execute_chunk_with_string = function(self, cmd_body)
        local id = self.V_cmd_fmt:unpack(cmd_body)
        argument = cmd_body:sub(self.V_cmd_fmt:packsize() + 1)
        if id then
            self:execute_chunk(id, argument)
        end
    end,
}

return executer