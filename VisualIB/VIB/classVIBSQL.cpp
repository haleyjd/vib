/*
 *  VIB Class Library - VIBSQL Wrapper
 *  @author James Haley
 */

#ifdef _MSC_VER
#pragma warning(disable : 4800 4355)
#endif

#include "classVIBSQL.h"
#include "VIBInternalErrors.h"
#include "VIBUtils.h"

// Property class implementations

VIB::SQL::SQLClass::SQLClass(SQL *pParent)
   : parent(pParent), Text()
{
   // Init properties
   Text.initCallbacks(
      [=] ()                     { return ElideVIBString(VIBSQL_SQL_GetText(parent->vsql)); },
      [=] (const std::string &s) { VIBSAFECALL(VIBSQL_SQL_SetText(parent->vsql, s.c_str())); });
}

VIB::SQL::ParamClass::ParamClass(SQL *pParent)
   : parent(pParent), name(), AsString(), IsNull(), SQLType()
{
   // Init properties
   AsString.initCallbacks(
      [=] () -> std::string { throw Error("Cannot get AsString"); return std::string(); },
      [=] (const std::string &s) 
      { 
         VIBSAFECALL(VIBSQL_Params_ByName_SetAsString(parent->vsql, name.c_str(), s.c_str())); 
      });

   IsNull.initCallbacks(
      [=] () -> bool      { throw Error("Cannot get IsNull"); return false; },
      [=] (const bool &b) { VIBSAFECALL(VIBSQL_Params_ByName_SetIsNull(parent->vsql, name.c_str(), b)); });

   SQLType.initCallbacks(
      [=] ()             { return VIBSQL_Params_ByName_GetSQLType(parent->vsql, name.c_str()); },
      [=] (const int &i) { throw Error("Cannot set SQLType"); });
}

auto VIB::SQL::ParamsSetClass::ByName(const std::string &str) -> ParamClass *
{
   pc.name = str;
   return &pc;
}

//
// Constructor
//
VIB::SQL::SQL(VIBSQL *pvsql)
   : sc(this), psc(this),
     Bof(), Database(), DBHandle(), Eof(), GenerateParamNames(), 
     GoToFirstRecordOnExecute(), Handle(), Open(), ParamCheck(), Params(), 
     Plan(), Prepared(), RecordCount(), RowsAffected(), SQLType(), 
     Transaction(), TRHandle(), UniqueRelationName()
{
   if(pvsql)
      vsql = pvsql;
   else if(!(vsql = VIBSQL_New()))
      throw Error("Could not instantiate an instance of VIBSQL");

   // Init properties

   // Bof
   Bof.initCallbacks(
      [=] ()              { return VIBSQL_Bof(vsql); },
      [=] (const bool &b) { throw Error("Cannot set Bof"); });

   // Database
   Database.initCallbacks(
      [=] ()                       { return VIBSQL_GetDatabase(vsql); },
      [=] (VIBDatabase *const &db) { VIBSAFECALL(VIBSQL_SetDatabase(vsql, db)); });

   // DBHandle
   DBHandle.initCallbacks(
      [=] ()                  { return VIBSQL_DBHandle(vsql); },
      [=] (void ***const &vp) { throw Error("Cannot set DBHandle"); });

   // Eof
   Eof.initCallbacks(
      [=] ()              { return VIBSQL_Eof(vsql); },
      [=] (const bool &b) { throw Error("Cannot set Eof"); });

   // GenerateParamNames
   GenerateParamNames.initCallbacks(
      [=] ()              { return VIBSQL_GetGenerateParamNames(vsql); },
      [=] (const bool &b) { VIBSAFECALL(VIBSQL_SetGenerateParamNames(vsql, b)); });

   // GoToFirstRecordOnExecute
   GoToFirstRecordOnExecute.initCallbacks(
      [=] ()              { return VIBSQL_GetGoToFirstRecordOnExecute(vsql); },
      [=] (const bool &b) { VIBSAFECALL(VIBSQL_SetGoToFirstRecordOnExecute(vsql, b)); });

   // Handle
   Handle.initCallbacks(
      [=] ()                 { return VIBSQL_Handle(vsql); },
      [=] (void **const &vp) { throw Error("Cannot set Handle"); });

   // Open
   Open.initCallbacks(
      [=] ()              { return VIBSQL_Open(vsql); },
      [=] (const bool &b) { throw Error("Cannot set Open"); });

   // ParamCheck
   ParamCheck.initCallbacks(
      [=] ()              { return VIBSQL_GetParamCheck(vsql); },
      [=] (const bool &b) { VIBSAFECALL(VIBSQL_SetParamCheck(vsql, b)); });

   // Params
   Params.initCallbacks(
      [=] ()                            { return &psc; },
      [=] (ParamsSetClass *const &ppsc) { throw Error("Cannot set Params"); });

   // Plan
   Plan.initCallbacks(
      [=] ()                     { return ElideVIBString(VIBSQL_Plan(vsql)); },
      [=] (const std::string &s) { throw Error("Cannot set Plan"); });

   // Prepared
   Prepared.initCallbacks(
      [=] ()              { return VIBSQL_Prepared(vsql); },
      [=] (const bool &b) { throw Error("Cannot set Prepared"); });

   // RecordCount
   RecordCount.initCallbacks(
      [=] ()             { return VIBSQL_RecordCount(vsql); },
      [=] (const int &i) { throw Error("Cannot set RecordCount"); });

   // RowsAffected
   RowsAffected.initCallbacks(
      [=] ()             { return VIBSQL_RowsAffected(vsql); },
      [=] (const int &i) { throw Error("Cannot set RowsAffected"); });

   // _SQL
   _SQL.initCallbacks(
      [=] ()                       { return &this->sc; },
      [=] (SQLClass *const &psql)  { throw Error("Cannot set _SQL"); });

   // SQLType
   SQLType.initCallbacks(
      [=] ()                       { return VIBSQL_SQLType(vsql); },
      [=] (const VIBSQLTypes &vst) { throw Error("Cannot set SQLType"); });

   // Transaction
   Transaction.initCallbacks(
      [=] ()                          { return VIBSQL_GetTransaction(vsql); },
      [=] (VIBTransaction *const &tr) { VIBSAFECALL(VIBSQL_SetTransaction(vsql, tr)); });

   // TRHandle
   TRHandle.initCallbacks(
      [=] ()                  { return VIBSQL_TRHandle(vsql); },
      [=] (void ***const &vp) { throw Error("Cannot set TRHandle"); });
   
   // UniqueRelationName
   UniqueRelationName.initCallbacks(
      [=] ()                     { return ElideVIBString(VIBSQL_GetUniqueRelationName(vsql)); },
      [=] (const std::string &s) { throw Error("Cannot set UniqueRelationName"); });
}

//
// Destructor
//
VIB::SQL::~SQL()
{
   if(vsql)
   {
      VIBSQL_Destroy(vsql);
      vsql = nullptr;
   }
}

int  VIB::SQL::Call(int ErrCode, bool RaiseError)
{
   int ret = 0;
   VIBSAFECALL(VIBSQL_Call(vsql, ErrCode, RaiseError, &ret));
   return ret;
}

void VIB::SQL::CheckClosed()
{
   VIBSAFECALL(VIBSQL_CheckClosed(vsql));
}

void VIB::SQL::CheckOpen()
{
   VIBSAFECALL(VIBSQL_CheckOpen(vsql));
}

void VIB::SQL::CheckValidStatement()
{
   VIBSAFECALL(VIBSQL_CheckValidStatement(vsql));
}

void VIB::SQL::Close()
{
   VIBSAFECALL(VIBSQL_Close(vsql));
}

void VIB::SQL::ExecQuery()
{
   VIBSAFECALL(VIBSQL_ExecQuery(vsql));
}

void VIB::SQL::FreeHandle()
{
   VIBSAFECALL(VIBSQL_FreeHandle(vsql));
}

void VIB::SQL::Prepare()
{
   VIBSAFECALL(VIBSQL_Prepare(vsql));
}

int  VIB::SQL::FieldIndex(const std::string &FieldName)
{
   int ret = 0;
   VIBSAFECALL(VIBSQL_FieldIndex(vsql, FieldName.c_str(), &ret));
   return ret;
}

std::string VIB::SQL::GetUniqueRelationName()
{
   return ElideVIBString(VIBSQL_GetUniqueRelationName(vsql));
}

// EOF

