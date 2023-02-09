""" Vim compatible NeoVim init
"
"   Copy this file to ~/.config/nvim/init.vim to enable
"   or invoke NeoVim as: nvim -u ~/.config/nvim/init-compat.vim
"   This NeoVim config will source your existing Vim ~/.vimrc
"
set runtimepath^=~/.vim runtimepath+=~/.vim/after
let &packpath = &runtimepath
" Use my existing .vimrc
source ~/.vimrc
" Exit NeoVim's terminal emulator (:term) by simply pressing escape
tmap <Esc> <C-\><C-n>
" Override those configurations for each project with a local .nvimrc
set exrc
" Allow scripts to run for specific file types
:filetype plugin on

