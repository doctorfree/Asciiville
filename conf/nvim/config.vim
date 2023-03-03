"""
"      _                      _    _           _   __   __
"     / \                    (_)  (_)         (_) [  | [  |
"    / _ \     .--.   .---.  __   __  _   __  __   | |  | | .---.
"   / ___ \   ( (`\] / /'`\][  | [  |[ \ [  ][  |  | |  | |/ /__\\
" _/ /   \ \_  `'.'. | \__.  | |  | | \ \/ /  | |  | |  | || \__.,
"|____| |____|[\__) )'.___.'[___][___] \__/  [___][___][___]'.__.'
"
"--------------Neovim Plugins Configuration Vimscript-------------
"
"  Version : 1.0.0
"  License : MIT
"  Author  : Ronald Record
"  URL     : https://github.com/doctorfree/nvim
"  Project : https://github.com/doctorfree/Asciiville
"-----------------------------------------------------------------

if exists('g:plugs["wilder.nvim"]')
  if !empty(glob(g:plugs['wilder.nvim'].dir.'/lua/wilder.lua'))
    lua require('plugins.wilder')
  endif
endif
" Need to configure Neodev prior to LSP
if exists('g:plugs["neodev.nvim"]')
  if !empty(glob(g:plugs['neodev.nvim'].dir.'/lua/neodev/init.lua'))
    lua require('plugins.neodev')
  endif
endif
""" Core plugin configuration (lua)
if exists('g:plugs["nvim-cmp"]')
  if !empty(glob(g:plugs['nvim-cmp'].dir.'/lua/cmp/init.lua'))
    lua require('plugins.nvim-cmp')
  endif
endif
if exists('g:plugs["nvim-lspconfig"]')
  if !empty(glob(g:plugs['nvim-lspconfig'].dir.'/lua/lspconfig.lua'))
    lua require('plugins.lspconfig')
  endif
endif
if exists('g:plugs["nvim-treesitter"]')
  if !empty(glob(g:plugs['nvim-treesitter'].dir.'/lua/nvim-treesitter.lua'))
    lua require('plugins.treesitter')
  endif
endif
if exists('g:plugs["telescope.nvim"]')
  if !empty(glob(g:plugs['telescope.nvim'].dir.'/lua/telescope/init.lua'))
    lua require('plugins.telescope')
  endif
endif
lua require('plugins.diagnostics')

" Uncomment if CoC is enabled above and Airline integration desired
" if exists('g:plugs["coc.nvim"]')
"   if !empty(glob(g:plugs['coc.nvim'].dir.'/autoload/coc.vim'))
"     lua require('plugins.coc')
"     if exists('g:plugs["vim-airline"]')
"       if !empty(glob(g:plugs['vim-airline'].dir.'/autoload/airline.vim'))
"         let g:airline#extensions#coc#enabled = 1
"         let airline#extensions#coc#error_symbol = 'E:'
"         let airline#extensions#coc#warning_symbol = 'W:'
"         let g:airline#extensions#coc#show_coc_status = 1
"         let airline#extensions#coc#stl_format_err = '%C(L%L)'
"         let airline#extensions#coc#stl_format_warn = '%C(L%L)'
"       endif
"     endif
"   endif
" endif

" Use the :Cheatsheet command which automatically uses Telescope
" if installed or falls back to showing all the cheatsheet files
" concatenated in a floating window. A default mapping <leader>?
" is provided for :Cheatsheet (not bound if already in use).
"
" Default cheatsheet configuration:
if exists('g:plugs["cheatsheet.nvim"]')
  if !empty(glob(g:plugs['cheatsheet.nvim'].dir.'/plugin/cheatsheet.vim'))
    lua require('plugins.cheatsheet')
  endif
endif

if has("termguicolors")
  let &t_8f = "\<Esc>[38;2;%lu;%lu;%lum"
  let &t_8b = "\<Esc>[48;2;%lu;%lu;%lum"
  set termguicolors
  if exists('g:plugs["nvim-colorizer.lua"]')
    if !empty(glob(g:plugs['nvim-colorizer.lua'].dir.'/lua/colorizer.lua'))
      lua require('colorizer').setup()
    endif
  endif
endif
if exists('g:plugs["nvim-dap"]')
  if !empty(glob(g:plugs['nvim-dap'].dir.'/lua/nvim-dap/plugin/dap.lua'))
    lua require('plugins.dap')
  endif
endif
if exists('g:plugs["nvim-dap-virtual-text"]')
  if !empty(glob(g:plugs['nvim-dap-virtual-text'].dir.'/lua/nvim-dap-virtual-text.lua'))
    lua require('plugins.dap-virtual-text')
  endif
endif
if exists('g:plugs["nvim-web-devicons"]')
  if !empty(glob(g:plugs['nvim-web-devicons'].dir.'/lua/nvim-web-devicons.lua'))
    lua require('plugins.devicons')
  endif
endif
if exists('g:plugs["fidget.nvim"]')
  if !empty(glob(g:plugs['fidget.nvim'].dir.'/lua/fidget.lua'))
    lua require('fidget').setup{}
  endif
endif
if exists('g:plugs["neotest"]')
  if !empty(glob(g:plugs['neotest'].dir.'/lua/neotest/init.lua'))
    lua require('plugins.neotest')
  endif
endif
if exists('g:plugs["go.nvim"]')
  if !empty(glob(g:plugs['go.nvim'].dir.'/lua/go.lua'))
    lua require('plugins.go')
  endif
endif
if exists('g:plugs["gitsigns.nvim"]')
  if !empty(glob(g:plugs['gitsigns.nvim'].dir.'/lua/gitsigns.lua'))
    lua require('gitsigns').setup()
  endif
endif
if exists('g:plugs["inlay-hints.nvim"]')
  if !empty(glob(g:plugs['inlay-hints.nvim'].dir.'/lua/inlay-hints/init.lua'))
    lua require('inlay-hints').setup()
  endif
endif
if exists('g:plugs["rust-tools.nvim"]')
  if !empty(glob(g:plugs['rust-tools.nvim'].dir.'/lua/rust-tools/init.lua'))
    lua require('plugins.rust-tools')
  endif
endif
if exists('g:plugs["lualine.nvim"]')
  if !empty(glob(g:plugs['lualine.nvim'].dir.'/lua/lualine.lua'))
    lua require('plugins.lualine')
  endif
endif
if exists('g:plugs["tabline.nvim"]')
  if !empty(glob(g:plugs['tabline.nvim'].dir.'/lua/tabline.lua'))
    lua require('plugins.tabline')
  endif
endif
if exists('g:plugs["neo-tree.nvim"]')
  if !empty(glob(g:plugs['neo-tree.nvim'].dir.'/lua/neo-tree.lua'))
    let g:neo_tree_remove_legacy_commands = 1
    lua require('plugins.neo-tree')
  endif
endif
if exists('g:plugs["project.nvim"]')
  if !empty(glob(g:plugs['project.nvim'].dir.'/lua/project_nvim/init.lua'))
    lua require('plugins.project')
  endif
endif
if exists('g:plugs["alpha-nvim"]')
  if !empty(glob(g:plugs['alpha-nvim'].dir.'/lua/alpha.lua'))
    lua require('plugins.alpha.alpha')
  endif
endif
if exists('g:plugs["asciiart.nvim"]')
  if !empty(glob(g:plugs['asciiart.nvim'].dir.'/lua/asciiart/init.lua'))
    lua require('plugins.asciiart')
  endif
endif
"
" To use the Startify dashboard rather than Alpha, uncomment this line,
" comment the Alpha entries, and uncomment the startify Plug entry above
" source $HOME/.config/nvim/reference/startify.vim
"
if exists('g:plugs["toggleterm.nvim"]')
  if !empty(glob(g:plugs['toggleterm.nvim'].dir.'/lua/toggleterm.lua'))
    lua require('toggleterm').setup()
  endif
endif
if exists('g:plugs["which-key.nvim"]')
  if !empty(glob(g:plugs['which-key.nvim'].dir.'/lua/which-key/init.lua'))
    lua require('plugins.which-key')
  endif
endif
if exists('g:plugs["nvim-navic"]')
  if !empty(glob(g:plugs['nvim-navic'].dir.'/lua/nvim-navic/init.lua'))
    lua require('plugins.navic')
  endif
endif
" if exists('g:plugs["noice.nvim"]')
"   if !empty(glob(g:plugs['noice.nvim'].dir.'/lua/noice/init.lua'))
"     lua require('plugins.noice')
"   endif
" endif
if exists('g:plugs["lsp_signature.nvim"]')
  if !empty(glob(g:plugs['lsp_signature.nvim'].dir.'/lua/lsp_signature/init.lua'))
    lua require('plugins.signature')
  endif
endif
if exists('g:plugs["null-ls.nvim"]')
  if !empty(glob(g:plugs['null-ls.nvim'].dir.'/lua/null-ls/init.lua'))
    lua require('plugins.null-ls')
  endif
endif
if exists('g:plugs["auto-session"]')
  if !empty(glob(g:plugs['auto-session'].dir.'/lua/auto-session.lua'))
    lua require('plugins.autosession')
  endif
endif
if exists('g:plugs["trouble.nvim"]')
  if !empty(glob(g:plugs['trouble.nvim'].dir.'/lua/trouble/init.lua'))
    lua require('plugins.trouble')
  endif
endif
if exists('g:plugs["ChatGPT.nvim"]')
  if !empty(glob(g:plugs['ChatGPT.nvim'].dir.'/plugin/chatgpt.lua'))
    lua require('chatgpt').setup()
  endif
endif
if exists('g:plugs["zen-mode.nvim"]')
  if !empty(glob(g:plugs['zen-mode.nvim'].dir.'/plugin/zen-mode.vim'))
    lua require('plugins.zen-mode')
  endif
endif
