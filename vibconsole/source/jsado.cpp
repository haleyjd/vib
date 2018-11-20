/*
   JS Wrappers for ADO Database Engine
*/

#ifndef VIBC_NO_ADODB

#include "jsengine2.h"
#include "jsnatives.h"

#include "adodatabase.h"

//=============================================================================
//
// ADORecordSet Wrapper
//

class PrivateADORecordSet : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   ADORecordSetPtr rs;

   PrivateADORecordSet(ADORecordSetPtr prs) : PrivateData(), rs(std::move(prs))
   {
   }

   static void NewAsReturnVal(JSContext *cx, jsval *vp, ADORecordSetPtr rsp);
};

static void ADORecordSet_Finalize(JSContext *cx, JSObject *obj)
{
   auto priv = PrivateData::GetFromJSObject<PrivateADORecordSet>(cx, obj);

   if(priv)
   {
      delete priv;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

/**
 * JSClass for ADORecordSet
 */
static JSClass adoRecordSetClass =
{
   "ADORecordSet",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   ADORecordSet_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivateADORecordSet, adoRecordSetClass)

//
// ADORecordSet_Open
//
// Wrapper method for ADORecordSet::open
//
static JSBool ADORecordSet_Open(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   JSBool res  = JS_FALSE;

   auto priv = PrivateData::MustGetFromThis<PrivateADORecordSet>(cx, vp);

   if(argc >= 1)
   {
      const char *query = SafeGetStringBytes(cx, argv[0], &argv[0]);

      try
      {
         res = (priv->rs->open(query) == S_OK ? JS_TRUE : JS_FALSE);
      }
      catch(_com_error &err)
      {
         ADOReadOnlyDB::PrintError(err);
      }
      catch(...)
      {
      }
   }

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
   return JS_TRUE;
}

//
// ADORecordSet_Close
//
// Wrapper method for ADORecordSet::close
//
static JSBool ADORecordSet_Close(JSContext *cx, uintN argc, jsval *vp)
{
   auto   priv = PrivateData::GetFromThis<PrivateADORecordSet>(cx, vp);
   JSBool res  = JS_FALSE;

   if(!priv)
      return JS_FALSE;

   try
   {
      res = (priv->rs->close() == S_OK ? JS_TRUE : JS_FALSE);
   }
   catch(_com_error &err)
   {
      ADOReadOnlyDB::PrintError(err);
   }
   catch(...)
   {
   }

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
   return JS_TRUE;
}

//
// ADORecordSet_AtEOF
//
// Wrapper method for ADORecordSet::atEOF
//
static JSBool ADORecordSet_AtEOF(JSContext *cx, uintN argc, jsval *vp)
{
   auto   priv = PrivateData::GetFromThis<PrivateADORecordSet>(cx, vp);
   JSBool res  = JS_FALSE;

   if(!priv)
      return JS_FALSE;

   try
   {
      res = (priv->rs->atEOF() ? JS_TRUE : JS_FALSE);
   }
   catch(_com_error &err)
   {
      ADOReadOnlyDB::PrintError(err);
   }
   catch(...)
   {
   }

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
   return JS_TRUE;
}

//
// ADORecordSet_Next
//
// Wrapper method for ADORecordSet::next
//
static JSBool ADORecordSet_Next(JSContext *cx, uintN argc, jsval *vp)
{
   auto   priv = PrivateData::GetFromThis<PrivateADORecordSet>(cx, vp);
   JSBool res  = JS_FALSE;

   if(!priv)
      return JS_FALSE;

   try
   {
      res = (priv->rs->next() == S_OK ? JS_TRUE : JS_FALSE);
   }
   catch(_com_error &err)
   {
      ADOReadOnlyDB::PrintError(err);
   }
   catch(...)
   {
   }

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
   return JS_TRUE;
}

//
// ADORecordSet_GetValue
//
// Wrapper method for ADORecordSet::getValue
//
static JSBool ADORecordSet_GetValue(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto priv = PrivateData::MustGetFromThis<PrivateADORecordSet>(cx, vp);

   if(argc >= 1)
   {
      try
      {  
         const char *fieldName = SafeGetStringBytes(cx, argv[0], &argv[0]);
         std::string res = priv->rs->getValue(fieldName);

         JSString *jres = AssertJSNewStringCopyZ(cx, res.c_str());
         JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jres));
         return JS_TRUE;
      }
      catch(_com_error &err)
      {
         ADOReadOnlyDB::PrintError(err);
      }
      catch(...)
      {
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

//
// ADORecordSet_GetMap
//
// Wrapper method for ADORecordSet::getMap
//
static JSBool ADORecordSet_GetMap(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivateADORecordSet>(cx, vp);

   if(!priv)
      return JS_FALSE;

   try
   {
      std::map<std::string, std::string> mapstrs;
      priv->rs->getMap(mapstrs);
      LazyStringMap_ReturnObject(cx, vp, mapstrs);
      return JS_TRUE;
   }
   catch(_com_error &err)
   {
      ADOReadOnlyDB::PrintError(err);
   }
   catch(...)
   {
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

//
// ADORecordSet_GetVecMap
//
// Wrapper method for ADORecordSet::getVecMap
//
static JSBool ADORecordSet_GetVecMap(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivateADORecordSet>(cx, vp);

   if(!priv)
      return JS_FALSE;

   try
   {
      std::vector<std::map<std::string, std::string>> vecmap;
      priv->rs->getVecMap(vecmap);
      LazyVecMap_ReturnObject(cx, vp, vecmap);
      return JS_TRUE;
   }
   catch(_com_error &err)
   {
      ADOReadOnlyDB::PrintError(err);
   }
   catch(...)
   {
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

/**
 * ADORecordSet JS Method Table
 */
static JSFunctionSpec adoRecordSetFuncs[] =
{
   JSE_FN("open",      ADORecordSet_Open,      0, 0, 0),
   JSE_FN("close",     ADORecordSet_Close,     0, 0, 0),
   JSE_FN("atEOF",     ADORecordSet_AtEOF,     0, 0, 0),
   JSE_FN("next",      ADORecordSet_Next,      0, 0, 0),
   JSE_FN("getValue",  ADORecordSet_GetValue,  1, 0, 0),
   JSE_FN("getMap",    ADORecordSet_GetMap,    0, 0, 0),
   JSE_FN("getVecMap", ADORecordSet_GetVecMap, 0, 0, 0),
   JS_FS_END
};

void PrivateADORecordSet::NewAsReturnVal(JSContext *cx, jsval *vp, ADORecordSetPtr rsp)
{  
   try
   {
      JSObject *newObj = AssertJSNewObject(cx, &adoRecordSetClass, nullptr, nullptr);
      AutoNamedRoot anr(cx, newObj, "NewADORecordSet");
      JS_DefineFunctions(cx, newObj, adoRecordSetFuncs);
      std::unique_ptr<PrivateADORecordSet> prs(new PrivateADORecordSet(std::move(rsp)));
      prs->setToJSObjectAndRelease(cx, newObj, prs);
      JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newObj));
   }
   catch(const JSEngineError &)
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
   }
}

//=============================================================================
//
// ADOCommand Wrapper
//

class PrivateADOCommand : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   ADOCommandPtr cmd;

   PrivateADOCommand(ADOCommandPtr pcmd) : PrivateData(), cmd(std::move(pcmd))
   {
   }

   static void NewAsReturnVal(JSContext *cx, jsval *vp, ADOCommandPtr rsp);
};

static void ADOCommand_Finalize(JSContext *cx, JSObject *obj)
{
   auto priv = PrivateData::GetFromJSObject<PrivateADOCommand>(cx, obj);

   if(priv)
   {
      delete priv;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

/**
 * JSClass for ADOCommand
 */
static JSClass adoCommandClass =
{
   "ADOCommand",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   ADORecordSet_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivateADOCommand, adoCommandClass)

//
// ADOCommand_Execute
//
// Wrapper for ADOCommand::execute
//
static JSBool ADOCommand_Execute(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivateADOCommand>(cx, vp);

   try
   {
      PrivateADORecordSet::NewAsReturnVal(cx, vp, priv->cmd->execute());
      return JS_TRUE;
   }
   catch(_com_error &err)
   {
      ADOReadOnlyDB::PrintError(err);
   }
   catch(...)
   {
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}


// TODO:
#if 0
   /**
    * Add a boolean parameter to a prepared query.
    * @return COM HRESULT; S_OK if successful, error code otherwise.
    */
   HRESULT addParameter(bool);
   /**
    * Add a double floating-point parameter to a prepared query.
    * @return COM HRESULT; S_OK if successful, error code otherwise.
    */
   HRESULT addParameter(double);
   /**
    * Add an integer parameter to a prepared query.
    * @return COM HRESULT; S_OK if successful, error code otherwise.
    */
   HRESULT addParameter(int);
   /**
    * Add an unsigned integer parameter to a prepared query.
    * @return COM HRESULT; S_OK if successful, error code otherwise.
    */
   HRESULT addParameter(unsigned int);
   /**
    * Add a string parameter to a prepared query.
    * @return COM HRESULT; S_OK if successful, error code otherwise.
    */
   HRESULT addParameter(const char *);
   /**
    * Remove all parameters from a prepared query.
    */
   HRESULT clearParameters();
#endif

/**
 * ADOCommand JS Method Table
 */
static JSFunctionSpec adoCommandFuncs[] =
{
   JSE_FN("execute", ADOCommand_Execute, 0, 0, 0),
   JS_FS_END
};

void PrivateADOCommand::NewAsReturnVal(JSContext *cx, jsval *vp, ADOCommandPtr cmd)
{   
   try
   {
      JSObject *newObj = AssertJSNewObject(cx, &adoCommandClass, nullptr, nullptr);
      AutoNamedRoot anr(cx, newObj, "NewADOCommand");
      JS_DefineFunctions(cx, newObj, adoCommandFuncs);
      std::unique_ptr<PrivateADOCommand> pcmd(new PrivateADOCommand(std::move(cmd)));
      pcmd->setToJSObjectAndRelease(cx, newObj, pcmd);
      JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newObj));
   }
   catch(const JSEngineError &)
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
   }
}

//=============================================================================
//
// ADOReadOnlyDB Wrapper
//

class PrivateADOReadOnlyDB : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   ADOReadOnlyDB db;

   PrivateADOReadOnlyDB() : PrivateData(), db()
   {
   }
};

//
// ADOReadOnlyDB_New - JS Constructor
//
// Construct a C++ ADOReadOnlyDB and wrap it in the new JavaScript object's
// private data field.
//
static JSBool ADOReadOnlyDB_New(JSContext *cx, JSObject *obj, uintN argc,
                               jsval *argv, jsval *rval)
{
   if(!JS_IsConstructing(cx))
      return JS_FALSE;

   try
   {
      std::unique_ptr<PrivateADOReadOnlyDB> db(new PrivateADOReadOnlyDB());
      db->setToJSObjectAndRelease(cx, obj, db);
   }
   catch(...)
   {
      JS_ReportError(cx, "Cannot construct ADOReadOnlyDB, native failure");
      return JS_FALSE;
   }

   *rval = JSVAL_VOID;
   return JS_TRUE;
}

//
// ADOReadOnlyDB_Finalize - Class Finalization Hook
//
// Destroy the C++ ADOReadOnlyDB instance before the GC releases the JS proxy.
//
static void ADOReadOnlyDB_Finalize(JSContext *cx, JSObject *obj)
{
   auto priv = PrivateData::GetFromJSObject<PrivateADOReadOnlyDB>(cx, obj);

   if(priv)
   {
      delete priv;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

/**
 * JSClass for ADOReadOnlyDB
 */
static JSClass adoReadOnlyDBClass =
{
   "ADOReadOnlyDB",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   ADOReadOnlyDB_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivateADOReadOnlyDB, adoReadOnlyDBClass)

//
// ADOReadOnlyDB_Open
//
// Wrapper method for ADOReadOnlyDB::open
//
static JSBool ADOReadOnlyDB_Open(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   JSBool res  = JS_FALSE;

   auto priv = PrivateData::MustGetFromThis<PrivateADOReadOnlyDB>(cx, vp);

   if(argc >= 1)
   {
      const char *connectionStr = SafeGetStringBytes(cx, argv[0], &argv[0]);
      try
      {
         res = (priv->db.open(connectionStr) == S_OK ? JS_TRUE : JS_FALSE);
      }
      catch(_com_error &cerr)
      {
         ADOReadOnlyDB::PrintError(cerr);
      }
      catch(...)
      {
      }
   }

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
   return JS_TRUE;
}

//
// ADOReadOnlyDB_Close
//
// Wrapper method for ADOReadOnlyDB::close
//
static JSBool ADOReadOnlyDB_Close(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto   priv = PrivateData::GetFromThis<PrivateADOReadOnlyDB>(cx, vp);
   JSBool res  = JS_FALSE;

   if(!priv)
      return JS_FALSE;

   try
   {
      res = (priv->db.close() == S_OK ? JS_TRUE : JS_FALSE);
   }
   catch(_com_error &cerr)
   {
      ADOReadOnlyDB::PrintError(cerr);
   }
   catch(...)
   {
   }

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
   return JS_TRUE;
}

//
// ADOReadOnlyDB_GetRecordSet
//
// Wrapper method for ADOReadOnlyDB::getRecordSet
//
static JSBool ADOReadOnlyDB_GetRecordSet(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivateADOReadOnlyDB>(cx, vp);
   if(!priv)
      return JS_FALSE;
   
   try
   {
      PrivateADORecordSet::NewAsReturnVal(cx, vp, priv->db.getRecordSet());
      return JS_TRUE;
   }
   catch(_com_error &err)
   {
      ADOReadOnlyDB::PrintError(err);
   }
   catch(...)
   {
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

//
// ADOReadOnlyDB_GetCommand
//
// Wrapper method for ADOReadOnlyDB::getCommand
//
static JSBool ADOReadOnlyDB_GetCommand(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto priv = PrivateData::MustGetFromThis<PrivateADOReadOnlyDB>(cx, vp);

   if(argc >= 1)
   {
      const char *sql = SafeGetStringBytes(cx, argv[0], &argv[0]);
      JSBool prepared = JS_FALSE;
      if(argc >= 2)
         JS_ValueToBoolean(cx, argv[1], &prepared);

      try
      {
         PrivateADOCommand::NewAsReturnVal(cx, vp, priv->db.getCommand(sql, prepared == JS_TRUE));
         return JS_TRUE;
      }
      catch(_com_error &err)
      {
         ADOReadOnlyDB::PrintError(err);
      }
      catch(...)
      {
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

//
// ADOReadOnlyDB_GetCommandFile
//
// Wrapper method for ADOReadOnlyDB::getCommandFile
//
static JSBool ADOReadOnlyDB_GetCommandFile(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto priv = PrivateData::MustGetFromThis<PrivateADOReadOnlyDB>(cx, vp);

   if(argc >= 1)
   {
      const char *fn = SafeGetStringBytes(cx, argv[0], &argv[0]);
      JSBool prepared = JS_FALSE;
      if(argc >= 2)
         JS_ValueToBoolean(cx, argv[1], &prepared);

      try
      {
         PrivateADOCommand::NewAsReturnVal(cx, vp, priv->db.getCommandFile(fn, prepared == JS_TRUE));
         return JS_TRUE;
      }
      catch(_com_error &err)
      {
         ADOReadOnlyDB::PrintError(err);
      }
      catch(...)
      {
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

//
// ADOReadOnlyDB_GetCommandFile
//
// Wrapper method for ADOReadOnlyDB::execute
//
static JSBool ADOReadOnlyDB_Execute(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   try
   {
      auto priv = PrivateData::MustGetFromThis<PrivateADOReadOnlyDB>(cx, vp);
      if(argc >= 1)
      {
         const char *cmd = SafeGetStringBytes(cx, argv[0], &argv[0]);
         try
         {
            priv->db.execute(cmd);
         }
         catch(_com_error &err)
         {
            ADOReadOnlyDB::PrintError(err);
         }
         catch(...)
         {
         }
      }
   }
   catch(const JSEngineError &)
   {
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

/**
 * ADOReadOnlyDB JS Method Table
 */
static JSFunctionSpec adoReadOnlyDBFuncs[] =
{
   JSE_FN("open",           ADOReadOnlyDB_Open,           1, 0, 0),
   JSE_FN("close",          ADOReadOnlyDB_Close,          0, 0, 0),
   JSE_FN("getRecordSet",   ADOReadOnlyDB_GetRecordSet,   0, 0, 0),
   JSE_FN("getCommand",     ADOReadOnlyDB_GetCommand,     2, 0, 0),
   JSE_FN("getCommandFile", ADOReadOnlyDB_GetCommandFile, 2, 0, 0),
   JS_FN("execute",        ADOReadOnlyDB_Execute,        1, 0, 0),
   JS_FS_END
};

static NativeInitCode ADOReadOnlyDB_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &adoReadOnlyDBClass, ADOReadOnlyDB_New,
                           0, nullptr, adoReadOnlyDBFuncs, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native adoReadOnlyDBNative("ADOReadOnlyDB", ADOReadOnlyDB_Create);

#endif // VIBC_NO_ADODB

// EOF

