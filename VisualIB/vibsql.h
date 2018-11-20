/** @file vibsql.h
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * TIBSQL Wrapper
 * @author James Haley
 *
 */

#ifndef VIBSQL_H__
#define VIBSQL_H__

#include "vibdefines.h"
#include "vibtypes.h"
#include "vibdatabase.h"
#include "vibtransaction.h"
#include "vibstring.h"

/**
 * Wrapper for a TIBSQL instance. 
 * Allocate using VIBSQL_New and release with VIBSQL_Destroy.
 */
struct VIBSQL
{
   VIBBOOL  isWeak; //!< If VIBTRUE, the VIBSQL instance does not own the wrapped TIBSQL.
   void    *opaque; //!< Opaque pointer to the TIBSQL instance.
};

#ifdef __cplusplus
extern "C" {
#endif

/** Constructor */
VIBDLLFUNC VIBSQL *        VIBCALL VIBSQL_New();
/** Destructor */
VIBDLLFUNC void            VIBCALL VIBSQL_Destroy(VIBSQL *vsql);
/** Execute an InterBase API call */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_Call(VIBSQL *vsql, int ErrCode, VIBBOOL RaiseError, int *ret);
/** Check if is closed */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_CheckClosed(VIBSQL *vsql);
/** Check if is open */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_CheckOpen(VIBSQL *vsql);
/** Check if statement is valid */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_CheckValidStatement(VIBSQL *vsql);
/** Close the SQL query object */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_Close(VIBSQL *vsql);
/** Execute the query */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_ExecQuery(VIBSQL *vsql);
/** Free the query's InterBase handle */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_FreeHandle(VIBSQL *vsql);
/** Prepare the query; a plan can be obtained afterward. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_Prepare(VIBSQL *vsql);
/** Get the query's unique relation name */
VIBDLLFUNC VIBString *     VIBCALL VIBSQL_GetUniqueRelationName(VIBSQL *vsql);
/** Test for BOF */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_Bof(VIBSQL *vsql);
/** Obtain the InterBase DB handle */
VIBDLLFUNC void ***        VIBCALL VIBSQL_DBHandle(VIBSQL *vsql);
/** Test for EOF */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_Eof(VIBSQL *vsql);
/** Get an index for a fieldname */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_FieldIndex(VIBSQL *vsql, const char *FieldName, int *ret);
/** Open the SQL query */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_Open(VIBSQL *vsql);
/** Find a SQL parameter by name as set its IsNull property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_Params_ByName_SetIsNull(VIBSQL *vsql, const char *name, VIBBOOL IsNull);
/** Find a SQL parameter by name and set its value using AsString. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_Params_ByName_SetAsString(VIBSQL *vsql, const char *name, const char *str);
/** Find a SQL parameter by name and retrieve its InterBase SQL type. */
VIBDLLFUNC int             VIBCALL VIBSQL_Params_ByName_GetSQLType(VIBSQL *vsql, const char *name);
/** 
 * Obtain a plan for the SQL query.
 * @pre The query must be prepared first.
 */
VIBDLLFUNC VIBString *     VIBCALL VIBSQL_Plan(VIBSQL *vsql);
/** Test if the query has been prepared. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_Prepared(VIBSQL *vsql);
/** Get the number of records returned by this query */
VIBDLLFUNC int             VIBCALL VIBSQL_RecordCount(VIBSQL *vsql);
/** Get the number of database table rows affected by this query. */
VIBDLLFUNC int             VIBCALL VIBSQL_RowsAffected(VIBSQL *vsql);
/** Obtain the type of SQL query. */
VIBDLLFUNC VIBSQLTypes     VIBCALL VIBSQL_SQLType(VIBSQL *vsql);
/** Obtain the InterBase TR handle for the query. */
VIBDLLFUNC void ***        VIBCALL VIBSQL_TRHandle(VIBSQL *vsql);
/** Obtain the InterBase handle for the query. */
VIBDLLFUNC void **         VIBCALL VIBSQL_Handle(VIBSQL *vsql);
/** Get the GenerateParamNames property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_GetGenerateParamNames(VIBSQL *vsql);
/** Set the GenerateParamNames property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_SetGenerateParamNames(VIBSQL *vsql, VIBBOOL GenerateParamNames);
/** Get the unique relation name via the TIBSQL::UniqueRelationName property. */
VIBDLLFUNC VIBString *     VIBCALL VIBSQL_UniqueRelationName(VIBSQL *vsql);
/** 
 * Obtain the database this query is targeting. 
 * @warning You must destroy the VIBDatabase instance when you are finished with it.
 */
VIBDLLFUNC VIBDatabase *   VIBCALL VIBSQL_GetDatabase(VIBSQL *vsql);
/** Set the database for this query. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_SetDatabase(VIBSQL *vsql, VIBDatabase *Database);
/** Get the GoToFirstRecordOnExecute property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_GetGoToFirstRecordOnExecute(VIBSQL *vsql);
/** Set the GoToFirstRecordOnExecute property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_SetGoToFirstRecordOnExecute(VIBSQL *vsql, VIBBOOL GoToFirstRecordOnExecute);
/** Get the ParamCheck property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_GetParamCheck(VIBSQL *vsql);
/** Set the ParamCheck property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_SetParamCheck(VIBSQL *vsql, VIBBOOL ParamCheck);
/** Set the SQL Text property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_SQL_SetText(VIBSQL *vsql, const char *Text);
/** 
 * Get the SQL Text property. 
 * @warning You must destroy the VIBString instanced when you are finished with it.
 */
VIBDLLFUNC VIBString *     VIBCALL VIBSQL_SQL_GetText(VIBSQL *vsql);
/** 
 * Get this query's transaction.
 * @warning You must destroy the VIBTransaction when you are finished with it. 
 */
VIBDLLFUNC VIBTransaction *VIBCALL VIBSQL_GetTransaction(VIBSQL *vsql);
/** Set this query's transaction. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBSQL_SetTransaction(VIBSQL *vsql, VIBTransaction *Transaction);

#ifdef __cplusplus
}
#endif

// TIBXSQLVAR type values, from the InterBase API
#define VIB_SQL_VARYING   0x1c0
#define VIB_SQL_TEXT      0x1c4
#define VIB_SQL_DOUBLE    0x1e0
#define VIB_SQL_FLOAT     0x1e2
#define VIB_SQL_LONG      0x1f0
#define VIB_SQL_SHORT     0x1f4
#define VIB_SQL_TIMESTAMP 0x1fe
#define VIB_SQL_BLOB      0x208
#define VIB_SQL_D_FLOAT   0x212
#define VIB_SQL_ARRAY     0x21c
#define VIB_SQL_QUAD      0x226
#define VIB_SQL_TYPE_TIME 0x230
#define VIB_SQL_TYPE_DATE 0x23a
#define VIB_SQL_INT64     0x244
#define VIB_SQL_DATE      0x1fe

#endif

// EOF

