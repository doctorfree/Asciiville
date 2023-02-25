-------------------------------------------------------------------------------
-- Status line at the bottom
-- nvim-lualine/lualine.nvim
-------------------------------------------------------------------------------
local ok, lualine = pcall(require, 'lualine')
if not ok then return end

local navic_ok, navic = pcall(require, 'nvim-navic')
local winbar_cfg = {}
local inactive_winbar_cfg = {}

local fmt_stat = function()
    local stat = ''
    stat = stat .. 'spaces=' .. vim.opt_local.tabstop._value
    return stat
end

if navic_ok then
    winbar_cfg = {
        lualine_a = {},
        lualine_b = {},
        lualine_c = {
            { 'filename', path = 3, file_status = true, newfile_status = true },
            { navic.get_location, cond = navic.is_available },
        },
        lualine_x = {},
        lualine_y = { 'progress' },
        lualine_z = { 'location' }
    }
    inactive_winbar_cfg = {
        lualine_a = {},
        lualine_b = {},
        lualine_c = {
            { 'filename', path = 3, file_status = true, newfile_status = true },
        },
        lualine_x = {},
        lualine_y = {},
        lualine_z = {}
    }
end

lualine.setup({
    options = {
        icons_enabled = true,
        -- theme = 'auto',
        theme = 'tokyonight',
        --component_separators = { left = '', right = ''},
        component_separators = { left = '', right = '' },
        --section_separators = { left = '', right = ''},
        section_separators = { left = '', right = '' },
        disabled_filetypes = {
            statusline = {},
            winbar = {
                'NvimTree',
                'Outline',
                'toggleterm',
                'alpha',
                'dap-repl',
                'packer',
                'Trouble',
                'dapui_scopes',
                'dapui_breakpoints',
                'dapui_stacks',
                'dapui_watches',
                'dap-terminal',
                'dapui_console',
                'dashboard',
                'help',
                'lazy',
                'lir',
                'neogitstatus',
                'oil',
                'spectre_panel',
                'startify',
            },
        },
        always_divide_middle = true,
        globalstatus = true,
    },
    sections = {
        lualine_a = { 'mode' },
        lualine_b = { 'branch', 'diff', 'diagnostics' },
        lualine_c = { { 'filename', path = 1, } },
        lualine_x = { fmt_stat, 'encoding', 'fileformat', 'filetype' },
        lualine_y = {},
        lualine_z = {},
    },
    inactive_sections = {
        lualine_a = {},
        lualine_b = {},
        lualine_c = { 'filename' },
        lualine_x = { 'location' },
        lualine_y = {},
        lualine_z = {}
    },
    tabline = {},
    winbar = winbar_cfg,
    inactive_winbar = inactive_winbar_cfg,

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
    -- extenstions = { 'fugitive', 'fzf', 'toggleterm', 'nvim-dap-ui' }
    extensions = { 'nvim-tree', 'quickfix', 'fugitive', 'symbols-outline', 'toggleterm', 'nvim-dap-ui' }
})
