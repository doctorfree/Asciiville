local M = {}

local function get_nvim_lsp_diagnostic(severity)
    if next(vim.lsp.buf_get_clients(0)) == nil then
        return "0 "
    end

    return #vim.diagnostic.get(vim.api.nvim_get_current_buf(), { severity = severity }) .. " "
end

function M.error()
    return get_nvim_lsp_diagnostic(vim.diagnostic.severity.ERROR)
end

function M.warning()
    return get_nvim_lsp_diagnostic(vim.diagnostic.severity.WARN)
end

function M.line()
    return "%#StatuslineDiagnosticsError#"
        .. [[ %{luaeval('require("statusline.diagnostics").error()')}]]
        .. "%#StatuslineDiagnosticsWarning#"
        .. [[ %{luaeval('require("statusline.diagnostics").warning()')}]]
        .. "  "
end

return M
