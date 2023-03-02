local status_ok, alpha = pcall(require, "alpha")
if not status_ok then
  return
end

local path_ok, path = pcall(require, "plenary.path")
if not path_ok then
  return
end

-- Number of recent files shown in dashboard
-- 0 disables showing recent files
-- local dashboard_recent_files = 5
-- disable the header of the dashboard
-- local disable_dashboard_header = false
-- disable quick links of the dashboard
-- local disable_dashboard_quick_links = false
local settings = require('settings')

local dashboard = require("alpha.themes.dashboard")
local nvim_web_devicons = require("nvim-web-devicons")
local cdir = vim.fn.getcwd()

local function get_extension(fn)
  local match = fn:match("^.+(%..+)$")
  local ext = ""
  if match ~= nil then
    ext = match:sub(2)
  end
  return ext
end

local function icon(fn)
  local nwd = require("nvim-web-devicons")
  local ext = get_extension(fn)
  return nwd.get_icon(fn, ext, { default = true })
end

local function file_button(fn, sc, short_fn)
  short_fn = short_fn or fn
  local ico_txt
  -- Could remove some cruft but i'm scared. If it ain't broke don't fix it
  local fb_hl = {}

  local ico, hl = icon(fn)
  local hl_option_type = type(nvim_web_devicons.highlight)
  if hl_option_type == "boolean" then
    if hl and nvim_web_devicons.highlight then
      table.insert(fb_hl, { hl, 0, 1 })
    end
  end
  if hl_option_type == "string" then
    table.insert(fb_hl, { nvim_web_devicons.highlight, 0, 1 })
  end
  ico_txt = ico .. "  "

  local file_button_el = dashboard.button(sc, ico_txt .. short_fn, "<cmd>e " .. fn .. " <CR>")
  local fn_start = short_fn:match(".*/")
  if fn_start ~= nil then
    table.insert(fb_hl, { "Comment", #ico_txt - 2, #fn_start + #ico_txt - 2 })
  end
  file_button_el.opts.hl = 'AlphaButtons'
  return file_button_el
end

local default_mru_ignore = { "gitcommit" }

local mru_opts = {
  ignore = function(mrupath, ext)
    return (string.find(mrupath, "COMMIT_EDITMSG")) or (vim.tbl_contains(default_mru_ignore, ext))
  end,
}

--- @param start number
--- @param cwd string optional
--- @param items_number number optional number of items to generate, default = 10
local function mru(start, cwd, items_number, opts)
  opts = opts or mru_opts
  items_number = items_number or 9

  local oldfiles = {}
  for _, v in pairs(vim.v.oldfiles) do
    if #oldfiles == items_number then
      break
    end
    local cwd_cond
    if not cwd then
      cwd_cond = true
    else
      cwd_cond = vim.startswith(v, cwd)
    end
    local ignore = (opts.ignore and opts.ignore(v, get_extension(v))) or false
    if (vim.fn.filereadable(v) == 1) and cwd_cond and not ignore then
      oldfiles[#oldfiles + 1] = v
    end
  end

  local special_shortcuts = { "a" }
  local target_width = 35

  local tbl = {}
  for i, fn in ipairs(oldfiles) do
    local short_fn
    if cwd then
      short_fn = vim.fn.fnamemodify(fn, ":.")
    else
      short_fn = vim.fn.fnamemodify(fn, ":~")
    end

    if #short_fn > target_width then
      short_fn = path.new(short_fn):shorten(1, { -2, -1 })
      if #short_fn > target_width then
        short_fn = path.new(short_fn):shorten(1, { -1 })
      end
    end

    local shortcut = ""
    if i <= #special_shortcuts then
      shortcut = special_shortcuts[i]
    else
      shortcut = tostring(i + start - 1 - #special_shortcuts)
    end

    local file_button_el = file_button(fn, " " .. shortcut, short_fn)
    tbl[i] = file_button_el
  end
  return {
    type = "group",
    val = tbl,
    opts = {},
  }
end

local section_mru = {
  type = "group",
  val = {
    {
      type = "text",
      val = "Recent files",
      opts = {
        hl = "SpecialComment",
        shrink_margin = false,
        position = "center",
      },
    },
    { type = "padding", val = 1 },
    {
      type = "group",
      val = function()
        return { mru(1, cdir, settings.dashboard_recent_files) }
      end,
      opts = { shrink_margin = false },
    },
  },
}

-- This wouldn't be necessary if we could pass 'opts' in to dashboard.button()
local new_file_btn = dashboard.button('n', '  New File', ':ene <BAR> startinsert<CR>')
new_file_btn.opts.hl = 'AlphaShortcut'
local find_file_btn = dashboard.button('f', '  Find File', ':' .. require('utils.functions').project_files() .. '<CR>')
find_file_btn.opts.hl = 'AlphaShortcut'
local file_browser_btn = dashboard.button('b', '  File Browser', ':Telescope file_browser grouped=true<CR>')
file_browser_btn.opts.hl = 'AlphaShortcut'
local file_tree_btn = dashboard.button('e', '  File Tree', ':Neotree<CR>')
file_tree_btn.opts.hl = 'AlphaShortcut'
local find_text_btn = dashboard.button('t', '  Find Text', ':Telescope live_grep<CR>')
find_text_btn.opts.hl = 'AlphaShortcut'
local search_project_btn = dashboard.button('p', '  Search Projects', ':Telescope projects<CR>')
search_project_btn.opts.hl = 'AlphaShortcut'
local search_zoxide_btn = dashboard.button('z', '  Search Zoxide', ':Telescope zoxide list<CR>')
search_zoxide_btn.opts.hl = 'AlphaShortcut'
local recent_files_btn = dashboard.button('r', '  Search Recent Files', ':Telescope oldfiles<CR>')
recent_files_btn.opts.hl = 'AlphaShortcut'
local git_commit_btn = dashboard.button('g', '  Git Commit History', ':GV<CR>')
git_commit_btn.opts.hl = 'AlphaShortcut'
local quit_btn = dashboard.button('q', '  Quit', ':qa<CR>')
quit_btn.opts.hl = 'AlphaShortcut'

local health_btn = dashboard.button('h', '  Neovim Health', ':checkhealth<CR>')
health_btn.opts.hl = 'AlphaHeader'
local settings_btn = dashboard.button('s', '  Neovim Settings', ':e ~/.config/nvim/lua/settings.lua<CR>')
settings_btn.opts.hl = 'AlphaHeader'
local options_btn = dashboard.button('o', '  Neovim Options', ':e ~/.config/nvim/lua/options.lua<CR>')
options_btn.opts.hl = 'AlphaHeader'
local mappings_btn = dashboard.button('m', '  Keyboard Mappings', ':e ~/.config/nvim/lua/mappings.lua<CR>')
mappings_btn.opts.hl = 'AlphaHeader'
local status_btn = dashboard.button('S', '  Plugin Status', ':PlugStatus<CR>')
status_btn.opts.hl = 'AlphaShortcut'
local clean_btn = dashboard.button('C', '  Clean Plugins', ':PlugClean<CR>')
clean_btn.opts.hl = 'AlphaShortcut'
local install_btn = dashboard.button('I', '  Install Plugins', ':PlugInstall<CR>')
install_btn.opts.hl = 'AlphaShortcut'
local update_btn = dashboard.button('U', '  Update Plugins', ':PlugUpdate<CR>')
update_btn.opts.hl = 'AlphaShortcut'

local buttons = {
  type = 'group',
  val = {
    { type = 'text', val = 'Quick links', opts = { hl = 'SpecialComment', position = 'center' } },
    new_file_btn,
    find_file_btn,
    file_browser_btn,
    file_tree_btn,
    find_text_btn,
    search_project_btn,
    search_zoxide_btn,
    recent_files_btn,
    git_commit_btn,
    quit_btn,
    { type = 'padding', val = 1 },
    { type = 'text', val = 'Neovim Configuration', opts = { hl = 'SpecialComment', position = 'center' } },
    health_btn,
    settings_btn,
    options_btn,
    mappings_btn,
    { type = 'padding', val = 1 },
    { type = 'text', val = 'Plugin Management', opts = { hl = 'SpecialComment', position = 'center' } },
    status_btn,
    clean_btn,
    install_btn,
    update_btn,
  },
  position = 'center',
}

local header = {
  type = 'text',
  -- From https://gist.github.com/sRavioli/d6fb0a813b6affc171976b7dd09764d3
  val = require('plugins.alpha.headers')['random'],
  opts = {
    position = 'center',
    hl = 'AlphaHeader',
  },
}

-- Lua implementation of PHP scandir function
local function scandir(directory)
    local i, popen = 0, io.popen
    local pfile = popen('ls "'..directory..'"')
    for filename in pfile:lines() do
        i = i + 1
    end
    pfile:close()
    return i
end

local plugdir = os.getenv("HOME") .. "/.local/share/nvim/plugged"
local numplugs = scandir(plugdir)

-- If TZ is not set or for some reason os.date() is returning UTC
-- you can add the timezone offset manually like:
-- local timeShift = 8 * 60 * 60  -- 8 hours
-- local datetime = os.date('  %Y-%b-%d   %H:%M:%S', os.time() - timeShift)
local datetime = os.date('  %Y-%b-%d   %H:%M:%S', os.time())
local version = vim.version()
local nvim_version_info = "   v" .. version.major .. "." .. version.minor .. "." .. version.patch
local footer = {
  type = "text",
  val = datetime .. "    " .. numplugs .. " plugins" .. nvim_version_info,
  opts = {
    position = "center",
    hl = "AlphaFooter",
  },
}

local layout = {}
layout[0] = header
layout[1] = { type = "padding", val = 1 }
layout[2] = section_mru
layout[3] = { type = "padding", val = 1 }
layout[4] = buttons
layout[5] = { type = "padding", val = 1 }
layout[6] = footer

if settings.dashboard_recent_files == 0 then
  layout[1] = nil
  layout[2] = nil
end

if settings.disable_dashboard_header == true then
  layout[0] = nil
end

if settings.disable_dashboard_quick_links == true then
  layout[3] = nil
  layout[4] = nil
end

local colors = require('tokyonight.colors').setup()
local opts = {
  layout = layout,
  opts = {
    margin = 4,
    setup = function()
--    require("nvim-web-devicons").setup{
--      enabled = true,
--      highlight = true,
--    }
      vim.api.nvim_set_hl(0, "AlphaHeader", { fg = colors.blue })
      vim.api.nvim_set_hl(0, "AlphaHeaderLabel", { fg = colors.orange })
      vim.api.nvim_set_hl(0, "AlphaButtons", { fg = colors.teal })
      vim.api.nvim_set_hl(0, "AlphaShortcut", { fg = colors.cyan })
      vim.api.nvim_set_hl(0, "AlphaFooter", { fg = colors.purple, italic = true })
      vim.api.nvim_set_hl(0, "SpecialComment", { fg = colors.green, italic = true })
      vim.api.nvim_set_hl(0, "Comment", { fg = colors.yellow })
      vim.api.nvim_set_hl(0, "DashboardHeader", { fg = colors.blue })
      vim.api.nvim_set_hl(0, "DashboardHeaderLabel", { fg = colors.orange })
      vim.api.nvim_set_hl(0, "DashboardButtons", { fg = colors.cyan })
      vim.api.nvim_set_hl(0, "DashboardShortcut", { fg = colors.orange })
      vim.api.nvim_set_hl(0, "DashboardFooter", { fg = colors.yellow, italic = true })
    end
  },
}

alpha.setup(opts)

-- Disable folding on alpha buffer
vim.cmd([[
    autocmd FileType alpha setlocal nofoldenable
]])
vim.cmd([[
  autocmd User AlphaReady set showtabline=0 | autocmd BufUnload <buffer> set showtabline=2
]])
