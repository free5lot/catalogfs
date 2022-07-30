#ifndef INC_CATALOGFS_FILESTAT_PARSER_FORMAT_H
#define INC_CATALOGFS_FILESTAT_PARSER_FORMAT_H

#include "header_common.h"

#include <stdio.h>

// Forward declaration
struct filestat;

/** 
 * Read filestat struct from a file with filestat format
 * 
 * @param fp is a file handler of the filestat file to read from
 * @param my_stat is a target filestat stuct to read to
 * @return 0 on success, nonzero value on error
 */
int filestat_parser_format_read(FILE *fp, struct filestat *my_stat);

/** 
 * Write filestat to file by file descriptor
 * 
 * @param file_fd is a descriptor of the output file
 * @param my_stat is a filestat struct to be written
 * @param name is a file name field (not used in current version, outdated field)
 * @param path is a file path field (not used in current version, outdated field)
 * @return 0 on success, nonzero value on error
 */
int filestat_parser_format_write(const int file_fd,
								 const struct filestat *const my_stat,
								 const char *const name,
								 const char *const path);

#endif // INC_CATALOGFS_FILESTAT_PARSER_FORMAT_H
