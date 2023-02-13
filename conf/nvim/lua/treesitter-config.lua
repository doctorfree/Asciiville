require('nvim-treesitter.configs').setup {
  ensure_installed = {
    "bash",
    "c",
    "cmake",
    "comment",
    "css",
    "go",
    "help",
    "html",
    "java",
    "javascript",
    "json",
    "lua",
    "query",
    "python",
    "regex",
    "toml",
    "vim",
    "yaml"
  },
  sync_install = false,
  highlight = {
    enable = true,
    additional_vim_regex_highlighting = false,
  },
}
