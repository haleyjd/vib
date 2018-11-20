/** @file vibexception.h
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * Internal Exception Utilities
 * @author James Haley
 *
 * @warning Internal only. Do not include in user programs!
 */

#ifndef VIBEXCEPTION_H__
#define VIBEXCEPTION_H__

__declspec(dllexport) void __cdecl VIBError_Report(const char *message, int iberror, int sqlcode);

// For just catching an error
#define CATCH_EIBERROR                       \
   catch(EIBError &error)                    \
   {                                         \
      VIBError_Report(error.Message.c_str(), \
                      error.IBErrorCode,     \
                      error.SQLCode);        \
   }                                         \
   catch(...)                                \
   {                                         \
      VIBError_Report("Internal", 0, 0);     \
   }

// Catch error and return VIBFALSE
#define CATCH_EIBERROR_RF                    \
   catch(EIBError &error)                    \
   {                                         \
      VIBError_Report(error.Message.c_str(), \
                      error.IBErrorCode,     \
                      error.SQLCode);        \
      return VIBFALSE;                       \
   }                                         \
   catch(...)                                \
   {                                         \
      VIBError_Report("Internal", 0, 0);     \
      return VIBFALSE;                       \
   }


#endif

// EOF

