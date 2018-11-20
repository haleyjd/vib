/*
 * VIB::Database Class
 */

#ifdef _MSC_VER
// "forcing value to bool 'true' or 'false' (performance warning)" - Not important.
// "'this' used in base member initializer list" - I'm only storing the pointer, not using it!
#pragma warning(disable : 4800 4355)
#endif

// VisualIB Headers
#include "../vibdatabase.h"

// VIB Headers
#include "classVIBDatabase.h"
#include "VIBInternalErrors.h"
#include "VIBUtils.h"

// Property class implementations

void VIB::Database::ParamClass::Add(const std::string &value)
{
   VIBSAFECALL(VIBDatabase_Params_Add(parent->vdb, value.c_str()));
}

std::string VIB::Database::DBParamClass::operator [] (int i)
{
   return ElideVIBString(VIBDatabase_DBParamByDPB(parent->vdb, i));
}

//
// Constructor
//
VIB::Database::Database(VIBDatabase *pvdb)
   : paramsObj(this), dbParamObj(this),
     AllowStreamedConnected(), Connected(), DatabaseName(), DataSetCount(), 
     DBParamByDPB(), DBSQLDialect(), Handle(), HandleIsShared(), IdleTimer(), 
     IsReadOnly(), LoginPrompt(), Params(), SQLDialect(), SQLObjectCount(), 
     TraceFlags(), TransactionCount()
{
   if(pvdb)
      vdb = pvdb; // NB: takes ownership!
   else if(!(vdb = VIBDatabase_New()))
      throw Error("Failed to construct an instance of VIBDatabase");

   // Setup properties

   // AllowStreamedConnected
   AllowStreamedConnected.initCallbacks(
      [=] ()              { return VIBDatabase_GetAllowStreamedConnected(vdb); },
      [=] (const bool &b) { VIBSAFECALL(VIBDatabase_SetAllowStreamedConnected(vdb, b)); });

   // Connected
   Connected.initCallbacks(
      [=] ()              { return VIBDatabase_GetConnected(vdb); },
      [=] (const bool &b) { VIBSAFECALL(VIBDatabase_SetConnected(vdb, b)); });

   // DatabaseName
   DatabaseName.initCallbacks(
      [=] ()                     { return ElideVIBString(VIBDatabase_GetDatabaseName(vdb)); },
      [=] (const std::string &s) { VIBSAFECALL(VIBDatabase_SetDatabaseName(vdb, s.c_str())); } );

   // DataSetCount
   DataSetCount.initCallbacks(
      [=] ()             { return VIBDatabase_DataSetCount(vdb); },
      [=] (const int &i) { throw Error("Cannot set DataSetCount"); });

   // DBParamByDPB
   DBParamByDPB.initCallbacks(
      [=] ()                        { return &dbParamObj; },
      [=] (DBParamClass *const dbp) { throw Error("Cannot set DBParamByDPB"); });
   
   // DBSQLDialect
   DBSQLDialect.initCallbacks(
      [=] ()             { return VIBDatabase_DBSQLDialect(vdb); },
      [=] (const int &i) { throw Error("Cannot set DBSQLDialect"); });

   // Handle
   Handle.initCallbacks(
      [=] ()                  { return VIBDatabase_Handle(vdb); },
      [=] (void **const &vp ) { VIBSAFECALL(VIBDatabase_SetHandle(vdb, vp)); });

   // HandleIsShared
   HandleIsShared.initCallbacks(
      [=] ()              { return VIBDatabase_HandleIsShared(vdb); },
      [=] (const bool &b) { throw Error("Cannot set HandleIsShared"); });

   // IdleTimer
   IdleTimer.initCallbacks(
      [=] ()             { return VIBDatabase_GetIdleTimer(vdb); },
      [=] (const int &i) { VIBSAFECALL(VIBDatabase_SetIdleTimer(vdb, i)); });

   // IsReadOnly
   IsReadOnly.initCallbacks(
      [=] ()             { return VIBDatabase_IsReadOnly(vdb); },
      [=] (const int &i) { throw Error("Cannot set IsReadOnly"); });
   
   // LoginPrompt
   LoginPrompt.initCallbacks(
      [=] ()              { return VIBDatabase_GetLoginPrompt(vdb); },
      [=] (const bool &b) { VIBSAFECALL(VIBDatabase_SetLoginPrompt(vdb, b)); });

   // Params
   Params.initCallbacks(
      [=] ()                    { return &paramsObj; },
      [=] (ParamClass *const p) { throw Error("Cannot set Params"); });

   // SQLDialect
   SQLDialect.initCallbacks(
      [=] ()             { return VIBDatabase_GetSQLDialect(vdb); },
      [=] (const int &i) { VIBSAFECALL(VIBDatabase_SetSQLDialect(vdb, i)); });

   // SQLObjectCount
   SQLObjectCount.initCallbacks(
      [=] ()             { return VIBDatabase_SQLObjectCount(vdb); },
      [=] (const int &i) { throw Error("Cannot set SQLObjectCount"); });

   // TraceFlags
   TraceFlags.initCallbacks(
      [=] ()                        { return VIBDatabase_GetTraceFlags(vdb); },
      [=] (const VIBTraceFlags &tf) { VIBSAFECALL(VIBDatabase_SetTraceFlags(vdb, tf)); });

   // TransactionCount
   TransactionCount.initCallbacks(
      [=] ()             { return VIBDatabase_TransactionCount(vdb); }, 
      [=] (const int &i) { throw Error("Cannot set TransactionCount"); });
}

//
// Destructor
//
VIB::Database::~Database()
{
   if(vdb)
   {
      VIBDatabase_Destroy(vdb);
      vdb = nullptr;
   }
}

void VIB::Database::CloseDataSets()
{
   VIBSAFECALL(VIBDatabase_CloseDataSets(vdb));
}

void VIB::Database::CheckActive()
{
   VIBSAFECALL(VIBDatabase_CheckActive(vdb));
}

void VIB::Database::CheckInactive()
{
   VIBSAFECALL(VIBDatabase_CheckInactive(vdb));
}

void VIB::Database::CreateDatabase()
{
   VIBSAFECALL(VIBDatabase_CreateDatabase(vdb));
}

void VIB::Database::DropDatabase()
{
   VIBSAFECALL(VIBDatabase_DropDatabase(vdb));
}

void VIB::Database::ForceClose()
{
   VIBSAFECALL(VIBDatabase_ForceClose(vdb));
}

int VIB::Database::IndexOfDBConst(const char *st)
{
   int ret = 0;
   VIBSAFECALL(VIBDatabase_IndexOfDBConst(vdb, st, &ret));
   return ret;
}

bool VIB::Database::TestConnected()
{
   return VIBDatabase_TestConnected(vdb);
}

void VIB::Database::CheckDatabaseName()
{
   VIBSAFECALL(VIBDatabase_CheckDatabaseName(vdb));
}

int  VIB::Database::Call(int ErrCode, bool RaiseError)
{
   int ret = 0;
   VIBSAFECALL(VIBDatabase_Call(vdb, ErrCode, RaiseError, &ret));
   return ret;
}

void VIB::Database::RemoveTransactions()
{
   VIBSAFECALL(VIBDatabase_RemoveTransactions(vdb));
}

void VIB::Database::Open()
{
   VIBSAFECALL(VIBDatabase_Open(vdb));
}

void VIB::Database::Close()
{
   VIBSAFECALL(VIBDatabase_Close(vdb));
}

void VIB::Database::FlushSchema()
{
   VIBSAFECALL(VIBDatabase_FlushSchema(vdb));
}

bool VIB::Database::Has_COMPUTED_BLR(const std::string &Relation, const std::string &Field)
{
   return VIBDatabase_Has_COMPUTED_BLR(vdb, Relation.c_str(), Field.c_str());
}

bool VIB::Database::Has_DEFAULT_VALUE(const std::string &Relation, const std::string &Field)
{
   return VIBDatabase_Has_DEFAULT_VALUE(vdb, Relation.c_str(), Field.c_str());
}

// EOF

