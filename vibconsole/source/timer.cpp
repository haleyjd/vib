/*

  Library / Platform Agnostic Timing

*/

#if !defined(VIBC_NO_WIN32)
/* Win32 Implementations */
#include <Windows.h>
#if !defined(VIBC_NO_MMSYSTEM)
/* Use MMSYSTEM timer if available */
#include <MMSystem.h>

unsigned int Timer_getMS()
{
   return timeGetTime();
}
#else
/* Use Win32 GetTickCount */
unsigned int Timer_getMS()
{
   return GetTickCount();
}
#endif
#elif !defined(VIBC_NO_SDL)
/* Use SDL Timer if available */
#include "SDL.h"

unsigned int Timer_getMS()
{
   return SDL_GetTicks();
}
#else
/* No implementation provided, error */
#error Need an implementation for Timer_getMS()!
#endif

// EOF

