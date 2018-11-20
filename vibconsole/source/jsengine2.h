/** @file jsengine2.h
 *
 */

#ifndef JSENGINE2_H__
#define JSENGINE2_H__

#include <exception>
#include <map>
#include <string>
#include "myjsconfig.h"
#include "jsapi.h"

const char *SafeGetStringBytes(JSContext *cx, jsval value, jsval *root);
bool SafeInstanceOf(JSContext *cx, JSClass *jsClass, jsval val);
void AssertInstanceOf(JSContext *cx, JSClass *jsClass, jsval val);
bool JSObjectToStringMap(JSContext *cx, JSObject *obj, std::map<std::string, std::string> &strMap);
void AssertJSObjectToStringMap(JSContext *cx, JSObject *obj, std::map<std::string, std::string> &strMap);
bool JSObjectToStrIntMap(JSContext *cx, JSObject *obj, std::map<std::string, int> &strIntMap);
int  JSEngine_ValueToInteger(jsval *val);

//
// GetPrivate
//
// Retrieve the private data pointer from a particular JSObject
//
template<typename T> inline T *GetPrivate(JSContext *cx, JSObject *obj)
{
   return static_cast<T *>(JS_GetPrivate(cx, obj));
}

// 
// GetInstancePrivate
//
// Retrieve the private data pointer from a JSObject if and only if it is
// of the appropriate class.
//
template<typename T> inline T *GetInstancePrivate(JSContext *cx, JSObject *obj, JSClass *objClass)
{
   void *v = JS_GetInstancePrivate(cx, obj, objClass, nullptr);

   return v ? static_cast<T *>(v) : nullptr;
}

//
// GetThisPrivate
//
// Retrieve the private data pointer from the "this" object on which a native
// method was invoked, in particular, inside a JSFastNative.
//
template<typename T> inline T *GetThisPrivate(JSContext *cx, jsval *vp)
{
   return static_cast<T *>(JS_GetPrivate(cx, JS_THIS_OBJECT(cx, vp)));
}

//
// GetThisInstancePrivate
//
// Retrieve the private data pointer from the "this" object on which a native
// method was invoked, inside a JSFastNative, and if and only if it is of the
// appropriate class.
//
template<typename T> inline T *GetThisInstancePrivate(JSContext *cx, jsval *vp, JSClass *objClass, jsval *argv)
{
   void *v = JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, vp), objClass, argv);

   return v ? static_cast<T *>(v) : nullptr;
}

//
// JSEngineError
// 
// Exception type thrown by any JSEngine utilities which throw exceptions.
//
class JSEngineError : public std::runtime_error
{
protected:
   bool isMemoryError;

public:
   JSEngineError() : std::runtime_error(""), isMemoryError(false)
   {
   }
   JSEngineError(const char *msg, bool isMem = false) 
      : std::runtime_error(msg), isMemoryError(isMem)
   {
   }
   JSEngineError(const std::string &str, bool isMem = false) 
      : std::runtime_error(str), isMemoryError(isMem)
   {
   }
   JSEngineError(const JSEngineError &other) 
      : std::runtime_error(other), isMemoryError(other.isMemoryError)
   {
   }

   JSBool propagateToJS(JSContext *cx) const
   {
      if(!JS_IsExceptionPending(cx)) // already a pending exception? don't overwrite it
      {
         if(isMemoryError)
            JS_ReportOutOfMemory(cx);
         else
            JS_ReportError(cx, what());
      }
      return JS_FALSE;
   }
};

//
// Exception wrapper generation template
//
template<JSFastNative fn>
JSBool JSEngineWrapperFn(JSContext *cx, uintN argc, jsval *vp)
{
   try
   {
      return fn(cx, argc, vp);
   }
   catch(const JSEngineError &err)
   {
      return err.propagateToJS(cx);
   }
}

template<JSNative fn>
JSBool JSEngineNativeWrapper(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
   try
   {
      return fn(cx, obj, argc, argv, rval);
   }
   catch(const JSEngineError &err)
   {
      return err.propagateToJS(cx);
   }
}

template<JSPropertyOp fn>
JSBool JSEnginePropertyOpWrapper(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   try
   {
      return fn(cx, obj, idval, vp);
   }
   catch(const JSEngineError &err)
   {
      return err.propagateToJS(cx);
   }
}

//
// Native wrapper macros
//
#define JSE_WRAP(fn) JSEngineWrapperFn<fn>
#define JSE_FN(name, fastcall, minargs, nargs, flags) \
   JS_FN(name, JSE_WRAP(fastcall), minargs, nargs, flags)

static inline JSObject *AssertJSNewObject(JSContext *cx, JSClass *clasp, JSObject *proto, JSObject *parent)
{
   JSObject *ret;
   if(!(ret = JS_NewObject(cx, clasp, proto, parent)))
      throw JSEngineError("Out of memory", true);
   return ret;
}

static inline JSObject *AssertJSNewArrayObject(JSContext *cx, jsint length, jsval *vector)
{
   JSObject *ret;
   if(!(ret = JS_NewArrayObject(cx, length, vector)))
      throw JSEngineError("Out of memory", true);
   return ret;
}

static inline JSString *AssertJSNewStringCopyZ(JSContext *cx, const char *s)
{
   JSString *ret;
   if(!(ret = JS_NewStringCopyZ(cx, s)))
      throw JSEngineError("Out of memory", true);
   return ret;
}

static inline JSString *AssertJSNewStringCopyN(JSContext *cx, const char *s, size_t n)
{
   JSString *ret;
   if(!(ret = JS_NewStringCopyN(cx, s, n)))
      throw JSEngineError("out of memory", true);
   return ret;
}

static inline JSIdArray *AssertJSEnumerateAll(JSContext *cx, JSObject *obj)
{
   JSIdArray *ids;
   if(!(ids = JS_EnumerateAll(cx, obj)))
      throw JSEngineError("Cannot instantiate jsid array");
   return ids;
}

static inline void AssertJSIdToValue(JSContext *cx, jsid id, jsval *vp)
{
   if(!JS_IdToValue(cx, id, vp))
      throw JSEngineError("Cannot convert jsid to jsval");
}

static inline JSObject *AssertJSDefineObject(JSContext *cx, JSObject *obj, const char *name, JSClass *clasp, JSObject *proto, uintN attrs)
{
   JSObject *newObj;
   if(!(newObj = JS_DefineObject(cx, obj, name, clasp, proto, attrs)))
      throw JSEngineError("Object construction error");
   return newObj;
}

static inline void AssertJSDefineFunctions(JSContext *cx, JSObject *obj, JSFunctionSpec *fs)
{
   if(!JS_DefineFunctions(cx, obj, fs))
      throw JSEngineError("Cannot instantiate object methods");
}

static inline void AssertJSDefineProperty(JSContext *cx, JSObject *obj, const char *name, jsval value,
                                          JSPropertyOp getter, JSPropertyOp setter, uintN attrs)
{
   if(!JS_DefineProperty(cx, obj, name, value, getter, setter, attrs))
   {
      std::string err = std::string("Cannot define property ") + name;
      throw JSEngineError(err);
   }
}

static inline void AssertJSDefineUCProperty(JSContext *cx, JSObject *obj, const jschar *name, size_t namelen, jsval value,
                                            JSPropertyOp getter, JSPropertyOp setter, uintN attrs)
{
   if(!JS_DefineUCProperty(cx, obj, name, namelen, value, getter, setter, attrs))
      throw JSEngineError("Cannot define property");
}

static inline void AssertJSSetElement(JSContext *cx, JSObject *obj, jsint index, jsval *vp)
{
   if(!JS_SetElement(cx, obj, index, vp))
      throw JSEngineError("Cannot set element on array");
}

static inline void AssertJSDefineProperties(JSContext *cx, JSObject *obj, JSPropertySpec *ps)
{
   if(!JS_DefineProperties(cx, obj, ps))
      throw JSEngineError("Cannot instantiate object properties");
}

static inline JSObject *AssertJSNewScriptObject(JSContext *cx, JSScript *script)
{
   JSObject *obj;
   if(!(obj = JS_NewScriptObject(cx, script)))
      throw JSEngineError("Error creating script object");
   return obj;
}

static inline void AssertJSCallFunctionName(JSContext *cx, JSObject *obj, const char *name, uintN argc, jsval *argv, jsval *rval)
{
   if(!JS_CallFunctionName(cx, obj, name, argc, argv, rval))
   {
      std::string str = std::string("Could not invoke JS function ") + name;
      throw JSEngineError(str);
   }
}

#define ASSERT_IS_OBJECT(v, name)                    \
   if(!JSVAL_IS_OBJECT((v)))                         \
      throw JSEngineError(name " is not an Object")

#define ASSERT_OBJECT_IS_FUNCTION(cx, obj)             \
   if(!JS_ObjectIsFunction((cx), (obj)))               \
      throw JSEngineError("Object is not a function")

#define ASSERT_VALUE_IS_FUNCTION(cx, val)                                            \
   if(!JSVAL_IS_OBJECT((val)) || !JS_ObjectIsFunction((cx), JSVAL_TO_OBJECT((val)))) \
      throw JSEngineError("Value is not a function")

#define ASSERT_ARGC_GE(argc, limit, func)                      \
   if((argc) < (limit))                                        \
      throw JSEngineError("Insufficient arguments for " func)

#define ASSERT_IS_CONSTRUCTING(cx, func)                            \
   if(!JS_IsConstructing(cx))                                       \
      throw JSEngineError(func " must be called as a constructor")

//
// AutoContext
//
// This object keeps track of a JSContext and will destroy it when the
// object is destroyed.
//
class AutoContext
{
public:
   JSContext *ctx;

   AutoContext(JSRuntime *rt, size_t stackChunkSize)
   {
      if(!(ctx = JS_NewContext(rt, stackChunkSize)))
         throw JSEngineError("Context creation failed");
   }

   ~AutoContext()
   {
      if(ctx)
      {
         JS_DestroyContext(ctx);
         ctx = nullptr;
      }
   }
};

//
// AutoIdArray
//
class AutoIdArray
{
public:
   JSContext *ctx;
   JSIdArray *ids;

   AutoIdArray(JSContext *cx, JSIdArray *idArr) : ctx(cx), ids(idArr)
   {
   }
   ~AutoIdArray()
   {
      if(ids)
      {
         JS_DestroyIdArray(ctx, ids);
         ids = nullptr;
      }
   }
};

//
// AutoNamedRoot
//
// This object keeps track of a named JS GC root and will release it when
// the object is destroyed - useful for tying GC roots to C++ stack frames.
// Either use the full constructor, or call init() when you have an object
// to root. The AutoNamedRoot is not reusable; subsequent calls to init()
// will fail with a false return value.
//
class AutoNamedRoot
{
private:
   JSContext *cx;
   void      *root;
   AutoNamedRoot(const AutoNamedRoot &other); // Not copyable.

public:
   AutoNamedRoot() : cx(nullptr), root(nullptr) {}

   AutoNamedRoot(JSContext *pcx, void *ptr, const char *name)
      : cx(pcx), root(ptr)
   {        
      if(!JS_AddNamedRoot(cx, &root, name))
         throw JSEngineError("Failed to create named GC root");
   }

   ~AutoNamedRoot()
   {
      if(root)
      {
         JS_RemoveRoot(cx, &root);
         root = nullptr;
      }
   }

   // Call to late-initialize an AutoNamedRoot
   void init(JSContext *pcx, void *ptr, const char *name)
   {
      // Cannot root a null value; useful for rooting the return
      // value of a JSAPI call without having to separately check it
      // for a non-null return value first.
      if(ptr && !root)
      {
         cx   = pcx;
         root = ptr;
         if(!JS_AddNamedRoot(cx, &root, name))
            throw JSEngineError("Failed to create named GC root");
      }
   }
};

static inline JSString *AssertJSValueToStringRooted(JSContext *cx, jsval &val, AutoNamedRoot &anr)
{
   JSString *jstr;
   if(!(jstr = JS_ValueToString(cx, val)))
      throw JSEngineError("Cannot convert value to string");
   anr.init(cx, jstr, "AssertJSValueToStringRooted");
   return jstr;
}

class AutoJSValueToStringRooted
{
protected:
   AutoNamedRoot anr;
   JSString *jstr;

public:
   AutoJSValueToStringRooted() : anr(), jstr(nullptr)
   {
   }

   AutoJSValueToStringRooted(JSContext *cx, jsval &val) : anr(), jstr(nullptr)
   {
      jstr = AssertJSValueToStringRooted(cx, val, anr);
   }

   void init(JSContext *cx, jsval &val)
   {
      jstr = AssertJSValueToStringRooted(cx, val, anr);
   }

   operator JSString *() const 
   { 
      if(!jstr)
         throw JSEngineError("Invalid auto rooted string");
      return jstr; 
   }
};

class JSEvalContextPimpl;

// JS Evaluation Context
class JSEvalContext
{
private:
   JSEvalContextPimpl *pImpl;

public:
   JSEvalContext();
   ~JSEvalContext();

   bool setGlobal(JSClass *globalClass, JSObject *parent = nullptr);

   JSContext *getContext() const;
   JSObject  *getGlobal()  const;
};

typedef enum
{
   NOSUCHPROPERTY,  // Can't find that as a native
   RESOLUTIONERROR, // A JSAPI error occurred while resolving
   RESOLVED         // Successful resolution/init
} NativeInitCode;

typedef NativeInitCode (*NativeInitFn)(JSContext *, JSObject *);

//
// This class represents a JS native, be it a class to register,
// or a property or method of the context global. Instances are self-
// registering at construction and can provide any arbitrary function
// for initialization within a given context via NativeInitFn.
//
class Native
{
protected:
   // Static hash table
   enum { NUMCHAINS = 257 };
   static Native *chains[NUMCHAINS];

   // Instance variables

   Native       *next; // Next on hash chain
   NativeInitFn  init; // Init function, to instantiate within a context
   const char   *name; // Name, for lookup

   void add(); // Called when object is constructing

public:
   Native(const char *pName, NativeInitFn pInit)
      : name(pName), init(pInit)
   {
      // Add self to class's static hash of all instances
      add();
   }

   // Initialize a native by name within the given context and object.
   static NativeInitCode InitByName(const char *name,
      JSContext *cx, JSObject *obj);

   static NativeInitCode EnumerateAll(JSContext *cx, JSObject *obj);
};

//
// Derive all JSObject private data from this class, for type safety. If
// native methods are accessed via .call / .apply / .bind, they may not be
// invoked on the same type of object for which they were created.
//
class PrivateData 
{
protected:
   static JSClass *ObjClass; // JSClass that instances of this native use

public:
   PrivateData()
   {
   }

   virtual ~PrivateData()
   {
   }

   template<typename T> static T *GetFromJSObject(JSContext *cx, JSObject *obj)
   {
      return dynamic_cast<T *>(GetInstancePrivate<PrivateData>(cx, obj, T::ObjClass));
   }

   template<typename T> static T *MustGetFromJSObject(JSContext *cx, JSObject *obj)
   {
      T *ret;
      if(!(ret = dynamic_cast<T *>(GetInstancePrivate<PrivateData>(cx, obj, T::ObjClass))))
         throw JSEngineError("Cannot obtain object private data");
      return ret;
   }

   template<typename T> static T *GetFromThis(JSContext *cx, jsval *vp)
   {
      return dynamic_cast<T *>(GetThisInstancePrivate<PrivateData>(cx, vp, T::ObjClass, JS_ARGV(cx, vp)));
   }

   template<typename T> static T *MustGetFromThis(JSContext *cx, jsval *vp)
   {
      T *ret;
      if(!(ret = dynamic_cast<T *>(GetThisInstancePrivate<PrivateData>(cx, vp, T::ObjClass, JS_ARGV(cx, vp)))))
         throw JSEngineError("Cannot obtain this instance private data");
      return ret;
   }

   static JSClass *GetJSClass() { return ObjClass; }

   void setToJSObject(JSContext *cx, JSObject *obj)
   {
      if(!JS_SetPrivate(cx, obj, this))
         throw JSEngineError("Cannot assign private data to object");
   }

   template<typename T>
   void setToJSObjectAndRelease(JSContext *cx, JSObject *obj, std::unique_ptr<T> &ptr)
   {
      setToJSObject(cx, obj);
      ptr.release();
   }
};

// Use this in the class definition for every PrivateData descendant
#define DECLARE_PRIVATE_DATA()  \
   public:                      \
      static JSClass *ObjClass; \
   private:

// Use this in a single translation module, once for each PrivateData descendant
#define DEFINE_PRIVATE_DATA(className, jsClass) \
   JSClass *className :: ObjClass = &jsClass;

//=============================================================================
// 
// CompiledScript
//

class CompiledScript
{
protected:
   JSContext *cx;     // Execution context
   JSObject  *global; // Global object
   JSObject  *ob;     // Script object
   JSScript  *script; // Compiled script code

public:
   /**
    * Compile a string into JavaScript bytecode.
    * For code that is run repeatedly, it is far more efficient to compile the
    * script into bytecode once, and then hold onto this object for later 
    * execution.
    * @param filename Filename to use in error and warning messages.
    * @param script Non-null, null-terminated C string to use as a script.
    * @return A CompiledScript proxy, non-null, but not necessarily valid. If
    *    invalid, any attempt to execute the CompiledScript object will return
    *    false from its execute() methods. The wrapped script is GC-rooted
    *    until the CompiledScript object is destroyed.
    */
   CompiledScript(const char *filename, const char *script);

   /**
    * Compile a file into JavaScript bytecode.
    * For code that is run repeatedly, it is far more efficient to compile the
    * script into bytecode once, and then hold onto this object for later 
    * execution.
    * @param filename File to load and compile into bytecode.
    * @return A CompiledScript proxy, non-null, but not necessarily valid. If
    *    invalid, any attempt to execute the CompiledScript object will return
    *    false from its execute() methods. The wrapped script is GC-rooted
    *    until the CompiledScript object is destroyed.
    */
   CompiledScript(const char *filename);

   CompiledScript(JSContext *pcx, JSObject *pglobal, const char *filename, const char *pscript);
   CompiledScript(JSEvalContext *ctx, const char *filename, const char *pscript);
   CompiledScript(JSContext *pcx, JSObject *pglobal, const char *filename, JSString *jstr);
   CompiledScript(JSEvalContext *ctx, const char *filename, JSString *jstr);

   ~CompiledScript();

   bool valid() const { return !!script; }

   bool execute();
   bool executeWithResult(std::string &str);
   bool executeWithResult(int &i);
   bool executeWithResult(unsigned int &ui);
   bool executeWithResult(double &d);
   bool executeWithResult(bool &b);

   JSContext *getContext() const { return cx;     }
   JSObject  *getGlobal()  const { return global; }
   JSScript  *getScript()  const { return script; }
};

//=============================================================================
// 
// External Interface
//

bool JSEngine_Init();
void JSEngine_Shutdown();
void JSEngine_AddInputLine(const std::string &inputLine);
void JSEngine_GetInputPrompt(std::string &prompt);

bool JSEngine_EvaluateInContext(JSEvalContext *gctx, const char *filename, const char *script, jsval *rval);
bool JSEngine_EvaluateScript(const char *filename, const char *script, jsval *rval);
bool JSEngine_EvaluateFile(const char *filename, jsval *rval);
bool JSEngine_EvaluateUCJSStringInContext(JSEvalContext *ecx, JSString *jstr, jsval *rval);
bool JSEngine_EvaluateUCJSString(JSString *jstr, jsval *rval);
bool JSEngine_EvaluateAugmentedFile(JSEvalContext *ecx, const char *filename, jsval *rval);

bool JSEngine_EvaluateFileInContext(JSEvalContext *gctx, const char *filename, jsval *rval);
JSEvalContext *JSEngine_NewSandbox(JSObject *injectProperties = nullptr, bool parented = true, bool withExtensions = false);
JSEvalContext *JSEngine_NewRestrictedContext();

#endif

// EOF

