local lsp = require("languages.lsp")

return {
    efm = {
        {
            formatCommand = "prettier --tab-width=2 --use-tabs=false --stdin-filepath ${INPUT}",
            formatStdin = true,
        },
    },

    all_format = { efm = "Prettier" },
    default_format = "efm",
    lsp_server = "yamlls",

    lsp = {
        capabilities = lsp.capabilities,
        on_attach = lsp.on_attach,
    },
}
