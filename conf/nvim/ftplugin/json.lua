local lsp = require('doctorfree.lsp')

--Enable (broadcasting) snippet capability for completion
local capabilities = vim.lsp.protocol.make_client_capabilities()
capabilities.textDocument.completion.completionItem.snippetSupport = true

require('lspconfig').jsonls.setup {
  capabilities = capabilities,
}

vim.lsp.start {
  name = 'jsonls',
  cmd = { "vscode-json-language-server", "--stdio" },
  on_attach = lsp.on_attach,
  capabilities = lsp.capabilities,
  init_options = {
    diagnostic = { enable = false },
  }
}
