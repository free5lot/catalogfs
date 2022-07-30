# CatalogFS

**CatalogFS** - is a FUSE-based filesystem for viewing indexes (snapshots) of your data.

Perfect for indexing backups on disconnected HDD, SSD, CD, DVD or any other storage.

Index includes full file tree with all `metadata` (`names`, `sizes`, `ctime`, `atime`, `mtime`) and optional `SHA-256` hashes but no actual file data content and thus has small size. These indexes (can be called catalogs) have the same hierarchy of directories and files as the original directory and take almost no disk space but allow to check what was present in original directories or backups.

**NOTE: IT IS NOT A FILESYSTEM FOR CREATING BACKUPS BECAUSE NO ACTUAL FILE DATA IS STORED.**

But it's a **very** convenient way to keep track of your backups, especially ones that are not easily accessible, like external USB disks, CDs, flash or remote drives.


The ability of `CatalogFS` to show the original metadata including sizes of files allows to view snapshots using any file manager (Dolphin, Nautilus and etc.), use tools to analyze the occupied space distribution (`Filelight`, `Disk Usage Analyzer`, `Baobab` and etc) and even properly compare directories with your backup snapshots.

Best used with `CatalogFS_Lister` python script that quickly creates `CatalogFS` indexes and can calculate and store `SHA-256` hashes of original files. Both `CatalogFS` filesystem and `CatalogFS_Lister` script can be used separately with great results, but using them provides the best experience.

See `CatalogFS_Lister` project for details.


## How to use CatalogFS

#### To create an index (snapshot) of your data (e.g. external backup drive, CD/DVD and etc.)

1. Create a directory to store your index (snapshot):

   ```
   $ mkdir "/home/user/my_music_collection"
   ```

2. Make an index (snapshot) of data you want:

 - It can be done using `CatalogFS_Lister` python script (**recommended**):
 
   ```
   $ ./catalogfs_lister.py "/media/cdrom" "/home/user/my_music_collection"
   ```

   Using this script is a recommended way because it's faster and has an optional ability to calculate and save hashes of files:
   
   ```
   $ ./catalogfs_lister.py --sha256 "/media/cdrom" "/home/user/my_music_collection"
   ```
   
   Note that hashes calculations are quite slow for obvious reasons.

   More information on `CatalogFS_Lister` python script is available in help:
   
   ```
   $ ./catalogfs_lister.py --help
   ```

 - Or you can mount `CatalogFS` over an empty directory and copy data files there using any file manager or commands in terminal.
   
   Note that modification and other times won't stay original because of copying process.

   ```
   $ ./catalogfs "/home/user/my_music_collection"
   
   $ cp -RT "/media/cdrom" "/home/user/my_music_collection"
   
   $ fusermount -u "/home/user/my_music_collection"
   ```

   During this copy process the actual data IS NOT stored in the index, only metadata is.

   Saving files to `CatalogFS` is almost instant but reading files from the source is slower. Any copying tool will spend time to actually read the entire source file.


#### To view previously created index (snapshot) of your data.

You can view the index (snapshot) as it is, with any file manager it's already a lot. But if you want to view it with original file-sizes, stats and/or modification times you should mount the index with CatalogFS filesystem as described below.

1. Mount the index (snapshot) to any directory.

   You can simply mount `CatalogFS` over the same index directory. It will temporary hide index files showing ones with fake size and other stats:
   
   ```
   $ ./catalogfs "/home/user/my_music_collection"
   ```

   One should consider mounting the `CatalogFS` index read-only to avoid accidental change of the index if necessary (e.g. to preserve index of backup unmodified). To do so one can pass read-only (ro) option:
   
   ```
   $ ./catalogfs -o ro "/home/user/my_music_collection"
   ```

   Or you can mount it to another directory.
   ```
   $ mkdir "/home/user/my_music_collection_catalogfs_view"
   
   $ ./catalogfs -o ro --source="/home/user/my_music_collection" "/home/user/my_music_collection_catalogfs_view"
   ```

   The mounted directory will show all files from the source (e.g. CD or backup disk) except it is not possible to read (open, view) the content of any file.

   More information on `CatalogFS` commandline is also available in help:
   
   ```
   $ ./catalogfs --help
   ```

2. After using and viewing of index (snapshot) - you should unmount it. It can be done the same way as any other `FUSE` filesystem with a command `fusermount -u mountpoint_path`, for example:

   In case of mounting over the original index:
   
   ```
   $ fusermount -u "/home/user/my_music_collection"
   ```

   Or in case of mounting to a different directory:
   
   ```
   $ fusermount -u "/home/user/my_music_collection_catalogfs_view"
   ```




### Command-line usage:
```
catalogfs --source=source_dir_path mountpoint_path
```

where:

`source_dir_path` is the path (directory) of the index.

`mountpoint_path` is the path (directory) to show the files with metadata from index.

If `--source` argument is not provided the `mountpoint_path` is used as a source directory (it's a mode of mounting over the existing index to hide it with browsable fake files).

For other command line arguments run the application with `-h/--help` argument.



## Some technical details

All paths are stored in `char[]` as it's a usual practice (for `FUSE`, too), it does not mean that paths have only `ANSI` chars, quite the opposite,
in most cases they are `UTF-8` strings (e.g. `ext4`). But the code does not have to know it, because path separator is the the same for `UTF-8` and `ANSI` and it's the only char that is used explicitly in code for paths.

Symlinks are copied and stored as-is by design because they have no contents.

Once files are stored in the index (by copying or using script), the data in them will not be modified by design for archival purposes. Changing metadata of files will affect only real files in the source directory but not the metadata inside the stored files.

After `open()`/`create()` calls the information about size of the content is kept in the memory and is written to the index file only on `release()` call, so the whole copying process should take no time on receiving end.


This filesystem never uses nor relies on `MAX_PATH`, because `MAX_PATH` is a terrible thing. `MAX_PATH` is different on different platforms and different filesystems. `FUSE`, kernel or user's software may limit the path if needed, but `CatalogFS` itself tries to stay as flexible as possible.

This filesystem works in a single-thread mode because multi-threading is not required because it is already already super fast in writing and reading as no actual contents of file is used. Single-thread mode may increase `FUSE` filesystem's stability, and that is way more important.



## Build and formatting details

The code is expected to provide no errors and no warnings when build with gcc like that:
```
gcc -std=c11 -Wall -Wextra -g `pkg-config fuse3 --cflags --libs`
```

The doc-style for comments is similar to styles of `FUSE` and `Linux` kernel.

Int variables are defined similar to `Linux` kernel code (often no extra zero-initialization and declaration can be quite above the first use).

The tab size is 4 spaces, tabs are used for indentation and aligning.


## License
Copyright (C) 2020-present Zakhar Semenov

This program can be distributed under the terms of the GNU GPLv3 or later.

