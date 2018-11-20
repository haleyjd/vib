/** @file vibstring.h
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * Lightweight string object
 * @author James Haley
 *
 */

#ifndef VIBSTRING_H__
#define VIBSTRING_H__

#include "vibdefines.h"

/**
 * Lightweight string object used to elide AnsiString return values.
 * Allocate using VIBString_New and release with VIBString_Destroy.
 */
struct VIBString
{
   char *str; //!< Pointer to dynamically allocated null-terminated string.
};

#ifdef __cplusplus
extern "C" {
#endif

/** Constructor */
VIBDLLFUNC VIBString * VIBCALL VIBString_New(const char *str);
/** Destructor */
VIBDLLFUNC void        VIBCALL VIBString_Destroy(VIBString *str);
/** Accessor */
VIBDLLFUNC const char *VIBCALL VIBString_CStr(VIBString *str);

#ifdef __cplusplus
}
#endif

#endif

// EOF

