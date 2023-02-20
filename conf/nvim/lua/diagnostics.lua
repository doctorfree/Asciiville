local sign = function(opts)
  vim.fn.sign_define(opts.name, {
    texthl = opts.name,
    text = opts.text,
    numhl = ''
  })
end

sign({name = 'DiagnosticSignError', text = "✘"})
sign({name = 'DiagnosticSignWarn', text = ""})
sign({name = 'DiagnosticSignHint', text = "•"})
sign({name = 'DiagnosticSignInfo', text = ""})

vim.diagnostic.config({
  virtual_text = false,
  update_in_insert = false,
  signs = true,
  float = {
    focusable = false,
    style = "minimal",
    header = "",
    prefix = "",
    border = "rounded",
    format = function(diagnostic)
      return string.format(
        "%s (%s)",
        diagnostic.message,
        diagnostic.source
      )
    end,
  },
})

-- Show line diagnostics automatically in hover window
vim.cmd([[
  autocmd! CursorHold,CursorHoldI * lua vim.diagnostic.open_float(nil, { focus = false })
]])

-- Mappings.
-- See `:help vim.diagnostic.*` for documentation on any of the below functions
local opts = { noremap=true, silent=true }
vim.keymap.set('n', '<space>e', vim.diagnostic.open_float, opts)
vim.keymap.set('n', '[d', vim.diagnostic.goto_prev, opts)
vim.keymap.set('n', ']d', vim.diagnostic.goto_next, opts)
vim.keymap.set('n', '<space>q', vim.diagnostic.setloclist, opts)
