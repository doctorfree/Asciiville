local lsp = require("languages.lsp")

return {
    efm = {
        {
            formatCommand = "rustfmt --edition 2018",
            formatStdin = true,
        },
    },

    all_format = {
        efm = "Rustfmt",
        rust_analyzer = "Rust-Analyzer",
    },

    default_format = "efm",
    lsp_server = "rust_analyzer",

    lsp = {
        capabilities = lsp.capabilities,
        on_attach = function(client, bufnr)
            lsp.on_attach(client, bufnr)
            require("nvim-navic").attach(client, bufnr)
        end,

        settings = {
            ["rust_analyzer"] = {
                cargo = {
                    loadOutDirsFromCheck = true,
                },
            },
        },
    },
}
