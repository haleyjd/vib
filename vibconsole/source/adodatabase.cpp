/*
   NDWCoalesce

   ADO Database Abstraction
*/

#ifndef VIBC_NO_ADODB

#include <memory>
#include <exception>
#include <cstdio>

#include <Windows.h>
#include "adodatabase.h"
#include "util.h"

#import "msado15.dll" rename("EOF", "ADOEOF")

//=============================================================================
//
// ADORecordSet Private Implementation Details
//

class ADORecordSetPimpl
{
public:
   ADOReadOnlyDB *db;
   ADODB::_RecordsetPtr recordset;
};

//=============================================================================
//
// ADOCommand Private Implementation Details
//

class ADOCommandPimpl
{
public:
   ADOReadOnlyDB *db;
   ADODB::_CommandPtr command;
};

//=============================================================================
// 
// ADOReadOnlyDB Private Implementation Details
//

class ADOReadOnlyDBPimpl
{
public:
   ADODB::_ConnectionPtr connection;
};

//=============================================================================
//
// ADORecordSet Implementation
//

//
// Protected Constructor
//
// ADOReadOnlyDB can construct this class using this constructor.
//
ADORecordSet::ADORecordSet(ADOReadOnlyDB *db)
   : pImpl(nullptr)
{
   HRESULT hr;

   pImpl = new ADORecordSetPimpl;
   pImpl->db = db;

   if(FAILED(hr = pImpl->recordset.CreateInstance(__uuidof(ADODB::Recordset))))
      throw _com_error(hr);
}

//
// Protected Constructor
//
// This is for ADOCommand. The ADORecordSet will end up wrapping an instance of
// ADODB::Recordset as returned by ADODB::Command::Execute, rather than one it
// instantiated itself.
//
ADORecordSet::ADORecordSet()
{
   pImpl = new ADORecordSetPimpl;
}

//
// Destructor
//
ADORecordSet::~ADORecordSet()
{
   if(pImpl)
   {
      try
      {
         delete pImpl;
      }
      catch(...)
      {
         ADOReadOnlyDB::PrintError("Failed to destroy a record set");
      }
      pImpl = nullptr;
   }
}

//
// ADORecordSet::open
//
// Open a new recordset using a SQL query.
// The recordset is always unidirectional and read-only.
//
HRESULT ADORecordSet::open(const char *query)
{
   ADOReadOnlyDBPimpl *dbpImpl = pImpl->db->pImpl;

   return pImpl->recordset->Open(query, dbpImpl->connection.GetInterfacePtr(),
      ADODB::adOpenForwardOnly, ADODB::adLockReadOnly, ADODB::adCmdText);
}

//
// ADORecordSet::close
//
// Explicitly close an ADORecordSet.
//
HRESULT ADORecordSet::close()
{
   return pImpl->recordset->Close();
}

//
// ADORecordSet::atEOF
//
// Return true if recordset is at EOF
//
bool ADORecordSet::atEOF()
{
   return !!(pImpl->recordset->ADOEOF);
}

//
// ADORecordSet::next
//
// Move to the next record in the dataset.
//
HRESULT ADORecordSet::next()
{
   return pImpl->recordset->MoveNext();
}

//
// DATEToString
//
// Convert ADO DATE representation into a Firebird-acceptable timestamp string.
//
static std::string DATEToString(DATE &date)
{
   SYSTEMTIME st;

   if(VariantTimeToSystemTime(date, &st))
   {
      Pstamp stamp;

      stamp.year   = st.wYear;
      stamp.month  = st.wMonth;
      stamp.day    = st.wDay;
      stamp.hour   = st.wHour;
      stamp.minute = st.wMinute;
      stamp.second = st.wSecond;
      stamp.millisecond = st.wMilliseconds;

      if(stamp.IsValidStamp())
         return stamp.OutStamp(global_mode);
   }

   return "null";
}

//
// DECIMALToDoubleStr
//
// Convert an ADO DECIMAL value to a string representation. Loss of precision
// may be implied.
//
static std::string DECIMALToDoubleStr(DECIMAL &d)
{
   long double dbl = 
      ((d.sign & DECIMAL_NEG) ? -1.0 : 1.0) * 
      (long double(d.Lo64) + long double(d.Hi32) * long double(1ULL<<32) * long double(1ULL<<32)) *
      pow(10.0, -d.scale);

   return std::to_string(dbl);
}

//
// VariantToString
//
// Attempt to convert an ADO variant to string. Various field types are
// explicitly supported. For ones that are not, the default conversion 
// will be attempted; if it throws, the return value will be 
// "Unsupported type".
//
static std::string VariantToString(_variant_t &var)
{
   std::string str;

   switch(var.vt)
   {
   // INTEGER TYPES
   case VT_UI1:
      str = UIntToString(var.bVal);
      break;
   case VT_I2:
      str = IntToString(var.iVal);
      break;
   case VT_I4:
      str = IntToString(var.lVal);
      break;
   case VT_BOOL:
      str = IntToString(var.boolVal);
      break;

   // DATE TYPES
   case VT_DATE:
      str = DATEToString(var.date);
      break;

   // FLOAT TYPES
   case VT_R4:
      str = std::to_string((long double)var.fltVal);
      break;
   case VT_R8:
      str = std::to_string((long double)var.dblVal);
      break;
   case VT_DECIMAL:
      str = DECIMALToDoubleStr(var.decVal);
      break;

   // STRING TYPES
   case VT_BSTR:
      str = _bstr_t(var.bstrVal);
      break;

   // Special cases
   case VT_NULL:
      str = "";
      break;
   case VT_ERROR:
      str = "error";
      break;
   case VT_UNKNOWN:
      str = "unknown";
      break;
   default:
      // try to convert it directly...
      try
      {
         var.ChangeType(VT_BSTR);
         str = _bstr_t(var.bstrVal);
      }
      catch(...)
      {
         str = "Unsupported type";
      }
      break;
   }

   return StripSurrounding(str, " ");
}

//
// ADORecordSet::getValue
//
std::string ADORecordSet::getValue(const char *fieldName)
{
   auto var = pImpl->recordset->Fields->GetItem(fieldName)->GetValue();
   
   return VariantToString(var);
}

//
// ADORecordSet::getMap
//
void ADORecordSet::getMap(std::map<std::string, std::string> &map)
{
   long limit = pImpl->recordset->GetFields()->Count;
   for(long i = 0; i < limit; i++)
   {
      std::string name = pImpl->recordset->GetFields()->Item[i]->Name;
      auto var = pImpl->recordset->GetFields()->Item[i]->GetValue();
      map[name] = VariantToString(var);
   }
}

//
// ADORecordSet::getVecMap
//
void ADORecordSet::getVecMap(std::vector<std::map<std::string, std::string>> &vecmap)
{
   while(!atEOF())
   {
      vecmap.push_back(std::map<std::string, std::string>());
      getMap(vecmap.back());
      next();
   }
}

//=============================================================================
//
// ADO Command Wrapper 
//

//
// Protected Constructor
// Only ADOReadOnlyDB can instantiate this class.
//
ADOCommand::ADOCommand(ADOReadOnlyDB *db, const char *cmdText, bool prepared)
{
   HRESULT hr;

   pImpl = new ADOCommandPimpl;
   pImpl->db = db;

   if(FAILED(hr = pImpl->command.CreateInstance(__uuidof(ADODB::Command))))
      throw _com_error(hr);

   pImpl->command->ActiveConnection = db->pImpl->connection;
   pImpl->command->CommandText      = cmdText;
   pImpl->command->PutPrepared(prepared);
}

//
// Destructor
//
ADOCommand::~ADOCommand()
{
   if(pImpl)
   {
      try
      {
         delete pImpl;
      }
      catch(...)
      {
         ADOReadOnlyDB::PrintError("Failed to destroy a command");
      }
      pImpl = nullptr;
   }
}

//
// ADOCommand_AddParameter
//
// Template to add a parameter to an ADO Command object
//
template<typename T>
static HRESULT ADOCommand_AddParameter(ADOCommandPimpl *cmdp, T val, ADODB::DataTypeEnum dte)
{
   auto param = cmdp->command->CreateParameter(L"", dte, ADODB::adParamInput, -1, _variant_t(val));
   return cmdp->command->Parameters->Append(param);
}

//
// ADOCommand::addParameter(bool)
//
// Add a boolean parameter.
//
HRESULT ADOCommand::addParameter(bool b)
{
   return ADOCommand_AddParameter<bool>(pImpl, b, ADODB::adBoolean);
}
 
//
// ADOCommand::addParameter(double)
//
// Add a floating point parameter.
//
HRESULT ADOCommand::addParameter(double d)
{
   return ADOCommand_AddParameter<double>(pImpl, d, ADODB::adDouble);
}
   
//
// ADOCommand::addParameter(int)
//
// Add an integer parameter.
//
HRESULT ADOCommand::addParameter(int i)
{
   return ADOCommand_AddParameter<int>(pImpl, i, ADODB::adInteger);
}
  
//
// ADOCommand::addParameter(unsigned int)
//
// Add an unsigned integer parameter.
//
HRESULT ADOCommand::addParameter(unsigned int ui)
{
   return ADOCommand_AddParameter<unsigned int>(pImpl, ui, ADODB::adUnsignedInt);
}
   
//
// ADOCommand::addParameter(const char *)
//
// Add a string parameter.
//
HRESULT ADOCommand::addParameter(const char *ccp)
{
   return ADOCommand_AddParameter<_bstr_t>(pImpl, ccp, ADODB::adBSTR);
}

//
// ADOCommand::clearParameters
//
// Delete all parameters added to the prepared query.
//
HRESULT ADOCommand::clearParameters()
{
   long count = pImpl->command->Parameters->GetCount() - 1;

   for(auto i = count; i >= 0; i--)
   {
      HRESULT hr;
      if(FAILED(hr = pImpl->command->Parameters->Delete(i)))
         return hr;
   }

   return S_OK;
}

//
// ADOCommand::execute
//
// Execute a command, without parameters.
//
ADORecordSetPtr ADOCommand::execute()
{
   ADORecordSetPtr rs(new ADORecordSet());

   rs->pImpl->db        = pImpl->db;
   rs->pImpl->recordset = pImpl->command->Execute(nullptr, nullptr, ADODB::adCmdText);

   return rs;
}

//=============================================================================
//
// ADO Read-Only Database Implementation
//

//
// ADOReadOnlyDB::InitADO
//
// Static method; do one-time ADO setup.
//
bool ADOReadOnlyDB::InitADO()
{
   static bool adoInitialized = false;

   if(!adoInitialized)
   {
      if(FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
         return false;
      adoInitialized = true;
   }
   return true;
}

//
// Constructor
//
ADOReadOnlyDB::ADOReadOnlyDB()
   : pImpl(nullptr)
{
   HRESULT hr;

   // Ensure one-time ADO init is complete
   if(!InitADO())
      throw std::exception("Could not complete ADO one-time initialization");

   // Instantiate private implementation
   pImpl = new ADOReadOnlyDBPimpl();

   if(FAILED((hr = pImpl->connection.CreateInstance(__uuidof(ADODB::Connection)))))
      throw _com_error(hr);

   pImpl->connection->CursorLocation = ADODB::adUseClient;
}

//
// Destructor
//
ADOReadOnlyDB::~ADOReadOnlyDB()
{
   if(pImpl)
   {
      try
      {
         delete pImpl;
      }
      catch(...)
      {
         ADOReadOnlyDB::PrintError("Failed to destroy a database object");
      }
      pImpl = nullptr;
   }
}

//
// ADOReadOnlyDB::open
//
// Open a database connection.
//
HRESULT ADOReadOnlyDB::open(const char *connectionStr)
{
   return pImpl->connection->Open(connectionStr, L"", L"", ADODB::adConnectUnspecified);
}

//
// ADOReadOnlyDB::close
//
// Explicitly close an ADODB connection.
//
HRESULT ADOReadOnlyDB::close()
{
   return pImpl->connection->Close();
}

//
// ADOReadOnlyDB::getRecordSet
//
// Get a record set associated with this database.
//
ADORecordSetPtr ADOReadOnlyDB::getRecordSet()
{
   return ADORecordSetPtr(new ADORecordSet(this));
}

//
// ADOReadOnlyDB::getCommand
//
// Get a preparable command.
//
ADOCommandPtr ADOReadOnlyDB::getCommand(const char *sqlCmd, bool prepared)
{
   return ADOCommandPtr(new ADOCommand(this, sqlCmd, prepared));
}

//
// ADOReadOnlyDB::getCommandFile
//
// Load a text file and use it as a SQL command.
//
ADOCommandPtr ADOReadOnlyDB::getCommandFile(const char *filename, bool prepared)
{
   char *text = LoadTextFile(filename);
   if(!text)
      throw std::exception("Can't open file in ADOReadOnlyDB::getCommandFile");
   std::unique_ptr<char []> txtFile(text);

   return ADOCommandPtr(new ADOCommand(this, txtFile.get(), prepared));
}

//
// ADOReadOnlyDB::execute
//
void ADOReadOnlyDB::execute(const char *str)
{
   pImpl->connection->Execute(str, nullptr, ADODB::adOptionUnspecified);
}

//
// ADOReadOnlyDB::PrintError
//
// Static method.
// Print information about a thrown COM error.
//
void ADOReadOnlyDB::PrintError(_com_error &e)
{
   _bstr_t bstrSource(e.Source());
   _bstr_t bstrDescrp(e.Description());

   std::printf("ADO Exception caught:\n"
               " Code    = %08lx\n"
               " Meaning = %s\n"
               " Source  = %s\n"
               " Descrip = %s\n",
               e.Error(), e.ErrorMessage(), 
               (char *)bstrSource, (char *)bstrDescrp);
   
}

void ADOReadOnlyDB::PrintError(const char *msg)
{
   std::printf("Exception caught:\n%s\n", msg);
}

#endif // VIBC_NO_ADODB

// EOF

