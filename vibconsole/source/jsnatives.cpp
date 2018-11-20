/*
   JS Native Classes and Functions
*/

#ifndef VIBC_NO_WIN32
#include <Windows.h>
#endif

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "inifile.h"
#include "jsengine2.h"
#include "timer.h"
#include "main.h"

#ifndef VIBC_NO_VISUALIB
#include "prometheusdb.h"
#endif

#include "util.h"
#include "jsnatives.h"
#include "sqlLib.h"

// Private for the PrivateData class
JSClass *PrivateData::ObjClass = nullptr;

//=============================================================================
//
// Core class
//
// The "Core" class interfaces with the interpreter itself, providing
// facilities such as garbage collection and script loading.
//

//
// Signal an unconditional garbage collection pass.
//
static JSBool Core_GC(JSContext *cx, uintN argc, jsval *vp)
{
   JS_GC(cx);

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

//
// Perform garbage collection only if it is needed.
//
static JSBool Core_MaybeGC(JSContext *cx, uintN argc, jsval *vp)
{
   JS_MaybeGC(cx);

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

//
// Load and execute a script
//
static JSBool Core_LoadScript(JSContext *cx, uintN argc, jsval *vp)
{
   JSBool    ok   = JS_FALSE;
   JSObject *obj  = JS_GetGlobalForObject(cx, JS_THIS_OBJECT(cx, vp));
   jsval    *argv = JS_ARGV(cx, vp);

   if(argc >= 1)
   {
      JSScript *script;
      const char *filename = SafeGetStringBytes(cx, argv[0], &argv[0]);

      uint32 oldopts = JS_GetOptions(cx);
      JS_SetOptions(cx, oldopts | JSOPTION_COMPILE_N_GO);

      try
      {
         if((script = JS_CompileFile(cx, obj, filename)))
         {
            JSObject *scriptRoot = AssertJSNewScriptObject(cx, script);
            AutoNamedRoot root(cx, scriptRoot, "Core_LoadScript");
            jsval result = JSVAL_VOID;

            if((ok = JS_ExecuteScript(cx, obj, script, &result)))
               JS_SET_RVAL(cx, vp, result);
            else
            {
               JS_ReportPendingException(cx);
               throw JSEngineError("Error executing script");
            }
         }
         else
         {
            JS_ReportPendingException(cx);
            throw JSEngineError("Error compiling script");
         }
      }
      catch(const JSEngineError &err)
      {
         ok = err.propagateToJS(cx);
      }

      JS_SetOptions(cx, oldopts);
   }

   return ok;
}

//
// Load a module
//
static JSBool Core_LoadModule(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   
   ASSERT_ARGC_GE(argc, 1, "loadModule");

   // argument is always the module filename
   const char *filename = SafeGetStringBytes(cx, argv[0], &argv[0]);

   jsval rval = JSVAL_VOID;
   std::unique_ptr<JSEvalContext> ecx(JSEngine_NewSandbox());

   if(!JSEngine_EvaluateFileInContext(ecx.get(), filename, &rval))
      throw JSEngineError("Module evaluation failed");

   JS_SET_RVAL(cx, vp, rval);
   return JS_TRUE;
}

//
// Load a file; the "mixin" value in the global scope is the second argument to
// the function, allowing properties to be defined on it by the code in the
// module file. 
//
// What is a mixin anyway?
// See http://en.wikipedia.org/wiki/Mixin for information.
//
static JSBool Core_LoadMixin(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 2, "loadMixin");
   ASSERT_IS_OBJECT(argv[1], "target object");

   // argument 1 is the filename; argument 2 is the target object
   const char *filename  = SafeGetStringBytes(cx, argv[0], &argv[0]);
   JSObject   *targetObj = JSVAL_TO_OBJECT(argv[1]);
   std::unique_ptr<JSEvalContext> ecx(JSEngine_NewSandbox());

   AssertJSDefineProperty(ecx->getContext(), ecx->getGlobal(), "mixin", argv[1], nullptr, nullptr, 0);

   JSObject *argsObj = AssertJSNewArrayObject(ecx->getContext(), argc, argv);
   AssertJSDefineProperty(ecx->getContext(), ecx->getGlobal(), "arguments", OBJECT_TO_JSVAL(argsObj), nullptr, nullptr, 0);

   jsval rval = JSVAL_VOID;
   if(!JSEngine_EvaluateFileInContext(ecx.get(), filename, &rval))
      throw JSEngineError("Mixin evaluation failed");

   JS_SET_RVAL(cx, vp, rval);
   return JS_TRUE;
}

//
// Load a JSON file
//
static JSBool Core_LoadJSON(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "loadJSON");

   // argument is always the JSON filename
   const char *filename = SafeGetStringBytes(cx, argv[0], &argv[0]);

   std::unique_ptr<JSEvalContext> ecx(JSEngine_NewRestrictedContext());
   jsval rval = JSVAL_VOID;
   if(!JSEngine_EvaluateAugmentedFile(ecx.get(), filename, &rval))
      throw JSEngineError("JSON evaluation failed");

   JS_SET_RVAL(cx, vp, rval);
   return JS_TRUE;
}

//
// Evaluate a string (mostly the same as the built-in eval function)
//
static JSBool Core_EvalString(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "evalString");

   AutoJSValueToStringRooted jstr(cx, argv[0]);
   jsval rval = JSVAL_VOID;
   if(!JSEngine_EvaluateUCJSString(jstr, &rval))
      throw JSEngineError("String evaluation failed");

   JS_SET_RVAL(cx, vp, rval);
   return JS_TRUE;
}

//
// Evaluate a string inside a restricted context.
//
static JSBool Core_EvalUntrustedString(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "evalUntrustedString");

   AutoJSValueToStringRooted jstr(cx, argv[0]);
   jsval rval = JSVAL_VOID;
   std::unique_ptr<JSEvalContext> ecx(JSEngine_NewRestrictedContext());

   if(!JSEngine_EvaluateUCJSStringInContext(ecx.get(), jstr, &rval))
      throw JSEngineError("String evaluation failed");

   JS_SET_RVAL(cx, vp, rval);
   return JS_TRUE;
}

//
// Evaluate a string inside a sandbox
//
static JSBool Core_EvalSandbox(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   JSObject *injectProps = nullptr;
   bool parented = true;
   bool withExtensions = false;

   ASSERT_ARGC_GE(argc, 1, "evalString");

   if(argc >= 2)
   {
      JSBool p = JS_TRUE, e = JS_FALSE;
      if(JSVAL_IS_BOOLEAN(argv[1]))
      {
         // parent sandbox to the global context?
         JS_ValueToBoolean(cx, argv[1], &p);
         parented = !!p;
      }

      if(argc >= 3)
      {
         if(JSVAL_IS_BOOLEAN(argv[2]))
         {
            // apply extensions? (NB: parent == true implies this, since will inherit from global)
            JS_ValueToBoolean(cx, argv[2], &e);
            withExtensions = !!e;
         }
      }

      if(argc >= 4 && JSVAL_IS_OBJECT(argv[3]))
         injectProps = JSVAL_TO_OBJECT(argv[3]);
   }

   AutoJSValueToStringRooted jstr(cx, argv[0]);
   jsval rval = JSVAL_VOID;
   std::unique_ptr<JSEvalContext> ecx(JSEngine_NewSandbox(injectProps, parented, withExtensions));

   if(!JSEngine_EvaluateUCJSStringInContext(ecx.get(), jstr, &rval))
      throw JSEngineError("String evaluation failed");

   JS_SET_RVAL(cx, vp, rval);
   return JS_TRUE;
}

//
// Exit the interpreter
//
static JSBool Core_Exit(JSContext *cx, uintN argc, jsval *vp)
{
    if(!MainLoopRunning)
       exit(ProcessReturnCode); // exit directly if not in main loop

    MainLoopRunning = false;

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

//
// Get current execution time in milliseconds
//
static JSBool Core_GetMS(JSContext *cx, uintN argc, jsval *vp)
{
   jsval r;
   if(JS_NewNumberValue(cx, Timer_getMS(), &r))
   {
      JS_SET_RVAL(cx, vp, r);
      return JS_TRUE;
   }
   else
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      JS_ReportOutOfMemory(cx);
      return JS_FALSE;
   }
}

//
// Change interpreter interactive state
//
static JSBool Core_SetInteractive(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   JSBool interactiveValue = true;
   if(argc >= 1)
      JS_ValueToBoolean(cx, argv[0], &interactiveValue);

   NonInteractive = !interactiveValue;

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSClass core_class =
{
   "CoreClass",
   0,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   JS_FinalizeStub,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSFunctionSpec coreJSMethods[] =
{
   JSE_FN("GC",                  Core_GC,                  0, 0, 0),
   JSE_FN("maybeGC",             Core_MaybeGC,             0, 0, 0),
   JSE_FN("loadScript",          Core_LoadScript,          1, 0, 0),
   JSE_FN("loadModule",          Core_LoadModule,          1, 0, 0),
   JSE_FN("loadMixin",           Core_LoadMixin,           2, 0, 0),
   JSE_FN("loadJSON",            Core_LoadJSON,            1, 0, 0),
   JSE_FN("evalString",          Core_EvalString,          1, 0, 0),
   JSE_FN("evalUntrustedString", Core_EvalUntrustedString, 1, 0, 0),
   JSE_FN("evalSandbox",         Core_EvalSandbox,         1, 0, 0),
   JSE_FN("exit",                Core_Exit,                0, 0, 0),
   JSE_FN("getMS",               Core_GetMS,               0, 0, 0),
   JSE_FN("setInteractive",      Core_SetInteractive,      0, 0, 0),
   JS_FS_END
};

static NativeInitCode Env_Create(JSContext *cx, JSObject *obj)
{
   try
   {
      JSObject *newArray = AssertJSNewArrayObject(cx, 0, nullptr);
      AutoNamedRoot anr(cx, newArray, "NewEnvArray");

      jsint count = 0;
      for(auto env = myenvp; *env; env++)
      {
         JSString *obj = AssertJSNewStringCopyZ(cx, *env);
         AutoNamedRoot snr(cx, obj, "NewStringRoot");

         jsval v = STRING_TO_JSVAL(obj);
         AssertJSSetElement(cx, newArray, count, &v);

         ++count;
      }

      AssertJSDefineProperty(cx, obj, "env", OBJECT_TO_JSVAL(newArray), nullptr, nullptr, 0);    
      return RESOLVED;
   }
   catch(const JSEngineError &)
   {
      return RESOLUTIONERROR;
   }
}

static NativeInitCode Core_Create(JSContext *cx, JSObject *global)
{
   JSObject *obj;

   if(!(obj = JS_DefineObject(cx, global, "Core", &core_class, nullptr, JSPROP_PERMANENT)))
      return RESOLUTIONERROR;

   if(!JS_DefineFunctions(cx, obj, coreJSMethods))
      return RESOLUTIONERROR;

   return Env_Create(cx, obj);
}

static Native coreGlobalNative("Core", Core_Create);

//=============================================================================
//
// Console class
//
// Allows direct interaction with the console.
//

std::ofstream echoFile;

// Print a message to the console
static JSBool Console_Print(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   for(uintN i = 0; i < argc; i++)
   {
      const char *str = SafeGetStringBytes(cx, argv[i], &argv[i]);
      std::cout << str;
      if(echoFile)
         echoFile << str;
   }

   if(echoFile)
      echoFile.flush();

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

// Print a message to the console, and end with a linebreak.
static JSBool Console_PrintLine(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   for(uintN i = 0; i < argc; i++)
   {
      const char *str = SafeGetStringBytes(cx, argv[i], &argv[i]);
      std::cout << str;
      if(echoFile)
         echoFile << str;
   }

   std::cout << std::endl;
   if(echoFile)
   {
      echoFile << std::endl;
      echoFile.flush();
   }

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

// Get a line of input form the console
static JSBool Console_GetLine(JSContext *cx, uintN argc, jsval *vp)
{
   std::string cppstr;

   std::getline(std::cin, cppstr);

   JSString *str = AssertJSNewStringCopyZ(cx, cppstr.c_str());
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(str));

   return JS_TRUE;
}

static JSBool Console_StartEcho(JSContext *cx, uintN argc, jsval *vp)
{
   if(echoFile.is_open())
      throw JSEngineError("Already logging");

   jsval *argv = JS_ARGV(cx, vp);
   ASSERT_ARGC_GE(argc, 1, "startEcho");
   const char *filename = SafeGetStringBytes(cx, argv[0], &argv[0]);

   echoFile.open(filename, std::ios::out);
   if(!echoFile)
      throw JSEngineError("Could not open echo log");

   JS_SET_RVAL(cx, vp, JSVAL_TRUE);
   return JS_TRUE;
}

static JSBool Console_StopEcho(JSContext *cx, uintN argc, jsval *vp)
{
   if(!echoFile.is_open())
      JS_SET_RVAL(cx, vp, JSVAL_FALSE);
   else
   {
      echoFile.close();
      JS_SET_RVAL(cx, vp, JSVAL_TRUE);
   }

   return JS_TRUE;
}

static JSBool Console_Resize(JSContext *cx, uintN argc, jsval *vp)
{
#ifndef VIBC_NO_WIN32
   jsval *argv = JS_ARGV(cx, vp);
   SMALL_RECT r;
   COORD      c;
   COORD      largest;
   HANDLE     hConOut;
   CONSOLE_SCREEN_BUFFER_INFO csbi;

   memset(&csbi, 0, sizeof(csbi));

   hConOut = GetStdHandle(STD_OUTPUT_HANDLE);

   largest = GetLargestConsoleWindowSize(hConOut);

   if(!GetConsoleScreenBufferInfo(hConOut, &csbi))
      throw JSEngineError("Cannot retrieve console info");

   ASSERT_ARGC_GE(argc, 2, "Console.resize");

   int32 width, height;
   JS_ValueToECMAInt32(cx, argv[0], &width);
   JS_ValueToECMAInt32(cx, argv[1], &height);

   if(width > largest.X)
      width = largest.X;

   if(height > largest.Y)
      height = largest.Y;

   c.X = (SHORT)width;
   c.Y = (SHORT)height;

   if(!SetConsoleScreenBufferSize(hConOut, c))
      throw JSEngineError("Cannot set console buffer size");

   r.Left   = csbi.srWindow.Left;
   r.Top    = csbi.srWindow.Top;
   r.Right  = (SHORT)(width - 1);
   r.Bottom = (SHORT)(height - 1);

   if(!SetConsoleWindowInfo(hConOut, TRUE, &r))
      throw JSEngineError("Cannot resize console window");
#endif

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Console_SetTextAttrib(JSContext *cx, uintN argc, jsval *vp)
{
#ifndef VIBC_NO_WIN32
   jsval *argv = JS_ARGV(cx, vp);
   WORD color = 0x07;

   if(argc >= 1)
   {
      int32 bleh;
      JS_ValueToECMAInt32(cx, argv[0], &bleh);
      color = (WORD)bleh;
   }
   
   HANDLE hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
   SetConsoleTextAttribute(hConOut, color);
#endif

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Console_SetTitle(JSContext *cx, uintN argc, jsval *vp)
{
#ifndef VIBC_NO_WIN32
   const char *title = "vibconsole";
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 1)
      title = SafeGetStringBytes(cx, argv[0], &argv[0]);

   SetConsoleTitleA(title);
#endif

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Console_FillOutputAttrib(JSContext *cx, uintN argc, jsval *vp)
{
#ifndef VIBC_NO_WIN32
   jsval *argv = JS_ARGV(cx, vp);
   HANDLE hStdOut;
   COORD  maxSize;
   DWORD  num;
   COORD  start;
   WORD color = 0x07;

   if(argc >= 1)
   {
      int32 bleh;
      JS_ValueToECMAInt32(cx, argv[0], &bleh);
      color = (WORD)bleh;
   }

   hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
   maxSize = GetLargestConsoleWindowSize(hStdOut);

   start.X = start.Y = 0;

   FillConsoleOutputAttribute(hStdOut, color, (maxSize.X*maxSize.Y), start, &num);
#endif

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSClass console_class =
{
   "ConsoleClass",
   0,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   JS_FinalizeStub,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSFunctionSpec consoleJSMethods[] =
{
   JSE_FN("print",            Console_Print,            0, 0, 0),
   JSE_FN("println",          Console_PrintLine,        0, 0, 0),
   JSE_FN("getline",          Console_GetLine,          0, 0, 0),
   JSE_FN("startEcho",        Console_StartEcho,        1, 0, 0),
   JSE_FN("stopEcho",         Console_StopEcho,         0, 0, 0),
   JSE_FN("resize",           Console_Resize,           2, 0, 0),
   JSE_FN("setTextAttrib",    Console_SetTextAttrib,    0, 0, 0),
   JSE_FN("fillOutputAttrib", Console_FillOutputAttrib, 0, 0, 0),
   JSE_FN("setTitle",         Console_SetTitle,         0, 0, 0),
   JS_FS_END
};
   
static NativeInitCode Console_Create(JSContext *cx, JSObject *global)
{
   JSObject *obj;

   if(!(obj = JS_DefineObject(cx, global, "Console", &console_class, nullptr, JSPROP_PERMANENT)))
      return RESOLUTIONERROR;

   if(!JS_DefineFunctions(cx, obj, consoleJSMethods))
      return RESOLUTIONERROR;

   return RESOLVED;
}

static Native consoleGlobalNative("Console", Console_Create);

//=============================================================================
//
// Environment object
//


//=============================================================================
//
// Byte buffer class
//

NativeByteBuffer::NativeByteBuffer(size_t pSize) : PrivateData()
{
   size = pSize;
   memory = static_cast<unsigned char *>(calloc(1, size));
}

NativeByteBuffer::NativeByteBuffer(unsigned char *pMemory, size_t pSize)
      : size(pSize), memory(pMemory)
{
}

NativeByteBuffer::NativeByteBuffer(const NativeByteBuffer &other) : PrivateData()
{
   size = other.size;
   memory = static_cast<unsigned char *>(malloc(size));
   memcpy(memory, other.memory, size);
}

NativeByteBuffer::~NativeByteBuffer()
{
   if(memory)
   {
      free(memory);
      memory = nullptr;
   }
}

void NativeByteBuffer::resize(size_t newSize)
{
   size = newSize;
   memory = static_cast<unsigned char *>(realloc(memory, size));
}

void NativeByteBuffer::toString(std::string &out) const
{
   out.assign(reinterpret_cast<const char *>(memory), size);
}

void NativeByteBuffer::fromString(const std::string &in) 
{
   size_t destsize = in.length();
   resize(destsize);
   memcpy(memory, in.c_str(), destsize);
}

JSString *NativeByteBuffer::toUCString(JSContext *cx)
{
   size_t  dstlen = 0;
   jschar *outbuf;

   if(JS_DecodeBytes(cx, reinterpret_cast<const char *>(memory), size, nullptr, &dstlen) && dstlen > 0)
   {
      outbuf = new jschar [dstlen];
      if(outbuf)
      {
         JS_DecodeBytes(cx, reinterpret_cast<const char *>(memory), size, outbuf, &dstlen);
         JSString *jstr = JS_NewUCStringCopyN(cx, outbuf, dstlen);
         delete [] outbuf;
         return jstr;
      }
   }
   return nullptr;
}

static void ByteBuffer_Finalize(JSContext *cx, JSObject *obj);

static JSClass bytebuffer_class =
{
   "ByteBuffer",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   ByteBuffer_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(NativeByteBuffer, bytebuffer_class)

// Constructor
static JSBool ByteBuffer_New(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
                             jsval *rval)
{
   ASSERT_IS_CONSTRUCTING(cx, "ByteBuffer");

   if(argc >= 1)
   {
      if(SafeInstanceOf(cx, &bytebuffer_class, argv[0]))
      {
         // copy construct
         auto copyBuffer = PrivateData::MustGetFromJSObject<NativeByteBuffer>(cx, JSVAL_TO_OBJECT(argv[0]));
         std::unique_ptr<NativeByteBuffer> newBuffer(new NativeByteBuffer(*copyBuffer));
         newBuffer->setToJSObjectAndRelease(cx, obj, newBuffer);
         return JS_TRUE;
      }
      else
      {
         uint32 i;
         JSBool success = JS_ValueToECMAUint32(cx, argv[0], &i);
         if(success)
         {
            std::unique_ptr<NativeByteBuffer> newBuffer(new NativeByteBuffer((size_t)i));
            newBuffer->setToJSObjectAndRelease(cx, obj, newBuffer);
            return JS_TRUE;
         }
      }
   }
   throw JSEngineError("ByteBuffer: need an initial size or existing buffer to copy");
}

// Finalizer
static void ByteBuffer_Finalize(JSContext *cx, JSObject *obj)
{
   auto buffer = PrivateData::GetFromJSObject<NativeByteBuffer>(cx, obj);

   if(buffer)
      delete buffer;

   JS_SetPrivate(cx, obj, nullptr);
}

// Resize
static JSBool ByteBuffer_Resize(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto buffer = PrivateData::MustGetFromThis<NativeByteBuffer>(cx, vp);
   ASSERT_ARGC_GE(argc, 1, "ByteBuffer::resize");

   uint32 size;
   JS_ValueToECMAUint32(cx, argv[0], &size);

   if(size > 0)
      buffer->resize(size);

   JS_SET_RVAL(cx, vp, JSVAL_TRUE);
   return JS_TRUE;
}

// to String
static JSBool ByteBuffer_ToString(JSContext *cx, uintN argc, jsval *vp)
{
   auto buffer = PrivateData::MustGetFromThis<NativeByteBuffer>(cx, vp);
   std::string out;

   buffer->toString(out);

   JSString *jstr = AssertJSNewStringCopyN(cx, out.c_str(), out.length());
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

// to UC String
static JSBool ByteBuffer_ToUCString(JSContext *cx, uintN argc, jsval *vp)
{
   auto buffer = PrivateData::GetFromThis<NativeByteBuffer>(cx, vp);
   if(!buffer)
      return JS_FALSE;
   
   JSString *jstr = buffer->toUCString(cx);
   JS_SET_RVAL(cx, vp, jstr ? STRING_TO_JSVAL(jstr) : JSVAL_NULL);
   return JS_TRUE;
}

// from String
static JSBool ByteBuffer_FromString(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto buffer = PrivateData::MustGetFromThis<NativeByteBuffer>(cx, vp);
   ASSERT_ARGC_GE(argc, 1, "ByteBuffer::fromString");

   buffer->fromString(SafeGetStringBytes(cx, argv[0], &argv[0]));
   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool ByteBuffer_SetBytesAt(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto buffer = PrivateData::MustGetFromThis<NativeByteBuffer>(cx, vp);
   ASSERT_ARGC_GE(argc, 2, "ByteBuffer::setBytesAt");

   uint32 idx = 0;
   JS_ValueToECMAUint32(cx, argv[0], &idx);

   if(idx >= buffer->getSize())
      throw JSEngineError("Index out of bounds");

   unsigned char *bufferptr = buffer->getBuffer() + idx;
   unsigned int   len       = buffer->getSize();
   JSObject *arrayObj;

   if(JSVAL_IS_OBJECT(argv[1]) && JS_IsArrayObject(cx, (arrayObj = JSVAL_TO_OBJECT(argv[1]))))
   {
      jsuint arrayLen = 0;

      JS_GetArrayLength(cx, arrayObj, &arrayLen);

      for(jsuint i = 0; i < arrayLen && idx + i < len; i++)
      {
         jsval valAtIndex = JSVAL_VOID;
         if(!JS_LookupElement(cx, arrayObj, (jsint)i, &valAtIndex))
            break;

         uint32 intVal = 0;
         JS_ValueToECMAUint32(cx, valAtIndex, &intVal);
         *(bufferptr + i) = static_cast<unsigned char>(intVal);
      }
   }
   else
   {
      uint32 intVal = 0;
      JS_ValueToECMAUint32(cx, argv[1], &intVal);
      *bufferptr = static_cast<unsigned char>(intVal);
   }

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSFunctionSpec byteBufferJSMethods[] =
{
   JSE_FN("resize",     ByteBuffer_Resize,     1, 0, 0),
   JSE_FN("toString",   ByteBuffer_ToString,   0, 0, 0),
   JSE_FN("toUCString", ByteBuffer_ToUCString, 0, 0, 0),
   JSE_FN("fromString", ByteBuffer_FromString, 1, 0, 0),
   JSE_FN("setBytesAt", ByteBuffer_SetBytesAt, 2, 0, 0),
   JS_FS_END
};

static JSBool ByteBuffer_GetSize(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto buffer = PrivateData::GetFromJSObject<NativeByteBuffer>(cx, obj);
   if(!buffer)
      return JS_FALSE;

   jsint jsSize = static_cast<jsint>(buffer->getSize());

   *vp = INT_TO_JSVAL(jsSize);
   return JS_TRUE;
}

static JSPropertySpec byteBufferProps[] =
{
   { 
      "size", 0, 
      JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED,
      ByteBuffer_GetSize, nullptr
   },

   { nullptr }
};

static NativeInitCode ByteBuffer_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &bytebuffer_class, 
                           JSEngineNativeWrapper<ByteBuffer_New>, 
                           0, byteBufferProps, byteBufferJSMethods, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native byteBufferGlobalNative("ByteBuffer", ByteBuffer_Create);

JSObject *NativeByteBuffer::ExternalCreate(JSContext *cx, NativeByteBuffer *nbb, AutoNamedRoot &anr)
{
   JSObject *newObj = nullptr;
   
   try
   { 
      newObj = AssertJSNewObject(cx, &bytebuffer_class, nullptr, nullptr);
      anr.init(cx, newObj, "ExternalByteBuffer");
      AssertJSDefineFunctions(cx, newObj, byteBufferJSMethods);
      AssertJSDefineProperties(cx, newObj, byteBufferProps);
      nbb->setToJSObject(cx, newObj);
   }
   catch(const JSEngineError &)
   {
      newObj = nullptr;
   }

   return newObj;
}

//=============================================================================
//
// File class
//
// Generic access to disk file operations
//

// File internal native class
class NativeFile : public PrivateData
{
   DECLARE_PRIVATE_DATA()

protected:
   FILE *f; 

public:
   NativeFile() : PrivateData(), f(nullptr)
   {
   }

   NativeFile(const char *filename, const char *mode) : PrivateData()
   {
      f = fopen(filename, mode);
   }

   ~NativeFile()
   {
      if(f)
      {
         fflush(f);
         fclose(f);
         f = nullptr;
      }
   }

   long size()
   {
      long pos = 0;

      if(f)
      {
         pos = ftell(f);
         fseek(f, 0, SEEK_END);
         pos = ftell(f);
         fseek(f, pos, SEEK_SET);
      }

      return pos;
   }

   int puts(const char *str)
   {
      if(f)
         return fputs(str, f);
      else
         return EOF;
   }

   int getc()
   {
      if(f)
         return fgetc(f);
      else
         return EOF;
   }

   int putc(int c)
   {
      if(f)
         return fputc(c, f);
      else
         return EOF;
   }

   size_t read(NativeByteBuffer *buffer, size_t size, size_t count)
   {
      if(f)
      {
         size_t bufferSize = buffer->getSize();
         if(bufferSize != size * count)
            buffer->resize(size * count);
         return fread(buffer->getBuffer(), size, count, f);
      }
      else
         return 0;
   }

   size_t write(NativeByteBuffer *buffer, size_t totalSize)
   {
      if(f)
      {
         size_t bufferSize  = buffer->getSize();
         size_t sizeToWrite = bufferSize < totalSize ? bufferSize : totalSize;

         return fwrite(buffer->getBuffer(), 1, totalSize, f);
      }
      else
         return 0;
   }

   int flush()
   {
      if(f)
         return fflush(f);
      else 
         return EOF;
   }

   void close()
   {
      if(f)
      {
         fclose(f);
         f = nullptr;
      }
   }

   bool isOpen() const { return (f != nullptr); }
};

// Constructor
static JSBool File_New(JSContext *cx, JSObject *obj, uintN argc, jsval *argv,
                       jsval *rval)
{
   ASSERT_IS_CONSTRUCTING(cx, "File");
   ASSERT_ARGC_GE(argc, 1, "File");

   const char *filename;
   const char *mode = "r";

   if(argc >= 1)
      filename = SafeGetStringBytes(cx, argv[0], &argv[0]);

   if(argc >= 2)
      mode = SafeGetStringBytes(cx, argv[1], &argv[1]);

   std::unique_ptr<NativeFile> newFile(new NativeFile(filename, mode));
   newFile->setToJSObjectAndRelease(cx, obj, newFile);
   return JS_TRUE;
}

// Finalizer
static void File_Finalize(JSContext *cx, JSObject *obj)
{
   auto file = PrivateData::GetFromJSObject<NativeFile>(cx, obj);

   if(file)
   {
      delete file;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

// size
static JSBool File_Size(JSContext *cx, uintN argc, jsval *vp)
{
   auto file = PrivateData::GetFromThis<NativeFile>(cx, vp);
   if(!file)
      return JS_FALSE;

   jsint jsSize = static_cast<jsint>(file->size());
   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(jsSize));
   return JS_TRUE;
}

// puts
static JSBool File_Puts(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto file = PrivateData::MustGetFromThis<NativeFile>(cx, vp);
   ASSERT_ARGC_GE(argc, 1, "File::puts");

   const char *strToPut = SafeGetStringBytes(cx, argv[0], &argv[0]);

   jsint ret = static_cast<jsint>(file->puts(strToPut));
   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ret));
   return JS_TRUE;
}

// read
static JSBool File_Read(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto file = PrivateData::MustGetFromThis<NativeFile>(cx, vp);
   ASSERT_ARGC_GE(argc, 1, "File::read");
   AssertInstanceOf(cx, &bytebuffer_class, argv[0]);

   JSObject *nbbObj = JSVAL_TO_OBJECT(argv[0]);
   auto nbb = PrivateData::MustGetFromJSObject<NativeByteBuffer>(cx, nbbObj);

   size_t sizeToRead = nbb->getSize();
   if(argc >= 2)
   {
      uint32 i = 0;
      JS_ValueToECMAUint32(cx, argv[1], &i);
      sizeToRead = static_cast<size_t>(i);
   }

   size_t result = file->read(nbb, 1, sizeToRead);
   jsint  jsResult = static_cast<jsint>(result);

   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(jsResult));
   return JS_TRUE;
}

// write
static JSBool File_Write(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto file = PrivateData::MustGetFromThis<NativeFile>(cx, vp);
   ASSERT_ARGC_GE(argc, 1, "File::write");
   AssertInstanceOf(cx, &bytebuffer_class, argv[0]);

   JSObject *nbbObj = JSVAL_TO_OBJECT(argv[0]);
   auto nbb = PrivateData::MustGetFromJSObject<NativeByteBuffer>(cx, nbbObj);

   size_t sizeToWrite = nbb->getSize();
   if(argc >= 2)
   {
      uint32 i = 0;
      JS_ValueToECMAUint32(cx, argv[1], &i);
      sizeToWrite = static_cast<size_t>(i);
   }

   size_t result = file->write(nbb, sizeToWrite);
   jsint  jsResult = static_cast<jsint>(result);

   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(jsResult));
   return JS_TRUE;
}

// isOpen
static JSBool File_IsOpen(JSContext *cx, uintN argc, jsval *vp)
{
   auto file = PrivateData::GetFromThis<NativeFile>(cx, vp);
   if(!file)
      return JS_FALSE;

   bool isOpen = file->isOpen();
   JS_SET_RVAL(cx, vp, isOpen ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

// flush
static JSBool File_Flush(JSContext *cx, uintN argc, jsval *vp)
{
   auto file = PrivateData::GetFromThis<NativeFile>(cx, vp);
   if(!file)
      return JS_FALSE;

   jsint i = file->flush();
   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(i));
   return JS_TRUE;
}

// close
static JSBool File_Close(JSContext *cx, uintN argc, jsval *vp)
{
   auto file = PrivateData::GetFromThis<NativeFile>(cx, vp);
   if(!file)
      return JS_FALSE;

   file->close();
   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSClass file_class =
{
   "File",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   File_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(NativeFile, file_class)

static JSFunctionSpec fileJSMethods[] =
{
   JSE_FN("size",   File_Size,   0, 0, 0),
   JSE_FN("puts",   File_Puts,   1, 0, 0),
   JSE_FN("read",   File_Read,   1, 0, 0),
   JSE_FN("write",  File_Write,  1, 0, 0),
   JSE_FN("isOpen", File_IsOpen, 0, 0, 0),
   JSE_FN("flush",  File_Flush,  0, 0, 0),
   JSE_FN("close",  File_Close,  0, 0, 0),
   JS_FS_END
};

static NativeInitCode File_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &file_class, 
                           JSEngineNativeWrapper<File_New>, 
                           0, nullptr, fileJSMethods, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native fileGlobalNative("File", File_Create);

//=============================================================================
//
// LazyStringMap
//

class PrivateStringMap : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   bool mapOwned;
   std::map<std::string, std::string> *strmap;

   PrivateStringMap() : PrivateData()
   {
      mapOwned = true;
      strmap = new std::map<std::string, std::string>();
   }

   PrivateStringMap(std::map<std::string, std::string> *map) : PrivateData()
   {
      mapOwned = false;
      strmap = map;
   }

   ~PrivateStringMap()
   {
      if(strmap && mapOwned)
         delete strmap;
   }
};

//
// LazyStringMap_SetProperty
//
// Allows writing new strings into the map.
//
static JSBool LazyStringMap_SetProperty(JSContext *cx, JSObject *obj, jsval idval,
                                        jsval *vp)
{
   auto priv = PrivateData::MustGetFromJSObject<PrivateStringMap>(cx, obj);
   AutoJSValueToStringRooted jstr(cx, idval);  
   AutoJSValueToStringRooted newStr(cx, *vp);      

   const char *name = JS_GetStringBytes(jstr);

   // Reflect the value into the native map
   (*priv->strmap)[name] = JS_GetStringBytes(newStr);
   return JS_TRUE;
}

//
// LazyStringMap_Resolve
//
// When a string is looked for in the map, check the internal STL object and
// then reflect that property into JS if it exists.
//
static JSBool LazyStringMap_Resolve(JSContext *cx, JSObject *obj, jsval id,
                                    uintN flags, JSObject **objp)
{
   auto priv = PrivateData::GetFromJSObject<PrivateStringMap>(cx, obj);

   // get property name
   if(priv && JSVAL_IS_STRING(id))
   {
      JSString *jstr = JSVAL_TO_STRING(id);
      const char *name = JS_GetStringBytes(jstr);

      if(name && (priv->strmap->find(name) != priv->strmap->end())) // is this key in the map?
      {
         // Create a new JSString with the same value
         JSString *newProp = JS_NewStringCopyZ(cx, (*priv->strmap)[name].c_str());

         // Now add it to the JS object as a property with the key as its name
         if(newProp &&
            JS_DefineProperty(cx, obj, name, STRING_TO_JSVAL(newProp), nullptr,
                              LazyStringMap_SetProperty, JSPROP_ENUMERATE))
         {
            *objp = obj;
            return JS_TRUE;
         }
      }
   }

   *objp = nullptr;
   return JS_TRUE;
}

//
// LazyStringMap_Enumerate
//
// The object is about to be enumerated, so we need to reflect all existing
// strings in the internal STL object.
//
static JSBool LazyStringMap_Enumerate(JSContext *cx, JSObject *obj)
{
   auto priv = PrivateData::GetFromJSObject<PrivateStringMap>(cx, obj);

   if(priv)
   {
      auto itr = priv->strmap->begin();
      JSBool found = JS_FALSE;

      for(; itr != priv->strmap->end(); ++itr)
      {
         // Add all key/value pairs not already in the object
         if(JS_HasProperty(cx, obj, itr->first.c_str(), &found) && found == JS_FALSE)
         {
            JSString *newProp = JS_NewStringCopyZ(cx, itr->second.c_str());
            JS_DefineProperty(cx, obj, itr->first.c_str(), STRING_TO_JSVAL(newProp),
                              nullptr, LazyStringMap_SetProperty, JSPROP_ENUMERATE);
         }
      }

      return JS_TRUE;
   }

   return JS_FALSE;
}

/** 
 * LazyStringMap JS Class.
 *
 * LazyStringMap reflects a map<string, string> into JS as an object with
 * lazy properties (ie., they are reflected into the object when asked for and
 * no earlier).
 */
static JSClass lazyStringMapClass =
{
   "LazyStringMap",
   JSCLASS_HAS_PRIVATE | JSCLASS_NEW_RESOLVE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JSEnginePropertyOpWrapper<LazyStringMap_SetProperty>,
   LazyStringMap_Enumerate,
   (JSResolveOp)LazyStringMap_Resolve,
   JS_ConvertStub,
   JS_FinalizeStub,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivateStringMap, lazyStringMapClass)

static JSBool LazyStringMap_ToCommaString(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::MustGetFromThis<PrivateStringMap>(cx, vp);

   std::string str = MapToDelimString(*priv->strmap, "", "", ",", "", "", "", "", "=");
   JSString *jstr  = AssertJSNewStringCopyZ(cx, str.c_str()); // FIXME/TODO: auto string
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

static JSFunctionSpec lazyStringMapMethods[] =
{
   JSE_FN("toCommaString", LazyStringMap_ToCommaString, 0, 0, 0),
   JS_FS_END
};

static NativeInitCode LazyStringMap_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &lazyStringMapClass, nullptr, 
                           0, nullptr, nullptr, nullptr, nullptr);

   if(!obj)
      return RESOLUTIONERROR;

   if(!JS_DefineFunctions(cx, obj, lazyStringMapMethods))
      return RESOLUTIONERROR;

   return RESOLVED;
}

static Native lazyStringMapGlobalNative("LazyStringMap", LazyStringMap_Create);

// For External use:

void LazyStringMap_ReturnObject(JSContext *cx, jsval *vp, std::map<std::string, std::string> &sm)
{
   try
   {
      JSObject *newObj = AssertJSNewObject(cx, &lazyStringMapClass, nullptr, nullptr);
      AutoNamedRoot anr(cx, newObj, "NewLazyStringMap");
      AssertJSDefineFunctions(cx, newObj, lazyStringMapMethods);
      std::unique_ptr<PrivateStringMap> psm(new PrivateStringMap());
      *psm->strmap = sm;
      psm->setToJSObjectAndRelease(cx, newObj, psm);
      JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newObj));
   }
   catch(const JSEngineError &)
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
   }
}

//=============================================================================
//
// LazyVecMap
//

class PrivateVecMap : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   std::vector<std::map<std::string, std::string>> vecmap;

   PrivateVecMap() : PrivateData(), vecmap()
   {
   }
};

//
// LazyVecMap_Finalize
//
// Destroy the C++ vecmap when being collected by the GC.
//
static void LazyVecMap_Finalize(JSContext *cx, JSObject *obj)
{
   auto vm = PrivateData::GetFromJSObject<PrivateVecMap>(cx, obj);
   if(vm)
   {
      delete vm;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

//
// LazyVecMap_Resolve
//
// The constituent stringmap rows of the vector are not reflected into JS until
// an access attempt is made at that particular index.
//
static JSBool LazyVecMap_Resolve(JSContext *cx, JSObject *obj, jsval id,
                                 uintN flags, JSObject **objp)
{
   try
   {
      auto priv = PrivateData::MustGetFromJSObject<PrivateVecMap>(cx, obj);

      // get property name
      if(JSVAL_IS_INT(id))
      {
         jsint index = JSVAL_TO_INT(id);

         if(index >= 0 && index < static_cast<jsint>(priv->vecmap.size()))
         {
            JSObject *newObj =
               JS_DefineObject(cx, obj, reinterpret_cast<const char *>(index),
                               &lazyStringMapClass, nullptr,
                               JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_INDEX);

            // Private data for a LazyStringMap points to the stringmap
            std::unique_ptr<PrivateStringMap> newStrMap(new PrivateStringMap(&priv->vecmap[index]));

            newStrMap->setToJSObjectAndRelease(cx, newObj, newStrMap);
            *objp = obj;
            return JS_TRUE;
         }
      }
   }
   catch(const JSEngineError &)
   {
   }

   *objp = nullptr;
   return JS_TRUE;
}

//
// LazyVecMap_Enumerate
//
// Before enumeration, ensure all vector rows have been reflected.
//
static JSBool LazyVecMap_Enumerate(JSContext *cx, JSObject *obj)
{
   try
   {
      auto priv = PrivateData::MustGetFromJSObject<PrivateVecMap>(cx, obj);

      for(size_t i = 0; i < priv->vecmap.size(); ++i)
      {
         // JS_DefineObject will return null if the property already exists. 
         JSObject *newObj =
            JS_DefineObject(cx, obj, reinterpret_cast<const char *>(i),
                            &lazyStringMapClass, nullptr,
                            JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_INDEX);

         // Private data for a LazyStringMap points to the stringmap
         if(newObj)
         {
            std::unique_ptr<PrivateStringMap> newStrMap(new PrivateStringMap(&priv->vecmap[i]));
            newStrMap->setToJSObjectAndRelease(cx, newObj, newStrMap);
         }
      }
      return JS_TRUE;
   }
   catch(const JSEngineError &)
   {
   }

   return JS_FALSE;
}

//
// LazyVecMap_Size
//
// Return the size of the vecmap
//
static JSBool LazyVecMap_Size(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivateVecMap>(cx, vp);
   if(!priv)
      return JS_FALSE;

   size_t len = priv->vecmap.size();

   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(static_cast<jsint>(len)));
   return JS_TRUE;
}

static JSBool LazyVecMap_toCSV(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::MustGetFromThis<PrivateVecMap>(cx, vp);
   jsval *argv = JS_ARGV(cx, vp);
   const auto &vm = priv->vecmap;
   std::string str;

   for(auto vmitr = vm.cbegin(); vmitr != vm.cend(); ++vmitr)
   {
      const auto &row = *vmitr;

      for(auto rowitr = row.cbegin(); rowitr != row.cend(); ++rowitr)
      {
         str += "\"" + ReplaceStringWithString(ReplaceStringWithString(ReplaceStringWithString(rowitr->second, "\"", "\"\""), "\n", " "), "\r", "") + "\"";
      }
      if(str[str.length() - 1] == ',')
         str.pop_back();
      str += '\n';
   }

   JSString *jstr = AssertJSNewStringCopyZ(cx, str.c_str()); // TODO: auto string
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

/**
 * LazyVecMap JS Class
 *
 * A lazy vector of LazyStringMap objects.
 */
static JSClass lazyVecMapClass =
{
   "LazyVecMap",
   JSCLASS_HAS_PRIVATE | JSCLASS_NEW_RESOLVE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   LazyVecMap_Enumerate,
   (JSResolveOp)LazyVecMap_Resolve,
   JS_ConvertStub,
   LazyVecMap_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivateVecMap, lazyVecMapClass)

static JSFunctionSpec lazyVecMapMethods[] =
{
   JSE_FN("size",  LazyVecMap_Size,  0, 0, 0),
   JSE_FN("toCSV", LazyVecMap_toCSV, 0, 0, 0),
   JS_FS_END
};

//
// CSVtoVecMapWrapper
//
// Allow loading CSV data files from JavaScript.
//
static JSBool CSVtoVecMapWrapper(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 1)
   {
      const char *filename = SafeGetStringBytes(cx, argv[0], &argv[0]);
      try
      {
         std::string errmsg;
         utilvecmap  output;

         bool res = CSVtoVecMap(filename, '\"', ',', output, errmsg, true);
         if(!res)
         {
            std::string msg = "Error in CSVtoVecMap: " + errmsg;
            throw JSEngineError(msg);
         }

         JSObject *obj = AssertJSNewObject(cx, &lazyVecMapClass, nullptr, nullptr);
         JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));

         std::unique_ptr<PrivateVecMap> pvm(new PrivateVecMap());
         pvm->vecmap = output;
         pvm->setToJSObjectAndRelease(cx, obj, pvm);

         return JS_TRUE;
      }
      catch(...)
      {
         JS_ReportError(cx, "Exception thrown by CSVtoVecMap");
         return JS_FALSE;
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_TRUE);
   return JS_TRUE;
}

static JSFunctionSpec lazyVecMapStatics[] =
{
   JSE_FN("FromCSV", CSVtoVecMapWrapper, 1, 0, 0),
   JS_FS_END
};

static NativeInitCode LazyVecMap_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &lazyVecMapClass, nullptr,
                           0, nullptr, lazyVecMapMethods, nullptr, lazyVecMapStatics);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native lazyVecMapGlobalNative("LazyVecMap", LazyVecMap_Create);

// For external use:

void LazyVecMap_ReturnObject(JSContext *cx, jsval *vp, std::vector<std::map<std::string, std::string>> &vm)
{
   try
   {
      JSObject *newObj = AssertJSNewObject(cx, &lazyVecMapClass, nullptr, nullptr);
      AutoNamedRoot anr(cx, newObj, "NewLazyVecMap");
      AssertJSDefineFunctions(cx, newObj, lazyVecMapMethods);
      std::unique_ptr<PrivateVecMap> pvm(new PrivateVecMap());
      pvm->vecmap = vm;
      pvm->setToJSObjectAndRelease(cx, newObj, pvm);
      JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newObj));
   }
   catch(const JSEngineError &)
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
   }
}

//=============================================================================
//
// LazyMapStringMap
//

class PrivateMapMap : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   utilmapmap *mapmap;

   PrivateMapMap(utilmapmap *mm) : PrivateData(), mapmap(mm)
   {
   }
};


//
// LazyMapStringMap_Resolve
//
// The stringmap rows of the outer map are not reflected into JS until an
// explicit request is made for one of them by name.
//
static JSBool LazyMapStringMap_Resolve(JSContext *cx, JSObject *obj, jsval id,
                                       uintN flags, JSObject **objp)
{
   try
   {
      auto priv = PrivateData::MustGetFromJSObject<PrivateMapMap>(cx, obj);

      // get property name
      if(JSVAL_IS_STRING(id))
      {
         JSString *jstr = JSVAL_TO_STRING(id);
         const char *name = JS_GetStringBytes(jstr);

         if(name && (priv->mapmap->find(name) != priv->mapmap->end()))
         {
            JSObject *newObj =
               JS_DefineObject(cx, obj, name, &lazyStringMapClass, nullptr,
                               JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);

            if(newObj)
            {
               // Private data for a LazyStringMap points to the stringmap
               std::unique_ptr<PrivateStringMap> newStrMap(new PrivateStringMap(&((*priv->mapmap)[name])));
               newStrMap->setToJSObjectAndRelease(cx, newObj, newStrMap);
               *objp = obj;
               return JS_TRUE;
            }
         }
      }
   }
   catch(const JSEngineError &)
   {
   }

   *objp = nullptr;
   return JS_TRUE;
}

//
// LazyMapStringMap_Enumerate
//
// Before enumeration, ensure all inner stringmaps have been reflected.
//
static JSBool LazyMapStringMap_Enumerate(JSContext *cx, JSObject *obj)
{
   try
   {
      auto priv = PrivateData::MustGetFromJSObject<PrivateMapMap>(cx, obj);
      auto itr = priv->mapmap->begin();

      for(; itr != priv->mapmap->end(); ++itr)
      {
         // JS_DefineObject will return null if the property already exists.
         JSObject *newObj =
            JS_DefineObject(cx, obj, itr->first.c_str(), &lazyStringMapClass, nullptr,
                            JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);

         // Private data for a LazyStringMap points to the stringmap
         if(newObj)
         {
            std::unique_ptr<PrivateStringMap> newStrMap(new PrivateStringMap(&itr->second));
            newStrMap->setToJSObjectAndRelease(cx, newObj, newStrMap);
         }
      }

      return JS_TRUE;
   }
   catch(const JSEngineError &)
   {
   }
   
   return JS_FALSE;
}

/** 
 * LazyMapStringMap JS Class.
 *
 * A lazy map of stringmaps.
 */
static JSClass lazyMapStringMapClass =
{
   "LazyMapStringMap",
   JSCLASS_HAS_PRIVATE | JSCLASS_NEW_RESOLVE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   LazyMapStringMap_Enumerate,
   (JSResolveOp)LazyMapStringMap_Resolve,
   JS_ConvertStub,
   JS_FinalizeStub,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivateMapMap, lazyMapStringMapClass)

static NativeInitCode LazyMapStringMap_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &lazyMapStringMapClass, nullptr,
                           0, nullptr, nullptr, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native lazyMapStringMapGlobalNative("LazyMapStringMap", LazyMapStringMap_Create);

//=============================================================================
//
// IniFile
//

static JSClass iniFileClass =
{
   "IniFile",
   0,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   JS_FinalizeStub,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

//
// IniFile_GetIniOptions
//
// Wrapper method for IniFile::GetIniOptions
// Note: returns a LazyMapStringMap instance into JS.
//
static JSBool IniFile_GetIniOptions(JSContext *cx, uintN argc, jsval *vp)
{
   JSObject *newObj = AssertJSNewObject(cx, &lazyMapStringMapClass, nullptr, nullptr);
   AutoNamedRoot anr(cx, newObj, "NewMapMap");
   std::unique_ptr<PrivateMapMap> nmm(new PrivateMapMap(&IniFile::GetIniOptions()));
   nmm->setToJSObjectAndRelease(cx, newObj, nmm);
   JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newObj));      
   return JS_TRUE;
}

/** 
 * IniFile JS Method Table
 */
static JSFunctionSpec iniFileFuncs[] =
{
   JSE_FN("getIniOptions", IniFile_GetIniOptions, 0, 0, 0),
   JS_FS_END
};

static NativeInitCode IniFile_Create(JSContext *cx, JSObject *global)
{
   JSObject *obj;

   if(!(obj = JS_DefineObject(cx, global, "iniFile", &iniFileClass, nullptr, JSPROP_PERMANENT)))
      return RESOLUTIONERROR;

   if(!JS_DefineFunctions(cx, obj, iniFileFuncs))
      return RESOLUTIONERROR;

   return RESOLVED;
}

static Native iniFileGlobalNative("iniFile", IniFile_Create);

//=============================================================================
//
// PrometheusDB
//

#ifndef VIBC_NO_VISUALIB

class PrivatePrometheusDB : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   PrometheusDB db;

   PrivatePrometheusDB() : PrivateData(), db()
   {
   }
};

//
// PrometheusDB_New - JS Constructor
//
// Construct a C++ PrometheusDB and wrap it in the new JavaScript object's
// private data field.
//
static JSBool PrometheusDB_New(JSContext *cx, JSObject *obj, uintN argc,
                               jsval *argv, jsval *rval)
{
   ASSERT_IS_CONSTRUCTING(cx, "PrometheusDB");

   std::unique_ptr<PrivatePrometheusDB> db(new PrivatePrometheusDB());
   db->setToJSObjectAndRelease(cx, obj, db);

   *rval = JSVAL_VOID;
   return JS_TRUE;
}

//
// PrometheusDB_Finalize - Class Finalization Hook
//
// Destroy the C++ PrometheusDB instance before the GC releases the JS proxy.
//
static void PrometheusDB_Finalize(JSContext *cx, JSObject *obj)
{
   auto priv = PrivateData::GetFromJSObject<PrivatePrometheusDB>(cx, obj);

   if(priv)
   {
      delete priv;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

/**
 * JSClass for PrometheusDB
 */
static JSClass prometheusDBClass =
{
   "PrometheusDB",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   PrometheusDB_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivatePrometheusDB, prometheusDBClass)

//
// PrometheusDB_Connect
//
// Wrapper method for PrometheusDB::connect
//
static JSBool PrometheusDB_Connect(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto   priv = PrivateData::GetFromThis<PrivatePrometheusDB>(cx, vp);
   JSBool res  = JS_FALSE;

   if(!priv)
      return JS_FALSE;

   if(argc >= 3)
   {
      const char *addr = SafeGetStringBytes(cx, argv[0], &argv[0]);
      const char *user = SafeGetStringBytes(cx, argv[1], &argv[1]);
      const char *pswd = SafeGetStringBytes(cx, argv[2], &argv[2]);

      res = priv->db.connect(addr, user, pswd) ? JS_TRUE : JS_FALSE;
   }
   else if(argc == 1)
   {
      const char *section = SafeGetStringBytes(cx, argv[0], &argv[0]);
      res = priv->db.connect(section) ? JS_TRUE : JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
   return JS_TRUE;
}

//
// PrometheusDB_IsConnected
//
// Wrapper method for PrometheusDB::isConnected
//
static JSBool PrometheusDB_IsConnected(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePrometheusDB>(cx, vp);

   JSBool res = (priv && priv->db.isConnected()) ? JS_TRUE : JS_FALSE;

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
   return JS_TRUE;
}

//
// PrometheusDB_Disconnect
//
// Wrapper method for PrometheusDB::disconnect
//
static JSBool PrometheusDB_Disconnect(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePrometheusDB>(cx, vp);
   if(!priv)
      return JS_FALSE;

   priv->db.disconnect();

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

//
// PrometheusDB_ExecuteStatement
//
static JSBool PrometheusDB_ExecuteStatement(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto   priv = PrivateData::GetFromThis<PrivatePrometheusDB>(cx, vp);
   if(!priv)
      return JS_FALSE;

   if(argc >= 1)
   {
      const char *sql = SafeGetStringBytes(cx, argv[0], &argv[0]);
      if(sql != "")
      {
         JSBool res = priv->db.executeStatement(sql) ? JS_TRUE : JS_FALSE;

         JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
         return JS_TRUE;
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_FALSE);
   return JS_TRUE;
}

//
// PrometheusDB_GetOneField
//
// Wrapper method for PrometheusDB::getOneField
//
static JSBool PrometheusDB_GetOneField(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto   priv = PrivateData::MustGetFromThis<PrivatePrometheusDB>(cx, vp);

   if(argc >= 1)
   {
      const char *sql = SafeGetStringBytes(cx, argv[0], &argv[0]);
      std::string results;

      priv->db.getOneField(sql, results);

      JSString *jres = AssertJSNewStringCopyZ(cx, results.c_str());
      JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jres));
   }
   else
      JS_SET_RVAL(cx, vp, JSVAL_NULL);

   return JS_TRUE;
}

//
// PrometheusDB_SQLToVecMap
//
// Wrapper method for PrometheusDB::sqlToVecMap
//
static JSBool PrometheusDB_SQLToVecMap(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 1)
   {
      const char *sql = SafeGetStringBytes(cx, argv[0], &argv[0]);
      pdb::vecmap vm;

      if(sql)
      {
         auto priv = PrivateData::MustGetFromThis<PrivatePrometheusDB>(cx, vp);
         priv->db.sqlToVecMap(sql, vm);

         JSObject *newObj = AssertJSNewObject(cx, &lazyVecMapClass, nullptr, nullptr);
         AutoNamedRoot anr(cx, newObj, "NewVecMap");
         AssertJSDefineFunctions(cx, newObj, lazyVecMapMethods);
         std::unique_ptr<PrivateVecMap> pvm(new PrivateVecMap());
         pvm->vecmap = vm;
         pvm->setToJSObjectAndRelease(cx, newObj, pvm);

         JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newObj));
         return JS_TRUE;
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

/** 
 * PrometheusDB JS Method Table
 */
static JSFunctionSpec prometheusDBFuncs[] =
{
   JSE_FN("connect",          PrometheusDB_Connect,          0, 0, 0),
   JSE_FN("isConnected",      PrometheusDB_IsConnected,      0, 0, 0),
   JSE_FN("disconnect",       PrometheusDB_Disconnect,       0, 0, 0),
   JSE_FN("executeStatement", PrometheusDB_ExecuteStatement, 1, 0, 0),
   JSE_FN("getOneField",      PrometheusDB_GetOneField,      1, 0, 0),
   JSE_FN("sqlToVecMap",      PrometheusDB_SQLToVecMap,      1, 0, 0),
   JS_FS_END
};

static NativeInitCode PrometheusDB_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &prometheusDBClass, 
                           JSEngineNativeWrapper<PrometheusDB_New>,
                           0, nullptr, prometheusDBFuncs, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native prometheusDBGlobalNative("PrometheusDB", PrometheusDB_Create);

//=============================================================================
//
// PrometheusTransaction
//

class PPTransaction : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   PrometheusTransaction ta;

   PPTransaction() : PrivateData(), ta()
   {
   }
};

//
// PrometheusTransaction_New - JS Constructor
//
// Construct a C++ PrometheusTransaction and wrap it in the new JavaScript
// object's private data field.
//
static JSBool PrometheusTransaction_New(JSContext *cx, JSObject *obj, uintN argc,
                                        jsval *argv, jsval *rval)
{
   ASSERT_IS_CONSTRUCTING(cx, "PrometheusTransaction");

   std::unique_ptr<PPTransaction> ta(new PPTransaction());
   ta->setToJSObjectAndRelease(cx, obj, ta);

   *rval = JSVAL_VOID;
   return JS_TRUE;
}

//
// PrometheusTransaction_Finalize - Class Finalization Hook
//
// Destroy the C++ PrometheusTransaction instance before the GC releases the
// JS proxy.
//
static void PrometheusTransaction_Finalize(JSContext *cx, JSObject *obj)
{
   auto priv = PrivateData::GetFromJSObject<PPTransaction>(cx, obj);

   if(priv)
   {
      delete priv;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

static JSBool PrometheusTransaction_IsActive(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PPTransaction>(cx, vp);
   if(!priv)
      return JS_FALSE;

   JSBool res = priv->ta.isActive() ? JS_TRUE : JS_FALSE;

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
   return JS_TRUE;
}

static JSBool PrometheusTransaction_Commit(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PPTransaction>(cx, vp);
   if(!priv)
      return JS_FALSE;

   JSBool res = priv->ta.commit() ? JS_TRUE : JS_FALSE;

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
   return JS_TRUE;
}

static JSBool PrometheusTransaction_Rollback(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PPTransaction>(cx, vp);
   if(!priv)
      return JS_FALSE;

   JSBool res = priv->ta.rollback() ? JS_TRUE : JS_FALSE;

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
   return JS_TRUE;
}

static JSBool PrometheusTransaction_StdTransaction(JSContext *cx, uintN argc,
                                                   jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto priv = PrivateData::GetFromThis<PPTransaction>(cx, vp);
   JSBool result = JS_FALSE;

   if(!priv)
      return JS_FALSE;

   if(argc >= 1 && SafeInstanceOf(cx, &prometheusDBClass, argv[0]))
   {
      JSObject *obj = JSVAL_TO_OBJECT(argv[0]);
      auto db = PrivateData::GetFromJSObject<PrivatePrometheusDB>(cx, obj);
      
      result = priv->ta.stdTransaction(db->db) ? JS_TRUE : JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(result));
   return JS_TRUE;
}

//
// PrometheusTransaction_ExecuteStatement
//
static JSBool PrometheusTransaction_ExecuteStatement(JSContext *cx, uintN argc,
                                                     jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto priv = PrivateData::GetFromThis<PPTransaction>(cx, vp);
   if(!priv)
      return JS_FALSE;

   if(argc >= 1)
   {
      const char *sql = SafeGetStringBytes(cx, argv[0], &argv[0]);
      if(sql != "")
      {
         JSBool res = priv->ta.executeStatement(sql) ? JS_TRUE : JS_FALSE;

         JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
         return JS_TRUE;
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_FALSE);
   return JS_TRUE;
}

//
// PrometheusTransaction_GetNextId
//
static JSBool PrometheusTransaction_GetNextId(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto priv = PrivateData::MustGetFromThis<PPTransaction>(cx, vp);

   if(argc >= 1)
   {
      const char *table = SafeGetStringBytes(cx, argv[0], &argv[0]);
      std::string result;

      priv->ta.getNextId(table, result);

      JSString *jres = AssertJSNewStringCopyZ(cx, result.c_str());
      JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jres));
   }
   else
      JS_SET_RVAL(cx, vp, JSVAL_NULL);

   return JS_TRUE;
}

//
// PrometheusTransaction_GetOneField
//
// Wrapper method for PrometheusTransaction::getOneField
//
static JSBool PrometheusTransaction_GetOneField(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto priv = PrivateData::MustGetFromThis<PPTransaction>(cx, vp);

   if(argc >= 1)
   {
      const char *sql = SafeGetStringBytes(cx, argv[0], &argv[0]);
      std::string results;

      priv->ta.getOneField(sql, results);

      JSString *jres = AssertJSNewStringCopyZ(cx, results.c_str());
      JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jres));
   }
   else
      JS_SET_RVAL(cx, vp, JSVAL_NULL);

   return JS_TRUE;
}

//
// PrometheusTransaction_SQLToVecMap
//
// Wrapper method for PrometheusTransaction::sqlToVecMap
//
static JSBool PrometheusTransaction_SQLToVecMap(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 1)
   {
      const char *sql = SafeGetStringBytes(cx, argv[0], &argv[0]);
      pdb::vecmap vm;
      if(sql)
      {
         auto priv = PrivateData::MustGetFromThis<PPTransaction>(cx, vp);
         priv->ta.sqlToVecMap(sql, vm);

         JSObject *newObj = AssertJSNewObject(cx, &lazyVecMapClass, nullptr, nullptr);
         AutoNamedRoot anr(cx, newObj, "NewVecMap");
         AssertJSDefineFunctions(cx, newObj, lazyVecMapMethods);
         std::unique_ptr<PrivateVecMap> pvm(new PrivateVecMap);
         pvm->vecmap = vm;
         pvm->setToJSObjectAndRelease(cx, newObj, pvm);
         JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newObj));
         return JS_TRUE;
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

//
// PrometheusTransaction_SQLToMap
//
// Wrapper method for PrometheusTransaction::sqlToMap
//
static JSBool PrometheusTransaction_SQLToMap(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 1)
   {
      const char *sql = SafeGetStringBytes(cx, argv[0], &argv[0]);
      pdb::stringmap sm;
      if(sql)
      {
         auto priv = PrivateData::MustGetFromThis<PPTransaction>(cx, vp);
         priv->ta.sqlToMap(sql, sm);

         JSObject *newObj = AssertJSNewObject(cx, &lazyStringMapClass, nullptr, nullptr);
         AutoNamedRoot anr(cx, newObj, "NewStringMap");
         std::unique_ptr<PrivateStringMap> psm(new PrivateStringMap());
         *psm->strmap = sm;
         psm->setToJSObjectAndRelease(cx, newObj, psm);
         JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newObj));
         return JS_TRUE;
      }
   }
   
   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

//
// PrometheusTransaction_GetFullRecord
//
// Wrapper method for PrometheusTransaction::getFullRecord
//
static JSBool PrometheusTransaction_GetFullRecord(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 2)
   {
      const char *tableName = SafeGetStringBytes(cx, argv[0], &argv[0]);
      const char *id        = SafeGetStringBytes(cx, argv[1], &argv[1]);
      pdb::stringmap sm;
      bool res = false;

      auto priv = PrivateData::MustGetFromThis<PPTransaction>(cx, vp);
      if((res = priv->ta.getFullRecord(tableName, id, sm)))
      {
         JSObject *newObj = AssertJSNewObject(cx, &lazyStringMapClass, nullptr, nullptr);
         AutoNamedRoot anr(cx, newObj, "NewStringMap");
         std::unique_ptr<PrivateStringMap> psm(new PrivateStringMap());
         *psm->strmap = sm;
         psm->setToJSObjectAndRelease(cx, newObj, psm);
         JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newObj));
         return JS_TRUE;
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

//
// PrometheusTransaction_Insert
//
// Wrapper method for PrometheusTransaction::executeInsertStatement
//
static JSBool PrometheusTransaction_Insert(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto priv = PrivateData::GetFromThis<PPTransaction>(cx, vp);
   if(!priv)
      return JS_FALSE;

   if(argc >= 3)
   {
      if(!JSVAL_IS_OBJECT(argv[1]) || !JSVAL_IS_OBJECT(argv[2]))
      {
         JS_SET_RVAL(cx, vp, JSVAL_FALSE);
         return JS_TRUE;
      }

      const char *tableName    = SafeGetStringBytes(cx, argv[0], &argv[0]);
      JSObject   *fieldMapObj  = JSVAL_TO_OBJECT(argv[1]);
      JSObject   *fieldOptsObj = JSVAL_TO_OBJECT(argv[2]);

      pdb::stringmap   fieldMap;
      pdb::strtointmap fieldOpts;

      if(!JSObjectToStringMap(cx, fieldMapObj,  fieldMap) ||
         !JSObjectToStrIntMap(cx, fieldOptsObj, fieldOpts))
      {
         JS_SET_RVAL(cx, vp, JSVAL_FALSE);
         return JS_TRUE;
      }

      JSBool res = priv->ta.executeInsertStatement(tableName, fieldMap, &fieldOpts);
      
      JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
      return JS_TRUE;
   }

   JS_SET_RVAL(cx, vp, JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool PrometheusTransaction_Update(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto priv = PrivateData::GetFromThis<PPTransaction>(cx, vp);
   if(!priv)
      return JS_FALSE;

   if(argc >= 3)
   {
      if(!JSVAL_IS_OBJECT(argv[1]) || !JSVAL_IS_OBJECT(argv[2]))
      {
         JS_SET_RVAL(cx, vp, JSVAL_FALSE);
         return JS_TRUE;
      }

      const char *tableName    = SafeGetStringBytes(cx, argv[0], &argv[0]);
      JSObject   *fieldMapObj  = JSVAL_TO_OBJECT(argv[1]);
      JSObject   *fieldOptsObj = JSVAL_TO_OBJECT(argv[2]);

      pdb::stringmap   fieldMap;
      pdb::strtointmap fieldOpts;

      if(!JSObjectToStringMap(cx, fieldMapObj,  fieldMap) ||
         !JSObjectToStrIntMap(cx, fieldOptsObj, fieldOpts))
      {
         JS_SET_RVAL(cx, vp, JSVAL_FALSE);
         return JS_TRUE;
      }

      JSBool res = priv->ta.executeUpdateStatement(tableName, fieldMap, &fieldOpts);
      
      JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
      return JS_TRUE;
   }

   JS_SET_RVAL(cx, vp, JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool PrometheusTransaction_LockForUpdate(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto priv = PrivateData::GetFromThis<PPTransaction>(cx, vp);
   if(!priv)
      return JS_FALSE;

   if(argc >= 2)
   {
      const char *tableName = SafeGetStringBytes(cx, argv[0], &argv[0]);
      const char *id        = SafeGetStringBytes(cx, argv[1], &argv[1]);

      JSBool res = priv->ta.lockForUpdate(tableName, id) ? JS_TRUE : JS_FALSE;

      JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
      return JS_TRUE;
   }

   JS_SET_RVAL(cx, vp, JSVAL_FALSE);
   return JS_TRUE;
}

/**
 * PrometheusTransaction JS Class
 */
static JSClass prometheusTransactionClass =
{
   "PrometheusTransaction",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   PrometheusTransaction_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PPTransaction, prometheusTransactionClass)

/**
 * PrometheusTransaction JS Methods Table
 */
static JSFunctionSpec prometheusTransactionFuncs[] =
{
   JSE_FN("isActive",               PrometheusTransaction_IsActive,         0, 0, 0),
   JSE_FN("commit",                 PrometheusTransaction_Commit,           0, 0, 0),
   JSE_FN("rollback",               PrometheusTransaction_Rollback,         0, 0, 0),
   JSE_FN("stdTransaction",         PrometheusTransaction_StdTransaction,   1, 0, 0),
   JSE_FN("getNextId",              PrometheusTransaction_GetNextId,        1, 0, 0),
   JSE_FN("executeStatement",       PrometheusTransaction_ExecuteStatement, 1, 0, 0),
   JSE_FN("executeInsertStatement", PrometheusTransaction_Insert,           3, 0, 0),
   JSE_FN("executeUpdateStatement", PrometheusTransaction_Update,           3, 0, 0),
   JSE_FN("getOneField",            PrometheusTransaction_GetOneField,      1, 0, 0),
   JSE_FN("sqlToVecMap",            PrometheusTransaction_SQLToVecMap,      1, 0, 0),
   JSE_FN("lockForUpdate",          PrometheusTransaction_LockForUpdate,    2, 0, 0),
   JSE_FN("sqlToMap",               PrometheusTransaction_SQLToMap,         1, 0, 0),
   JSE_FN("getFullRecord",          PrometheusTransaction_GetFullRecord,    2, 0, 0),
   JS_FS_END
};

static NativeInitCode PrometheusTransaction_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &prometheusTransactionClass, 
                           JSEngineNativeWrapper<PrometheusTransaction_New>,
                           0, nullptr, prometheusTransactionFuncs, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native prometheusTRGlobalNative("PrometheusTransaction", PrometheusTransaction_Create);

//=============================================================================
//
// PrometheusLookups
//

static JSBool PrometheusLookups_LoadLookup(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 2 && SafeInstanceOf(cx, &prometheusDBClass, argv[0]))
   {
      JSObject   *obj = JSVAL_TO_OBJECT(argv[0]);
      auto        pdb = PrivateData::GetFromJSObject<PrivatePrometheusDB>(cx, obj);
      const char *tbl = SafeGetStringBytes(cx, argv[1], &argv[1]);

      PrometheusLookups::LoadLookup(pdb->db, tbl);
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

static JSBool PrometheusLookups_LoadCustom(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 3 && SafeInstanceOf(cx, &prometheusDBClass, argv[0]))
   {
      JSObject   *obj = JSVAL_TO_OBJECT(argv[0]);
      auto        pdb = PrivateData::GetFromJSObject<PrivatePrometheusDB>(cx, obj);
      const char *tbl = SafeGetStringBytes(cx, argv[1], &argv[1]);
      const char *sql = SafeGetStringBytes(cx, argv[2], &argv[2]);

      PrometheusLookups::LoadCustom(pdb->db, tbl, sql);
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

static JSBool PrometheusLookups_PurgeLookups(JSContext *cx, uintN argc, jsval *vp)
{
   PrometheusLookups::PurgeLookups();

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

static JSBool PrometheusLookups_IdOf(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 2)
   {
      const char *table = SafeGetStringBytes(cx, argv[0], &argv[0]);
      const char *value = SafeGetStringBytes(cx, argv[1], &argv[1]);

      JSString *jstr;
      if((jstr = JS_NewStringCopyZ(cx, PrometheusLookups::IdOf(table, value).c_str())))
      {
         JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
         return JS_TRUE;
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

static JSBool PrometheusLookups_ValueOf(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 2)
   {
      const char *table = SafeGetStringBytes(cx, argv[0], &argv[0]);
      const char *id    = SafeGetStringBytes(cx, argv[1], &argv[1]);

      JSString *jstr;
      if((jstr = JS_NewStringCopyZ(cx, PrometheusLookups::ValueOf(table, id).c_str())))
      {
         JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
         return JS_TRUE;
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

static JSBool PrometheusLookups_IsValidID(JSContext *cx, uintN argc, jsval *vp)
{
   JSBool rval = JS_FALSE;
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 2)
   {
      const char *table = SafeGetStringBytes(cx, argv[0], &argv[0]);
      const char *id    = SafeGetStringBytes(cx, argv[1], &argv[1]);

      rval = PrometheusLookups::IsValidID(table, id) ? JS_TRUE : JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(rval));
   return JS_TRUE;
}

static JSBool PrometheusLookups_IsValidValue(JSContext *cx, uintN argc, jsval *vp)
{
   JSBool rval = JS_FALSE;
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 2)
   {
      const char *table = SafeGetStringBytes(cx, argv[0], &argv[0]);
      const char *value = SafeGetStringBytes(cx, argv[1], &argv[1]);

      rval = PrometheusLookups::IsValidValue(table, value) ? JS_TRUE : JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(rval));
   return JS_TRUE;
}

static JSClass prometheusLookupsClass =
{
   "PrometheusLookupsClass",
   0,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   JS_FinalizeStub,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSFunctionSpec prometheusLookupsFuncs[] =
{
   JSE_FN("LoadLookup",   PrometheusLookups_LoadLookup,   2, 0, 0),
   JSE_FN("LoadCustom",   PrometheusLookups_LoadCustom,   3, 0, 0),
   JSE_FN("PurgeLookups", PrometheusLookups_PurgeLookups, 0, 0, 0),
   JSE_FN("IdOf",         PrometheusLookups_IdOf,         2, 0, 0),
   JSE_FN("ValueOf",      PrometheusLookups_ValueOf,      2, 0, 0),
   JSE_FN("IsValidID",    PrometheusLookups_IsValidID,    2, 0, 0),
   JSE_FN("IsValidValue", PrometheusLookups_IsValidValue, 2, 0, 0),
   JS_FS_END
};

static NativeInitCode PrometheusLookups_Create(JSContext *cx, JSObject *global)
{
   JSObject *obj;

   if(!(obj = JS_DefineObject(cx, global, "PrometheusLookups", &prometheusLookupsClass, nullptr, JSPROP_PERMANENT)))
      return RESOLUTIONERROR;

   if(!JS_DefineFunctions(cx, obj, prometheusLookupsFuncs))
      return RESOLUTIONERROR;

   return RESOLVED;
}

static Native lookupsGlobalNative("PrometheusLookups", PrometheusLookups_Create);

#endif // VIBC_NO_VISUALIB

//=============================================================================
//
// String Utilities
//
// Obviously I'm not going to preemptively wrap the entirety of util.h, so if
// you need something not already in here, add it yourself ;)
//

// Various classes of functions exist amongst the Prometheus utilites libs:

/** String Conversion Function - takes an existing string and returns a new one */
typedef std::string (*StringConversionFunc)(const std::string &);

/** String Conversion w/string argument */
typedef std::string (*StringConversionArgFunc)(const std::string &, const std::string &);

/** String Conversion w/bool argument */
typedef std::string (*StringConversionBoolFunc)(const std::string &, bool);

/** String Tester - takes a string and returns bool */
typedef bool (*StringTestFunc)(const std::string &);

/** String Comparison - takes two strings and returns bool. */
typedef bool (*StringComparisonFunc)(const std::string &, const std::string &);

/** String From Bool - take a bool, return a string */
typedef std::string (*StringFromBoolFunc)(bool);

/** String Generator - takes nothing, returns a string */
typedef std::string (*StringGeneratorFunc)();

// And, we need a wrapper to thunk arguments and return values for each sort:

//
// StringConversionWrapper
//
// Call a native function that takes one string and returns a new one, having
// performed some conversion operation on it.
//
static JSBool StringConversionWrapper(JSContext *cx, uintN argc, jsval *vp,
                                      StringConversionFunc func)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 1)
   {
      const char *cstr = SafeGetStringBytes(cx, argv[0], &argv[0]);
      JSString *jstr;
      
      if((jstr = JS_NewStringCopyZ(cx, func(cstr).c_str())))
      {
         JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
         return JS_TRUE;
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

//
// StringConversionArgWrapper
//
// Call a native function that takes two strings and returns a new one, having
// performed some conversion operation on the first string using the second as
// a parameter.
//
static JSBool StringConversionArgWrapper(JSContext *cx, uintN argc, jsval *vp,
                                         StringConversionArgFunc func)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 2)
   {
      const char *cstr1 = SafeGetStringBytes(cx, argv[0], &argv[0]);
      const char *cstr2 = SafeGetStringBytes(cx, argv[1], &argv[1]);
      JSString *jstr;

      if((jstr = JS_NewStringCopyZ(cx, func(cstr1, cstr2).c_str())))
      {
         JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
         return JS_TRUE;
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

//
// StringConversionBoolWrapper
//
// Call a native function that takes one string and a boolean argument,
// and uses those together to produce a new string with some type of
// conversion applied to it.
//
static JSBool StringConversionBoolWrapper(JSContext *cx, uintN argc, jsval *vp,
                                          StringConversionBoolFunc func)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 1)
   {
      JSString *jstr;
      JSBool bArg = JS_FALSE;
      const char *cstr = SafeGetStringBytes(cx, argv[0], &argv[0]);
      if(argc >= 2)
      {
         if(!JS_ValueToBoolean(cx, argv[1], &bArg))
            bArg = JS_FALSE; // value could be undefined if fails
      }

      if((jstr = JS_NewStringCopyZ(cx, func(cstr, !!bArg).c_str())))
      {
         JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
         return JS_TRUE;
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

//
// StringTestWrapper
//
// Call a native function that takes one string and returns a boolean, having
// tested some property of the string in question.
//
static JSBool StringTestWrapper(JSContext *cx, uintN argc, jsval *vp,
                                StringTestFunc func)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 1)
   {
      const char *cstr = SafeGetStringBytes(cx, argv[0], &argv[0]);

      JSBool res = func(cstr) ? JS_TRUE : JS_FALSE;

      JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
      return JS_TRUE;
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

//
// StringComparisonWrapper
//
// Call a native function that takes two strings and returns a comparison
// boolean value.
//
static JSBool StringComparisonWrapper(JSContext *cx, uintN argc, jsval *vp,
                                      StringComparisonFunc func)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 2)
   {
      const char *cstr1 = SafeGetStringBytes(cx, argv[0], &argv[0]);
      const char *cstr2 = SafeGetStringBytes(cx, argv[1], &argv[1]);

      JSBool res = func(cstr1, cstr2) ? JS_TRUE : JS_FALSE;

      JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(res));
      return JS_TRUE;
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

//
// StringFromBoolWrapper
//
// Call a native function that takes one bool and generates a string.
//
static JSBool StringFromBoolWrapper(JSContext *cx, uintN argc, jsval *vp,
                                    StringFromBoolFunc func)
{
   jsval *argv = JS_ARGV(cx, vp);

   JSString *jstr;
   JSBool bArg = JS_FALSE;
   if(argc >= 1)
   {
      if(!JS_ValueToBoolean(cx, argv[0], &bArg))
         bArg = JS_FALSE; // value could be undefined if fails
   }

   if((jstr = JS_NewStringCopyZ(cx, func(!!bArg).c_str())))
   {
      JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
      return JS_TRUE;
   }
   else
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }
}

//
// StringGeneratorWrapper
//
// Call a native function that takes nothing and returns a string.
//
static JSBool StringGeneratorWrapper(JSContext *cx, uintN argc, jsval *vp,
                                     StringGeneratorFunc func)
{
   JSString *jstr;

   if((jstr = JS_NewStringCopyZ(cx, func().c_str())))
   {
      JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
      return JS_TRUE;
   }
   else
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }
}

//=============================================================================
//
// Integer Utilities
//
// Same deal, but these functions take one or more integers.
//

/** Integer Test Function - take an int, and return a boolean */
typedef bool (*IntTestFunc)(int);

static JSBool IntTestWrapper(JSContext *cx, uintN argc, jsval *vp, 
                             IntTestFunc func)
{
   jsval *argv = JS_ARGV(cx, vp);
   int32 ji = 0;
   bool  cppres = false;

   if(argc >= 1)
   {
      if(JS_ValueToECMAInt32(cx, argv[0], &ji))
         cppres = func(static_cast<int>(ji));
   }

   JS_SET_RVAL(cx, vp, cppres ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

/** Integer Dyadic - takes two, returns one. */
typedef int (*IntDyadicFunc)(int, int);

static JSBool IntDyadicWrapper(JSContext *cx, uintN argc, jsval *vp,
                               IntDyadicFunc func)
{
   jsval *argv = JS_ARGV(cx, vp);
   int32 ja = 0, jb = 0;

   if(argc >= 1)
      JS_ValueToECMAInt32(cx, argv[0], &ja);
   if(argc >= 2)
      JS_ValueToECMAInt32(cx, argv[1], &jb);

   jsint ret = static_cast<jsint>(func(static_cast<int>(ja), static_cast<int>(jb)));

   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ret));
   return JS_TRUE;
}

//=============================================================================
//
// Auto Native Table Building
//

NativeWrapper *NativeWrapper::wrappers;

/** 
 * Use this macro to create the JavaScript FastNative function for each native
 * you want to support. A NativeWrapper object instance is also instantiated,
 * and each of these throws itself onto the class's linked list when it is
 * constructed at program startup. That linked list is walked below in
 * NativeWrapper::AddNatives, so that each native function gets reflected into
 * JS as a method of the "Utils" object, which is a property of the JS global.
 *
 * How it works on the call stack:
 * - JavaScript Interpreter            (a script has called a native method)
 *   - NATIVE_WRAPPER-defined Function (a JSFastNative callback)
 *      - wrapperFunc                  (handle arg/retval conversions - "thunk")
 *         - nativeFunc                (just a normal C++ function)
 */
#define NATIVE_WRAPPER(nativeFunc, numArgs, wrapperFunc)                 \
   static JSBool nativeFunc ## WF (JSContext *cx, uintN argc, jsval *vp) \
   {                                                                     \
      try { return wrapperFunc(cx, argc, vp, nativeFunc); }              \
      catch(...) {                                                       \
         /* We must not propagate any exceptions here */                 \
         JS_ReportError(cx, "Exception in native method");               \
         return JS_FALSE;                                                \
      }                                                                  \
   }                                                                     \
   static NativeWrapper nativeFunc ## Wrapper(#nativeFunc, nativeFunc ## WF, numArgs)

NATIVE_WRAPPER(AcronymString,         1, StringConversionWrapper);
NATIVE_WRAPPER(CollapseSpaces,        1, StringConversionWrapper);
NATIVE_WRAPPER(ConvertBinary,         1, StringConversionWrapper);
NATIVE_WRAPPER(ConvertBool,           1, StringConversionWrapper);
NATIVE_WRAPPER(ConvertTime,           1, StringConversionWrapper);
NATIVE_WRAPPER(CurrentDate,           0, StringGeneratorWrapper);
NATIVE_WRAPPER(CurrentTime,           0, StringGeneratorWrapper);
NATIVE_WRAPPER(FormatAddress,         1, StringConversionWrapper);
NATIVE_WRAPPER(FormatPhoneNumber,     1, StringConversionWrapper);
NATIVE_WRAPPER(FormatZipCode,         1, StringConversionWrapper);
NATIVE_WRAPPER(GenerateUUID,          0, StringGeneratorWrapper);
NATIVE_WRAPPER(InitialsSearchString1, 1, StringConversionWrapper);
NATIVE_WRAPPER(InitialsSearchString2, 1, StringConversionWrapper);
NATIVE_WRAPPER(IsBlankOrZero,         1, StringTestWrapper);
NATIVE_WRAPPER(IsBracketed,           1, StringTestWrapper);
NATIVE_WRAPPER(IsFloat,               1, StringTestWrapper);
NATIVE_WRAPPER(IsInt,                 1, StringTestWrapper);
NATIVE_WRAPPER(IsLeapYear,            1, IntTestWrapper);
NATIVE_WRAPPER(LastDayOf,             2, IntDyadicWrapper);
NATIVE_WRAPPER(LikeString,            2, StringComparisonWrapper);
NATIVE_WRAPPER(LowercaseFirstChar,    1, StringConversionWrapper);
NATIVE_WRAPPER(LowercaseString,       1, StringConversionWrapper);
NATIVE_WRAPPER(PowerSearchString,     1, StringConversionWrapper);
NATIVE_WRAPPER(PrettyDate,            1, StringConversionWrapper);
NATIVE_WRAPPER(PrettySql,             1, StringConversionWrapper);
NATIVE_WRAPPER(PrettySSN,             1, StringConversionWrapper);
NATIVE_WRAPPER(PrettyString,          1, StringConversionWrapper);
NATIVE_WRAPPER(RightNow,              1, StringFromBoolWrapper);
NATIVE_WRAPPER(SafeSQLString,         2, StringConversionBoolWrapper);
NATIVE_WRAPPER(SentenceString,        1, StringConversionWrapper);
NATIVE_WRAPPER(StripAll,              2, StringConversionArgWrapper);
NATIVE_WRAPPER(StripAllExcept,        2, StringConversionArgWrapper);
NATIVE_WRAPPER(StripFirst,            1, StringConversionWrapper);
NATIVE_WRAPPER(StripLast,             1, StringConversionWrapper);
NATIVE_WRAPPER(StripLeading,          2, StringConversionArgWrapper);
NATIVE_WRAPPER(StripSurrounding,      2, StringConversionArgWrapper);
NATIVE_WRAPPER(StripTrailing,         2, StringConversionArgWrapper);
NATIVE_WRAPPER(TwoDecimals,           1, StringConversionWrapper);
NATIVE_WRAPPER(UppercaseCompare,      2, StringComparisonWrapper);
NATIVE_WRAPPER(UppercaseFirstChar,    1, StringConversionWrapper);
NATIVE_WRAPPER(UppercaseString,       1, StringConversionWrapper);

//
// unique functions
//

// LoadTextFile
static JSBool LoadTextFileWrapper(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc >= 1)
   {
      char *fileText = LoadTextFile(SafeGetStringBytes(cx, argv[0], &argv[0]));

      if(fileText)
      {
         JSString *jstr = JS_NewStringCopyZ(cx, fileText);
         JS_SET_RVAL(cx, vp, jstr ? STRING_TO_JSVAL(jstr) : JSVAL_NULL);
         delete [] fileText;
         return JS_TRUE;
      }
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

static NativeWrapper loadTextFileWrapper("LoadTextFile", LoadTextFileWrapper, 1);

// IsIn
// Not actually a wrapper; rather, a reimplementation that doesn't use varargs.
static JSBool IsInWrapper(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   std::set<std::string> argset;

   if(argc < 2)
   {
      JS_ReportError(cx, "Not enough arguments to IsIn");
      return JS_FALSE;
   }

   for(uintN i = 1; i < argc; i++)
      argset.insert(SafeGetStringBytes(cx, argv[i], &argv[i]));

   const char *val = SafeGetStringBytes(cx, argv[0], &argv[0]);

   JS_SET_RVAL(cx, vp, argset.find(val) != argset.end() ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static NativeWrapper isInWrapper("IsIn", IsInWrapper, 0);

/** This class is used for all utilities. */
static JSClass utilsClass =
{
   "UtilsClass",
   0,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   JS_FinalizeStub,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

NativeInitCode NativeWrapper::Utils_Create(JSContext *cx, JSObject *global)
{
   JSObject *obj;

   if(!(obj = JS_DefineObject(cx, global, "Utils", &utilsClass, nullptr, JSPROP_PERMANENT)))
      return RESOLUTIONERROR;

   NativeWrapper *nw = NativeWrapper::wrappers;
   while(nw)
   {
      JS_DefineFunction(cx, obj, nw->name, (JSNative)nw->func, nw->numargs,
                        JSFUN_FAST_NATIVE | JSFUN_STUB_GSOPS);
      nw = nw->next;
   }
   
   return RESOLVED;
}

static Native utilsGlobalNative("Utils", NativeWrapper::Utils_Create);

//=============================================================================
//
// Enumerations
//

/**
 * This JSClass is used for all enumerations.
 * Enumerations are reflectable as JS objects having one read-only integer
 * property per enumeration line.
 */
JSClass enumClass =
{
   "Enum",
   0,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   JS_FinalizeStub,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

void AddEnumerationProperties(JSContext *cx, JSObject *obj, EnumEntry *enumEntries)
{
   EnumEntry *curEntry = enumEntries;

   while(curEntry->name)
   {
      JS_DefineProperty(cx, obj, curEntry->name, INT_TO_JSVAL(curEntry->value),
                        nullptr, nullptr,
                        JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
      ++curEntry;
   }
}

#ifndef VIBC_NO_VISUALIB

/**
 * SQLOptions JS Enumeration.
 * Needed, ex., for third argument to PrometheusTransaction::executeInsertStatement
 */
static EnumEntry sqlFlagsEnum[] =
{
   ENUMENTRY(sql_full_processing),
   ENUMENTRY(sql_no_uppercasing),
   ENUMENTRY(sql_no_quoting),
   ENUMENTRY(sql_no_left_trimming),
   ENUMENTRY(sql_no_right_trimming),
   ENUMENTRY(sql_no_trimming),
   ENUMENTRY(sql_no_processing),
   ENUMEND()
};

static NativeInitCode SQLFlags_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_DefineObject(cx, global, "SQLOptions", &enumClass, nullptr, 0);
   if(obj)
   {
      AddEnumerationProperties(cx, obj, sqlFlagsEnum);
      return RESOLVED;
   }
   else
      return RESOLUTIONERROR;
}

static Native sqlFlagsEnumNative("SQLOptions", SQLFlags_Create);

#endif // VIBC_NO_VISUALIB

static EnumEntry dateTimeFlagsEnum[] =
{
   ENUMENTRY(unknown),
   ENUMENTRY(yy),
   ENUMENTRY(yyyy),
   ENUMENTRY(mdy),
   ENUMENTRY(ymd),
   ENUMENTRY(dmy),
   ENUMENTRY(ampm),
   ENUMENTRY(military),
   ENUMENTRY(timefirst),
   ENUMENTRY(datefirst),
   ENUMENTRY(withsec),
   ENUMENTRY(withoutsec),
   ENUMENTRY(no_zeros),
   ENUMENTRY(left_zeros),
   ENUMENTRY(separators),
   ENUMENTRY(no_separators),
   ENUMENTRY(withmillis),
   ENUMENTRY(withoutmillis),
   ENUMENTRY(global_mode), 
   ENUMENTRY(datetimeoffset_mode),
   ENUMEND()
};

static NativeInitCode DateTimeFlags_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_DefineObject(cx, global, "DateTimeFlags", &enumClass, nullptr, 0);
   if(obj)
   {
      AddEnumerationProperties(cx, obj, dateTimeFlagsEnum);
      return RESOLVED;
   }
   else
      return RESOLUTIONERROR;
}

static Native dateTimeEnumNative("DateTimeFlags", DateTimeFlags_Create);

static EnumEntry dtModTypes[] =
{
   ENUMENTRY(in_undef),
   ENUMENTRY(in_seconds),
   ENUMENTRY(in_minutes),
   ENUMENTRY(in_hours),
   ENUMENTRY(in_days),
   ENUMENTRY(in_weeks),
   ENUMENTRY(in_months),
   ENUMENTRY(in_years),
   ENUMENTRY(in_msec),
   ENUMEND()
};

static NativeInitCode DateTimeMod_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_DefineObject(cx, global, "DateTimeMod", &enumClass, nullptr, 0);
   if(obj)
   {
      AddEnumerationProperties(cx, obj, dtModTypes);
      return RESOLVED;
   }
   else
      return RESOLUTIONERROR;
}

static Native dtFmtEnumNative("DateTimeMod", DateTimeMod_Create);


// EOF
