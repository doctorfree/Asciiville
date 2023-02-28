-- Dashboard
vim.g.dashboard_preview_command = "cat"
-- let g:dashboard_preview_file="~/.config/environment.d/sway.conf"
vim.g.dashboard_preview_pipeline = "lolcat"
-- let g:dashboard_preview_file_width=70
-- let g:dashboard_preview_file_height=10

-- vim.g.dashboard_session_directory = '~/.config/nvim/sessions'
-- vim.g.dashboard_default_executive = 'telescope'
-- vim.cmd("let g:dashboard_default_executive = 'telescope'")
-- vim.cmd("let g:dashboard_session_directory = $HOME..'/.config/nvim/sessions'")
-- vim.cmd("let packages = len(globpath('~/.local/share/nvim/site/pack/packer/start', '*', 0, 1))")

-- vim.api.nvim_exec([[
--     let g:dashboard_custom_footer = ['LuaJIT loaded '..packages..' packages']
-- ]], false)

-- vim.g.dashboard_custom_section = {
--     a = {description = {'  Reload Last Session            SPC q l'}, command = 'SessionLoad'},
--     b = {description = {'  Recently Opened Files          SPC f r'}, command = 'Telescope oldfiles'},
--     c = {description = {'  Open Project                   SPC f p'}, command = 'Telescope marks'},
--     d = {description = {'  Jump to Bookmark               SPC f b'}, command = 'Telescope project'},
--     e = {description = {'  Find File                      SPC f f'}, command = 'Telescope find_files'},
--     f = {description = {'  Find Word                      SPC s p'}, command = 'Telescope live_grep'},
--     g = {description = {'  Open Neovim Configuration     SPC f P'}, command = ':e ~/.config/nvim/init.vim'},
-- }

vim.g.dashboard_custom_header = {
       "            :h-                                  Nhy`               ",
       "           -mh.                           h.    `Ndho               ",
       "           hmh+                          oNm.   oNdhh               ",
       "          `Nmhd`                        /NNmd  /NNhhd               ",
       "          -NNhhy                      `hMNmmm`+NNdhhh               ",
       "          .NNmhhs              ```....`..-:/./mNdhhh+               ",
       "           mNNdhhh-     `.-::///+++////++//:--.`-/sd`               ",
       "           oNNNdhhdo..://++//++++++/+++//++///++/-.`                ",
       "      y.   `mNNNmhhhdy+/++++//+/////++//+++///++////-` `/oos:       ",
       " .    Nmy:  :NNNNmhhhhdy+/++/+++///:.....--:////+++///:.`:s+        ",
       " h-   dNmNmy oNNNNNdhhhhy:/+/+++/-         ---:/+++//++//.`         ",
       " hd+` -NNNy`./dNNNNNhhhh+-://///    -+oo:`  ::-:+////++///:`        ",
       " /Nmhs+oss-:++/dNNNmhho:--::///    /mmmmmo  ../-///++///////.       ",
       "  oNNdhhhhhhhs//osso/:---:::///    /yyyyso  ..o+-//////////:/.      ",
       "   /mNNNmdhhhh/://+///::://////     -:::- ..+sy+:////////::/:/.     ",
       "     /hNNNdhhs--:/+++////++/////.      ..-/yhhs-/////////::/::/`    ",
       "       .ooo+/-::::/+///////++++//-/ossyyhhhhs/:///////:::/::::/:    ",
       "       -///:::::::////++///+++/////:/+ooo+/::///////.::://::---+`   ",
       "       /////+//++++/////+////-..//////////::-:::--`.:///:---:::/:   ",
       "       //+++//++++++////+++///::--                 .::::-------::   ",
       "       :/++++///////////++++//////.                -:/:----::../-   ",
       "       -/++++//++///+//////////////               .::::---:::-.+`   ",
       "       `////////////////////////////:.            --::-----...-/    ",
       "        -///://////////////////////::::-..      :-:-:-..-::.`.+`    ",
       "         :/://///:///::://::://::::::/:::::::-:---::-.-....``/- -   ",
       "           ::::://::://::::::::::::::----------..-:....`.../- -+oo/ ",
       "            -/:::-:::::---://:-::-::::----::---.-.......`-/.      ``",
       "           s-`::--:::------:////----:---.-:::...-.....`./:          ",
       "          yMNy.`::-.--::..-dmmhhhs-..-.-.......`.....-/:`           ",
       "         oMNNNh. `-::--...:NNNdhhh/.--.`..``.......:/-              ",
       "        :dy+:`      .-::-..NNNhhd+``..`...````.-::-`                ",
       "                        .-:mNdhh:.......--::::-`                    ",
       "                           yNh/..------..`                          ",
       "                                                                    ",
}

require('dashboard').setup {
--  theme = 'hyper' --  theme is doom and hyper default is hyper
--  disable_move    --  defualt is false disable move keymap for hyper
--  shortcut_type   --  shorcut type 'letter' or 'number'
--  config = {},    --  config used for theme
--  hide = {
--    statusline    -- hide statusline default is true
--    tabline       -- hide the tabline
--    winbar        -- hide winbar
--  },
--  preview = {
--    command       -- preview command
--    file_path     -- preview file path
--    file_height   -- preview file height
--    file_width    -- preview file width
--  },
  theme = 'hyper',
  config = {
    week_header = {
     enable = true,
    },
    packages = { enable = false },
    shortcut = {
      { desc = ' Update', group = '@property', action = 'PlugUpdate', key = 'u' },
      { desc = ' Find File', group = 'Label', action = 'Telescope find_files', key = 'f', },
      { desc = ' Find Word', group = 'Label', action = 'Telescope live_grep', key = 'w', },
      { desc = ' File Browser', group = 'Label', action = 'Telescope file_browser', key = 'b', },
      { desc = ' Colorschemes', group = 'Number', action = 'Telescope colorscheme', key = 'c', },
    },
--  project = {
--    enable = true,
--    limit = 8,
--    icon = 'your icon',
--    label = '',
--    action = 'Telescope find_files cwd='
--  },
--    {
--      desc = ' Apps',
--      group = 'DiagnosticHint',
--      action = 'Telescope app',
--      key = 'a',
--    },
--    {
--      desc = ' dotfiles',
--      group = 'Number',
--      action = 'Telescope dotfiles',
--      key = 'd',
--    },
--  mru = { limit = 10, icon = 'your icon', label = '', },
--  footer = {}, -- footer
--   },
-- }

-- Doom theme has different configuration options
-- require('dashboard').setup {
--  theme = 'hyper' --  theme is doom and hyper default is hyper
--  disable_move    --  defualt is false disable move keymap for hyper
--  shortcut_type   --  shorcut type 'letter' or 'number'
--  config = {},    --  config used for theme
--  hide = {
--    statusline    -- hide statusline default is true
--    tabline       -- hide the tabline
--    winbar        -- hide winbar
--  },
--  preview = {
--    command       -- preview command
--    file_path     -- preview file path
--    file_height   -- preview file height
--    file_width    -- preview file width
--  },
--  theme = 'doom',
--  config = {
--    header = {vim.g.dashboard_custom_header}, --your header
--    center = {
--    {
--      icon = ' ',
--      desc = 'Find Dotfiles',
--      key = 'f',
--      keymap = 'SPC f d',
--      action = 'lua print(3)'
--    },
--    {
--      icon = '\uf1b2 ',
--      icon_hl = 'Title',
--      desc = 'Find File           ',
--      desc_hl = 'String',
--      key = 'b',
--      keymap = 'SPC f f',
--      key_hl = 'Number',
--      action = 'lua print(2)'
--    },
--    { desc = ' Update', group = '@property', action = 'PlugUpdate', key = 'u' },
--    { desc = ' Find File', group = 'Label', action = 'Telescope find_files', key = 'f', },
--    { desc = ' Find Word', group = 'Label', action = 'Telescope live_grep', key = 'w', },
--    { desc = ' File Browser', group = 'Label', action = 'Telescope file_browser', key = 'b', },
--    { desc = ' Colorschemes', group = 'Number', action = 'Telescope colorscheme', key = 'c', },
--  },
--  footer = {}  --your footer
  }
}
