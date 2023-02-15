# Asciiville Neovim text editor

[Neovim](https://neovim.io) is a fork of the [Vim](vim.md) text editor that strives to improve the extensibility and maintainability of Vim. Some features of the fork include built-in Language Server Protocol support, support for asynchronous I/O, and support for scripting using Lua.

![](neovim.png)

## Table of Contents

1. [Introduction](#introduction)
1. [Configuration](#configuration)
1. [Plugins](#plugins)
1. [Usage](#usage)

## Introduction

Neovim has the same configuration syntax as Vim (unless vim9script is used); thus the same configuration file can be used with both editors, although there are minor differences in details of options. If the added features of Neovim are not used, Neovim is compatible with almost all of Vim's features.

The Neovim project was started in 2014, with some Vim community members offering early support of the high-level refactoring effort to provide better scripting, plugins, and integration with modern GUIs. The project is free software and its source code is available on [GitHub](https://github.com/neovim/neovim).

The Neovim editor is available in native packaging format on almost all Linux distributions as well as packaging for Mac and Windows, making it possible to install it on almost every operating system. However, some distributions provide an older version of Neovim and the Asciiville configuration of Neovim takes advantage of many of the recently implemented features. In order to provide a current version of Neovim on all supported Asciiville platforms, the Asciiville version of Neovim is installed with [Homebrew](https://brew.sh). In addition to installing the latest version of Neovim, the Asciiville initialization process installs a number of Neovim plugins and supporting utilities. This includes extensive support for a wide variety of programming languages, both syntax highlighting and syntax checking as well as autocompletion. The Asciiville configuration of Neovim includes Asciiville themes and color schemes for both Neovim itself and the [Airline status plugin](https://github.com/vim-airline/vim-airline).

Repository: https://github.com/neovim/neovim

Website: https://neovim.io/

## Configuration]

The primary Neovim configuration and startup file is `$HOME/.config/nvim/init.vim`.

Some common startup options set in this configuration file include:

```vim
set nocompatible            " disable compatibility to old-time vi
set showmatch               " show matching brackets.
set ignorecase              " case insensitive matching
set mouse=v                 " middle-click paste with mouse
set hlsearch                " highlight search results
set autoindent              " indent a new line the same amount as the line just typed
set number                  " add line numbers
set wildmode=longest,list   " get bash-like tab completions
set cc=88                   " set colour columns for good coding style
filetype plugin indent on   " allows auto-indenting depending on file type
set tabstop=2               " number of columns occupied by a tab character
set expandtab               " convert tabs to white space
set shiftwidth=2            " width for autoindents
set softtabstop=2           " see multiple spaces as tabstops so <BS> does the right thing```

## Plugins

See a list of
[Neovim plugins installed, configured, and enabled in Asciiville](Plugins-urls.md).

## Usage

Extensive documentation for Neovim is available at https://neovim.io/doc/user/

Learning Vi/Vim/Neovim is a lifetime task. It's a complex and expanding Universe.
But getting started isn't too difficult and learning something new everyday is fun.
Don't get discouraged. The reward will be worth the effort.

Most, if not all, of the information in the
[Vim cheat sheet](https://github.com/doctorfree/cheat-sheets-plus/blob/main/text/vim.md)
applies equally to `nvim`.

Asciiville installs and enables the
[Neovim cheatsheets plugin](https://github.com/sudormrfbin/cheatsheet.nvim)
that provides searchable cheatsheets from within Neovim itself.

Neovim and Vim provide several modes for different kinds of text manipulation.

Pressing 'i' in normal mode enters insert mode.
'<Esc>' goes back to normal mode, which doesn't allow regular text insertion.

### Open a file
```shell
nvim path/to/file
```

### Enter text editing mode (insert mode):
```shell
<Esc>i
```

### Copy ("yank") or cut ("delete") the current line (paste it with `P`):
```shell
<Esc>yy|dd
```

### Enter normal mode and undo the last operation:
```shell
<Esc>u
```

### Search for a pattern in the file (press `n`/`N` to go to next/previous match):
```shell
<Esc>/search_pattern<Enter>
```

### Perform a regular expression substitution in the whole file:
```shell
<Esc>:%s/regular_expression/replacement/g<Enter>
```

### Enter normal mode and save (write) the file, and quit:
```shell
<Esc>:wq<Enter>
```

### Quit without saving:
```shell
<Esc>:q!<Enter>
```

## See also

- [Vim](vim.md)
