local M = {}

function M.file_supported(file_extension)
    return file_extension == "java" or file_extension == "md"
end

local function java_name(filename)
    return string.sub(filename, 1, #filename - 5)
end

local function slice(tbl, s, e)
    local new_tbl = {}

    for i = s, e do
        table.insert(new_tbl, tbl[i])
    end

    return new_tbl
end

local function java_package_name(path, filename)
    local t = {}
    local find_java = false
    local position = 0
    for str in string.gmatch(path, "([^/]+)") do
        if str == "java" then
            find_java = true
            position = #t + 1
        end

        if str ~= filename then
            table.insert(t, str)
        end
    end

    if find_java then
        local new_tbl = slice(t, position + 1, #t)
        return table.concat(new_tbl, ".")
    end

    return t[#t]
end

local expr = {
    "{{_java_package_name_}}",
    "{{_cursor_}}",
    "{{_java_name}}",
}

local expand = {
    [expr[1]] = function(ctx)
        local package_name = java_package_name(ctx.path, ctx.filename)
        return ctx.line:gsub(expr[1], package_name)
    end,
    [expr[2]] = function(ctx)
        return ctx.line:gsub(expr[2], "")
    end,
    [expr[3]] = function(ctx)
        local package_name = java_name(ctx.filename)
        return ctx.line:gsub(expr[1], package_name)
    end,
}

function M.generate(file_extension, filename, path, file_type)
    if file_extension == "md" and filename == "README.md" then
        return require("languages.markdown").template[file_type]
    elseif file_extension == "java" then
        local template = require("languages.java").template[file_type]
        local name = java_name(filename)
        local group = java_package_name(path, filename)

        return string.format(template, group, name)
    else
        return ""
    end
end

return M
