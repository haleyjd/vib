/*

   jhaley - Configuration for jsapi
   
*/

#ifndef MYCONFIG_H__
#define MYCONFIG_H__

// Try masquerading as MS Visual Studio...
#if defined(__BORLANDC__) && !defined(_MSC_VER)
#define _MSC_VER 1400
#endif

#ifndef _WINDOWS
#define _WINDOWS
#endif

#ifndef _X86_
#define _X86_ 1
#endif

#ifndef JSFILE
#define JSFILE
#endif

#ifndef NDEBUG
#define NDEBUG
#endif

#undef _DEBUG
#undef DEBUG

#ifndef WIN32
#define WIN32
#endif

#ifndef XP_WIN
#define XP_WIN
#endif

#endif

// EOF

