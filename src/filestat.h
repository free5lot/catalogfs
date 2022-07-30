#ifndef INC_CATALOGFS_FILESTAT_H
#define INC_CATALOGFS_FILESTAT_H

#include "header_common.h"

#include <stdint.h>

/**
 * Information that is actually stored inside filestat files as contents
 */
struct filestat
{
	/** Size of file, in bytes. */
	// cppcheck-suppress unusedStructMember
	int64_t size;

	/** Number of 512-byte blocks allocated. */
	// cppcheck-suppress unusedStructMember
	int64_t blocks;

	/** File mode. */
	// cppcheck-suppress unusedStructMember
	uint32_t mode;

	/** User ID of the file's owner. */
	// cppcheck-suppress unusedStructMember
	uint32_t uid;
	/** Group ID of the file's group. */
	// cppcheck-suppress unusedStructMember
	uint32_t gid;

	/** Time of last access. */
	// cppcheck-suppress unusedStructMember
	int64_t atime;
	/** Time of last modification. */
	// cppcheck-suppress unusedStructMember
	int64_t mtime;
	/** Time of last status change. */
	// cppcheck-suppress unusedStructMember
	int64_t ctime;

	/** Nsecs of last access. */
	// cppcheck-suppress unusedStructMember
	int64_t atimensec;
	/** Nsecs of last modification. */
	// cppcheck-suppress unusedStructMember
	int64_t mtimensec;
	/** Nsecs of last status change. */
	// cppcheck-suppress unusedStructMember
	int64_t ctimensec;

	/** Hard Link count. */
	// cppcheck-suppress unusedStructMember
	uint64_t nlink;
	/** Optimal block size for I/O. */
	// cppcheck-suppress unusedStructMember
	int64_t blksize;
};

#endif // INC_CATALOGFS_FILESTAT_H
