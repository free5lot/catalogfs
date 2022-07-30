/*
  Copyright (C) 2020-present Zakhar Semenov
  
  This program can be distributed under the terms of the GNU GPLv3 or later.
*/

/**
 * CatalogFS - is a FUSE-based filesystem for viewing indexes (snapshots) of your data.
 * Perfect for indexing backups on disconnected HDD, SSD, CD, DVD or any other storages.
 * 
 * Index includes full file tree with all metadata (names, sizes, ctime, atime, mtime) 
 * and optional SHA-256 hashes BUT no actual file data content and thus has small size.
 * These indexes (can be called catalogs) have CatalogFS-compartible format.
 * The index has the same hierarchy of directories and files as the original directory.
 * Saved CatalogFS snapshots take almost no disk space but allow to check what was 
 * present in original directories or backups.
 * 
 * NOTE: IT IS NOT A FILESYSTEM FOR CREATING BACKUPS BECAUSE NO ACTUAL FILE DATA IS STORED.
 * 
 * But it's a VERY convenient way to keep track of your backups, especially ones that
 * are not easily connectable like external USB disks, CDs, flash or remote drives.
 * 
 * 
 * The ability of CatalogFS to show the original metadata including sizes of files allows
 * to view snapshots using any file manager (Dolphin, Nautilus and etc.), use tools to
 * analyze the occupied space distribution (Filelight, Disk Usage Analyzer, Baobab and etc)
 * and even properly compare directories with your backup snapshots.
 * 
 * Best used with CatalogFS_Lister python script that quickly creates CatalogFS indexes
 * and can calculate and store SHA-256 hashes of original files.
 * 
 * Both CatalogFS filesystem and CatalogFS_Lister script can be used separately 
 * with great results, but using them together provides the best experience.
 * 
 * See CatalogFS_Lister project for details.
 * 
 */

/**
 * --------------------
 * How to use CatalogFS
 * --------------------
 * 
 * 
 * To create an index (snapshot) of your data (e.g. external backup drive, CD/DVD and etc.)
 * 
 * 1. Create a directory to store your index (snapshot):
 *    $ mkdir "/home/user/my_music_collection"
 * 
 * 2. Make an index (snapshot) of data you want:
 * 
 *  - It can be done using CatalogFS_Lister python script (recommended):
 *    $ ./catalogfs_lister.py "/media/cdrom" "/home/user/my_music_collection"
 * 
 *    Using this script is a recommended way because it's faster and has an optional ability 
 *    to calculate and save hashes of files:
 *    $ ./catalogfs_lister.py --sha256 "/media/cdrom" "/home/user/my_music_collection"
 *    Note that hashes calculations are quite slow for obvious reasons.
 * 
 *    More information on CatalogFS_Lister python script is available in help:
 *    $ ./catalogfs_lister.py --help
 * 
 *  - Or you can mount CatalogFS over an empty directory and copy data files there using any 
 *    file manager or commands in terminal.
 *    Note that modification and other times won't stay original because of copying process.
 * 
 *    $ ./catalogfs "/home/user/my_music_collection"
 *    $ cp -RT "/media/cdrom" "/home/user/my_music_collection"
 *    $ fusermount -u "/home/user/my_music_collection"
 * 
 *    During this copy process the actual data IS NOT stored in the index, only metadata is.
 * 
 *    Saving files to CatalogFS is almost instant but reading files from the source is slower.
 *    Any copying tool will spend time to actually read the entire source file.
 * 
 * 
 * To view previously created index (snapshot) of your data.
 * 
 * You can view the index (snapshot) as it is, with any file manager it's already a lot.
 * But if you want to view it with original file-sizes, stats and/or modification times 
 * you should mount the index with CatalogFS filesystem as described below.
 * 
 * 1. Mount the index (snapshot) to any directory.
 * 
 *    You can simply mount catalogfs over the same index directory.
 *    It will temporary hide index files showing ones with fake size and other stats:
 *    $ ./catalogfs "/home/user/my_music_collection"
 * 
 *    One should consider mounting the CatalogFS index read-only to avoid accidental
 *    change of the index if necessary (e.g. to preserve index of backup unmodified).
 *    To do so one can pass read-only (ro) option:
 *    $ ./catalogfs -o ro "/home/user/my_music_collection"
 * 
 *    Or you can mount it to another directory.
 *    $ mkdir "/home/user/my_music_collection_catalogfs_view"
 *    $ ./catalogfs -o ro --source="/home/user/my_music_collection" "/home/user/my_music_collection_catalogfs_view"
 * 
 *    The mounted directory will show all files from the source (e.g. CD or backup disk)
 *    except it is not possible to read (open, view) the content of any file.
 * 
 *    More information on CatalogFS commandline is also available in help:
 *    $ ./catalogfs --help
 * 
 * 2. After using and viewing of index (snapshot) - you should unmount it.
 * 
 *    It can be done the same way as any other FUSE filesystem
 *    with a command "fusermount -u mountpoint_path":
 * 
 *    In case of mounting over the original index:
 *    $ fusermount -u "/home/user/my_music_collection"
 * 
 *    Or in case of mounting to a different directory:
 *    $ fusermount -u "/home/user/my_music_collection_catalogfs_view"
 * 
 */

/**
 * Usage:
 * catalogfs --source=source_dir_path mountpoint_path
 * 
 * source_dir_path is the path (directory) of the index.
 * mountpoint_path is the path (directory) to show the files with metadata from index.
 * 
 * If --source argument is not provided the mountpoint_path is used as a source directory
 * (it's a mode of mounting over the existing index to hide it with browsable fake files).
 *
 * For other command line arguments run the application with -h/--help argument.
 */

/**
 * Some technical details:
 * 
 * All paths are stored in char[] as it's a usual practice (for FUSE, too),
 * It does not mean that paths have only ANSI chars, quite the opposite,
 * in most cases they are UTF-8 strings (e.g. ext4).
 * But the code does not have to know it, because path separator is the the same 
 * for UTF-8 and ANSI and it's the only char that is used explicitly in code for paths.
 * 
 * Symlinks are copied and stored as-is by design because they have no contents.
 * 
 * Once files are stored in the index (by copying or using script), the data in them will not 
 * be modified by design for archival purposes. Changing metadata of files will affect only 
 * real files in the source directory but not the metadata inside the stored files.
 *
 * After open()/create() calls the information about size of the content is kept in the memory
 * and is written to the index file only on release() call, so the whole copying process 
 * should take no time on receiving end.
 * 
 *
 * This filesystem never uses nor relies on MAX_PATH, because MAX_PATH is a terrible thing.
 * MAX_PATH is different on different platforms and different filesystems.
 * FUSE, kernel or user's software may limit the path if needed, but CatalogFS itself tries
 * to stay as flexible as possible.
 * 
 * This filesystem works in a single-thread mode because multi-threading is not required because 
 * it is already already super fast in writing and reading as no actual contents of file is used.
 * Single-thread mode may increase FUSE filesystem's stability, and that is way more important.
 */

/**
 * Build and formatting details:
 * 
 * The code is expected to provide no errors and no warnings when build with gcc like that:
 * gcc -std=c11 -Wall -Wextra -g `pkg-config fuse3 --cflags --libs`
 * 
 * The doc-style for comments is similar to styles of FUSE and Linux kernel.
 * 
 * Int variables are defined similar to Linux kernel code (often no extra zero-initialization 
 * and declaration can be quite above the first use).
 * 
 * The tab size is 4 spaces, tabs are used for indentation and aligning.
 */

#define CATALOGFS_VERSION "3.0RC6"
#define FUSE_USE_VERSION 31

#include "header_common.h"

#include <fuse.h>

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#ifdef __FreeBSD__
#include <sys/socket.h>
#include <sys/un.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/file.h> /* flock(2) */

#include "filestat.h"
#include "filestat_converter.h"
#include "filestat_parser.h"

#include "log.h"

/**
 * A struct for storing private_data that is passed to all callback FUSE functions
 */
struct my_private_data
{
	/** Path of the source directory (can be underlying) */
	char *source_dir_path;

	/** File descriptor of the source directory (can be underlying) */
	int source_dir_fd;

	/** DIR * of the source directory (can be underlying) */
	DIR *source_dir_dir;

	/** Path of the mountpoint */
	char *mountpoint_path;

	/** Optional logfile if set by command-line option */
	FILE *logfile;

	/** Log only errors to logfile */
	bool log_only_errors;

	/** Ignore mode from filestat files and show real file's mode */
	bool ignore_saved_chmod;

	/** Ignore a/c/mtimes from filestat files and show real file's times */
	bool ignore_saved_times;

	/** Use uid from filestat files instead of real file's uid */
	bool use_saved_uid;

	/** Use gid from filestat files instead of real file's gid */
	bool use_saved_gid;
};

/**
 * Macro to obtain a pointer to the my_private_data struct
 */
#define MY_DATA ((struct my_private_data *)fuse_get_context()->private_data)

/**
 * Macro to obtain a pointer to the source directory file descriptor (source_dir_fd)
 */
#define MY_DIR_FD (MY_DATA->source_dir_fd)

/**
 * Macro to convert the path from absolute path inside the mounted FS to a 
 * relative one inside source directory.
 * It allows to use relative path with *at() functions as a relative path 
 * to the saved source_dir_fd. Otherwise these functions ignore directory descriptor.
 * Mostly this marco replaces path with path + 1 (skipping leading slash).
 */
#define RELPATH(path) ( \
	((path)[0] == '/') ? ((strlen(path) == 1) ? "." : ((path) + 1)) : (((path)[0] == '\0') ? "." : (path)))

/**
 * Structure to be stored in fh field of fuse_file_info that is allocated and 
 * used for every opened file.
 * This approach allows to skip all file size changes until flush() or release()
 * functions are called and write the content of the file only once per 
 * open()/create() call.
 */
struct my_fh_fileinfo
{
	/** File FD */
	int file_fd;

	/** File size in bytes */
	int64_t file_size;
};

/**
 * A simple wrapper for pointer cast to my_fh_fileinfo
 * 
 * @param fh is the file handle id that is actually a pointer to struct
 * @return pointer to my_fh_fileinfo struct 
 */
static inline struct my_fh_fileinfo *get_fh_fileinfo(uint64_t fh)
{
	return (struct my_fh_fileinfo *)(uintptr_t)fh;
}

/**
 * Get mode_t for file with the relative path in the directory file descriptor
 * 
 * @param dir_fd is the directory file descriptor
 * @param relpath is the relative file path
 * @return mode_t on success, 0 on error
 */
static mode_t get_mode_by_path(const int dir_fd, const char *relpath)
{
	struct stat stbuf;
	memset(&stbuf, 0, sizeof(stbuf));
	int res = fstatat(dir_fd, relpath, &stbuf, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return 0;

	return stbuf.st_mode;
}

/**
 * Save filestat to a file by the file descriptor with custom size and blocks fields,
 * while other fields are taken from the real file with provided relative path
 * 
 * @param file_fd is the file descriptor
 * @param relpath is the relative file path
 * @param file_size is the file size to use for filestat size and blocks fields
 * @return 0 on success, nonzero value on error
 */
static int save_filestat(const int file_fd, const char *relpath, int64_t file_size)
{
	int res;

	// Make a skeleton of filestat from the real file of underlying (source_dir) file
	struct filestat my_stat;
	res = fill_filestat_from_realfile(&my_stat, MY_DIR_FD, relpath);
	if (res != 0)
		return res;

	// Copy size from my_fh_fileinfo, as we are ignoring actual filesystem writing
	my_stat.size = file_size;
	my_stat.blocks = convert_filesize_to_fileblocks(file_size);

	// Both dirname() and basename() may modify the contents of path,
	// so it may be desirable to pass a copy when calling one of these functions.
	char *path_copy = strdup(relpath);
	if (path_copy == NULL)
		return -1;

	char *name = strdup(basename(path_copy));
	free(path_copy);

	if (name == NULL)
		return -1;

	res = write_filestat(file_fd, &my_stat, name, relpath);

	free(name);

	if (res != 0)
		return res;

	return 0;
}

/**
 * Free my_private_data struct including its fields
 * 
 * @param my_data is the private data struct
 */
static void free_my_private_data(struct my_private_data *my_data)
{
	if (my_data == NULL)
		return;

	free(my_data->mountpoint_path);
	my_data->mountpoint_path = NULL;
	free(my_data->source_dir_path);
	my_data->source_dir_path = NULL;

	if (my_data->source_dir_dir != NULL)
	{
		closedir(my_data->source_dir_dir);
		my_data->source_dir_dir = NULL;
	}

	if (my_data->logfile != NULL)
	{
		(void)fclose(my_data->logfile);
		my_data->logfile = NULL;
	}

	free(my_data);
}

/* ----------------------------------------------------------- *
 * Implementation of FUSE callbacks.
 * Functions that implement fuse_operations callback functions.
 * NOTE: See FUSE documentation for more details.
 * ----------------------------------------------------------- */

/** Initialize filesystem */
static void *catalogfs_init(struct fuse_conn_info *conn,
							struct fuse_config *cfg)
{
	LOG_START(NULL)

	(void)conn;
	cfg->use_ino = 1;

	/*
	 * Pick up changes from the lower filesystem right away.
	 * This is also necessary for better hardlink support.
	 * When the kernel calls the unlink() handler, it does not 
	 * know the inode of the to-be-removed entry and can 
	 * therefore not invalidate the cache of the associated 
	 * inode - resulting in an incorrect st_nlink value being 
	 * reported for any remaining hardlinks to this inode. 
	 */
	cfg->entry_timeout = 0;
	cfg->attr_timeout = 0;
	cfg->negative_timeout = 0;

	/*
	 * NOTE: it's possible to check that all functions actually use provided source_dir_fd
	 * and they do not rely on the CWD. Something like that is possible:
	 
	 //chdir("/home/user/some_completely_different_dir/");
	 */

	/*
	 * Return pointer to my_private_data stuct, that was allocated in main()
	 * and passed to fuse_main() as the last argument
	 */
	return (void *)MY_DATA;
}

/** Clean up filesystem */
static void catalogfs_destroy(void *private_data)
{
	LOG_START(NULL)

	// Free the private_data memory, that was allocated in main()
	if (private_data != NULL)
	{
		struct my_private_data *my_data = (struct my_private_data *)private_data;
		free_my_private_data(my_data);
	}
}

/** Get file attributes */
static int catalogfs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
	LOG_START(path)

	(void)fi;

	int res = fstatat(MY_DIR_FD, RELPATH(path), stbuf, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
	{
		RETURN_CODE_ERROR(path, -errno)
	}

	if (!S_ISREG(stbuf->st_mode) &&
		!S_ISDIR(stbuf->st_mode) &&
		!S_ISLNK(stbuf->st_mode))
	{
		RETURN_CODE_ERROR(path, -EPERM)
	}

	// Replace file size that is visible to user for regular files
	if (S_ISREG(stbuf->st_mode))
	{
		if (stbuf->st_size == 0)
		{
			/*
			* New file, that still was not released, so, there is 
			* no need to fill its contents with filestat metadata.
			* Do nothing, let it be as it is for now.
			*/
		}
		else
		{
			// Make a skeleton of filestat from a real file
			struct filestat my_stat;
			res = fill_filestat_from_stat(&my_stat, stbuf);
			if (res != 0)
			{
				RETURN_CODE_ERROR(path, -EPERM)
			}

			res = read_filestat(MY_DIR_FD, RELPATH(path), &my_stat);
			if (res != 0)
			{
				RETURN_CODE_ERROR(path, res)
			}

			res = fill_stat_from_filestat_with_options(
				stbuf,
				&my_stat,
				!(MY_DATA->ignore_saved_chmod),
				!(MY_DATA->ignore_saved_times),
				MY_DATA->use_saved_uid,
				MY_DATA->use_saved_gid);
			if (res != 0)
			{
				RETURN_CODE_ERROR(path, -EPERM)
			}
		}
	}

	RETURN_CODE_OK(path, 0)
}

/** Read the target of a symbolic link */
static int catalogfs_readlink(const char *path, char *buf, size_t size)
{
	LOG_START(path)

	/// NOTE: Passing (size - 1) was taken from the libfuse reference example passthrough_fh.c
	ssize_t res = readlinkat(MY_DIR_FD, RELPATH(path), buf, size - 1);
	if (res == -1)
	{
		RETURN_CODE_ERROR(path, -errno)
	}

	buf[res] = '\0'; // be sure to null-terminate according to documentation

	RETURN_CODE_OK(path, 0)
}

/** Read directory */
static int catalogfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
							 off_t offset, struct fuse_file_info *fi,
							 enum fuse_readdir_flags flags)
{
	LOG_START(path)

	(void)offset;
	(void)fi;
	(void)flags;

	int fd = openat(MY_DIR_FD, RELPATH(path), O_DIRECTORY);
	if (fd == -1)
	{
		RETURN_CODE_ERROR(path, -errno)
	}

	DIR *dir = fdopendir(fd);
	if (dir == NULL)
	{
		/// NOTE: No close(fd) because closedir(dir) will do it
		(void)closedir(dir);
		RETURN_CODE_ERROR(path, -errno)
	}

	struct dirent *de;
	while ((de = readdir(dir)) != NULL)
	{
		/*
		 * Just enumerate all files.
		 * Do not fill mode_t argument of filler() as we have no proper path,
		 * FUSE will ask for mode_t later itself (on file-by-file basis via getattr())
		 */
		filler(buf, de->d_name, NULL, 0, (enum fuse_fill_dir_flags)0);
	}

	/// NOTE: No close(fd) because closedir(dir) will do it
	(void)closedir(dir);

	RETURN_CODE_OK(path, 0)
}

/** Create a directory */
static int catalogfs_mkdir(const char *path, mode_t mode)
{
	LOG_START(path)

	int res;

	res = mkdirat(MY_DIR_FD, RELPATH(path), mode);
	if (res == -1)
	{
		RETURN_CODE_ERROR(path, -errno)
	}

	RETURN_CODE_OK(path, 0)
}

/** Remove a file */
static int catalogfs_unlink(const char *path)
{
	LOG_START(path)

	int res;

	res = unlinkat(MY_DIR_FD, RELPATH(path), 0);
	if (res == -1)
	{
		RETURN_CODE_ERROR(path, -errno)
	}

	RETURN_CODE_OK(path, 0)
}

/** Remove a directory */
static int catalogfs_rmdir(const char *path)
{
	LOG_START(path)

	int res;

	res = unlinkat(MY_DIR_FD, RELPATH(path), AT_REMOVEDIR);
	if (res == -1)
	{
		RETURN_CODE_ERROR(path, -errno)
	}

	RETURN_CODE_OK(path, 0)
}

/** Create a symbolic link */
static int catalogfs_symlink(const char *from, const char *to)
{
	LOG_START(from)

	int res;

	res = symlinkat(from, MY_DIR_FD, RELPATH(to));
	if (res == -1)
	{
		RETURN_CODE_ERROR(from, -errno)
	}

	RETURN_CODE_OK(from, 0)
}

/** Rename a file */
static int catalogfs_rename(const char *from, const char *to, unsigned int flags)
{
	LOG_START(from)

	int res;

	/**
	 * NOTE: renameat2() supports flags, we can pass them.
	 * Not sure is it possible to have problems with allowing flags.
	 * But we will avoid those anyway by not allowing flags for stability.
	*/
	if (flags)
	{
		RETURN_CODE_ERROR(from, -EINVAL)
	}

	res = renameat2(MY_DIR_FD, RELPATH(from), MY_DIR_FD, RELPATH(to), flags);
	if (res == -1)
	{
		RETURN_CODE_ERROR(from, -errno)
	}

	/**
	 * NOTE: we do not change the name field inside filestat file.
	 * In the latest format there is no name field at all.
	 * And in older formats it is preserved original for several purposes,
	 * one of them being - to preserve the original archival information.
	 */

	RETURN_CODE_OK(from, 0)
}

/** Create a hard link to a file */
static int catalogfs_link(const char *from, const char *to)
{
	LOG_START(from)

	int res;

	res = linkat(MY_DIR_FD, RELPATH(from), MY_DIR_FD, RELPATH(to), 0);
	if (res == -1)
	{
		RETURN_CODE_ERROR(from, -errno)
	}

	RETURN_CODE_OK(from, 0)
}

/** Change the permission bits of a file */
static int catalogfs_chmod(const char *path, mode_t mode,
						   struct fuse_file_info *fi)
{
	LOG_START(path)

	(void)fi;
	int res;

	res = fchmodat(MY_DIR_FD, RELPATH(path), mode, 0);

	if (res == -1)
	{
		RETURN_CODE_ERROR(path, -errno)
	}

	/**
	 * NOTE: we do not change mode field inside filestat file.
	 * It's preserved original for several purposes,
	 * one of them being - to preserve the original archival information.
	 * 
	 * To change mode field in the file - one need to reopen or recreate it.
	 */

	RETURN_CODE_OK(path, 0)
}

/** Change the owner and group of a file */
static int catalogfs_chown(const char *path, uid_t uid, gid_t gid,
						   struct fuse_file_info *fi)
{
	LOG_START(path)

	(void)fi;
	int res;

	res = fchownat(MY_DIR_FD, RELPATH(path),
				   uid, gid, AT_SYMLINK_NOFOLLOW);

	if (res == -1)
	{
		RETURN_CODE_ERROR(path, -errno)
	}

	/**
	 * NOTE: we do not change own fields inside filestat file.
	 * It's preserved original for several purposes,
	 * one of them being - to preserve the original archival information.
	 * 
	 * To change own fields in the file - one need to reopen or recreate it.
	 */

	RETURN_CODE_OK(path, 0)
}

/** Change the access and modification times of a file with nanosecond resolution */
static int catalogfs_utimens(const char *path, const struct timespec ts[2],
							 struct fuse_file_info *fi)
{
	LOG_START(path)

	(void)fi;
	int res;

	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(MY_DIR_FD, RELPATH(path), ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
	{
		RETURN_CODE_ERROR(path, -errno)
	}

	/**
	 * NOTE: we do not change time fields inside filestat file.
	 * It's preserved original for several purposes,
	 * one of them being - to preserve the original archival information.
	 * 
	 * To change time fields in the file - one need to reopen or recreate it.
	 */

	RETURN_CODE_OK(path, 0)
}

/** Create and open a file */
static int catalogfs_create(const char *path, mode_t mode,
							struct fuse_file_info *fi)
{
	LOG_START(path)

	if (!S_ISREG(mode))
	{
		RETURN_CODE_ERROR(path, -EPERM)
	}

	if (fi == NULL)
	{
		RETURN_CODE_ERROR(path, -EINVAL)
	}

	// Store my_fh_fileinfo if needed and it's a regular file
	if (fi->fh == 0 && S_ISREG(mode))
	{
		int fd = openat(MY_DIR_FD, RELPATH(path), fi->flags, mode);
		if (fd == -1)
		{
			RETURN_CODE_ERROR(path, -errno)
		}

		struct my_fh_fileinfo *data = (struct my_fh_fileinfo *)malloc(sizeof(struct my_fh_fileinfo));
		if (data == NULL)
		{
			(void)close(fd);
			RETURN_CODE_ERROR(path, -ENOMEM)
		}
		else
		{
			/**
			 * NOTE: the `else` statement by itself is redundant here, 
			 * because previous if() has return inside,
			 * but clang analyzer does not see return in RETURN_CODE macro 
			 * and provides with a warning that data pointer can be NULL.
			 */

			memset(data, 0, sizeof(struct my_fh_fileinfo));

			// Keep file descriptor
			data->file_fd = fd;

			// Set size to zero as it's create() function
			data->file_size = 0;

			/// NOTE: This FUSE convention causes false cppcheck warning about potential memory leak
			fi->fh = (uint64_t)data;
		}
	}

	/// NOTE: The FUSE convention above causes false cppcheck warning about potential memory leak
	// cppcheck-suppress memleak
	RETURN_CODE_OK(path, 0)
}

/** Open a file */
static int catalogfs_open(const char *path, struct fuse_file_info *fi)
{
	LOG_START(path)

	(void)fi;

	// Allow to open file only using create()
	RETURN_CODE_ERROR(path, -EACCES)
}

/** Read data from an open file */
static int catalogfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	LOG_START(path)

	(void)path;
	(void)buf;
	(void)size;
	(void)offset;
	(void)fi;

	// Do not allow to read anything as files do not have actual data contents
	RETURN_CODE_ERROR(path, -EPERM)
}

/** Write data to an open file */
static int catalogfs_write(const char *path, const char *buf, size_t size,
						   off_t offset, struct fuse_file_info *fi)
{
	LOG_START(path)

	(void)buf;

	// Allow writing only to previously opened or created regular files
	if (!S_ISREG(get_mode_by_path(MY_DIR_FD, RELPATH(path))))
	{
		RETURN_CODE_ERROR(path, -EPERM)
	}

	if (fi == NULL || fi->fh == 0)
	{
		RETURN_CODE_ERROR(path, -EPERM)
	}

	struct my_fh_fileinfo *data = get_fh_fileinfo(fi->fh);

	if (data == NULL || data->file_fd == -1)
	{
		RETURN_CODE_ERROR(path, -EPERM)
	}

	ssize_t min_file_size = offset + (ssize_t)size;
	if (data->file_size < min_file_size)
	{
		data->file_size = min_file_size;
	}

	RETURN_BYTES_COUNT(path, (int)size)
}

/** Get file system statistics */
static int catalogfs_statfs(const char *path, struct statvfs *stbuf)
{
	LOG_START(path)

	int res;
	(void)path;

	res = fstatvfs(MY_DIR_FD, stbuf);
	if (res == -1)
	{
		RETURN_CODE_ERROR(path, -errno)
	}

	RETURN_CODE_OK(path, 0)
}

/** Possibly flush cached data */
static int catalogfs_flush(const char *path, struct fuse_file_info *fi)
{
	LOG_START(path)

	// In fi->fh we have size of file stored
	if (fi == NULL || fi->fh == 0)
	{
		RETURN_CODE_ERROR(path, -EPERM)
	}

	struct my_fh_fileinfo *data = get_fh_fileinfo(fi->fh);
	if (data == NULL || data->file_fd == -1)
	{
		RETURN_CODE_ERROR(path, -EPERM)
	}

	/* 
	 * Note that file descriptors created by dup(2) or fork(2) share the current 
	 * file position pointer, so seeking on such files may be a subject of race condition.
	 */
	int dup_fd = dup(data->file_fd);
	if (dup_fd == -1)
	{
		RETURN_CODE_ERROR(path, -errno)
	}

	int res = save_filestat(dup_fd, RELPATH(path), data->file_size);
	if (res != 0)
	{
		RETURN_CODE_ERROR(path, res)
	}

	/* 
	 * This is called from every close() on an open file, so we call
	 * close() on the underlying filesystem. But since flush may be
	 * called multiple times for an open file, this must not really
	 * close the file. This is important if used on a network
	 * filesystem like NFS which flush the data/metadata on close() 
	 */
	res = close(dup_fd);
	if (res == -1)
	{
		RETURN_CODE_ERROR(path, -errno)
	}

	RETURN_CODE_OK(path, 0)
}

/** Release an open file
 * 
 * NOTE: 
 * It is not possible to return an error from release() because the 
 * return value of release() is ignored.
 * If you need to return errors on close, you must do that from flush().
 */
static int catalogfs_release(const char *path, struct fuse_file_info *fi)
{
	LOG_START(path)

	// In fi->fh we have size of file stored
	if (fi == NULL || fi->fh == 0)
	{
		RETURN_CODE_ERROR(path, -EPERM)
	}

	struct my_fh_fileinfo *data = get_fh_fileinfo(fi->fh);
	if (data == NULL || data->file_fd == -1)
	{
		RETURN_CODE_ERROR(path, -EPERM)
	}

	int res = save_filestat(data->file_fd, RELPATH(path), data->file_size);
	if (res != 0)
	{
		RETURN_CODE_ERROR(path, res)
	}

	res = close(data->file_fd);
	if (res == -1)
	{
		RETURN_CODE_ERROR(path, -errno)
	}

	free(data);

	RETURN_CODE_OK(path, 0)
}

/**
 * Set FUSE operations callbacks to catalogfs functions
 * 
 * @param oper is the fuse_operations struct with FUSE callbacks
 */
static void set_fuse_operations(struct fuse_operations *oper)
{
	memset(oper, 0, sizeof(struct fuse_operations));
	oper->init = catalogfs_init;
	oper->destroy = catalogfs_destroy;
	oper->getattr = catalogfs_getattr;
	/* no access() since we always use -o default_permissions */
	oper->readlink = catalogfs_readlink;
	oper->readdir = catalogfs_readdir;
	/* no mknod() since we use create and mkdir for regular files and dirs*/
	oper->mkdir = catalogfs_mkdir;
	oper->symlink = catalogfs_symlink;
	oper->unlink = catalogfs_unlink;
	oper->rmdir = catalogfs_rmdir;
	oper->rename = catalogfs_rename;
	oper->link = catalogfs_link;
	oper->chmod = catalogfs_chmod;
	oper->chown = catalogfs_chown;
	oper->utimens = catalogfs_utimens;

	oper->open = catalogfs_open;
	oper->create = catalogfs_create;

	oper->read = catalogfs_read;
	oper->write = catalogfs_write;

	oper->statfs = catalogfs_statfs;

	oper->flush = catalogfs_flush;
	oper->release = catalogfs_release;
}

/**
 * Command line options
 *
 * We cannot set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options
{
	/** Directory path to use as underlying source (by default: the same as mountpoint) */
	const char *source;

	/** Log file path (by default: NULL, not logging) */
	const char *logfile;

	/** Flag that show help argument was passed */
	int show_help;

	/** Flag that show version argument was passed */
	int show_version;

	/** Directory path for mount point */
	const char *mountpoint;

	/** Log only errors to logfile */
	int log_only_errors;

	/** Ignore mode from filestat files and show real file's mode */
	int ignore_saved_chmod;

	/** Ignore a/c/mtimes from filestat files and show real file's times */
	int ignore_saved_times;

	/** Use uid from filestat files instead of real file's uid */
	int use_saved_uid;

	/** Use gid from filestat files instead of real file's gid */
	int use_saved_gid;

} options;

/**
 * Macro for filling the fuse_opt struct
 */
#define MY_OPT(t, p, v)                   \
	{                                     \
		t, offsetof(struct options, p), v \
	}

/**
 * List of command arguments and corresponding options to set
 */
static struct fuse_opt option_spec[] = {

	/** Directory path to use as underlying source */
	MY_OPT("--source=%s", source, 0),

	/** Log file path */
	MY_OPT("--logfile=%s", logfile, 0),

	/** Flag that show help argument was passed */
	MY_OPT("-h", show_help, 1),
	/** Flag that show help argument was passed */
	MY_OPT("--help", show_help, 1),

	/** Flag that show version argument was passed */
	MY_OPT("-V", show_version, 1),
	/** Flag that show version argument was passed */
	MY_OPT("--version", show_version, 1),

	/** Log only errors to logfile */
	MY_OPT("-e", log_only_errors, 1),
	/** Log only errors to logfile */
	MY_OPT("--log_only_errors", log_only_errors, 1),

	/** Ignore mode from filestat files and show real file's mode */
	MY_OPT("-m", ignore_saved_chmod, 1),
	/** Ignore mode from filestat files and show real file's mode */
	MY_OPT("--ignore_saved_chmod", ignore_saved_chmod, 1),

	/** Ignore a/c/mtimes from filestat files and show real file's times */
	MY_OPT("-t", ignore_saved_times, 1),
	/** Ignore a/c/mtimes from filestat files and show real file's times */
	MY_OPT("--ignore_saved_times", ignore_saved_times, 1),

	/** Use uid from filestat files instead of real file's uid */
	MY_OPT("-u", use_saved_uid, 1),
	/** Use uid from filestat files instead of real file's uid */
	MY_OPT("--use_saved_uid", use_saved_uid, 1),

	/** Use gid from filestat files instead of real file's gid */
	MY_OPT("-g", use_saved_gid, 1),
	/** Use gid from filestat files instead of real file's gid */
	MY_OPT("--use_saved_gid", use_saved_gid, 1),

	FUSE_OPT_END};

/**
 * Print help in case of -h/--help command line arguments
 * 
 * @param program_name is the name of the running application
 */
static void print_help(const char *program_name)
{
	PrintToStdoutF("usage: %s [options] <mountpoint>", program_name);
	PrintToStdout("File-system specific options:");
	PrintToStdout("     --source=<s>          directory to use as underlying");
	PrintToStdout("                           (default: the same as mountpoint)");
	PrintToStdout("     --logfile=<s>         path for log file");
	PrintToStdout("                           (default: not logging)");
	PrintToStdout("-e   --log_only_errors     log only errors to log file");
	PrintToStdout("                           (default: log all if file is provided)");
	PrintToStdout("-m   --ignore_saved_chmod  ignore saved mode and show real file's mode");
	PrintToStdout("                           (default: use saved mode)");
	PrintToStdout("-t   --ignore_saved_times  ignore saved times and show real file's times");
	PrintToStdout("                           (default: use saved times)");
	PrintToStdout("-u   --use_saved_uid       use saved uid from file instead of underlying file's uid");
	PrintToStdout("                           (default: use underlying file's uid)");
	PrintToStdout("-g   --use_saved_gid       use saved gid from file instead of underlying file's gid");
	PrintToStdout("                           (default: use underlying file's gid)");
}

/**
 * Print version in case of -V/--version command line arguments
 * (will be followed by FUSE's -V output)
 * 
 * @param program_name is the name of the running application
 */
static void print_version(const char *program_name)
{
	PrintToStdoutF("%s version: v%s (FUSE: v%d)\n",
				   program_name,
				   CATALOGFS_VERSION,
				   FUSE_MAJOR_VERSION);
}

/**
 * Argument processing function for fuse_opt_parse().
 * Should have fuse_opt_proc_t type.
 *
 * The return value of this function determines whether this argument
 * is to be inserted into the output argument vector, or discarded.
 *
 * @param data is the user data passed to the fuse_opt_parse() function
 * @param arg is the whole argument or option
 * @param key determines why the processing function was called
 * @param outargs the current output argument list
 * @return -1 on error, 0 if arg is to be discarded, 1 if arg should be kept
 */
static int my_opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs)
{
	(void)(data);
	(void)(outargs);

	if (key == FUSE_OPT_KEY_NONOPT &&
		options.mountpoint == NULL)
	{
		options.mountpoint = strdup(arg);

		/// NOTE: return 1 so that FUSE would see the mountpoint itself
		return 1;
	}
	return 1;
}

/**
 * Main (an entry point)
 *
 * @param argc is the arguments count
 * @param argv is the arguments array
 * @return 0 on success, nonzero value (mostly -1) on error
 */
int main(int argc, char *argv[])
{
	(void)umask(0);

	struct fuse_operations catalogfs_oper;
	set_fuse_operations(&catalogfs_oper);

	// Command line options
	options.source = NULL;
	options.logfile = NULL;
	options.mountpoint = NULL;

	// Parsing arguments using FUSE
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	if (fuse_opt_parse(&args, &options, option_spec, my_opt_proc) == -1)
	{
		PrintToStderr("Call of fuse_opt_parse() failed");
		return -1;
	}

	bool no_mountpoint = (options.mountpoint == NULL || strlen(options.mountpoint) == 0);
	if (options.show_help ||
		no_mountpoint)
	{
		print_help(argv[0]);
		fuse_opt_add_arg(&args, "-h");
		args.argv[0][0] = '\0';

		int ret = fuse_main(args.argc, args.argv, &catalogfs_oper, NULL);
		fuse_opt_free_args(&args);
		return (no_mountpoint) ? -1 : ret;
	}

	if (options.show_version)
	{
		print_version(argv[0]);
		fuse_opt_add_arg(&args, "-V");
		args.argv[0][0] = '\0';

		int ret = fuse_main(args.argc, args.argv, &catalogfs_oper, NULL);
		fuse_opt_free_args(&args);
		return ret;
	}

	// Allocate private data
	// Passed to fuse_main, returned by catalogfs_init() and fetched by MY_DATA macro
	struct my_private_data *my_data = (struct my_private_data *)malloc(sizeof(struct my_private_data));
	if (my_data == NULL)
	{
		PrintToStderr("Call of malloc() for private data failed");
		return -ENOMEM;
	}
	memset(my_data, 0, sizeof(struct my_private_data));

	if (options.logfile != NULL &&
		strlen(options.logfile) != 0)
	{
		PrintToStdoutF("Log is set to: %s", options.logfile);

		my_data->logfile = fopen(options.logfile, "at");
		if (my_data->logfile == NULL)
		{
			PrintToStderr("Failed to open log file");
			free_my_private_data(my_data);
			fuse_opt_free_args(&args);
			return -1;
		}
	}
	else
	{
		my_data->logfile = NULL;
		PrintToStdout("Not logging because no logfile option was provided");
	}

	my_data->mountpoint_path = realpath(options.mountpoint, NULL);
	if (my_data->mountpoint_path == NULL)
	{
		PrintToStderr("Path of mountpoint is not valid");
		free_my_private_data(my_data);
		fuse_opt_free_args(&args);
		return -1;
	}

	PrintToStdoutF("Mountpoint path: %s", my_data->mountpoint_path);

	if (options.source == NULL ||
		strlen(options.source) == 0)
	{
		PrintToStdout("No source directory provided, using mountpoint instead (mount over the same directory)");

		char *copy = strdup(my_data->mountpoint_path);
		if (copy == NULL)
		{
			PrintToStderr("Failed to copy string");
			free_my_private_data(my_data);
			fuse_opt_free_args(&args);
			return -1;
		}
		my_data->source_dir_path = copy;
	}
	else
	{
		my_data->source_dir_path = realpath(options.source, NULL);
	}

	if (my_data->source_dir_path == NULL ||
		strlen(my_data->source_dir_path) == 0)
	{
		PrintToStderr("Path of source_dir is not valid");
		free_my_private_data(my_data);
		fuse_opt_free_args(&args);
		return -1;
	}

	PrintToStdoutF("Source directory path: %s", my_data->source_dir_path);

	my_data->source_dir_dir = opendir(my_data->source_dir_path);

	if (my_data->source_dir_dir == NULL)
	{
		PrintToStderr("Call of opendir() for source_dir failed");
		free_my_private_data(my_data);
		fuse_opt_free_args(&args);
		return -1;
	}

	my_data->source_dir_fd = dirfd(my_data->source_dir_dir);

	if (my_data->source_dir_fd == -1)
	{
		(void)closedir(my_data->source_dir_dir);
		my_data->source_dir_dir = NULL;

		PrintToStderr("Call of dirfd() for source directory failed");

		free_my_private_data(my_data);
		fuse_opt_free_args(&args);
		return -1;
	}

	my_data->log_only_errors = (options.log_only_errors != 0);
	my_data->ignore_saved_chmod = (options.ignore_saved_chmod != 0);
	my_data->ignore_saved_times = (options.ignore_saved_times != 0);
	my_data->use_saved_uid = (options.use_saved_uid != 0);
	my_data->use_saved_gid = (options.use_saved_gid != 0);

	/**
	 * This filesystem works in a single-thread mode because multi-threading is not required because 
	 * it is already already super fast in writing and reading as no actual contents of file is used.
	 * Single-thread mode may increase FUSE filesystem's stability, and that is way more important.
	 */
	fuse_opt_add_arg(&args, "-s");

	/**
	 * In general, all methods are expected to perform any necessary permission checking.
	 * However, a filesystem may delegate this task to the kernel by passing 
	 * the `default_permissions` mount option to `fuse_new()`.
	 * In this case, methods will only be called if the kernel's permission check 
	 * has succeeded.
	 */
	fuse_opt_add_arg(&args, "-odefault_permissions");

	// Finally call fuse_main()
	int ret = fuse_main(args.argc, args.argv, &catalogfs_oper, my_data);
	fuse_opt_free_args(&args);
	return ret;
}
