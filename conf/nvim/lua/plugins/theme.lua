local settings = require("settings")

if settings.theme == "nightfox" then
  return {
    "EdenEast/nightfox.nvim",
    config = function()
      require("plugins.themes.nightfox")
    end,
  }
elseif settings.theme == "tundra" then
  return {
    "sam4llis/nvim-tundra",
    config = function()
      require("plugins.themes.tundra")
    end,
  }
elseif settings.theme == "tokyonight" then
  return {
    "folke/tokyonight.nvim",
    branch = "main",
    config = function()
      require("plugins.themes.tokyonight")
    end,
  }
elseif settings.theme == "kanagawa" then
  return {
    "rebelot/kanagawa.nvim",
    config = function()
      require("lua.plugins.themes.kanagawa")
    end,
  }
else
  return {
    "catppuccin/nvim",
    name = "catppuccin",
    config = function()
      require("plugins.themes.catppuccin")
    end,
  }
end
