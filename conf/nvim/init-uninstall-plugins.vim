""" Use this Neovim configuration to uninstall all plugins
"
"   Open Neovim with 'nvim -u ~/.config/nvim/init-uninstall-plugins.vim'
"   Run ':PlugClean'
"
"   After plugins have been removed, reinstall the Neovim plugins with:
"     nvim -c 'PlugInstall' -c 'qa'
"
"
call plug#begin()
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

set timeoutlen=250             " Time to wait after ESC (default causes delay)
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
let mapleader = ','
let maplocalleader = '	'      " Tab as a local leader
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
" The keybinds can be changed:
"
" Default keys
" call wilder#setup({
"       \ 'modes': [':', '/', '?'],
"       \ 'next_key': '<Tab>',
"       \ 'previous_key': '<S-Tab>',
"       \ 'accept_key': '<Down>',
"       \ 'reject_key': '<Up>',
"       \ })

" Airline and Lightline users:
" wilder#wildmenu_airline_theme() and wilder#wildmenu_lightline_theme() can be used.
"
if exists('g:plugs["wilder.nvim"]')
  if !empty(glob(g:plugs['wilder.nvim'].dir.'/autoload/wilder.vim'))
    call wilder#setup({'modes': [':', '/', '?']})
    call wilder#set_option('renderer', wilder#wildmenu_renderer(
        \ wilder#wildmenu_airline_theme({
        \   'highlighter': wilder#lua_fzy_highlighter(),
        \   'separator': ' · ',
        \ })))
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
let g:updatetime = 300

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

if !empty(glob('/home/linuxbrew/.linuxbrew/bin/python3'))
  let g:python3_host_prog = '/home/linuxbrew/.linuxbrew/bin/python3'
endif
if !empty(glob('/home/linuxbrew/.linuxbrew/bin/doq'))
  let g:pydocstring_doq_path = '/home/linuxbrew/.linuxbrew/bin/doq'
endif

""" Core plugin configuration (lua)
" Use airline rather than lualine
" require('config.lualine')
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
    "lua-language-server",
    "vimls",
    "yaml-language-server",
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
" By default the <leader> key is \.
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

if exists('g:plugs["nvim-dap"]')
  if !empty(glob(g:plugs['nvim-dap'].dir.'/lua/nvim-dap/plugin/dap.lua'))
    lua require('config.dap')
  endif
endif
if exists('g:plugs["fidget.nvim"]')
  if !empty(glob(g:plugs['fidget.nvim'].dir.'/lua/fidget.lua'))
    lua require"fidget".setup{}
  endif
endif
if exists('g:plugs["inlay-hints.nvim"]')
  if !empty(glob(g:plugs['inlay-hints.nvim'].dir.'/lua/inlay-hints/init.lua'))
    lua require("inlay-hints").setup()
  endif
endif
if exists('g:plugs["rust-tools.nvim"]')
  if !empty(glob(g:plugs['rust-tools.nvim'].dir.'/lua/rust-tools/init.lua'))
    lua require('config.rust-tools')
  endif
endif
if exists('g:plugs["toggleterm.nvim"]')
  if !empty(glob(g:plugs['toggleterm.nvim'].dir.'/lua/toggleterm.lua'))
    lua require('toggleterm').setup()
  endif
endif
if exists('g:plugs["ChatGPT.nvim"]')
  if !empty(glob(g:plugs['ChatGPT.nvim'].dir.'/plugin/chatgpt.lua'))
    lua require('chatgpt').setup()
  endif
endif

""" Custom Mappings (lua custom mappings are within individual lua config files)
"
" Core
let mapleader=","
nmap <leader>q :NvimTreeFindFileToggle<CR>
nmap \ <leader>q
nmap <leader>r :so ~/.config/nvim/init.vim<CR>
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

if has("termguicolors")
  let &t_8f = "\<Esc>[38;2;%lu;%lu;%lum"
  let &t_8b = "\<Esc>[48;2;%lu;%lu;%lum"
  set termguicolors
endif

" If your terminal doesn't handle italics, bold, underline, or undercurl
" then they can be disabled in the asciiville colorscheme with:
" let g:asciiville_italic = 0
" let g:asciiville_bold = 0
" let g:asciiville_underline = 0
" let g:asciiville_undercurl = 0
" Comment out to use everforest below
colorscheme asciiville
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
let g:syntastic_html_checkers = []

if exists(':GuiFont')
  GuiFont! JetBrains Mono:h22
endif
