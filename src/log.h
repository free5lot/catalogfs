#ifndef INC_CATALOGFS_LOG_H
#define INC_CATALOGFS_LOG_H

#include "header_common.h"

/** 
 * Log a formatted message to a file (if needed)
 * 
 * @param fp is a log file to use (can be NULL for disabled logging)
 * @param is_error means the message should be considered and marked as error
 * @param func_name is the calling function name
 * @param path is a path of processing file (can be NULL if not applicable)
 * @param format is a format string for the rest of the arguments
 */
void Log(FILE *fp, bool is_error, const char *const func_name, const char *const path, const char *const format, ...);

/** 
 * Log the start of some function to a file (if needed)
 * 
 * @param fp is the log file to use (can be NULL for disabled logging)
 * @param func_name is the calling function name
 * @param path is the path of processing file (can be NULL if not applicable)
 */
void LogStart(FILE *fp, const char *const func_name, const char *const path);

/** 
 * Log a success return code to a file (if needed)
 * 
 * @param fp is a log file to use (can be NULL for disabled logging)
 * @param func_name is the calling function name
 * @param path is a path of processing file (can be NULL if not applicable)
 * @param code is return code of the function (0 value usually)
 */
void LogReturnCodeOK(FILE *fp, const char *const func_name, const char *const path, int code);

/** 
 * Log an error return code to a file (if needed)
 * 
 * @param fp is a log file to use (can be NULL for disabled logging)
 * @param func_name is the calling function name
 * @param path is a path of processing file (can be NULL if not applicable)
 * @param code is return code of the function (nonzero value usually)
 */
void LogReturnCodeError(FILE *fp, const char *const func_name, const char *const path, int code);

/** 
 * Log the number of bytes processed to a file (if needed)
 * 
 * @param fp is a log file to use (can be NULL for disabled logging)
 * @param func_name is the calling function name
 * @param path is a path of processing file (can be NULL if not applicable)
 * @param bytes is number of bytes processed
 */
void LogReturnBytesCount(FILE *fp, const char *const func_name, const char *const path, int bytes);

/** 
 * Print a formatted message to stdout
 * 
 * @param format is a format for rest of the arguments
 */
void PrintToStdoutF(const char *const format, ...);

/** 
 * Print a message to stderr
 * 
 * @param message is a message to print
 */
void PrintToStdout(const char *const message);

/** 
 * Print a formatted message to stderr
 * 
 * @param format is a format for rest of the arguments
 */
void PrintToStderrF(const char *const format, ...);

/** 
 * Print a message to stderr
 * 
 * @param message is a message to print
 */
void PrintToStderr(const char *const message);

/**
 * Wrapper for start of function for logging purposes
 */
#define LOG_START(path)                                 \
	{                                                   \
		if (!MY_DATA->log_only_errors)                  \
		{                                               \
			LogStart(MY_DATA->logfile, __func__, path); \
		}                                               \
	}

/**
 * Wrapper for returning of code for logging purposes
 */
#define RETURN_CODE_OK(path, code)                                   \
	{                                                                \
		if (!MY_DATA->log_only_errors)                               \
		{                                                            \
			LogReturnCodeOK(MY_DATA->logfile, __func__, path, code); \
		}                                                            \
		return (code);                                               \
	}

/**
 * Wrapper for returning error with code for logging purposes
 */
#define RETURN_CODE_ERROR(path, code)                               \
	{                                                               \
		LogReturnCodeError(MY_DATA->logfile, __func__, path, code); \
		return (code);                                              \
	}

/**
 * Wrapper for returning of bytes count for logging purposes
 */
#define RETURN_BYTES_COUNT(path, bytes)                                   \
	{                                                                     \
		if (!MY_DATA->log_only_errors)                                    \
		{                                                                 \
			LogReturnBytesCount(MY_DATA->logfile, __func__, path, bytes); \
		}                                                                 \
		return (bytes);                                                   \
	}

#endif // INC_CATALOGFS_LOG_H
