local module = {}

function module.print_package_path()
    mapper.print("package.path: " .. package.path)
end

return module
