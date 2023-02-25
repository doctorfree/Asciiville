local M = {}

function M.line()
    local os = vim.api.nvim_eval("&fileformat")
    if os == "unix" then
        return "LF"
    elseif os == "mac" then
        return "CR"
    else
        return "CRLF"
    end
end

return M
