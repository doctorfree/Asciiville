/*
Copyright (c) 2003 Bruno T. C. de Oliveira

LICENSE INFORMATION:
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public
License along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
Copyright (c) 2002 Bruno T. C. de Oliveira

INFORMAÇÕES DE LICENÇA:
Este programa é um software de livre distribuição; você pode
redistribuí-lo e/ou modificá-lo sob os termos da GNU General
Public License, conforme publicado pela Free Software Foundation,
pela versão 2 da licença ou qualquer versão posterior.

Este programa é distribuído na esperança de que ele será útil
aos seus usuários, porém, SEM QUAISQUER GARANTIAS; sem sequer
a garantia implícita de COMERCIABILIDADE ou DE ADEQUAÇÃO A
QUALQUER FINALIDADE ESPECÍFICA. Consulte a GNU General Public
License para obter mais detalhes (uma cópia acompanha este
programa, armazenada no arquivo COPYING).
*/

#include "document.h"
#include "vlayer.h"
#include "export.h"
#include <stdbool.h>

#include "bores/bores.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

bool clear_screen = false;
bool suppress_newlines = false;
                           /* whether to suppress newlines on output. */
bool use_color = true;
char *outputfile;

FILE *f;

Document *doc;    /* the document we are outputting */
int layer_number; /* the layer we will output; -1 means composite */
Layer *lyr;       /* the layer we are printing; NULL if printing composite */
int width, height;

struct html_output_state_t {
   bool font_tag_open, blink_tag_open;  /* which html tags are open */
   int color;       /* The color we are currently outputting text in.
                     * This is a full 0-15 color code.*/
   int bgcolor;     /* The color we are currently putting text onto.
                     * This is a full 0-15 color code.*/
};

/* Extracts a cell from the appropriate layer (or build one, 
 * if we are dealing with a composite). Returns it. */
void get_decoded_cell(int x, int y, DecodedCell *dec) {
   if (lyr) decode_cell(&lyr->cells[x][y], dec);
   else {
      Cell c = document_calc_effective_cell(doc, x, y);
      decode_cell(&c, dec);
   }
}

void output_text(void) {
   VirtualLayer *vl;

   if (lyr) vl = vlayer_create_from_layer(lyr);
   else     vl = vlayer_create_from_composite(doc);

   if (!vl) {
      fprintf(stderr, "Error creating virtual layer. Is the document empty?\n");
      exit(2);
   }

   if (clear_screen) fputs("\x1B[2J\x1B[H", f);
   if (!export_vlayer_to_ansi(vl, use_color, !suppress_newlines, false, f)) {
      fprintf(stderr, "Error exporting virtual layer to ansi.\n");
      exit(2);
   }

   vlayer_destroy(vl);
}

#define COL_BLACK 0
#define COL_RED 1
#define COL_GREEN 2
#define COL_YELLOW 3
#define COL_BLUE 4
#define COL_MAGENTA 5
#define COL_CYAN 6
#define COL_WHITE 7

static char *hex_codes[16] = {
     "000000", "990000", "009900", "999900",
     "000099", "990099", "009999", "999999",
     "404040", "ff0000", "00ff00", "ffff00",
     "0000ff", "ff00ff", "00ffff", "ffffff" };

void output_html(void) {
   int x, y;
   struct html_output_state_t cur_state;
   DecodedCell dec;

   fputs("<html><body style=\"color:#ffffff;background-color:#000000;\">"
                                                                "<pre>", f);

   for (y = 0; y < height; y++) {
      for (x = 0; x < width; x++) {
         get_decoded_cell(x, y, &dec);

	 /* first set up the blink tag */
	 if (use_color && dec.blink != cur_state.blink_tag_open) {
	    fprintf(f, "<%sblink>", dec.blink ? "" : "/");
	    cur_state.blink_tag_open = dec.blink;
	 }

         /* set up color */
 	 if (use_color && (dec.fg != cur_state.color ||
                            dec.bg != cur_state.bgcolor)) {
 	    if (cur_state.font_tag_open) fputs("</span>", f);
 	    fprintf(f, "<span style=\"color:#%s;background-color:#%s;\">",
                     hex_codes[dec.fg], hex_codes[dec.bg]);
  	    cur_state.color = dec.fg;
 	    cur_state.bgcolor = dec.bg;
  	    cur_state.font_tag_open = true;
	 }
	 
	 /* now render the character */
         if (dec.ch >= 0 && dec.ch <= 32) fputs(" ", f);
	 else if (dec.ch == '&')          fputs("&amp;", f);
         else if (dec.ch == '<')          fputs("&lt;", f);
	 else if (dec.ch == '>')          fputs("&gt;", f);
         else                             fputc(dec.ch, f);
      }
      fputs("\n", f);
   }

   /* close tags as needed and call it a day */
   if (cur_state.font_tag_open) fputs("</span>", f);
   if (cur_state.blink_tag_open) fputs("</blink>", f);
   fputs("</pre></body></html>", f);
}

void output_comment(void) {
   printf ("%s\n", doc->metainfo);
}

void output_layercount(void) {
   printf ("%d\n", doc->layer_count);
}

#define MYSYNTAX \
   "Syntax: aecat [-b] [-c] [-{n|N}] [{-p | -l <layer_num>}]\n" \
   "               [-f <format>] [-o <output_file>] inputfile\n" \
   "\n" \
   "   -f : specifies output format - can be text, html or comment\n" \
   "        The 'comment' format extracts document metadata.\n" \
   "   -c : prepend a 'clear screen' escape sequence (only valid when\n" \
   "        outputting text)\n" \
   "   -o : writes output to specified file rather than stdout\n" \
   "   -b : disables output of color (only characters will be\n" \
   "        printed).\n" \
   "   -l : specifies which layer of the document is to be used.\n" \
   "        (must be an index, not a layer name). By default,\n" \
   "        layer 0 will be used.\n" \
   "   -L : displays how many layers the document has.\n" \
   "   -p : exports a composite, that is, overlays all visible layers,\n" \
   "        paying attention to layer transparency, etc. The size of the\n" \
   "        composite will be the size of the first layer\n" \
   "   -n : suppress output of newlines\n" \
   "   -h : prints this help text\n"

void rtfm() {  /* rtfm = Read the FAQ and Manual, of course :-) */
   fprintf(stderr, MYSYNTAX);
   exit(1);
}

#define FMT_TEXT 1
#define FMT_HTML 2
#define FMT_COMMENT 3
#define FMT_LAYERS 4

int main(int argc, char **argv) {
   static char optstring[] = "o:chl:Lbpf:n";
   char *inputfile;
   int ch;
   
   int out_fmt = 0; /* In what format to output. */

   while (0 < (ch = getopt(argc, argv, optstring)) ) {
      switch (ch) {
         case 'c': clear_screen = true; break;
         case 'o': outputfile = strdup(optarg); break;
         case 'l': layer_number = atoi(optarg); break;
         case 'L': out_fmt = FMT_LAYERS; break;
         case 'p': layer_number = -1; break;
         case 'b': use_color = false; break;
	 case 'n': suppress_newlines = true; break;
         case 'f':
            if (strcmp(optarg, "text") == 0)
               out_fmt = FMT_TEXT;
            else if (strcmp(optarg, "html") == 0)
               out_fmt = FMT_HTML;
            else if (strcmp(optarg, "comment") == 0)
               out_fmt = FMT_COMMENT;
            else {
               fprintf(stderr, "Invalid format \"%s\"\n", optarg);
               exit(1);
            }
            break;
         default : rtfm();
      }
   }

   if (!out_fmt)
      out_fmt = FMT_TEXT;
   
   if (out_fmt != FMT_TEXT && clear_screen) 
      fprintf(stderr, "Warning: -c option only valid when outputting text\n");

   if (out_fmt != FMT_TEXT && suppress_newlines)
      fprintf(stderr, "Warning: -n option only valid when "
                      "outputting text\n");
   
   if (optind >= argc) rtfm();
   inputfile = strdup(argv[optind]);

   if (! (doc = document_load_from(inputfile)) ) {
      fprintf(stderr, "Error loading document from %s (bad format?).\n", 
                                                        inputfile);
      fprintf(stderr, "Error description:\n   %s\n", aeff_get_error());
      exit(1);
   }

   if (layer_number >= doc->layer_count) {
      if (layer_number)
         fprintf(stderr, "Document %s does not have the specified layer: %d\n",
                inputfile, layer_number);
      else
         fprintf(stderr, "Document %s has no layers.\n", inputfile);
      exit(1);
   }

   width = doc->layers[0]->width;
   height = doc->layers[0]->height;
   lyr = layer_number >= 0 ? doc->layers[layer_number] : NULL;
   
   if (outputfile && !(f = fopen(outputfile, "w"))) {
      fprintf(stderr, "Error opening %s for writing.\n", outputfile);
      exit(1);
   }
   else f = stdout; /* print to standard output if no output file specified */

   switch(out_fmt) {
      case FMT_TEXT: output_text(); break;
      case FMT_HTML: output_html(); break;
      case FMT_COMMENT: output_comment(); break;
      case FMT_LAYERS: output_layercount(); break;
   }

   if (f != stdout) fclose(f);
   return 0;
}
