local lsp = require('mrcjk.lsp')

require('neodev').setup {
  override = function(root_dir, library)
    local util = require('neodev.util')
    if util.has_file(root_dir, '/etc/nixos') or util.has_file(root_dir, 'nvim-config') then
      library.enabled = true
      library.plugins = true
    end
  end,
  lspconfig = false,
}

local root_files = {
  '.git',
  '.luarc.json',
  '.luarc.jsonc',
  '.luacheckrc',
  '.stylua.toml',
  'stylua.toml',
  'selene.toml',
  'selene.yml',
}

local runtime_paths = vim.api.nvim_get_runtime_file('', true)
local library_paths = vim.tbl_filter(function(path)
  return vim.endswith(path, 'neotest') or vim.endswith(path, 'plenary.nvim') or vim.endswith(path, 'telescope.nvim')
end, runtime_paths)

vim.lsp.start {
  name = 'luals',
  cmd = { 'lua-language-server' },
  root_dir = vim.fs.dirname(vim.fs.find(root_files, { upward = true })[1]),
  on_attach = lsp.on_attach,
  capabilities = lsp.capabilities,
  before_init = require('neodev.lsp').before_init,
  settings = {
    Lua = {
      runtime = {
        version = 'LuaJIT',
      },
      diagnostics = {
        -- Get the language server to recognize the `vim` global
        globals = {
          'vim',
          'describe',
          'it',
          'assert',
        },
      },
      workspace = {
        -- Make the server aware of Neovim runtime files
        -- TODO: Refine to library plugins and neovim API
        library = library_paths,
      },
      -- Do not send telemetry data containing a randomized but unique identifier
      telemetry = {
        enable = false,
      },
    },
  },
}
