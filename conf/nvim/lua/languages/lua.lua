local lsp = require("languages.lsp")

function os.capture(cmd, raw)
    local f = assert(io.popen(cmd, "r"))
    local s = assert(f:read("*a"))
    f:close()
    if raw then
        return s
    end
    s = string.gsub(s, "^%s+", "")
    s = string.gsub(s, "%s+$", "")
    s = string.gsub(s, "[\n\r]+", " ")
    return s
end

local sumneko_binary = os.capture("which lua-language-server")

local runtime_path = vim.split(package.path, ";")
table.insert(runtime_path, "lua/?.lua")
table.insert(runtime_path, "lua/?/init.lua")

return {
    efm = {
        {
            formatCommand = "stylua - --config-path ~/.config/stylua/stylua.toml",
            formatStdin = true,
        },
    },

    all_format = { efm = "Stylua" },
    default_format = "efm",
    lsp_server = "sumneko_lua",

    lsp = {
        capabilities = lsp.capabilities,

        on_attach = function(client, bufnr)
            lsp.on_attach(client, bufnr)
            require("nvim-navic").attach(client, bufnr)
        end,

        cmd = { sumneko_binary, "-E" },

        settings = {
            Lua = {
                runtime = {
                    version = "LuaJIT",
                    path = runtime_path,
                },

                diagnostics = {
                    globals = { "vim" },
                },

                workspace = {
                    maxPreload = 2000,
                    preloadFileSize = 150,
                    library = vim.api.nvim_get_runtime_file("", true),
                },

                telemetry = {
                    enable = false,
                },
            },
        },
    },
}
