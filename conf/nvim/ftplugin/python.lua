local lsp = require('mrcjk.lsp')

-- local dap_python = require('dap-python')
local on_pylsp_attach = function(client, bufnr)
  lsp.on_attach(client, bufnr)
  lsp.on_dap_attach(bufnr)
  -- local opts = { noremap=true, silent=true }
  -- vim.keymap.set('n', '<leader>dn', dap_python.test_method, opts)
  -- vim.keymap.set('n', '<leader>df', dap_python.test_class, opts)
  -- vim.keymap.set('v', '<leader>ds', dap_python.debug_selection, opts)
end

local config = {
  cmd = { 'pylsp' },
  root_dir = vim.fs.dirname(vim.fs.find({ '.git', 'setup.py', 'setup.cfg', 'pyproject.toml' }, { upward = true })[1]),
  on_attach = on_pylsp_attach,
  capabilities = lsp.capabilities,
  settings = {
    pylsp = {
      plugins = {
        flake8 = { enabled = true },
        pycodestyle = { enabled = false },
        pyflakes = { enabled = false },
        pylint = { enabled = false },
        mccabe = { enabled = false },
      },
    },
  },
}

vim.lsp.start(config, {
  reuse_client = function(client, conf)
    return client.name == conf.name and client.config.root_dir == conf.root_dir
  end,
})
