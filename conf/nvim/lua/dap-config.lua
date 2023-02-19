local dap = require('dap')
local home = os.getenv('HOME')

-- Virtual text
vim.g.dap_virtual_text = true
-- request variable values for all frames (experimental)
vim.g.dap_virtual_text = 'all frames'

-- dap-ui
-- require("dapui").setup()

-- dap

dap.toggle_conditional_breakpoint = function()
  dap.toggle_breakpoint(vim.fn.input { prompt = 'Breakpoint condition: ' }, nil, nil, true)
end
local widgets = require('dap.ui.widgets')
dap.sidebar = widgets.sidebar(widgets.scopes)

vim.fn.sign_define('DapBreakpoint', { text = '', texthl = '', linehl = '', numhl = '' })

vim.cmd([[
  command! -nargs=0 DapContinue :lua require('dap').continue()
]])
vim.cmd([[
  command! -nargs=0 DapBreakpoints :lua require('dap').list_breakpoints()
]])
vim.cmd([[
  command! -nargs=0 DapSidebar :lua require('dap-setup').sidebar.toggle()
]])

dap.defaults.fallback.external_terminal = {
  command = '/usr/bin/alacritty',
  args = { '-e' },
}

-- Haskell
dap.adapters.haskell = {
  type = 'executable',
  command = 'haskell-debug-adapter',
  args = { '--hackage-version=0.0.33.0' },
}
dap.configurations.haskell = {
  {
    type = 'ghc',
    request = 'launch',
    name = 'haskell-debug-adapter',
    internalConsoleOptions = 'openOnSessionStart',
    workspace = '${workspaceRoot}',
    startup = '${file}',
    startupFunc = '',
    startupArgs = '',
    stopOnEntry = true,
    mainArgs = '',
    logFile = vim.fn.stdpath('data') .. '/haskell-dap.log',
    logLevel = 'WARNING',
    ghciEnv = vim.empty_dict(),
    ghciPrompt = 'λ: ',
    -- Adjust the prompt to the prompt you see when you invoke the stack ghci command below
    ghciInitialPrompt = 'Prelude> ',
    -- ghciCmd= 'stack ghci --test --no-load --no-build --main-is TARGET --ghci-options -fprint-evld-with-show',
    ghciCmd = 'cabal new-repl TARGET',
    forceInspect = false,
  },
}

-- Lua
dap.configurations.lua = {
  {
    type = 'nlua',
    request = 'attach',
    name = 'Attach to running Neovim instance',
    port = 44444,
  },
}
dap.adapters.nlua = function(callback, config)
  local port = config.port
  local options = {
    args = {
      '-e',
      vim.v.progpath,
      '-c',
      string.format('lua require("osv").launch({port = %d})', port),
    },
    cwd = vim.fn.getcwd(),
    detached = true,
  }
  local handle
  local pid_or_err
  handle, pid_or_err = vim.loop.spawn('alacritty', options, function(code)
    handle:close()
    if code ~= 0 then
      print('nvim exited', code)
    end
  end)
  assert(handle, 'Could not run alacritty:' .. pid_or_err)

  -- doing a `client = new_tcp(); client:connect()` within vim.wait doesn't work
  -- because an extra client connecting confuses `osv`, so sleep a bit instead
  -- to wait until server is started
  vim.cmd.sleep()
  callback { type = 'server', host = '127.0.0.1', port = port }
end

-- Python (TODO)
-- require('dap-python').setup('~/.virtualenvs/tools/bin/python')

-- Go
dap.adapters.go = {
  type = 'executable',
  command = 'node',
  args = { home .. '/dev/golang/vscode-go/dist/debugAdapter.js' },
  enrich_config = function(conf, on_config)
    if not conf.dlvToolPath then
      conf.dlvToolPath = '/usr/bin/dlv'
    end
    on_config(conf)
  end,
}
dap.configurations.go = {
  {
    type = 'go',
    name = 'Debug',
    request = 'launch',
    showLog = true,
    program = '${file}',
  },
}

-- C++ / Rust
dap.adapters.cpp = {
  type = 'executable',
  attach = {
    pidProperty = 'pid',
    pidSelect = 'ask',
  },
  command = 'lldb-vscode',
  env = {
    LLDB_LAUNCH_FLAG_LAUNCH_IN_TTY = 'YES',
  },
  name = 'lldb',
}

local last_gdb_config

dap.start_c_debugger = function(args, mi_mode, mi_debugger_path)
  if args and #args > 0 then
    last_gdb_config = {
      type = 'cpp',
      name = args[1],
      request = 'launch',
      program = table.remove(args, 1),
      args = args,
      cwd = vim.fn.getcwd(),
      env = { 'VAR1=value1', 'VAR2=value' }, -- environment variables are set via `ENV_VAR_NAME=value` pairs
      externalConsole = true,
      MIMode = mi_mode or 'gdb',
      MIDebuggerPath = mi_debugger_path,
    }
  end

  if not last_gdb_config then
    print('No binary to debug set! Use ":DebugC <binary> <args>" or ":DebugRust <binary> <args>"')
    return
  end

  dap.run(last_gdb_config)
  dap.repl.open()
end

vim.cmd([[
    command! -complete=file -nargs=* DebugC lua require "my_debug".start_c_debugger({<f-args>}, "gdb")
]])
vim.cmd([[
    command! -complete=file -nargs=* DebugRust lua require "my_debug".start_c_debugger({<f-args>}, "gdb", "rust-gdb")
]])
