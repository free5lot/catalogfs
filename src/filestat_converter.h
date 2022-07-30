#ifndef INC_CATALOGFS_FILESTAT_CONVERTER_H
#define INC_CATALOGFS_FILESTAT_CONVERTER_H

#include "header_common.h"

// Forward declaration
struct filestat;
struct stat;

/* ----------------------------------------------------------- *
 * Convertion and copying (filling) of stat structures
 * ----------------------------------------------------------- */

/** 
 * Fill filestat struct my_stat from stat struct stbuf 
 * 
 * @param my_stat is the file descriptor
 * @param stbuf is the relative file path
 * @return 0 on success, not 0 on error
 */
int fill_filestat_from_stat(struct filestat *my_stat, const struct stat *const stbuf);

/** 
 * Fill filestat struct from a real file with the relative path in the provided directory
 * 
 * @param my_stat is the target filestat struct
 * @param dir_fd is the source directory file descriptor
 * @param relpath is the real file's relative path in the directory
 * @return 0 on success, nonzero value on error
 */
int fill_filestat_from_realfile(struct filestat *my_stat, int dir_fd, const char *relpath);

/** 
 * Fill stat struct from filestat struct with custom values for some fields
 * 
 * @param stbuf is the target stat struct
 * @param my_stat is the source filestat struct
 * @param mode determines if the mode field should be copied
 * @param times determines if the *time and *atimensec fields should be copied
 * @param uid determines if the uid field should be copied
 * @param gid determines if the gid field should be copied
 * @return 0 on success, nonzero value on error
 */
int fill_stat_from_filestat_with_options(struct stat *stbuf,
										 const struct filestat *const my_stat,
										 bool mode, bool times, bool uid, bool gid);

/* ----------------------------------------------------------- *
 * Auxillary functions
 * ----------------------------------------------------------- */

/** 
 * Convert filesize to fileblocks with 512 block size
 * 
 * @param size is the size of file
 * @return number of required blocks for provided file size
 */
int64_t convert_filesize_to_fileblocks(int64_t size);

#endif // INC_CATALOGFS_FILESTAT_CONVERTER_H
