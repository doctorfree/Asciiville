local lsp = require("languages.lsp")

return {
    efm = {
        {
            formatCommand = "prettier --tab-width=4 --use-tabs=false --stdin-filepath ${INPUT}",
            formatStdin = true,
        },
    },

    all_format = { efm = "Prettier" },
    default_format = "efm",
    lsp_server = "jsonls",

    lsp = {
        capabilities = lsp.capabilities,
        on_attach = function(client, bufnr)
            lsp.on_attach(client, bufnr)
            require("nvim-navic").attach(client, bufnr)
        end,
    },
}
