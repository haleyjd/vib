/** @file vibdefines.h
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * Globally required macros.
 * @author James Haley
 *
 */

#ifndef VIBDEFINES_H__
#define VIBDEFINES_H__

#ifdef VIB_EXPORTING
/** 
 * VIBDLLFUNC is defined as __declspec(dllexport), to determine DLL exports, 
 * when building the DLL inside C++Builder. VIB_EXPORTING determines which 
 * definition of the macro is used, and it should only be defined when building
 * a DLL, <b>not</b> inside user projects.
 */
#define VIBDLLFUNC __declspec(dllexport)
#else
/** 
 * VIBDLLFUNC is defined as __declspec(dllexport), to determine DLL exports, 
 * when building the DLL inside C++Builder. VIB_EXPORTING determines which 
 * definition of the macro is used, and it should only be defined when building
 * a DLL, <b>not</b> inside user projects.
 */
#define VIBDLLFUNC 
#endif

/**
 * VIBCALL defines the calling convention for all VisualIB entry points.
 */
#define VIBCALL __cdecl

#endif

// EOF

