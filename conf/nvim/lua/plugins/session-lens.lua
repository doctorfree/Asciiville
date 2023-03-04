require('session-lens').setup {
  -- path_display={'shorten'},
  prompt_title = 'Neovim Sessions'
}
require('telescope').load_extension('session-lens')
