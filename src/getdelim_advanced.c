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

#include "header_common.h"

#ifndef SSIZE_MAX
#define SSIZE_MAX ((ssize_t)(SIZE_MAX / 2))
#endif

#if USE_UNLOCKED_IO
#include "unlocked-io.h"
#define getc_maybe_unlocked(fp) getc(fp)
#elif !HAVE_FLOCKFILE || !HAVE_FUNLOCKFILE || !HAVE_DECL_GETC_UNLOCKED
#undef flockfile
#undef funlockfile
#define flockfile(x) ((void)0)
#define funlockfile(x) ((void)0)
#define getc_maybe_unlocked(fp) getc(fp)
#else
#define getc_maybe_unlocked(fp) getc_unlocked(fp)
#endif

#include "getdelim_advanced.h"

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
ssize_t getdelim_advanced(char **lineptr, size_t *n, int delimiter, int additional_delimiter, FILE *fp, size_t max_size, ssize_t *read_before_eof)
{
	ssize_t result = 0;
	size_t cur_len = 0;

	*read_before_eof = -1;

	if (lineptr == NULL || n == NULL || fp == NULL)
	{
		errno = EINVAL;
		return -1;
	}

	flockfile(fp);

	if (*lineptr == NULL || *n == 0)
	{
		char *new_lineptr;

		size_t starting_size = 120;
		if (max_size > 0 && max_size < starting_size)
		{
			starting_size = max_size;
		}
		*n = starting_size;

		new_lineptr = (char *)realloc(*lineptr, *n);
		if (new_lineptr == NULL)
		{
			errno = ENOMEM;
			result = -1;
			goto unlock_return;
		}
		*lineptr = new_lineptr;
	}

	for (;;)
	{
		int i;

		i = getc_maybe_unlocked(fp);
		if (i == EOF)
		{
			result = -1;
			*read_before_eof = (ssize_t)cur_len;
			break;
		}

		/* Make enough space for len+1 (for final NUL) bytes. */
		if (cur_len + 1 >= *n)
		{
			size_t needed_max =
				SSIZE_MAX < SIZE_MAX ? (size_t)SSIZE_MAX + 1 : SIZE_MAX;
			size_t needed = 2 * *n + 1; /* Be generous. */

			if (max_size > 0 && max_size < needed_max)
			{
				needed_max = max_size;
			}

			char *new_lineptr;

			if (needed_max < needed)
				needed = needed_max;
			if (cur_len + 1 >= needed)
			{
				errno = EOVERFLOW;
				result = -1;
				goto unlock_return;
			}

			new_lineptr = (char *)realloc(*lineptr, needed);
			if (new_lineptr == NULL)
			{
				errno = ENOMEM;
				result = -1;
				goto unlock_return;
			}

			*lineptr = new_lineptr;
			*n = needed;
		}

		(*lineptr)[cur_len] = (char)i;
		cur_len++;

		if (i == delimiter)
			break;

		if (additional_delimiter >= 0 &&
			i == additional_delimiter)
		{
			break;
		}
	}

	(*lineptr)[cur_len] = '\0';
	if (result == 0)
	{
		result = (ssize_t)cur_len;
	}

unlock_return:
	funlockfile(fp); /* doesn't set errno */

	return result;
}
