/*
 * Copyright 2006-2016 Christian Stigen Larsen
 * Copyright 2020 Christoph Raitzig
 * Copyright 2022 Ronald Record
 * Distributed under the GNU General Public License (GPL) v2.
 */

#include "config.h"

#include <stdio.h>
#include <math.h>
#include <wchar.h>
#include <limits.h>
#include <errno.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_CURSES_H
#include <curses.h>
#endif

#ifdef HAVE_TERM_H
#include <term.h>
#endif

#include "jp2a.h"
#include "options.h"
#include "terminal.h"
#include "html.h"

// Default options
int verbose = 0;
int auto_height = 1;
int auto_width = 0;

int width =
#ifdef FEAT_TERMLIB
 0;
#else
 78;
#endif

int height = 0;
int use_border = 0;
int invert = 1;
int flipx = 0;
int flipy = 0;
int html = 0;
int xhtml = 0;
int colorfill = 0;
int convert_grayscale = 0;
int html_fontsize = 8;
int html_bold = 1;
const char* html_title_raw = HTML_DEFAULT_TITLE;
char* html_title = NULL;
int html_rawoutput = 0;
int debug = 0;
int clearscr = 0;
int term_width = 0;
int term_height = 0;
int usecolors = 0;
int colorDepth = 0;

int termfit =
#ifdef FEAT_TERMLIB
 TERM_FIT_AUTO;
#else
 0;
#endif

int ascii_palette_length = 23;
#if ASCII
char ascii_palette[ASCII_PALETTE_SIZE + 1] = ASCII_PALETTE_DEFAULT;
#else
char ascii_palette[ASCII_PALETTE_SIZE * MB_LEN_MAX + 1] = ASCII_PALETTE_DEFAULT;
unsigned char ascii_palette_indizes[ASCII_PALETTE_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};
char ascii_palette_lengths[ASCII_PALETTE_SIZE] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
#endif

// Default weights
float redweight = 0.2989f;
float greenweight = 0.5866f;
float blueweight = 0.1145f;

// calculated in parse_options
float RED[256], GREEN[256], BLUE[256], ALPHA[256], GRAY[256];

const char *fileout = "-";

const char* version   = PACKAGE_STRING;
const char* copyright = "Copyright 2006-2016 Christian Stigen Larsen\n"
	"Copyright 2020 Christoph Raitzig\n"
	"Copyright 2022 Ronald Record";
const char* license   = "Distributed under the GNU General Public License (GPL) v2.";
const char* url       = PACKAGE_URL;

void print_version() {
	fprintf(stderr, "%s\n%s\n%s\n", version, copyright, license);
}

void help() {
	print_version();

	fputs(
"\n"
#ifdef FEAT_CURL
"Usage: jp2a [ options ] [ file(s) | URL(s) ]\n\n"

"Convert files or URLs from JPEG/PNG format to ASCII.\n\n"
#else
"Usage: jp2a [ options ] [ file(s) ]\n\n"

"Convert files in JPEG/PNG format to ASCII.\n\n"
#endif
"OPTIONS\n"
"  -                 Read images from standard input.\n"
"      --blue=N.N    Set RGB to grayscale conversion weight, default is 0.1145\n"
"  -b, --border      Print a border around the output image.\n"
"      --chars=...   Select character palette used to paint the image.\n"
"                    Leftmost character corresponds to black pixel, right-\n"
"                    most to white.  Minimum two characters must be specified.\n"
"      --clear       Clears screen before drawing each output image.\n"
"      --colors      Use true colors or, if true color is not supported, ANSI\n"
"                    in output.\n"
"      --color-depth=N   Use a specific color-depth for terminal output. Valid\n"
"                        values are: 4 (for ANSI), 8 (for 256 color palette)\n"
"                        and 24 (for truecolor or 24-bit color).\n"
"  -d, --debug       Print additional debug information.\n"
"      --fill        When used with --color and/or --htmlls or --xhtml, color\n"
"                    each character's background.\n"
"  -x, --flipx       Flip image in X direction.\n"
"  -y, --flipy       Flip image in Y direction.\n"
#ifdef FEAT_TERMLIB
"  -f, --term-fit    Use the largest image dimension that fits in your terminal\n"
"                    display with correct aspect ratio.\n"
"      --term-height Use terminal display height.\n"
"      --term-width  Use terminal display width.\n"
"  -z, --term-zoom   Use terminal display dimension for output.\n"
#endif
"      --grayscale   Convert image to grayscale when using --htmlls or --xhtml\n"
"                    or --colors\n"
"      --green=N.N   Set RGB to grayscale conversion weight, default is 0.5866\n"
"      --height=N    Set output height, calculate width from aspect ratio.\n"
"  -h, --help        Print program help.\n"
"      --htmlls      Produce HTML (Living Standard) output.\n"
"      --html        Produce strict XHTML 1.0 output (will produce HTML output\n"
"                    from version 2.0.0 onward).\n"
"      --xhtml       Produce strict XHTML 1.0 output.\n" // Obsoletely Fabulous
"      --html-fill   Same as --fill (will be phased out).\n"
"      --html-fontsize=N   Set fontsize to N pt, default is 4.\n"
"      --html-no-bold      Do not use bold characters with HTML output\n"
"      --html-raw    Output raw HTML codes, i.e. without the <head> section etc.\n"
"                    (Will use <br> for version 2.0.0 and above.)\n"
"      --html-title=...  Set HTML output title\n"
"  -i, --invert      Invert output image.  Use if your display has a dark\n"
"                    background.\n"
"      --background=dark   These are just mnemonics whether to use --invert\n"
"      --background=light  or not.  If your console has light characters on\n"
"                    a dark background, use --background=dark.\n"
"      --output=...  Write output to file.\n"
"      --red=N.N     Set RGB to grayscale conversion weight, default 0.2989f.\n"
"      --size=WxH    Set output width and height.\n"
"  -v, --verbose     Verbose output.\n"
"  -V, --version     Print program version.\n"
"      --width=N     Set output width, calculate height from ratio.\n"
"\n"
#ifdef FEAT_TERMLIB
"  The default mode is `jp2a --term-fit --background=dark'.\n"
#else
"  The default mode is `jp2a --width=78 --background=dark'.\n"
#endif
"  See the man-page for jp2a for more detailed help text.\n"
"\n", stderr);

	fprintf(stderr, "Project homepage on %s\n", url);
	fprintf(stderr, "Report bugs to <%s>\n", PACKAGE_BUGREPORT);
}

void precalc_rgb(float red, float green, float blue) {
	int n;
	float sum = red + green + blue;
	red /= sum;
	green /= sum;
	blue /= sum;
	for ( n=0; n<256; ++n ) {
		RED[n]   = ((float) n) * red / 255.0f;
		GREEN[n] = ((float) n) * green / 255.0f;
		BLUE[n]  = ((float) n) * blue / 255.0f;
		ALPHA[n] = ((float) n) / 255.0f;
		GRAY[n]  = ((float) n) / 255.0f;
	}
}

void parse_options(int argc, char** argv) {
	// make code more readable
	#define IF_OPTS(sopt, lopt)     if ( !strcmp(s, sopt) || !strcmp(s, lopt) )
	#define IF_OPT(sopt)            if ( !strcmp(s, sopt) )
	#define IF_VARS(format, v1, v2) if ( sscanf(s, format, v1, v2) == 2 )
	#define IF_VAR(format, v1)      if ( sscanf(s, format, v1) == 1 )

	int n, files, fit_to_use;

	for ( n=1, files=0; n<argc; ++n ) {
		const char *s = argv[n];

		if ( *s != '-' ) { // count files to read
			++files; continue;
		}
	
		IF_OPT ("-")                             { ++files; continue; }
		IF_OPTS("-h", "--help")                  { help(); exit(0); }
		IF_OPTS("-v", "--verbose")               { verbose = 1; continue; }
		IF_OPTS("-d", "--debug")                 { debug = 1; continue; }
		IF_OPT ("--clear")                       { clearscr = 1; continue; }
		IF_OPTS("--color", "--colors")           { usecolors = 1;
		if ( debug ) {
			char *colorterm = getenv("COLORTERM");
			if ( colorterm==NULL ) {
				fprintf(stderr, "Environment variable COLORTERM not set.\n");
			} else {
				fprintf(stderr, "Environment variable COLORTERM: %s\n", colorterm);
			}
		}
		colorDepth = ( supports_true_color() )? 24 : 4; continue; }
		IF_VAR ("--color-depth=%d", &colorDepth) {
			switch(colorDepth) {
				case 4:
				case 8:
				case 24:
					usecolors = 1;
					break;
				default:
					colorDepth = 0;
					usecolors = 0;
			}
			continue; }
		IF_OPT ("--fill")                        { colorfill = 1; continue; }
		IF_OPT ("--grayscale")                   { usecolors = 1; convert_grayscale = 1; continue; }
		IF_OPT ("--htmlls")                      { html = 1; continue; }
		IF_OPT ("--html")                        { xhtml = 1; continue; }
		IF_OPT ("--xhtml")                       { xhtml = 1; continue; }
		IF_OPT ("--html-fill")                   { colorfill = 1; fputs("warning: --html-fill has changed to --fill\n", stderr); continue; } // TODO: phase out
		IF_OPT ("--html-no-bold")                { html_bold = 0; continue; }	
		IF_OPT ("--html-raw")                    { xhtml = 1; html_rawoutput = 1; continue; }
		IF_OPTS("-b", "--border")                { use_border = 1; continue; }
		IF_OPTS("-i", "--invert")                { invert = !invert; continue; }
		IF_OPT("--background=dark")              { invert = 1; continue; }
		IF_OPT("--background=light")             { invert = 0; continue; }
		IF_OPTS("-x", "--flipx")                 { flipx = 1; continue; }
		IF_OPTS("-y", "--flipy")                 { flipy = 1; continue; }
		IF_OPTS("-V", "--version")               { print_version(); exit(0); }
		IF_VAR ("--width=%d", &width)            { auto_height += 1; continue; }
		IF_VAR ("--height=%d", &height)          { auto_width += 1; continue; }
		IF_VAR ("--red=%f", &redweight)          { continue; }
		IF_VAR ("--green=%f", &greenweight)      { continue; }
		IF_VAR ("--blue=%f", &blueweight)        { continue; }
		IF_VAR ("--html-fontsize=%d",
			&html_fontsize)             { continue; }

		IF_VARS("--size=%dx%d",&width, &height) {
			auto_width = auto_height = 0; continue;
		}

#ifdef FEAT_TERMLIB
		IF_OPTS("-z", "--term-zoom")        { termfit = TERM_FIT_ZOOM; continue; }
		IF_OPT ("--term-height")            { termfit = TERM_FIT_HEIGHT; continue; }
		IF_OPT ("--term-width")             { termfit = TERM_FIT_WIDTH; continue; }
		IF_OPTS("-f", "--term-fit")         { termfit = TERM_FIT_AUTO; continue; }
#endif

		if ( !strncmp(s, "--output=", 9) ) {
			fileout = s+9;
			continue;
		}

		if ( !strncmp(s, "--html-title=", 13) ) {
			html_title_raw = s + 13;
			continue;
		}

		if ( !strncmp(s, "--chars=", 8) ) {

#if ASCII
			if ( strlen(s)-8 > ASCII_PALETTE_SIZE ) {
				fprintf(stderr,
					"Too many ascii characters specified (max %d)\n",
					ASCII_PALETTE_SIZE);
				exit(1);
			}
#else
			if ( strlen(s)-8 > ASCII_PALETTE_SIZE * MB_LEN_MAX ) {
				fprintf(stderr,
					"Too many characters specified (max %d)\n",
					ASCII_PALETTE_SIZE);
				exit(1);
			}
#endif
	
			// don't use sscanf, we need to read spaces as well
			strcpy(ascii_palette, s+8);
#if ASCII
			ascii_palette_length = strlen(ascii_palette);
#else
			int i = 0;
			int count = 0;
			size_t curCharlen;
			while ( ascii_palette[i] != '\0' ) {
				ascii_palette_indizes[count] = i;
				curCharlen = mbrlen(ascii_palette + i, MB_LEN_MAX, NULL);
				ascii_palette_lengths[count] = curCharlen;
				if ( curCharlen == -1 ) {
					fprintf(stderr, "Error with custom chars: %s\n", strerror(errno));
					exit(1);
				} else
				if ( curCharlen == -2 ) {
					fprintf(stderr, "Error while parsing custom chars.");
					exit(1);
				}
				i += curCharlen;
				count++;
			}
			if ( count > ASCII_PALETTE_SIZE ) {
				fprintf(stderr,
					"Too many characters specified (max %d)\n",
					ASCII_PALETTE_SIZE);
				exit(1);
			}
			ascii_palette_length = count;
#endif
			continue;
		}

		fprintf(stderr, "Unknown option %s\n\n", s);
		help();
		exit(1);

	} // args ...

	if ( !files ) {
		fputs("No files specified.\n\n", stderr);
		help();
		exit(1);
	}

#ifdef FEAT_TERMLIB
	if ( (width || height) && termfit==TERM_FIT_AUTO ) {
		// disable default --term-fit if dimensions are given
		termfit = 0;
	}
#endif

	if ( termfit ) {
		char* err = "";

		if ( get_termsize(&term_width, &term_height, &err) <= 0 ) {
			fputs(err, stderr);
			fputc('\n', stderr);
			exit(1);
		}

#ifdef __CYGWIN__
	// On Cygwin, if I don't decrement term_width, then you'll get extra
	// blank lines for some window sizes, hence we decrease by one.
	--term_width;
#endif

		fit_to_use = termfit;

		if ( termfit == TERM_FIT_AUTO ) {
			// use the smallest of terminal width or height 
			// to guarantee that image fits in display.

			if ( term_width <= term_height )
				fit_to_use = TERM_FIT_WIDTH;
			else
				fit_to_use = TERM_FIT_HEIGHT;
		}

		switch ( fit_to_use ) {
		case TERM_FIT_ZOOM:
			auto_width = auto_height = 0;
			width = term_width - use_border*2;
			height = term_height - 1 - use_border*2;
			break;

		case TERM_FIT_WIDTH:
			width = term_width - use_border*2;
			height = 0;
			auto_height += 1;
			break;

		case TERM_FIT_HEIGHT:
			width = 0;
			height = term_height - 1 - use_border*2;
			auto_width += 1;
			break;
		}
	}

	// only --width specified, calc width
	if ( auto_width==1 && auto_height == 1 )
		auto_height = 0;

	// --width and --height is the same as using --size
	if ( auto_width==2 && auto_height==1 )
		auto_width = auto_height = 0;

	if ( ascii_palette_length < 2 ) {
		fputs("You must specify at least two characters in --chars.\n",
			stderr);
		exit(1);
	}
	
	if ( (width < 1 && !auto_width) || (height < 1 && !auto_height) ) {
		fputs("Invalid width or height specified\n", stderr);
		exit(1);
	}

	if ( redweight < 0 || greenweight < 0 || blueweight < 0 ) {
		fputs("Weights can't be negative.\n", stderr);
		exit(1);
	}
	if ( !isfinite(redweight) || !isfinite(greenweight) || !isfinite(blueweight) ) {
		// This can happen if a number can not be represented as floating point, e.g. 3e400.
		fputs("Did not understand a weight - possibly to large.\n", stderr);
		exit(1);
	}
	if ( (redweight + greenweight + blueweight) == 0.0 ) {
		fputs("At least one weight must be non-zero.\n", stderr);
		exit(1);
	}

	if ( *fileout == 0 ) {
		fputs("Empty output filename.\n", stderr);
		exit(1);
	}

	if ( html && xhtml ) {
		fputs("Only HTML or XHTML possible, using HTML.\n", stderr);
		xhtml = 0;
	}

	if ( html || xhtml ) {
		if ( !escape_title() ) {
			fprintf(stderr, "Not enough memory.");
			exit(1);
		}
	}

	precalc_rgb(redweight, greenweight, blueweight);
}
