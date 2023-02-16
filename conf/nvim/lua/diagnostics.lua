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
--  virtual_text = false,
  virtual_text = {
    prefix = '●', -- Could be '●', '▎', 'x'
  },
  severity_sort = true,
  float = {
    border = 'rounded',
    source = 'always',
  },
})

-- Alternately

-- local signs = { Error = "✘", Warn = "", Hint = "•", Info = "" }

-- for type, icon in pairs(signs) do
--     local hl = "DiagnosticSign" .. type
--     vim.fn.sign_define(hl, { text = icon, texthl = hl, numhl = hl })
-- end

-- vim.diagnostic.config({
--     virtual_text = {
--         prefix = '●', -- Could be '●', '▎', 'x'
--     }
-- })

