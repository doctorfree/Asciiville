local lsp = require("languages.lsp")

return {
    efm = {
        {
            formatCommand = "prettier --tab-width=4 --use-tabs=false --stdin-filepath ${INPUT}",
            formatStdin = true,
        },
        {
            lintCommand = "eslint_d -f unix --stdin --stdin-filename ${INPUT}",
            lintIgnoreExitCode = true,
            lintStdin = true,
            lintFormats = { "%f:%l:%c: %m" },
        },
    },

    all_format = {
        efm = "Prettier",
        tsserver = "Tssever",
    },

    default_format = "efm",
    lsp = {
        capabilities = lsp.capabilities,
        on_attach = function(client, bufnr)
            lsp.on_attach(client, bufnr)
            require("nvim-navic").attach(client, bufnr)
        end,
    },
}
