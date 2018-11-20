/*
 * VIB Class Library - DataSet wrapper
 * @author James Haley
 */

#ifdef _MSC_VER
#pragma warning(disable : 4800 4355)
#endif

#include "../vibdatabase.h"
#include "../vibdataset.h"

#include "classVIBDataSet.h"
#include "VIBInternalErrors.h"
#include "VIBUtils.h"

//
// Property class implementations
//

VIB::DataSet::FieldByNameClass::FieldByNameClass(VIB::DataSet *pParent) 
   : parent(pParent), AsString()
{
   // init properties
   AsString.initCallbacks(
      [=] ()                       
      { 
         return ElideVIBString(VIBDataSet_FieldByName_AsString(parent->vds, fieldname.c_str())); 
      },
      [=] (const std::string &str) { throw Error("Cannot set AsString"); });
}

VIB::DataSet::FieldClass::FieldClass(DataSet *pds)
   : ds(pds), index(0), 
     AsString(),DataType(), FieldKind(), FieldName()
{
   // Init properties

   // AsString
   AsString.initCallbacks(
      [=] ()                     { return ElideVIBString(VIBDataSet_Fields_AsString(ds->vds, index)); },
      [=] (const std::string &s) { throw Error("Cannot set AsString"); });

   // DataType
   DataType.initCallbacks(
      [=] ()                       { return VIBDataSet_Fields_DataType(ds->vds, index); },
      [=] (const VIBFieldType &ft) { throw Error("Cannot set DataType"); });

   // FieldKind
   FieldKind.initCallbacks(
      [=] ()                       { return VIBDataSet_Fields_FieldKind(ds->vds, index); },
      [=] (const VIBFieldKind &fk) { throw Error("Cannot set FieldKind"); });

   // FieldName
   FieldName.initCallbacks(
      [=] ()                     { return ElideVIBString(VIBDataSet_Fields_FieldName(ds->vds, index)); },
      [=] (const std::string &s) { throw Error("Cannot set FieldName"); });
}

VIB::DataSet::FieldsClass::FieldsClass(DataSet *pParent)
   : parent(pParent), fp(pParent), Count(), Fields()
{
   // Init properties

   // Fields array
   Fields.initCallbacks(
      [=] ()                     { return &fp; },
      [=] (FieldProp *const &fp) { throw Error("Cannot set FieldProp"); });

   // Count
   Count.initCallbacks(
      [=] ()             { return VIBDataSet_Fields_Count(parent->vds); },
      [=] (const int &i) { throw Error("Cannot set Count"); });
}

VIB::DataSet::SelectSQLClass::SelectSQLClass(DataSet *pParent)
   : parent(pParent), Text()
{
   // Init properties
   Text.initCallbacks(
      [=] ()                     { return ElideVIBString(VIBDataSet_SelectSQL_GetText(parent->vds)); },
      [=] (const std::string &s) { VIBSAFECALL(VIBDataSet_SelectSQL_SetText(parent->vds, s.c_str())); });
}

void VIB::DataSet::SelectSQLClass::Clear()
{
   VIBSAFECALL(VIBDataSet_SelectSQL_Clear(parent->vds));
}

void VIB::DataSet::SelectSQLClass::Add(const std::string &str)
{
   VIBSAFECALL(VIBDataSet_SelectSQL_Add(parent->vds, str.c_str()));
}

//
// Constructor
//
VIB::DataSet::DataSet(VIBDataSet *pvds)
   : fbnc(this), fc(this), sql(this),
     Database(), FieldCount(), Fields(), SelectSQL(), Transaction(), UniDirectional()
{
   if(pvds)
      vds = pvds;
   else if(!(vds = VIBDataSet_New()))
      throw Error("Could not instantiate an instance of VIBDataSet");

   // Init properties

   // Database
   Database.initCallbacks(
      [=] ()                       { return VIBDataSet_GetDatabase(vds); },
      [=] (VIBDatabase *const &db) { VIBSAFECALL(VIBDataSet_SetDatabase(vds, db)); });
      
   // FieldCount
   FieldCount.initCallbacks(
      [=] ()             { return VIBDataSet_FieldCount(vds); },
      [=] (const int &i) { throw Error("Cannot set FieldCount"); });

   // Fields
   Fields.initCallbacks(
      [=] ()                        { return &fc; },
      [=] (FieldsClass *const &pfc) { throw Error("Cannot set Fields"); });

   // SelectSQL
   SelectSQL.initCallbacks(
      [=] ()                            { return &sql; },
      [=] (SelectSQLClass *const &psql) { throw Error("Cannot set SelectSQL"); });

   // Transaction
   Transaction.initCallbacks(
      [=] ()                          { return VIBDataSet_GetTransaction(vds); },
      [=] (VIBTransaction *const &tr) { VIBSAFECALL(VIBDataSet_SetTransaction(vds, tr)); });

   // UniDirectional
   UniDirectional.initCallbacks(
      [=] ()              { return VIBDataSet_GetUniDirectional(vds); },
      [=] (const bool &b) { VIBSAFECALL(VIBDataSet_SetUniDirectional(vds, b)); });
}

//
// Destructor
//
VIB::DataSet::~DataSet()
{
   if(vds)
   {
      VIBDataSet_Destroy(vds);
      vds = nullptr;
   }
}

void VIB::DataSet::Open()
{
   VIBSAFECALL(VIBDataSet_Open(vds));
}

void VIB::DataSet::First()
{
   VIBSAFECALL(VIBDataSet_First(vds));
}

void VIB::DataSet::Next()
{
   VIBSAFECALL(VIBDataSet_Next(vds));
}

void VIB::DataSet::Close()
{
   VIBSAFECALL(VIBDataSet_Close(vds));
}

void VIB::DataSet::Prepare()
{
   VIBSAFECALL(VIBDataSet_Prepare(vds));
}

bool VIB::DataSet::Eof()
{
   return VIBDataSet_Eof(vds);
}

std::string VIB::DataSet::Plan()
{
   return ElideVIBString(VIBDataSet_Plan(vds));
}

VIB::DataSet::FieldByNameClass *VIB::DataSet::FieldByName(const std::string &fieldname)
{
   fbnc.fieldname = fieldname;
   return &fbnc;
}

// EOF

