local langtree = false
-- stylua: ignore start
local ts_ensure_installed = { "go", "css", "html", "javascript", "typescript", "json", "c", "java", "toml", "tsx", "lua", "cpp", "python", "rust", "yaml", "vue", "vim", "org"}
-- stylua: ignore end

local treesitter = function()

  local enable = false
  local has_ts = pcall(require, 'nvim-treesitter.configs')
  if not has_ts then
    vim.notify('ts not installed')
    return
  end
  local lines = vim.fn.line('$')
  if lines > 20000 then -- skip some settings for large file
    vim.cmd([[syntax manual]])
    return
  end

  if lines > 10000 then
    enable = true
    langtree = false
    vim.cmd([[syntax on]])
  else
    enable = true
    langtree = true
  end

  require('nvim-treesitter.configs').setup({
    highlight = {
      enable = enable, -- false will disable the whole extension
      additional_vim_regex_highlighting = { 'org' }, -- unless not supported by ts
      disable = { 'elm' }, -- list of language that will be disabled
      use_languagetree = langtree,
      custom_captures = { todo = 'Todo' },
    },
  })
end

local treesitter_obj = function()
  local lines = vim.fn.line('$')
  if lines > 8000 then -- skip some settings for large file
    return
  end

  local enable = true
  if lines > 4000 then
    enable = false
  end
  require('nvim-treesitter.configs').setup({

    indent = { enable = enable },
    context_commentstring = { enable = enable, enable_autocmd = false },
    incremental_selection = {
      enable = false, -- use textobjects
      -- disable = {"elm"},
      keymaps = {
        -- mappings for incremental selection (visual mappings)
        init_selection = 'gnn', -- maps in normal mode to init the node/scope selection
        scope_incremental = 'gnn', -- increment to the upper scope (as defined in locals.scm)
        node_incremental = '<TAB>', -- increment to the upper named parent
        node_decremental = '<S-TAB>', -- decrement to the previous node
      },
    },
    textobjects = {
      -- syntax-aware textobjects
      lsp_interop = {
        enable = false, -- use LSP
        peek_definition_code = { ['DF'] = '@function.outer', ['CF'] = '@class.outer' },
      },
      move = {
        enable = true,
        set_jumps = true, -- whether to set jumps in the jumplist
        goto_next_start = {
          [']m'] = '@function.outer',
          [']['] = '@class.outer',
          [']o'] = '@loop.*',
        },
        goto_next_end = { [']M'] = '@function.outer', [']]'] = '@class.outer' },
        goto_previous_start = { ['[m'] = '@function.outer', ['[['] = '@class.outer' },
        goto_previous_end = { ['[M'] = '@function.outer', ['[]'] = '@class.outer' },
      },
      select = {
        enable = enable,
        lookahead = true,
        keymaps = {
          -- You can use the capture groups defined in textobjects.scm
          ['af'] = { '@function.outer', desc = 'select inner class' },
          ['if'] = { '@function.inner', desc = 'select inner function' },
          ['ac'] = { '@class.outer', desc = 'select outer class' },
          ['ic'] = { '@class.inner', desc = 'select inner class' },
        },
      },
      swap = { -- use ISWAP
        enable = false,
        -- swap_next = { ["<leader>a"] = "@parameter.inner" },
        -- swap_previous = { ["<leader>A"] = "@parameter.inner" },
      },
    },
    -- ensure_installed = "maintained"
    ensure_installed = ts_ensure_installed,
  })

end

local treesitter_ref = function()
  local enable
  if vim.fn.line('$') > 5000 then -- skip for large file
    enable = false
  else
    enable = true
  end

  require('nvim-treesitter.configs').setup({
    refactor = {
      highlight_definitions = { enable = enable },
      highlight_current_scope = { enable = false }, -- prefer black-line
      smart_rename = {
        enable = false, -- prefer lsp rename
        -- keymaps = {
        --   smart_rename = "<Leader>gr", -- mapping to rename reference under cursor
        -- },
      },
      navigation = {
        enable = false, -- use navigator
        -- keymaps = {
        --   goto_definition = "gnd", -- mapping to go to definition of symbol under cursor
        --   list_definitions = "gnD", -- mapping to list all definitions in current file
        --   list_definitions_toc = "gO", -- gT navigator
        --   -- goto_next_usage = "<c->>",
        --   -- goto_previous_usage = "<c-<>",
        -- },
      },
    },
    matchup = {
      enable = enable, -- mandatory, false will disable the whole extension
      disable = { 'ruby' }, -- optional, list of language that will be disabled
    },
    autopairs = { enable = enable },
    autotag = { enable = enable },
  })
  local parser_config = require('nvim-treesitter.parsers').get_parser_configs()
  parser_config.norg = {
    install_info = {
      url = 'https://github.com/nvim-neorg/tree-sitter-norg',
      files = { 'src/parser.c', 'src/scanner.cc' },
      branch = 'main',
    },
  }
end

function textsubjects()

 local enable = false
  local en_subjects = true
  if vim.fn.line('$') > 5000 then -- skip for large file
    enable = false
  end
  require('nvim-treesitter.configs').setup({

    textsubjects = {
      enable = enable,
      prev_selection = ',',
      keymaps = {
        ['.'] = 'textsubjects-smart',
        -- [";"] = "textsubjects-container-outer",
        -- ["i;"] = "textsubjects-container-inner",
      },
    },
  })
end

local function tshopper()
  -- keymaps
end

local treesitter_context = function(width)
  local ok, ts = pcall(require, 'nvim-treesitter')
  if not ok or not ts then
    return ' '
  end
  local en_context = true

  if vim.fn.line('$') > 5000 then -- skip for large file
    return ' '
  end
  local disable_ft = {
    'NvimTree',
    'neo-tree',
    'guihua',
    'packer',
    'guihua_rust',
    'clap_input',
    'clap_spinner',
    'TelescopePrompt',
    'csv',
    'txt',
    'defx',
  }
  if vim.tbl_contains(disable_ft, vim.o.ft) then
    return ' '
  end
  local type_patterns = {
    'class',
    'function',
    'method',
    'interface',
    'type_spec',
    'table',
    'if_statement',
    'for_statement',
    'for_in_statement',
    'call_expression',
    'comment',
  }

  if vim.o.ft == 'json' then
    type_patterns = { 'object', 'pair' }
  end
  if not en_context then
    return ' '
  end

  local f = require('nvim-treesitter').statusline({
    indicator_size = width,
    type_patterns = type_patterns,
  })
  local context = string.format('%s', f) -- convert to string, it may be a empty ts node

  if context == 'vim.NIL' then
    return ' '
  end

  return ' ' .. context
end
vim.api.nvim_create_autocmd({ 'FileType' }, {
  group = vim.api.nvim_create_augroup('SyntaxFtAuGroup', {}),
  callback = function()
    local ft = vim.o.ft
    if vim.tbl_contains(ts_ensure_installed, ft) then
      return
    end
    -- in case ts not installed

    local fsize = vim.fn.getfsize(vim.fn.expand('%:p:f')) or 1
    if fsize < 100000 then
      vim.cmd('syntax on')
    end
  end,
})
return {
  treesitter = treesitter,
  treesitter_obj = treesitter_obj,
  treesitter_ref = treesitter_ref,
  textsubjects = textsubjects,
  context = treesitter_context,
  installed = ts_ensure_installed,
  tshopper = tshopper,
}
