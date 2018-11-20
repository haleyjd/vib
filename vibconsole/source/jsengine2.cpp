/*

   JS Engine 2.0

   This is a refinement of the module that is in hl7daemon, making use of
   lessons I have learned with Aeon in Eternity.

*/

#include <iostream>
#include <map>
#include <string>

#include "jsengine2.h"
#include "jsnatives.h"
#include "util.h"

static JSRuntime     *runtime;
static JSEvalContext *gContext;

#define RUNTIME_HEAP_SIZE 64L * 1024L * 1024L
#define STACK_CHUNK_SIZE  8192

//=============================================================================
//
// Utilities
//

//
// Takes some of the pain out of converting jsval to JSString by handling
// return value checks and GC rooting. "root" is NOT an optional parameter!
// This version is for use inside JSNative/JSFastNative callbacks where
// the string can be rooted to an argument or return value.
//
const char *SafeGetStringBytes(JSContext *cx, jsval value, jsval *root)
{
   const char *retval = "";
   JSString   *jstr   = JS_ValueToString(cx, value);

   if(jstr)
   {
      *root  = STRING_TO_JSVAL(jstr);
      if(!(retval = JS_GetStringBytes(jstr)))
         retval = "";
   }

   return retval;
}

//
// Overload taking a reference to an AutoNamedRoot object. For use in
// contexts where value roots are not available, such as when dealing with
// JSAPI return values from JS_EvaluateString etc.
//
const char *SafeGetStringBytes(JSContext *cx, jsval value, AutoNamedRoot &root)
{
   const char *retval = "";
   
   try
   {
      auto jstr = AssertJSValueToStringRooted(cx, value, root);
      if(!(retval = JS_GetStringBytes(jstr)))
         retval = "";
   }
   catch(const JSEngineError &)
   {
   }

   return retval;
}

//
// SafeInstanceOf
//
// Test if a jsval is an object, and if so, if it is an instance of the
// provided JSClass.
//
bool SafeInstanceOf(JSContext *cx, JSClass *jsClass, jsval val)
{
   JSObject *obj;

   return (JSVAL_IS_OBJECT(val) &&
           (obj = JSVAL_TO_OBJECT(val)) &&
           JS_InstanceOf(cx, obj, jsClass, nullptr));
}

void AssertInstanceOf(JSContext *cx, JSClass *jsClass, jsval val)
{
   if(!SafeInstanceOf(cx, jsClass, val))
   {
      std::string className = jsClass->name ? jsClass->name : "undefined";
      std::string str = "Object is not an instance of " + className;
      throw JSEngineError(str);
   }
}

//
// Safely convert a jsid to a JSString
//
JSString *SafeIdToString(JSContext *cx, jsid id)
{
   jsval value = JSVAL_VOID;

   if(JS_IdToValue(cx, id, &value) && JSVAL_IS_STRING(value))
      return JSVAL_TO_STRING(value);
   else
      return nullptr;
}

//
// SafeIdToChars
//
// For a rooted property id only.
//
const char *SafeIdToChars(JSContext *cx, jsid id)
{
   JSString *jstr;

   if((jstr = SafeIdToString(cx, id)))
      return JS_GetStringBytes(jstr);
   else
      return nullptr;
}

jschar *SafeIdToJSChars(JSContext *cx, jsid id, size_t &size)
{
   JSString *jstr;
   if((jstr = SafeIdToString(cx, id)))
   {
      size = JS_GetStringLength(jstr);
      return JS_GetStringChars(jstr);
   }
   else
      return nullptr;
}

//
// SafePropertyString
//
// Assuming a named property belonging to a rooted object, converts the named
// property to a JSString *, if it exists and is a string. Otherwise, null is
// returned.
//
JSString *SafePropertyString(JSContext *cx, JSObject *obj, const char *propName)
{
   jsval propVal = JSVAL_VOID;

   if(JS_GetProperty(cx, obj, propName, &propVal) && JSVAL_IS_STRING(propVal))
      return JSVAL_TO_STRING(propVal);
   else
      return nullptr;
}

//
// SafePropertyChars
//
// As above, but also converts the JSString by calling JS_GetStringBytes.
// Again, everything had better be rooted already.
//
const char *SafePropertyChars(JSContext *cx, JSObject *obj, const char *propName)
{
   JSString *jstr;

   if((jstr = SafePropertyString(cx, obj, propName)))
      return JS_GetStringBytes(jstr);
   else
      return nullptr;
}

//
// AutoJSPropertyIterator
//
// Acquires and releases a GC root automatically while creating a JS
// property iterator, to protect the iterator object.
//
class AutoJSPropertyIterator
{
protected:
   JSBool     rooted;
   JSContext *cx;
   JSObject  *itr;

public:
   AutoJSPropertyIterator(JSContext *pcx, JSObject *obj)
      : cx(pcx), itr(nullptr), rooted(JS_FALSE)
   {
      if((itr = JS_NewPropertyIterator(cx, obj)))
         rooted = JS_AddNamedRoot(cx, &itr, "AutoJSPropertyIterator");
   }

   virtual ~AutoJSPropertyIterator()
   {
      if(itr && rooted)
         JS_RemoveRoot(cx, &itr);
      cx  = nullptr;
      itr = nullptr;
      rooted = false;
   }

   bool valid() const { return (itr && rooted); }
   JSObject *getObject() { return itr; }
};

class AutoJSPropertyIteratorAsserted : public AutoJSPropertyIterator
{
protected:
   bool valid() const { return AutoJSPropertyIterator::valid(); }

public:
   AutoJSPropertyIteratorAsserted(JSContext *cx, JSObject *obj)
      : AutoJSPropertyIterator(cx, obj)
   {
      if(!itr || !rooted)
         throw JSEngineError("Cannot create rooted JSProperty iterator");
   }
};

//
// Convert a JavaScript object's enumerable properties into a map<string, string>
//
bool JSObjectToStringMap(JSContext *cx, JSObject *obj, std::map<std::string, std::string> &strMap)
{
   AutoJSPropertyIterator itr(cx, obj);

   if(!itr.valid())
      return false;

   jsid cur_id;
   while(JS_NextProperty(cx, itr.getObject(), &cur_id) && cur_id != JSVAL_VOID)
   {
      const char *propName = SafeIdToChars(cx, cur_id);
      if(!propName)
         continue;

      const char *propValue = SafePropertyChars(cx, obj, propName);
      if(!propValue)
         continue;

      strMap[propName] = propValue;
   }

   return true;
}

void AssertJSObjectToStringMap(JSContext *cx, JSObject *obj, std::map<std::string, std::string> &strMap)
{
   AutoJSPropertyIteratorAsserted itr(cx, obj); // throws if fails to create or root
   jsid cur_id;

   while(JS_NextProperty(cx, itr.getObject(), &cur_id) && cur_id != JSVAL_VOID)
   {
      const char *propName = SafeIdToChars(cx, cur_id);
      if(!propName)
         continue;

      const char *propValue = SafePropertyChars(cx, obj, propName);
      if(!propValue)
         continue;

      strMap[propName] = propValue;
   }
}

//
// Convert a JavaScript object's enumerable properties into a map<string, int>
//
bool JSObjectToStrIntMap(JSContext *cx, JSObject *obj, std::map<std::string, int> &strIntMap)
{
   AutoJSPropertyIterator itr(cx, obj);

   if(!itr.valid())
      return false;

   jsid cur_id;
   while(JS_NextProperty(cx, itr.getObject(), &cur_id) && cur_id != JSVAL_VOID)
   {
      const char *propName = SafeIdToChars(cx, cur_id);
      if(!propName)
         continue;

      jsval propValue = JSVAL_VOID;
      if(JS_GetProperty(cx, obj, propName, &propValue) && JSVAL_IS_INT(propValue))
         strIntMap[propName] = static_cast<int>(JSVAL_TO_INT(propValue));
   }

   return true;
}

//
// Convert jsval to integer using the global context
//
int JSEngine_ValueToInteger(jsval *val)
{
   int32 ret = 0;
   JS_ValueToECMAInt32(gContext->getContext(), *val, &ret);
   return static_cast<int>(ret);
}

//=============================================================================
//
// Evaluation Contexts
//

// JSEvalContext - Private implementation object
class JSEvalContextPimpl
{
public:
   AutoContext  autoContext; // context for execution
   JSObject    *global;      // JS global object

   JSEvalContextPimpl()
      : autoContext(runtime, STACK_CHUNK_SIZE),
        global(nullptr)
   {
   }
};

//
// JSEvalContext Constructor
//
JSEvalContext::JSEvalContext() 
{
   pImpl = new JSEvalContextPimpl();
}

//
// EvalContext Destructor
//
JSEvalContext::~JSEvalContext()
{
   if(pImpl)
   {
      delete pImpl;
      pImpl = nullptr;
   }
}

//
// JSEvalContext::setGlobal
//
// Instantiate an object of the indicated JSClass as the global object of the
// contained JSContext.
//
bool JSEvalContext::setGlobal(JSClass *globalClass, JSObject *parent)
{
   JSContext *ctx = pImpl->autoContext.ctx;

   if(ctx && (pImpl->global = JS_NewObject(ctx, globalClass, nullptr, parent)))
   {
      JS_SetGlobalObject(ctx, pImpl->global);
      return true;
   }
   else
      return false;
}

//
// JSEvalContext::getContext
//
// Get the JSContext for this evaluation context.
//
JSContext *JSEvalContext::getContext() const
{
   return pImpl->autoContext.ctx;
}

//
// JSEvalContext::getGlobal
//
// Get a pointer to the global object instance for this evaluation context.
//
JSObject *JSEvalContext::getGlobal() const
{
   return pImpl->global;
}

//============================================================================
//
// Native
//
// Management of JS native classes/objects/methods, for support of
// memory- and time-efficient lazy property resolution.
//

// Static hash table
Native *Native::chains[NUMCHAINS];

//
// D_HashTableKeyCase
//
// Based on SGI STL string hash; fast, with good hash key properties.
// Available under BSD license.
//
static inline unsigned int D_HashTableKeyCase(const char *str)
{
   const char *c = str;
   unsigned int h = 0;

   while(*c)
   {
      h = 5 * h + *c;
      ++c;
   }

   return h;
}

//
// Native::add
//
// Add an instance to the class's static hash table when it is constructed.
//
void Native::add()
{
   unsigned int hc = D_HashTableKeyCase(name) % NUMCHAINS;

   next = chains[hc];
   chains[hc] = this;
}

//
// Native::InitByName
//
// Lookup the native by name and then initialize it within a specific JS
// context and object.
//
NativeInitCode Native::InitByName(const char *name, JSContext *cx, JSObject *obj)
{
   unsigned int hc = D_HashTableKeyCase(name) % NUMCHAINS;
   Native *cur = chains[hc];

   while(cur && strcmp(cur->name, name))
      cur = cur->next;

   return cur ? cur->init(cx, obj) : NOSUCHPROPERTY;
}

//
// Native::EnumerateAll
//
// Add all lazy-resolved natives to the context/object that haven't
// already been added, "just-in-time" for enumeration of the context
// globals.
//
NativeInitCode Native::EnumerateAll(JSContext *cx, JSObject *obj)
{
   NativeInitCode ret = RESOLVED;

   // Run down each hash chain
   for(int i = 0; i < NUMCHAINS; i++)
   {
      Native *cur = chains[i];

      while(cur)
      {
         JSBool found = JS_FALSE;
         if(!JS_AlreadyHasOwnProperty(cx, obj, cur->name, &found))
         {
            ret = RESOLUTIONERROR; // JSAPI error
            break;
         }
         else if(!found) // Not already defined?
         {
            NativeInitCode initCode = cur->init(cx, obj);
            if(initCode == RESOLUTIONERROR)
            {
               ret = initCode; // JSAPI error in init callback
               break;
            }
         }
         cur = cur->next;
      }
   }

   return ret;
}

//=============================================================================
//
// Global JavaScript Object and Methods
//

//
// Enumeration callback for the global JS object class. Enumerate all Native
// objects so that they add themselves to the global as properties or methods.
//
static JSBool global_enumerate(JSContext *cx, JSObject *obj)
{
   return (JS_EnumerateStandardClasses(cx, obj) && 
           Native::EnumerateAll(cx, obj) == RESOLVED);
}

//
// Resolution callback for the global JS object class. If the named property
// is not a standard symbol, check the Native object registry for a match.
//
static JSBool global_resolve(JSContext *cx, JSObject *obj, jsval id, uintN flags,
                             JSObject **objp)
{
   if((flags & JSRESOLVE_ASSIGNING) == 0)
   {
      JSBool resolved;

      if(!JS_ResolveStandardClass(cx, obj, id, &resolved))
         return JS_FALSE;

      if(resolved)
      {
         *objp = obj;
         return JS_TRUE;
      }
      else if(JSVAL_IS_STRING(id))
      {
         // Check Natives
         JSString   *idstr = JSVAL_TO_STRING(id);
         const char *name  = JS_GetStringBytes(idstr);

         switch(Native::InitByName(name, cx, obj))
         {
         case RESOLVED:
            *objp = obj;
            break;
         case NOSUCHPROPERTY:
            break; // That's fine, return true below.
         case RESOLUTIONERROR:
            return JS_FALSE; // This on the other hand means something is wrong
         }
      }
   }

   return JS_TRUE;
}

// The global object class.
static JSClass global_class =
{
   "global",                                   // name
   JSCLASS_NEW_RESOLVE | JSCLASS_GLOBAL_FLAGS, // flags
   JS_PropertyStub,                            // addProperty
   JS_PropertyStub,                            // delProperty
   JS_PropertyStub,                            // getProperty
   JS_PropertyStub,                            // setProperty
   global_enumerate,                           // enumerate
   (JSResolveOp)global_resolve,                // resolve
   JS_ConvertStub,                             // convert
   JS_FinalizeStub,                            // finalize
   JSCLASS_NO_OPTIONAL_MEMBERS                 // getObjectOps etc.
};

// Sandbox is used as the global when loading modules
static JSClass sandbox_class =
{
   "Sandbox",
   JSCLASS_NEW_RESOLVE | JSCLASS_GLOBAL_FLAGS,
   JS_PropertyStub,                            
   JS_PropertyStub,                            
   JS_PropertyStub,                             
   JS_PropertyStub,                            
   global_enumerate,                            
   (JSResolveOp)global_resolve,                
   JS_ConvertStub,                              
   JS_FinalizeStub,                             
   JSCLASS_NO_OPTIONAL_MEMBERS                  
};

// The restricted global object has no access to native objects.
// This is for use in security-sensitive applications such as JSON evaluation.
static JSClass restricted_class =
{
   "RestrictedContext",
   JSCLASS_GLOBAL_FLAGS,
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

//=============================================================================
//
// Core Engine Routines
//

#ifndef VIBC_NO_WIN32
#include <Windows.h>
#endif

static void SetErrorReportConsoleAttribs()
{
#ifndef VIBC_NO_WIN32
   HANDLE hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
   SetConsoleTextAttribute(hConOut, FOREGROUND_RED|FOREGROUND_INTENSITY);
#endif
}

static void ResetConsoleAttribs()
{
#ifndef VIBC_NO_WIN32
   HANDLE hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
   SetConsoleTextAttribute(hConOut, FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
#endif
}

//
// Error reporting callback inserted into all JSContext objects; prints error
// information to the console.
//
static void ErrorReport(JSContext *cx, const char *message, JSErrorReport *report)
{
   std::string msg;
   bool        addedFile = false;

   if(!report)
   {
      msg = message;
      std::cout << msg << std::endl;
      if(echoFile)
      {
         echoFile << msg << std::endl;
         echoFile.flush();
      }
      return;
   }
   
   if(report->filename)
   {
      msg += "In file " + std::string(report->filename);
      addedFile = true;

      if(report->lineno)
         msg += " @ line " + IntToString(static_cast<int>(report->lineno));
   }
   if(addedFile)
      msg += ":\n";
   if(JSREPORT_IS_WARNING(report->flags))
      msg += "Warning: ";
   
   msg += message;

   if(report->linebuf)
      msg += "\nContext: " + std::string(report->linebuf);

   SetErrorReportConsoleAttribs();
   std::cout << msg << std::endl;
   ResetConsoleAttribs();
   
   if(echoFile)
   {
      echoFile << msg << std::endl;
      echoFile.flush();
   }
}

//
// New context creation callback which is assigned to the JSRuntime at startup.
// This will set the error reporter and JavaScript version in that context.
//
static JSBool ContextCallback(JSContext *cx, uintN contextOp)
{
   if(contextOp == JSCONTEXT_NEW)
   {
      JS_SetErrorReporter(cx, ErrorReport);
      JS_SetVersion(cx, JSVERSION_LATEST);
   }
   
   return JS_TRUE;
}

//
// JSEngine_RunExtensions
//
// The extensions.js file is expected to contain any configuration which should
// apply both to the global execution context and to any sandbox contexts that
// may be created.
//
bool JSEngine_RunExtensions(JSEvalContext *ecx)
{
   // Run the extensions script
   jsval rval;
   return JSEngine_EvaluateFileInContext(ecx, "extensions.js", &rval);
}

//
// JSEngine_RunAutoexec
//
// The autoexec.js file is expected to contain any configuration which should 
// apply to the global execution context.
//
bool JSEngine_RunAutoexec(JSEvalContext *ecx)
{
   // Run the autoexec script
   jsval rval;
   return JSEngine_EvaluateFileInContext(ecx, "autoexec.js", &rval);
}


//
// JSEngine_Init
//
// Initialize the SpiderMonkey JSAPI library and create a default global 
// execution context.
//
bool JSEngine_Init()
{
   try
   {
      // Create runtime
      if(!(runtime = JS_NewRuntime(RUNTIME_HEAP_SIZE)))
         return false;

      // Set context callback
      JS_SetContextCallback(runtime, ContextCallback);

      // Create default global execution context
      gContext = new JSEvalContext();

      // Create the JS global object and initialize it
      if(!gContext->setGlobal(&global_class))
         return false;

      // Run extensions.js for the global context
      JSEngine_RunExtensions(gContext);

      // Run autoexec.js for the global context
      JSEngine_RunAutoexec(gContext);
   }
   catch(const JSEngineError &)
   {
      return false;
   }

   return true;
}

//
// JSEngine_Shutdown
//
// Destroy the global execution context and shutdown SpiderMonkey.
//
void JSEngine_Shutdown()
{
   if(gContext)
   {
      delete gContext;
      gContext = nullptr;
   }

   if(runtime)
   {
      JS_DestroyRuntime(runtime);
      runtime = nullptr;
      JS_ShutDown();
   }
}

//=============================================================================
//
// CompiledScript Methods
//

//
// String compiling constructor
//
CompiledScript::CompiledScript(const char *filename, const char *script) : ob(nullptr)
{
   // TODO: Allow compilation/execution within specified contexts
   this->cx     = gContext->getContext(); 
   this->global = gContext->getGlobal();

   this->script = JS_CompileScript(cx, global, script, strlen(script), filename, 1);

   if(this->script)
   {
      this->ob = JS_NewScriptObject(cx, this->script);
      JS_AddRoot(cx, &ob);
   }
}

//
// File compiling constructor
//
CompiledScript::CompiledScript(const char *filename) : ob(nullptr)
{
   this->cx     = gContext->getContext();
   this->global = gContext->getGlobal();

   this->script = JS_CompileFile(cx, global, filename);

   if(this->script)
   {
      this->ob = JS_NewScriptObject(cx, script);
      JS_AddRoot(cx, &ob);
   }
}

CompiledScript::CompiledScript(JSContext *pcx, JSObject *pglobal, const char *filename, const char *pscript) : ob(nullptr)
{
   cx     = pcx;
   global = pglobal;
   script = JS_CompileScript(cx, global, pscript, strlen(pscript), filename, 1);
   if(script)
   {
      ob = JS_NewScriptObject(cx, script);
      JS_AddRoot(cx, &ob);
   }
}

CompiledScript::CompiledScript(JSEvalContext *ctx, const char *filename, const char *pscript)
{
   cx     = ctx->getContext();
   global = ctx->getGlobal();
   script = JS_CompileScript(cx, global, pscript, strlen(pscript), filename, 1);
   if(script)
   {
      ob = JS_NewScriptObject(cx, script);
      JS_AddRoot(cx, &ob);
   }
}

CompiledScript::CompiledScript(JSContext *pcx, JSObject *pglobal, const char *filename, JSString *jstr) : ob(nullptr)
{
   cx     = pcx;
   global = pglobal;
   script = JS_CompileUCScript(cx, global, JS_GetStringChars(jstr), JS_GetStringLength(jstr), filename, 1);
   if(script)
   {
      ob = JS_NewScriptObject(cx, script);
      JS_AddRoot(cx, &ob);
   }
}

CompiledScript::CompiledScript(JSEvalContext *ctx, const char *filename, JSString *jstr) : ob(nullptr)
{
   cx     = ctx->getContext();
   global = ctx->getGlobal();
   script = JS_CompileUCScript(cx, global, JS_GetStringChars(jstr), JS_GetStringLength(jstr), filename, 1);
   if(script)
   {
      ob = JS_NewScriptObject(cx, script);
      JS_AddRoot(cx, &ob);
   }
}

//
// Destructor
//
CompiledScript::~CompiledScript()
{
   if(cx && ob)
      JS_RemoveRoot(cx, &ob);

   cx     = nullptr;
   global = nullptr;
   ob     = nullptr;
   script = nullptr;
}

//
// CompiledScript::execute
//
// Just run a script.
//
bool CompiledScript::execute()
{
   if(ob && script)
   {
      jsval rval;
      return (JS_ExecuteScript(cx, global, script, &rval) == JS_TRUE);
   }

   return false;
}

//
// CompiledScript::executeWithResult
//
// Execute the script and translate its return value into a string.
//
bool CompiledScript::executeWithResult(std::string &str)
{
   if(ob && script)
   {
      jsval rval = JSVAL_VOID;
      
      if(JS_ExecuteScript(cx, global, script, &rval) == JS_TRUE)
      {
         try
         {
            AutoJSValueToStringRooted jstr(cx, rval);
            str = JS_GetStringBytes(jstr);
            return true;
         }
         catch(const JSEngineError &)
         {
         }
      }
   }

   return false;
}

//
// CompiledScript::executeWithResult
//
// Execute the script and translate its result into a signed integer.
//
bool CompiledScript::executeWithResult(int &i)
{
   if(ob && script)
   {
      jsval rval = JSVAL_VOID;

      if(JS_ExecuteScript(cx, global, script, &rval) == JS_TRUE)
      {
         int32 res = 0;
         if(JS_ValueToECMAInt32(cx, rval, &res))
         {
            i = static_cast<int>(res);
            return true;
         }
      }
   }

   return false;
}

//
// CompiledScript::executeWithResult
//
// Execute the script and translate its result into an unsigned integer.
//
bool CompiledScript::executeWithResult(unsigned int &ui)
{
   if(ob && script)
   {
      jsval rval = JSVAL_VOID;

      if(JS_ExecuteScript(cx, global, script, &rval) == JS_TRUE)
      {
         uint32 res = 0;
         if(JS_ValueToECMAUint32(cx, rval, &res))
         {
            ui = static_cast<unsigned int>(res);
            return true;
         }
      }
   }

   return false;
}

//
// CompiledScript::executeWithResult
//
// Execute the script and translate its result into a double.
//
bool CompiledScript::executeWithResult(double &d)
{
   if(ob && script)
   {
      jsval rval = JSVAL_VOID;

      if(JS_ExecuteScript(cx, global, script, &rval) == JS_TRUE)
      {
         jsdouble res = 0.0;
         if(JS_ValueToNumber(cx, rval, &res))
         {
            d = static_cast<double>(res);
            return true;
         }
      }
   }

   return false;
}

//
// CompiledScript::executeWithResult
//
// Execute the script and translate its result into a boolean.
//
bool CompiledScript::executeWithResult(bool &b)
{
   if(ob && script)
   {
      jsval rval = JSVAL_VOID;

      if(JS_ExecuteScript(cx, global, script, &rval) == JS_TRUE)
      {
         JSBool rb = JS_FALSE;
         if(JS_ValueToBoolean(cx, rval, &rb))
         {
            b = (rb == JS_TRUE);
            return true;
         }
      }
   }

   return false;
}

//=============================================================================
//
// Command-line Interpreter Interface
//

// Persistent state of the command line interpreter
struct cmdlinestate_t
{
   unsigned int lineno;    // current line number
   unsigned int startline; // starting line number
   std::string  buffer;    // input buffer

   cmdlinestate_t() : lineno(1), startline(1), buffer("") {}
};

static cmdlinestate_t cmdlinestate;

//
// JSEngine_AddInputLine
//
// Add a line of input to the accumulating console input buffer. Once a
// compilable unit is accumulated, or a JS error occurs, the code will
// be executed and then the buffer is flushed.
//
void JSEngine_AddInputLine(const std::string &inputLine)
{
   cmdlinestate.buffer += inputLine + "\n";
   ++cmdlinestate.lineno;

   JSContext *cx     = gContext->getContext();
   JSObject  *global = gContext->getGlobal();

   size_t      length = cmdlinestate.buffer.length();
   const char *script = cmdlinestate.buffer.c_str();

   if(JS_BufferIsCompilableUnit(cx, global, script, length))
   {
      jsval retval = JSVAL_VOID;

      if(JS_EvaluateScript(cx, global, script, length, "console", 
                           cmdlinestate.startline, &retval))
      {
         AutoNamedRoot root;
         std::string result = SafeGetStringBytes(cx, retval, root);
         if(result != "undefined")
         {
            std::cout << result << std::endl;
            if(echoFile)
            {
               echoFile << result << std::endl;
               echoFile.flush();
            }
         }
      }

      JS_ReportPendingException(cx);

      cmdlinestate.lineno = cmdlinestate.startline = 1;
      cmdlinestate.buffer.clear();
   }
}

//
// JSEngine_GetInputPrompt
//
// Return a suitable string for use as an input prompt on the console.
// This is context sensitive as to whether or not input is continuing from the
// previous line.
//
void JSEngine_GetInputPrompt(std::string &prompt)
{
   prompt = (cmdlinestate.lineno == cmdlinestate.startline) ? "js> " : "";
}

//=============================================================================
//
// Evaluation and Script Execution Functions
//

//
// JSEngine_EvaluateInContext
//
// Evaluate script code within a particular evaluation context. This may be the
// global default context or a sandbox.
//
bool JSEngine_EvaluateInContext(JSEvalContext *gctx, const char *filename, const char *script, jsval *rval)
{
   JSContext *cx     = gctx->getContext();
   JSObject  *global = gctx->getGlobal();

   return (JS_EvaluateScript(cx, global, script, strlen(script), filename, 1, rval) == JS_TRUE);
}

//
// JSEngine_EvaluateUCInContext
//
// Evaluate Unicode script code within a particular evaluation context. This may be the
// global default context or a sandbox.
//
bool JSEngine_EvaluateUCInContext(JSEvalContext *gctx, const char *filename, 
                                  const jschar *chars, uintN length, jsval *rval)
{
   JSContext *cx     = gctx->getContext();
   JSObject  *global = gctx->getGlobal();

   return (JS_EvaluateUCScript(cx, global, chars, length, filename, 1, rval) == JS_TRUE);
}


//
// JSEngine_EvaluateScript
//
// As above, but passing the default global context unconditionally.
//
bool JSEngine_EvaluateScript(const char *filename, const char *script, jsval *rval)
{
   return JSEngine_EvaluateInContext(gContext, filename, script, rval);
}

//
// JSEngine_EvaluateUCScript
//
// Evaluate a Unicode script in the global context.
//
bool JSEngine_EvaluateUCScript(const char *filename, const jschar *chars, 
                               uintN length, jsval *rval)
{
   return JSEngine_EvaluateUCInContext(gContext, filename, chars, length, rval);
}

//
// JSEngine_EvaluateFile
//
// Load a text file from disk and then evaluate it within the global context.
//
bool JSEngine_EvaluateFile(const char *filename, jsval *rval)
{
   char *text;
   
   if((text = LoadTextFile(filename)))
   {
      bool res = JSEngine_EvaluateScript(filename, text, rval);
      delete [] text;
      return res;
   }

   return false;
}

//
// JSEngine_EvaluateJSStringInContext
//
// Evaluate the contents of a JSString object. Ensure the string is rooted.
//
bool JSEngine_EvaluateJSStringInContext(JSEvalContext *ecx, JSString *jstr, jsval *rval)
{
   const char *str = JS_GetStringBytes(jstr);
   return str ? JSEngine_EvaluateInContext(ecx, "String", str, rval) : false;
}

//
// JSEngine_EvaluateJSString
//
// Evaluate the contents of a JSString object in the global context.
//
bool JSEngine_EvaluateJSString(JSString *jstr, jsval *rval)
{
   return JSEngine_EvaluateJSStringInContext(gContext, jstr, rval);
}

//
// JSEngine_EvaluateUCJSStringInContext
//
// Like above, but Unicode.
//
bool JSEngine_EvaluateUCJSStringInContext(JSEvalContext *ecx, JSString *jstr, jsval *rval)
{
   const jschar *jchars = JS_GetStringChars(jstr);
   size_t        jlen   = JS_GetStringLength(jstr);
   return jchars ? JSEngine_EvaluateUCInContext(ecx, "String", jchars, (uintN)jlen, rval) : false;
}

//
// JSEngine_EvaluateUCJSString
//
// Evaluate a Unicode JSString in the global context.
//
bool JSEngine_EvaluateUCJSString(JSString *jstr, jsval *rval)
{
   return JSEngine_EvaluateUCJSStringInContext(gContext, jstr, rval);
}

//
// JSEngine_EvaluateAugmentedFile
//
// Load a text file from disk, wrap it with parentheses, and then evaluate it 
// within the given context. Particularly useful for evaluating JSON files.
//
bool JSEngine_EvaluateAugmentedFile(JSEvalContext *ecx, const char *filename, jsval *rval)
{
   char *text;

   if((text = LoadTextFile(filename)))
   {
      std::string augFile = "(";
      augFile += text;
      augFile += ")";

      bool res = JSEngine_EvaluateInContext(ecx, filename, augFile.c_str(), rval);
      delete [] text;
      return res;
   }

   return false;
}

//
// JSEngine_EvaluateFileInContext
//
// As above, but accept an evaluation context in which to execute the file's code.
//
bool JSEngine_EvaluateFileInContext(JSEvalContext *gctx, const char *filename, jsval *rval)
{
   char *text;

   if((text = LoadTextFile(filename)))
   {
      bool res = JSEngine_EvaluateInContext(gctx, filename, text, rval);
      delete [] text;
      return res;
   }

   return false;
}

//=============================================================================
//
// Sandboxing and Secure Context Creation
//

//
// JSEngine_NewSandbox
//
// Create and return a new sandboxed evaluation context; the JSContext
// within has its own independent global object not subject to any changes
// that may have been made within the default global context - this makes
// it especially useful for evaluation of modules. If the sandbox is not to
// be persistent, wrap the return value using a std::unique_ptr so that the
// context will be released when your function returns.
//
// Absolutely *never* destroy the evaluation context and *then* try to 
// access or add a reference to any object that was created as a result of 
// execution within it - you'll instantly crash the program if you do, as 
// context destruction causes vehement garbage collection to occur.
//
// Make sure anything that you want to persist past the destruction of the
// sandbox has already been rooted to something related to the global context,
// be it an object property, return value, etc.
//
JSEvalContext *JSEngine_NewSandbox(JSObject *injectProperties, bool parented, bool withExtensions)
{  
   std::unique_ptr<JSEvalContext> nc(new JSEvalContext());
   nc->setGlobal(&sandbox_class, parented ? gContext->getGlobal() : nullptr);

   if(!parented && withExtensions)
      JSEngine_RunExtensions(nc.get());

   if(injectProperties)
   {
      JSContext *cx   = nc->getContext();
      JSObject  *glob = nc->getGlobal();
      AutoJSPropertyIterator itr(cx, injectProperties);

      if(!itr.valid())
      {
         return nc.release(); // oh well >_>
      }

      jsid cur_id;
      while(JS_NextProperty(cx, itr.getObject(), &cur_id) && cur_id != JSVAL_VOID)
      {
         size_t len = 0;
         jschar *propName = SafeIdToJSChars(cx, cur_id, len);
         if(!propName)
            continue;

         jsval propValue = JSVAL_VOID;
         if(JS_GetUCProperty(cx, injectProperties, propName, len, &propValue))
            JS_DefineUCProperty(cx, glob, propName, len, propValue, nullptr, nullptr, 0);
      }
   }

   return nc.release();
}

//
// JSEngine_NewRestrictedContext
//
// For evaluation of sensitive code, a global context can be constructed without
// access to any native objects supported by the embedding application.
//
JSEvalContext *JSEngine_NewRestrictedContext()
{
   auto nc = new JSEvalContext();
   nc->setGlobal(&restricted_class);
   return nc;
}

// EOF

