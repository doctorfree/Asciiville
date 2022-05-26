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

#ifndef bores_util_h
#define bores_util_h
#include <sys/time.h>
#include <stdbool.h>
#include <stdio.h>

#define FMTCALL(x, f) { char *fmts = dsprintf x; f; zfree(&fmts); }

/* If (*ptr) { free(*ptr), *ptr = NULL; }. Then, makes *ptr = 
 * strdup(new_value) if new_value is not NULL. Returns the value *ptr 
 * after the assignment. */
char *dstrset(char **ptr, const char *new_value);

/* If *str { free(*str); *str = NULL; } */
void zfree(char **str);

/* If ptr, free(ptr) */
void sfree(void *ptr);

/* Does what you expect */
int minimum(int a, int b);

/* Duh. */
int maximum(int a, int b);

/* Does what you expect */
double minimum_d(double a, double b);

/* Duh. */
double maximum_d(double a, double b);

/* Allocates memory like malloc, with the following differences: (1) exits
 * program with a fatal error if memory cannot be allocated; (2) zeroes
 * out the newly allocated buffer before returning. This function returns
 * NULL if and only if bytes <= 0. */
void *zalloc(int bytes);

/* If <buf> is not null, works just like realloc. If <buf> is null,
 * calls zalloc instead of realloc. Returns a pointer to the reallocated
 * (or newly allocated) buffer; NULL if and only if newsize <= 0. */
void *srealloc(void *buf, int newsize);

/* Returns true if the passed character ch is printable */
int printable_char(int ch);

/* Returns a dynamically allocated string whose length is exactly <size>.
 * If strlen(<orig>) <= <size>, returned string is a copy of <orig>
 * padded with padch characters. If strlen(<orig>) > <size>, returned
 * string is a copy of <orig> clipped to <size>.
 *
 * Returns NULL if the fitting cannot be done because <size> is invalid */
char *str_fit_ex(const char *orig, int size, int padch);

/* Convenience function that calls
 * str_fit_ex(orig, size, ' ') */
char *str_fit(const char *orig, int size);

/* Returns difference in seconds between the two passed
 * timevals. This difference will be positive if a > b,
 * negative if a < b, or zero if a and b are the same */
float timeval_dif(const struct timeval* a, const struct timeval* b);

/* Saves the string pointed to by <str> in the file <f>, in a format
 * that can later be read back by floadstr() */
void fsavestr(const char *str, FILE *f);

/* Loads a string from file <f>, as saved by fsavestr(). Returns a dynamically
 * allocated string (which the caller must free) or NULL if there is an
 * error. */
char *floadstr(FILE *f);

/* Calculates the intersection of two integer intervals with inclusive
 * endpoints. Interval [a0..a1], where a0 <= a1, is intersected with
 * interval [b0..b1], where b0 <= b1. The resulting interval [c0..c1] is
 * stored in variables c0 and c1. An empty interval is denoted by
 * c0 > c1. */
void interval_intersect(int a0, int a1, int b0, int b1, int *c0, int *c1);

/* Same as interval_intersect, but with doubles */
void interval_intersect_d(double a0, double a1, double b0, double b1, 
                          double *c0, double *c1);

/* Does what you expect */
int minimum(int a, int b);

/* Duh. */
int maximum(int a, int b);

/* Sorts the two values v0 and v1, returning the result in variables
 * r0 and r1. Return value has the property that *r0 <= *r1 */
void sort_two(int v0, int v1, int *r0, int *r1);

/* Behaves like sprintf, but returns a dynamically allocated string
 * as large as necessary to contain the results. Uses zalloc to allocate
 * memory, so memory allocation does not fail (a fatal error is produced
 * if system has insufficient memory) */
char *dsprintf(const char *fmt, ...);

/* Reads a full line from file f, allocating space as needed to contain it.
 * Returns a dynamically allocated buffer, or NULL if EOF was read before
 * any characters could be read in. The \n will NOT be included in the
 * returned string (unlike fgets()). */
char *freadline(FILE *f);

/* Same as freadline, but in this function you specify a custom function
 * to read characters from your stream, and the file handle is opaque
 * (that is, can be anything you want, not necessarily a FILE*). 
 * When this function needs a character, it will call the readch
 * function passing it the file handle fh, and expect it to return
 * the next character on the stream or -1 if EOF is reached. */
char *freadline_ex(void *fh, int (*readch)(void*));

/* Returns a new string that results from removing leading and trailing
 * spaces from string s */
char *strtrim(const char *s);

/* Like strdup(), but aborts the program with a fatal error if memory
 * cannot be allocated (rather than return NULL as strdup() does). 
 * Also, if s is NULL, returns NULL rather than segfaulting (which is
 * what strdup() does). */
char *sstrdup(const char *s);

/* Returns a dynamically allocated string that results from the concatenation
 * of strings s and t. NULL parameters are NOT an error and will be
 * equivalent to empty strings. */
char *dstrcat(const char *s, const char *t);

/* Returns the hexadecimal representation of the given character
 * as a two-digit hexadecimal number stored in the array ret, which
 * must be of size 3. A trailing \0 is appended. The hex2chr function
 * does the reverse */
void chr2hex(int c, char* ret);
int  hex2chr(const char *hex);

#endif

