/** @file vibdataset.cpp
 *
 * VisualIB - InterBase wrapper for use in Visual Studio.
 * TIBDataSet Wrapper
 * @author James Haley
 *
 */

#include "vibinlines.h"

#include "vibstring.h"
#include "vibexception.h"
//
// VIBDataSet_New
//
VIBDataSet *VIBCALL VIBDataSet_New()
{
   VIBDataSet *newds = NULL;

   try
   {
      TIBDataSet *tds = new TIBDataSet(NULL);

      if(tds)
      {
         newds = new VIBDataSet;
         newds->opaque = tds;
         newds->isWeak = VIBFALSE;
      }
   }
   CATCH_EIBERROR

   return newds;
}

//
// VIBDataSet_Destroy
//
void VIBCALL VIBDataSet_Destroy(VIBDataSet *vds)
{
   if(!vds->isWeak)
   {
      try
      {
         TIBDataSet *tds = TDSForVDS(vds);
         delete tds;
      }
      CATCH_EIBERROR
   }
   vds->opaque = NULL;
   delete vds;
}

//
// VIBDataSet_SetDatabase
//
VIBBOOL VIBCALL VIBDataSet_SetDatabase(VIBDataSet *vds, VIBDatabase *vdb)
{
   TIBDataSet  *tds = TDSForVDS(vds);
   TIBDatabase *tdb = TDBForVDB(vdb);

   try
   {
      tds->Database = tdb;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDataSet_GetDatabase
//
VIBDatabase *VIBCALL VIBDataSet_GetDatabase(VIBDataSet *vds)
{
   VIBDatabase *vdb = NULL;
   TIBDataSet  *tds = TDSForVDS(vds);

   try
   {
      TIBDatabase *tdb = tds->Database;

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
// VIBDataSet_SetTransaction
//
VIBBOOL VIBCALL VIBDataSet_SetTransaction(VIBDataSet *vds, VIBTransaction *vtr)
{
   TIBDataSet     *tds = TDSForVDS(vds);
   TIBTransaction *ttr = TTRForVTR(vtr);

   try
   {
      tds->Transaction = ttr;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDataSet_GetTransaction
//
VIBTransaction *VIBCALL VIBDataSet_GetTransaction(VIBDataSet *vds)
{
   VIBTransaction *vtr = NULL;
   TIBDataSet     *tds = TDSForVDS(vds);

   try
   {
      TIBTransaction *ttr = tds->Transaction;

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
// VIBDataSet_SetUniDirectional
//
VIBBOOL VIBCALL VIBDataSet_SetUniDirectional(VIBDataSet *vds, VIBBOOL UniDirectional)
{
   TIBDataSet *tds = TDSForVDS(vds);

   try
   {
      tds->UniDirectional = (UniDirectional ? true : false);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDataSet_GetUniDirectional
//
VIBBOOL VIBCALL VIBDataSet_GetUniDirectional(VIBDataSet *vds)
{
   TIBDataSet *tds = TDSForVDS(vds);
   VIBBOOL res = VIBFALSE;

   try
   {
      bool cppres = tds->UniDirectional;

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBDataSet_SelectSQL_Clear
//
VIBBOOL VIBCALL VIBDataSet_SelectSQL_Clear(VIBDataSet *vds)
{
   TIBDataSet *tds = TDSForVDS(vds);
   
   try
   {
      tds->SelectSQL->Clear();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDataSet_SelectSQL_Add
//
VIBBOOL VIBCALL VIBDataSet_SelectSQL_Add(VIBDataSet *vds, const char *sql)
{
   TIBDataSet *tds = TDSForVDS(vds);
   
   try
   {
      tds->SelectSQL->Add(sql);
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDataSet_SelectSQL_GetText
//
VIBString *VIBCALL VIBDataSet_SelectSQL_GetText(VIBDataSet *vds)
{
   TIBDataSet *tds = TDSForVDS(vds);
   VIBString  *ret = NULL;
   
   try
   {
      AnsiString sql = tds->SelectSQL->Text;

      ret = VIBString_New(sql.c_str());
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBDataSet_SelectSQL_SetText
//
VIBBOOL VIBCALL VIBDataSet_SelectSQL_SetText(VIBDataSet *vds, const char *sql)
{
   TIBDataSet *tds = TDSForVDS(vds);
   
   try
   {
      tds->SelectSQL->Text = sql;
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDataSet_Eof
//
VIBBOOL VIBCALL VIBDataSet_Eof(VIBDataSet *vds)
{
   TIBDataSet *tds = TDSForVDS(vds);
   VIBBOOL res = VIBTRUE;

   try
   {
      bool cppres = tds->Eof;

      res = (cppres ? VIBTRUE : VIBFALSE);
   }
   CATCH_EIBERROR

   return res;
}

//
// VIBDataSet_Open
//
VIBBOOL VIBCALL VIBDataSet_Open(VIBDataSet *vds)
{
   TIBDataSet *tds = TDSForVDS(vds);

   try
   {
      tds->Open();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDataSet_First
//
VIBBOOL VIBCALL VIBDataSet_First(VIBDataSet *vds)
{
   TIBDataSet *tds = TDSForVDS(vds);

   try
   {
      tds->First();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDataSet_Next
//
VIBBOOL VIBCALL VIBDataSet_Next(VIBDataSet *vds)
{
   TIBDataSet *tds = TDSForVDS(vds);

   try
   {
      tds->Next();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDataSet_Close
//
VIBBOOL VIBCALL VIBDataSet_Close(VIBDataSet *vds)
{
   TIBDataSet *tds = TDSForVDS(vds);

   try
   {
      tds->Close();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDataSet_Prepare
//
VIBBOOL VIBCALL VIBDataSet_Prepare(VIBDataSet *vds)
{
   TIBDataSet *tds = TDSForVDS(vds);

   try
   {
      tds->Prepare();
   }
   CATCH_EIBERROR_RF

   return VIBTRUE;
}

//
// VIBDataSet_Plan
//
VIBString *VIBCALL VIBDataSet_Plan(VIBDataSet *vds)
{
   TIBDataSet *tds = TDSForVDS(vds);
   VIBString  *ret = NULL;
   
   try
   {
      AnsiString plan = tds->Plan;

      ret = VIBString_New(plan.c_str());
   }
   CATCH_EIBERROR

   return ret;
}

//
// VIBDataSet_Fields_Count
//
int VIBCALL VIBDataSet_Fields_Count(VIBDataSet *vds)
{
   TIBDataSet *tds = TDSForVDS(vds);
   int count = 0;

   try
   {
      count = tds->Fields->Count;
   }
   CATCH_EIBERROR

   return count;
}

//
// VIBDataSet_Fields_AsString
//
VIBString *VIBCALL VIBDataSet_Fields_AsString(VIBDataSet *vds, int Index)
{
   TIBDataSet *tds  = TDSForVDS(vds);
   VIBString  *vstr = NULL;

   try
   {
      AnsiString str = tds->Fields->Fields[Index]->AsString;

      vstr = VIBString_New(str.c_str());
   }
   CATCH_EIBERROR

   return vstr;
}

//
// VIBDataSet_Fields_FieldName
//
VIBString *VIBCALL VIBDataSet_Fields_FieldName(VIBDataSet *vds, int Index)
{
   TIBDataSet *tds  = TDSForVDS(vds);
   VIBString  *vstr = NULL;

   try
   {
      AnsiString str = tds->Fields->Fields[Index]->FieldName;

      vstr = VIBString_New(str.c_str());
   }
   CATCH_EIBERROR

   return vstr;
}

//
// VIBDataSet_FieldByName_AsString
//
VIBString *VIBCALL VIBDataSet_FieldByName_AsString(VIBDataSet *vds, const char *fieldname)
{
   TIBDataSet *tds  = TDSForVDS(vds);
   VIBString  *vstr = NULL;

   try
   {
      AnsiString fn  = fieldname;
      AnsiString str = tds->FieldByName(fn)->AsString;

      vstr = VIBString_New(str.c_str());
   }
   CATCH_EIBERROR

   return vstr;
}

//
// VIBDataSet_FieldCount
//
int VIBCALL VIBDataSet_FieldCount(VIBDataSet *vds)
{
   int count = 0;

   try
   {
      count = TDSForVDS(vds)->FieldCount;
   }
   CATCH_EIBERROR

   return count;
}

//
// VIBDataSet_Fields_DataType
//
VIBFieldType VIBCALL VIBDataSet_Fields_DataType(VIBDataSet *vds, int Idx)
{
   VIBFieldType fieldType = vib_ftUnknown;

   try
   {
      TFieldType tft = TDSForVDS(vds)->Fields->Fields[Idx]->DataType;

      switch(tft)
      {
      case ftUnknown:     fieldType = vib_ftUnknown;     break;
      case ftString:      fieldType = vib_ftString;      break;
      case ftSmallint:    fieldType = vib_ftSmallint;    break;
      case ftInteger:     fieldType = vib_ftInteger;     break;
      case ftWord:        fieldType = vib_ftWord;        break;
      case ftBoolean:     fieldType = vib_ftBoolean;     break;
      case ftFloat:       fieldType = vib_ftFloat;       break;
      case ftCurrency:    fieldType = vib_ftCurrency;    break;
      case ftBCD:         fieldType = vib_ftBCD;         break;
      case ftDate:        fieldType = vib_ftDate;        break;
      case ftTime:        fieldType = vib_ftTime;        break;
      case ftDateTime:    fieldType = vib_ftDateTime;    break;
      case ftBytes:       fieldType = vib_ftBytes;       break;
      case ftVarBytes:    fieldType = vib_ftVarBytes;    break;
      case ftAutoInc:     fieldType = vib_ftAutoInc;     break;
      case ftBlob:        fieldType = vib_ftBlob;        break;
      case ftMemo:        fieldType = vib_ftMemo;        break;
      case ftGraphic:     fieldType = vib_ftGraphic;     break;
      case ftFmtMemo:     fieldType = vib_ftFmtMemo;     break;
      case ftParadoxOle:  fieldType = vib_ftParadoxOle;  break;
      case ftDBaseOle:    fieldType = vib_ftDBaseOle;    break;
      case ftTypedBinary: fieldType = vib_ftTypedBinary; break;
      case ftCursor:      fieldType = vib_ftCursor;      break;
      case ftFixedChar:   fieldType = vib_ftFixedChar;   break;
      case ftWideString:  fieldType = vib_ftWideString;  break;
      case ftLargeint:    fieldType = vib_ftLargeint;    break;
      case ftADT:         fieldType = vib_ftADT;         break;
      case ftArray:       fieldType = vib_ftArray;       break;
      case ftReference:   fieldType = vib_ftReference;   break;
      case ftDataSet:     fieldType = vib_ftDataSet;     break;
      case ftOraBlob:     fieldType = vib_ftOraBlob;     break;
      case ftOraClob:     fieldType = vib_ftOraClob;     break;
      case ftVariant:     fieldType = vib_ftVariant;     break;
      case ftInterface:   fieldType = vib_ftInterface;   break;
      case ftIDispatch:   fieldType = vib_ftIDispatch;   break;
      case ftGuid:        fieldType = vib_ftGuid;        break;
      }
   }
   CATCH_EIBERROR

   return fieldType;
}

//
// VIBDataSet_Fields_FieldKind
//
VIBFieldKind VIBCALL VIBDataSet_Fields_FieldKind(VIBDataSet *vds, int Idx)
{
   VIBFieldKind kind = vib_fkData;

   try
   {
      TFieldKind tfk = TDSForVDS(vds)->Fields->Fields[Idx]->FieldKind;

      switch(tfk)
      {
      case fkData:         kind = vib_fkData;         break;
      case fkCalculated:   kind = vib_fkCalculated;   break;
      case fkLookup:       kind = vib_fkLookup;       break;
      case fkInternalCalc: kind = vib_fkInternalCalc; break;
      }
   }
   CATCH_EIBERROR

   return kind;
}

// EOF

