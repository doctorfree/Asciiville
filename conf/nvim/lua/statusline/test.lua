local api = vim.api
local M = {}

-- Port tests to Neotest framework
-- function M.is_test_file()
--     local current_test = api.nvim_eval("ultest#status()")
--     if current_test["tests"] > 0 then
--         return true
--     else
--         return false
--     end
-- end

-- local function get_test(field)
--     local current_test = api.nvim_eval("ultest#status()")
--     return current_test[field] .. " "
-- end

function M.tests()
    return get_test("tests")
end

function M.failed()
    return get_test("failed")
end

function M.passed()
    return get_test("passed")
end

function M.running()
    return get_test("running")
end

function M.line()
    return "%#StatuslineTest#"
        .. [[ %{luaeval('require("statusline.test").tests()')}]]
        .. "%#StatuslinePassed#"
        .. [[ %{luaeval('require("statusline.test").passed()')}]]
        .. "%#StatuslineFailed#"
        .. [[ %{luaeval('require("statusline.test").failed()')}]]
        .. "%#StatuslineRunning#"
        .. [[ %{luaeval('require("statusline.test").running()')}]]
end

return M
