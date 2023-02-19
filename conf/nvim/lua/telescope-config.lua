require('telescope').setup {
  defaults = {
    file_ignore_patterns = {
      '.git/', 'node_modules/', '.npm/', '*[Cc]ache/', '*-cache*',
      '.dropbox/', '.dropbox_trashed/', '*.py[co]', '*.sw?', '*~',
      '*.sql', '*.tags*', '*.gemtags*', '*.csv', '*.tsv', '*.tmp*',
      '*.old', '*.plist', '*.pdf', '*.log', '*.jpg', '*.jpeg', '*.png',
      '*.tar.gz', '*.tar', '*.zip', '*.class', '*.pdb', '*.dll',
      '*.dat', '*.mca', '__pycache__', '.mozilla/', '.electron/',
      '.vpython-root/', '.gradle/', '.nuget/', '.cargo/', '.evernote/',
      '.azure-functions-core-tools/', 'yay/', '.local/share/Trash/',
      '.local/share/nvim/swap/', 'code%-other/'
    }
  }
}

require('telescope').load_extension('fzf')

vim.api.nvim_set_keymap('n', '<C-p>', '<cmd>lua require("telescope.builtin").find_files({hidden = true})<CR>', {})
