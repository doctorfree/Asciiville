local lsp = require("languages.lsp")

return {
    efm = {
        {
            formatCommand = "goimports",
            formatStdin = true,
        },
        {
            formatCommand = "gofumpt",
            formatStdin = true,
        },
        {
            lintCommand = "golangci-lint run",
            lintIgnoreExitCode = true,
            lintStdin = true,
            lintFormats = {
                "%f:%l:%c: %m",
            },
        },
    },

    all_format = { efm = "Goimports   Gofumpt   Golangci" },
    default_format = "efm",
    lsp_server = "gopls",

    lsp = {
        capabilities = lsp.capabilities,
        settings = {
            gopls = {
                analyses = {
                    unusedparams = true,
                },
            },
        },
        on_attach = function(client, bufnr)
            lsp.on_attach(client, bufnr)
            require("nvim-navic").attach(client, bufnr)
        end,
    },
}
