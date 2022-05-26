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

#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

char *dstrset(char **ptr, const char *new_value) {
   if (*ptr) free(*ptr);
   return (*ptr = new_value ? sstrdup(new_value) : 0);
}

void *zalloc(int bytes) {
   void *buf;

   if (bytes <= 0) return 0;

   if (!(buf = malloc(bytes))) {
      fprintf(stderr, "*** FATAL ERROR ***\n"
                      "zalloc failed to allocate memory.\n"
                      "Request size: %d bytes.\n\nSorry.\n", bytes);
      abort();
   }

   memset(buf, 0, bytes);
   return buf;
}

char* sstrdup(const char *s) {
   char *r;
   if (!s) return 0;

   if ( !(r = strdup(s)) ) {
      fprintf(stderr, "*** FATAL ERROR ***\n"
                      "sstrdup failed to allocate memory.\n");
      abort();
   }
   return r;
}

void *srealloc(void *buf, int newsize) {
   if (!buf) return zalloc(newsize);
   if (!(buf = realloc(buf, newsize))) {
      fprintf(stderr, "*** FATAL ERROR ***\n"
                      "srealloc failed to reallocate memory.\n"
                      "Request size: %d bytes.\n\nSorry.\n", newsize);
      abort();
   }
   return buf;
}

int minimum(int a, int b) { return (a < b) ? a : b; }
int maximum(int a, int b) { return (a > b) ? a : b; }
double minimum_d(double a, double b) { return (a < b) ? a : b; }
double maximum_d(double a, double b) { return (a > b) ? a : b; }


int printable_char(int ch) {
   if (ch < 0 || ch > 255) return 0;     /* not in char range */
   if (ch <= 31 || ch == 127) return 0;  /* control character */
   return 1;
}

char *str_fit_ex(const char *orig, int size, int padch) {
   char *s; char *p;
   if (size <= 0) return 0;
   
   p = s = zalloc(size + 1);
   while (p - s < size)
      *(p++) = (*orig) ? *(orig++) : padch;
   
   *p = 0;
   return s;
}

char *str_fit(const char *orig, int size) {
   return str_fit_ex(orig, size, ' ');
}

float timeval_dif(const struct timeval* a, const struct timeval* b) {
   long to_remove;
   float af, bf;
   if (a->tv_sec > b->tv_sec) to_remove = b->tv_sec;
   else to_remove = a->tv_sec;
   
   af = (float)(a->tv_sec - to_remove) + a->tv_usec / 1000000.0f;
   bf = (float)(b->tv_sec - to_remove) + b->tv_usec / 1000000.0f;
   return af - bf;
}

void fsavestr(const char *str, FILE *f) {
   int len = strlen(str);

   fwrite(&len, sizeof(int), 1, f);
   while (*str) fputc(*(str++), f);
}

char *floadstr(FILE *f) {
   char *str, *p; int len;
   
   fread(&len, sizeof(int), 1, f);

   p = str = zalloc(len + 1);
   while (len--) *(p++) = fgetc(f);

   return str;
}

void interval_intersect(int a0, int a1, int b0, int b1, int *c0, int *c1) {
   *c0 = maximum(a0, b0);
   *c1 = minimum(a1, b1);
}

void interval_intersect_d(double a0, double a1, double b0, double b1, 
                          double *c0, double *c1) {
   *c0 = maximum_d(a0, b0);
   *c1 = minimum_d(a1, b1);
}

void sort_two(int v0, int v1, int *r0, int *r1) {
  if (v0 <= v1) {
     if (r0) *r0 = v0;
     if (r1) *r1 = v1;
  }
  else {
     if (r0) *r0 = v1;
     if (r1) *r1 = v0;
  }
}

char *dsprintf(const char *fmt, ...) {
   /* this function was adapted from the example provided in 
    * Linux man page for printf(3) */
   int n, size = 100;
   char *p;
   va_list ap;
   
   p = zalloc(size);
   
   while (1) {
      /* Try to print in the allocated space. */
      va_start(ap, fmt); n = vsnprintf (p, size, fmt, ap); va_end(ap);
      
      /* If that worked, return the string. */
      if (n > -1 && n < size)  return p;
      
      /* Else try again with more space. */
      if (n > -1)    /* glibc 2.1 */
         size = n+1; /* precisely what is needed */
      else           /* glibc 2.0 */
          size *= 2;  /* twice the old size */
      
      p = srealloc (p, size);
   }
}

void sfree(void *ptr) {
   if (ptr) free(ptr);
}

void zfree(char **str) {
   if (*str) {
      free(*str);
      *str = 0;
   }
}

char *freadline(FILE *f) {
   return freadline_ex((void*)f, (int(*)(void*)) fgetc);
}

char *freadline_ex(void *fh, int (*readch)(void*)) {
   int size = 64;
   char *buf;
   int i;
   char ch;
   
   /* read in the first character */
   ch = readch(fh);
   if (ch < 0) return NULL;
   
   /* allocate buffer */
   buf = zalloc(size + 1);
   i = 0;

   /* store and read over and over until \n or EOF */
   while (ch != '\n' && ch != -1) {
      buf[i++] = ch;
      if (i >= size) buf = srealloc(buf, 1 + (size += size));

      ch = readch(fh);
   }

   buf[i] = 0; /* we know we can write on buf[i] because we know i <= size,
                * (remember that buf has capacity is size + 1) */
   return buf;
}

char *strtrim(const char *s) {
   char *t, *u;
   while (*s == ' ' || *s == '\t' || *s == '\n') s++;
   t = sstrdup(s);
   u = t + strlen(t) - 1;
   while (u >= t && (*u == ' ' || *u == '\t' || *u == '\n')) u--;
   u[1] = 0;
   return t;
}

char *dstrcat(const char *s, const char *t) {
   char *u = zalloc(1 + (s ? strlen(s) : 0) + (t ? strlen(t) : 0));
   if (s) strcpy(u, s);
   if (t) strcat(u, t);
   return u; /* zalloc zeroes out the buffer, so even if s == t == NULL,
              * u will be a single '\0', which is a correct result.  */
}

void chr2hex(int c, char* ret) {
   static char hs[] = "0123456789abcdef";

   if (c <= 0) c = 0; 
   if (c >= 256) c = 255;

   ret[0] = hs[c / 16];
   ret[1] = hs[c % 16];
   ret[2] = 0;
}

static int hex_digit_val(char c) {
   if (c >= '0' && c <= '9') return c - '0';
   else if (c >= 'a' && c <= 'f') return c - 'a' + 10;
   else if (c >= 'A' && c <= 'F') return c - 'A' + 10;
   else return 0;
}

int hex2chr(const char *hex) {
   return hex_digit_val(hex[0]) * 16 + hex_digit_val(hex[1]);
}

