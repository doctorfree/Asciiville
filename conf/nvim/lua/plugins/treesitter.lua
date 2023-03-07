local settings = require('settings')

require('nvim-treesitter.configs').setup {
  ensure_installed = settings.treesitter_ensure_installed,
  auto_install = true,
  sync_install = false,
  highlight = {
    enable = true,
    additional_vim_regex_highlighting = false,
  },
}
