local lsp = require("languages.lsp")

return {
    efm = {
        {
            formatCommand = [[dart format -o show . | sed '$d']],
            formatStdin = true,
        },
    },

    all_format = { efm = "" },
    default_format = "efm",
    lsp_server = "dartls",

    lsp = {
        capabilities = lsp.capabilities,
        on_attach = function(client, bufnr)
            lsp.on_attach(client, bufnr)
            require("nvim-navic").attach(client, bufnr)
        end,
    },
}
