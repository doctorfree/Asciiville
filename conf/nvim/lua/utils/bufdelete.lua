local bo = vim.bo
local cmd = vim.cmd
local api = vim.api

local M = {}

function M.bufdelete()
    if bo.modified then
        cmd("write")
    end

    local bufnr = api.nvim_get_current_buf()

    -- NOTE: Get list of valid buffers
    local buffers = vim.tbl_filter(function(buf)
        return bo[buf].buflisted and api.nvim_buf_is_valid(buf)
    end, api.nvim_list_bufs())

    if #buffers == 1 then
        -- NOTE: Hide scrollview before close buffer
        cmd("silent! ScrollViewDisable | bd " .. bufnr .. " | silent! ScrollViewEnable")

        -- NOTE: Show Dashboard when closing the last buffer
        require("dashboard").instance()
    else
        if bufnr ~= buffers[#buffers] then
            cmd("bnext")
        else
            cmd("bprevious")
        end

        -- NOTE: Hide scrollview before close buffer
        cmd("silent! ScrollViewDisable | bd " .. bufnr .. " | silent! ScrollViewEnable")
    end
end

return M
