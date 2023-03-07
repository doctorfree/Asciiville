local status_ok, auto_session = pcall(require, "auto-session")
if not status_ok then return end

function _G.close_all_floating_wins()
  for _, win in ipairs(vim.api.nvim_list_wins()) do
    local config = vim.api.nvim_win_get_config(win)
    if config.relative ~= '' then
      vim.api.nvim_win_close(win, false)
    end
  end
end

require('auto-session').setup {
}

auto_session.setup({
  log_level = 'error',
  auto_session_enable_last_session = true,
  auto_session_enabled = true,
  auto_save_enabled = true,
  auto_restore_enabled = false,
  auto_session_root_dir = vim.fn.stdpath("config") .. "/sessions/",
  auto_session_create_enabled = true,
  auto_session_suppress_dirs = nil,
  auto_session_use_git_branch = false,
  -- the configs below are lua only
  bypass_session_save_file_types = nil,
  pre_save_cmds = { _G.close_all_floating_wins },
  cwd_change_handling = {
    restore_upcoming_session = true, -- already the default, no need to specify like this, only here as an example
    pre_cwd_changed_hook = nil, -- already the default, no need to specify like this, only here as an example
    post_cwd_changed_hook = function() -- example refreshing the lualine status line _after_ the cwd changes
      require("lualine").refresh() -- refresh lualine so the new session name is displayed in the status bar
    end,
  },
})
