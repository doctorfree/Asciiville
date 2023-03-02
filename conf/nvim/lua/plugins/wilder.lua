local wilder = require('wilder')
wilder.setup({ modes = { ':', '/', '?' } })

local gradient = {
    '#f4468f', '#fd4a85', '#ff507a', '#ff566f', '#ff5e63',
    '#ff6658', '#ff704e', '#ff7a45', '#ff843d', '#ff9036',
    '#f89b31', '#efa72f', '#e6b32e', '#dcbe30', '#d2c934',
    '#c8d43a', '#bfde43', '#b6e84e', '#aff05b'
}

for i, fg in ipairs(gradient) do
    gradient[i] = wilder.make_hl('WilderGradient' .. i, 'Pmenu', { { a = 1 }, { a = 1 }, { foreground = fg } })
end

--- local airlinemenu_renderer = wilder.wildmenu_renderer(
---     wilder.wildmenu_airline_theme({
---         highlighter = wilder.lua_fzy_highlighter(),
---         separator = ' · ',
---     })
--- )

local popupmenu_renderer = wilder.popupmenu_renderer(
    wilder.popupmenu_border_theme({
        winblend = 20,
        border = 'rounded',
        empty_message = wilder.popupmenu_empty_message_with_spinner(),
        -- }),
        highlights = {
            gradient = gradient, -- must be set
            -- selected_gradient key can be set to apply gradient highlighting for the selected candidate.
        },
        highlighter = wilder.highlighter_with_gradient({
            wilder.lua_fzy_highlighter(), -- or wilder.lua_fzy_highlighter(),
        }),
        left = {
            ' ',
            wilder.popupmenu_devicons(),
            wilder.popupmenu_buffer_flags({
                flags = ' A + ',
                icons = { ['+'] = 'modified', a = 'active', h = 'hidden', ['%'] = 'current  ', ['#'] = 'alternate' },
            }),
        },
        right = {
            ' ',
            wilder.popupmenu_scrollbar(),
        },
    })
)

local wildmenu_renderer = wilder.wildmenu_renderer({
    highlighter = wilder.highlighter_with_gradient({
        wilder.lua_fzy_highlighter(), -- or wilder.lua_fzy_highlighter(),
    }),
    separator = ' ¬∑ ',
    left = { ' ', wilder.wildmenu_spinner(), ' ' },
    right = { ' ', wilder.wildmenu_index() },
})

wilder.set_option('renderer', wilder.renderer_mux({
    [':'] = popupmenu_renderer,
    ['?'] = popupmenu_renderer,
    ['/'] = popupmenu_renderer,
    substitute = wildmenu_renderer
--- ['?'] = airlinemenu_renderer,
--- ['/'] = airlinemenu_renderer,
--- substitute = wildmenu_renderer
}))
