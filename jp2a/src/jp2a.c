/*
 * Copyright 2006-2016 Christian Stigen Larsen
 * Copyright 2020 Christoph Raitzig
 * Copyright 2022 Ronald Record
 * Distributed under the GNU General Public License (GPL) v2.
 */

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include "jp2a.h"
#include "options.h"
#include "image.h"
#include "curl.h"

#ifdef WIN32
#include <windows.h>
#include <io.h>
#define close _close
#include <fcntl.h>
#endif

int main(int argc, char** argv) {
	int store_width, store_height, store_autow, store_autoh;
	FILE *fout = stdout;
	// FILEs from downloads and pipes are not seekable.
	// Solution: Copy in a buffer and use fmemopen.
	char *buffer = NULL;
	size_t buffer_size = 0;

	error_collector errors;
	int retval = 0;
#ifdef FEAT_CURL
	FILE *fr;
	int fd;
#endif
	FILE *fp;
	int n;

#if ! ASCII
	setlocale(LC_ALL, "");
#endif

	parse_options(argc, argv);

	store_width = width;
	store_height = height;
	store_autow = auto_width;
	store_autoh = auto_height;

	if ( strcmp(fileout, "-") ) {
		if ( (fout = fopen(fileout, "wb")) == NULL ) {
			fprintf(stderr, "Could not open '%s' for writing.\n", fileout);
			free(html_title);
			return 1;
		}
	}

	if ( html && !html_rawoutput ) print_html_document_start(html_fontsize, fout);
	else if ( xhtml && !html_rawoutput ) print_xhtml_document_start(html_fontsize, fout);
	free(html_title);

	for ( n=1; n<argc; ++n ) {

		width = store_width;
		height = store_height;
		auto_width = store_autow;
		auto_height = store_autoh;

		// skip options
		if ( argv[n][0]=='-' && argv[n][1] )
			continue;

		errors.jpeg_status = 0;
		errors.png_status = 0;

		// read from stdin
		if ( argv[n][0]=='-' && !argv[n][1] ) {
			#ifdef _WIN32
			// Good news, everyone!
			_setmode( _fileno( stdin ), _O_BINARY );
			#endif

			size_t actual_size = 0;
			if ( read_into_buffer(stdin, &buffer, &buffer_size, &actual_size) ) {
#ifndef _WIN32
				FILE *buffer_f = fmemopen(buffer, actual_size, "rb");
#else
				HANDLE handle = fmemopen(buffer, actual_size, "rb");
				FILE *buffer_f = NULL;
				if ( handle != NULL )
					buffer_f = _fdopen(_open_osfhandle(handle, _O_RDONLY), "rb");
#endif
				if ( buffer_f != NULL ) {
					decompress_jpeg(buffer_f, fout, &errors);
					fclose(buffer_f);
				}
#ifdef _WIN32
				CloseHandle(handle);
#endif
			}

			if ( errors.jpeg_status && errors.png_status )
				retval = 1;
			continue;
		}

		#ifdef FEAT_CURL
		if ( is_url(argv[n]) ) {

			if ( verbose )
				fprintf(stderr, "URL: %s\n", argv[n]);

			fd = curl_download(argv[n], debug);

			if ( (fr = fdopen(fd, "rb")) == NULL ) {
				fputs("Could not fdopen read pipe\n", stderr);
				return 1;
			}

			size_t actual_size = 0;
			if ( read_into_buffer(fr, &buffer, &buffer_size, &actual_size) ) {
#ifndef _WIN32
				FILE *buffer_f = fmemopen(buffer, actual_size, "rb");
#else
				HANDLE handle = fmemopen(buffer, actual_size, "rb");
				FILE *buffer_f = NULL;
				if ( handle != NULL )
					buffer_f = _fdopen(_open_osfhandle(handle, _O_RDONLY), "rb");
#endif
				if ( buffer_f != NULL ) {
					int urllen = strlen(argv[n]);
					if ( urllen > 4 && strcmp(".png", argv[n] + (urllen - 4)) == 0 )
						decompress_png(buffer_f, fout, &errors);
					else
						decompress_jpeg(buffer_f, fout, &errors);
					fclose(buffer_f);
				}
#ifdef _WIN32
				CloseHandle(handle);
#endif
			}
			fclose(fr);
			close(fd);
			
			if ( errors.jpeg_status && errors.png_status )
				retval = 1;
			continue;
		}
		#endif

		// read files
		if ( (fp = fopen(argv[n], "rb")) != NULL ) {
			if ( verbose )
				fprintf(stderr, "File: %s\n", argv[n]);

			int namelen = strlen(argv[n]);
			if ( namelen > 4 && strcmp(".png", argv[n] + (namelen - 4)) == 0 )
				decompress_png(fp, fout, &errors);
			else
				decompress_jpeg(fp, fout, &errors);
			fclose(fp);

			if ( errors.jpeg_status && errors.png_status )
				retval = 1;
			continue;

		} else {
			fprintf(stderr, "Can't open %s\n", argv[n]);
			return 1;
		}
	}

	if ( html && !html_rawoutput ) print_html_document_end(fout);
	else if ( xhtml && !html_rawoutput ) print_xhtml_document_end(fout);

	if ( buffer_size != 0 ) {
		free(buffer);
	}
	if ( fout != stdout )
		fclose(fout);

	return retval;
}

int read_into_buffer(FILE *fp, char **buffer, size_t *buffer_size, size_t *actual_size) {
#define BUFFER_ALLOC_INCREMENTS 16384
	*actual_size = 0;
	if ( *buffer_size == 0 ) {
		*buffer_size = BUFFER_ALLOC_INCREMENTS;
		*buffer = malloc(*buffer_size);
		if ( *buffer == NULL ) {
			fprintf(stderr, "Not enough memory. Skipping an image.\n");
			*buffer_size = 0;
			return 0;
		}
	}
	char *current = *buffer;
	while ( !feof(fp) ) {
		*actual_size += fread(current, 1, BUFFER_ALLOC_INCREMENTS, fp);
		if ( *actual_size == *buffer_size ) {
			*buffer_size += BUFFER_ALLOC_INCREMENTS;
			if ( debug )
				fprintf(stdout, "Reallocating to: %ld\n", *buffer_size);
			current = realloc(*buffer, *buffer_size);
			if ( current == NULL ) {
				fprintf(stderr, "Not enough memory. Skipping an image.\n");
				*buffer_size -= BUFFER_ALLOC_INCREMENTS;
				return 0;
			}
			*buffer = current;
		}
		current = *buffer + *actual_size;
	}
	if ( debug )
		fprintf(stderr, "Size: %ld\n", *actual_size);
	return 1;
}

#ifdef _WIN32
HANDLE fmemopen(void *buf, size_t size, const char *mode) {
	TCHAR temp_path[1024];
	DWORD temp_path_len = GetTempPathA(1024, temp_path);
	if ( temp_path_len == 0 || temp_path_len > 1024 )
		return NULL;
	TCHAR temp_file_name[MAX_PATH];
	UINT unique = GetTempFileNameA(temp_path, "jp2a", 0, temp_file_name);
	if ( unique == 0 )
		return NULL;
	HANDLE outputf = CreateFileA(temp_file_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD bytes_written = 0;
	BOOL written = WriteFile(outputf, buf, size, &bytes_written, NULL);
	if ( written == FALSE )
		return NULL;
	DWORD retval = SetFilePointer(outputf, 0, NULL, FILE_BEGIN);
	if ( retval == INVALID_SET_FILE_POINTER )
		return NULL;
	return outputf;
}
#endif
