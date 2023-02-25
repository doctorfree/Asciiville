local M = {}

local settings = require("statusline.settings")
local setting_languages = require("languages.languages")
local filetypes = settings.efm.filetypes

local formatters = {}
local default_formatter = {}
for _, filetype in pairs(filetypes) do
    formatters[filetype] = setting_languages[filetype].all_format
    default_formatter[filetype] = setting_languages[filetype].default_format
end

M.default_formatter = default_formatter

M.choose_formatter = function()
    local fileType = vim.bo.filetype
    local all_formatters = formatters[fileType]
    local store_formatters = {}

    local count = 0
    for key, _ in pairs(all_formatters) do
        count = count + 1
        store_formatters[#store_formatters + 1] = key
        print("[" .. count .. "] " .. key)
    end

    if count > 1 then
        local option = vim.fn.input("Choose your formmatter: ")

        M.default_formatter[fileType] = store_formatters[tonumber(option)]
    end
end

M.formatter_status = function()
    local fileType = vim.bo.filetype
    if M.default_formatter[fileType] then
        local name = formatters[fileType][M.default_formatter[fileType]]
        if name ~= "" then
            return "   " .. name
        end
    end

    return ""
end

M.format = function()
    local fileType = vim.bo.filetype
    local code_formatter = M.default_formatter[fileType]

    vim.lsp.buf.format({
        filter = function(client)
            return client.name == code_formatter
        end,

        -- Ormolu (formatter of Haskell) is too slow
        timeout_ms = 2000,
    })
end

M.range_format = function()
    local vim_mode = vim.api.nvim_eval("mode()")

    if vim_mode == "v" then
        local start_position = vim.api.nvim_eval('getpos("v")')
        local end_position = vim.api.nvim_eval('getpos(".")')

        vim.lsp.buf.range_formatting({}, { start_position[2], 0 }, { end_position[2], 0 })
    end
end

return M
