local fn = vim.fn
local settings = require('settings')

local signs = settings.signs
local kind_icons = settings.kinds

for type, icon in pairs(signs) do
    fn.sign_define(type, { text = icon, texthl = type, numhl = '' })
end

-- Credit https://github.com/neovim/nvim-lspconfig/wiki/UI-customization
local kinds = vim.lsp.protocol.CompletionItemKind
for i, kind in ipairs(kinds) do
    kinds[i] = kind_icons[kind] or kind
end
