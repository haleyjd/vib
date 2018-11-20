/** @file vibtransaction.cpp
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * TIBTransaction Wrapper
 * @author James Haley
 *
 */

#include "vibinlines.h"

#include "vibexception.h"

//
// VIBTransaction_New
//
VIBTransaction *VIBCALL VIBTransaction_New()
{
   TIBTransaction *ttr = NULL;
   VIBTransaction *vtr = NULL;

   try
   {
      ttr = new TIBTransaction(NULL);

      if(ttr)
      {
         vtr = new VIBTransaction;
         vtr->opaque = ttr;
         vtr->isWeak = VIBFALSE;
      }
   }
   CATCH_EIBERROR

   return vtr;
}

//
// VIBTransaction_Destroy
//
void VIBCALL VIBTransaction_Destroy(VIBTransaction *vtr)
{
   if(!vtr->isWeak)
   {
      try
      {
         TIBTransaction *ttr = TTRForVTR(vtr);
         delete ttr;
      }
      CATCH_EIBERROR
   }

   vtr->opaque = NULL;
   delete vtr;
}

//
// VIBTransaction_Call
//
VIBBOOL VIBCALL VIBTransaction_Call(VIBTransaction *vtr, int ErrCode, VIBBOOL RaiseError, int *ret)
{
   try
   {
      *ret = TTRForVTR(vtr)->Call(ErrCode, (RaiseError == VIBTRUE));
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_Commit
//
VIBBOOL VIBCALL VIBTransaction_Commit(VIBTransaction *vtr)
{
   try
   {
      TTRForVTR(vtr)->Commit();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_CommitRetaining
//
VIBBOOL VIBCALL VIBTransaction_CommitRetaining(VIBTransaction *vtr)
{
   try
   {
      TTRForVTR(vtr)->CommitRetaining();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_Rollback
//
VIBBOOL VIBCALL VIBTransaction_Rollback(VIBTransaction *vtr)
{
   try
   {
      TTRForVTR(vtr)->Rollback();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_RollbackRetaining
//
VIBBOOL VIBCALL VIBTransaction_RollbackRetaining(VIBTransaction *vtr)
{
   try
   {
      TTRForVTR(vtr)->RollbackRetaining();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_StartTransaction
//
VIBBOOL VIBCALL VIBTransaction_StartTransaction(VIBTransaction *vtr)
{
   try
   {
      TTRForVTR(vtr)->StartTransaction();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_CheckInTransaction
//
VIBBOOL VIBCALL VIBTransaction_CheckInTransaction(VIBTransaction *vtr)
{
   try
   {
      TTRForVTR(vtr)->CheckInTransaction();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_CheckNotInTransaction
//
VIBBOOL VIBCALL VIBTransaction_CheckNotInTransaction(VIBTransaction *vtr)
{
   try
   {
      TTRForVTR(vtr)->CheckNotInTransaction();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_CheckAutoStop
//
VIBBOOL VIBCALL VIBTransaction_CheckAutoStop(VIBTransaction *vtr)
{
   try
   {
      TTRForVTR(vtr)->CheckAutoStop();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_AddDatabase
//
VIBBOOL VIBCALL VIBTransaction_AddDatabase(VIBTransaction *vtr, VIBDatabase *vdb, int *ret)
{
   try
   {
      TIBTransaction *ttr = TTRForVTR(vtr);
      TIBDatabase    *tdb = TDBForVDB(vdb);

      *ret = ttr->AddDatabase(tdb);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_FindDatabase
//
VIBBOOL VIBCALL VIBTransaction_FindDatabase(VIBTransaction *vtr, VIBDatabase *vdb, int *ret)
{
   try
   {
      TIBTransaction *ttr = TTRForVTR(vtr);
      TIBDatabase    *tdb = TDBForVDB(vdb);

      *ret = ttr->FindDatabase(tdb);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_FindDefaultDatabase
//
VIBDatabase *VIBCALL VIBTransaction_FindDefaultDatabase(VIBTransaction *vtr)
{
   VIBDatabase *ret = NULL;

   try
   {
      TIBDatabase *tdb = TTRForVTR(vtr)->FindDefaultDatabase();

      if(tdb)
      {
         ret = new VIBDatabase;
         ret->opaque = tdb;
         ret->isWeak = VIBTRUE;
      }
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBTransaction_RemoveDatabase
//
VIBBOOL VIBCALL VIBTransaction_RemoveDatabase(VIBTransaction *vtr, int Idx)
{
   try
   {
      TTRForVTR(vtr)->RemoveDatabase(Idx);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_RemoveDatabases
//
VIBBOOL VIBCALL VIBTransaction_RemoveDatabases(VIBTransaction *vtr)
{
   try
   {
      TTRForVTR(vtr)->RemoveDatabases();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_CheckDatabasesInList
//
VIBBOOL VIBCALL VIBTransaction_CheckDatabasesInList(VIBTransaction *vtr)
{
   try
   {
      TTRForVTR(vtr)->CheckDatabasesInList();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_DatabaseCount
//
// Read-only property.
//
int VIBCALL VIBTransaction_DatabaseCount(VIBTransaction *vtr)
{
   int count = 0;

   try
   {
      count = TTRForVTR(vtr)->DatabaseCount;
   }
   CATCH_EIBERROR

   return count;
}

//
// VIBTransaction_Databases
//
// Read-only property.
//
VIBDatabase *VIBCALL VIBTransaction_Databases(VIBTransaction *vtr, int Index)
{
   VIBDatabase *vdb = NULL;
   
   try
   {
      TIBDatabase *tdb = TTRForVTR(vtr)->Databases[Index];

      if(tdb)
      {
         vdb = new VIBDatabase;
         vdb->opaque = tdb;
         vdb->isWeak = VIBTRUE;
      }
   }
   CATCH_EIBERROR

   return vdb;
}

//
// VIBTransaction_SQLObjectCount
//
// Read-only property.
//
int VIBCALL VIBTransaction_SQLObjectCount(VIBTransaction *vtr)
{
   int count = 0;

   try
   {
      count = TTRForVTR(vtr)->SQLObjectCount;
   }
   CATCH_EIBERROR

   return count;
}

// TODO: TIBBase *SQLObjects[int Index] (Get Only)

//
// VIBTransaction_Handle
//
void *VIBCALL VIBTransaction_Handle(VIBTransaction *vtr)
{
   void *handle = NULL;

   try
   {
      handle = TTRForVTR(vtr)->Handle;
   }
   CATCH_EIBERROR

   return handle;
}

//
// VIBTransaction_HandleIsShared
//
// Read-only property.
//
VIBBOOL VIBCALL VIBTransaction_HandleIsShared(VIBTransaction *vtr)
{
   VIBBOOL result = VIBFALSE;
   
   try
   {
      bool cppres = TTRForVTR(vtr)->HandleIsShared;

      result = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return result;
}

//
// VIBTransaction_InTransaction
//
// Read-only property.
//
VIBBOOL VIBCALL VIBTransaction_InTransaction(VIBTransaction *vtr)
{
   VIBBOOL result = VIBFALSE;
   
   try
   {
      bool cppres = TTRForVTR(vtr)->InTransaction;

      result = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return result;
}

//
// VIBTransaction_TPB
//
// Read-only property.
//
char *VIBCALL VIBTransaction_TPB(VIBTransaction *vtr)
{
   char *result = NULL;
   
   try
   {
      result = TTRForVTR(vtr)->TPB;
   }
   CATCH_EIBERROR

   return result;
}

//
// VIBTransaction_TPBLength
//
// Read-only property.
//
short VIBCALL VIBTransaction_TPBLength(VIBTransaction *vtr)
{
   short result = 0;

   try
   {
      result = TTRForVTR(vtr)->TPBLength;
   }
   CATCH_EIBERROR

   return result;
}

//
// VIBTransaction_GetActive
//
// Read-write property.
//
VIBBOOL VIBCALL VIBTransaction_GetActive(VIBTransaction *vtr)
{
   VIBBOOL res = VIBFALSE;

   try
   {
      bool cppres = TTRForVTR(vtr)->Active;

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBTransaction_SetActive
//
// Read-write property.
//
VIBBOOL VIBCALL VIBTransaction_SetActive(VIBTransaction *vtr, VIBBOOL Active)
{
   try
   {
      TTRForVTR(vtr)->Active = (Active ? true : false);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_GetDefaultDatabase
//
// Read-write property.
//
VIBDatabase *VIBCALL VIBTransaction_GetDefaultDatabase(VIBTransaction *vtr)
{
   VIBDatabase *vdb = NULL;

   try
   {
      TIBDatabase *tdb = TTRForVTR(vtr)->DefaultDatabase;

      if(tdb)
      {
         vdb = new VIBDatabase;
         vdb->opaque = tdb;
         vdb->isWeak = VIBTRUE;
      }
   }
   CATCH_EIBERROR

   return vdb;
}

//
// VIBTransaction_SetDefaultDatabase
//
// Read-write property.
//
VIBBOOL VIBCALL VIBTransaction_SetDefaultDatabase(VIBTransaction *vtr, VIBDatabase *DefaultDatabase)
{
   try
   {
      TIBTransaction *ttr = TTRForVTR(vtr);
      TIBDatabase    *tdb = TDBForVDB(DefaultDatabase);

      ttr->DefaultDatabase = tdb;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_GetIdleTimer
//
// Read-write property.
//
int VIBCALL VIBTransaction_GetIdleTimer(VIBTransaction *vtr)
{
   int ret = 0;

   try
   {
      ret = TTRForVTR(vtr)->IdleTimer;
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBTransaction_SetIdleTimer
//
// Read-write property.
//
VIBBOOL VIBCALL VIBTransaction_SetIdleTimer(VIBTransaction *vtr, int IdleTimer)
{
   try
   {
      TTRForVTR(vtr)->IdleTimer = IdleTimer;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_GetDefaultAction
//
// Read-write property.
//
VIBTransactionAction VIBCALL VIBTransaction_GetDefaultAction(VIBTransaction *vtr)
{
   VIBTransactionAction action = vib_TACommit;

   try
   {
      TIBTransactionAction taction = TTRForVTR(vtr)->DefaultAction;

      switch(taction)
      {
      case TARollback:          action = vib_TARollback;          break;
      case TACommit:            action = vib_TACommit;            break;
      case TARollbackRetaining: action = vib_TARollbackRetaining; break;
      case TACommitRetaining:   action = vib_TACommitRetaining;   break;
      }
   }
   CATCH_EIBERROR

   return action;
}

//
// VIBTransaction_SetDefaultAction
//
// Read-write property.
//
VIBBOOL VIBCALL VIBTransaction_SetDefaultAction(VIBTransaction *vtr, VIBTransactionAction action)
{
   try
   {
      TIBTransactionAction taction;

      switch(action)
      {
      case vib_TARollback:          taction = TARollback;          break;
      case vib_TACommit:            taction = TACommit;            break;
      case vib_TARollbackRetaining: taction = TARollbackRetaining; break;
      case vib_TACommitRetaining:   taction = TACommitRetaining;   break;
      }

      TTRForVTR(vtr)->DefaultAction = taction;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBTransaction_Params_Add
//
VIBBOOL VIBCALL VIBTransaction_Params_Add(VIBTransaction *vtr, const char *Param)
{
   try
   {
      TTRForVTR(vtr)->Params->Add(Param);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

// TODO: Any other support for TIBTransaction::Params

//
// VIBTransaction_GetAutoStopAction
//
// Read-write property.
//
VIBAutoStopAction VIBCALL VIBTransaction_GetAutoStopAction(VIBTransaction *vtr)
{
   VIBAutoStopAction action = 0;

   try
   {
      TAutoStopAction taction = TTRForVTR(vtr)->AutoStopAction;

      switch(taction)
      {
      case saNone:              action = vib_saNone;              break;
      case saRollback:          action = vib_saRollback;          break;
      case saCommit:            action = vib_saCommit;            break;
      case saRollbackRetaining: action = vib_saRollbackRetaining; break;
      case saCommitRetaining:   action = vib_saCommitRetaining;   break;
      }
   }
   CATCH_EIBERROR

   return action;
}

//
// VIBTransaction_SetAutoStopAction
//
// Read-write property.
//
VIBBOOL VIBCALL VIBTransaction_SetAutoStopAction(VIBTransaction *vtr, VIBAutoStopAction action)
{
   try
   {
      TAutoStopAction taction;

      switch(action)
      {
      case vib_saNone:              taction = saNone;              break;
      case vib_saRollback:          taction = saRollback;          break;
      case vib_saCommit:            taction = saCommit;            break;
      case vib_saRollbackRetaining: taction = saRollbackRetaining; break;
      case vib_saCommitRetaining:   taction = saCommitRetaining;   break;
      }

      TTRForVTR(vtr)->AutoStopAction = taction;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

// TODO: Event OnIdleTimer

// EOF

