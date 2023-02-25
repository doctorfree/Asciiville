local api = vim.api

local M = {}

function M.on_attach(client, bufnr)
    local opts = { noremap = true, silent = true }

    if client.server_capabilities.documentSymbolProvider then
        require("nvim-navic").attach(client, bufnr)
    end

    -- Autocomplete signature hints
    require "lsp_signature".on_attach({
        bind = true, -- This is mandatory, otherwise border config won't get registered.
        handler_opts = {
          border = "rounded"
        }
    })

    local function buf_set_keymap(mode, mapping, command)
        api.nvim_buf_set_keymap(bufnr, mode, mapping, command, opts)
    end

    if vim.bo.filetype == "zig" then
        client.resolved_capabilities.code_action = true
    end

    buf_set_keymap("n", "<Leader>d", "<Cmd>lua vim.lsp.buf.definition()<CR>")
    buf_set_keymap("n", "<Leader>a", "<Cmd>lua vim.lsp.buf.code_action()<CR>")
    buf_set_keymap("n", "<Leader><Leader>", "<Cmd>lua vim.lsp.buf.hover()<CR>")
    buf_set_keymap("n", "<Leader>r", '<cmd>lua require("utils.core").rename_popup()<CR>')
    buf_set_keymap("n", "[d", "<cmd>lua vim.diagnostic.goto_prev()<CR>")
    buf_set_keymap("n", "]d", "<cmd>lua vim.diagnostic.goto_next()<CR>")

    buf_set_keymap("n", "<M-x>", [[<Cmd>lua require('sidebar').toggle('symbol')<CR>]])
    buf_set_keymap("n", "[x", "<cmd>AerialPrev<CR>")
    buf_set_keymap("n", "]x", "<cmd>AerialNext<CR>")
    -- Signature toggle
    buf_set_keymap("n", "<C-k>", function()
        require('lsp_signature').toggle_float_win()
        end)
    buf_set_keymap("n", "<Leader>k", function()
        vim.lsp.buf.signature_help()
        end)
end

local capabilities = vim.lsp.protocol.make_client_capabilities()
-- capabilities = require("cmp_nvim_lsp").update_capabilities(capabilities)
capabilities = require('lsp-selection-range').update_capabilities(capabilities)

M.capabilities = capabilities

return M
