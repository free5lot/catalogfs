#include "header_common.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#include "filestat_parser.h"
#include "filestat.h"
#include "filestat_format_constants.h"
#include "filestat_parser_format.h"

/** 
 * Check that size of filestat file is small enough to ignore huge and invalid files.
 * 
 * @param fd is the file descriptor of the filestat file
 * @return 0 on success, nonzero value on error
 */
static int check_filestat_file_size_is_small_enough(int fd)
{
	struct stat stbuf;
	memset(&stbuf, 0, sizeof(struct stat));

	int res = fstat(fd, &stbuf);
	if (res == -1)
		return -errno;

	if (!S_ISREG(stbuf.st_mode))
		return -EPERM;

	if (stbuf.st_size > FILESTAT_MAXSIZE)
		return -EPERM; // or -EFBIG should be better?

	return 0;
}

/** 
 * Read filestat from a real file with relative path in the provided directory
 * 
 * @param dir_fd is a directory's file descriptor
 * @param relpath is a file's relative path in the directory
 * @param my_stat is a target filestat struct for storing read data (not zeroed before use!)
 * @return 0 on success, nonzero value on error
 */
int read_filestat(const int dir_fd, const char *relpath, struct filestat *my_stat)
{
	if (my_stat == NULL)
		return -EINVAL;

	/*
	 * NOTE: we do not clear my_stat on purpose, because filestat file 
	 * is allowed not to have all existing fields.

	//memset(my_stat, 0, sizeof(struct filestat));
	 */

	int res;

	int fd = openat(dir_fd, relpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = check_filestat_file_size_is_small_enough(fd);
	if (res != 0)
	{
		(void)close(fd);
		return res;
	}

	// Making FILE wrapper
	FILE *fp = fdopen(fd, "rb"); // NOTE: "b" flag is mostly ignored on modern nix systems
	if (fp == NULL)
	{
		int errno_stored = errno;
		(void)close(fd);
		return -errno_stored;
	}

	// Main function that reads from FILE*
	res = filestat_parser_format_read(fp, my_stat);
	if (res != 0)
	{
		/// NOTE: No close(fd) because fclose(fp) will do it
		(void)fclose(fp);
		return res;
	}

	/// NOTE: No close(fd) because fclose(fp) will do it
	(void)fclose(fp);

	return 0;
}

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
				   const char *const path)
{
	if (file_fd == 0 || my_stat == NULL)
	{
		return -EPERM;
	}

	int res;

	res = filestat_parser_format_write(file_fd, my_stat, name, path);
	if (res != 0)
	{
		return res;
	}

	return 0;
}
