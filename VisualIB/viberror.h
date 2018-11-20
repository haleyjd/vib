/** @file viberror.h
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * Error Handling and Propagation
 * @author James Haley
 *
 */

#ifndef VIBERROR_H__
#define VIBERROR_H__

#include "vibdefines.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * We can't throw exceptions directly out of the library, as I want it to be as
 * widely usable as possible. Instead, routines that intercept an EIBError or
 * EIBInterBaseError will report it to this module, which can be queried
 * externally when a routine gives an unexpected return value.
 * @param [inout] message Receives the last error message, if any; "" if not.
 * @param [inout] iberror Receives the last IBErrorCode
 * @param [inout] sqlcode Receives the last SQLCode
 * @warning All three parameters must be non-NULL.
 */
VIBDLLFUNC void VIBCALL VIBError_GetLastError(const char **message, int *iberror, int *sqlcode);

#ifdef __cplusplus
}
#endif

#endif

// EOF

