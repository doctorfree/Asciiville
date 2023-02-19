local lsp = require('mrcjk.lsp')
local jdtls = require('jdtls')

local on_jdtls_attach = function(client, bufnr)
  lsp.on_attach(client, bufnr)
  -- With `hotcodereplace = 'auto' the debug adapter will try to apply code changes
  -- you make during a debug session immediately.
  -- You can use the `JdtHotcodeReplace` command to trigger it manually
  -- jdtls.setup_dap {
  --   hotcodereplace = 'auto',
  -- }
  jdtls.setup.add_commands()
  local opts = { noremap = true, silent = true }
  vim.keymap.set('n', '<A-o>', jdtls.organize_imports, opts)
  vim.keymap.set('n', '<A-v>', jdtls.extract_variable, opts)
  vim.keymap.set('v', '<A-v>', function()
    jdtls.extract_variable(true)
  end, opts)
  vim.keymap.set('v', '<A-m>', function()
    jdtls.extract_method(true)
  end, opts)
  -- nvim-dap (requires java-debug and vscode-java-test bundles)
  -- vim.keymap.set('n', '<leader>df', jdtls.test_class, opts)
  -- vim.keymap.set('n', '<leader>dn', jdtls.test_nearest_method, opts)
end

-- XXX: Duplicate.
-- TODO: Extract to lsp module.
local capabilities = require('cmp_nvim_lsp').default_capabilities()
capabilities = require('lsp-selection-range').update_capabilities(capabilities)

local root_dir = require('jdtls.setup').find_root { 'build.gradle', 'gradlew', 'mvnw', '.classpath' }
local workspace_folder = vim.fn.stdpath('data') .. '/.workspace/' .. vim.fn.fnamemodify(root_dir, ':p:h:t')

local settings = {
  java = {
    signatureHelp = { enabled = true },
    contentProvider = { preferred = 'fernflower' },
    completion = {
      favoriteStaticMembers = {
        'org.junit.jupiter.api.Assertions.*',
        'org.mockito.Mockito.*',
        'io.vavr.API.$',
        'io.vavr.API.Case',
        'io.vavr.API.Match',
        'io.vavr.API.For',
        'io.vavr.Predicates.not',
      },
    },
    sources = {
      organizeImports = {
        starThreshold = 9999,
        staticStarThreshold = 9999,
      },
    },
    codeGeneration = {
      toString = {
        template = '${object.className}{${member.name()}=${member.value}, ${otherMembers}}',
      },
    },
  },
}

jdtls.start_or_attach {
  capabilities = capabilities,
  cmd = { 'jdt-language-server', '-configuration', '-data', workspace_folder },
  settings = settings,
  on_attach = on_jdtls_attach,
  on_init = function(client, _)
    client.notify('workspace/didChangeConfiguration', { settings = settings })
  end,
}
