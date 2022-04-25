/*! \file
 * \noop Copyright 2020 Christoph Raitzig
 * \noop Copyright 2022 Ronald Record
 * 
 * \brief HTML/XHTML functions.
 *
 * \author Christoph Raitzig
 * \copyright Distributed under the GNU General Public License (GPL) v2.
 */

#ifndef INC_JP2A_HTML_H
#define INC_JP2A_HTML_H

#include "config.h"

/*!
 * \brief Prints the start of a HTML document.
 *
 * \param fontsize font size
 * \param fout stream to print to
 */
void print_html_document_start(const int fontsize, FILE *fout);

/*!
 * \brief Prints the start of a HTML image.
 *
 * \param f stream to print to
 */
void print_html_image_start(FILE *f);

/*!
 * \brief Prints the end of a HTML document.
 *
 * \param fout stream to print to
 */
void print_html_document_end(FILE *fout);

/*!
 * \brief Prints the end of a HTML image.
 *
 * \param f stream to print to
 */
void print_html_image_end(FILE *f);
#if ASCII

/*!
 * \brief Prints a char of the output image for HTML.
 *
 * \param fout stream to print to
 * \param ch char to print
 * \param red_fg,green_fg,blue_fg foreground color
 * \param red_bg,green_bg,blue_bg background color
 */
void print_html_char(FILE *fout, const char ch,
	const int red_fg, const int green_fg, const int blue_fg,
	const int red_bg, const int green_bg, const int blue_bg);
#else

/*!
 * \brief Prints a char of the output image for HTML.
 *
 * \param fout stream to print to
 * \param ch char to print
 * \param red_fg,green_fg,blue_fg foreground color
 * \param red_bg,green_bg,blue_bg background color
 */
void print_html_char(FILE *fout, const char* ch,
	const int red_fg, const int green_fg, const int blue_fg,
	const int red_bg, const int green_bg, const int blue_bg);
#endif

/*!
 * \brief Prints a new line (for HTML, i.e. "<br>").
 *
 * \param fout stream to print to
 */
void print_html_newline(FILE *fout);

/*!
 * \brief Prints the start of a XHTML document.
 *
 * \param fontsize font size
 * \param fout stream to print to
 */
void print_xhtml_document_start(const int fontsize, FILE *fout);

/*!
 * \brief Prints the start of a XHTML image.
 *
 * \param f stream to print to
 */
void print_xhtml_image_start(FILE *f);

/*!
 * \brief Prints the end of a XHTML document.
 *
 * \param fout stream to print to
 */
void print_xhtml_document_end(FILE *fout);

/*!
 * \brief Prints the end of a XHTML image.
 *
 * \param f stream to print to
 */
void print_xhtml_image_end(FILE *f);
#if ASCII

/*!
 * \brief Prints a char of the output image for XHTML.
 *
 * \param fout stream to print to
 * \param ch char to print
 * \param red_fg,green_fg,blue_fg foreground color
 * \param red_bg,green_bg,blue_bg background color
 */
void print_xhtml_char(FILE *fout, const char ch,
	const int red_fg, const int green_fg, const int blue_fg,
	const int red_bg, const int green_bg, const int blue_bg);
#else

/*!
 * \brief Prints a char of the output image for XHTML.
 *
 * \param fout stream to print to
 * \param ch char to print
 * \param red_fg,green_fg,blue_fg foreground color
 * \param red_bg,green_bg,blue_bg background color
 */
void print_xhtml_char(FILE *fout, const char* ch,
	const int red_fg, const int green_fg, const int blue_fg,
	const int red_bg, const int green_bg, const int blue_bg);
#endif

/*!
 * \brief Prints a new line (for XHTML, i.e. "<br/>").
 *
 * \param fout stream to print to
 */
void print_xhtml_newline(FILE *fout);
#if ASCII

/*!
 * \brief Converts a char to a HTML entity if necessary.
 *
 * For example, "<" is converted to "&lt;".
 *
 * \param ch char to convert
 * \return the HTML entity or the original char if a conversion is not necassary
 */
const char* html_entity(const char ch);
#else

/*!
 * \brief Converts a char to a HTML entity if necessary.
 *
 * For example, "<" is converted to "&lt;".
 *
 * \param ch char to convert
 * \return the HTML entity or the original char if a conversion is not necassary
 */
const char* html_entity(const char* ch);
#endif

/*!
 * \brief Prints the CSS for a HTML/XHTML document.
 *
 * \param fontsize font size
 * \param f stream to print to
 */
void print_css(const int fontsize, FILE *f);

/*!
 * \brief Escapes the title as necesary.
 *
 * Takes the title in #html_title_raw and escapes chars to HTML entities as necessary and saves the resulting title in #html_title.
 *
 * \return true if successful, false otherwise
 */
int escape_title();

#endif
