#include "header_common.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#include "filestat.h"
#include "filestat_format_constants.h"
#include "filestat_converter.h"
#include "filestat_parser.h"

/** 
 * Fill filestat struct from stat struct 
 * 
 * @param my_stat is the target filestat struct
 * @param stbuf is the source stat struct
 * @return 0 on success, -1 on error (NULL argment)
 */
int fill_filestat_from_stat(struct filestat *my_stat, const struct stat *const stbuf)
{
	if (my_stat == NULL || stbuf == NULL)
		return -1;

	my_stat->size = stbuf->st_size;
	my_stat->blocks = stbuf->st_blocks;
	my_stat->mode = stbuf->st_mode;
	my_stat->uid = stbuf->st_uid;
	my_stat->gid = stbuf->st_gid;
	my_stat->atime = stbuf->st_atim.tv_sec;
	my_stat->mtime = stbuf->st_mtim.tv_sec;
	my_stat->ctime = stbuf->st_ctim.tv_sec;
	my_stat->atimensec = stbuf->st_atim.tv_nsec;
	my_stat->mtimensec = stbuf->st_mtim.tv_nsec;
	my_stat->ctimensec = stbuf->st_ctim.tv_nsec;
	my_stat->nlink = stbuf->st_nlink;
	my_stat->blksize = stbuf->st_blksize;

	return 0;
}

/** 
 * Fill filestat struct from a real file with the relative path in the provided directory
 * 
 * @param my_stat is the target filestat struct
 * @param dir_fd is the source directory file descriptor
 * @param relpath is the real file's relative path in the directory
 * @return 0 on success, nonzero value on error
 */
int fill_filestat_from_realfile(struct filestat *my_stat, int dir_fd, const char *relpath)
{
	int res;

	struct stat stbuf;
	memset(&stbuf, 0, sizeof(struct stat));
	res = fstatat(dir_fd, relpath, &stbuf, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return -EPERM;

	memset(my_stat, 0, sizeof(struct filestat));
	res = fill_filestat_from_stat(my_stat, &stbuf);
	if (res == -1)
		return -EPERM;

	return 0;
}

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
										 bool mode, bool times, bool uid, bool gid)
{
	if (my_stat == NULL || stbuf == NULL)
	{
		return -1;
	}

	stbuf->st_size = my_stat->size;
	stbuf->st_blocks = my_stat->blocks;

	if (mode)
	{
		stbuf->st_mode = my_stat->mode;
	}

	if (uid)
	{
		stbuf->st_uid = my_stat->uid;
	}

	if (gid)
	{
		stbuf->st_gid = my_stat->gid;
	}

	if (times)
	{
		stbuf->st_atim.tv_sec = my_stat->atime;
		stbuf->st_mtim.tv_sec = my_stat->mtime;
		stbuf->st_ctim.tv_sec = my_stat->ctime;

		stbuf->st_atim.tv_nsec = my_stat->atimensec;
		stbuf->st_mtim.tv_nsec = my_stat->mtimensec;
		stbuf->st_ctim.tv_nsec = my_stat->ctimensec;
	}

	/* Maybe it's better to keep real nlink and st_blksize from the source (real) dir:
	
	//stbuf->st_nlink = my_stat->nlink;
	//my_stat->blksize = stbuf->st_blksize;
	*/

	return 0;
}

/** 
 * Convert filesize to fileblocks with 512 block size
 * 
 * @param size is the size of file
 * @return number of required blocks for provided file size
 */
int64_t convert_filesize_to_fileblocks(int64_t size)
{
	return (size / 512) + 1;
}
