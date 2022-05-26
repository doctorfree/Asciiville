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

/* This module handles reading and writing aewan-formatted files.
 * These functions provide an abstration layer for actual I/O,
 * nothing else. The document_save, layer_save, and other functions
 * call the functions herein. 
 *
 * When any functions herein find an error, they set an internal
 * static error description variable in addition to returning an error
 * code. You can query the error description through a call to
 * aeff_get_error()
 *
 * All aeff_read_* functions return bool indicating success or failure,
 * and set the internal error string on the latter case. They also take
 * label parameters, which they use to check whether the next piece
 * of data in the stream has that label (returning an error if it doesn't).
 */

#ifndef _btco_aewan_aeff_h
#define _btco_aewan_aeff_h

#include <stdbool.h>
#include "bores/bores.h"

typedef struct AeFile_ AeFile; /* opaque file type */

/* Open a file for I/O. <filename> is the name of the file to open,
 * <mode> is either 'r' or 'w' and you can probably guess what it means. 
 * Returns the file handle or NULL on error (in which case the internal
 * error string is set appropriately) */
AeFile* aeff_open(const char *filename, char mode);

/* Queries for a description of the last error. CAUTION: this function
 * returns a pointer to an internal buffer that may be deallocated
 * or overwritten at whim. So don't keep the returned pointer, just
 * use it and forget about it, ask for it again later if you need it. 
 *
 * This function returns NULL if no error string is set. */
const char *aeff_get_error(void);

/* Set an artificial error. */
void aeff_set_error(const char *e);

/* Closes a file opened with aeff_open */
void aeff_close(AeFile* f);

/* Reads/Writes a header (headers contain no data, they are just
 * used for consistency checking) */
void aeff_write_header(AeFile *f, const char *header_name);
bool aeff_read_header(AeFile *f, const char *header_name);

/* Reads/Writes a footer (footers contain no data, they are just
 * used for consistency checking) */
void aeff_write_footer(AeFile *f, const char *footer_name);
bool aeff_read_footer(AeFile *f, const char *footer_name);

/* Reads/writes a labeled integer */
void aeff_write_int(AeFile *f, const char *label, int a);
bool aeff_read_int(AeFile *f, const char *label, int *ret);

/* Reads/writes a labeled boolean */
void aeff_write_bool(AeFile *f, const char *label, bool a);
bool aeff_read_bool(AeFile *f, const char *label, bool *ret);

/* Reads/writes a labeled string.
 * On the read function, the returned pointer points to a DYNAMICALLY
 * ALLOCATED block that the caller is responsible for freeing. */
void aeff_write_string(AeFile *f, const char *label, const char *s);
bool aeff_read_string(AeFile *f, const char *label, char **s);

#endif


