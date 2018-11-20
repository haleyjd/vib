/*
  node.js compatible process object
*/

#ifdef _MSC_VER
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <string>
#include <exception>

#include "jsengine2.h"
#include "jsnatives.h"
#include "main.h"

static JSBool Process_abort(JSContext *cx, uintN argc, jsval *vp)
{
   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   abort();
   return JS_TRUE; // unreachable.
}

static JSBool Process_chdir(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   try
   {
      ASSERT_ARGC_GE(argc, 1, "chdir");
      const char *dirname = SafeGetStringBytes(cx, argv[0], &argv[0]);

      if(chdir(dirname))
         throw JSEngineError("Failed to change working directory");

      JS_SET_RVAL(cx, vp, JSVAL_VOID);
      return JS_TRUE;
   }
   catch(const JSEngineError &err)
   {
      return err.propagateToJS(cx);
   }
}

static std::string SafeGetCwd()
{
   std::string ret;
   const size_t chunkSize = 256;
   const size_t maxChunks = 10240;

   char stackBuffer[chunkSize];
   if(getcwd(stackBuffer, sizeof(stackBuffer)))
   {
      ret = stackBuffer;
      return ret;
   }
   if(errno != ERANGE)
      throw JSEngineError("Cannot determine the current path.");

   for(size_t chunks = 2; chunks < maxChunks; chunks++)
   {
      std::unique_ptr<char []> cwd(new char [chunkSize * chunks]);
      if(getcwd(cwd.get(), chunkSize * chunks))
      {
         ret = cwd.get();
         return ret;
      }
      if(errno != ERANGE)
         throw JSEngineError("Cannot determine the current path.");
   }

   throw JSEngineError("Cannot determine the current path, too long.");
}

static JSBool Process_cwd(JSContext *cx, uintN argc, jsval *vp)
{
   try
   {
      JSString *jstr  = AssertJSNewStringCopyZ(cx, SafeGetCwd().c_str());
      JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
      return JS_TRUE;
   }
   catch(const JSEngineError &err)
   {
      return err.propagateToJS(cx);
   }
}

static JSBool Process_exit(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv   = JS_ARGV(cx, vp);
   jsval  jsthis = JS_THIS(cx, vp);

   int32 code = 0;
   if(argc)
   {
      if(JS_ValueToECMAInt32(cx, argv[0], &code))
         ProcessReturnCode = int(code);
   }
   else if(JSVAL_IS_OBJECT(jsthis))
   {
      jsval prop;
      if(JS_GetProperty(cx, JSVAL_TO_OBJECT(jsthis), "exitCode", &prop))
      {
         if(JS_ValueToECMAInt32(cx, prop, &code))
            ProcessReturnCode = int(code);
      }
   }

   if(!MainLoopRunning)
       exit(ProcessReturnCode); // exit directly if not in main loop

    MainLoopRunning = false;

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return JS_TRUE;
}

// EOF

