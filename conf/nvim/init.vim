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
"
""" Vim-Plug managed plugins
"
" Use ':help vim-plug' or ':help plug-options' for assistance with Vim-Plug

call plug#begin()

" Core (treesitter, nvim-lspconfig, nvim-cmp, nvim-telescope, etc)
Plug 'nvim-treesitter/nvim-treesitter', { 'do': ':TSUpdate' }
Plug 'nvim-treesitter/playground'
Plug 'neovim/nvim-lspconfig'
Plug 'hrsh7th/cmp-nvim-lsp'
Plug 'hrsh7th/cmp-buffer'
Plug 'hrsh7th/cmp-path'
Plug 'hrsh7th/cmp-cmdline'
Plug 'hrsh7th/nvim-cmp'    " A completion engine plugin for Neovim
Plug 'tamago324/cmp-zsh'   " Zsh completion for cmp
Plug 'Shougo/deol.nvim'    " Recommended to use together
Plug 'jose-elias-alvarez/null-ls.nvim' " inject LSP diagnostics and more via Lua
" Snippets
" Plug 'L3MON4D3/LuaSnip', {'tag': 'v1.*', 'do': 'make install_jsregexp'}
Plug 'L3MON4D3/LuaSnip', {'tag': 'v1.*'}
Plug 'saadparwaiz1/cmp_luasnip'
Plug 'rafamadriz/friendly-snippets'
Plug 'gmarik/snipmate.vim'      " TextMate's snippets features in Vim
Plug 'gmarik/snipmate.snippets' " gmarik's custom snippet collection
" Plug 'hrsh7th/cmp-vsnip'
" Plug 'hrsh7th/vim-vsnip'
Plug 'hrsh7th/cmp-nvim-lsp-signature-help'
Plug 'nvim-lua/popup.nvim'
Plug 'nvim-lua/plenary.nvim'
" Telescope and extensions
Plug 'nvim-telescope/telescope.nvim'
Plug 'crispgm/telescope-heading.nvim'
Plug 'nvim-telescope/telescope-symbols.nvim'
Plug 'nvim-telescope/telescope-file-browser.nvim'
Plug 'nvim-telescope/telescope-ui-select.nvim'
Plug 'ptethng/telescope-makefile'

Plug 'anuvyklack/hydra.nvim'
Plug 'anuvyklack/keymap-layer.nvim'
Plug 'ahmedkhalf/project.nvim'

Plug 'nvim-tree/nvim-web-devicons'
Plug 'nvim-neo-tree/neo-tree.nvim', { 'branch': 'v2.x' }
Plug 'SmiteshP/nvim-navic'
Plug 'jvgrootveld/telescope-zoxide'
" Plug 'folke/noice.nvim'
Plug 'rcarriga/nvim-notify'

" A pretty list for showing diagnostics, references, telescope results, quickfix
" and location lists to help you solve all the trouble your code is causing
Plug 'folke/trouble.nvim'

" Debug adapter
Plug 'mfussenegger/nvim-dap'
Plug 'rcarriga/nvim-dap-ui'
Plug 'theHamsta/nvim-dap-virtual-text'

" Functionalities
Plug 'tpope/vim-fugitive'
Plug 'tpope/vim-sensible'
Plug 'mhinz/vim-signify'
Plug 'alvan/vim-closetag'
Plug 'tpope/vim-abolish'
Plug 'bogado/file-line'        " Enable opening a file in a given line
                               " vim index.html:20
                               " vim app/models/user.rb:1337
Plug 'tpope/vim-sleuth'        " Automatically adjust indentation

Plug 'folke/neodev.nvim'
Plug 'j-hui/fidget.nvim'
Plug 'simrat39/inlay-hints.nvim'
Plug 'camilledejoye/nvim-lsp-selection-range'
Plug 'simrat39/rust-tools.nvim'
Plug 'mrcjkb/haskell-tools.nvim', { 'branch': '1.x.x' }
Plug 'mfussenegger/nvim-jdtls'
Plug 'antoinemadec/FixCursorHold.nvim'
Plug 'nvim-neotest/neotest'
Plug 'nvim-neotest/neotest-python'
Plug 'nvim-neotest/neotest-plenary'
Plug 'nvim-neotest/neotest-vim-test'

" CoC Nodejs extension host
" Load extensions like VSCode and host language servers
" Commented out for now, using lspconfig and installed language servers
" See CoC wiki at https://github.com/neoclide/coc.nvim/wiki
" Plug 'neoclide/coc.nvim', {'branch': 'release'}
" or
" Plug 'neoclide/coc.nvim', {'branch': 'release', 'build': ':CocUpdate'}
" Install CoC extensions or configure language servers
" Plug 'rafcamlet/coc-nvim-lua'
" Plug 'neoclide/coc-tsserver', {'do': 'yarn install --frozen-lockfile'}
" Plug 'coc-java'
" Plug 'neoclide/coc-rust-analyzer'
" Plug 'neoclide/coc-css'
" Plug 'neoclide/coc-vimlsp'
" Plug 'neoclide/coc-snippets'
" Plug 'neoclide/coc-emmet'
" Plug 'neoclide/coc-json'
" Plug 'neoclide/coc-texlab'
" let g:coc_disable_startup_warning = 1

Plug 'jackguo380/vim-lsp-cxx-highlight' " C/C++/Cuda/ObjC semantic highlighting
Plug 'junegunn/vim-easy-align' " A simple, easy-to-use Vim alignment plugin
" Start interactive EasyAlign in visual mode (e.g. vipga)
"   xmap ga <Plug>(EasyAlign)
" Start interactive EasyAlign for a motion/text object (e.g. gaip)
"   nmap ga <Plug>(EasyAlign)
Plug 'scrooloose/nerdcommenter'
Plug 'KabbAmine/vCoolor.vim'
Plug 'RRethy/vim-illuminate'
Plug 'lewis6991/gitsigns.nvim'
Plug 'tpope/vim-git'            " Syntax, indent, and filetype for Git
" Git integration - :Git (or just :G) calls any arbitrary Git command
Plug 'junegunn/gv.vim'      " A git commit browser (requires vim-fugitive)
" :GV to open commit browser
"     You can pass git log options to the command, e.g. :GV -S foobar -- plugins
" :GV! will only list commits that affected the current file
" :GV? fills the location list with the revisions of the current file
Plug 'nvim-lualine/lualine.nvim'
Plug 'kdheepak/tabline.nvim'
" Plug 'vim-airline/vim-airline' " Nifty status of your current file
" let g:airline#extensions#tabline#enabled = 1
" let g:bufferline_echo = 0
" let g:airline_powerline_fonts = 1
"
" let g:airline#extensions#default#layout = [
"     \ [ 'a', 'b', 'c' ],
"     \ [ 'x', 'y', 'z']
"     \ ]
" Plug 'vim-airline/vim-airline-themes' " Airline status themes
" let g:airline_theme='asciiville'
Plug 'fladson/vim-kitty' " Kitty config syntax highlighting for vim
" Language support
Plug 'ray-x/lsp_signature.nvim'
Plug 'ray-x/guihua.lua'        " Floating window support
Plug 'ray-x/go.nvim'           " Go language support for Neovim
Plug 'yuezk/vim-js'            " Syntax highlighting for JavaScript and Flow.js
Plug 'leafgarland/typescript-vim' " Typescript syntax
" To disable built-in Typescript indentation:
" let g:typescript_indent_disable = 1
Plug 'maxmellon/vim-jsx-pretty' " The React syntax highlighting and indenting
Plug 'tpope/vim-repeat'     " Remaps '.' to repeat the last plugin map as a whole
Plug 'AndrewRadev/splitjoin.vim' " Switch between single-line and multiline
Plug 'gmarik/github-search.vim'  " Search Github and clone repos with Vim
Plug 'gmarik/ide-popup.vim' " Make Vim completion popup menu work like in an IDE
Plug 'lambdalisue/suda.vim' " Alternative sudo for vim
" Re-open a current file with sudo
" :SudaRead
" Open /etc/sudoers with sudo
" :SudaRead /etc/sudoers
" Forcedly save a current file with sudo
" :SudaWrite
" Write contents to /etc/profile
" :SudaWrite /etc/profile
" Plug 'ctrlpvim/ctrlp.vim'  " Fuzzy file, buffer, mru, tag finder for Vim
" Plug 'junegunn/fzf', { 'do': { -> fzf#install() } }
" Plug 'junegunn/fzf.vim'    " Things you can do with fzf and Vim
Plug 'nvim-telescope/telescope-fzf-native.nvim', { 'do': 'make' }
Plug 'romgrk/fzy-lua-native' " Needed for lua_fzy_highlighter in wilder
Plug 'sheerun/vim-polyglot'  " Better syntax highlighting
Plug 'folke/which-key.nvim'  " Easily find key map bindings
" See https://github.com/akinsho/toggleterm.nvim for toggleterm setup options
Plug 'akinsho/toggleterm.nvim', {'tag' : '*'}
Plug 'folke/zen-mode.nvim'
Plug 'MunifTanjim/nui.nvim'

" Set OPENAI_API_KEY environment var to enable :ChatGPT and :ChatGPTActAs
" See https://github.com/jackMort/ChatGPT.nvim for setup options and usage
Plug 'jackMort/ChatGPT.nvim'

" Register vim-plug as a plugin to enable help  (e.g. :help plug-options)
Plug 'junegunn/vim-plug'

" Functionalities - Python
Plug 'psf/black', { 'branch': 'stable' }
Plug 'heavenshell/vim-pydocstring'
Plug 'davidhalter/jedi-vim'    " Python autocompletion
Plug 'klen/python-mode'        " Python IDE

" Aesthetics - Colorschemes
Plug 'norcalli/nvim-colorizer.lua'
Plug 'zaki/zazen'
Plug 'yuttie/hydrangea-vim'
Plug 'flazz/vim-colorschemes'  " One stop shop for vim colorschemes
Plug 'doctorfree/asciiville.nvim'
Plug 'doctorfree/asciiart.nvim'
Plug 'm00qek/baleia.nvim', { 'tag': 'v1.2.0' }
Plug 'sainnhe/everforest'
Plug 'catppuccin/nvim'
Plug 'EdenEast/nightfox.nvim'
Plug 'folke/tokyonight.nvim'
Plug 'sam4llis/nvim-tundra'
" Uncomment to play with colorschemes
Plug 'doctorfree/SetColorSchemes.vim' " Easily switch colorschemes

" Aesthetics - Others
if has('nvim')
  function! UpdateRemotePlugins(...)
    " Needed to refresh runtime files
    let &rtp=&rtp
    UpdateRemotePlugins
  endfunction

  Plug 'gelguy/wilder.nvim', { 'do': function('UpdateRemotePlugins') }
else
  Plug 'gelguy/wilder.nvim'

  " To use Python remote plugin features in Vim, can be skipped
  Plug 'roxma/nvim-yarp'
  Plug 'roxma/vim-hug-neovim-rpc'
endif

Plug 'junegunn/rainbow_parentheses.vim'
Plug 'junegunn/limelight.vim'
Plug 'junegunn/vim-journal'
Plug 'goolord/alpha-nvim'
" Plug 'mhinz/vim-startify'

" Cellular automata animations based on the content of neovim buffer
" https://github.com/Eandrju/cellular-automaton.nvim
" Trigger it using the command:
"   :CellularAutomaton make_it_rain
" or
"   :CellularAutomaton game_of_life
" or create a mapping:
"   vim.keymap.set("n", "<leader>fml", "<cmd>CellularAutomaton make_it_rain<CR>")
" Close animation window with one of: q/<Esc>/<CR>
Plug 'eandrju/cellular-automaton.nvim'

" Cheat sheets
Plug 'sudormrfbin/cheatsheet.nvim'  " :Cheatsheet

" Media preview in Telescope using ueberzug/jp2a/etc
Plug 'dharmx/telescope-media.nvim'

call plug#end()

scriptencoding utf-8           " UTF-8 all the way
syntax on                      " Enable syntax

if exists('g:plugs["wilder.nvim"]')
  if !empty(glob(g:plugs['wilder.nvim'].dir.'/lua/wilder.lua'))
    lua require('plugins.wilder')
  endif
endif

set vb t_vb=                  " Disable any beeps or flashes on error

set statusline=%<%f\          " Custom statusline
set stl+=[%{&ff}]             " Show fileformat
set stl+=%y%m%r%=
set stl+=%-14.(%l,%c%V%)\ %P

" Need to configure Neodev prior to LSP
if exists('g:plugs["neodev.nvim"]')
  if !empty(glob(g:plugs['neodev.nvim'].dir.'/lua/neodev/init.lua'))
    lua require('plugins.neodev')
  endif
endif
""" Core plugin configuration (lua)
" Add these:  cssmodules ansible haskell sql
if exists('g:plugs["nvim-treesitter"]')
  if !empty(glob(g:plugs['nvim-treesitter'].dir.'/autoload/nvim_treesitter.vim'))
    lua << EOF
servers = {
    "pyright",
    -- LSP
    "awk_ls",
    "bashls",
    "dockerfile-language-server",
    "typescript-language-server",
    "lua_ls",
    "vimls",
    "texlab",
    "jsonls",
    "yamlls",
    -- Formatter
    "black",
    "prettier",
    "stylua",
    "shfmt",
    -- Linter
    "eslint_d",
    "shellcheck",
    "tflint",
    "yamllint",
}

require('plugins.nvim-cmp')
require('plugins.lspconfig')
require('plugins.treesitter')
require('plugins.telescope')
require('plugins.diagnostics')
EOF
  endif
endif

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
    lua << EOF
require('cheatsheet').setup({
    -- Whether to show bundled cheatsheets

    -- For generic cheatsheets like default, unicode, nerd-fonts, etc
    -- bundled_cheatsheets = {
    --     enabled = {},
    --     disabled = {},
    -- },
    bundled_cheatsheets = true,

    -- For plugin specific cheatsheets
    -- bundled_plugin_cheatsheets = {
    --     enabled = {},
    --     disabled = {},
    -- }
    bundled_plugin_cheatsheets = true,

    -- For bundled plugin cheatsheets, do not show a sheet if you
    -- don't have the plugin installed (searches runtimepath for
    -- same directory name)
    include_only_installed_plugins = true,

    -- Key mappings bound inside the telescope window
    telescope_mappings = {
        ['<CR>'] = require('cheatsheet.telescope.actions').select_or_fill_commandline,
        ['<A-CR>'] = require('cheatsheet.telescope.actions').select_or_execute,
        ['<C-Y>'] = require('cheatsheet.telescope.actions').copy_cheat_value,
        ['<C-E>'] = require('cheatsheet.telescope.actions').edit_user_cheatsheet,
    }
})
EOF
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
" source $HOME/.config/nvim/startify.vim
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
