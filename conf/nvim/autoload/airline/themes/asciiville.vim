"----------------------------------------------------------------
"                         ,e, ,e,           ,e, 888 888           
"  /~~~8e   d88~\  e88~~\  "   "  Y88b    /  "  888 888  e88~~8e  
"      88b C888   d888    888 888  Y88b  /  888 888 888 d888  88b 
" e88~-888  Y88b  8888    888 888   Y88b/   888 888 888 8888__888 
"C888  888   888D Y888    888 888    Y8/    888 888 888 Y888    , 
" "88_-888 \_88P   "88__/ 888 888     Y     888 888 888  "88___/  
"                                                                 
"----------------------------------------------------------------
"  Theme   : Asciiville
"  Version : 1.0.0
"  License : MIT
"  Author  : Ronald Record
"  URL     : https://github.com/doctorfree/nvim.git
" ----------------------------------------------------------------
"  Colors will be adapted to the current colorscheme.
" ----------------------------------------------------------------

let g:airline#themes#asciiville#palette = {}

function! airline#themes#asciiville#refresh()

	let s:N1 = airline#themes#get_highlight2(['LineNr', 'bg'], ['ModeMsg', 'fg'], 'none')
	let s:N2 = airline#themes#get_highlight2(['LineNr', 'bg'], ['LineNr', 'fg'], 'none')
	let s:N3 = airline#themes#get_highlight2(['ModeMsg', 'fg'], ['StatusLine', 'bg'], 'none')
	let g:airline#themes#asciiville#palette.normal = airline#themes#generate_color_map(s:N1, s:N2, s:N3)

	let s:I1 = airline#themes#get_highlight2(['LineNr', 'bg'], ['Question', 'fg'], 'none')
	let s:I2 = airline#themes#get_highlight2(['LineNr', 'bg'], ['LineNr', 'fg'], 'none')
	let s:I3 = airline#themes#get_highlight2(['Question', 'fg'], ['StatusLine', 'bg'], 'none')
	let g:airline#themes#asciiville#palette.insert = airline#themes#generate_color_map(s:I1, s:I2, s:I3)

	let s:R1 = airline#themes#get_highlight2(['LineNr', 'bg'], ['ErrorMsg', 'fg'], 'none')
	let s:R2 = airline#themes#get_highlight2(['LineNr', 'bg'], ['LineNr', 'fg'], 'none')
	let s:R3 = airline#themes#get_highlight2(['ErrorMsg', 'fg'], ['StatusLine', 'bg'], 'none')
	let g:airline#themes#asciiville#palette.replace = airline#themes#generate_color_map(s:R1, s:R2, s:R3)

	let s:V1 = airline#themes#get_highlight2(['LineNr', 'bg'], ['WarningMsg', 'fg'], 'none')
	let s:V2 = airline#themes#get_highlight2(['LineNr', 'bg'], ['LineNr', 'fg'], 'none')
	let s:V3 = airline#themes#get_highlight2(['WarningMsg', 'fg'], ['StatusLine', 'bg'], 'none')
	let g:airline#themes#asciiville#palette.visual = airline#themes#generate_color_map(s:V1, s:V2, s:V3)

	let s:IA1 = airline#themes#get_highlight2(['LineNr', 'fg'], ['StatusLine', 'bg'], 'none')
	let s:IA2 = airline#themes#get_highlight2(['LineNr', 'fg'], ['StatusLine', 'bg'], 'none')
	let s:IA3 = airline#themes#get_highlight2(['LineNr', 'fg'], ['StatusLine', 'bg'], 'none')
	let g:airline#themes#asciiville#palette.inactive = airline#themes#generate_color_map(s:IA1, s:IA2, s:IA3)

	" Accent color
	" It helps to remove the bold typography into modes section
	let g:airline#themes#asciiville#palette.accents = {'black' : airline#themes#get_highlight2(['LineNr', 'bg'], ['ModeMsg', 'fg'], 'none')}

	" Settings
	let g:airline_symbols.paste = 'Ξ'
	let g:airline_symbols.spell = 'S'

endfunction

call airline#themes#asciiville#refresh()
