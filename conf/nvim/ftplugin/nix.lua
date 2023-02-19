local lsp = require('mrcjk.lsp')

vim.lsp.start {
  name = 'nil_ls',
  cmd = { 'nil' },
  root_dir = vim.fs.dirname(vim.fs.find({ 'flake.nix', '.git' }, { upward = true })[1]),
  on_attach = lsp.on_attach,
  capabilities = lsp.capabilities,
}
