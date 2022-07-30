/*
 * Modified version (advanced) of getdelim supporting 2 delimiters
 */

/* getdelim.c --- Implementation of replacement getdelim function.
Copyright (C) 1994, 1996, 1997, 1998, 2001, 2003, 2005 Free
Software Foundation, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

#ifndef INC_GETDELIM_ADVANCED_H
#define INC_GETDELIM_ADVANCED_H

#include "header_common.h"

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

/**
 * My advanced modification of get_delim() with additional delimiter
 * 
 * Read up to (and including) a DELIMITER or Additional DELIMITER
 * from FP into *LINEPTR (and NUL-terminate it) up to max_size length.
 *  
 * The *LINEPTR is a pointer returned from malloc (or NULL),
 * pointing to *N characters of space. It is realloc-ed as necessary.
 * 
 * Param additional_delimiter can be -1 to be ignored
 * Param max_size can be 0 to be ignored
 * 
 * Returns the number of characters read (not including the null terminator),
 * or -1 on error or EOF.
 * 
 * @param lineptr is the buffer for read chars before (and including) any delimiter
 * @param n is the size of allocated buffer for lineptr
 * @param delimiter is the delimiter to search for
 * @param additional_delimiter is the additional delimiter to search for (-1 means to ignore it)
 * @param fp is the FILE pointer of file to read from
 * @param max_size is the maximum size to be read before delimiter (0 means no limit)
 * @param read_before_eof in case of EOF it has the number of characters read (like return value), otherwise -1
 * @return the number of characters read (not including the null terminator), -1 on error or EOF
 */
ssize_t getdelim_advanced(char **lineptr, size_t *n, int delimiter, int additional_delimiter, FILE *fp, size_t max_size, ssize_t *read_before_eof);

#endif // INC_GETDELIM_ADVANCED_H
