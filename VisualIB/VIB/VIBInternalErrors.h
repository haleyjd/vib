/** @file VIBInternalErrors.h
 *
 *  VIB Library - Internal error handling functions.
 *  Not for inclusion into user code.
 *  @author James Haley
 */

#ifndef VIBINTERNALERRORS_H__
#define VIBINTERNALERRORS_H__

#include "../viberror.h"
#include "classVIBError.h"

static inline void TestForVIBError(bool result)
{
   if(!result)
   {
      const char *msg;
      int IBErrorCode;
      int SQLCode;

      VIBError_GetLastError(&msg, &IBErrorCode, &SQLCode);

      if(IBErrorCode == 0 && SQLCode == 0) // internal error
         throw VIB::Error(msg);
      else
         throw VIB::IBError(msg, IBErrorCode, SQLCode);
   }
}

#define VIBSAFECALL(func) TestForVIBError(!!(func))

#endif

// EOF

