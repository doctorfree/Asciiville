local lsp = require("languages.lsp")

return {
    efm = {
        {
            formatCommand = "black --line-length 79 --quiet -",
            formatStdin = true,
        },
        {
            lintCommand = "flake8 --stdin-display-name ${INPUT} -",
            lintStdin = true,
            lintFormats = {
                "%f:%l:%c: %m",
            },
        },
    },

    all_format = { efm = "Black" },
    default_format = "efm",
    lsp_server = "pyright",

    lsp = {
        capabilities = lsp.capabilities,
        on_attach = function(client, bufnr)
            lsp.on_attach(client, bufnr)
            require("nvim-navic").attach(client, bufnr)
        end,
    },
}
