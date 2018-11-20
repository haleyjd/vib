/** @file vibtransaction.h
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * TIBTransaction Wrapper
 * @author James Haley
 *
 */

#ifndef VIBTRANSACTION_H__
#define VIBTRANSACTION_H__

#include "vibdefines.h"
#include "vibtypes.h"

/**
 * Wrapper for a TIBTransaction.
 */
struct VIBTransaction
{
   VIBBOOL  isWeak; //!< If VIBTRUE, the VIBTransaction does not own the TIBTransaction instance.
   void    *opaque; //!< Opaque pointer to the TIBTransaction.
};

#include "vibdatabase.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Constructor */
VIBDLLFUNC VIBTransaction *     VIBCALL VIBTransaction_New();
/** Destructor */
VIBDLLFUNC void                 VIBCALL VIBTransaction_Destroy(VIBTransaction *vtr);
/** Execute an InterBase API call */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_Call(VIBTransaction *vtr, int ErrCode, VIBBOOL RaiseError, int *ret);
/** Commit and close the transaction. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_Commit(VIBTransaction *vtr);
/** Commit the transaction and retain its active state. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_CommitRetaining(VIBTransaction *vtr);
/** Rollback and close the transaction. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_Rollback(VIBTransaction *vtr);
/** Rollback the transaction and retain its active state. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_RollbackRetaining(VIBTransaction *vtr);
/** Start the transaction. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_StartTransaction(VIBTransaction *vtr);
/** Check if the transaction is active. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_CheckInTransaction(VIBTransaction *vtr);
/** Check if the transaction is not active. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_CheckNotInTransaction(VIBTransaction *vtr);
/** Check the AutoStop property. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_CheckAutoStop(VIBTransaction *vtr);
/** Add a database to this transaction. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_AddDatabase(VIBTransaction *vtr, VIBDatabase *vdb, int *ret);
/** Check if the database is related to this transaction. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_FindDatabase(VIBTransaction *vtr, VIBDatabase *vdb, int *ret);
/**
 * Obtain the default database property of this transaction.
 * @warning You must destroy the VIBDatabase instance when you are finished with it.
 */
VIBDLLFUNC VIBDatabase *        VIBCALL VIBTransaction_FindDefaultDatabase(VIBTransaction *vtr);
/** Remove a database from the transaction. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_RemoveDatabase(VIBTransaction *vtr, int Idx);
/** Remove all databases from the transaction. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_RemoveDatabases(VIBTransaction *vtr);
/** Check if there are databases attached to the transaction. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_CheckDatabasesInList(VIBTransaction *vtr);
/** Obtain a count of the databases related to this transaction. */
VIBDLLFUNC int                  VIBCALL VIBTransaction_DatabaseCount(VIBTransaction *vtr);
/** 
 * Obtain a pointer to the Nth database associated with the transaction.
 * @warning You must destroy the VIBDatabase instance when you are finished with it.
 */
VIBDLLFUNC VIBDatabase *        VIBCALL VIBTransaction_Databases(VIBTransaction *vtr, int Index);
/* Obtain a count of SQLObjects attached to the transaction. */
VIBDLLFUNC int                  VIBCALL VIBTransaction_SQLObjectCount(VIBTransaction *vtr);
/** Obtain the transaction's handle. */
VIBDLLFUNC void *               VIBCALL VIBTransaction_Handle(VIBTransaction *vtr);
/** Test if the handle is shared. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_HandleIsShared(VIBTransaction *vtr);
/** Test if the transaction is running. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_InTransaction(VIBTransaction *vtr);
/** Get the TPB property. */
VIBDLLFUNC char *               VIBCALL VIBTransaction_TPB(VIBTransaction *vtr);
/** Get the length of the TPB property. */
VIBDLLFUNC short                VIBCALL VIBTransaction_TPBLength(VIBTransaction *vtr);
/** Get the active state of the transaction. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_GetActive(VIBTransaction *vtr);
/** Set the active state of the transaction. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_SetActive(VIBTransaction *vtr, VIBBOOL Active);
/** 
 * Get the transaction's default database.
 * @warning You must destroy the VIBDatabase instance when you are finished with it.
 */
VIBDLLFUNC VIBDatabase *        VIBCALL VIBTransaction_GetDefaultDatabase(VIBTransaction *vtr);
/** Set the transaction's default database. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_SetDefaultDatabase(VIBTransaction *vtr, VIBDatabase *DefaultDatabase);
/** Get the IdleTimer property. */
VIBDLLFUNC int                  VIBCALL VIBTransaction_GetIdleTimer(VIBTransaction *vtr);
/** Set the IdleTimer property. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_SetIdleTimer(VIBTransaction *vtr, int IdleTimer);
/** Get the transaction's default action. */
VIBDLLFUNC VIBTransactionAction VIBCALL VIBTransaction_GetDefaultAction(VIBTransaction *vtr);
/** Set the transaction's default action. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_SetDefaultAction(VIBTransaction *vtr, VIBTransactionAction action);
/** Add a parameter to the transaction. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_Params_Add(VIBTransaction *vtr, const char *Param);
/** Get the transaction's autostop action. */
VIBDLLFUNC VIBAutoStopAction    VIBCALL VIBTransaction_GetAutoStopAction(VIBTransaction *vtr);
/** Set the transaction's autostop action. */
VIBDLLFUNC VIBBOOL              VIBCALL VIBTransaction_SetAutoStopAction(VIBTransaction *vtr, VIBAutoStopAction action);

#ifdef __cplusplus
}
#endif

#endif

// EOF

