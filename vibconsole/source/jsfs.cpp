/*
  node.js compatible File System module
*/

#ifdef _MSC_VER
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
#include <direct.h>
#include <io.h>
#define F_OK 0
#define W_OK 2
#define R_OK 4
#define S_ISDIR(x)  ((((x) & _S_IFMT)==_S_IFDIR)?1:0)
#define S_ISCHR(x)  ((((x) & _S_IFMT)==_S_IFCHR)?1:0)
#define S_ISFIFO(x) ((((x) & _S_IFMT)==_S_IFIFO)?1:0)
#define S_ISREG(x)  ((((x) & _S_IFMT)==_S_IFREG)?1:0)
#define S_IRUSR _S_IREAD
#define S_IWUSR _S_IWRITE
#define S_IXUSR _S_IEXEC
#define S_IRWXU (S_IRUSR|S_IWUSR|S_IXUSR)
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <sys/stat.h>

#include "jsengine2.h"
#include "jsnatives.h"

//
// Stat object
//

class PrivateStat : public PrivateData
{
   DECLARE_PRIVATE_DATA()

protected:
   struct stat data;

public:
   PrivateStat() : PrivateData(), data()
   {
   }

   ~PrivateStat()
   {
   }

   struct stat &getData() { return data; }
};

static JSBool FS_Stat_isFile(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivateStat>(cx, vp);
   if(!priv)
   {
      JS_ReportError(cx, "this not an instance of Stat");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, !S_ISDIR(priv->getData().st_mode) ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool FS_Stat_isDirectory(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivateStat>(cx, vp);
   if(!priv)
   {
      JS_ReportError(cx, "this not an instance of Stat");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, S_ISDIR(priv->getData().st_mode) ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool FS_Stat_isBlockDevice(JSContext *cx, uintN argc, jsval *vp)
{
   JS_SET_RVAL(cx, vp, JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool FS_Stat_isCharacterDevice(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivateStat>(cx, vp);
   if(!priv)
   {
      JS_ReportError(cx, "this not an instance of Stat");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, S_ISCHR(priv->getData().st_mode) ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool FS_Stat_isSymbolicLink(JSContext *cx, uintN argc, jsval *vp)
{
   JS_SET_RVAL(cx, vp, JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool FS_Stat_isFIFO(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivateStat>(cx, vp);
   if(!priv)
   {
      JS_ReportError(cx, "this not an instance of Stat");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, S_ISFIFO(priv->getData().st_mode) ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool FS_Stat_isSocket(JSContext *cx, uintN argc, jsval *vp)
{
   JS_SET_RVAL(cx, vp, JSVAL_FALSE);
   return JS_TRUE;
}

static JSFunctionSpec StatMethods[] =
{
   JS_FN("isFile",            FS_Stat_isFile,            0, 0, 0),
   JS_FN("isDirectory",       FS_Stat_isDirectory,       0, 0, 0),
   JS_FN("isBlockDevice",     FS_Stat_isBlockDevice,     0, 0, 0),
   JS_FN("isCharacterDevice", FS_Stat_isCharacterDevice, 0, 0, 0),
   JS_FN("isSymbolicLink",    FS_Stat_isSymbolicLink,    0, 0, 0),
   JS_FN("isFIFO",            FS_Stat_isFIFO,            0, 0, 0),
   JS_FN("isSocket",          FS_Stat_isSocket,          0, 0, 0),
   JS_FS_END
};

static JSBool FS_Stat_GetDev(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PrivateData::GetFromJSObject<PrivateStat>(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint   val = static_cast<jsint>(priv->getData().st_dev);
   *vp = INT_TO_JSVAL(val);
   return JS_TRUE;
}

static JSBool FS_Stat_GetIno(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PrivateData::GetFromJSObject<PrivateStat>(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint   val = static_cast<jsint>(priv->getData().st_ino);
   *vp = INT_TO_JSVAL(val);
   return JS_TRUE;
}

static JSBool FS_Stat_GetMode(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PrivateData::GetFromJSObject<PrivateStat>(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint   val = static_cast<jsint>(priv->getData().st_mode);
   *vp = INT_TO_JSVAL(val);
   return JS_TRUE;
}

static JSBool FS_Stat_GetNLink(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PrivateData::GetFromJSObject<PrivateStat>(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint   val = static_cast<jsint>(priv->getData().st_nlink);
   *vp = INT_TO_JSVAL(val);
   return JS_TRUE;
}

static JSBool FS_Stat_GetUID(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PrivateData::GetFromJSObject<PrivateStat>(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint   val = static_cast<jsint>(priv->getData().st_uid);
   *vp = INT_TO_JSVAL(val);
   return JS_TRUE;
}

static JSBool FS_Stat_GetGID(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PrivateData::GetFromJSObject<PrivateStat>(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint   val = static_cast<jsint>(priv->getData().st_gid);
   *vp = INT_TO_JSVAL(val);
   return JS_TRUE;
}

static JSBool FS_Stat_GetRDev(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PrivateData::GetFromJSObject<PrivateStat>(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint   val = static_cast<jsint>(priv->getData().st_rdev);
   *vp = INT_TO_JSVAL(val);
   return JS_TRUE;
}

static JSBool FS_Stat_GetSize(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PrivateData::GetFromJSObject<PrivateStat>(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsdouble *nd = JS_NewDouble(cx, priv->getData().st_size);
   *vp = DOUBLE_TO_JSVAL(*nd);
   return JS_TRUE;
}


static JSPropertySpec StatProps[] =
{
   {
      "dev", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED,
      FS_Stat_GetDev, nullptr 
   },

   {
      "ino", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED,
      FS_Stat_GetIno, nullptr
   },

   {
      "mode", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED,
      FS_Stat_GetMode, nullptr
   },

   {
      "nlink", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED,
      FS_Stat_GetNLink, nullptr
   },

   {
      "uid", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED,
      FS_Stat_GetUID, nullptr
   },

   {
      "gid", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED,
      FS_Stat_GetGID, nullptr
   },

   {
      "rdev", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED,
      FS_Stat_GetRDev, nullptr
   },

   {
      "size", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY|JSPROP_SHARED,
      FS_Stat_GetSize, nullptr
   },

   { NULL }
};

static JSClass stat_class =
{
   "Stat",
   JSCLASS_HAS_PRIVATE,
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

DEFINE_PRIVATE_DATA(PrivateStat, stat_class)

//
// Main FS routines
//

static JSBool FS_accessSync(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   int mode = F_OK;

   try
   {
      ASSERT_ARGC_GE(argc, 1, "accessSync");
      if(argc >= 2)
      {
         int32 i;
         JS_ValueToECMAInt32(cx, argv[1], &i);
         mode = (int)i;
      }

      const char *fn = SafeGetStringBytes(cx, argv[0], &argv[0]);

      if(access(fn, mode))
         throw JSEngineError("Cannot access file with specified mode");

      JS_SET_RVAL(cx, vp, JSVAL_VOID);
      return JS_TRUE;
   }
   catch(const JSEngineError &err)
   {
      return err.propagateToJS(cx);
   }
}

static JSBool FS_existsSync(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 1)
   {
      JS_ReportError(cx, "Insufficient arguments to existsSync");
      return JS_FALSE;
   }

   const char *fn = SafeGetStringBytes(cx, argv[0], &argv[0]);
   jsval ret = JSVAL_FALSE;
   if(!access(fn, F_OK))
      ret = JSVAL_TRUE;

   JS_SET_RVAL(cx, vp, ret);
   return JS_TRUE;
}

static JSBool FS_mkdirSync(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
#ifndef _MSC_VER
   mode_t mode = 0777;
#endif

   if(argc < 1)
   {
      JS_ReportError(cx, "Insufficient arguments to mkdirSync");
      return JS_FALSE;
   }
#ifndef _MSC_VER
   if(argc >= 2)
   {
      int32 i;
      JS_ValueToECMAInt32(cx, argv[1], &i);
      mode = (mode_t)i;
   }
#endif

   const char *dirname = SafeGetStringBytes(cx, argv[0], &argv[0]);

#ifdef _MSC_VER
   mkdir(dirname);
#else
   mkdir(dirname, mode);
#endif

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool FS_renameSync(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 2)
   {
      JS_ReportError(cx, "Insufficient arguments to renameSync");
      return JS_FALSE;
   }

   const char *oldfn = SafeGetStringBytes(cx, argv[0], &argv[0]);
   const char *newfn = SafeGetStringBytes(cx, argv[1], &argv[1]);

   rename(oldfn, newfn); // FIXME: throws if fails

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool FS_rmdirSync(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 1)
   {
      JS_ReportError(cx, "Insufficient arguments to rmdirSync");
      return JS_FALSE;
   }

   const char *dirname = SafeGetStringBytes(cx, argv[0], &argv[0]);
   rmdir(dirname); // FIXME: throws if fails

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool FS_statSync(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   std::unique_ptr<PrivateStat> ps(new PrivateStat());
   struct stat &st = ps.get()->getData();

   try
   {
      ASSERT_ARGC_GE(argc, 1, "unlinkSync");

      const char *fn = SafeGetStringBytes(cx, argv[0], &argv[0]);
      if(stat(fn, &st))
      {
         JSObject *newObj = AssertJSNewObject(cx, &stat_class, nullptr, nullptr);
         AutoNamedRoot anr(cx, newObj, "FS_statSync");

         AssertJSDefineFunctions(cx, newObj, StatMethods);
         AssertJSDefineProperties(cx, newObj, StatProps);

         ps->setToJSObjectAndRelease(cx, newObj, ps);
         JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newObj));
         return JS_TRUE;
      }
      else
         throw JSEngineError("Unable to stat");
   }
   catch(const JSEngineError &err)
   {
      return err.propagateToJS(cx);
   }
}

static JSBool FS_unlinkSync(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 1)
   {
      JS_ReportError(cx, "Insufficient arguments to unlinkSync");
      return JS_FALSE;
   }

   const char *fn = SafeGetStringBytes(cx, argv[0], &argv[0]);
   unlink(fn); // FIXME: throws if fails

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}


// EOF

