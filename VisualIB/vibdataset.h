/** @file vibdataset.h
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * TIBDataSet Wrapper
 * @author James Haley
 *
 */

#ifndef VIBDATASET_H__
#define VIBDATASET_H__

#include "vibdefines.h"
#include "vibtypes.h"
#include "vibdatabase.h"
#include "vibtransaction.h"
#include "vibstring.h"

/**
 * Wrapper for TIBDataSet.
 * Allocate using VIBDataSet_New, and release with VIBDataSet_Destroy.
 */
struct VIBDataSet
{
   VIBBOOL  isWeak; //!< If true, the VIBDataSet does not own its TIBDataSet.
   void    *opaque; //!< Opaque pointer to TIBDataSet instance
};

#ifdef __cplusplus
extern "C" {
#endif

/** Constructor */
VIBDLLFUNC VIBDataSet *    VIBCALL VIBDataSet_New();
/** Destructor */
VIBDLLFUNC void            VIBCALL VIBDataSet_Destroy(VIBDataSet *vds);
/** Set the VIBDatabase instance for the VIBDataSet */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDataSet_SetDatabase(VIBDataSet *vds, VIBDatabase *vdb);
/** 
 * Retrieve the VIBDatabase instance for the VIBDataSet.
 * @warning You must destroy the VIBDatabase when you are finished with it.
 */
VIBDLLFUNC VIBDatabase *   VIBCALL VIBDataSet_GetDatabase(VIBDataSet *vds);
/** Set the VIBTransaction instance for the VIBDataSet */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDataSet_SetTransaction(VIBDataSet *vds, VIBTransaction *vtr);
/**
 * Retrieve the VIBTransaction instance for the VIBDataSet.
 * @warning You must destroy the VIBTransaction when you are finished with it.
 */
VIBDLLFUNC VIBTransaction *VIBCALL VIBDataSet_GetTransaction(VIBDataSet *vds);
/** Set the UniDirectional property */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDataSet_SetUniDirectional(VIBDataSet *vds, VIBBOOL UniDirectional);
/** Get the UniDirectional property */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDataSet_GetUniDirectional(VIBDataSet *vds);
/** Clear the SelectSQL statement list */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDataSet_SelectSQL_Clear(VIBDataSet *vds);
/** Add a string to the SelectSQL statement list */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDataSet_SelectSQL_Add(VIBDataSet *vds, const char *sql);
/** 
 * Obtain all SQL statements as a single string 
 * @warning You must destroy the VIBString instance when you are finished with it.
 */
VIBDLLFUNC VIBString *     VIBCALL VIBDataSet_SelectSQL_GetText(VIBDataSet *vds);
/** Set the text of the SelectSQL list to a single string.*/
VIBDLLFUNC VIBBOOL         VIBCALL VIBDataSet_SelectSQL_SetText(VIBDataSet *vds, const char *sql);
/** Check if the dataset is at the end of its set of records */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDataSet_Eof(VIBDataSet *vds);
/** Open the dataset by executing the SelectSQL statement */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDataSet_Open(VIBDataSet *vds);
/** Place the dataset cursor at the first record */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDataSet_First(VIBDataSet *vds);
/** Step to the next record in the dataset */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDataSet_Next(VIBDataSet *vds);
/** Close the dataset */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDataSet_Close(VIBDataSet *vds);
/** Prepare the dataset's SelectSQL statement but do not execute it. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDataSet_Prepare(VIBDataSet *vds);
/** 
 * Obtain a plan for the SelectSQL statement.
 * @pre The dataset's query must be prepared first.
 */
VIBDLLFUNC VIBString *     VIBCALL VIBDataSet_Plan(VIBDataSet *vds);
/** Get a count of the fields according to the TFields subobject. */
VIBDLLFUNC int             VIBCALL VIBDataSet_Fields_Count(VIBDataSet *vds);
/** 
 * Get the value of a specific field as a string. 
 * @warning You must destroy the VIBString instance when you are finished with it.
 */
VIBDLLFUNC VIBString *     VIBCALL VIBDataSet_Fields_AsString(VIBDataSet *vds, int Index);
/**
 * Get the name of a specific field as a string.
 * @warning You must destroy the VIBString instance when you are finished with it.
 */
VIBDLLFUNC VIBString *     VIBCALL VIBDataSet_Fields_FieldName(VIBDataSet *vds, int Index);
/**
 * Get the value of a specific field, found by field name, as a string.
 * @warning You must destroy the VIBString instance when you are finished with it.
 */
VIBDLLFUNC VIBString *     VIBCALL VIBDataSet_FieldByName_AsString(VIBDataSet *vds, const char *fieldname);
/** Get a count of the fields according to the TIBDataSet itself. */
VIBDLLFUNC int             VIBCALL VIBDataSet_FieldCount(VIBDataSet *vds);
/** Get the type of a specific field in the data set. */
VIBDLLFUNC VIBFieldType    VIBCALL VIBDataSet_Fields_DataType(VIBDataSet *vds, int Idx);
/** Get the kind of a specific field in the data set. */
VIBDLLFUNC VIBFieldKind    VIBCALL VIBDataSet_Fields_FieldKind(VIBDataSet *vds, int Idx);

#ifdef __cplusplus
}
#endif

#endif

// EOF

