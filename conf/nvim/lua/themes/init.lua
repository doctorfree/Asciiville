-- local highlights = require('themes.hl')
-- local function setup(opts)
--   if opts == nil then opts = { theme = "pop" } end
--   local colors = require('themes.schemes.' .. opts.theme).get_colors()
--   highlights.highlight_all(colors, opts)
-- end

-- return { setup = setup }
--
local settings = require("settings")
local utils = require("utils.functions")
local conf = "themes/" .. settings.theme
local path = vim.fn.stdpath "config" .. "/lua/" .. conf .. ".lua"

if utils.file_or_dir_exists(path) then
  require(conf)
else
  vim.cmd("colorscheme " .. settings.theme)
end
