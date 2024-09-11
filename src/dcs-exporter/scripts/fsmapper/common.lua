local common = {}

function common.instantiate(class, super, ...)
    local self = (super and super.new(...) or {})
    setmetatable(self, {__index = class})
    setmetatable(class, {__index = super})
    return self
end

local chunk_env = {fsmapper = {}}
setmetatable(chunk_env, {__index = getfenv(1)})
function common.configure_fenv(func)
    env = {fsmapper_local = {}}
    setmetatable(env, {__index = chunk_env})
    setfenv(func, env)
    return func
end

function common.split(str, delimiter)
    local result = {}
    for chunk in str:gmatch('([^' .. delimiter .. ']+)') do
        result[#result + 1] = chunk
    end
    return result
end

return common
