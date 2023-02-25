-- require("autocommands")
require("autocommands.statusline")
require("autocommands.winbar")
-- vim.opt.winbar = require('winbar').get_winbar()
require('lualine').setup {
  options = {
    icons_enabled = true,
    -- theme = 'auto',
    theme = 'tokyonight',
    --component_separators = { left = '', right = ''},
    component_separators = { left = '╲', right = '╱' },
    --section_separators = { left = '', right = ''},
    section_separators = { left = '', right = '' },
    disabled_filetypes = { 'NvimTree' },
    always_divide_middle = true,
    globalstatus = false,
  },
  sections = {
    lualine_a = {'mode'},
    lualine_b = {'branch', 'diff', 'diagnostics'},
    lualine_c = {
      {
        'buffers',
        show_filename_only = true,
        show_bufnr = true,
        mode = 4,
        filetype_names = {
          TelescopePrompt = 'Telescope',
          dashboard = 'Dashboard',
          packer = 'Packer',
          fzf = 'FZF',
          alpha = 'Alpha',
        },
        buffers_color = {
          -- Same values as the general color option can be used here.
          active = 'lualine_b_normal', -- Color for active buffer.
          inactive = 'lualine_b_inactive', -- Color for inactive buffer.
        },
      },
    },
    lualine_x = {'encoding', 'fileformat'},
    lualine_y = {'filetype'},
    lualine_z = {{'os.date("%-I:%M %p")', color = {gui='NONE'}}}
  },
  inactive_sections = {
    lualine_a = {},
    lualine_b = {},
    lualine_c = {'filename'},
    lualine_x = {'filetype'},
    lualine_y = {},
    lualine_z = {}
  },
  tabline = {},
  winbar = {
    lualine_z = {
      {
        'filename',
        path = 1,
        file_status = true,
        newfile_status = true,
      },
    },
  },

  -- Available extensions:
  --   aerial
  --  chadtree
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
  extenstions = { 'fugitive', 'fzf', 'toggleterm', 'nvim-dap-ui' }
}
