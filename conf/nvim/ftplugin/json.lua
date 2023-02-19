local lsp = require('mrcjk.lsp')

vim.lsp.start {
  name = 'jsonls',
  cmd = { 'json-languageserver', '--stdio' },
  on_attach = lsp.on_attach,
  capabilities = lsp.capabilities,
}
