#ifndef INC_CATALOGFS_HEADER_COMMON_H
#define INC_CATALOGFS_HEADER_COMMON_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*
 * Some specific GNU stuff is used in the project and in FUSE
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <errno.h>

#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

#endif // INC_CATALOGFS_HEADER_COMMON_H
