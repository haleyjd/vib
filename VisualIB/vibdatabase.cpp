/** @file vibdatabase.cpp
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * TIBDatabase Wrapper
 * @author James Haley
 *
 */

#include "vibinlines.h"

#include "vibexception.h"
#include "vibstring.h"


//
// VIBDatabase_New
//
VIBDatabase * VIBCALL VIBDatabase_New()
{
   TIBDatabase *tdb = NULL;
   VIBDatabase *vdb = NULL;
   
   try
   {
      tdb = new TIBDatabase(NULL);

      if(tdb)
      {
         vdb = new VIBDatabase;
         vdb->opaque = tdb;
         vdb->isWeak = VIBFALSE;
      }
   }
   CATCH_EIBERROR

   return vdb;
}

//
// VIBDatabase_Destroy
//
void VIBCALL VIBDatabase_Destroy(VIBDatabase *vdb)
{
   if(!vdb->isWeak)
   {
      try
      {
         TIBDatabase *tdb = TDBForVDB(vdb);
         delete tdb;
      }
      CATCH_EIBERROR
   }

   vdb->opaque = NULL;
   delete vdb;
}

// TODO: void ApplyUpdates(const Db::TDataSet* * DataSets, const int DataSets_Size)

//
// VIBDatabase_CloseDataSets
//
VIBBOOL VIBCALL VIBDatabase_CloseDataSets(VIBDatabase *vdb)
{
   try
   {
      TDBForVDB(vdb)->CloseDataSets();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_CheckActive
//
VIBBOOL VIBCALL VIBDatabase_CheckActive(VIBDatabase *vdb)
{
   try
   {
      TDBForVDB(vdb)->CheckActive();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_CheckInactive
//
VIBBOOL VIBCALL VIBDatabase_CheckInactive(VIBDatabase *vdb)
{
   try
   {
      TDBForVDB(vdb)->CheckInactive();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_CreateDatabase
//
VIBBOOL VIBCALL VIBDatabase_CreateDatabase(VIBDatabase *vdb)
{
   try
   {
      TDBForVDB(vdb)->CreateDatabase();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_DropDatabase
//
VIBBOOL VIBCALL VIBDatabase_DropDatabase(VIBDatabase *vdb)
{
   try
   {
      TDBForVDB(vdb)->DropDatabase();
   }
   CATCH_EIBERROR_RF
   
   return VIBTRUE;
}

//
// VIBDatabase_ForceClose
//
VIBBOOL VIBCALL VIBDatabase_ForceClose(VIBDatabase *vdb)
{
   try
   {
      TDBForVDB(vdb)->ForceClose();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

// TODO: void GetFieldNames(const AnsiString TableName, Classes::TStrings* List)
// TODO: void GetTableNames(Classes::TStrings* List, bool SystemTables);

//
// VIBDatabase_IndexOfDBConst
//
VIBBOOL VIBCALL VIBDatabase_IndexOfDBConst(VIBDatabase *vdb, const char *st, int *ret)
{
   try
   {
      *ret = TDBForVDB(vdb)->IndexOfDBConst(st);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_TestConnected
//
VIBBOOL VIBCALL VIBDatabase_TestConnected(VIBDatabase *vdb)
{
   VIBBOOL res = VIBFALSE;

   try
   {
      res = (TDBForVDB(vdb)->TestConnected() == true);
   }
   CATCH_EIBERROR_RF   
   
   return res;
}

//
// VIBDatabase_CheckDatabaseName
//
VIBBOOL VIBCALL VIBDatabase_CheckDatabaseName(VIBDatabase *vdb)
{
   try
   {
      TDBForVDB(vdb)->CheckDatabaseName();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_Call
//
VIBBOOL VIBCALL VIBDatabase_Call(VIBDatabase *vdb, int ErrCode, VIBBOOL RaiseError, int *ret)
{
   try
   {
      *ret = TDBForVDB(vdb)->Call(ErrCode, (RaiseError == VIBTRUE));
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_AddTransaction
//
VIBBOOL VIBCALL VIBDatabase_AddTransaction(VIBDatabase *vdb, VIBTransaction *vtr, int *ret)
{
   try
   {
      *ret = TDBForVDB(vdb)->AddTransaction(TTRForVTR(vtr));
   }
   CATCH_EIBERROR_RF
   
   return VIBTRUE;
}

//
// VIBDatabase_FindTransaction
//
VIBBOOL VIBCALL VIBDatabase_FindTransaction(VIBDatabase *vdb, VIBTransaction *vtr, int *ret)
{
   try
   {
      *ret = TDBForVDB(vdb)->FindTransaction(TTRForVTR(vtr));
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_FindDefaultTransaction
//
VIBTransaction *VIBCALL VIBDatabase_FindDefaultTransaction(VIBDatabase *vdb)
{
   VIBTransaction *ret = NULL;

   try
   {
      TIBTransaction *ttr = TDBForVDB(vdb)->FindDefaultTransaction();

      if(ttr)
      {
         ret = new VIBTransaction;
         ret->opaque = ttr;
         ret->isWeak = VIBTRUE;
      }
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBDatabase_RemoveTransaction
//
VIBBOOL VIBCALL VIBDatabase_RemoveTransaction(VIBDatabase *vdb, int Idx)
{
   try
   {
      TDBForVDB(vdb)->RemoveTransaction(Idx);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_RemoveTransactions
//
VIBBOOL VIBCALL VIBDatabase_RemoveTransactions(VIBDatabase *vdb)
{
   try
   {
      TDBForVDB(vdb)->RemoveTransactions();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_SetHandle
//
VIBBOOL VIBCALL VIBDatabase_SetHandle(VIBDatabase *vdb, void **Value)
{
   try
   {
      TDBForVDB(vdb)->SetHandle(Value);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_Handle(VIBDatabase *vdb)
//
// Read-only property.
//
void **VIBCALL VIBDatabase_Handle(VIBDatabase *vdb)
{
   void **ret = NULL;

   try
   {
      ret = TDBForVDB(vdb)->Handle;
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBDatabase_IsReadOnly
//
// Read-only property.
//
VIBBOOL VIBCALL VIBDatabase_IsReadOnly(VIBDatabase *vdb)
{
   VIBBOOL ret = VIBFALSE;

   try
   {
      bool cppret = TDBForVDB(vdb)->IsReadOnly;
      
      ret = (cppret ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBDatabae_DBParamByDPB
//
VIBString *VIBCALL VIBDatabase_DBParamByDPB(VIBDatabase *vdb, int Idx)
{
   VIBString *newstr = NULL;

   try
   {
      AnsiString astr = TDBForVDB(vdb)->DBParamByDPB[Idx];

      newstr = VIBString_New(astr.c_str());
   }
   CATCH_EIBERROR

   return newstr;
}

//
// VIBDatabase_SQLObjectCount
//
// Read-only property.
//
int VIBCALL VIBDatabase_SQLObjectCount(VIBDatabase *vdb)
{
   int ret = 0;

   try
   {
      ret = TDBForVDB(vdb)->SQLObjectCount;
   }
   CATCH_EIBERROR

   return ret;
}

// TODO: TIBBase *SQLObjects[int Index] (Get Only)

//
// VIBDatabase_HandleIsShared
//
// Read-only property.
//
VIBBOOL VIBCALL VIBDatabase_HandleIsShared(VIBDatabase *vdb)
{
   VIBBOOL ret = VIBFALSE;

   try
   {
      bool cppret = TDBForVDB(vdb)->HandleIsShared;

      ret = (cppret ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBDatabase_TransactionCount
//
// Read-only property.
//
int VIBCALL VIBDatabase_TransactionCount(VIBDatabase *vdb)
{
   int ret = 0;

   try
   {
      ret = TDBForVDB(vdb)->TransactionCount;
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBDatabase_Transactions
//
// Read-only property.
//
VIBTransaction *VIBCALL VIBDatabase_Transactions(VIBDatabase *vdb, int Index)
{
   VIBTransaction *ret = NULL;

   try
   {
      TIBTransaction *ttr = TDBForVDB(vdb)->Transactions[Index];

      if(ttr)
      {
         ret = new VIBTransaction;
         ret->opaque = ttr;
         ret->isWeak = VIBTRUE;
      }
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBDatabase_InternalTransaction
//
// Read-only property.
//
VIBTransaction *VIBCALL VIBDatabase_InternalTransaction(VIBDatabase *vdb)
{
   VIBTransaction *ret = NULL;

   try
   {
      TIBTransaction *ttr = TDBForVDB(vdb)->InternalTransaction;

      if(ttr)
      {
         ret = new VIBTransaction;
         ret->opaque = ttr;
         ret->isWeak = VIBTRUE;
      }
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBDatabase_Has_DEFAULT_VALUE
//
VIBBOOL VIBCALL VIBDatabase_Has_DEFAULT_VALUE(VIBDatabase *vdb, const char *Relation, const char *Field)
{
   VIBBOOL res = VIBFALSE;

   try
   {
      bool cppres = TDBForVDB(vdb)->Has_DEFAULT_VALUE(Relation, Field);

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBDatabase_Has_COMPUTED_BLR
//
VIBBOOL VIBCALL VIBDatabase_Has_COMPUTED_BLR(VIBDatabase *vdb, const char *Relation, const char *Field)
{
   VIBBOOL res = VIBFALSE;

   try
   {
      bool cppres = TDBForVDB(vdb)->Has_COMPUTED_BLR(Relation, Field);

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBDatabase_FlushSchema
//
VIBBOOL VIBCALL VIBDatabase_FlushSchema(VIBDatabase *vdb)
{
   try
   {
      TDBForVDB(vdb)->FlushSchema();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_GetDatabaseName
//
VIBString *VIBCALL VIBDatabase_GetDatabaseName(VIBDatabase *vdb)
{
   VIBString *newstr = NULL;

   try
   {
      AnsiString dbname = TDBForVDB(vdb)->DatabaseName;

      newstr = VIBString_New(dbname.c_str());
   }
   CATCH_EIBERROR

   return newstr;
}

//
// VIBDatabase_SetDatabaseName
//
VIBBOOL VIBCALL VIBDatabase_SetDatabaseName(VIBDatabase *vdb, const char *name)
{
   try
   {
      TDBForVDB(vdb)->DatabaseName = name;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_Params_Add
//
VIBBOOL VIBCALL VIBDatabase_Params_Add(VIBDatabase *vdb, const char *param)
{
   try
   {
      TDBForVDB(vdb)->Params->Add(param);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

// TODO: Additional support for TIBDatabase::Params

//
// VIBDatabase_GetLoginPrompt
//
// Read-write property.
//
VIBBOOL VIBCALL VIBDatabase_GetLoginPrompt(VIBDatabase *vdb)
{
   VIBBOOL res = VIBFALSE;

   try
   {
      bool cppres = TDBForVDB(vdb)->LoginPrompt;

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBDatabase_SetLoginPrompt
//
// Read-write property.
//
VIBBOOL VIBCALL VIBDatabase_SetLoginPrompt(VIBDatabase *vdb, VIBBOOL LoginPrompt)
{
   try
   {
      TDBForVDB(vdb)->LoginPrompt = (LoginPrompt ? true : false);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_GetDefaultTransaction
//
// Read-write property.
//
VIBTransaction *VIBCALL VIBDatabase_GetDefaultTransaction(VIBDatabase *vdb)
{
   VIBTransaction *ret = NULL;

   try
   {
      TIBTransaction *ttr = TDBForVDB(vdb)->DefaultTransaction;

      if(ttr)
      {
         ret = new VIBTransaction;
         ret->opaque = ttr;
         ret->isWeak = VIBTRUE;
      }
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBDatabase_SetDefaultTransaction
//
// Read-write property.
//
VIBBOOL VIBCALL VIBDatabase_SetDefaultTransaction(VIBDatabase *vdb, VIBTransaction *vtr)
{
   try
   {
      TDBForVDB(vdb)->DefaultTransaction = TTRForVTR(vtr);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_GetIdleTimer
//
// Read-write property.
//
int VIBCALL VIBDatabase_GetIdleTimer(VIBDatabase *vdb)
{
   int ret = 0;

   try
   {
      ret = TDBForVDB(vdb)->IdleTimer;
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBDatabase_SetIdleTimer
//
// Read-write property.
//
VIBBOOL VIBCALL VIBDatabase_SetIdleTimer(VIBDatabase *vdb, int IdleTimer)
{
   try
   {
      TDBForVDB(vdb)->IdleTimer = IdleTimer;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_GetSQLDialect
//
// Read-write property.
//
int VIBCALL VIBDatabase_GetSQLDialect(VIBDatabase *vdb)
{
   int ret = 0;

   try
   {
      ret = TDBForVDB(vdb)->SQLDialect;
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBDatabase_SetSQLDialect
//
// Read-write property.
//
VIBBOOL VIBCALL VIBDatabase_SetSQLDialect(VIBDatabase *vdb, int SQLDialect)
{
   try
   {
      TDBForVDB(vdb)->SQLDialect = SQLDialect;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_DBSQLDialect
//
// Read-only property.
//
int VIBCALL VIBDatabase_DBSQLDialect(VIBDatabase *vdb)
{
   int ret = 0;

   try
   {
      ret = TDBForVDB(vdb)->DBSQLDialect;
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBDatabase_GetTraceFlags
//
// Read-write property.
//
VIBTraceFlags VIBCALL VIBDatabase_GetTraceFlags(VIBDatabase *vdb)
{
   VIBTraceFlags flags = 0;

   try
   {
      TTraceFlags ttf = TDBForVDB(vdb)->TraceFlags;

      if(ttf.Contains(tfQPrepare))
         flags |= vib_tfQPrepare;
      if(ttf.Contains(tfQExecute))
         flags |= vib_tfQExecute;
      if(ttf.Contains(tfQFetch))
         flags |= vib_tfQFetch;
      if(ttf.Contains(tfError))
         flags |= vib_tfError;
      if(ttf.Contains(tfStmt))
         flags |= vib_tfStmt;
      if(ttf.Contains(tfConnect))
         flags |= vib_tfConnect;
      if(ttf.Contains(tfTransact))
         flags |= vib_tfTransact;
      if(ttf.Contains(tfBlob))
         flags |= vib_tfBlob;
      if(ttf.Contains(tfService))
         flags |= vib_tfService;
      if(ttf.Contains(tfMisc))
         flags |= vib_tfMisc;
   }
   CATCH_EIBERROR

   return flags;
}

//
// VIBDatabase_SetTraceFlags
//
// Read-write property.
//
VIBBOOL VIBCALL VIBDatabase_SetTraceFlags(VIBDatabase *vdb, VIBTraceFlags flags)
{
   try
   {
      TDBForVDB(vdb)->TraceFlags.Clear();

      if(flags & vib_tfQPrepare)
         TDBForVDB(vdb)->TraceFlags << tfQPrepare;
      if(flags & vib_tfQExecute)
         TDBForVDB(vdb)->TraceFlags << tfQExecute;
      if(flags & vib_tfQFetch)
         TDBForVDB(vdb)->TraceFlags << tfQFetch;
      if(flags & vib_tfError)
         TDBForVDB(vdb)->TraceFlags << tfError;
      if(flags & vib_tfStmt)
         TDBForVDB(vdb)->TraceFlags << tfStmt;
      if(flags & vib_tfConnect)
         TDBForVDB(vdb)->TraceFlags << tfConnect;
      if(flags & vib_tfTransact)
         TDBForVDB(vdb)->TraceFlags << tfTransact;
      if(flags & vib_tfBlob)
         TDBForVDB(vdb)->TraceFlags << tfBlob;
      if(flags & vib_tfService)
         TDBForVDB(vdb)->TraceFlags << tfService;
      if(flags & vib_tfMisc)
         TDBForVDB(vdb)->TraceFlags << tfMisc;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_GetAllowStreamedConnected
//
// Read-write property.
//
VIBBOOL VIBCALL VIBDatabase_GetAllowStreamedConnected(VIBDatabase *vdb)
{
   VIBBOOL res = VIBTRUE;

   try
   {
      bool cppres = TDBForVDB(vdb)->AllowStreamedConnected;

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBDatabase_SetAllowStreamedConnected
//
// Read-write property.
//
VIBBOOL VIBCALL VIBDatabase_SetAllowStreamedConnected(VIBDatabase *vdb, VIBBOOL AllowStreamedConnected)
{
   try
   {
      TDBForVDB(vdb)->AllowStreamedConnected = (AllowStreamedConnected ? true : false);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

// TODO: Events: AfterConnect, AfterDisconnect, BeforeConnect, BeforeDisconnect
// TODO: Events: OnLogin, OnIdleTimer, OnDialectDowngradeWarning

//
// Inherited Properties and Methods (from TCustomConnection)
//

//
// VIBDatabase_Open
//
VIBBOOL VIBCALL VIBDatabase_Open(VIBDatabase *vdb)
{
   try
   {
      TDBForVDB(vdb)->Open();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_Close
//
VIBBOOL VIBCALL VIBDatabase_Close(VIBDatabase *vdb)
{
   try
   {
      TDBForVDB(vdb)->Close();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_GetConnected
//
VIBBOOL VIBCALL VIBDatabase_GetConnected(VIBDatabase *vdb)
{
   VIBBOOL res = VIBFALSE;
   
   try
   {
      bool cppres = TDBForVDB(vdb)->Connected;

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBDatabase_SetConnected
//
VIBBOOL VIBCALL VIBDatabase_SetConnected(VIBDatabase *vdb, VIBBOOL Connected)
{
   try
   {
      TDBForVDB(vdb)->Connected = (Connected ? true : false);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDatabase_DataSets
//
VIBDataSet *VIBCALL VIBDatabase_DataSets(VIBDatabase *vdb, int Index)
{
   VIBDataSet *vds = NULL;

   try
   {
      TIBDataSet *tds = dynamic_cast<TIBDataSet *>(TDBForVDB(vdb)->DataSets[Index]);

      if(tds)
      {
         vds = new VIBDataSet;
         vds->opaque = tds;
         vds->isWeak = VIBTRUE;
      }
   }
   CATCH_EIBERROR

   return vds;
}

//
// VIBDatabase_DataSetCount
//
// Read-only property.
//
int VIBCALL VIBDatabase_DataSetCount(VIBDatabase *vdb)
{
   int ret = 0;

   try
   {
      ret = TDBForVDB(vdb)->DataSetCount;
   }
   CATCH_EIBERROR

   return ret;
}

// EOF

 