local lsp = require("languages.lsp")

-- Fix difference offset encoding in clangd and efm
local capabilities = lsp.capabilities
capabilities.offsetEncoding = { "utf-16" }

return {
    efm = {
        {
            formatCommand = [[clang-format -style='{BasedOnStyle: LLVM, IndentWidth: 4}']],
            formatStdin = true,
        },
    },

    all_format = { efm = "Clang" },
    default_format = "efm",
    lsp_server = "clangd",

    lsp = {
        capabilities = capabilities,
        cmd = { "clangd-12" },
        on_attach = function(client, bufnr)
            lsp.on_attach(client, bufnr)
            require("nvim-navic").attach(client, bufnr)
        end,
    },
}
