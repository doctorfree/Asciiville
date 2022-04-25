/*! \file
 * \noop Copyright 2006-2016 Christian Stigen Larsen
 * \noop Copyright 2020 Christoph Raitzig
 * \noop Copyright 2022 Ronald Record
 *
 * \brief The main function and a helper function.
 *
 * \author Christian Stigen Larsen
 * \author Christoph Raitzig
 * \copyright Distributed under the GNU General Public License (GPL) v2.
 */

#ifndef INC_JP2A_H
#define INC_JP2A_H

#ifdef _WIN32
#include <windows.h>
#endif

/*!
 * \brief The main function.
 *
 * \param argc argument count
 * \param argv the arguments
 */
int main(int argc, char** argv);

/*!
 * \brief Reads from a stream into a buffer.
 *
 * This function is used to make a seekable stream from a non-seekable stream:
 * Read the contents of the non-seekable stream into a buffer (with this function) and open a stream to this buffer (with fmemopen()).
 * The buffer is (re)allocated as needed.
 *
 * \param fp stream to read into buffer
 * \param buffer the buffer to read into
 * \param buffer_size the allocated size of the buffer
 * \param actual_size the number of bytes read into the buffer
 * \return true if sucessful, false otherwise
 */
int read_into_buffer(FILE *fp, char **buffer, size_t *buffer_size, size_t *actual_size);

#ifdef _WIN32

/*!
 * \brief fmemopen alternative for Windows.
 *
 * Windows does not support opening a file stream to part of the RAM.
 * This function creates a temporary file and writes the contents of the buffer to it.
 *
 * \param buf data to write to the file
 * \param size number of bytes to write
 * \param mode ignored, for compatibility with the actual fmemopen()
 * \return the file handle (or NULL if an error occurred)
 */
HANDLE fmemopen(void *buf, size_t size, const char *mode);
#endif

#endif
