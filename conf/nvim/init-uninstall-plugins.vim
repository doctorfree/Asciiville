""" Use this NeoVim configuration to uninstall all plugins
"
"   Open NeoVim with 'nvim -u ~/.config/nvim/init-uninstall-plugins.vim'
"   Run ':PlugClean'
"
"   After plugins have been removed, reinstall the NeoVim plugins with:
"     nvim -c 'PlugInstall' -c 'qa'
"
""" Vim-Plug managed plugins
"
" Using plug.vim Plugin manager from https://github.com/junegunn/vim-plug
" Commands
" PlugInstall [name ...] [#threads]  Install plugins
" PlugUpdate  [name ...] [#threads]  Install or update plugins
" PlugClean[!]  Remove unlisted plugins (bang version will clean without prompt)
" PlugUpgrade   Upgrade vim-plug itself
" PlugStatus    Check the status of plugins
" PlugDiff      Examine changes from the previous update and the pending changes
" PlugSnapshot[!] [output path]  Generate script for restoring
"                                the current snapshot of the plugins
" The default plugin directory will be as follows:
"   - Vim (Linux/macOS): '~/.vim/plugged'
"   - Vim (Windows): '~/vimfiles/plugged'
"   - Neovim (Linux/macOS/Windows): stdpath('data') . '/plugged'
" You can specify a custom plugin directory by passing it as the argument
"   - e.g. `call plug#begin('~/.vim/plugged')`
"   - Avoid using standard Vim directory names like 'plugin'
" Make sure you use single quotes

" Shorthand notation; fetches https://github.com/junegunn/vim-easy-align
" Plug 'junegunn/vim-easy-align'
" Any valid git URL is allowed
" Plug 'https://github.com/junegunn/vim-github-dashboard.git'
" Multiple Plug commands can be written in a single line using | separators
" Plug 'SirVer/ultisnips' | Plug 'honza/vim-snippets'
" On-demand loading
" Plug 'scrooloose/nerdtree', { 'on':  'NERDTreeToggle' }
" Plug 'tpope/vim-fireplace', { 'for': 'clojure' }
" Using a non-default branch
" Plug 'rdnetto/YCM-Generator', { 'branch': 'stable' }
" Using a tagged release; wildcard allowed (requires git 1.9.2 or above)
" Plug 'fatih/vim-go', { 'tag': '*' }
" Plugin options
" Plug 'nsf/gocode', { 'tag': 'v.20150303', 'rtp': 'vim' }
" Plugin outside ~/.vim/plugged with post-update hook
" Plug 'junegunn/fzf', { 'dir': '~/.fzf', 'do': './install --all' }
" Unmanaged plugin (manually installed and updated)
" Plug '~/my-prototype-plugin'

call plug#begin()

" Core (treesitter, nvim-lspconfig, nvim-cmp, nvim-telescope)
" Plug 'nvim-treesitter/nvim-treesitter', { 'do': ':TSUpdate' }
" Plug 'nvim-treesitter/playground'
" Plug 'neovim/nvim-lspconfig'
" Plug 'hrsh7th/cmp-nvim-lsp'
" Plug 'hrsh7th/cmp-buffer'
" Plug 'hrsh7th/cmp-path'
" Plug 'hrsh7th/cmp-cmdline'
" Plug 'hrsh7th/nvim-cmp'    " A completion engine plugin for neovim
" Plug 'hrsh7th/cmp-vsnip'
" Plug 'hrsh7th/vim-vsnip'
" Plug 'hrsh7th/cmp-nvim-lsp-signature-help'
" Plug 'nvim-lua/popup.nvim'
" Plug 'nvim-lua/plenary.nvim'
" Plug 'nvim-telescope/telescope.nvim'
" Plug 'nvim-telescope/telescope-fzf-native.nvim', { 'do': 'make' }
" Plug 'kyazdani42/nvim-web-devicons'
" Plug 'kyazdani42/nvim-tree.lua'

" Functionalities
" Plug 'tpope/vim-fugitive'
" Plug 'tpope/vim-sensible'
" Plug 'tpope/vim-surround'
" Plug 'mhinz/vim-signify'
" Plug 'jiangmiao/auto-pairs'
" Plug 'alvan/vim-closetag'
" Plug 'tpope/vim-abolish'
" Plug 'bogado/file-line'        " Enable opening a file in a given line
                               " vim index.html:20
                               " vim app/models/user.rb:1337
" Plug 'junegunn/vim-easy-align' " A simple, easy-to-use Vim alignment plugin
" Start interactive EasyAlign in visual mode (e.g. vipga)
"   xmap ga <Plug>(EasyAlign)
" Start interactive EasyAlign for a motion/text object (e.g. gaip)
"   nmap ga <Plug>(EasyAlign)
" Plug 'scrooloose/nerdcommenter'
" Plug 'Yggdroot/indentLine'
" Plug 'chrisbra/Colorizer'
" Plug 'KabbAmine/vCoolor.vim'
" Plug 'dkarter/bullets.vim'
" Plug 'wellle/context.vim'
" Plug 'antoinemadec/FixCursorHold.nvim'
" Plug 'tpope/vim-git'            " Syntax, indent, and filetype for Git
" Git integration - :Git (or just :G) calls any arbitrary Git command
" Plug 'junegunn/gv.vim'      " A git commit browser (requires vim-fugitive)
" :GV to open commit browser
"     You can pass git log options to the command, e.g. :GV -S foobar -- plugins
" :GV! will only list commits that affected the current file
" :GV? fills the location list with the revisions of the current file
" Plug 'vim-airline/vim-airline' " Nifty status of your current file
" let g:airline#extensions#tabline#enabled = 1
" let g:bufferline_echo = 0
" let g:airline_powerline_fonts = 1
" Remove the error and warning sections from Airline layout
" let g:airline#extensions#default#layout = [
"     \ [ 'a', 'b', 'c' ],
"     \ [ 'x', 'y', 'z', 'error', 'warning' ]
"     \ ]
" let g:airline#extensions#default#layout = [
"     \ [ 'a', 'b', 'c' ],
"     \ [ 'x', 'y', 'z']
"     \ ]
" Plug 'vim-airline/vim-airline-themes' " Airline status themes
" let g:airline_theme='simple'
" let g:airline_theme='dark-powerline'
" let g:airline_theme='google_dark'
" Plug 'tpope/vim-sleuth'        " Automatically adjust indentation
" Make your Vim/Neovim as smart as VSCode
" Plug 'neoclide/coc.nvim', {'branch': 'release'}
" let g:coc_disable_startup_warning = 1
" Plug 'fladson/vim-kitty' " Kitty config syntax highlighting for vim
" Language support
" Plug 'fatih/vim-go'            " Go language support for Vim
" Plug 'yuezk/vim-js'            " Syntax highlighting for JavaScript and Flow.js
" Plug 'leafgarland/typescript-vim' " Typescript syntax
" To disable built-in Typescript indentation:
" let g:typescript_indent_disable = 1
" Plug 'maxmellon/vim-jsx-pretty' " The React syntax highlighting and indenting
" Plug 'gmarik/snipmate.vim'      " TextMate's snippets features in Vim
" Plug 'gmarik/snipmate.snippets' " gmarik's custom snippet collection
" Plug 'gmarik/vim-markdown'      " Markdown syntax support for Vim
" Plug 'tpope/vim-repeat'     " Remaps '.' to repeat the last plugin map as a whole
" Plug 'tpope/vim-surround'   " Delete/change/add parentheses/quotes/XML-tags/more
" Plug 'tpope/vim-unimpaired' " Pairs of handy bracket mappings
" Plug 'AndrewRadev/splitjoin.vim' " Switch between single-line and multiline
" Plug 'gmarik/github-search.vim'  " Search Github and clone repos with Vim
" Plug 'gmarik/ide-popup.vim' " Make Vim completion popup menu work like in an IDE
" Plug 'lambdalisue/suda.vim' " Alternative sudo for vim
" Re-open a current file with sudo
" :SudaRead
" Open /etc/sudoers with sudo
" :SudaRead /etc/sudoers
" Forcedly save a current file with sudo
" :SudaWrite
" Write contents to /etc/profile
" :SudaWrite /etc/profile
" Plug 'tomtom/tlib_vim'     " Some utility functions
" Plug 'tomtom/tcomment_vim' " Easy to use, file-type sensible comments for Vim
" Plug 'ctrlpvim/ctrlp.vim'  " Fuzzy file, buffer, mru, tag finder for Vim
" Plug 'junegunn/fzf', { 'do': { -> fzf#install() } }
" Plug 'junegunn/fzf.vim'    " Things you can do with fzf and Vim

" Functionalities - Python
" Plug 'psf/black', { 'branch': 'stable' }
" Plug 'heavenshell/vim-pydocstring'
" Plug 'davidhalter/jedi-vim'    " Python autocompletion
" Plug 'klen/python-mode'        " Python IDE
" let g:pymode = 1
" let g:pymode_warnings = 1

" Aesthetics - Colorschemes
" Plug 'zaki/zazen'
" Plug 'yuttie/hydrangea-vim'
" Plug 'flazz/vim-colorschemes'  " One stop shop for vim colorschemes
" Plug 'gmarik/ingretu'
" Uncomment to play with colorschemes
" Plug 'felixhummel/setcolors.vim' " Easily switch colorschemes

" Aesthetics - Others
" Plug 'junegunn/rainbow_parentheses.vim'
" Plug 'junegunn/limelight.vim'
" Plug 'junegunn/vim-journal'

" Cheat sheets
" Plug 'sudormrfbin/cheatsheet.nvim'

call plug#end()

" General "{{{
set backspace=indent
set backspace+=eol
set backspace+=start
set history=256 " Number of things to remember in history
set ruler       " Show the cursor position all the time
set showcmd     " Display an incomplete command in statusline
set incsearch   " Show matches while typing
if has('mouse')
  set mouse=a   " Enable mouse in GUI mode
  set mousehide " Hide mouse after chars typed
endif

if &t_Co > 2 || has("gui_running")
  syntax on     " Enable syntax
  set hlsearch  " Highlight search
endif

scriptencoding utf-8           " UTF-8 all the way
set encoding=utf-8

set timeoutlen=250             " Time to wait after ESC (default causes an annoying delay)
set clipboard+=unnamed         " Yanks go on clipboard instead.
set pastetoggle=<F10>          " Toggle between paste and normal: for 'safer' pasting from keyboard
set shiftround                 " Round indent to multiple of 'shiftwidth'
set tags=.git/tags;$HOME       " Consider the repo tags first, then
                               " Walk directory tree upto $HOME looking for tags
                               " Note `;` sets the stop folder. :h file-search

set modeline
set modelines=5                " Default numbers of lines to read for modeline instructions

set autowrite                  " Writes on make/shell commands
set autoread

set nobackup
set nowritebackup
set directory=/tmp//           " Prepend(^=) $HOME/.tmp/ to default path; use full path as backup filename(//)
set noswapfile                 "

set hidden                     " The current buffer can be put to the background without writing to disk

set ignorecase                 " Be case insensitive when searching
set smartcase                  " Be case sensitive when input has a capital letter

let g:is_posix = 1             " Vim's default is archaic bourne shell, bring it up to the 90s
let mapleader = ','
let maplocalleader = '	'      " Tab as a local leader
let g:netrw_banner = 0         " Do not show Netrw help banner
" "}}}

" Formatting "{{{
set fo+=o                      " Automatically insert the current comment leader after hitting 'o' or 'O' in Normal mode.
set fo-=r                      " Do not automatically insert a comment leader after an enter
set fo-=t                      " Do no auto-wrap text using textwidth (does not apply to comments)

" set nowrap
" set textwidth=0                " Don't wrap lines by default

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
" set synmaxcol=250              " limit syntax highlighting to 128 columns

set nonumber                  " line numbers Off
set showmatch                 " Show matching brackets.
set matchtime=2               " Bracket blinking.

set wildmode=longest,list     " At command line, complete longest common string, then list alternatives.

set completeopt-=preview      " Disable auto opening preview window

set novisualbell              " No blinking
set noerrorbells              " No noise.
set vb t_vb=                  " Disable any beeps or flashes on error

" set laststatus=2              " Always show status line.
" set shortmess=atI             " Shortens messages

set statusline=%<%f\          " Custom statusline
set stl+=[%{&ff}]             " Show fileformat
set stl+=%y%m%r%=
set stl+=%-14.(%l,%c%V%)\ %P

set foldenable                " Turn on folding
set foldmethod=marker         " Fold on the marker
set foldlevel=100             " Don't autofold anything (but I can still fold manually)

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

" HTML, XML, Jinja
autocmd FileType html setlocal shiftwidth=2 tabstop=2 softtabstop=2
autocmd FileType css setlocal shiftwidth=2 tabstop=2 softtabstop=2
autocmd FileType xml setlocal shiftwidth=2 tabstop=2 softtabstop=2
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

" indentLine
let g:indentLine_char = '▏'
let g:indentLine_defaultGroup = 'NonText'
" Disable indentLine from concealing json and markdown syntax (e.g. ```)
let g:vim_json_syntax_conceal = 0
let g:vim_markdown_conceal = 0
let g:vim_markdown_conceal_code_blocks = 0

" FixCursorHold for better performance
let g:cursorhold_updatetime = 100

" context.vim
let g:context_nvim_no_redraw = 1

" Neovim :Terminal
"
" Exit NeoVim's terminal emulator (:term) by simply pressing escape
tmap <Esc> <C-\><C-n>
tmap <C-w> <Esc><C-w>
"tmap <C-d> <Esc>:q<CR>
autocmd BufWinEnter,WinEnter term://* startinsert
autocmd BufLeave term://* stopinsert

" Python
let g:python3_host_prog = '~/.config/nvim/env/bin/python3'
let g:pydocstring_doq_path = '~/.config/nvim/env/bin/doq'

""" Core plugin configuration (lua)
" Use airline rather than lualine
" require('lualine-config')
" lua << EOF
" servers = {
"     'pyright',
"     --'tsserver', -- uncomment for typescript. See https://github.com/neovim/nvim-lspconfig/blob/master/doc/server_configurations.md for other language servers
" }
" require('treesitter-config')
" require('nvim-cmp-config')
" require('lspconfig-config')
" require('telescope-config')
" require('nvim-tree-config')
" require('diagnostics')
" EOF

" Use the :Cheatsheet command which automatically uses Telescope
" if installed or falls back to showing all the cheatsheet files
" concatenated in a floating window. A default mapping <leader>?
" is provided for :Cheatsheet (not bound if already in use).
" By default the <leader> key is \.
"
" Default cheatsheet configuration:
" lua << EOF
" require("cheatsheet").setup({
"     -- Whether to show bundled cheatsheets
" 
"     -- For generic cheatsheets like default, unicode, nerd-fonts, etc
"     -- bundled_cheatsheets = {
"     --     enabled = {},
"     --     disabled = {},
"     -- },
"     bundled_cheatsheets = true,
" 
"     -- For plugin specific cheatsheets
"     -- bundled_plugin_cheatsheets = {
"     --     enabled = {},
"     --     disabled = {},
"     -- }
"     bundled_plugin_cheatsheets = true,
" 
"     -- For bundled plugin cheatsheets, do not show a sheet if you
"     -- don't have the plugin installed (searches runtimepath for
"     -- same directory name)
"     include_only_installed_plugins = true,
" 
"     -- Key mappings bound inside the telescope window
"     telescope_mappings = {
"         ['<CR>'] = require('cheatsheet.telescope.actions').select_or_fill_commandline,
"         ['<A-CR>'] = require('cheatsheet.telescope.actions').select_or_execute,
"         ['<C-Y>'] = require('cheatsheet.telescope.actions').copy_cheat_value,
"         ['<C-E>'] = require('cheatsheet.telescope.actions').edit_user_cheatsheet,
"     }
" })
" EOF

""" Custom Functions

" Trim Whitespaces
function! TrimWhitespace()
    let l:save = winsaveview()
    %s/\\\@<!\s\+$//e
    call winrestview(l:save)
endfunction

""" Custom Mappings (vim) (lua custom mappings are within individual lua config files)

" Core
let mapleader=","
nmap <leader>q :NvimTreeFindFileToggle<CR>
nmap \ <leader>q
nmap <leader>r :so ~/.config/nvim/init.vim<CR>
nmap <leader>t :call TrimWhitespace()<CR>
xmap <leader>a gaip*
nmap <leader>a gaip*
nmap <leader>h :RainbowParentheses!!<CR>
nmap <leader>j :set filetype=journal<CR>
nmap <leader>k :ColorToggle<CR>
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

" Solidity (requires: npm install --save-dev prettier prettier-plugin-solidity)
autocmd Filetype solidity nmap <leader>p :0,$!npx prettier %<CR>

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
if has("gui_running")
  colorscheme ingretu
else
" colorscheme darkspectrum
  colorscheme darktango
endif
set guifont=Inconsolata:h18
let g:syntastic_html_checkers = []
