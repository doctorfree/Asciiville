/*! \file
 * \noop Copyright 2020 Christoph Raitzig
 * \noop Copyright 2022 Ronald Record
 * 
 * \brief Functions for querying terminal features.
 *
 * \author Christoph Raitzig
 * \copyright Distributed under the GNU General Public License (GPL) v2.
 */

#ifndef INC_JP2A_TERMINAL_H
#define INC_JP2A_TERMINAL_H

/*!
 * \brief Get the terminal dimensions.
 *
 * \param width_,height_ stores the dimensions
 * \param error stores an error (if one occurs)
 * \return 1  success\n
 *         0  terminal type not defined\n
 *         -1  termcap database inaccessible\n
 *         -2  environment variable TERM not set\n
 */
int get_termsize(int* width_, int* height_, char** error);

/*!
 * \brief Checks whether the terminal supports true color.
 *
 * \return true if the terminal supports true color, false otherwise
 */
int supports_true_color();

#endif
