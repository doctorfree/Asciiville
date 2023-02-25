return {

    efm = {
        {
            formatCommand = "prettier --stdin-filepath ${INPUT}",
            formatStdin = true,
        },
        {
            lintCommand = "markdownlint -s",
            lintStdin = true,
            lintFormats = {
                "%f:%l %m",
                "%f:%l:%c %m",
                "%f: %l: %m",
            },
        },
    },

    all_format = { efm = "Prettier" },
    default_format = "efm",

    template = {
        ["1"] = [[
        # 



        ## Installation



        ## Usage



        ## Contributing

        Pull requests are welcome. For major changes, 
        please open an issue first to discuss what you would like to change.

        Please make sure to update tests as appropriate.

        ## License

        [MIT](https://choosealicense.com/licenses/mit/)
        ]],
    },
}
