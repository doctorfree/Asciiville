local lsp = require('mrcjk.lsp')
local ht = require('haskell-tools')

lsp.start_or_attach_haskell_tools()

local bufnr = vim.api.nvim_get_current_buf()
local keymap_opts = { noremap = true, silent = true, buffer = bufnr }
vim.keymap.set('n', '<leader>rr', ht.repl.toggle, keymap_opts)
vim.keymap.set('n', '<leader>rl', ht.repl.reload, keymap_opts)
vim.keymap.set('n', '<leader>rf', function()
  ht.repl.toggle(vim.api.nvim_buf_get_name(bufnr))
end, keymap_opts)
vim.keymap.set('n', '<leader>rq', ht.repl.quit, keymap_opts)
vim.keymap.set('n', '<leader>rp', ht.repl.paste, keymap_opts)
vim.keymap.set('n', '<leader>rt', ht.repl.paste_type, keymap_opts)
vim.keymap.set('n', '<leader>rw', ht.repl.cword_type, keymap_opts)
vim.keymap.set('n', '<space>gp', ht.project.telescope_package_grep, keymap_opts)
vim.keymap.set('n', '<space>gf', ht.project.telescope_package_files, keymap_opts)
-- TODO: remove when ambiguous target issue is resolved
vim.keymap.set('n', '<leader>tt', '<cmd>TermExec cmd="cabal v2-repl %"<CR>', keymap_opts)
