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
local dashboard_recent_files = 5
-- disable the header of the dashboard
local disable_dashboard_header = false
-- disable quick links of the dashboard
local disable_dashboard_quick_links = false


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
  file_button_el.opts.hl = fb_hl
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
        return { mru(1, cdir, dashboard_recent_files) }
      end,
      opts = { shrink_margin = false },
    },
  },
}

local buttons = {
  type = "group",
  val = {
    { type = "text", val = "Quick links", opts = { hl = "SpecialComment", position = "center" } },
    dashboard.button("f", "  Find File", ":" .. require("utils.functions").telescope_find_files() .. "<CR>"),
    dashboard.button("b", "  File Browser", ":Telescope file_browser grouped=true<CR>"),
    dashboard.button("t", "  Find Text", ":Telescope live_grep<CR>"),
    dashboard.button("p", "  Search Projects", ":Telescope projects<CR>"),
    dashboard.button("z", "  Search Zoxide", ":Telescope zoxide list<CR>"),
    dashboard.button("r", "  Recent Files", ":Telescope oldfiles<CR>"),
    dashboard.button("e", "  New File", ":ene <BAR> startinsert<CR>"),
    dashboard.button("h", "  Commit History", ":GV<CR>"),
    dashboard.button("q", "  Quit", ":qa<CR>"),
    { type = "padding", val = 1 },
    { type = "text", val = "Plugin Management", opts = { hl = "SpecialComment", position = "center" } },
    dashboard.button("h", "  Health", ":checkhealth<CR>"),
    dashboard.button("s", "  Plugin Status", ":PlugStatus<CR>"),
    dashboard.button("c", "  Clean Plugins", ":PlugClean<CR>"),
    dashboard.button("i", "  Install Plugins", ":PlugInstall<CR>"),
    dashboard.button("u", "  Update Plugins", ":PlugUpdate<CR>"),
  },
  position = "center",
}

local header = {
  type = "text",
  -- From https://gist.github.com/sRavioli/d6fb0a813b6affc171976b7dd09764d3
  val = require('config.headers')['random'],
  opts = {
    position = "center",
    hl = "AlphaHeader",
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

if dashboard_recent_files == 0 then
  layout[1] = nil
  layout[2] = nil
end

if disable_dashboard_header == true then
  layout[0] = nil
end

if disable_dashboard_quick_links == true then
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
      vim.api.nvim_set_hl(0, "AlphaButtons", { fg = colors.cyan })
      vim.api.nvim_set_hl(0, "AlphaShortcut", { fg = colors.orange })
      vim.api.nvim_set_hl(0, "AlphaFooter", { fg = colors.yellow, italic = true })
      vim.api.nvim_set_hl(0, "SpecialComment", { fg = colors.yellow, italic = true })
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
