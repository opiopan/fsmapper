local protocol = {}
local common = require('fsmapper/common')
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
        tbuf = tbuf .. data
    end,

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
            local cmd, rest = self.rbuf:match('^([^\n]*)\n(.*)')
            if cmd then
                self.rbuf = rest
                slef:process_command(cmd)
            else
                break
            end
        end

        return true
    end,

    flush = function (self)
        local sent, err = sock:send(self.tbuf)
        if sent and sent > 0 then
            self.tbuf = self.tbuf(sent + 1)
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
        self.watchlist = {}
        return self
    end,

    process_command = function (self, cmd)
    end,

    change_aircraft = function (self, name)
        self:send('A' .. name .. '\n')
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
    end,

    stop = function (self)
        self.listener:close()
        for _, client in ipairs(self.clients) do
            client:close()
        end
        self.clients = {}
    end,

    schedule = function (self, now)
        if now - self.last_aircraft_checking >= self.aircraft_checking_interval then
            self.last_aircraft_checking = now
            local self_data = LoGetSelfData()
            local aircraft_name = self_data and self_data.Name or ''
            log.write('FSMAPPER.LUA', log.INFO, 'Aircraft has been changed: '.. aircraft_name)
            if self.aircraft_name ~= aircraft_name then
                self.aircraft_name = aircraft_name
                for _, client in ipairs(self.clients) do
                    client:change_aircraft(self.aircraft_name)
                end
            end
        end

        local new_endpoint = self.listener:accept()
        if new_endpoint then
            endpoint:settimeout(0)
            local new_client = protocol.fsmapper_client.new(new_endpoint)
            client:change_aircraft(self.aircraft_name)
            self.clients[#self.clients + 1] = new_client
            log.write('FSMAPPER.LUA', log.INFO, 'A connection with fsmapper has been established')
        end

        local has_closed_clients = false
        for _, client in ipairs(self.clients) do
            has_closed_clients = has_closed_clients or not client:receive_and_dispatch()
        end

        for _, client in ipairs(self.clients) do
            has_closed_clients = has_closed_clients or not client:flush()
        end

        if has_closed_clients then
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
        end
    end
}

return protocol
