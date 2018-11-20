/** @file vibsql.cpp
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * TIBSQL Wrapper
 * @author James Haley
 *
 */

#include "vibinlines.h"

#include "vibexception.h"
#include "vibstring.h"

//
// VIBSQL_New
//
VIBSQL *VIBCALL VIBSQL_New()
{
   TIBSQL *tsql = NULL;
   VIBSQL *vsql = NULL;

   try
   {
      tsql = new TIBSQL(NULL);

      if(tsql)
      {
         vsql = new VIBSQL;
         vsql->opaque = tsql;
         vsql->isWeak = VIBFALSE;
      }
   }
   CATCH_EIBERROR

   return vsql;
}

//
// VIBSQL_Destroy
//
void VIBCALL VIBSQL_Destroy(VIBSQL *vsql)
{
   if(!vsql->isWeak)
   {
      try
      {
         TIBSQL *tsql = TSQLForVSQL(vsql);
         delete tsql;
      }
      CATCH_EIBERROR
   }

   vsql->opaque = NULL;
   delete vsql;
}

// TODO: void BatchInput(TIBBatchInput *InputObject)
// TODO: void BatchOutput(TIBBatchOutput *OutputObject)

//
// VIBSQL_Call
//
VIBBOOL VIBCALL VIBSQL_Call(VIBSQL *vsql, int ErrCode, VIBBOOL RaiseError, int *ret)
{
   try
   {
      *ret = TSQLForVSQL(vsql)->Call(ErrCode, (RaiseError == VIBTRUE));
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBSQL_CheckClosed
//
VIBBOOL VIBCALL VIBSQL_CheckClosed(VIBSQL *vsql)
{
   try
   {
      TSQLForVSQL(vsql)->CheckClosed();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBSQL_CheckOpen
//
VIBBOOL VIBCALL VIBSQL_CheckOpen(VIBSQL *vsql)
{
   try
   {
      TSQLForVSQL(vsql)->CheckOpen();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBSQL_CheckValidStatement
//
VIBBOOL VIBCALL VIBSQL_CheckValidStatement(VIBSQL *vsql)
{
   try
   {
      TSQLForVSQL(vsql)->CheckValidStatement();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBSQL_Close
//
VIBBOOL VIBCALL VIBSQL_Close(VIBSQL *vsql)
{
   try
   {
      TSQLForVSQL(vsql)->Close();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

// TODO: TIBXSQLDA *Current()

//
// VIBSQL_ExecQuery
//
VIBBOOL VIBCALL VIBSQL_ExecQuery(VIBSQL *vsql)
{
   try
   {
      TSQLForVSQL(vsql)->ExecQuery();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

// TODO: TIBXSQLVAR *FieldByName(AnsiString FieldName)

//
// VIBSQL_FreeHandle
//
VIBBOOL VIBCALL VIBSQL_FreeHandle(VIBSQL *vsql)
{
   try
   {
      TSQLForVSQL(vsql)->FreeHandle();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

// TODO: TIBXSQLDA *Next()

//
// VIBSQL_Prepare
//
VIBBOOL VIBCALL VIBSQL_Prepare(VIBSQL *vsql)
{
   try
   {
      TSQLForVSQL(vsql)->Prepare();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBSQL_GetUniqueRelationName
//
VIBString *VIBCALL VIBSQL_GetUniqueRelationName(VIBSQL *vsql)
{
   VIBString *newstr = NULL;

   try
   {
      AnsiString relname = TSQLForVSQL(vsql)->GetUniqueRelationName();

      newstr = VIBString_New(relname.c_str());
   }
   CATCH_EIBERROR

   return newstr;
}


// TODO: TIBXSQLVAR *ParamByName(AnsiString Idx)

//
// VIBSQL_Bof
//
// Read-only property
//
VIBBOOL VIBCALL VIBSQL_Bof(VIBSQL *vsql)
{
   VIBBOOL res = VIBFALSE;

   try
   {
      bool cppres = TSQLForVSQL(vsql)->Bof;

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBSQL_DBHandle
//
// Read-only property.
//
void ***VIBCALL VIBSQL_DBHandle(VIBSQL *vsql)
{
   void ***res = NULL;

   try
   {
      res = TSQLForVSQL(vsql)->DBHandle;
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBSQL_Eof
//
// Read-only property
//
VIBBOOL VIBCALL VIBSQL_Eof(VIBSQL *vsql)
{
   VIBBOOL res = VIBFALSE;

   try
   {
      bool cppres = TSQLForVSQL(vsql)->Eof;

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

// TODO: TIBXSQLVAR *Fields[int Idx] (Get)

//
// VIBSQL_FieldIndex
//
VIBBOOL VIBCALL VIBSQL_FieldIndex(VIBSQL *vsql, const char *FieldName, int *ret)
{
   try
   {
      *ret = TSQLForVSQL(vsql)->FieldIndex[FieldName];
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBSQL_Open
//
// Read-only property
//
VIBBOOL VIBCALL VIBSQL_Open(VIBSQL *vsql)
{
   VIBBOOL res = VIBFALSE;

   try
   {
      bool cppres = TSQLForVSQL(vsql)->Open;

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBSQL_Params_ByName_SetIsNull
//
VIBBOOL VIBCALL VIBSQL_Params_ByName_SetIsNull(VIBSQL *vsql, const char *name, VIBBOOL IsNull)
{
   try
   {
      bool cppIsNull = (IsNull ? true : false);
      TSQLForVSQL(vsql)->Params->ByName(name)->IsNull = cppIsNull;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBSQL_Params_ByName_SetAsString
//
VIBBOOL VIBCALL VIBSQL_Params_ByName_SetAsString(VIBSQL *vsql, const char *name, const char *str)
{
   try
   {
      TSQLForVSQL(vsql)->Params->ByName(name)->AsString = str;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBSQL_Params_ByName_GetSQLType
//
int VIBCALL VIBSQL_Params_ByName_GetSQLType(VIBSQL *vsql, const char *name)
{
   int ret = 0;

   try
   {
      ret = TSQLForVSQL(vsql)->Params->ByName(name)->SQLType;
   }
   CATCH_EIBERROR

   return ret;
}

// TODO: Additional support for VIBSQL::Params

//
// VIBSQL_Plan
//
VIBString *VIBCALL VIBSQL_Plan(VIBSQL *vsql)
{
   VIBString *newstr = NULL;

   try
   {
      AnsiString plan = TSQLForVSQL(vsql)->Plan;

      newstr = VIBString_New(plan.c_str());
   }
   CATCH_EIBERROR

   return newstr;
}

//
// VIBSQL_Prepared
//
// Read-only property
//
VIBBOOL VIBCALL VIBSQL_Prepared(VIBSQL *vsql)
{
   VIBBOOL res = VIBFALSE;

   try
   {
      bool cppres = TSQLForVSQL(vsql)->Prepared;

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBSQL_RecordCount
//
// Read-only property.
//
int VIBCALL VIBSQL_RecordCount(VIBSQL *vsql)
{
   int count = 0;

   try
   {
      count = TSQLForVSQL(vsql)->RecordCount;
   }
   CATCH_EIBERROR

   return count;
}

//
// VIBSQL_RowsAffected
//
// Read-only property.
//
int VIBCALL VIBSQL_RowsAffected(VIBSQL *vsql)
{
   int count = 0;

   try
   {
      count = TSQLForVSQL(vsql)->RowsAffected;
   }
   CATCH_EIBERROR

   return count;
}

//
// VIBSQL_SQLType
//
// Read-only property.
//
VIBSQLTypes VIBCALL VIBSQL_SQLType(VIBSQL *vsql)
{
   VIBSQLTypes res = vib_SQLUnknown;

   try
   {
      TIBSQLTypes sqltype = TSQLForVSQL(vsql)->SQLType;

      switch(sqltype)
      {
      case SQLUnknown:          res = vib_SQLUnknown;          break;  
      case SQLSelect:           res = vib_SQLSelect;           break;
      case SQLInsert:           res = vib_SQLInsert;           break;
      case SQLUpdate:           res = vib_SQLUpdate;           break;
      case SQLDelete:           res = vib_SQLDelete;           break;
      case SQLDDL:              res = vib_SQLDDL;              break;
      case SQLGetSegment:       res = vib_SQLGetSegment;       break;
      case SQLPutSegment:       res = vib_SQLPutSegment;       break;
      case SQLExecProcedure:    res = vib_SQLExecProcedure;    break;
      case SQLStartTransaction: res = vib_SQLStartTransaction; break; 
      case SQLCommit:           res = vib_SQLCommit;           break;
      case SQLRollback:         res = vib_SQLRollback;         break;
      case SQLSelectForUpdate:  res = vib_SQLSelectForUpdate;  break;
      case SQLSetGenerator:     res = vib_SQLSetGenerator;     break;
      }
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBSQL_TRHandle
//
// Read-only property.
//
void ***VIBCALL VIBSQL_TRHandle(VIBSQL *vsql)
{
   void ***res = NULL;

   try
   {
      res = TSQLForVSQL(vsql)->TRHandle;
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBSQL_Handle
//
// Read-only property.
//
void **VIBCALL VIBSQL_Handle(VIBSQL *vsql)
{
   void **res = NULL;

   try
   {
      res = TSQLForVSQL(vsql)->Handle;
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBSQL_GetGenerateParamNames
//
// Read-write property.
//
VIBBOOL VIBCALL VIBSQL_GetGenerateParamNames(VIBSQL *vsql)
{
   VIBBOOL res = VIBFALSE;

   try
   {
      bool cppres = TSQLForVSQL(vsql)->GenerateParamNames;

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBSQL_SetGenerateParamNames
//
// Read-write property.
//
VIBBOOL VIBCALL VIBSQL_SetGenerateParamNames(VIBSQL *vsql, VIBBOOL GenerateParamNames)
{
   try
   {
      TSQLForVSQL(vsql)->GenerateParamNames = (GenerateParamNames ? true : false);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBSQL_UniqueRelationName
//
VIBString *VIBCALL VIBSQL_UniqueRelationName(VIBSQL *vsql)
{
   VIBString *newstr = NULL;

   try
   {
      AnsiString relname = TSQLForVSQL(vsql)->UniqueRelationName;

      newstr = VIBString_New(relname.c_str());
   }
   CATCH_EIBERROR

   return newstr;
}

//
// VIBSQL_GetDatabase
//
// Read-write property.
//
VIBDatabase *VIBCALL VIBSQL_GetDatabase(VIBSQL *vsql)
{
   VIBDatabase *vdb = NULL;

   try
   {
      TIBDatabase *tdb = TSQLForVSQL(vsql)->Database;

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
// VIBSQL_SetDatabase
//
// Read-write property.
//
VIBBOOL VIBCALL VIBSQL_SetDatabase(VIBSQL *vsql, VIBDatabase *Database)
{
   try
   {
      TIBSQL      *tsql = TSQLForVSQL(vsql);
      TIBDatabase *tdb  = TDBForVDB(Database);

      tsql->Database = tdb;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBSQL_GetGoToFirstRecordOnExecute
//
// Read-write property.
//
VIBBOOL VIBCALL VIBSQL_GetGoToFirstRecordOnExecute(VIBSQL *vsql)
{
   VIBBOOL res = VIBTRUE;

   try
   {
      bool cppres = TSQLForVSQL(vsql)->GoToFirstRecordOnExecute;

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBSQL_SetGoToFirstRecordOnExecute
//
// Read-write property.
//
VIBBOOL VIBCALL VIBSQL_SetGoToFirstRecordOnExecute(VIBSQL *vsql, VIBBOOL GoToFirstRecordOnExecute)
{
   try
   {
      TSQLForVSQL(vsql)->GoToFirstRecordOnExecute = (GoToFirstRecordOnExecute ? true : false);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBSQL_GetParamCheck
//
// Read-write property.
//
VIBBOOL VIBCALL VIBSQL_GetParamCheck(VIBSQL *vsql)
{
   VIBBOOL res = VIBFALSE;

   try
   {
      bool cppres = TSQLForVSQL(vsql)->ParamCheck;

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBSQL_SetParamCheck
//
// Read-write property.
//
VIBBOOL VIBCALL VIBSQL_SetParamCheck(VIBSQL *vsql, VIBBOOL ParamCheck)
{
   try
   {
      TSQLForVSQL(vsql)->ParamCheck = (ParamCheck ? true : false);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBSQL_SQL_SetText
//
VIBBOOL VIBCALL VIBSQL_SQL_SetText(VIBSQL *vsql, const char *Text)
{
   try
   {
      TSQLForVSQL(vsql)->SQL->Text = Text;
   }
   CATCH_EIBERROR_RF
   
   return VIBTRUE;
}

//
// VIBSQL_SQL_GetText
//
VIBString *VIBCALL VIBSQL_SQL_GetText(VIBSQL *vsql)
{
   VIBString *newstr = NULL;

   try
   {
      AnsiString str = TSQLForVSQL(vsql)->SQL->Text;

      newstr = VIBString_New(str.c_str());
   }
   CATCH_EIBERROR

   return newstr;
}

//
// VIBSQL_GetTransaction
//
// Read-write property.
//
VIBTransaction *VIBCALL VIBSQL_GetTransaction(VIBSQL *vsql)
{
   VIBTransaction *vtr = NULL;

   try
   {
      TIBTransaction *ttr = TSQLForVSQL(vsql)->Transaction;

      if(ttr)
      {
         vtr = new VIBTransaction;
         vtr->opaque = ttr;
         vtr->isWeak = VIBTRUE;
      }
   }
   CATCH_EIBERROR

   return vtr;
}

//
// VIBSQL_SetTransaction
//
// Read-write property.
//
VIBBOOL VIBCALL VIBSQL_SetTransaction(VIBSQL *vsql, VIBTransaction *Transaction)
{
   try
   {
      TIBSQL         *tsql = TSQLForVSQL(vsql);
      TIBTransaction *ttr  = TTRForVTR(Transaction);

      tsql->Transaction = ttr;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

// TODO: TNotifyEvent OnSQLChanging

// EOF

