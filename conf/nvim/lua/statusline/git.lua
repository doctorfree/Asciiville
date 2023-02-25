local M = {}

-- Credit https://github.com/glepnir/galaxyline.nvim/blob/main/lua/galaxyline/provider_vcs.lua
function M.get_hunks_data()
    -- diff data 1:add 2:modified 3:remove
    local diff_data = { 0, 0, 0 }
    if vim.fn.exists("b:gitsigns_status") == 1 then
        local gitsigns_dict = vim.api.nvim_buf_get_var(0, "gitsigns_status")
        diff_data[1] = tonumber(gitsigns_dict:match("+(%d+)")) or 0
        diff_data[2] = tonumber(gitsigns_dict:match("~(%d+)")) or 0
        diff_data[3] = tonumber(gitsigns_dict:match("-(%d+)")) or 0
    end

    return diff_data
end

function M.line()
    return "%#StatuslineDiffAdded#"
        .. [[ %{luaeval('require("statusline.git").get_hunks_data()[1]')}]]
        .. " %#StatuslineDiffModified#"
        .. [[ %{luaeval('require("statusline.git").get_hunks_data()[2]')}]]
        .. " %#StatuslineDiffRemoved#"
        .. [[ %{luaeval('require("statusline.git").get_hunks_data()[3]')}]]
        .. "  "
end

return M
