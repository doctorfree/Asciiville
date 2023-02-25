local api = vim.api
local fn = vim.fn

local M = {}

function M.navic()
    return require("nvim-navic").get_location()
end

-- Credit https://github.com/glepnir/galaxyline.nvim/blob/main/lua/galaxyline/provider_vcs.lua
function M.get_hunks_data()
    -- diff data 1:add 2:modified 3:remove
    local diff_data = { 0, 0, 0 }
    if fn.exists("b:gitsigns_status") == 1 then
        local gitsigns_dict = api.nvim_buf_get_var(0, "gitsigns_status")
        diff_data[1] = tonumber(gitsigns_dict:match("+(%d+)")) or 0
        diff_data[2] = tonumber(gitsigns_dict:match("~(%d+)")) or 0
        diff_data[3] = tonumber(gitsigns_dict:match("-(%d+)")) or 0
    end

    return diff_data
end

local async_load = vim.loop.new_async(vim.schedule_wrap(function()
    local line
    local file_type = vim.bo.filetype

    if fn.winwidth(0) > 30 then
        line = ""

        -- Left of status line
        if file_type ~= "Help" then
            line = line .. [[%{luaeval('require("winbar").navic()')}]]
        end
    else
        line = ""
    end

    vim.wo.winbar = line
end))

local supported_filetype = { "lua", "typescript", "javascript", "markdown" }

function M.load()
    local filetype = vim.bo.filetype
    if vim.tbl_contains(supported_filetype, filetype) then
        async_load:send()
    else
        vim.wo.winbar = ""
    end
end

return M
