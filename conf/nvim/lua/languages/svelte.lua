local lsp = require("languages.lsp")

return {
    efm = {
        {
            formatCommand = "prettier --tab-width 4 --svelte-indent-script-and-style false --use-tabs=false --stdin-filepath ${INPUT} --plugin-search-dir .",
            formatStdin = true,
        },
    },

    all_format = { efm = "Pretter" },
    default_format = "efm",
    lsp_server = "svelte",

    lsp = {
        capabilities = lsp.capabilities,
        on_attach = lsp.on_attach,
    },
}
