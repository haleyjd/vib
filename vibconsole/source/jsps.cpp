/*
   vibconsole
   PowerShell bindings
*/

#ifndef VIBC_NO_POWERSHELL

#include "jsengine2.h"
#include "jsnatives.h"

#include "PSProxyCLR.h"

class PrivatePowerShell : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   PSProxyCLRClass ps;

   PrivatePowerShell() : PrivateData(), ps()
   {
   }
};

//
// PowerShell_New - JS Constructor
//
// Construct a C# PowerShell, via a C++/CLI wrapper class, and wrap it in the new 
// JavaScript object's private data field. Total chain of interlanguage communication:
//  JavaScript <-> C++ <-> C++/CLI <-> C# <-> PowerShell Interpreter
//
static JSBool PowerShell_New(JSContext *cx, JSObject *obj, uintN argc,
                             jsval *argv, jsval *rval)
{
   ASSERT_IS_CONSTRUCTING(cx, "PowerShell");

   try
   {
      std::unique_ptr<PrivatePowerShell> ps(new PrivatePowerShell());
      ps->setToJSObjectAndRelease(cx, obj, ps);
   }
   catch(...)
   {
      throw JSEngineError("Cannot construct PowerShell, native failure");
   }

   *rval = JSVAL_VOID;
   return JS_TRUE;
}

static void PowerShell_Finalize(JSContext *cx, JSObject *obj)
{
   auto priv = PrivateData::GetFromJSObject<PrivatePowerShell>(cx, obj);

   if(priv)
   {
      delete priv;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

/**
 * JSClass for PowerShell
 */
static JSClass powerShellClass =
{
   "PowerShell",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   PowerShell_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivatePowerShell, powerShellClass)

static JSBool PowerShell_addArgument(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto ps = PrivateData::MustGetFromThis<PrivatePowerShell>(cx, vp);
   ASSERT_ARGC_GE(argc, 1, "PowerShell.addArgument");
   const char *arg = SafeGetStringBytes(cx, argv[0], &argv[0]);
   try
   {
      ps->ps.addArgument(arg);
   }
   catch(...)
   {
      throw JSEngineError("Native exception thrown in PowerShell.addArgument");
   }

   JS_SET_RVAL(cx, vp, JS_THIS(cx, vp));
   return JS_TRUE;
}

static JSBool PowerShell_addCommand(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto ps = PrivateData::MustGetFromThis<PrivatePowerShell>(cx, vp);
   ASSERT_ARGC_GE(argc, 1, "PowerShell.addCommand");
   const char *cmd = SafeGetStringBytes(cx, argv[0], &argv[0]);
   try
   {
      ps->ps.addCommand(cmd);
   }
   catch(...)
   {
      throw JSEngineError("Native exception thrown in PowerShell.addCommand");
   }
   JS_SET_RVAL(cx, vp, JS_THIS(cx, vp));
   return JS_TRUE;
}

static JSBool PowerShell_addParameter(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto ps = PrivateData::MustGetFromThis<PrivatePowerShell>(cx, vp);
   ASSERT_ARGC_GE(argc, 1, "PowerShell.addParameter");
   const char *param = SafeGetStringBytes(cx, argv[0], &argv[0]);
   try
   {
      ps->ps.addParameter(param);
   }
   catch(...)
   {
      throw JSEngineError("Native exception thrown in PowerShell.addParameter");
   }
   JS_SET_RVAL(cx, vp, JS_THIS(cx, vp));
   return JS_TRUE;
}

static JSBool PowerShell_addScript(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto ps = PrivateData::MustGetFromThis<PrivatePowerShell>(cx, vp);
   ASSERT_ARGC_GE(argc, 1, "PowerShell.addScript");
   const char *script = SafeGetStringBytes(cx, argv[0], &argv[0]);
   try
   {
      ps->ps.addScript(script);
   }
   catch(...)
   {
      throw JSEngineError("Native exception thrown in PowerShell.addScript");
   }
   JS_SET_RVAL(cx, vp, JS_THIS(cx, vp));
   return JS_TRUE;
}

static JSBool PowerShell_invoke(JSContext *cx, uintN argc, jsval *vp)
{
   auto ps = PrivateData::MustGetFromThis<PrivatePowerShell>(cx, vp);
   try
   {
      auto deleter = [&ps] (char *c) { if(c) ps->ps.freeResult(c); };
      std::unique_ptr<char, decltype(deleter)> res(ps->ps.invoke(), deleter);

      if(res)
      {
         JSString *jstr = JS_NewStringCopyZ(cx, res.get());
         JS_SET_RVAL(cx, vp, jstr ? STRING_TO_JSVAL(jstr) : JSVAL_NULL);
      }
      else
         JS_SET_RVAL(cx, vp, JSVAL_NULL);
   }
   catch(...)
   {
      throw JSEngineError("Native exception thrown in PowerShell.invoke");
   }

   return JS_TRUE;
}

/**
 * PowerShell JS Method Table
 */
static JSFunctionSpec powerShellFuncs[] =
{
   JSE_FN("addArgument",  PowerShell_addArgument,  1, 0, 0),
   JSE_FN("addCommand",   PowerShell_addCommand,   1, 0, 0),
   JSE_FN("addParameter", PowerShell_addParameter, 1, 0, 0),
   JSE_FN("addScript",    PowerShell_addScript,    1, 0, 0),
   JSE_FN("invoke",       PowerShell_invoke,       0, 0, 0),
   JS_FS_END
};

static NativeInitCode PowerShell_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &powerShellClass, 
                           JSEngineNativeWrapper<PowerShell_New>,
                           0, nullptr, powerShellFuncs, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native powerShellNative("PowerShell", PowerShell_Create);

#endif

// EOF

