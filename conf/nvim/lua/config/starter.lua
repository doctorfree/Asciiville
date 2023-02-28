--- Example configurations
--- >
--- Configuration similar to 'glepnir/dashboard-nvim':
local starter = require('mini.starter')
starter.setup({

  evaluate_single = true,
  footer = os.date(),
  header = table.concat({
      [[  /\ \¬\___  ___/\   /(•)_ __ ___  ]],
      [[ /  \/ / _ \/ _ \ \ / / | '_ ` _ \ ]],
      [[/ /\  /  __/ (_) \ V /| | | | | | |]],
      [[\_\ \/ \___|\___/ \_/ |_|_| |_| |_|]],
      [[___________________________________]],
  }, "\n"),
--  header = function()
--    local hour = tonumber(vim.fn.strftime('%H'))
--    -- [04:00, 12:00) - morning, [12:00, 20:00) - day, [20:00, 04:00) - evening
--    local part_id = math.floor((hour + 4) / 8) + 1
--    local day_part = ({ 'evening', 'morning', 'afternoon', 'evening' })[part_id]
--    local username = vim.loop.os_get_passwd()['username'] or 'USERNAME'
--
--    return ('Good %s, %s\nWelcome to Asciiville Neovim'):format(day_part, username)
--  end,
  items = {
    starter.sections.sessions(5, true),
    function()
      return {
        {action = 'Telescope find_files',  name = 'Files',     section = 'Telescope'},
        {action = 'Telescope live_grep',   name = 'Live grep', section = 'Telescope'},
        {action = 'Telescope oldfiles',    name = 'Old files', section = 'Telescope'},
      }
    end,
    { action = "tab G", name = "G: Fugitive", section = "Git" },
    { action = "PlugUpdate", name = "U: Update Plugins", section = "Plugins" },
    starter.sections.builtin_actions(),
    starter.sections.recent_files(10, true),
    starter.sections.recent_files(10, false),
  },
  content_hooks = {
    starter.gen_hook.adding_bullet(),
    starter.gen_hook.aligning('center', 'center'),
    starter.gen_hook.indexing('all', { 'Builtin actions' }),
    starter.gen_hook.padding(3, 2),
  },
})
--- <
---
--- >
--- Mini.Starter that looks like Alpha
---
--- local status, starter = pcall(require, "mini.starter")
--- if not status then
--- return
--- end

--- starter.setup({
---     content_hooks = {
---         starter.gen_hook.adding_bullet(""),
---         starter.gen_hook.aligning("center", "center"),
---     },
---     evaluate_single = true,
---     footer = os.date(),
---     header = table.concat({
---         [[  /\ \?\___  ___/\   /(?)_ __ ___  ]],
---         [[ /  \/ / _ \/ _ \ \ / / | '_ ` _ \ ]],
---         [[/ /\  /  __/ (_) \ V /| | | | | | |]],
---         [[\_\ \/ \___|\___/ \_/ |_|_| |_| |_|]],
---         [[???????????????????????????????????]],
---     }, "\n"),
---     query_updaters = [[abcdefghilmoqrstuvwxyz0123456789_-,.ABCDEFGHIJKLMOQRSTUVWXYZ]],
---     items = {
---         starter.sections.recent_files(10, false),
---         { action = "tab G", name = "G: Fugitive", section = "Git" },
---         { action = "PlugUpdate", name = "U: Update Plugins", section = "Plugins" },
---         { action = "enew", name = "E: New Buffer", section = "Builtin actions" },
---         { action = "qall!", name = "Q: Quit Neovim", section = "Builtin actions" },
---     },
--- })
---
--- vim.cmd([[
---   augroup MiniStarterJK
---     au!
---     au User MiniStarterOpened nmap <buffer> j <Cmd>lua MiniStarter.update_current_item('next')<CR>
---     au User MiniStarterOpened nmap <buffer> k <Cmd>lua MiniStarter.update_current_item('prev')<CR>
---     au User MiniStarterOpened nmap <buffer> <C-p> <Cmd>Telescope find_files<CR>
---     au User MiniStarterOpened nmap <buffer> <C-n> <Cmd>Telescope file_browser<CR>
---   augroup END
--- ]])
---
--- <
--- Configuration similar to 'mhinz/vim-startify':
--- >
--- local starter = require('mini.starter')
--- starter.setup({
---   evaluate_single = true,
---   header = "Welcome to Asciiville Neovim",
---   items = {
---     starter.sections.builtin_actions(),
---     starter.sections.recent_files(10, false),
---     starter.sections.recent_files(10, true),
---     -- Use this if you set up 'mini.sessions'
---     starter.sections.sessions(5, true)
---   },
---   content_hooks = {
---     starter.gen_hook.adding_bullet(),
---     starter.gen_hook.indexing('all', { 'Builtin actions' }),
---     starter.gen_hook.padding(3, 2),
---   },
--- })
--- <
--- Elaborated configuration showing capabilities of custom items,
--- header/footer, and content hooks:
--- >
---   local my_items = {
---     { name = 'Echo random number', action = 'lua print(math.random())', section = 'Section 1' },
---     function()
---       return {
---         { name = 'Item #1 from function', action = [[echo 'Item #1']], section = 'From function' },
---         { name = 'Placeholder (always incative) item', action = '', section = 'From function' },
---         function()
---           return {
---             name = 'Item #1 from double function',
---             action = [[echo 'Double function']],
---             section = 'From double function',
---           }
---         end,
---       }
---     end,
---     { name = [[Another item in 'Section 1']], action = 'lua print(math.random() + 10)', section = 'Section 1' },
---   }
---
---   local footer_n_seconds = (function()
---     local timer = vim.loop.new_timer()
---     local n_seconds = 0
---     timer:start(0, 1000, vim.schedule_wrap(function()
---       if vim.api.nvim_buf_get_option(0, 'filetype') ~= 'starter' then
---         timer:stop()
---         return
---       end
---       n_seconds = n_seconds + 1
---       MiniStarter.refresh()
---     end))
---
---     return function()
---       return 'Number of seconds since opening: ' .. n_seconds
---     end
---   end)()
---
---   local hook_top_pad_10 = function(content)
---     -- Pad from top
---     for _ = 1, 10 do
---       -- Insert at start a line with single content unit
---       table.insert(content, 1, { { type = 'empty', string = '' } })
---     end
---     return content
---   end
---
---   local starter = require('mini.starter')
---   starter.setup({
---     items = my_items,
---     footer = footer_n_seconds,
---     content_hooks = { hook_top_pad_10 },
---   })
--- <
