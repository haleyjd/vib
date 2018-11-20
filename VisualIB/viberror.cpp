/** @file viberror.cpp
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * Error Handling and Propagation
 * @author James Haley
 *
 */

#include <cstdlib>
#include "vibdefines.h"
#include "viberror.h"

/** Tracking data for the last error to occur. */
struct VIBError
{
   char *Message;
   int   IBErrorCode;
   int   SQLCode;
};

static VIBError lastError;

//
// VIBError_GetLastError
//
void VIBCALL VIBError_GetLastError(const char **message, int *iberror, int *sqlcode)
{
   try
   {
      if(lastError.Message)
         *message = lastError.Message;
      else
         *message = "";

      *iberror = lastError.IBErrorCode;
      *sqlcode = lastError.SQLCode;
   }
   catch(...)
   {
   }
}

extern char *cpp_strdup(const char *input);

/**
 * Internal function. Saves the current error.
 */
void VIBCALL VIBError_Report(const char *message, int iberror, int sqlcode)
{
   try
   {
      if(lastError.Message)
         delete [] lastError.Message;
      lastError.Message     = cpp_strdup(message);
      lastError.IBErrorCode = iberror;
      lastError.SQLCode     = sqlcode;
   }
   catch(...)
   {
   }
}

// EOF

