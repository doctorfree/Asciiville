local lsp = require("languages.lsp")

return {
    efm = {
        {
            formatCommand = "prettier --tab-width=2 --xml-whitespace-sensitivity ignore --stdin-filepath ${INPUT}",
            formatStdin = true,
        },
    },

    all_format = { efm = "Prettier", lemminx = "Lemminx" },
    default_format = "efm",
    lsp_server = "lemminx",

    lsp = {
        capabilities = lsp.capabilities,
        on_attach = function(client, bufnr)
            lsp.on_attach(client, bufnr)
            require("nvim-navic").attach(client, bufnr)
        end,

        cmd = {
            "java",
            "-jar",
            "/home/nguyenhoangnam/.local/share/lsp/lemminx/org.eclipse.lemminx/target/org.eclipse.lemminx-uber.jar",
        },
    },
}
