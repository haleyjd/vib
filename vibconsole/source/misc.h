/** @file misc.h
 * Miscellaneous Utilities
 */

#ifndef MISC_H__
#define MISC_H__

/**
 * Reports an error and asks the user if they want to continue execution or not.
 * @param msg printf-style format string for message.
 * @return True if user wants to continue, false if user wants to stop.
 */
bool ReportError(const char *msg, ...);

/** 
 * Report a fatal error condition and terminate program execution.
 * @param msg printf-style format string for message.
 * @return Function does not return.
 */
void FatalError(const char *msg, ...);

#endif

// EOF

