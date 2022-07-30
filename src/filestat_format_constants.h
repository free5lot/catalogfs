#ifndef INC_CATALOGFS_FILESTAT_FORMAT_CONSTANTS_H
#define INC_CATALOGFS_FILESTAT_FORMAT_CONSTANTS_H

#include "header_common.h"

/** Header option of the file (must be the first option) */
#define FILESTAT_HEADER_OPTION "CatalogFS"

/** Version of the current file stat format */
#define FILESTAT_VERSION_3 ((uint32_t)3)

/** Maximum stats file size (1MiB), should be enough for filestat information. */
#define FILESTAT_MAXSIZE (1048576)

/** Maximum length of filestat header in bytes */
#define FILESTAT_MAX_HEADER_LENGTH (120)

/** Maximum length of option part (before separator) */
#define FILESTAT_MAX_LENGTH_OPTION (1024)

/** Maximum length of value part (after separator) */
#define FILESTAT_MAX_LENGTH_VALUE (1048576)

/** Character that starts comment line */
#define FILESTAT_COMMENT_CHAR_1 ('#')
/** To be semi-compatible with ini files, just in case */
#define FILESTAT_COMMENT_CHAR_2 (';')

/** Character that is considered as an option-value separator */
#define FILESTAT_SEPARATOR_CHAR_MAIN ('=')

/** Characters that are considered to be a proper new line character */
#define FILESTAT_NEWLINE_CHAR_1 ('\n')
/** Alternative new line, just in case, so Windows and Mac users suffer less */
#define FILESTAT_NEWLINE_CHAR_2 ('\r')

/* ----------------------------------------------------------- *
 * Legacy format support
 * ----------------------------------------------------------- */

/** Character that is considered an option-value separator in older legacy formats */
#define FILESTAT_LEGACY_SEPARATOR_CHAR (':')

/** For ability to load old (legacy) formart v1 */
#define FILESTAT_LEGACY_HEADER_V1 ("CatalogFS.File.1")
/** For ability to load old (legacy) formart v2 */
#define FILESTAT_LEGACY_HEADER_V2 ("CatalogFS.File.2")

/**
 * Some options from old format (mostly 'name') require some tricky code 
 * to be used to read them, because those options allow newlines and any
 * other chars and terminated with '\0' only.
 * After removal of this tricky code and deprecated such complicated values
 * in filestat files, we have to terminate reading old (legacy) files
 * when we face these options.
 * That's OK, because these options are always the last ones by design.
 */
#define FILESTAT_LEGACY_TERMINAL_OPTION_1 ("name")
/** Second legacy terminal option is path */
#define FILESTAT_LEGACY_TERMINAL_OPTION_2 ("path")

/* ----------------------------------------------------------- */

#endif // INC_CATALOGFS_FILESTAT_FORMAT_CONSTANTS_H
