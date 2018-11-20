/** @file VIB.h
 *
 * VIB Library - Main Include File.
 * This will include both the VisualIB library, and, if in C++,
 * the VIB class library as well.
 * @author James Haley
 */
 
#ifndef VIB_H__
#define VIB_H__

// Include VisualIB

// Utilities
#include "vibdefines.h"     // Global defines
#include "vibtypes.h"       // Types
#include "viberror.h"       // Error handling
#include "vibstring.h"      // Strings

// Primary Classes
#include "vibdatabase.h"    // Databases
#include "vibtransaction.h" // Transactions
#include "vibdataset.h"     // Data Sets
#include "vibsql.h"         // SQL Queries

#if defined(__cplusplus) && !defined(VIB_NO_CLASSES)

// VIB Class Library
#include "VIB/classVIBError.h"       // Exceptions
#include "VIB/classVIBDatabase.h"    // Databases
#include "VIB/classVIBTransaction.h" // Transactions
#include "VIB/classVIBDataSet.h"     // Data Sets
#include "VIB/classVIBSQL.h"         // SQL Queries

#endif

#endif

// EOF

