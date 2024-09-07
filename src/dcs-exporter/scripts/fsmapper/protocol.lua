local protocol = {}
local common = require('fsmapper/common')
local observer = require('fsmapper/observer')
local socket = require('socket')

protocol.connection = {
    new = function (sock)
        local self = common.instantiate(protocol.connection)
        self.is_enabled = true
        self.sock = sock
        self.rbuf = ''
        self.tbuf = ''
        return self
    end,

    close = function (self)
        socket.try(self.sock:close())
        self.is_enabled = false
        self.rbuf = ''
        self.tbuf = ''
        self.watchlist = {}
    end,

    send = function (self, data)
        self.tbuf = self.tbuf .. data
    end,

    rcv_fmt = fsmapper.utils.struct('c1s3'),
    receive_and_dispatch = function (self)
        local data, err, partial = self.sock:receive(4096)
        if data then
            self.rbuf = self.rbuf .. data
        elseif partial and #partial > 0 then
            self.rbuf = self.rbuf .. partial
        elseif err == 'closed' then
            self.is_enabled = false
            return false
        end

        while true do
            local cmd, body = self.rcv_fmt:unpack(self.rbuf)
            if body then
                self.rbuf = self.rbuf:sub(body:len() + 5)
                self:process_command(cmd, body)
            else
                break
            end
        end

        return true
    end,

    flush = function (self)
        local sent, err = self.sock:send(self.tbuf)
        if sent and sent > 0 then
            self.tbuf = self.tbuf:sub(sent + 1)
        elseif err == 'closed' then
            self.is_enabled = false
            return false
        end
        return true
    end,
}

protocol.fsmapper_client = {
    new = function (sock)
        local self = common.instantiate(protocol.fsmapper_client, protocol.connection, sock)
        self.observers = observer.observer_list.new()
        return self
    end,

    process_command = function (self, cmd, body)
        local processor = self[cmd]
        if processor then
            result, msg = pcall(processor, self, body)
            if not result then
                log.write('FSMAPPER.LUA', log.ERROR, 'Processing a request packet has failed: ' .. msg)
            end
        else
            fsmapper.log("unsuported command: " .. cmd)
        end
    end,

    rcv_p_fmt = fsmapper.utils.struct('I4I4'),
    float_fmt = fsmapper.utils.struct('f'),
    P = function (self, body)
        local device, command = self.rcv_p_fmt:unpack(body)
        for offset = self.rcv_p_fmt:packsize() + 1, body:len(), self.float_fmt:packsize() do
            print('offseet: '..offset)
            local value = self.float_fmt:unpack(body, offset)
            GetDevice(device):performClickableAction(command, value)
        end
    end,

    O = function (self, body)
        self.observers:manipulate_observer(body)
    end,

    C = function (self, body)
        self.observers:clear()
    end,

    refresh_observers = function (self, now)
        self.observers:refresh(now, self)
    end,

    version_cmd_fmt = fsmapper.utils.struct('c1I3 i4i4i4i4 s4'),
    inform_version = function (self, version)
        local msg = self.version_cmd_fmt:pack(
            'V', self.version_cmd_fmt:packsize() + version.ProductName:len() - 4,
            version.ProductVersion[1],
            version.ProductVersion[2],
            version.ProductVersion[3],
            version.ProductVersion[4],
            version.ProductName)
        self:send(msg)
    end,

    aircraft_cmd_fmt = fsmapper.utils.struct('c1s3'),
    change_aircraft = function (self, name)
        local msg = self.aircraft_cmd_fmt:pack('A', name)
        self:send(msg)
    end,
}

protocol.server = {
    new = function (args)
        local self = common.instantiate(protocol.server)
        args = args or {}
        self.tcp_host = args.tcp_host or 'localhost'
        self.tcp_port = args.tcp_port or 8544
        self.udp_host = args.udp_host or 'localhost'
        self.udp_port = args.udp_port or 8544
        self.aircraft_checking_interval = args.aircraft_checking_interval or 1
        self.clients = {}
        self.aircraft_name = ''
        self.last_aircraft_checking = -100
        return self
    end,

    start = function (self)
        self.listener = socket.tcp()
        socket.try(self.listener:bind('localhost', fsmapper.config.tcp_port))
        self.listener:settimeout(0)
        self.listener:listen(1)
        self.version_info =  LoGetVersionInfo()
    end,

    stop = function (self)
        self.listener:close()
        for _, client in ipairs(self.clients) do
            client:close()
        end
        self.clients = {}
    end,
    
    schedule_before_frame = function(self, now)
        local new_endpoint = self.listener:accept()
        if new_endpoint then
            new_endpoint:settimeout(0)
            local new_client = protocol.fsmapper_client.new(new_endpoint)
            new_client:inform_version(self.version_info)
            new_client:change_aircraft(self.aircraft_name)
            self.clients[#self.clients + 1] = new_client
            log.write('FSMAPPER.LUA', log.INFO, 'A connection with fsmapper has been established')
        end

        local has_closed_clients = false
        for _, client in ipairs(self.clients) do
            has_closed_clients = not client:receive_and_dispatch() or has_closed_clients
        end

        if has_closed_clients then
            self:cleanup_closed_clients()
        end
    end,

    schedule_after_frame = function (self, now)
        if now - self.last_aircraft_checking >= self.aircraft_checking_interval then
            self.last_aircraft_checking = now
            local self_data = LoGetSelfData()
            local aircraft_name = self_data and self_data.Name or ''
            if self.aircraft_name ~= aircraft_name then
                log.write('FSMAPPER.LUA', log.INFO, 'Aircraft has been changed: '.. aircraft_name)
                self.aircraft_name = aircraft_name
                for _, client in ipairs(self.clients) do
                    client:change_aircraft(self.aircraft_name)
                end
            end
        end

        for _, client in ipairs(self.clients) do
            client:refresh_observers(now)
        end

        local has_closed_clients = false
        for _, client in ipairs(self.clients) do
            has_closed_clients = not client:flush() or has_closed_clients
        end

        if has_closed_clients then
            self:cleanup_closed_clients()
        end
    end,

    cleanup_closed_clients = function(self)
        local new_clients = {}
        for _, client in ipairs(self.clients) do
            if client.is_enabled then
                new_clients[#new_clients + 1] = client
            else
            log.write('FSMAPPER.LUA', log.INFO, 'A connection with fsmapper has been lost')
                client:close()
            end
        end
        self.clients = new_clients
    end,
}    

return protocol
