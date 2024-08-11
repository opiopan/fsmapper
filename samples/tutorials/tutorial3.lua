device = mapper.device{
    name = 'Tutorial',
    type = 'dinput',
    -----------------------------------------------------------------
    -- Specify the 'identifier' in one of the formats below according
    -- to your environment as follwing.
    --   1: identifier = {name = 'Product Name'}
    --   2: identifier = {guid = '{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}'}
    identifier = {index = 1},
    -----------------------------------------------------------------
    modifiers = {
        {class = 'binary', modtype = 'button'},
    }
}
local events = device:get_events()

mapper.set_primary_mappings{
    {
        event = events.button1.down,
        action = function ()
            msfs.mfwasm.execute_rpn('0 (A:LIGHT LANDING:0, Bool) ! (>K:2:LANDING_LIGHTS_SET)')
        end
    },
    {
        event = events.button2.down,
        action = msfs.mfwasm.rpn_executer('0 (A:LIGHT STROBE:0, Bool) ! (>K:2:STROBES_SET)')
    },
}