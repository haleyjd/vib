/**
 * Miscellaneous Utilities
 */

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

using namespace std;

//
// ReportError
//
// Reports an error and asks the user if they want to continue execution or not.
//
bool ReportError(const char *msg, ...)
{
   va_list args;

   va_start(args, msg);

   printf("Error: ");
   vprintf(msg, args);
   printf("\nDo you want to continue? ('n' to quit): ");

   va_end(args);

   char c = getc(stdin);

   return !(c == 'n' || c == 'N');
}

//
// FatalError
//
void FatalError(const char *msg, ...)
{
   va_list args;

   va_start(args, msg);

   printf("Error: ");
   vprintf(msg, args);
   putc('\n', stdout);
   getc(stdin);
   exit(-1);

   va_end(args);
}

// EOF

