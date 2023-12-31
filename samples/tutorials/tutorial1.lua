local my_event = mapper.register_event('My Event')
mapper.set_primary_mappings{
    {
        event = my_event,
        action = function (evid, value)
            mapper.print('Value: ' .. value)
            mapper.delay(2000, function ()
                mapper.raise_event(my_event, value + 1)
            end)
        end
    }
}
mapper.raise_event(my_event, 1)
