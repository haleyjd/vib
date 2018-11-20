/*
   JS Bindings for libcurl functionality
*/

#ifndef VIBC_NO_LIBCURL

#include "jsengine2.h"
#include "jsnatives.h"
#include "curl_file.h"
#include "main.h"

//=============================================================================
//
// Shared libcurl utils
//

//
// InitcURLForJS
//
// Call libcurl's global initialization routine the first time any libcurl
// functionality is going to be used.
//
static void InitcURLForJS()
{
   static bool alreadyDone = false;

   if(!alreadyDone)
   {
      if(!curl_global_init(CURL_GLOBAL_ALL))
         new ShutdownAction(curl_global_cleanup);
      else
         throw JSEngineError("Failed to initialize cURL");
      
      alreadyDone = true;
   }   
}

//=============================================================================
//
// CURLFile
//
// Ability to read from URLs
//

// File internal native class
class NativeCURLFile : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   std::unique_ptr<CURLFile> f; 

   NativeCURLFile() : PrivateData(), f(new CURLFile())
   {
   }

   NativeCURLFile(const char *url, const char *operation) 
      : PrivateData(), f(new CURLFile(url, operation))
   {
   }
};

// Constructor
static JSBool CURLFile_New(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
                           jsval *rval)
{
   ASSERT_IS_CONSTRUCTING(cx, "CURLFile");

   // make sure libcurl is initialized
   InitcURLForJS();

   const char *filename = nullptr;
   const char *mode     = "r";

   if(argc >= 1)
      filename = SafeGetStringBytes(cx, argv[0], &argv[0]);

   if(argc >= 2)
      mode = SafeGetStringBytes(cx, argv[1], &argv[1]);

   std::unique_ptr<NativeCURLFile> newFile;
   if(filename)
      newFile.reset(new NativeCURLFile(filename, mode));
   else
      newFile.reset(new NativeCURLFile());

   newFile->setToJSObjectAndRelease(cx, obj, newFile);
   return JS_TRUE;
}

// Finalizer
static void CURLFile_Finalize(JSContext *cx, JSObject *obj)
{
   auto file = PrivateData::GetFromJSObject<NativeCURLFile>(cx, obj);

   if(file)
   {
      delete file;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

// open
static JSBool CURLFile_open(JSContext *cx, uintN argc, jsval *vp)
{
   auto file = PrivateData::MustGetFromThis<NativeCURLFile>(cx, vp);
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "CURLFile::open");

   const char *filename;
   const char *mode = "r";

   if(argc >= 1)
      filename = SafeGetStringBytes(cx, argv[0], &argv[0]);

   if(argc >= 2)
      mode = SafeGetStringBytes(cx, argv[1], &argv[1]);

   bool res = file->f->open(filename, mode);

   JS_SET_RVAL(cx, vp, res ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

// openPost
static JSBool CURLFile_openPost(JSContext *cx, uintN argc, jsval *vp)
{
   auto file = PrivateData::MustGetFromThis<NativeCURLFile>(cx, vp);
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "CURLFile::openPost");

   const char *filename;
   const char *post = "";

   if(argc >= 1)
      filename = SafeGetStringBytes(cx, argv[0], &argv[0]);

   if(argc >= 2)
      post = SafeGetStringBytes(cx, argv[1], &argv[1]);

   bool res = file->f->openPost(filename, post);

   JS_SET_RVAL(cx, vp, res ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

// isOpen
static JSBool CURLFile_isOpen(JSContext *cx, uintN argc, jsval *vp)
{
   auto file = PrivateData::GetFromThis<NativeCURLFile>(cx, vp);
   if(!file)
      return JS_FALSE;

   bool isOpen = file->f->isOpen();
   JS_SET_RVAL(cx, vp, isOpen ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

// close
static JSBool CURLFile_close(JSContext *cx, uintN argc, jsval *vp)
{
   auto file = PrivateData::GetFromThis<NativeCURLFile>(cx, vp);
   if(!file)
      return JS_FALSE;

   bool res = file->f->close();
   JS_SET_RVAL(cx, vp, res ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

// eof
static JSBool CURLFile_eof(JSContext *cx, uintN argc, jsval *vp)
{
   auto file = PrivateData::GetFromThis<NativeCURLFile>(cx, vp);
   if(!file)
      return JS_FALSE;

   bool res = file->f->eof();
   JS_SET_RVAL(cx, vp, res ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

// read
static JSBool CURLFile_read(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto file = PrivateData::GetFromThis<NativeCURLFile>(cx, vp);
   if(!file)
      return JS_FALSE;

   unsigned char  readBuffer[256];
   unsigned char *output = nullptr;
   unsigned char *outpos = nullptr;
   size_t nread;
   size_t total = 0;
   do
   {
      nread = file->f->read(readBuffer, 1, sizeof(readBuffer));
      if(total + nread > total)
      {
         ptrdiff_t offs = outpos - output;
         total += nread;
         output = static_cast<unsigned char *>(realloc(output, total));
         outpos = output + offs;
         memcpy(outpos, readBuffer, nread);
         outpos += nread;
      }
   }
   while(nread);

   if(output)
   {
      auto nbb = new NativeByteBuffer(output, total);
      if(nbb)
      {
         AutoNamedRoot anr;
         auto nobj = NativeByteBuffer::ExternalCreate(cx, nbb, anr);
         JS_SET_RVAL(cx, vp, nobj ? OBJECT_TO_JSVAL(nobj) : JSVAL_NULL);
      }
      else
      {
         free(output);
         JS_SET_RVAL(cx, vp, JSVAL_NULL);
      }
   }
   else
      JS_SET_RVAL(cx, vp, JSVAL_NULL);

   return JS_TRUE;
}

// gets
static JSBool CURLFile_gets(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto file = PrivateData::MustGetFromThis<NativeCURLFile>(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "CURLFile::gets");

   size_t sizeToRead;
   uint32 i = 0;
   JS_ValueToECMAUint32(cx, argv[0], &i);
   sizeToRead = static_cast<size_t>(i);
   std::unique_ptr<char []> str(new char [sizeToRead]);

   char *res = file->f->gets(str.get(), sizeToRead);
   if(res)
   {
      JSString *jstr = AssertJSNewStringCopyZ(cx, res);
      JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   }
   else
      JS_SET_RVAL(cx, vp, JSVAL_NULL);

   return JS_TRUE;
}

// rewind
static JSBool CURLFile_rewind(JSContext *cx, uintN argc, jsval *vp)
{
   auto file = PrivateData::GetFromThis<NativeCURLFile>(cx, vp);
   if(!file)
      return JS_FALSE;

   file->f->rewind();
   return JS_TRUE;
}

static JSClass curlfile_class =
{
   "CURLFile",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   CURLFile_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(NativeCURLFile, curlfile_class)

static JSFunctionSpec curlfileJSMethods[] =
{
   JSE_FN("open",     CURLFile_open,     1, 0, 0),
   JSE_FN("openPost", CURLFile_openPost, 1, 0, 0),
   JSE_FN("isOpen",   CURLFile_isOpen,   0, 0, 0),
   JSE_FN("close",    CURLFile_close,    0, 0, 0),
   JSE_FN("eof",      CURLFile_eof,      0, 0, 0),
   JSE_FN("read",     CURLFile_read,     0, 0, 0),
   JSE_FN("gets",     CURLFile_gets,     1, 0, 0),
   JSE_FN("rewind",   CURLFile_rewind,   0, 0, 0),
   JS_FS_END
};

static NativeInitCode CURLFile_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &curlfile_class, 
                           JSEngineNativeWrapper<CURLFile_New>, 
                           0, nullptr, curlfileJSMethods, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native curlfileGlobalNative("CURLFile", CURLFile_Create);

//=============================================================================
//
// CURLConnection
//
// Keep-alive cURL connection object; run multiple actions on the same handle.
//

// Connection internal native class
class NativeCURLConnection : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   std::unique_ptr<CURLConnection> f; 

   NativeCURLConnection() : PrivateData(), f(new CURLConnection())
   {
   }
};

// Constructor
static JSBool CURLConnection_New(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
                                 jsval *rval)
{
   ASSERT_IS_CONSTRUCTING(cx, "CURLConnection");

   // make sure libcurl is initialized
   InitcURLForJS();

   std::unique_ptr<NativeCURLConnection> newConn(new NativeCURLConnection());
   newConn->setToJSObjectAndRelease(cx, obj, newConn);
   return JS_TRUE;
}

// Finalizer
static void CURLConnection_Finalize(JSContext *cx, JSObject *obj)
{
   auto conn = PrivateData::GetFromJSObject<NativeCURLConnection>(cx, obj);

   if(conn)
   {
      delete conn;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

/* TODO: METHODS */

static JSBool CURLConnection_readURL(JSContext *cx, uintN argc, jsval *vp)
{
   auto conn = PrivateData::MustGetFromThis<NativeCURLConnection>(cx, vp);
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "CURLConnection::readURL()");

   const char *url = SafeGetStringBytes(cx, argv[0], &argv[0]);

   char *res = conn->f->readURL(url);
   if(res)
   {
      auto jstr = JS_NewStringCopyZ(cx, res);
      if(jstr)
         JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
      else
         JS_SET_RVAL(cx, vp, JSVAL_NULL);
      free(res);
   }
   else
      JS_SET_RVAL(cx, vp, JSVAL_NULL);

   return JS_TRUE;
}

static JSBool CURLConnection_postURL(JSContext *cx, uintN argc, jsval *vp)
{
   auto conn = PrivateData::MustGetFromThis<NativeCURLConnection>(cx, vp);
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 2, "CURLConnection::postURL()");

   const char *url  = SafeGetStringBytes(cx, argv[0], &argv[0]);
   const char *post = SafeGetStringBytes(cx, argv[1], &argv[1]);

   char *res = conn->f->postURL(url, post);
   if(res)
   {
      auto jstr = JS_NewStringCopyZ(cx, res);
      if(jstr)
         JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
      else
         JS_SET_RVAL(cx, vp, JSVAL_NULL);
      free(res);
   }
   else
      JS_SET_RVAL(cx, vp, JSVAL_NULL);

   return JS_TRUE;
}

static JSBool CURLConnection_close(JSContext *cx, uintN argc, jsval *vp)
{
   auto conn = PrivateData::GetFromThis<NativeCURLConnection>(cx, vp);
   if(!conn)
      return JS_FALSE;

   conn->f->close();
   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSClass curlconn_class =
{
   "CURLConnection",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   CURLConnection_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(NativeCURLConnection, curlconn_class)

static JSFunctionSpec curlconnJSMethods[] =
{
   JSE_FN("readURL", CURLConnection_readURL, 1, 0, 0),
   JSE_FN("postURL", CURLConnection_postURL, 1, 0, 0),
   JSE_FN("close",   CURLConnection_close,   0, 0, 0),
   JS_FS_END
};

static NativeInitCode CURLConnection_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &curlconn_class, 
                           JSEngineNativeWrapper<CURLConnection_New>, 
                           0, nullptr, curlconnJSMethods, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native curlconnGlobalNative("CURLConnection", CURLConnection_Create);

#endif // VIBC_NO_LIBCURL

// EOF

