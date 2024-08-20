local common = {}

function common.instantiate(class, super, ...)
    local self = (super and super.new(...) or {})
    setmetatable(self, {__index = class})
    setmetatable(class, {__index = super})
    return self
end

function common.split(str, delimiter)
    local result = {}
    for chunk in str:gmatch('([^' .. delimiter .. ']+)') do
        result[#result + 1] = chunk
    end
    return result
end

return common
