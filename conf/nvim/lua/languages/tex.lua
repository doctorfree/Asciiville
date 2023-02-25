local lsp = require("languages.lsp")

return {
    efm = {
        {
            formatCommand = "latexindent.pl -y=\"defaultIndent:'  '\"",
            formatStdin = true,
        },
    },

    all_format = { efm = "LatexIndent", texlab = "TexLab" },
    default_format = "efm",
    lsp_server = "texlab",

    lsp = {
        capabilities = lsp.capabilities,
        on_attach = function(client, bufnr)
            lsp.on_attach(client, bufnr)
            require("nvim-navic").attach(client, bufnr)
        end,
    },
}
