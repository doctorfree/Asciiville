local cmd = vim.cmd
local api = vim.api

local M = {
    is_rest = false,
    is_sql = false,
}

-- Count number of properties in table
-- Because of lua only count consecutive properties
-- @param T Table
function M.tablelength(T)
    local count = 0

    for _ in pairs(T) do
        count = count + 1
    end

    return count
end

-- Credit https://stackoverflow.com/questions/48402876/getting-current-file-name-in-lua
function M.get_file_name(path)
    return path:match("^.+/(.+)$")
end

function M.file_extension(filename)
    local t = {}
    for str in string.gmatch(filename, "([^%.]+)") do
        table.insert(t, str)
    end

    if #t == 1 or t[1] == "" then
        return ""
    end

    return t[#t]
end

function M.rest()
    if M.is_rest then
        vim.cmd("tabclose")

        M.is_rest = false
    else
        vim.cmd("tabedit new")
        vim.bo.bufhidden = "wipe"

        local valid_win = M.win_rest and api.nvim_win_is_valid(M.win_rest)
        local window = valid_win and M.win_rest or api.nvim_get_current_win()

        local valid_buf = M.buf_rest and api.nvim_buf_is_valid(M.buf_rest)
        local buf = valid_buf and M.buf_rest or api.nvim_create_buf(false, false)

        api.nvim_buf_set_name(buf, "test.http")

        api.nvim_buf_set_option(buf, "buftype", "nofile")
        api.nvim_buf_set_option(buf, "bufhidden", "delete")
        api.nvim_buf_set_option(buf, "filetype", "http")

        api.nvim_buf_set_keymap(
            buf,
            "n",
            "<Leader>r",
            [[<Cmd>lua require("rest-nvim").run()<CR>]],
            { noremap = true, silent = true }
        )

        api.nvim_set_current_buf(buf)
        api.nvim_win_set_buf(window, buf)

        M.is_rest = true
        M.win_rest = window
        M.buf_rest = buf
    end
end

function M.sql()
    if M.is_sql then
        vim.cmd("tabclose!")

        M.is_sql = false
    else
        vim.cmd("tabedit new")
        vim.bo.bufhidden = "wipe"

        local valid_win = M.win_sql and api.nvim_win_is_valid(M.win_sql)
        local window = valid_win and M.win_sql or api.nvim_get_current_win()

        local valid_buf = M.buf_sql and api.nvim_buf_is_valid(M.buf_sql)
        local buf = valid_buf and M.buf_sql or api.nvim_create_buf(false, false)

        api.nvim_buf_set_name(buf, "[no name].sql")

        api.nvim_buf_set_option(buf, "buftype", "nowrite")
        api.nvim_buf_set_option(buf, "bufhidden", "delete")
        api.nvim_buf_set_option(buf, "filetype", "sql")

        api.nvim_buf_set_keymap(buf, "n", "<Leader>r", [[<Cmd>SqlsExecuteQuery<CR>]], { noremap = true, silent = true })
        api.nvim_buf_set_keymap(
            buf,
            "n",
            "<Leader>p",
            [[<Cmd>SqlsSwitchConnection<CR>]],
            { noremap = true, silent = true }
        )
        api.nvim_buf_set_keymap(
            buf,
            "n",
            "<Leader>o",
            [[<Cmd>SqlsSwitchDatabase<CR>]],
            { noremap = true, silent = true }
        )

        api.nvim_set_current_buf(buf)
        api.nvim_win_set_buf(window, buf)

        M.is_sql = true
        M.win_sql = window
        M.buf_sql = buf
    end
end

M.map_filetype_filename = {
    sh = "*.sh,*.zsh",
    cmake = "CMakeLists.txt",
    css = "*.css",
    cpp = "*.cpp,*.hpp",
    dart = "*.dart",
    dockerfile = "Dockerfile",
    elixir = "*.ex,*.exs",
    go = "*.go",
    haskell = "*.hs",
    html = "*.html",
    java = "*.java",
    javascript = "*.js",
    javascriptreact = "*.jsx",
    json = "*.json",
    lua = "*.lua",
    markdown = "*.md",
    php = "*.php",
    python = "*.py",
    rust = "*.rs",
    solidity = "*.sol",
    svelte = "*.svelte",
    tex = "*.tex",
    typescript = "*.ts",
    typescriptreact = "*.tsx",
    xml = "*.xml",
    yaml = "*.yaml,*.yml",
}

function M.autocommand_by_filetypes(setting_filetypes, events, command)
    local autocmd = "autocmd " .. events .. " "
    local i = 1
    for _, filetype in pairs(setting_filetypes) do
        if i > 1 then
            autocmd = autocmd .. ","
        end

        autocmd = autocmd .. M.map_filetype_filename[filetype]

        i = i + 1
    end

    autocmd = autocmd .. " " .. command

    cmd(autocmd)
end

local function cmd_option(callback)
    return { noremap = true, silent = true, callback = callback }
end

function M.rename_popup()
    local current_name = api.nvim_eval('expand("<cword>")')

    local width = 50
    local buf = api.nvim_create_buf(false, true)
    local opts = {
        relative = "cursor",
        width = width,
        height = 1,
        col = 0,
        row = 1,
        anchor = "NW",
        border = "single",
        style = "minimal",
    }

    local win_handle = api.nvim_open_win(buf, 1, opts)

    local texts = { current_name }
    api.nvim_buf_set_lines(buf, 0, 1, false, texts)

    api.nvim_buf_set_keymap(buf, "n", "q", ":close<CR>", { silent = true, nowait = true, noremap = true })
    api.nvim_buf_set_keymap(
        buf,
        "i",
        "<CR>",
        "",
        cmd_option(function()
            local name = api.nvim_buf_get_lines(buf, 0, 1, false)[1]
            api.nvim_win_close(win_handle, true)

            if name ~= "" then
                vim.lsp.buf.rename(name)
            end

            local keys = vim.api.nvim_replace_termcodes("<ESC>l", true, false, true)
            api.nvim_feedkeys(keys, "i", true)
        end)
    )

    api.nvim_buf_set_keymap(
        buf,
        "n",
        "<CR>",
        "",
        cmd_option(function()
            local name = api.nvim_buf_get_lines(buf, 0, 1, false)[1]
            api.nvim_win_close(win_handle, true)

            if name ~= "" then
                vim.lsp.buf.rename(name)
            end

            local keys = vim.api.nvim_replace_termcodes("l", true, false, true)
            api.nvim_feedkeys(keys, "i", true)
        end)
    )

    api.nvim_buf_set_option(buf, "buftype", "nofile")
    api.nvim_buf_set_option(buf, "bufhidden", "delete")

    -- Normal to insert
    api.nvim_feedkeys("A", "n", true)
end

-- Port tests to Neotest framework
-- function M.toggle_test()
--     local current_test = api.nvim_eval("ultest#status()")
--     if current_test["tests"] > 0 then
--         if current_test["passed"] > 0 or current_test["failed"] > 0 or current_test["running"] > 0 then
--             vim.api.nvim_command("UltestClear")
--         else
--             vim.api.nvim_command("UltestNearest")
--         end
--     end
-- end

function M.cover_score()
    local score_raw = vim.api.nvim_exec('!go test -cover | sed -n 2p | cut -d " " -f 2 | tr -d "\\n"', true)
    local newline_position = string.find(score_raw, "\n")
    print(string.sub(score_raw, newline_position))
end

function M.project_files()
    local ok = pcall(require("telescope.builtin").git_files, { show_untracked = true })
    if not ok then
        require("telescope.builtin").find_files()
    end
end

return M
