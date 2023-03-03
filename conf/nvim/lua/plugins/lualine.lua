-------------------------------------------------------------------------------
-- Status line at the bottom
-- nvim-lualine/lualine.nvim
-------------------------------------------------------------------------------
local ok, lualine = pcall(require, 'lualine')
if not ok then return end

local navic = require('nvim-navic')
local settings = require("settings")

local tabline_cfg = {}
if not settings.disable_tabline then
  tabline_cfg = {
    lualine_a = {},
    lualine_b = {},
    lualine_c = { require'tabline'.tabline_buffers },
    lualine_x = { require'tabline'.tabline_tabs },
    lualine_y = {},
    -- lualine_z = {},
    lualine_z = { require('auto-session-library').current_session_name }
  }
end

local winbar_cfg = {}
local inactive_winbar_cfg = {}
if not settings.disable_winbar then
  winbar_cfg = {
    lualine_a = {},
    lualine_b = {},
    lualine_c = {
      { 'filename', path = 3, color = { bg = 'NONE' } },
      { navic.get_location, cond = navic.is_available },
    },
    lualine_x = {},
    lualine_y = {},
    lualine_z = {}
  }
  inactive_winbar_cfg = {
    lualine_a = {},
    lualine_b = {},
    lualine_c = {
      { 'filename', path = 1, color = { bg = 'NONE' } },
    },
    lualine_x = {},
    lualine_y = {},
    lualine_z = {}
  }
end

local fmt_stat = function()
    local stat = ''
    stat = stat .. 'spaces=' .. vim.opt_local.tabstop._value
    return stat
end

lualine.setup {
  options = {
    globalstatus = true,
    icons_enabled = true,
    -- theme = 'auto',
    -- theme = 'tokyonight',
    -- theme = 'everforest',
    theme = settings.theme,
    --component_separators = { left = '', right = '' },
    component_separators = { left = '', right = '' },
    --section_separators = { left = '', right = '' },
    section_separators = { left = '', right = '' },
    disabled_filetypes = {
      'neo-tree', 'NvimTree', 'Outline', 'toggleterm', 'dap-repl',
      'packer', 'Trouble', 'dapui_scopes', 'dapui_breakpoints', 'dapui_stacks',
      'dapui_watches', 'dap-terminal', 'dapui_console', 'dashboard', 'help',
      'lazy', 'lir', 'neogitstatus', 'oil', 'spectre_panel', 'startify'
    },
    always_divide_middle = true
  },
  sections = {
    lualine_a = { 'mode' },
    lualine_b = { 'branch', 'diff', 'diagnostics' },
    lualine_c = {
      { 'filename',
        path = 3,
        filetype_names = {
          TelescopePrompt = 'Telescope',
          dashboard = 'Dashboard',
          packer = 'Packer',
          fzf = 'FZF',
          alpha = 'Alpha',
        }
      },
      { navic.get_location, cond = navic.is_available },
    },
    lualine_x = { fmt_stat, 'encoding', 'fileformat', 'filetype' },
    lualine_y = { 'progress' },
    lualine_z = { 'location' }
  },
  inactive_sections = {
    lualine_a = {},
    lualine_b = {},
    lualine_c = { 'filename' },
    lualine_x = { 'location' },
    lualine_y = {},
    lualine_z = {}
  },

  tabline = tabline_cfg,
  winbar = winbar_cfg,
  inactive_winbar = inactive_winbar_cfg,

  -- Available extensions:
  --   aerial
  --   chadtree
  --   fern
  --   fugitive
  --   fzf
  --   man
  --   mundo
  --   neo-tree
  --   nerdtree
  --   nvim-dap-ui
  --   nvim-tree
  --   quickfix
  --   symbols-outline
  --   toggleterm
  extensions = { 'neo-tree', 'fugitive', 'fzf', 'toggleterm', 'nvim-dap-ui', 'quickfix' }
}
