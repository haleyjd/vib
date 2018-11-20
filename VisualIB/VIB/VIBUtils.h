/** @file VIBUtils.h
 *
 * VIB Class Library
 * Internal utilities
 * @author James Haley
 *
 * @warning This code is for internal library use; do not include in user code.
 */

#ifndef VIBUTILS_H__
#define VIBUTILS_H__

#include <string>
#include "../vibstring.h"
#include "classVIBError.h"

static inline std::string ElideVIBString(VIBString *vstr)
{
   std::string ret;
   
   if(!vstr)
      throw VIB::Error("Failed to get string property value");

   ret = VIBString_CStr(vstr);
   VIBString_Destroy(vstr);
   return ret;
}

#endif

// EOF

