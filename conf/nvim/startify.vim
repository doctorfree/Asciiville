"""
" returns all modified files of the current git repo
" `2>/dev/null` makes the command fail quietly, so that when we are not
" in a git repo, the list will be empty
function! s:gitModified()
    let files = systemlist('git ls-files -m 2>/dev/null')
    return map(files, "{'line': v:val, 'path': v:val}")
endfunction

" same as above, but show untracked files, honouring .gitignore
function! s:gitUntracked()
    let files = systemlist('git ls-files -o --exclude-standard 2>/dev/null')
    return map(files, "{'line': v:val, 'path': v:val}")
endfunction
" let g:startify_lists = [
"     \ { 'header': ['    MRU ' . getcwd()], 'type': 'dir' },
"     \ { 'header': ['    Global MRU'],      'type': 'files' },
"     \ { 'header': ['    Sessions'],        'type': 'sessions' },
"     \ { 'header': ['    Bookmarks'],       'type': 'bookmarks' },
"     \ { 'header': ['    Commands'],        'type': 'commands' },
"     \ ]
"
let g:startify_lists = [
    \ { 'type': 'files',     'header': ['   MRU']            },
    \ { 'type': 'dir',       'header': ['   MRU '. getcwd()] },
    \ { 'type': 'sessions',  'header': ['   Sessions']       },
    \ { 'type': 'bookmarks', 'header': ['   Bookmarks']      },
    \ { 'type': function('s:gitModified'),  'header': ['   git modified']},
    \ { 'type': function('s:gitUntracked'), 'header': ['   git untracked']},
    \ { 'type': 'commands',  'header': ['   Commands']       },
    \ ]
let g:startify_bookmarks = [
    \ { 'C': '~/.config/nvim/init.vim' },
    \ { 'Z': '~/.zshrc' },
    \]
let g:startify_commands = [
    \ {'M': ['Messages', 'messages']},
    \]
if exists('g:plugs["vim-startify"]')
  if !empty(glob(g:plugs['vim-startify'].dir.'/plugin/startify.vim'))
    lua require('startify-config')
  endif
endif
function! StartifyEntryFormat() abort
  return 'v:lua.webDevIcons(absolute_path) . " " . entry_path'
endfunction
