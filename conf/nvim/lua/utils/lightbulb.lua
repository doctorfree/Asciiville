local fn = vim.fn
local lsp = vim.lsp

local M = {}

-- Credit https://github.com/kosayoda/nvim-lightbulb/blob/master/lua/nvim-lightbulb.lua
local function _update_sign(new_line)
    local bufnr = vim.api.nvim_get_current_buf() or '%'
    local old_line = vim.b.code_action_line
    if old_line then
        fn.sign_unplace('code_action', { id = old_line, buffer = bufnr })

        vim.b.code_action_line = nil
    end

    if new_line and (vim.b.code_action_line ~= new_line) then
        fn.sign_place(new_line, 'code_action', 'CodeActionSign', bufnr, { lnum = new_line, priority = 10 })

        vim.b.code_action_line = new_line
    end
end

-- Credit https://github.com/kosayoda/nvim-lightbulb/blob/master/lua/nvim-lightbulb.lua
function M.code_action()
    local context = { diagnostics = lsp.diagnostic.get_line_diagnostics() }
    local params = lsp.util.make_range_params()

    params.context = context
    lsp.buf_request(0, 'textDocument/codeAction', params, function(err, actions)
        if err then
            return
        end

        if actions == nil or vim.tbl_isempty(actions) then
            _update_sign(nil)
        else
            _update_sign(params.range.start.line + 1)
        end
    end)
end

return M
