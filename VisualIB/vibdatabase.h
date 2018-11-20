/** @file vibdatabase.h
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * TIBDatabase Wrapper
 * @author James Haley
 *
 */

#ifndef VIBDATABASE_H__
#define VIBDATABASE_H__

#include "vibdefines.h"
#include "vibtypes.h"
#include "vibstring.h"

/**
 * Wrapper for TIBDatabase.
 * Allocate with VIBDatabase_New and destroy with VIBDatabase_Destroy.
 */
struct VIBDatabase
{
   VIBBOOL  isWeak; //!< If VIBTRUE, the VIBDatabase does not own the TIBDatabase instance.
   void    *opaque; //!< Opaque pointer to the TIBDatabase.
};

#include "vibtransaction.h"
#include "vibdataset.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Constructor */
VIBDLLFUNC VIBDatabase *   VIBCALL VIBDatabase_New();
/** Destructor */
VIBDLLFUNC void            VIBCALL VIBDatabase_Destroy(VIBDatabase *vdb);
/** Close all datasets. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_CloseDataSets(VIBDatabase *vdb);
/** Check if the database is active. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_CheckActive(VIBDatabase *vdb);
/** Check if the database is inactive. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_CheckInactive(VIBDatabase *vdb);
/** Create a database. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_CreateDatabase(VIBDatabase *vdb);
/** Drop a database. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_DropDatabase(VIBDatabase *vdb);
/** Force-close the connection. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_ForceClose(VIBDatabase *vdb);
/** Get the index of a DBConst. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_IndexOfDBConst(VIBDatabase *vdb, const char *st, int *ret);
/** Test the database connection state. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_TestConnected(VIBDatabase *vdb);
/** Check if the database name is defined. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_CheckDatabaseName(VIBDatabase *vdb);
/** Execute an InterBase API call. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_Call(VIBDatabase *vdb, int ErrCode, VIBBOOL RaiseError, int *ret);
/** Add a transaction to this database. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_AddTransaction(VIBDatabase *vdb, VIBTransaction *vtr, int *ret);
/** Check if the transaction is associated with this database. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_FindTransaction(VIBDatabase *vdb, VIBTransaction *vtr, int *ret);
/**
 * Get the default transaction instance for this database.
 * @warning You must destroy the VIBTransaction instance when you are finished with it.
 */
VIBDLLFUNC VIBTransaction *VIBCALL VIBDatabase_FindDefaultTransaction(VIBDatabase *vdb);
/** Remove a transaction from this database. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_RemoveTransaction(VIBDatabase *vdb, int Idx);
/** Remove all transactions from this database. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_RemoveTransactions(VIBDatabase *vdb);
/** Set the database's InterBase API handle. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_SetHandle(VIBDatabase *vdb, void **Value);
/** Obtain the database's InterBase API handle. */
VIBDLLFUNC void **         VIBCALL VIBDatabase_Handle(VIBDatabase *vdb);
/** Check the IsReadyOnly property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_IsReadOnly(VIBDatabase *vdb);
/**
 * Get a DBParam for a DPB value. 
 * @warning You must destroy the VIBString instance when you are finished with it. 
 */
VIBDLLFUNC VIBString *     VIBCALL VIBDatabase_DBParamByDPB(VIBDatabase *vdb, int Idx);
/** Get the count of SQLObjects attached to the database. */
VIBDLLFUNC int             VIBCALL VIBDatabase_SQLObjectCount(VIBDatabase *vdb);
/** Test if the database handle is shared. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_HandleIsShared(VIBDatabase *vdb);
/** Get a count of transactions attached to the database. */
VIBDLLFUNC int             VIBCALL VIBDatabase_TransactionCount(VIBDatabase *vdb);
/**
 * Get a transaction attached to the database by index.
 * @warning You must destroy the VIBTransaction instance when you are finished with it.
 */
VIBDLLFUNC VIBTransaction *VIBCALL VIBDatabase_Transactions(VIBDatabase *vdb, int Index);
/**
 * Get the internal transaction instance for the database.
 * @warning You must destroy the VIBTransaction instance when you are finished with it.
 */
VIBDLLFUNC VIBTransaction *VIBCALL VIBDatabase_InternalTransaction(VIBDatabase *vdb);
/** Get the DEFAULT_VALUE property of the indicated relation. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_Has_DEFAULT_VALUE(VIBDatabase *vdb, const char *Relation, const char *Field);
/** Get the COMPUTED_BLR property of the indicated relation. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_Has_COMPUTED_BLR(VIBDatabase *vdb, const char *Relation, const char *Field);
/** Flush the database schema. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_FlushSchema(VIBDatabase *vdb);
/**
 * Get the database name property.
 * @warning You must destroy the VIBString instance when you are finished with it.
 */
VIBDLLFUNC VIBString *     VIBCALL VIBDatabase_GetDatabaseName(VIBDatabase *vdb);
/** Set the database name property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_SetDatabaseName(VIBDatabase *vdb, const char *name);
/** Add a parameter to the database. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_Params_Add(VIBDatabase *vdb, const char *param);
/** Get the LoginPrompt property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_GetLoginPrompt(VIBDatabase *vdb);
/** Set the LoginPrompt property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_SetLoginPrompt(VIBDatabase *vdb, VIBBOOL LoginPrompt);
/**
 * Get the default transaction attached to this database.
 * @warning You must destroy the VIBTransaction instance when you are finished with it.
 */
VIBDLLFUNC VIBTransaction *VIBCALL VIBDatabase_GetDefaultTransaction(VIBDatabase *vdb);
/** Set the database's default transaction. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_SetDefaultTransaction(VIBDatabase *vdb, VIBTransaction *vtr);
/** Get the IdleTimer property. */
VIBDLLFUNC int             VIBCALL VIBDatabase_GetIdleTimer(VIBDatabase *vdb);
/** Set the IdleTimer property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_SetIdleTimer(VIBDatabase *vdb, int IdleTimer);
/** Get the SQLDialict property. */
VIBDLLFUNC int             VIBCALL VIBDatabase_GetSQLDialect(VIBDatabase *vdb);
/** Set the SQLDialect property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_SetSQLDialect(VIBDatabase *vdb, int SQLDialect);
/** Get the DBSQLDialect property. */
VIBDLLFUNC int             VIBCALL VIBDatabase_DBSQLDialect(VIBDatabase *vdb);
/** Get the database trace flags. */
VIBDLLFUNC VIBTraceFlags   VIBCALL VIBDatabase_GetTraceFlags(VIBDatabase *vdb);
/** Set the database trace flags. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_SetTraceFlags(VIBDatabase *vdb, VIBTraceFlags flags);
/** Get the AllowStreamedConnected property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_GetAllowStreamedConnected(VIBDatabase *vdb);
/** Set the AllowStreamedConnected property. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_SetAllowStreamedConnected(VIBDatabase *vdb, VIBBOOL AllowStreamedConnected);
/** Open the database. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_Open(VIBDatabase *vdb);
/** Close the database. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_Close(VIBDatabase *vdb);
/** Get the connected status of the database. */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_GetConnected(VIBDatabase *vdb);
/** Set the connected status of the database (connects if set to VIBTRUE and not conected). */
VIBDLLFUNC VIBBOOL         VIBCALL VIBDatabase_SetConnected(VIBDatabase *vdb, VIBBOOL Connected);
/** 
 * Get a pointer to a specific dataset associated with this database.
 * @warning You must destroy the VIBDataSet instance when you are finished with it.
 */
VIBDLLFUNC VIBDataSet *    VIBCALL VIBDatabase_DataSets(VIBDatabase *vdb, int Index);
/** Get a count of datasets attached to this database. */
VIBDLLFUNC int             VIBCALL VIBDatabase_DataSetCount(VIBDatabase *vdb);

#ifdef __cplusplus
}
#endif

#endif

// EOF

