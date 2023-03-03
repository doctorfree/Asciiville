local status_ok, auto_session = pcall(require, "auto-session")
if not status_ok then return end

auto_session.setup({
  log_level = 'error',
  auto_session_enable_last_session = true,
  auto_session_enabled = true,
  auto_save_enabled = true,
  auto_restore_enabled = false,
  auto_session_root_dir = vim.fn.stdpath("config") .. "/sessions/",
  auto_session_create_enabled = true,
  auto_session_suppress_dirs = nil,
  auto_session_use_git_branch = true,
  -- the configs below are lua only
  bypass_session_save_file_types = nil,
  cwd_change_handling = {
    restore_upcoming_session = true, -- already the default, no need to specify like this, only here as an example
    pre_cwd_changed_hook = nil, -- already the default, no need to specify like this, only here as an example
    post_cwd_changed_hook = function() -- example refreshing the lualine status line _after_ the cwd changes
      require("lualine").refresh() -- refresh lualine so the new session name is displayed in the status bar
    end,
  },
})
