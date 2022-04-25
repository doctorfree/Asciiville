/*! \file
 * \noop Copyright 2006-2016 Christian Stigen Larsen
 * \noop Copyright 2020 Christoph Raitzig
 *
 * \brief Functions that work directly with images.
 *
 * Call decompress_jpeg() or decompress_png() to read an input image from a stream and print it.
 * Which is called does not matter in regards to functionality, if decompress_jpeg() fails decompress_png() is called and vice versa.
 * For efficiency call the function that is most likely to work.
 *
 * All other functions in this file are called by decompress_jpeg() or decompress_png() or one of the functions they call.
 *
 * \author Christian Stigen Larsen
 * \author Christoph Raitzig
 * \copyright Distributed under the GNU General Public License (GPL) v2.
 */

#ifndef INC_JP2A_IMAGE_H
#define INC_JP2A_IMAGE_H

#include "config.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "jpeglib.h"
#include "png.h"
#include <setjmp.h>

#include "html.h"

/*! \struct Image_
 * \brief Holds a decompressed image.
 *
 * #pixel, #red, #green, #blue, #alpha and #yadds are arrays of size #width * #height.
 */
typedef struct Image_ {
	int width; //!< width
	int height; //!< height
	float *pixel; //!< luminosities (i.e. gray values)
	float *red; //!< red part
	float *green; //!< green part
	float *blue; //!< blue part
	float *alpha; //!< opacities
	int *yadds; //!< how many scanlines were used for a pixel, used by normalize()
	float resize_y; //!< Factor by which the output image is resized from the input image in the y dimension (height). For example for an output height of 40 and input height of 80 this equals approximately 0.5.
	float resize_x; //!< Factor by which the output image is resized from the input image in the x dimension (width). For example for an output width of 50 and input height of 100 this equals approximately 2.0.
	int *lookup_resx; //!< where to start and end using pixels of the input image to calculate a pixel of the output image along the x axis
} Image;
/*!
 * \typedef Image
 * \brief See #Image_
 */

/*! \struct my_jpeg_error_mgr
 * \brief This struct is used for custom error handling with libjpeg.
 */
typedef struct my_jpeg_error_mgr {
	struct jpeg_error_mgr pub; //!< libjpeg's error manager
	jmp_buf setjmp_buffer; //!< where to jump to if an error occurs
} my_jpeg_error_mgr;
/*!
 * \typedef my_jpeg_error_mgr
 * \brief See #my_jpeg_error_mgr
 */

/*!
 * \brief Pointer to a #my_jpeg_error_mgr.
 */
typedef struct my_jpeg_error_mgr *my_jpeg_error_ptr;

/*! \struct error_collector
 * \brief Contains and collects errors that occur while decompressing an image.
 *
 * With this struct decompress_jpeg() and decompress_png() can be called recursively since #jpeg_status and #png_status can be used to determine whether a previous call failed or not.
 */
typedef struct error_collector {
	my_jpeg_error_mgr *jpeg_error; //!< contains information about a JPEG decompression error
	char *png_error_msg; //!< error message for a PNG decompression error
	int jpeg_status; //!< true if an error occurred during JPEG decompression, false otherwise
	int png_status; //!< true if an error occurred during PNG decompression, false otherwise
} error_collector;
/*!
 * \typedef error_collector
 * \brief See #error_collector
 */

/*!
 * \brief Prints the top or bottom of a border around an image.
 *
 * \param width width of the output image
 */
void print_border(const int width);

/*!
 * \brief Prints an image.
 *
 * Calls print_image_colors() or print_image_no_colors().
 *
 * \param image the output image
 * \param f the stream to print to
 */
void print_image(Image *image, FILE *f);

/*!
 * \brief Prints an image with color.
 *
 * \param image the output image
 * \param chars the character palette
 * \param f the stream to print to
 */
void print_image_colors(const Image* const image, const int chars, FILE *f);

/*!
 * \brief Prints an image without color.
 *
 * \param image the output image
 * \param chars the character palette
 * \param f the stream to print to
 */
void print_image_no_colors(const Image* const image, const int chars, FILE *f);

/*!
 * \brief Clears the image (i.e. sets all pixels to black, alpha to max)
 *
 * \param i the image
 */
void clear(Image* i);

/*!
 * \brief Normalizes the image.
 *
 * After decompressing the image into RAM the G/RGB values must not be between 0 and 1. The functions normalizes them so that they are.
 *
 * \param i the image
 */
void normalize(Image* i);

/*!
 * \brief Prints a progress bar.
 *
 * \param progress The progress. Between 0 and 1 where 0 is 0% and 1 is 100%
 */
void print_progress(float progress);

/*!
 * \brief Prints some information about the image and how it will be printed.
 *
 * \param jpg contains information about the JPEG image
 */
void print_info_jpeg(const struct jpeg_decompress_struct* jpg);

/*!
 * \brief Prints some information about the image and how it will be printed.
 *
 * \param png_ptr necessary for calling libpng functions
 * \param info_ptr contains information about the PNG image
 */
void print_info_png(const png_structp png_ptr, const png_infop info_ptr);

/*!
 * \brief Processes a scanline of a JPEG image.
 *
 * \param jpg contains information about the JPEG image
 * \param scanline the scanline
 * \param i the output image
 */
void process_scanline_jpeg(const struct jpeg_decompress_struct *jpg,
	const JSAMPLE* scanline, Image* i);

/*!
 * \brief Processes a scanline of a PNG image.
 *
 * Supports a bit-depth of 8 and G, GA, RGB and RGBA.
 *
 * \param row scanline (i.e. the row of pixels)
 * \param current_y the current height
 * \param color_components the number of color components (e.g. 4 for RGBA)
 * \param i the output image
 */
void process_scanline_png(const png_bytep row, const int current_y, const int color_components, Image* i);

/*!
 * \brief Frees allocated memory of an image.
 *
 * \param i the image
 */
void free_image(Image* i);

/*!
 * \brief Allocates memory for holding the pixels etc. Sets the width and height.
 *
 * \param i the image
 */
void malloc_image(Image* i);

/*!
 * \brief Sets internal values necessary for processing scanlines.
 *
 * \param i the struct to hold the output image
 * \param src_width width of the source image
 * \param src_height height of the source image
 */
void init_image(Image *i, int src_width, int src_height);

/*!
 * \brief Decompresses and prints an image.
 *
 * Calls decompress_png() if the image is not a JPEG image.
 * Instead prints errors if there was an error when decompressing this image as JPEG previously.
 *
 * \param fin input stream, has to be seekable
 * \param fout stream to print the image to
 * \param errors contains previous errors and is used to save errors
 */
void decompress_jpeg(FILE *fin, FILE *fout, error_collector *errors);

/*!
 * \brief Callback for errors while decompressing a JPEG image.
 *
 * \param jerr contains information about the error
 */
void jpeg_error_exit(j_common_ptr jerr);

/*!
 * \brief Decompresses and prints an image.
 *
 * Calls decompress_jpeg() if the image is not a PNG image.
 * Instead prints errors if there was an error when decompressing this image as PNG previously.
 *
 * \param fin input stream, has to be seekable
 * \param fout stream to print the image to
 * \param errors contains previous errors and is used to save errors
 */
void decompress_png(FILE *fin, FILE *fout, error_collector *errors);

/*!
 * \brief Prints errors.
 *
 * Checks what errors have occurred and prints their error messages.
 *
 * \param errors the collected errors
 */
void print_errors(error_collector *errors);

#endif
