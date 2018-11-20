/*
   JS Basic Win32 Wrappings
*/

#ifndef VIBC_NO_WIN32

#include <map>
#include <string>
#include <Windows.h>
#include "jsengine2.h"
#include "jsnatives.h"

static void jschars_to_wstring(const jschar *chars, size_t len, std::wstring &wstr)
{
   wstr.resize(len);

   for(size_t i = 0; i < len; i++)
      wstr[i] = chars[i];
}

static void SafeGetStringChars(JSContext *cx, jsval value, jsval *root, std::wstring &wstr)
{
   wstr = L"";
   JSString *jstr = JS_ValueToString(cx, value);
   
   if(jstr)
   {
      *root = STRING_TO_JSVAL(jstr);
      jschars_to_wstring(JS_GetStringChars(jstr), JS_GetStringLength(jstr), wstr);
   }
}

//
// Throw up a Windows Message Box.
//
static JSBool JSWin_MessageBox(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 3)
   {
      JS_ReportError(cx, "Insufficient arguments to MessageBox");
      return JS_FALSE;
   }

   std::wstring text, caption;
   uint32 flags = 0;

   SafeGetStringChars(cx, argv[0], &argv[0], text);
   SafeGetStringChars(cx, argv[1], &argv[1], caption);
   JS_ValueToECMAUint32(cx, argv[2], &flags);

   int res = MessageBox(nullptr, text.c_str(), caption.c_str(), flags);
   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(res));
   return JS_TRUE;
}

//
// Create Process
//
static JSBool JSWin_Exec(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   DWORD  exitCode = 0;

   ASSERT_ARGC_GE(argc, 2, "exec");

   const char *appName = nullptr;
   const char *cmd     = nullptr;
   JSBool newConsole   = JS_FALSE;
   JSBool waitOnProc   = JS_FALSE;

   if(!JSVAL_IS_NULL(argv[0]) && !JSVAL_IS_VOID(argv[0]))
      appName = SafeGetStringBytes(cx, argv[0], &argv[0]);
   if(!JSVAL_IS_NULL(argv[1]) && !JSVAL_IS_VOID(argv[1]))
      cmd     = SafeGetStringBytes(cx, argv[1], &argv[1]);

   if(!appName && !cmd)
      throw JSEngineError("Must specify at least one of application name or command line");

   if(argc >= 3)
      JS_ValueToBoolean(cx, argv[2], &newConsole);
   if(argc >= 4)
      JS_ValueToBoolean(cx, argv[3], &waitOnProc);

   PROCESS_INFORMATION procInfo;
   STARTUPINFOA        startupInfo;

   memset(&procInfo,    0, sizeof(procInfo));
   memset(&startupInfo, 0, sizeof(startupInfo));

   startupInfo.cb = sizeof(startupInfo);

   BOOL result;
   char cmdline[MAX_PATH * 2];
   if(cmd)
   {
      strncpy(cmdline, cmd, sizeof(cmdline));
      result = CreateProcessA(appName, cmdline, nullptr, nullptr, FALSE, 
         newConsole ? CREATE_NEW_CONSOLE : NORMAL_PRIORITY_CLASS,
         nullptr, nullptr, &startupInfo, &procInfo);
   }
   else
   {
      result = CreateProcessA(appName, nullptr, nullptr, nullptr, FALSE,
         newConsole ? CREATE_NEW_CONSOLE : NORMAL_PRIORITY_CLASS,
         nullptr, nullptr, &startupInfo, &procInfo);
   }

   if(result)
   {
      if(waitOnProc)
      {
         WaitForSingleObject(procInfo.hProcess, INFINITE);
         GetExitCodeProcess(procInfo.hProcess, &exitCode);
      }
      else
         exitCode = 1;
      CloseHandle(procInfo.hProcess);
      CloseHandle(procInfo.hThread);
   }

   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(exitCode));
   return JS_TRUE;
}

//
// Shell out to cmd (convenience routine as compared to above)
//
JSBool JSWin_Shell(JSContext *cx, uintN argc, jsval *vp)
{
   PROCESS_INFORMATION procInfo;
   STARTUPINFOA        startupInfo;

   memset(&procInfo,    0, sizeof(procInfo));
   memset(&startupInfo, 0, sizeof(startupInfo));

   startupInfo.cb = sizeof(startupInfo);

   BOOL result;
   char cmdline[MAX_PATH * 2];
   strncpy(cmdline, "cmd.exe", sizeof(cmdline));
   result = CreateProcessA(nullptr, cmdline, nullptr, nullptr, FALSE, 
                           NORMAL_PRIORITY_CLASS, nullptr, nullptr, 
                           &startupInfo, &procInfo);   
   if(result)
   {
      WaitForSingleObject(procInfo.hProcess, INFINITE);
      CloseHandle(procInfo.hProcess);
      CloseHandle(procInfo.hThread);
   }

   JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL(result ? JS_TRUE : JS_FALSE));
   return JS_TRUE;
}

//=============================================================================
//
// JS Dialogs
//

static HMODULE LoadDialogModule(const char *moduleName)
{
   static std::map<std::string, HMODULE> modules;
   HMODULE module =  nullptr;
   
   // Load module if not already loaded
   auto itr = modules.find(moduleName);
   if(itr != modules.end())
      module = itr->second;
   else if((module = LoadLibraryA(moduleName)))
      modules[moduleName] = module;

   return module;
}

class PrivateDialog : public PrivateData
{
   DECLARE_PRIVATE_DATA()

protected:
   HWND           hwnd;
   JSContext     *cx;
   JSObject      *thisObj;
   jsval          func;
   AutoNamedRoot  funcRoot; // root for JSObject referring to callback function
   
public:
   PrivateDialog(JSContext *pcx, JSObject *pThisObj, jsval pFunc);
   ~PrivateDialog();

   bool   valid() const { return hwnd != nullptr; }
   void   create(HMODULE module, jsval *argv, int arg);
   void   destroy();
   JSBool invoke(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

PrivateDialog::PrivateDialog(JSContext *pcx, JSObject *pThisObj, jsval pFunc)
   : PrivateData(), hwnd(nullptr), 
     cx(pcx), thisObj(pThisObj), func(pFunc), funcRoot()
{
   funcRoot.init(cx, JSVAL_TO_OBJECT(func), "JSDialogFuncRoot");
}

PrivateDialog::~PrivateDialog()
{
   destroy();
}

//
// JSDialogProc
//
// Registered as dialog procedure for all JS dialogs.
//
static INT_PTR CALLBACK JSDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   auto pdlg = reinterpret_cast<PrivateDialog *>(GetWindowLongPtr(hwndDlg, GWLP_USERDATA));
   
   return (pdlg && pdlg->invoke(uMsg, wParam, lParam)) ? TRUE : FALSE;
}

void PrivateDialog::create(HMODULE module, jsval *argv, int arg)
{
   // Get resource number or name
   if(JSVAL_IS_STRING(argv[arg]))
   {
      const char *name = SafeGetStringBytes(cx, argv[arg], &argv[arg]);
      hwnd = CreateDialogA(module, name, nullptr, JSDialogProc);
   }
   else
   {
      int32 res = 0;
      JS_ValueToECMAInt32(cx, argv[arg], &res);
      hwnd = CreateDialogA(module, MAKEINTRESOURCEA(res), nullptr, JSDialogProc);
   }

   if(hwnd)
      SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG)this);
   else
      throw JSEngineError("Could not create dialog");
}

void PrivateDialog::destroy()
{
   if(hwnd)
      DestroyWindow(hwnd);
   hwnd = nullptr;
}

JSBool PrivateDialog::invoke(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   JSBool ret = JS_FALSE;
   jsval rval = JSVAL_VOID;
   jsval args[3];

   // FIXME/TODO: grossly insufficient for most messages; need a translation
   // routine.
   args[0] = INT_TO_JSVAL((int)uMsg);
   args[1] = INT_TO_JSVAL((int)wParam);
   args[2] = INT_TO_JSVAL((int)lParam);

   if(JS_CallFunctionValue(cx, thisObj, func, 3, args, &rval))
      JS_ValueToBoolean(cx, rval, &ret);
   else if(JS_IsExceptionPending(cx))
      JS_ReportPendingException(cx);

   return ret;
}

static void Dialog_Finalize(JSContext *cx, JSObject *obj)
{
   auto priv = PrivateData::GetFromJSObject<PrivateDialog>(cx, obj);

   if(priv)
   {
      delete priv;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

static JSClass dialogClass =
{
   "Dialog",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   Dialog_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivateDialog, dialogClass)

static JSBool Dialog_destroy(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivateDialog>(cx, vp);
   if(!priv)
      return JS_FALSE;

   priv->destroy();

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

static JSBool Dialog_valid(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivateDialog>(cx, vp);
   if(!priv)
      return JS_FALSE;

   JS_SET_RVAL(cx, vp, priv->valid() ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSFunctionSpec dialogFunctions[] =
{
   JSE_FN("destroy", Dialog_destroy, 0, 0, 0),
   JSE_FN("valid",   Dialog_valid,   0, 0, 0),
   JS_FS_END
};

static JSBool JSWin_CreateDialog(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
 
   ASSERT_ARGC_GE(argc, 3, "CreateDialog");

   // check callback is a function
   ASSERT_IS_OBJECT(argv[2], "Argument 2");
   ASSERT_OBJECT_IS_FUNCTION(cx, JSVAL_TO_OBJECT(argv[2]));

   // load module
   HMODULE mod = LoadDialogModule(SafeGetStringBytes(cx, argv[0], &argv[0]));
   if(!mod)
      throw JSEngineError("Unable to load module");

   // create new Dialog object
   JSObject *obj = AssertJSNewObject(cx, &dialogClass, nullptr, nullptr);
   AutoNamedRoot objRoot(cx, obj, "NewDialog");

   AssertJSDefineFunctions(cx, obj, dialogFunctions);

   // create new PrivateDialog
   std::unique_ptr<PrivateDialog> priv(new PrivateDialog(cx, obj, argv[2]));
   priv->create(mod, argv, 1);

   priv->setToJSObjectAndRelease(cx, obj, priv);
   JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj));
   return JS_TRUE;
}

//
// JSWin_PumpEvents
//
// If an event is waiting, translate and dispatch.
//
static JSBool JSWin_PumpEvents(JSContext *cx, uintN argc, jsval *vp)
{
   MSG msg;
   if(GetMessage(&msg, nullptr, 0, 0))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }

   JS_SET_RVAL(cx, vp, JSVAL_NULL);
   return JS_TRUE;
}

//
// End JS Dialogs
//
//=============================================================================

static JSClass win_class =
{
   "WinClass",
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

static JSFunctionSpec JSWinMethods[] =
{
   JSE_FN("CreateDialog", JSWin_CreateDialog, 3, 0, 0),
   JSE_FN("exec",         JSWin_Exec,         2, 0, 0),
   JSE_FN("MessageBox",   JSWin_MessageBox,   0, 0, 0),
   JSE_FN("PumpEvents",   JSWin_PumpEvents,   0, 0, 0),
   JSE_FN("shell",        JSWin_Shell,        0, 0, 0),
   JS_FS_END
};

// MB_ enum values for messageboxes
static EnumEntry mbEnumValues[] =
{
   { MB_ABORTRETRYIGNORE,  "ABORTRETRYIGNORE"  },
   { MB_CANCELTRYCONTINUE, "CANCELTRYCONTINUE" },
   { MB_HELP,              "HELP"              },
   { MB_OK,                "OK"                },
   { MB_OKCANCEL,          "CANCEL"            },
   { MB_RETRYCANCEL,       "RETRYCANCEL"       },
   { MB_YESNO,             "YESNO"             },
   { MB_YESNOCANCEL,       "YESNOCANCEL"       },
   { MB_ICONEXCLAMATION,   "ICONEXCLAMATION"   },
   { MB_ICONWARNING,       "ICONWARNING"       },
   { MB_ICONINFORMATION,   "ICONINFORMATION"   },
   { MB_ICONASTERISK,      "ICONASTERISK"      },
   { MB_ICONQUESTION,      "ICONQUESTION"      },
   { MB_ICONSTOP,          "ICONSTOP"          },
   { MB_ICONERROR,         "ICONERROR"         },
   { MB_ICONHAND,          "ICONHAND"          },
   ENUMEND()
};

// ID* message box return values
static EnumEntry idEnumValues[] =
{
   { IDABORT,    "ABORT"    },
   { IDCANCEL,   "CANCEL"   },
   { IDCONTINUE, "CONTINUE" },
   { IDIGNORE,   "IGNORE"   },
   { IDNO,       "NO"       },
   { IDOK,       "OK"       },
   { IDRETRY,    "RETRY"    },
   { IDTRYAGAIN, "TRYAGAIN" },
   { IDYES,      "YES"      },
   ENUMEND()
};

static EnumEntry wmEnumValues[] =
{
   // TODO: Moar
   { WM_CLOSE,   "CLOSE"   },
   { WM_COMMAND, "COMMAND" },
   { WM_CREATE,  "CREATE"  },
   { WM_DESTROY, "DESTROY" },
   ENUMEND()
};

static NativeInitCode Win_Create(JSContext *cx, JSObject *global)
{
   try
   {
      auto obj = AssertJSDefineObject(cx, global, "Win", &win_class, nullptr, JSPROP_PERMANENT);
      AssertJSDefineFunctions(cx, obj, JSWinMethods);
   
      auto mbObj = AssertJSDefineObject(cx, obj, "MB", &enumClass, nullptr, 0);
      AddEnumerationProperties(cx, mbObj, mbEnumValues);

      auto idObj = AssertJSDefineObject(cx, obj, "ID", &enumClass, nullptr, 0);
      AddEnumerationProperties(cx, idObj, idEnumValues);

      auto wmObj = AssertJSDefineObject(cx, obj, "WM", &enumClass, nullptr, 0);
      AddEnumerationProperties(cx, wmObj, wmEnumValues);
   }
   catch(const JSEngineError &)
   {
      return RESOLUTIONERROR;
   }

   return RESOLVED;
}

static Native winGlobalNative("Win", Win_Create);

//=============================================================================
//
// Shell Functions
//

//
// Initialize COM for the shell API if needed.
//
static void Shell_ComInit()
{
   static bool com_init = false;

   if(!com_init)
   {
      CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);
      com_init = true;
   }
}

//
// Execute shell command
//
static JSBool JSShell_Execute(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   // init com if needed
   Shell_ComInit();

   if(argc < 2)
   {
      JS_ReportError(cx, "Insufficient arguments to execute");
      return JS_FALSE;
   }

   // get verb (could be null, which means to use default action)
   bool useverb = false;
   std::wstring verb;
   if(!JSVAL_IS_NULL(argv[0]) && !JSVAL_IS_VOID(argv[0]))
   {
      useverb = true;
      SafeGetStringChars(cx, argv[0], &argv[0], verb);
   }

   // get file
   std::wstring file;
   SafeGetStringChars(cx, argv[1], &argv[1], file);

   // optional parameters
   bool useparams = false, usedir = false;
   std::wstring params; // command-line params for programs
   std::wstring dir;    // working directory
   int showcmd = SW_SHOWNORMAL;  // show flags

   if(argc >= 3)
   {
      useparams = true;
      SafeGetStringChars(cx, argv[2], &argv[2], params);
      if(argc >= 4)
      {
         usedir = true;
         SafeGetStringChars(cx, argv[3], &argv[3], dir);
         if(argc >= 5)
         {
            int32 tmp = 0;
            JS_ValueToECMAInt32(cx, argv[4], &tmp);
            showcmd = static_cast<int>(tmp);
         }
      }
   }

   int ret = (int)ShellExecute(GetConsoleWindow(), 
                               useverb ? verb.c_str() : nullptr,
                               file.c_str(), 
                               useparams ? params.c_str() : nullptr, 
                               usedir ? dir.c_str() : nullptr, 
                               showcmd);

   if(ret > 32)
   {
      JS_SET_RVAL(cx, vp, JSVAL_TRUE);
      return JS_TRUE;
   }
   else
   {
      const char *err = "Unknown error";
      switch(ret)
      {
      case ERROR_FILE_NOT_FOUND:
         err = "The specified file was not found.";
         break;
      case ERROR_PATH_NOT_FOUND:
         err = "The specified path was not found.";
         break;
      case ERROR_BAD_FORMAT:
         err = "The .exe file is invalid.";
         break;
      case SE_ERR_ACCESSDENIED:
         err = "Access denied.";
         break;
      case SE_ERR_ASSOCINCOMPLETE:
         err = "File name association incomplete or invalid.";
         break;
      case SE_ERR_DDEBUSY:
         err = "DDE busy.";
         break;
      case SE_ERR_DDEFAIL:
         err = "DDE transaction failed.";
         break;
      case SE_ERR_DDETIMEOUT:
         err = "DDE transaction timed out.";
         break;
      case SE_ERR_DLLNOTFOUND:
         err = "The specified DLL was not found.";
         break;
      case SE_ERR_NOASSOC:
         err = "No association for this file type.";
         break;
      case SE_ERR_OOM:
         err = "System out of memory.";
         break;
      case SE_ERR_SHARE:
         err = "Sharing violation.";
         break;
      }
      JS_ReportError(cx, err);
      return JS_FALSE;
   }
}

static JSClass shell_class =
{
   "ShellClass",
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

static JSFunctionSpec JSShellMethods[] =
{
   JSE_FN("execute", JSShell_Execute, 5, 0, 0),
   JS_FS_END
};

static NativeInitCode Shell_Create(JSContext *cx, JSObject *global)
{
   JSObject *obj;

   if(!(obj = JS_DefineObject(cx, global, "Shell", &shell_class, nullptr, JSPROP_PERMANENT)))
      return RESOLUTIONERROR;

   if(!JS_DefineFunctions(cx, obj, JSShellMethods))
      return RESOLUTIONERROR;

   return RESOLVED;
}

static Native shellGlobalNative("Shell", Shell_Create);

#if defined(VIBC_NO_SDL)

//
// Timer
//

static JSBool Timer_Delay(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   int32 ms = 1;

   ASSERT_ARGC_GE(argc, 1, "delay");
   JS_ValueToECMAInt32(cx, argv[0], &ms);

   Sleep(static_cast<DWORD>(ms));
   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Timer_GetTicks(JSContext *cx, uintN argc, jsval *vp)
{
   DWORD dwTime = timeGetTime();

   jsdouble *nd = JS_NewDouble(cx, dwTime); // FIXME/TODO: auto root
   JS_SET_RVAL(cx, vp, DOUBLE_TO_JSVAL(nd));
   return JS_TRUE;
}

static JSClass timerClass =
{
   "TimerClass",
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

static JSFunctionSpec timerJSMethods[] =
{
   JSE_FN("delay",    Timer_Delay,    1, 0, 0),
   JSE_FN("getTicks", Timer_GetTicks, 0, 0, 0),
   JS_FS_END
};

static NativeInitCode Timer_Create(JSContext *cx, JSObject *global)
{
   JSObject *obj;

   if(!(obj = JS_DefineObject(cx, global, "Timer", &timerClass, nullptr, JSPROP_PERMANENT)))
      return RESOLUTIONERROR;

   if(!JS_DefineFunctions(cx, obj, timerJSMethods))
      return RESOLUTIONERROR;

   return RESOLVED;
}

static Native timerGlobalNative("Timer", Timer_Create);
#endif

#endif // VIBC_NO_WIN32

// EOF

