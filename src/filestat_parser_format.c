#include "header_common.h"

#include <unistd.h>

#include "filestat_parser_format.h"

#include "filestat.h"
#include "filestat_format_constants.h"

#include "getdelim_advanced.h"

/** 
 * Trim the string of spaces (according to isspace())
 * Source string is not affected nor modified
 * 
 * @param str is the source string for trimming
 * @return new allocated trimmed string, NULL on error
 */
static char *trim_string(const char *str)
{
	if (!str)
		return NULL;

	ssize_t len = (ssize_t)strlen(str);
	if (len <= 0)
		return strdup("");

	while (len > 0 && isspace(str[len - 1]))
	{
		len--;
	}

	if (len == 0)
		return strdup("");

	while (*str && isspace(*str))
	{
		str++;
		len--;
	}

	if (!*str)
		return strdup("");

	return strndup(str, (size_t)len);
}

/** 
 * Check if the string is empty (NULL string is also considered to be empty)
 * 
 * @param str is the string to check
 * @return true if the string is empty, false otherwise
 */
static bool is_string_empty(const char *str)
{
	if (!str)
		return true;

	if (!*str)
		return true;

	return false;
}

/** 
 * Check if the string is empty or whitespace (according to isspace())
 * 
 * @param str is the string to check
 * @return true if the string is empty or whitespace, false otherwise
 */
static bool is_string_empty_or_whitespace(const char *str)
{
	if (!str)
		return true;

	if (!*str)
		return true;

	char *str_trimmed = trim_string(str);
	if (str_trimmed == NULL)
	{
		// some error happened, so we can't say if the string was empty or whitespace
		return false;
	}

	bool is_empty = is_string_empty(str_trimmed);
	free(str_trimmed);

	return is_empty;
}

/** 
 * Check if the string is a comment starting with FILESTAT_COMMENT_CHAR_*
 * Starting spaces are allowed and ignored
 * 
 * @param str is the string to check
 * @return true if the string is comment, false otherwise
 */
static bool is_string_a_comment(const char *str)
{
	if (!str)
		return false;

	if (!*str)
		return false;

	while (*str && isspace(*str))
	{
		str++;
	}

	if (str[0] == FILESTAT_COMMENT_CHAR_1 ||
		str[0] == FILESTAT_COMMENT_CHAR_2)
	{
		return true;
	}
	return false;
}

/** 
 * Read line from the filestat file handler using newline delimiters.
 * 
 * The provided buffer (if not NULL) will be reused and reallocated if needed.
 * If the function returns 0 it guaranties to free the buffer if it was NULL on the call.
 * 
 * EOF does not mean error and considered to be just another line delimiter for reading.
 * 
 * Read about getdelim() and local getdelim_advanced() for details.
 * 
 * @param buf is the buffer for storing the line (passing NULL is fine)
 * @param buf_size is the original and resulting buffer size
 * @param fp is the file handler of the filestat file
 * @param max_size is the maximum size to be read before delimiter (0 means no limit)
 * @return the number of characters read (not including the null terminator), negative number on error
 */
static ssize_t filestat_read_line(char **buf, size_t *buf_size, FILE *fp, size_t max_size)
{
	bool buf_was_null = (*buf == NULL);
	ssize_t read_before_eof = -1;
	ssize_t read = getdelim_advanced(buf, buf_size, FILESTAT_NEWLINE_CHAR_1, FILESTAT_NEWLINE_CHAR_2, fp, max_size, &read_before_eof);
	if (read < 0)
	{
		if (read_before_eof >= 0)
		{
			// NOTE: It's EOF, we allow that, it's also a kind of line delimiter
			return read_before_eof;
		}
		else
		{
			if (buf_was_null)
			{
				// we allocated it, so we should free it
				free(*buf);
				*buf = NULL;
				*buf_size = 0;
			}
			return read;
		}
	}

	return read;
}

/** 
 * Clean the line from the filestat file by removing 
 * newline chars (line delimiters) from the end.
 * 
 * @param line is the line (string) to clean
 */
static void filestat_clean_line(char *const line)
{
	ssize_t line_len = (ssize_t)strlen(line);
	if (line_len <= 0)
	{
		return;
	}

	if (line[line_len - 1] == FILESTAT_NEWLINE_CHAR_1 ||
		line[line_len - 1] == FILESTAT_NEWLINE_CHAR_2)
	{
		line[line_len - 1] = '\0';
	}
}

/** 
 * Parse a line from the filestat file to get an option-value pair.
 * 
 * Returns 0 and sets was_skipped flag in case of comment lines or whitespace lines.
 * 
 * Supports getting pairs from current and legacy formats.
 * 
 * @param line is a line (string) to get a pair from
 * @param was_skipped means the line was skipped (a comment or whitespace line)
 * @param option is a option string to get to
 * @param value is a value string to get to
 * @param use_legacy_format describes if the legacy format should be used for reading
 * @return 0 on success or skipping, nonzero value on parsing error
 */
static int filestat_parse_line(const char *const line, bool *was_skipped, char **option, char **value, const bool use_legacy_format)
{
	if (*option != NULL ||
		*value != NULL)
	{
		// We want to allocate outselves
		return -1;
	}

	ssize_t line_len = (ssize_t)strlen(line);

	// Skip comment lines
	if (is_string_a_comment(line))
	{
		*was_skipped = true;
		return 0;
	}

	// Skip empty and whitespace lines
	if (is_string_empty_or_whitespace(line))
	{
		*was_skipped = true;
		return 0;
	}

	*was_skipped = false;

	// It's not a comment line, let's parse it by splitting with separator
	char separator = (use_legacy_format) ? FILESTAT_LEGACY_SEPARATOR_CHAR : FILESTAT_SEPARATOR_CHAR_MAIN;

	char *separator_ptr = index(line, separator);
	if (separator_ptr == NULL)
	{
		return -1;
	}

	ssize_t option_len = separator_ptr - line;
	// Empty options are not allowed
	if (option_len <= 0 ||
		option_len > FILESTAT_MAX_LENGTH_OPTION)
	{
		return -1;
	}

	ssize_t value_len = line_len - ((separator_ptr + 1) - line);

	// Empty values ARE allowed, value_len can be 0
	if (value_len < 0 ||
		value_len > FILESTAT_MAX_LENGTH_VALUE)
	{
		return -1;
	}

	char *opt = (char *)malloc(((size_t)(option_len + 1) * sizeof(char)));
	if (!opt)
	{
		return -ENOMEM;
	}
	(void)strncpy(opt, line, (size_t)option_len);
	opt[option_len] = '\0';

	char *val = (char *)malloc(((size_t)(value_len + 1) * sizeof(char)));
	if (!val)
	{
		free(opt);
		opt = NULL;

		return -1;
	}
	(void)strncpy(val, separator_ptr + 1, (size_t)value_len);
	val[value_len] = '\0';

	*option = opt;
	*value = val;

	return 0;
}

/** 
 * Scan value string for formatted value and put it to the provided place pointer.
 * 
 * @param str_value is a string with a value
 * @param str_fmt is an expected format of the value
 * @param place is a target location for scanned value
 * @return 0 on success, nonzero value on error
 */
static int filestat_sscanf_value(const char *str_value, const char *str_fmt, void *place)
{
	if (str_value == NULL ||
		str_fmt == NULL ||
		place == NULL)
	{
		return -EINVAL;
	}

	const char *append_string = "%n";
	char *buf = (char *)malloc((strlen(str_fmt) + strlen(append_string) + 1) * sizeof(char));
	if (!buf)
	{
		return -ENOMEM;
	}

	strcpy(buf, str_fmt);
	strcat(buf, append_string);

	int pos = 0;
	int narg = sscanf(str_value, buf, place, &pos);
	free(buf);

	if (narg != 1)
	{
		return -EIO;
	}

	// Check to avoid wrong 123AB -> 123 conversion
	if ((size_t)pos != strlen(str_value))
	{
		return -EIO;
	}

	return 0;
}

/** 
 * Get the next option-value pair from the filestat file handler.
 * 
 * Automatically skips comment lines and whitespace lines.
 * 
 * In case the option-value pair is not extractable from the line,
 * the line that caused the fail of parsing is returned (failed line).
 * 
 * Supports getting pairs from current and legacy formats.
 * 
 * When the file is ended (end-of-file archived and nothing was read) 
 * the eof_file_finished flag is set to distinguish the situation from any errors.
 * 
 * @param eof_file_finished is the flag to set in case of end-of-file archived
 * @param option is a option string to read the option to
 * @param value is a value string to read the value to
 * @param fp is the file handler of the filestat file
 * @param max_size is the maximum size to be read before delimiter (0 means no limit)
 * @param failed_line is the returned line that caused the fail of parsing
 * @param use_legacy_format describes if the legacy format should be used for reading
 * @return 0 on success or eof, nonzero value on reading or parsing error
 */
static int filestat_get_next_option_pair(bool *eof_file_finished, char **option, char **value, FILE *fp, size_t max_size, char **failed_line, const bool use_legacy_format)
{
	*eof_file_finished = false;
	*failed_line = NULL;

	if (*option != NULL ||
		*value != NULL)
	{
		// We want to allocate ourselves
		return -1;
	}

	while (true)
	{
		char *line = NULL;
		size_t line_size = 0;
		ssize_t line_read = filestat_read_line(&line, &line_size, fp, max_size);
		if (line_read < 0)
		{
			return (errno > 0) ? (-errno) : -1;
		}

		if (line_read == 0)
		{
			// EOF, not an error
			*eof_file_finished = true;
			free(line);
			return 0;
		}

		filestat_clean_line(line);

		char *opt = NULL;
		char *val = NULL;
		bool was_skipped = false;
		filestat_parse_line(line, &was_skipped, &opt, &val, use_legacy_format);

		if (was_skipped)
		{
			// line was skipped, maybe it's a comment, no need to free opt and val
			free(line);
			continue;
		}

		char *opt_trimmed = trim_string(opt);
		free(opt);

		if (opt_trimmed == NULL)
		{
			// some error happened, so we can't say if the string was empty or whitespace
			free(val);
			*failed_line = line;
			return -1;
		}

		bool is_empty = is_string_empty(opt_trimmed);
		if (is_empty)
		{
			// option can not be empty
			free(opt_trimmed);
			free(val);

			*failed_line = line;
			return -1;
		}

		/**
		 * NOTE: we can trim value or not depending on option value
		 * but in current format we does not use values that should not be 
		 * trimmed (like or name, path), so we can trim it anyway.
		 */

		char *val_trimmed = trim_string(val);
		free(val);

		if (val_trimmed == NULL)
		{
			free(opt_trimmed);
			*failed_line = line;
			return -1;
		}

		*value = val_trimmed;
		*option = opt_trimmed;

		free(line);

		return 0;
	}
}

/** 
 * Process option-value pair from the filestat file to overwrite 
 * the corresponding field in the filestat struct.
 * 
 * Unknown option strings are ignored (it's consider to be OK)
 * 
 * @param option is an option string
 * @param value is a value string
 * @param my_stat is a filestat struct to put the processing result to
 * @return 0 on success or unknown option string, nonzero value on error
 */
static int filestat_process_option_pair(const char *const option, const char *const value, struct filestat *my_stat)
{
	int res = 0;
	if (strcmp(option, "size") == 0)
	{
		res = filestat_sscanf_value(value, "%" SCNd64, &my_stat->size);
	}
	else if (strcmp(option, "blocks") == 0)
	{
		res = filestat_sscanf_value(value, "%" SCNd64, &my_stat->blocks);
	}
	else if (strcmp(option, "mode") == 0)
	{
		res = filestat_sscanf_value(value, "%" SCNu32, &my_stat->mode);
	}
	else if (strcmp(option, "uid") == 0)
	{
		res = filestat_sscanf_value(value, "%" SCNu32, &my_stat->uid);
	}
	else if (strcmp(option, "gid") == 0)
	{
		res = filestat_sscanf_value(value, "%" SCNu32, &my_stat->gid);
	}
	else if (strcmp(option, "atime") == 0)
	{
		res = filestat_sscanf_value(value, "%" SCNd64, &my_stat->atime);
	}
	else if (strcmp(option, "mtime") == 0)
	{
		res = filestat_sscanf_value(value, "%" SCNd64, &my_stat->mtime);
	}
	else if (strcmp(option, "ctime") == 0)
	{
		res = filestat_sscanf_value(value, "%" SCNd64, &my_stat->ctime);
	}
	else if (strcmp(option, "atimensec") == 0)
	{
		res = filestat_sscanf_value(value, "%" SCNd64, &my_stat->atimensec);
	}
	else if (strcmp(option, "mtimensec") == 0)
	{
		res = filestat_sscanf_value(value, "%" SCNd64, &my_stat->mtimensec);
	}
	else if (strcmp(option, "ctimensec") == 0)
	{
		res = filestat_sscanf_value(value, "%" SCNd64, &my_stat->ctimensec);
	}
	else if (strcmp(option, "nlink") == 0)
	{
		res = filestat_sscanf_value(value, "%" SCNu64, &my_stat->nlink);
	}
	else if (strcmp(option, "blksize") == 0)
	{
		res = filestat_sscanf_value(value, "%" SCNd64, &my_stat->blksize);
	}
	else
	{
		// Ignore not-used and unknown fields (it's OK to have them)
		return 0;
	}

	return res;
}

/** 
 * Check if the option-value pair from the filestat file is a 
 * correct header-option with a supported format version in value.
 * 
 * @param option is an option that is checked to be FILESTAT_HEADER_OPTION
 * @param value is a value that should be a supported version of header
 * @return 0 if the header is correct and the version is supported, nonzero value on error
 */
static int filestat_is_header_correct(const char *const option, const char *const value)
{
	int res;
	if (strcmp(option, FILESTAT_HEADER_OPTION) != 0)
	{
		res = -1;
	}
	else
	{
		uint32_t version = 0;
		res = filestat_sscanf_value(value, "%" SCNu32, &version);
		if (res == 0)
		{
			if (version != FILESTAT_VERSION_3)
			{
				// unsupported version
				res = -1;
			}
		}
	}

	return res;
}

/** 
 * Check if the line (string) from the filestat file 
 * is a header of the older (legacy) format.
 * 
 * @param line is a line to check
 * @return true if the line is a legacy header, false otherwise
 */
static bool filestat_is_line_a_legacy_header(const char *const line)
{
	if (line == NULL)
	{
		return false;
	}

	if (strcmp(line, FILESTAT_LEGACY_HEADER_V1) == 0 ||
		strcmp(line, FILESTAT_LEGACY_HEADER_V2) == 0)
	{
		return true;
	}

	return false;
}

/** 
 * Check if the option of the legacy filestat file is a one
 * that requires to stop parsing file (as successfully finished)
 * because a lot of legacy tricky and complicated code has 
 * been removed for these legacy options parsing.
 * 
 * It's OK and desired, because these options (name and path) are not 
 * used anymore and present as the last ones in files by design.
 * 
 * @param option is the option name to check
 * @return true if option is a legacy terminal one, false otherwise
 */
static bool filestat_is_option_a_legacy_terminal_one(const char *const option)
{
	if (option == NULL)
	{
		return false;
	}

	if (strcmp(option, FILESTAT_LEGACY_TERMINAL_OPTION_1) == 0 ||
		strcmp(option, FILESTAT_LEGACY_TERMINAL_OPTION_2) == 0)
	{
		return true;
	}

	return false;
}

/** 
 * Read filestat struct from a file with filestat format
 * 
 * @param fp is a file handler of the filestat file to read from
 * @param my_stat is a target filestat stuct to read to
 * @return 0 on success, nonzero value on error
 */
int filestat_parser_format_read(FILE *fp, struct filestat *my_stat)
{
	bool it_is_header_line = true;

	bool use_legacy_format = false;

	while (true)
	{
		// Header is stronger limited, to avoid reading too much of wrong file format
		size_t max_size = (it_is_header_line) ? FILESTAT_MAX_HEADER_LENGTH : 0;

		bool eof_file_finished = false;
		char *option = NULL;
		char *value = NULL;
		char *failed_line = NULL;
		int res = filestat_get_next_option_pair(&eof_file_finished, &option, &value, fp, max_size, &failed_line, use_legacy_format);
		if (res < 0)
		{
			free(option);
			free(value);

			if (filestat_is_line_a_legacy_header(failed_line))
			{
				// it's an old (legacy) format, that we support
				use_legacy_format = true;
				it_is_header_line = false;
				continue;
			}
			else
			{
				// actual format error
				free(failed_line);
				return res;
			}
		}

		if (eof_file_finished)
		{
			break;
		}

		if (it_is_header_line)
		{
			res = filestat_is_header_correct(option, value);

			it_is_header_line = false;
		}
		else
		{
			if (use_legacy_format &&
				filestat_is_option_a_legacy_terminal_one(option))
			{
				// finish reading and return OK
				free(option);
				free(value);
				break;
			}

			res = filestat_process_option_pair(option, value, my_stat);
		}

		free(option);
		free(value);

		if (res != 0)
		{
			return -EPERM;
		}
	}

	// Check results
	if (
		my_stat->size < 0 ||
		my_stat->blocks < 0 ||
		(my_stat->atime < 0 || my_stat->ctime < 0 || my_stat->mtime < 0) ||
		(my_stat->atimensec < 0 || my_stat->ctimensec < 0 || my_stat->mtimensec < 0) ||
		my_stat->blksize < 0)
	{
		return -EPERM;
	}

	return 0;
}

/** 
 * Write filestat to a file by file descriptor
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
								 const char *const path)
{
	// Not used in current version, outdated field
	(void)name;
	// Not used in current version, outdated field
	(void)path;

	if (file_fd == 0 || my_stat == NULL)
	{
		return -EPERM;
	}

	ssize_t res;

	res = ftruncate(file_fd, 0);
	if (res != 0)
	{
		return -errno;
	}

	res = lseek(file_fd, 0, SEEK_SET);
	if (res != 0)
	{
		return -errno;
	}

	// Header
	res = dprintf(file_fd, "%s=%" PRIu32 "\n", FILESTAT_HEADER_OPTION, FILESTAT_VERSION_3);
	if (res < 0)
	{
		return -errno;
	}

	// Options
	res = dprintf(file_fd, "size=%" PRId64 "\n", my_stat->size);
	if (res < 0)
	{
		return -errno;
	}

	res = dprintf(file_fd, "blocks=%" PRId64 "\n", my_stat->blocks);
	if (res < 0)
	{
		return -errno;
	}

	res = dprintf(file_fd, "mode=%" PRIu32 "\n", my_stat->mode);
	if (res < 0)
	{
		return -errno;
	}

	res = dprintf(file_fd, "uid=%" PRIu32 "\n", my_stat->uid);
	if (res < 0)
	{
		return -errno;
	}

	res = dprintf(file_fd, "gid=%" PRIu32 "\n", my_stat->gid);
	if (res < 0)
	{
		return -errno;
	}

	res = dprintf(file_fd, "atime=%" PRId64 "\n", my_stat->atime);
	if (res < 0)
	{
		return -errno;
	}

	res = dprintf(file_fd, "mtime=%" PRId64 "\n", my_stat->mtime);
	if (res < 0)
	{
		return -errno;
	}

	res = dprintf(file_fd, "ctime=%" PRId64 "\n", my_stat->ctime);
	if (res < 0)
	{
		return -errno;
	}

	res = dprintf(file_fd, "atimensec=%" PRId64 "\n", my_stat->atimensec);
	if (res < 0)
	{
		return -errno;
	}

	res = dprintf(file_fd, "mtimensec=%" PRId64 "\n", my_stat->mtimensec);
	if (res < 0)
	{
		return -errno;
	}

	res = dprintf(file_fd, "ctimensec=%" PRId64 "\n", my_stat->ctimensec);
	if (res < 0)
	{
		return -errno;
	}

	res = dprintf(file_fd, "nlink=%" PRIu64 "\n", my_stat->nlink);
	if (res < 0)
	{
		return -errno;
	}

	res = dprintf(file_fd, "blksize=%" PRId64 "\n", my_stat->blksize);
	if (res < 0)
	{
		return -errno;
	}

	return 0;
}
