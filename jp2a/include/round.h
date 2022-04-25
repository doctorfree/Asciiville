/*! \file
 * \noop Copyright 2006-2016 Christian Stigen Larsen
 * \noop Copyright 2020 Christoph Raitzig
 * 
 * \brief Round macro.
 *
 * \author Christian Stigen Larsen
 * \author Christoph Raitzig
 * \copyright Distributed under the GNU General Public License (GPL) v2.
 */

#ifndef INC_JP2A_ROUND_H
#define INC_JP2A_ROUND_H

/*!
 * \brief Rounds a float to the nearest integer and casts to integer.
 */
# define ROUND(xfloat) (int) ( 0.5f + xfloat )

#endif
