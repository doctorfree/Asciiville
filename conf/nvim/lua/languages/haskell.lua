local lsp = require("languages.lsp")

return {
    efm = {
        {
            formatCommand = "ormolu",
            formatStdin = true,
        },
    },

    all_format = { efm = "Ormolu" },
    default_format = "efm",
    lsp_server = "hls",

    lsp = {
        capabilities = lsp.capabilities,
        on_attach = function(client, bufnr)
            lsp.on_attach(client, bufnr)
            require("nvim-navic").attach(client, bufnr)
        end,
    },
}
