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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define AEFLIC_RTFM \
"Syntax: aemakeflic [options] input.ae [outputfile.ext]\n" \
"\n" \
"   Produces an animation file from an aewan document. Each layer in the\n" \
"   document will be used as one frame of the animation. See the man page\n" \
"   for more information.\n" \
"\n" \
"   Options:\n" \
"\n" \
"      -f {less|sh}     movie format ('less movie' or shell-script)\n" \
"                       The default is less.\n" \
"      -N               suppress initial screen with instructions for\n" \
"                       the given format\n" \
"      -b               Disable colors (use text only)\n" \
"      -d {delay}       Delay in milliseconds between frames\n" \
"\n" \

#define DEFAULT_DELAY 100

/* settings */
FILE *outf;  /* the output file */
bool add_instructions = true;
Document *doc;  /* the document we are converting */
int width, height; /* animation dimensions */
bool use_color = true;
int interframe_delay = DEFAULT_DELAY; 
                      /* delay between frames (shellscript only) 
                       * given in milliseconds */

void rtfm() {
   fputs(AEFLIC_RTFM,stderr);
   exit(1);
}

void output_lessmovie_instructions() {
   const int instruction_lines = 18;  /* VERY IMPORTANT: keep this value
                                       * in sync with the message below.
                                       * It indicates how many lines it 
                                       * occupies! */
   const int instruction_cols = 70;   /* this can be overestimated but
                                       * not underestimated */
   int lines_left;
   int i;

   fprintf(outf,
         "----------------------------------------[ TOP ]----------\n"
         "AEWAN LESS-MOVIE\n\n"
         "This is an ascii animation intended to be viewed in the 'less'\n"
         "pager. You must set your terminal dimensions to the following:\n\n"
         "                %d columns x %d rows\n\n"
         "Then run 'less -c -r <filename>.ae' and hold down the PageDown key\n"
         "to see the animation play.\n\n"
         "You can tell if the terminal dimensions are correct by looking at\n"
         "the top and bottom dotted lines: the one marked '[ TOP ]' should\n"
         "appear at the topmost line of the terminal and the dotted line\n"
         "marked [ BOTTOM ] should appear at the bottommost. As you page-down\n"
         "a few times, they should REMAIN FIXED, not move around.\n\n"
         "This animation was generated with AEWAN, an open-source ascii and\n"
         "animation editor. Visit http://aewan.sourceforge.net for more info.\n"
      , width, height+1);

   lines_left = height - instruction_lines - 1;
   if (lines_left < 1 || width < instruction_cols) {
      fprintf(stderr, "Error: can't add instructions because the document\n"
                      "dimensions are too small. Height must be at least\n"
                      "%d and width must be at least %d.\n", 
                      instruction_lines, instruction_cols);
      exit(2);
   }

   while (lines_left > 1) {
      fputs("\n", outf);
      lines_left--;
   }

   fputs("----------------------------------------[ BOTTOM ]-------\n",outf);

   for (i = 4; i >= 1; i--) {
      fputs("----------------------------------------[ TOP ]----------\n",outf);
      lines_left = height - 2;

      fprintf(outf, "(movie starts in %d frame[s])...\n", i);
      lines_left--;

      while (lines_left-- > 0) fputs("\n", outf);
      fputs("----------------------------------------[ BOTTOM ]-------\n",outf);
   }
}

void output_shellscript_instructions() {
   fprintf(outf,
"echo 'AEWAN SHELLSCRIPT ANIMATION'\n"
"echo 'This is an aewan animation shellscript. It will now play an'\n"
"echo 'animation on your terminal. Please make sure the terminal'\n"
"echo 'has at least the following dimensions:'\n"
"echo '     %d lines, %d columns'\n"
"echo\n"
"echo 'This animation was generated with AEWAN, an open-source ascii art and'\n"
"echo 'and animation editor. For more info, see http://aewan.sourceforge.net'\n"
"echo\n"
"echo 'Press ENTER to start the animation'\n"
"read\n"
"echo -ne '\\e[2J\\e[H'\n"
      , height, width);
}

void output_lessmovie() {
   int i;

   if (add_instructions) output_lessmovie_instructions();

   for (i = 0; i < doc->layer_count; i++) {
      VirtualLayer *vl = vlayer_create_from_layer(doc->layers[i]);
      export_vlayer_to_ansi(vl, use_color, true, false, outf);
      vlayer_destroy(vl);
   }
}

void output_shellscript() {
   int i;

   fputs("#!/bin/bash\n"
         "# This is an aewan animation shellscript. Upon execution,\n"
         "# it will play an animation on the terminal.\n"
         "# Aewan is an open-source ascii art and animation editor.\n"
         "#       http://aewan.sourceforge.net\n"
         "\n"
         "echo -ne '\\e[2J\\e[H'\n", outf);

   if (add_instructions) output_shellscript_instructions();

   for (i = 0; i < doc->layer_count; i++) {
      VirtualLayer *vl = vlayer_create_from_layer(doc->layers[i]);
      fputs("echo -ne '\\e[H'\n", outf);
      fputs("cat <<QWPOEIURASDKJFAS\n", outf);
      export_vlayer_to_ansi(vl, use_color, true, true, outf);
      fputs("\nQWPOEIURASDKJFAS\n", outf);
                /* FIXME: heinous bug: if the layer drawing mentions
                 * the word QWPOEIURASDKJFAS at the beginning of a line,
                 * bash will get confused. */
      fprintf(outf, "sleep %.3f\n", interframe_delay / 1000.0f);
      vlayer_destroy(vl);
   }
}

#define FMT_LESS 0
#define FMT_SH   1
int main(int argc, char **argv) {
   const char optstr[] = "f:Nbd";
   int ch;
   int format = FMT_LESS;
   const char *inputfile, *outputfile;

   while (0 <= (ch = getopt(argc, argv, optstr))) {
      switch (ch) {
         case 'f':
            if (!strcasecmp(optarg, "less")) format = FMT_LESS;
            else if (!strcasecmp(optarg, "sh")) format = FMT_SH;
            else {
               fputs("Unrecognized animation format!\n", stderr);
               return 1;
            }
            break;
         case 'N': add_instructions = false; break;
         case 'b': use_color = false; break;
         case 'd': interframe_delay = atoi(optarg); break;
         default: rtfm();
      }
   }

   if (optind >= argc) rtfm();
   inputfile = argv[optind];
   outputfile = (optind + 1 >= argc) ? NULL : argv[optind+1];

   if (outputfile) {
      if (!(outf = fopen(outputfile, "w"))) {
         perror(outputfile);
         fputs("Can't write outputfile.\n", stderr);
         return 2;
      }
   }
   else outf = stdout; /* use stdout for output if no output file specified */
   
   /* load document */
   if (!(doc = document_load_from(inputfile))) {
      fprintf(stderr, "Error loading document from %s.\n", inputfile);
      fprintf(stderr, "Error description: %s\n", aeff_get_error());
      return 2;
   }

   if (0 >= doc->layer_count) {
      fprintf(stderr, "Document is empty. Can't create animation.\n");
      return 2;
   }

   width = doc->layers[0]->width;
   height = doc->layers[0]->height;

   switch (format) {
      case FMT_LESS: output_lessmovie(); break;
      case FMT_SH:   output_shellscript(); break;
   }

   if (outf != stdout) fclose(outf);
   return 0;
}

