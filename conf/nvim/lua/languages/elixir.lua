local lsp = require("languages.lsp")

return {
    efm = {
        {
            formatCommand = "mix format -",
            formatStdin = true,
        },
    },

    all_format = { efm = "Mix" },
    default_format = "efm",
    lsp_server = "elixirls",

    lsp = {
        capabilities = lsp.capabilities,
        on_attach = function(client, bufnr)
            lsp.on_attach(client, bufnr)
            require("nvim-navic").attach(client, bufnr)
        end,
        cmd = { "/home/nguyenhoangnam/.local/share/tool/elixir-ls/language_server.sh" },
    },
}
