-- fsmpapper.lua : Export script for connecting DCS with fsmapper
-- 
--   When fsmapper starts, the following line is inserted into export.lua, so that this file is loaded when DCS World runs.
--     fsmapper={};fsmapper.basedir='BASE_DIR_OF_FSMAPPER';dofile(fsmapper.basedir..'dcs-exporter/fsmapper.lua')

if is_fsmapper_exporter_initialized ~= true then
    is_fsmapper_exporter_initialized = true

    fsmapper.scriptdir = fsmapper.basedir..'dcs-exporter/'
    fsmapper.scriptpath = fsmapper.scriptdir..'fsmapper.lua'
    package.path = package.path .. ';' .. fsmapper.scriptdir .. '?.lua;'

    log.write('FSMAPPER.LUA',log.INFO,'Starting ['..fsmapper.scriptpath..']')

    --===========================================================================================
    -- Hook called before mission start
    --===========================================================================================
    local prev_export_start = LuaExportStart
    LuaExportStart = function()
        local self_data = LoGetSelfData()
        fsmapper.aircraft = self_data.Name and self_data.Name or ''

        log.write('FSMAPPER.LUA', log.INFO, 'Start a mission with '..fsmapper.aircraft)

        if prev_export_start then
            prev_export_start()
        end
    end

    --===========================================================================================
    -- Hook called after mission start
    --===========================================================================================
    local prev_export_stop = LuaExportStop
    LuaExportStop = function()
        log.write('FSMAPPER.LUA',log.INFO,'Stop a mission')

        if prev_export_stop then
            prev_export_stop()
        end
    end

    --===========================================================================================
    -- Hook called before every frame
    --===========================================================================================
    local prev_export_before_frame = LuaExportBeforeNextFrame
    LuaExportBeforeNextFrame = function()
        if prev_export_before_frame then
            prev_export_before_frame()
        end
    end

    --===========================================================================================
    -- Hook called before every frame
    --===========================================================================================
    local prev_export_after_frame = LuaExportAfterNextFrame
    LuaExportBeforeNextFrame = function()
        if prev_export_after_frame then
            prev_export_after_frame()
        end
    end
end
