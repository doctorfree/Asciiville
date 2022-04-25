/*! \file
 * \noop Copyright 2020 Christoph Raitzig
 *
 * \brief Functions for downloading images (with cURL).
 *
 * \author Christoph Raitzig
 * \copyright Distributed under the GNU General Public License (GPL) v2.
 */

#ifndef INC_JP2A_CURL_H
#define INC_JP2A_CURL_H

#ifdef FEAT_CURL

/*!
 * \brief Checks whether it is a supported URL.
 *
 * \param s the URL
 * \return 1 if it is a supported URL
 */
int is_url(const char* s);

/*!
 * \brief Downloads an image.
 *
 * \param url URL of the image
 * \param debug true if cURL should print debug information
 * \return read-only file-descriptor that must be closed.
 */
int curl_download(const char* url, const int debug);

#ifdef WIN32

/*!
 * \brief A custom write function.
 *
 * Required for Windows. Does the same as fwrite().
 *
 * \param buffer buffer to read data from
 * \param size the size of one item
 * \param nmemb the number of items
 * \param userp the file stream to write to
 * \return the number of items written
 */
size_t passthru_write(void *buffer, size_t size, size_t nmemb, void *userp);

/*!
 * \brief Actually download an image.
 */
void curl_download_child(void*);
#else

/*!
 * \brief Actually download an image.
 */
void curl_download_child();
#endif

#endif

#endif
