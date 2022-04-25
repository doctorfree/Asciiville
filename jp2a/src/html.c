/*
 * Copyright 2006-2016 Christian Stigen Larsen
 * Copyright 2020 Christoph Raitzig
 * Copyright 2022 Ronald Record
 * Distributed under the GNU General Public License (GPL) v2.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "html.h"
#include "options.h"

void print_html_document_start(const int fontsize, FILE *f) {

	fputs("<!DOCTYPE html>\n"
		"<html>\n"
		" <head>\n"
		" <meta charset=\"utf-8\">\n", f);
	fprintf(f,
		" <title>%s</title>\n", html_title);
	fputs(
		"  <style>\n", f);
	print_css(fontsize, f);
	fputs(
		"  </style>\n"
		" </head>\n"
		"<body>\n", f);
}

void print_html_image_start(FILE *f) {
	fputs("<div class='ascii'><pre>\n", f);
}

void print_html_document_end(FILE *f) {
	fputs("</body>\n</html>\n", f);
}

void print_html_image_end(FILE *f) {
	fputs("</pre>\n</div>\n", f);
}

#if ASCII
void print_html_char(FILE *f, const char ch,
	const int r_fg, const int g_fg, const int b_fg,
	const int r_bg, const int g_bg, const int b_bg)
#else
void print_html_char(FILE *f, const char* ch,
	const int r_fg, const int g_fg, const int b_fg,
	const int r_bg, const int g_bg, const int b_bg)
#endif
{
	if ( colorfill ) {
		fprintf(f, "<span style='color:#%02x%02x%02x; background-color:#%02x%02x%02x;'>%s</span>",
			r_fg, g_fg, b_fg,
			r_bg, g_bg, b_bg,
			html_entity(ch));
	} else
		fprintf(f, "<span style='color:#%02x%02x%02x;'>%s</span>",
			r_fg, g_fg, b_fg, html_entity(ch));
}

void print_html_newline(FILE *f) {
	fputs("<br>", f);
}

void print_xhtml_document_start(const int fontsize, FILE *f) {
	
	fputs(   "<?xml version='1.0' encoding='UTF-8' ?>\n"
		"<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN'"
		"  'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>\n"
		"<html xmlns='http://www.w3.org/1999/xhtml' lang='en' xml:lang='en'>\n"
		"<head>\n", f);
	fprintf(f,
		"<title>%s</title>\n", html_title);
	fputs(
		"<style type='text/css'>\n", f);
	print_css(fontsize, f);
	fputs(
		"</style>\n"
		"</head>\n"
		"<body>\n", f);
}

void print_xhtml_image_start(FILE *f) {
	fputs("<div class='ascii'><pre>\n", f);
}

void print_xhtml_document_end(FILE *f) {
	fputs("</body>\n</html>\n", f);
}

void print_xhtml_image_end(FILE *f) {
	fputs("</pre>\n</div>\n", f);
}

#if ASCII
void print_xhtml_char(FILE *f, const char ch,
	const int r_fg, const int g_fg, const int b_fg,
	const int r_bg, const int g_bg, const int b_bg)
#else
void print_xhtml_char(FILE *f, const char* ch,
	const int r_fg, const int g_fg, const int b_fg,
	const int r_bg, const int g_bg, const int b_bg)
#endif
{
	if ( colorfill ) {
		fprintf(f, "<span style='color:#%02x%02x%02x; background-color:#%02x%02x%02x;'>%s</span>",
			r_fg, g_fg, b_fg,
			r_bg, g_bg, b_bg,
			html_entity(ch));
	} else
		fprintf(f, "<span style='color:#%02x%02x%02x;'>%s</span>",
			r_fg, g_fg, b_fg, html_entity(ch));
}

void print_xhtml_newline(FILE *f) {
	fputs("<br/>", f);
}

void print_css(const int fontsize, FILE *f) {
	fputs("body {\n", f);
	fputs(!invert?
		"   background-color: white;\n" : "background-color: black;\n", f);
	fputs(  "}\n"
		".ascii {\n"
		"   font-family: Courier;\n", f); // should be a monospaced font
	if ( !usecolors )
	fputs(!invert?
		"   color: black;\n" : "   color: white;\n", f);
	fprintf(f,
		"   font-size:%dpt;\n", fontsize);
	if ( html_bold )
	fputs( 	"   font-weight: bold;\n", f);
	else
	fputs(  "   font-weight: normal;\n", f);
	fputs(
		"}\n", f);
}

#if ASCII
const char* html_entity(const char ch) { // if a html entity is larger than 6
		// chars, change escape_title accordingly
	static char s[2];
	switch ( ch ) {
#else
const char* html_entity(const char* ch) {
	switch ( ch[0] ) {
#endif
	case ' ': return "&nbsp;"; break;
	case '<': return "&lt;"; break;
	case '>': return "&gt;"; break;
	case '&': return "&amp;"; break;
	default:
#if ASCII
		s[0]=ch; s[1]=0; return s; break;
#else
		return ch;
#endif
	}
}

int escape_title() {
	if ( strlen(html_title_raw)==0 ) {
		return 1;
	}
	html_title = calloc(strlen(html_title_raw)*6 +1, sizeof(char)); // at most
			// 6 characters are returned by html_entity for each character
	if ( html_title==NULL ) {
		return 0;
	}
	int j = 0;
	int sizeNew;
	char* newChar;
	for (int i = 0; i < strlen(html_title_raw); i++) {
		if ( html_title_raw[i]=='&' ) { // otherwise HTML entities could not be
			// used on purpose
			html_title[j++] = '&';
			continue;
		}
#if ASCII
		const char* newChar = html_entity(html_title_raw[i]);
#else
		char tempString[2];
		tempString[0] = html_title_raw[i];
		tempString[1] = '\0';
		const char* newChar = html_entity(tempString);
#endif
		sizeNew = strlen(newChar);
		for (int k = 0; k < sizeNew; k++) {
			html_title[j+k] = newChar[k];
		}
		j += sizeNew;
	}
	if ( realloc(html_title, (j+1) * sizeof(char))==NULL )
		return 0;
	return 1;
}
