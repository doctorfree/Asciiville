local lsp = require("languages.lsp")

return {
    efm = {
        {
            lintCommand = "hadolint --no-color ${INPUT}",
            lintStdin = false,
            lintFormats = {
                "%f:%l %m",
            },
        },
    },

    all_format = { dockerls = "DockerLS" },
    default_format = "dockerls",
    lsp_server = "dockerls",

    lsp = {
        capabilities = lsp.capabilities,
        on_attach = function(client, bufnr)
            lsp.on_attach(client, bufnr)
            require("nvim-navic").attach(client, bufnr)
        end,
    },
}
