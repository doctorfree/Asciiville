local lsp = {}

local api = vim.api
local keymap = vim.keymap
local dap = require('dap')
local dap_widgets = require('dap.ui.widgets')
local dap_utils = require('dap.utils')
local dapui = require('dapui')

local inlayhints = require('inlay-hints')
inlayhints.setup()

local illuminate = require('illuminate')
illuminate.configure {
  delay = 200,
}

require('fidget').setup()

local default_on_codelens = vim.lsp.codelens.on_codelens
vim.lsp.codelens.on_codelens = function(err, lenses, ctx, _)
  if err or not lenses or not next(lenses) then
    return default_on_codelens(err, lenses, ctx, _)
  end
  for _, lens in pairs(lenses) do
    if lens and lens.command and lens.command.title then
      lens.command.title = ' ' .. lens.command.title
    end
  end
  return default_on_codelens(err, lenses, ctx, _)
end

function lsp.on_attach(client, bufnr)
  api.nvim_command('setlocal signcolumn=yes')

  if client.server_capabilities.documentSymbolProvider then
    require("nvim-navic").attach(client, bufnr)
  end

  -- Autocomplete signature hints
  require "lsp_signature".on_attach({
    bind = true, -- This is mandatory, otherwise border config won't get registered.
    handler_opts = {
      border = "rounded"
    }
  })

  local function buf_set_option(...)
    api.nvim_buf_set_option(bufnr, ...)
  end
  local function buf_set_var(...)
    api.nvim_buf_set_var(bufnr, ...)
  end

  -- Enable completion triggered by <c-x><c-o>
  buf_set_option('omnifunc', 'v:lua.vim.lsp.omnifunc')
  buf_set_option('bufhidden', 'hide')
  buf_set_var('lsp_client_id', client.id)

  -- Mappings.
  local opts = { noremap = true, silent = true, buffer = bufnr }
  keymap.set('n', 'gD', vim.lsp.buf.declaration, opts)
  keymap.set('n', 'gd', vim.lsp.buf.definition, opts)
  keymap.set('n', '<space>gt', vim.lsp.buf.type_definition, opts)
  -- keymap.set('n', 'K', vim.lsp.buf.hover, opts) -- overriden by nvim-ufo
  keymap.set('n', 'gi', vim.lsp.buf.implementation, opts)
  -- keymap.set('n', '<C-k>', vim.lsp.buf.signature_help, opts)
  keymap.set('n', '<space>wa', vim.lsp.buf.add_workspace_folder, opts)
  keymap.set('n', '<space>wr', vim.lsp.buf.remove_workspace_folder, opts)
  keymap.set('n', '<space>wl', function()
    vim.pretty_print(vim.lsp.buf.list_workspace_folders())
  end, opts)
  keymap.set('n', '<space>rn', vim.lsp.buf.rename, opts)
  keymap.set('n', '<space>o', vim.lsp.buf.workspace_symbol, opts)
  keymap.set('n', '<space>d', vim.lsp.buf.document_symbol, opts)
  keymap.set('n', '<M-CR>', vim.lsp.buf.code_action, opts)
  keymap.set('n', '<M-l>', vim.lsp.codelens.run, opts)
  keymap.set('n', '<space>cr', vim.lsp.codelens.refresh, opts)
  keymap.set('n', 'gr', vim.lsp.buf.references, opts)
  keymap.set('n', '<space>f', function()
    vim.lsp.buf.format { async = true }
  end, opts)
  keymap.set('n', 'vv', function()
    require('lsp-selection-range').trigger()
  end, opts)
  keymap.set('v', 'vv', function()
    require('lsp-selection-range').expand()
  end, opts)
  keymap.set('n', ']]', function()
    illuminate.goto_next_reference(true)
  end, opts)
  keymap.set('n', '[[', function()
    illuminate.goto_prev_reference(true)
  end, opts)

  inlayhints.on_attach(client, bufnr)

  local function get_active_clients(buf)
    return vim.lsp.get_active_clients { bufnr = buf, name = client.name }
  end
  local function buf_refresh_codeLens()
    vim.schedule(function()
      for _, c in pairs(get_active_clients(bufnr)) do
        if c.server_capabilities.codeLensProvider then
          vim.lsp.codelens.refresh()
          return
        end
      end
    end)
  end
  local group = api.nvim_create_augroup(string.format('lsp-%s-%s', bufnr, client.id), {})
  if client.server_capabilities.codeLensProvider then
    vim.api.nvim_create_autocmd({ 'InsertLeave', 'BufWritePost', 'TextChanged' }, {
      group = group,
      callback = buf_refresh_codeLens,
      buffer = bufnr,
    })
    buf_refresh_codeLens()
  end
end

function lsp.on_dap_attach(bufnr)
  local opts = { noremap = true, silent = true, buffer = bufnr }
  keymap.set('n', '<F5>', dap.stop, opts)
  keymap.set('n', '<F6>', dap.step_out, opts)
  keymap.set('n', '<F7>', dap.step_into, opts)
  keymap.set('n', '<F8>', dap.step_over, opts)
  keymap.set('n', '<F9>', dap.continue, opts)
  keymap.set('n', '<leader>b', dap.toggle_breakpoint, opts)
  -- keymap.set('n', '<leader>B', dap.toggle_conditional_breakpoint, opts) -- FIXME
  keymap.set('n', '<leader>dr', function()
    dap.repl.toggle { height = 15 }
  end, opts)
  keymap.set('n', '<leader>dl', dap.run_last, opts)
  keymap.set('n', '<leader>dS', function()
    dap_widgets.centered_float(dap_widgets.frames)
  end, opts)
  keymap.set('n', '<leader>ds', function()
    dap_widgets.centered_float(dap_widgets.scopes)
  end, opts)
  keymap.set('n', '<leader>dh', dap_widgets.hover, opts)
  keymap.set('v', '<leader>dH', function()
    dap_widgets.hover(dap_utils.get_visual_selection_text)
  end, opts)
  keymap.set('v', '<M-e>', dapui.eval, opts)
  keymap.set('v', '<M-k>', dapui.float_element, opts)
  keymap.set('n', '<leader>du', dapui.toggle, opts)
end

-- Set up lspconfig.
local capabilities = require('cmp_nvim_lsp').default_capabilities()
capabilities = require('lsp-selection-range').update_capabilities(capabilities)

-- foldingRange capabilities provided by the nvim-ufo plugin
capabilities.textDocument.foldingRange = {
  dynamicRegistration = false,
  lineFoldingOnly = true,
}

lsp.capabilities = capabilities

function lsp.start_or_attach_haskell_tools()
  local ht = require('haskell-tools')
  local telescope = require('telescope')
  local keymap_opts = { noremap = true, silent = true }

  ht.start_or_attach {
    tools = {
      repl = {
        handler = 'toggleterm',
        auto_focus = false,
      },
      codeLens = {
        autoRefresh = false,
        icon = '',
      },
      definition = {
        -- hoogle_signature_fallback = true,
      },
    },
    hls = {
      on_attach = function(client, bufnr)
        lsp.on_attach(client, bufnr)
        lsp.on_dap_attach(bufnr)
        local opts = vim.tbl_extend('keep', keymap_opts, { buffer = bufnr })
        -- vim.keymap.set('n', 'gh', ht.hoogle.hoogle_signature, opts)
        vim.keymap.set('n', '<space>tg', telescope.extensions.ht.package_grep, opts)
        vim.keymap.set('n', '<space>th', telescope.extensions.ht.package_hsgrep, opts)
        vim.keymap.set('n', '<space>tf', telescope.extensions.ht.package_files, opts)
        vim.keymap.set('n', 'gp', ht.project.open_package_yaml, opts)
        vim.keymap.set('n', '<space>ea', ht.lsp.buf_eval_all, opts)
      end,
      default_settings = {
        haskell = {
          formattingProvider = 'stylish-haskell',
          maxCompletions = 10,
        },
      },
    },
  }
end

api.nvim_create_autocmd('LspDetach', {
  group = api.nvim_create_augroup('lsp-detach', {}),
  callback = function(args)
    local group = api.nvim_create_augroup(string.format('lsp-%s-%s', args.buf, args.data.client_id), {})
    pcall(api.nvim_del_augroup_by_name, group)
  end,
})

api.nvim_create_user_command('LspStop', function(kwargs)
  local name = kwargs.fargs[1]
  for _, client in pairs(vim.lsp.get_active_clients()) do
    if client.name == name then
      vim.lsp.stop_client(client.id)
    end
  end
end, {
  nargs = 1,
  complete = function()
    return vim.tbl_map(function(c)
      return c.name
    end, vim.lsp.get_active_clients())
  end,
})

return lsp
