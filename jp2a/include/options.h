/*! \file
 * \noop Copyright 2006-2016 Christian Stigen Larsen
 * \noop Copyright 2020 Christoph Raitzig
 * \noop Copyright 2022 Ronald Record
 *
 * \brief Functions for parsing command line arguments.
 *
 * \author Christian Stigen Larsen
 * \author Christoph Raitzig
 * \copyright Distributed under the GNU General Public License (GPL) v2.
 */

#ifndef INC_JP2A_OPTIONS_H
#define INC_JP2A_OPTIONS_H

//! size of the ASCII palette (in chars)
#define ASCII_PALETTE_SIZE 256
//! default ASCII palette
#define ASCII_PALETTE_DEFAULT "   ...',;:clodxkO0KXNWM";
//! default title for HTML/XHTML documents
#define HTML_DEFAULT_TITLE "jp2a converted image"

extern int verbose; //!< produce verbose output
extern int auto_height; //!< automatically calculate the output width from the height
extern int auto_width; //!< automatically calculate the output height from the width
extern int width; //!< output width
extern int height; //!< output height
extern int use_border; //!< print a border around images
extern int invert; //!< invert output (color/gray channels, not alpha)
extern int flipx; //!< flip output along x axis
extern int flipy; //!< flip output along y axis
extern int html; //!< produce HTML output
extern int xhtml; //!< produce XHTML output
extern int html_fontsize; //!< font size for HTML/XHTML output
extern int colorfill; //!< also fill the background
extern int convert_grayscale; //!< produce grayscale output
extern const char *html_title_raw; //!< HTML title
extern char *html_title; //!< HTML title with escaped HTML entities
extern int html_rawoutput; //!< produce only the image part and not the rest of the website
extern int html_bold; //!< use bold characters in HTML output
extern int debug; //!< print debug information
extern int clearscr; //!< clear the screen before printing an image
extern int ascii_palette_length; //!< number of charactrs in the palette
extern char ascii_palette[]; //!< the palette
#if ! ASCII
extern unsigned char ascii_palette_indizes[]; //!< indizes of the first byte of each char in the palette
extern char ascii_palette_lengths[]; //!< byte lengths of the chars in the palette
#endif
extern float redweight; //!< weight for red
extern float greenweight; //!< weight for green
extern float blueweight; //!< weight for blue
extern float RED[256]; //!< red channel lookup table
extern float GREEN[256]; //!< green channel lookup table
extern float BLUE[256]; //!< blue channel lookup table
extern float ALPHA[256]; //!< alpha channel lookup table
extern float GRAY[256]; //!< gray channel lookup table
extern const char *fileout; //!< name of the output file (or - for stdout)
extern int usecolors; //!< produce colored output
extern int colorDepth; //!< output color bit depth (4 for ANSI, 8 for 256 colors, 24 for true color)
extern int termfit; //!< the way the output dimensions are calculated
extern int term_width; //!< width of the terminal
extern int term_height; //!< height of the terminal
#define TERM_FIT_ZOOM 1 //!< stretch image to fill the terminal
#define TERM_FIT_WIDTH 2 //!< calculate the width (based on the height)
#define TERM_FIT_HEIGHT 3 //!< calculate the height (based on the width)
#define TERM_FIT_AUTO 4 //!< use the largest possible dimensions while retaining the input image aspect ratio

/*!
 * \brief Prints version information.
 */
void print_version();

/*!
 * \brief Prints usage, version information etc.
 */
void help();

/*!
 * \brief Calculates red, green, blue, gray and alpha 8-bit to float lookup tables.
 *
 * \param red,green,blue weights for each color
 */
void precalc_rgb(const float red, const float green, const float blue);

/*!
 * \brief Parses the command line arguments.
 *
 * Options are saved with the global variables.
 * Exits if help or the version is printed.
 *
 * \param argc argument count
 * \param argv the arguments
 */
void parse_options(int argc, char** argv);

#endif
