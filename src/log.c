#include "header_common.h"

#include <time.h>
#include <stdarg.h>

#include "log.h"

/** 
 * Log a formatted message to a file (if needed)
 * 
 * @param fp is a log file to use (can be NULL for disabled logging)
 * @param is_error means the message should be considered and marked as error
 * @param func_name is the calling function name
 * @param path is a path of processing file (can be NULL if not applicable)
 * @param format is a format string for the rest of the arguments
 */
void Log(FILE *fp, bool is_error, const char *const func_name, const char *const path, const char *const format, ...)
{
	if (!fp)
		return;

	time_t t = time(NULL);
	char timestr[256];
	strftime(timestr, sizeof(timestr), "%Y.%m.%d %H:%M:%S",
			 localtime(&t));

	char *error_str = (is_error) ? " [ERROR]" : "";
	fprintf(fp, "%s: %s%s: ", timestr, func_name, error_str);

	va_list args;
	va_start(args, format);
	vfprintf(fp, format, args);
	va_end(args);

	if (path != NULL)
	{
		fprintf(fp, " (path: %s)", path);
	}
	fprintf(fp, "\n");
	fflush(fp);
}

/** 
 * Log the start of some function to a file (if needed)
 * 
 * @param fp is a log file to use (can be NULL for disabled logging)
 * @param func_name is the calling function name
 * @param path is a path of processing file (can be NULL if not applicable)
 */
void LogStart(FILE *fp, const char *const func_name, const char *const path)
{
	Log(fp, false, func_name, path, "started");
}

/** 
 * Log a success return code to a file (if needed)
 * 
 * @param fp is a log file to use (can be NULL for disabled logging)
 * @param func_name is the calling function name
 * @param path is a path of processing file (can be NULL if not applicable)
 * @param code is return code of the function (0 value usually)
 */
void LogReturnCodeOK(FILE *fp, const char *const func_name, const char *const path, int code)
{
	Log(fp, false, func_name, path, "exited (code: %d)", code);
}

/** 
 * Log an error return code to a file (if needed)
 * 
 * @param fp is a log file to use (can be NULL for disabled logging)
 * @param func_name is the calling function name
 * @param path is a path of processing file (can be NULL if not applicable)
 * @param code is return code of the function (nonzero value usually)
 */
void LogReturnCodeError(FILE *fp, const char *const func_name, const char *const path, int code)
{
	// break point here for debug

	Log(fp, true, func_name, path, "exited (code: %d)", code);
}

/** 
 * Log the number of bytes processed to a file (if needed)
 * 
 * @param fp is a log file to use (can be NULL for disabled logging)
 * @param func_name is the calling function name
 * @param path is a path of processing file (can be NULL if not applicable)
 * @param bytes is number of bytes processed
 */
void LogReturnBytesCount(FILE *fp, const char *const func_name, const char *const path, int bytes)
{
	Log(fp, false, func_name, path, "exited (bytes processed: %d)", (bytes));
}

/** 
 * Print a formatted message to stdout
 * 
 * @param format is a format for rest of the arguments
 */
void PrintToStdoutF(const char *const format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);

	fprintf(stdout, "\n");
	fflush(stdout);
}

/** 
 * Print a message to stderr
 * 
 * @param message is a message to print
 */
void PrintToStdout(const char *const message)
{
	PrintToStdoutF("%s", message);
}

/** 
 * Print a formatted message to stderr
 * 
 * @param format is a format for rest of the arguments
 */
void PrintToStderrF(const char *const format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	fprintf(stderr, "\n");
	fflush(stderr);
}

/** 
 * Print a message to stderr
 * 
 * @param message is a message to print
 */
void PrintToStderr(const char *const message)
{
	PrintToStderrF("%s", message);
}
