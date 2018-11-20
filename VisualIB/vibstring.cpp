/** @file vibstring.cpp
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * Lightweight string object
 * @author James Haley
 *
 */

#include <cstdlib>
#include <cstring>
#include "vibstring.h"

/**
 * Internal function.
 * A strdup substitute that uses new [] to do allocation.
 */
char *cpp_strdup(const char *input)
{
   std::size_t len = std::strlen(input) + 1;

   char *buffer = new char [len];

   if(buffer)
   {
      std::memset(buffer, 0, len);
      std::strcpy(buffer, input);
   }

   return buffer;
}

//
// VIBString_New
//
VIBString *VIBCALL VIBString_New(const char *str)
{
   VIBString *newstr = NULL; 
   
   try
   {
      newstr = new VIBString;
      newstr->str = cpp_strdup(str);
   }
   catch(...)
   {
      // cpp_strdup failed?
      if(newstr)
         delete newstr;
      newstr = NULL;
   }

   return newstr;
}

//
// VIBString_Destroy
//
void VIBCALL VIBString_Destroy(VIBString *str)
{
   try
   {
      if(str->str)
         delete [] str->str;
      str->str = NULL;
      delete str;
   }
   catch(...)
   {
   }
}

//
// VIBString_CStr
//
const char *VIBCALL VIBString_CStr(VIBString *str)
{
   return str->str;
}


// EOF

