/*
 * VIB::Transaction Class
 */

#ifdef _MSC_VER
// "forcing value to bool 'true' or 'false' (performance warning)" - Not important.
// "'this' used in base member initializer list" - I'm only storing the pointer, not using it!
#pragma warning(disable : 4800 4355)
#endif

#include "../vibtransaction.h"

#include "classVIBTransaction.h"
#include "classVIBDatabase.h"
#include "VIBInternalErrors.h"
#include "VIBUtils.h"

//
// Property class implementations
//

VIBDatabase *VIB::Transaction::DatabasesClass::operator [] (int i)
{
   return VIBTransaction_Databases(parent->vtr, i);
}

void VIB::Transaction::ParamClass::Add(const std::string &value)
{
   VIBSAFECALL(VIBTransaction_Params_Add(parent->vtr, value.c_str()));
}

//
// Constructor
//
VIB::Transaction::Transaction(VIBTransaction *pvtr)
   : dbObject(this), paramObject(this),
     Active(), AutoStopAction(), DatabaseCount(), Databases(), DefaultAction(),
     DefaultDatabase(), Handle(), HandleIsShared(), IdleTimer(),
     InTransaction(), Params(), SQLObjectCount(), TPB(), TPBLength()
{
   if(pvtr)
      vtr = pvtr; // NB: takes ownership
   else if(!(vtr = VIBTransaction_New()))
      throw Error("Failed to construct an instance of VIBTransaction");

   // Setup properties

   // Active
   Active.initCallbacks(
      [=] ()              { return VIBTransaction_GetActive(vtr); },
      [=] (const bool &b) { VIBSAFECALL(VIBTransaction_SetActive(vtr, b)); });

   // AutoStopAction
   AutoStopAction.initCallbacks(
      [=] ()                             { return VIBTransaction_GetAutoStopAction(vtr); },
      [=] (const VIBAutoStopAction &asa) { VIBSAFECALL(VIBTransaction_SetAutoStopAction(vtr, asa)); });

   // DatabaseCount
   DatabaseCount.initCallbacks(
      [=] ()             { return VIBTransaction_DatabaseCount(vtr); },
      [=] (const int &i) { throw Error("Cannot set DatabaseCount"); });

   // Databases
   Databases.initCallbacks(
      [=] ()                        { return &dbObject; },
      [=] (DatabasesClass *const d) { throw Error("Cannot set Databases"); });

   // DefaultAction
   DefaultAction.initCallbacks(
      [=] ()                               { return VIBTransaction_GetDefaultAction(vtr); },
      [=] (const VIBTransactionAction &ta) { VIBSAFECALL(VIBTransaction_SetDefaultAction(vtr, ta)); });

   // DefaultDatabase
   DefaultDatabase.initCallbacks(
      [=] ()                       { return VIBTransaction_GetDefaultDatabase(vtr); },
      [=] (VIBDatabase *const &db) { VIBSAFECALL(VIBTransaction_SetDefaultDatabase(vtr, db)); });
   
   // Handle
   Handle.initCallbacks(
      [=] ()               { return VIBTransaction_Handle(vtr); },
      [=] (void *const &p) { throw Error("Cannot set Handle"); });
   
   // HandleIsShared
   HandleIsShared.initCallbacks(
      [=] ()              { return VIBTransaction_HandleIsShared(vtr); },
      [=] (const bool &b) { throw Error("Cannot set HandleIsShared"); });
   
   // IdleTimer
   IdleTimer.initCallbacks(
      [=] ()             { return VIBTransaction_GetIdleTimer(vtr); },
      [=] (const int &i) { VIBSAFECALL(VIBTransaction_SetIdleTimer(vtr, i)); });

   // InTransaction
   InTransaction.initCallbacks(
      [=] ()              { return VIBTransaction_InTransaction(vtr); },
      [=] (const bool &b) { throw Error("Cannot set InTransaction"); });

   // Params
   Params.initCallbacks(
      [=] ()                    { return &paramObject; },
      [=] (ParamClass *const p) { throw Error("Cannot set Params"); });

   // SQLObjectCount
   SQLObjectCount.initCallbacks(
      [=] ()             { return VIBTransaction_SQLObjectCount(vtr); },
      [=] (const int &i) { throw Error("Cannot set SQLObjectCount"); });

   // TPB
   TPB.initCallbacks(
      [=] ()                { return VIBTransaction_TPB(vtr); },
      [=] (char *const &ch) { throw Error("Cannot set TPB"); });
    
   // TPBLength
   TPBLength.initCallbacks(
      [=] ()               { return VIBTransaction_TPBLength(vtr); },
      [=] (const short &s) { throw Error("Cannot set TPBLength"); });
}

//
// Destructor
//
VIB::Transaction::~Transaction()
{
   if(vtr)
   {
      VIBTransaction_Destroy(vtr);
      vtr = nullptr;
   }
}

int VIB::Transaction::Call(int ErrCode, bool RaiseError)
{
   int i = 0;
   VIBSAFECALL(VIBTransaction_Call(vtr, ErrCode, RaiseError, &i));
   return i;
}

void VIB::Transaction::Commit()
{
   VIBSAFECALL(VIBTransaction_Commit(vtr));
}

void VIB::Transaction::CommitRetaining()
{
   VIBSAFECALL(VIBTransaction_CommitRetaining(vtr));
}

void VIB::Transaction::Rollback()
{
   VIBSAFECALL(VIBTransaction_Rollback(vtr));
}

void VIB::Transaction::RollbackRetaining()
{
   VIBSAFECALL(VIBTransaction_RollbackRetaining(vtr));
}

void VIB::Transaction::StartTransaction()
{
   VIBSAFECALL(VIBTransaction_StartTransaction(vtr));
}

void VIB::Transaction::CheckInTransaction()
{
   VIBSAFECALL(VIBTransaction_CheckInTransaction(vtr));
}

void VIB::Transaction::CheckNotInTransaction()
{
   VIBSAFECALL(VIBTransaction_CheckNotInTransaction(vtr));
}

void VIB::Transaction::CheckAutoStop()
{
   VIBSAFECALL(VIBTransaction_CheckAutoStop(vtr));
}

int VIB::Transaction::AddDatabase(Database *db)
{
   int ret = 0;
   VIBSAFECALL(VIBTransaction_AddDatabase(vtr, db->getVIBDatabase(), &ret));
   return ret;
}

int VIB::Transaction::FindDatabase(Database *db)
{
   int ret = 0;
   VIBSAFECALL(VIBTransaction_FindDatabase(vtr, db->getVIBDatabase(), &ret));
   return ret;
}

VIBDatabase *VIB::Transaction::FindDefaultDatabase()
{
   VIBDatabase *ret = nullptr;
   VIBSAFECALL(ret = VIBTransaction_FindDefaultDatabase(vtr));
   return ret;
}

void VIB::Transaction::RemoveDatabase(int Idx)
{
   VIBSAFECALL(VIBTransaction_RemoveDatabase(vtr, Idx));
}

void VIB::Transaction::RemoveDatabases()
{
   VIBSAFECALL(VIBTransaction_RemoveDatabases(vtr));
}

void VIB::Transaction::CheckDatabasesInList()
{
   VIBSAFECALL(VIBTransaction_CheckDatabasesInList(vtr));
}

// EOF

