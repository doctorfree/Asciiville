local lsp = require("languages.lsp")

return {
    efm = {
        {
            formatCommand = "shfmt",
            formatStdin = true,
        },
        {
            lintCommand = "shellcheck -f gcc -x",
            lintSource = "shellcheck",
            lintFormats = {
                "%f:%l:%c: %trror: %m",
                "%f:%l:%c: %tarning: %m",
                "%f:%l:%c: %tote: %m",
            },
        },
    },

    all_format = { efm = "Shfmt   shellcheck" },
    default_format = "efm",
    lsp_server = "bashls",

    lsp = {
        capabilities = lsp.capabilities,
        on_attach = lsp.on_attach,
    },
}
