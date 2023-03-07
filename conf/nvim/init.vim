"""
"      _                      _    _           _   __   __
"     / \                    (_)  (_)         (_) [  | [  |
"    / _ \     .--.   .---.  __   __  _   __  __   | |  | | .---.
"   / ___ \   ( (`\] / /'`\][  | [  |[ \ [  ][  |  | |  | |/ /__\\
" _/ /   \ \_  `'.'. | \__.  | |  | | \ \/ /  | |  | |  | || \__.,
"|____| |____|[\__) )'.___.'[___][___] \__/  [___][___][___]'.__.'
"
"-----------------Neovim Initialization Vimscript----------------
"
"  Version : 1.0.0
"  License : MIT
"  Author  : Ronald Record
"  URL     : https://github.com/doctorfree/nvim
"  Project : https://github.com/doctorfree/Asciiville
"----------------------------------------------------------------
" Set ',' as the leader key
" See `:help mapleader`
" NOTE: Must happen before plugins
let mapleader = ','
let maplocalleader = ','

source ~/.config/nvim/plugins.vim

scriptencoding utf-8           " UTF-8 all the way
syntax on                      " Enable syntax

set vb t_vb=                  " Disable any beeps or flashes on error

set statusline=%<%f\          " Custom statusline
set stl+=[%{&ff}]             " Show fileformat
set stl+=%y%m%r%=
set stl+=%-14.(%l,%c%V%)\ %P

source ~/.config/nvim/config.vim

" Perform any necessary colorscheme setup
lua require("themes.init")

" Set global variables
lua require("globals")

" Plugin management via lazy
" require("lazy-init")
"
" Plugin management via Packer
" require("packer")
"
" Global Keymappings
lua require("mappings")

" All non plugin related (vim) options
lua require("options")

" Vim autocommands/autogroups
lua require("autocmd")

if !exists(":DiffOrig")
  command DiffOrig vert new | set bt=nofile | r # | 0d_ | diffthis | wincmd p | diffthis
endif
