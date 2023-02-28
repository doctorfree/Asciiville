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

" Uncomment and set OPENAI_API_KEY env var to enable :ChatGPT and :ChatGPTActAs
" See https://github.com/jackMort/ChatGPT.nvim for setup options and usage
" Plug 'jackMort/ChatGPT.nvim'

" Register vim-plug as a plugin to enable help  (e.g. :help plug-options)
Plug 'junegunn/vim-plug'

" Functionalities - Python
Plug 'psf/black', { 'branch': 'stable' }
Plug 'heavenshell/vim-pydocstring'
Plug 'davidhalter/jedi-vim'    " Python autocompletion
Plug 'klen/python-mode'        " Python IDE
let g:pymode = 1
let g:pymode_warnings = 1

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
" If using Airline, colorschemes must have a matching Airline theme with
" the same name. Currently available colorschemes with matching Airline theme:
" alduin angr apprentice badwolf behelit biogoo bubblegum cobalt2 cool cyberpunk
" desertink deus distinguished fairyfloss hybrid jellybeans kalisi kolor laederon
" lucius luna minimalist molokai monochrome onedark peaksea seagull seoul256
" sierra soda solarized sol transparent ubaryd understated wombat zenburn
let g:mycolorschemes = ['asciiville', 'everforest', 'cool', 'desertink', 'distinguished', 'hybrid', 'luna', 'molokai', 'solarized', 'zenburn']
" Set this to 1 if using Airline, 0 with Lualine
let g:setairlinetheme = 0

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

call plug#end()

" General "{{{
set backspace=indent
set backspace+=eol
set backspace+=start
set history=256 " Number of things to remember in history
set ruler       " Show the cursor position all the time
set showcmd     " Display an incomplete command in statusline
if has('mouse')
  set mouse=a   " Enable mouse in GUI mode
  set mousehide " Hide mouse after chars typed
endif

scriptencoding utf-8           " UTF-8 all the way
set encoding=utf-8

set timeoutlen=300             " Time to wait after ESC (default causes delay)
set clipboard+=unnamed         " Yanks go on clipboard instead.
set pastetoggle=<F10>          " Toggle between paste and normal: pasting from keyboard
set shiftround                 " Round indent to multiple of 'shiftwidth'
set tags=.git/tags;$HOME       " Consider the repo tags first, then
                               " Walk directory tree upto $HOME looking for tags
                               " Note `;` sets the stop folder. :h file-search
set modeline
set modelines=5                " Default number of lines to read for modeline
set autowrite                  " Writes on make/shell commands
set autoread

set nobackup
set nowritebackup
set directory=/tmp//           " Prepend(^=) $HOME/.tmp/ to default path
                               " Use full path as backup filename(//)
set noswapfile                 "

set hidden                     " Current buffer to background without writing to disk

syntax on                      " Enable syntax
set incsearch                  " Show matches while typing
set ignorecase                 " Be case insensitive when searching
set smartcase                  " Be case sensitive when input has a capital letter
set hlsearch                   " Highlight search

let g:is_posix = 1             " Vim's default is Bourne shell, bring it up to the 90s
let g:netrw_banner = 0         " Do not show Netrw help banner

" complete longest common string, then list alternatives.
set wildmode=longest,list
" Use wilder, see https://github.com/gelguy/wilder.nvimrc
" for extensive set of configuration examples
" When in : cmdline mode, wildmenu suggestions will be automatically provided.
" When searching using /, suggestions from the current buffer will be provided.
" Substring matching is used by default.
"
" Use <Tab> to cycle through the list forwards, and <S-Tab> to move backwards.
"
" Airline and Lightline users:
" wilder#wildmenu_airline_theme() and wilder#wildmenu_lightline_theme() can be used.
"
if exists('g:plugs["wilder.nvim"]')
  if !empty(glob(g:plugs['wilder.nvim'].dir.'/lua/wilder.lua'))
    lua require('config.wilder')
  endif
endif

" Formatting "{{{
set fo+=o  " Insert the current comment leader after 'o' or 'O' in Normal mode.
set fo-=r  " Do not automatically insert a comment leader after an enter
set fo-=t  " Do no auto-wrap text using textwidth (does not apply to comments)

" set nowrap
" set textwidth=0              " Don't wrap lines by default

set tabstop=2                  " Tab size eql 2 spaces
set softtabstop=2
set shiftwidth=2               " Default shift width for indents
set expandtab                  " Replace tabs with ${tabstop} spaces
set smarttab                   "

set cindent
set indentkeys-=0#             " Do not break indent on #
set cinkeys-=0#
set cinoptions=:s,ps,ts,cs
set cinwords=if,else,while,do
set cinwords+=for,switch,case
" "}}}

" Visual "{{{
set synmaxcol=250             " limit syntax highlighting to 250 columns

set nonumber                  " line numbers Off
set showmatch                 " Show matching brackets.
set matchtime=2               " Bracket blinking.

set novisualbell              " No blinking
set noerrorbells              " No noise.
set vb t_vb=                  " Disable any beeps or flashes on error

" set laststatus=2            " Always show status line.
" set shortmess=atI           " Shortens messages

set statusline=%<%f\          " Custom statusline
set stl+=[%{&ff}]             " Show fileformat
set stl+=%y%m%r%=
set stl+=%-14.(%l,%c%V%)\ %P

set foldenable                " Turn on folding
set foldmethod=marker         " Fold on the marker
set foldlevel=100             " Don't autofold anything (but still fold manually)

set foldopen=block,hor,tag    " What movements open folds
set foldopen+=percent,mark
set foldopen+=quickfix

set virtualedit=block

set splitbelow
set splitright

" set list                    " Display unprintable characters F12 - switches
set listchars=tab:\ ·,eol:¬
set listchars+=trail:·
set listchars+=extends:»,precedes:«
map <silent> <F12> :set invlist<CR>

if has('gui_running')
  set guioptions=cMg " Console dialogs, do not show menu and toolbar

  " Fonts
  " :set guifont=* " To launch a GUI dialog
  if has('mac')
    if has('macligatures')
      set antialias macligatures guifont=Fira\ Code\ Light:h13 " -> <=
    else
      set noantialias guifont=Andale\ Mono:h14
    end
  set fuoptions=maxvert,maxhorz ",background:#00AAaaaa
  else
  set guifont=Terminus:h16
  end
endif
" "}}}

""" Filetype-Specific Configurations

" CSS, HTML, LUA, JS, TS, XML, Jinja, YAML
autocmd FileType lua setlocal shiftwidth=2 tabstop=2 softtabstop=2
autocmd FileType html setlocal shiftwidth=2 tabstop=2 softtabstop=2
autocmd FileType javascript setlocal shiftwidth=2 tabstop=2 softtabstop=2
autocmd FileType typescript setlocal shiftwidth=2 tabstop=2 softtabstop=2
autocmd FileType css setlocal shiftwidth=2 tabstop=2 softtabstop=2
autocmd FileType scss setlocal shiftwidth=2 tabstop=2 softtabstop=2
autocmd FileType xml setlocal shiftwidth=2 tabstop=2 softtabstop=2
autocmd FileType xhtml setlocal shiftwidth=2 tabstop=2 softtabstop=2
autocmd FileType yaml setlocal shiftwidth=2 tabstop=2 softtabstop=2
autocmd FileType htmldjango setlocal shiftwidth=2 tabstop=2 softtabstop=2
autocmd FileType htmldjango inoremap {{ {{  }}<left><left><left>
autocmd FileType htmldjango inoremap {% {%  %}<left><left><left>
autocmd FileType htmldjango inoremap {# {#  #}<left><left><left>

" Markdown and Journal
autocmd FileType markdown setlocal shiftwidth=2 tabstop=2 softtabstop=2
autocmd FileType journal setlocal shiftwidth=2 tabstop=2 softtabstop=2

set completeopt=menu,menuone,noselect

" signify
let g:signify_sign_add = '│'
let g:signify_sign_delete = '│'
let g:signify_sign_change = '│'
hi DiffDelete guifg=#ff5555 guibg=none

" FixCursorHold for better performance
let g:updatetime = 200

" context.vim
let g:context_nvim_no_redraw = 1

" Neovim :Terminal
"
" Exit Neovim's terminal emulator (:term) by simply pressing escape
tmap <Esc> <C-\><C-n>
tmap <C-w> <Esc><C-w>
"tmap <C-d> <Esc>:q<CR>
autocmd BufWinEnter,WinEnter term://* startinsert
autocmd BufLeave term://* stopinsert

" The Homebrew installation location
if empty($HOMEBREW_PREFIX)
  if isdirectory("/home/linuxbrew/.linuxbrew")
    let g:homebrew_install_dir = "/home/linuxbrew/.linuxbrew"
  else
    if isdirectory("/opt/homebrew")
      let g:homebrew_install_dir = "/opt/homebrew"
    else
      if isdirectory("/usr/local")
        let g:homebrew_install_dir = "/usr/local"
      else
        let g:homebrew_install_dir = ""
      endif
    endif
  endif
else
  let g:homebrew_install_dir = $HOMEBREW_PREFIX
endif

" Replace these with actual paths
if !empty(glob('/path/to/python3'))
  let g:python3_host_prog = '/path/to/python3'
endif
if !empty(glob('/path/to/doq'))
  let g:pydocstring_doq_path = '/path/to/doq'
endif

" Need to convifure Neodev prior to LSP
if exists('g:plugs["neodev.nvim"]')
  if !empty(glob(g:plugs['neodev.nvim'].dir.'/lua/neodev/init.lua'))
    lua require('config.neodev')
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

require('config.nvim-cmp')
require('config.lspconfig')
require('config.treesitter')
require('config.telescope')
require('config.diagnostics')
EOF
  endif
endif

" Uncomment if CoC is enabled above and Airline integration desired
" if exists('g:plugs["coc.nvim"]')
"   if !empty(glob(g:plugs['coc.nvim'].dir.'/autoload/coc.vim'))
"     lua require('config.coc')
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
    lua require('config.dap')
  endif
endif
if exists('g:plugs["nvim-dap-virtual-text"]')
  if !empty(glob(g:plugs['nvim-dap-virtual-text'].dir.'/lua/nvim-dap-virtual-text.lua'))
    lua require('config.dap-virtual-text')
  endif
endif
if exists('g:plugs["nvim-web-devicons"]')
  if !empty(glob(g:plugs['nvim-web-devicons'].dir.'/lua/nvim-web-devicons.lua'))
    lua require('config.devicons')
  endif
endif
if exists('g:plugs["fidget.nvim"]')
  if !empty(glob(g:plugs['fidget.nvim'].dir.'/lua/fidget.lua'))
    lua require('fidget').setup{}
  endif
endif
if exists('g:plugs["neotest"]')
  if !empty(glob(g:plugs['neotest'].dir.'/lua/neotest/init.lua'))
    lua require('config.neotest')
  endif
endif
if exists('g:plugs["go.nvim"]')
  if !empty(glob(g:plugs['go.nvim'].dir.'/lua/go.lua'))
    lua require('config.go')
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
    lua require('config.rust-tools')
  endif
endif
if exists('g:plugs["lualine.nvim"]')
  if !empty(glob(g:plugs['lualine.nvim'].dir.'/lua/lualine.lua'))
    lua require('config.lualine')
  endif
endif
if exists('g:plugs["tabline.nvim"]')
  if !empty(glob(g:plugs['tabline.nvim'].dir.'/lua/tabline.lua'))
    lua require('config.tabline')
  endif
endif
if exists('g:plugs["neo-tree.nvim"]')
  if !empty(glob(g:plugs['neo-tree.nvim'].dir.'/lua/neo-tree.lua'))
    let g:neo_tree_remove_legacy_commands = 1
    lua require('config.neo-tree')
  endif
endif
if exists('g:plugs["project.nvim"]')
  if !empty(glob(g:plugs['project.nvim'].dir.'/lua/project_nvim/init.lua'))
    lua require('config.project')
  endif
endif
if exists('g:plugs["alpha-nvim"]')
  if !empty(glob(g:plugs['alpha-nvim'].dir.'/lua/alpha.lua'))
    lua require('config.alpha')
  endif
endif
if exists('g:plugs["asciiart.nvim"]')
  if !empty(glob(g:plugs['asciiart.nvim'].dir.'/lua/asciiart/init.lua'))
    lua require('config.asciiart')
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
    lua require('config.which-key')
  endif
endif
if exists('g:plugs["nvim-navic"]')
  if !empty(glob(g:plugs['nvim-navic'].dir.'/lua/nvim-navic/init.lua'))
    lua require('config.navic')
  endif
endif
" if exists('g:plugs["noice.nvim"]')
"   if !empty(glob(g:plugs['noice.nvim'].dir.'/lua/noice/init.lua'))
"     lua require('config.noice')
"   endif
" endif
if exists('g:plugs["lsp_signature.nvim"]')
  if !empty(glob(g:plugs['lsp_signature.nvim'].dir.'/lua/lsp_signature/init.lua'))
    lua require('config.signature')
  endif
endif
if exists('g:plugs["null-ls.nvim"]')
  if !empty(glob(g:plugs['null-ls.nvim'].dir.'/lua/null-ls/init.lua'))
    lua require('config.null-ls')
  endif
endif
if exists('g:plugs["trouble.nvim"]')
  if !empty(glob(g:plugs['trouble.nvim'].dir.'/lua/trouble/init.lua'))
    lua require('config.trouble')
  endif
endif
if exists('g:plugs["ChatGPT.nvim"]')
  if !empty(glob(g:plugs['ChatGPT.nvim'].dir.'/plugin/chatgpt.lua'))
    lua require('chatgpt').setup()
  endif
endif
if exists('g:plugs["zen-mode.nvim"]')
  if !empty(glob(g:plugs['zen-mode.nvim'].dir.'/plugin/zen-mode.vim'))
    lua require('config.zen-mode')
  endif
endif

""" Custom Mappings (lua custom mappings are within individual lua config files)
"
" Core
nmap <leader>q :NvimTreeFindFileToggle<CR>
nmap \ <leader>q
nmap <leader>r :so ~/.config/nvim/init.vim<CR>
xmap <leader>a gaip*
nmap <leader>a gaip*
nmap <leader>h :RainbowParentheses!!<CR>
nmap <leader>j :set filetype=journal<CR>
" nmap <leader>k :ColorToggle<CR>
nmap <leader>l :Limelight!!<CR>
xmap <leader>l :Limelight!!<CR>
nmap <silent> <leader><leader> :noh<CR>
nmap <Tab> :bnext<CR>
nmap <S-Tab> :bprevious<CR>
nmap <leader>$s <C-w>s<C-w>j:terminal<CR>:set nonumber<CR><S-a>
nmap <leader>$v <C-w>v<C-w>l:terminal<CR>:set number<CR><S-a>

" Python
autocmd Filetype python nmap <leader>d <Plug>(pydocstring)
autocmd FileType python nmap <leader>p :Black<CR>

" Telescope mappings
nnoremap <leader>ff <cmd>Telescope find_files<cr>
nnoremap <leader>fg <cmd>Telescope live_grep<cr>
nnoremap <leader>fb <cmd>Telescope buffers<cr>
nnoremap <leader>fh <cmd>Telescope help_tags<cr>
nnoremap <leader>fc <cmd>Telescope colorscheme<cr>
nnoremap <leader>f/ <cmd>Telescope current_buffer_fuzzy_find<cr>

" Override those configurations for each project with a local .nvimrc
set exrc

if has("autocmd")
  filetype plugin indent on
  augroup vimrcEx
  au!
  autocmd FileType text setlocal textwidth=78
  autocmd BufReadPost *
    \ if line("'\"") > 1 && line("'\"") <= line("$") |
    \   exe "normal! g`\"" |
    \ endif
  augroup END
  autocmd FileType * setlocal formatoptions-=c formatoptions-=r
else
  set autoindent    " always set autoindenting on
endif " has("autocmd")

if !exists(":DiffOrig")
  command DiffOrig vert new | set bt=nofile | r # | 0d_ | diffthis | wincmd p | diffthis
endif

" If your terminal doesn't handle italics, bold, underline, or undercurl
" then they can be disabled in the asciiville colorscheme with:
" let g:asciiville_italic = 0
" let g:asciiville_bold = 0
" let g:asciiville_underline = 0
" let g:asciiville_undercurl = 0
" Comment out to use everforest or tokyonight below
colorscheme asciiville
let g:asciiville_style = "deep ocean"
" Asciiville colorscheme commands:
" :AsciivilleDarkBlueSoft
" :AsciivilleDarkCyanSoft
" :AsciivilleDarkCyanHard
" :AsciivilleNightOrangeSoft
" :AsciivilleNightOrangeHard
" :AsciivilleNightRedSoft
" :AsciivilleNightRedHard
" :AsciivilleLightSoft
" :AsciivilleLightHard
if exists(":AsciivilleDarkBlueHard")
  AsciivilleDarkBlueHard
endif

" For dark version.
set background=dark
" For light version.
" set background=light

" Set contrast.
" This configuration option should be placed before `colorscheme everforest`.
" Available values: 'hard', 'medium'(default), 'soft'
let g:everforest_background = 'hard'

" For better performance
let g:everforest_better_performance = 1
" For a transparent background, set to 2 for status line transparency as well
let g:everforest_transparent_background = 1
" Dim inactive windows
let g:everforest_dim_inactive_windows = 1

" Uncomment to use the Everforest colorscheme
" colorscheme everforest
"
" Uncomment to use the Tokyonight colorscheme
" colorscheme tokyonight-night
"
let g:syntastic_html_checkers = []

if exists(':GuiFont')
  GuiFont! JetBrains Mono:h22
endif
