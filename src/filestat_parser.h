#ifndef INC_CATALOGFS_FILESTAT_PARSER_H
#define INC_CATALOGFS_FILESTAT_PARSER_H

#include "header_common.h"

#include <stdio.h>

// Forward declaration
struct filestat;

/** 
 * Read filestat from a real file with relative path in the provided directory
 * 
 * @param dir_fd is a directory's file descriptor
 * @param relpath is a file's relative path in the directory
 * @param my_stat is a target filestat struct for storing read data (not zeroed before use!)
 * @return 0 on success, nonzero value on error
 */
int read_filestat(const int dir_fd,
				  const char *relpath,
				  struct filestat *my_stat);

/** 
 * Write filestat to a file by file descriptor
 * 
 * @param file_fd is a descriptor of the output file
 * @param my_stat is a filestat struct to be written
 * @param name is a file name field (not used in current version, outdated field)
 * @param path is a file path field (not used in current version, outdated field)
 * @return 0 on success, nonzero value on error (mostly -errno)
 */
int write_filestat(const int file_fd,
				   const struct filestat *const my_stat,
				   const char *const name,
				   const char *const path);

#endif // INC_CATALOGFS_FILESTAT_PARSER_H
