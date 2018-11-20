/*
   JS Wrappers for Pdate / Ptime / Pstamp
*/

#include "jsengine2.h"
#include "util.h"
#include "jsnatives.h"

//=============================================================================
//
// Pdate / Ptime / Pstamp Wrappers
//
// While JS does have its own Date class, it is in several respects not as
// flexible as the util.cpp objects.
//

//
// PrivateData Classes
//

class PrivatePdate : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   Pdate date;
   
   PrivatePdate() : PrivateData(), date()
   {
   }
};

class PrivatePtime : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   Ptime time;

   PrivatePtime() : PrivateData(), time()
   {
   }
};

class PrivatePstamp : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   Pstamp stamp;

   PrivatePstamp() : PrivateData(), stamp()
   {
   }
};

static void Pdate_Finalize(JSContext *cx, JSObject *obj);
static void Ptime_Finalize(JSContext *cx, JSObject *obj);
static void Pstamp_Finalize(JSContext *cx, JSObject *obj);

//
// JavaScript Class Objects
//

static JSClass pDateClass =
{
   "Pdate",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   Pdate_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivatePdate, pDateClass)

static JSClass pTimeClass =
{
   "Ptime",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   Ptime_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivatePtime, pTimeClass)

static JSClass pStampClass =
{
   "Pstamp",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   Pstamp_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivatePstamp, pStampClass)

//
// Constructors
//

static JSBool Pdate_New(JSContext *cx, JSObject *obj, uintN argc,
                        jsval *argv, jsval *rval)
{
   ASSERT_IS_CONSTRUCTING(cx, "Pdate");

   std::unique_ptr<PrivatePdate> pd(new PrivatePdate());

   if(argc >= 1)
   {
      if(SafeInstanceOf(cx, &pDateClass, argv[0])) // copy construct
      {
         JSObject *argObj = JSVAL_TO_OBJECT(argv[0]);
         auto thatPriv = PrivateData::GetFromJSObject<PrivatePdate>(cx, argObj);
         if(thatPriv)
            pd->date = thatPriv->date;
      }
      else
      {
         const char *dateStr = SafeGetStringBytes(cx, argv[0], &argv[0]);
         pd->date.InDate(dateStr, global_mode);
      }
   }
   else
      pd->date = CurrentPdate();

   pd->setToJSObjectAndRelease(cx, obj, pd);
   *rval = JSVAL_VOID;
   return JS_TRUE;
}

static JSBool Ptime_New(JSContext *cx, JSObject *obj, uintN argc,
                        jsval *argv, jsval *rval)
{
   ASSERT_IS_CONSTRUCTING(cx, "Ptime");

   std::unique_ptr<PrivatePtime> pt(new PrivatePtime());

   if(argc >= 1)
   {
      if(SafeInstanceOf(cx, &pTimeClass, argv[0])) // copy construct
      {
         JSObject *argObj = JSVAL_TO_OBJECT(argv[0]);
         auto thatPriv = PrivateData::MustGetFromJSObject<PrivatePtime>(cx, argObj);
         pt->time = thatPriv->time;
      }
      else
      {
         const char *timeStr = SafeGetStringBytes(cx, argv[0], &argv[0]);
         pt->time.InTime(timeStr, global_mode);
      }
   }
   else
      pt->time = CurrentPtime();

   pt->setToJSObjectAndRelease(cx, obj, pt);
   *rval = JSVAL_VOID;
   return JS_TRUE;
}

static JSBool Pstamp_New(JSContext *cx, JSObject *obj, uintN argc,
                         jsval *argv, jsval *rval)
{
   ASSERT_IS_CONSTRUCTING(cx, "Pstamp");

   std::unique_ptr<PrivatePstamp> ps(new PrivatePstamp());

   if(argc >= 1)
   {
      if(SafeInstanceOf(cx, &pStampClass, argv[0])) // copy construct
      {
         JSObject *argObj = JSVAL_TO_OBJECT(argv[0]);
         auto thatPriv = PrivateData::GetFromJSObject<PrivatePstamp>(cx, argObj);
         if(thatPriv)
            ps->stamp = thatPriv->stamp;
      }
      else
      {
         int mode = global_mode;
         const char *timeStr = SafeGetStringBytes(cx, argv[0], &argv[0]);

         if(argc >= 2)
         {
            int32 tmp = 0;
            if(JS_ValueToECMAInt32(cx, argv[1], &tmp))
               mode = int(tmp);
         }
         ps->stamp.InStamp(timeStr, mode);
      }
   }
   else
      ps->stamp = CurrentPstamp();

   ps->setToJSObjectAndRelease(cx, obj, ps);
   *rval = JSVAL_VOID;
   return JS_TRUE;
}

//
// Finalizers
//

static void Pdate_Finalize(JSContext *cx, JSObject *obj)
{
   auto priv = PrivateData::GetFromJSObject<PrivatePdate>(cx, obj);

   if(priv)
   {
      delete priv;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

static void Ptime_Finalize(JSContext *cx, JSObject *obj)
{
   auto priv = PrivateData::GetFromJSObject<PrivatePtime>(cx, obj);

   if(priv)
   {
      delete priv;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

static void Pstamp_Finalize(JSContext *cx, JSObject *obj)
{
   auto priv = PrivateData::GetFromJSObject<PrivatePstamp>(cx, obj);

   if(priv)
   {
      delete priv;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

//
// Utilities
//

static Pdate *AllowDateOrStamp(JSContext *cx, JSObject *obj)
{
   if(JS_InstanceOf(cx, obj, &pDateClass, nullptr))
   {
      auto priv = PrivateData::GetFromJSObject<PrivatePdate>(cx, obj);
      if(priv)
         return &priv->date;
   }
   else if(JS_InstanceOf(cx, obj, &pStampClass, nullptr))
   {
      auto priv = PrivateData::GetFromJSObject<PrivatePstamp>(cx, obj);
      if(priv)
         return &priv->stamp;
   }

   return nullptr;
}

static Pdate *AllowDateOrStamp(JSContext *cx, jsval v)
{
   if(JSVAL_IS_OBJECT(v))
   {
      JSObject *obj = JSVAL_TO_OBJECT(v);
      return AllowDateOrStamp(cx, obj);
   }

   return nullptr;
}

static Ptime *AllowTimeOrStamp(JSContext *cx, JSObject *obj)
{
   if(JS_InstanceOf(cx, obj, &pTimeClass, nullptr))
   {
      auto priv = PrivateData::GetFromJSObject<PrivatePtime>(cx, obj);
      if(priv)
         return &priv->time;
   }
   else if(JS_InstanceOf(cx, obj, &pStampClass, nullptr))
   {
      auto priv = PrivateData::GetFromJSObject<PrivatePstamp>(cx, obj);
      if(priv)
         return &priv->stamp;
   }

   return nullptr;
}

static Ptime *AllowTimeOrStamp(JSContext *cx, jsval v)
{
   if(JSVAL_IS_OBJECT(v))
   {
      JSObject *obj = JSVAL_TO_OBJECT(v);
      return AllowTimeOrStamp(cx, obj);
   }

   return nullptr;
}

//
// Methods
//

static JSBool Shared_DayOfWeek(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   int dow = date->DayOfWeek();
   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(dow));
   return JS_TRUE;
}

// Pdate Methods
static JSBool Pdate_DayOfWeek(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_DayOfWeek(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_DayOfWeek(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_DayOfWeek(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_InDate(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   jsval *argv = JS_ARGV(cx, vp);
   
   if(argc == 2)
   {
      const char *datestr = SafeGetStringBytes(cx, argv[0], &argv[0]);
      int32 flags = 0;
      JS_ValueToECMAInt32(cx, argv[1], &flags);

      date->InDate(datestr, flags);
   }
   else if(argc == 1)
   {
      Pdate *argDate = AllowDateOrStamp(cx, argv[0]);
      if(argDate)
         date->InDate(*argDate);
      else
         ; // TODO: time_t ?
   }

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Pdate_InDate(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_InDate(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_InDate(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_InDate(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_IsValidDate(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   JS_SET_RVAL(cx, vp, date->IsValidDate() ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Pdate_IsValidDate(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_IsValidDate(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_IsValidDate(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_IsValidDate(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_DateMod(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc == 2)
   {
      int32 dist, metric;
      JS_ValueToECMAInt32(cx, argv[0], &dist);
      JS_ValueToECMAInt32(cx, argv[1], &metric);
      date->mod(dist, metric);
   }
   else if(argc == 1)
   {
      auto pd = AllowDateOrStamp(cx, argv[0]);
      if(pd)
         date->mod(*pd);
      else
      {
         JS_ReportError(cx, "Parameter 1 is not an instance of Pdate");
         return JS_FALSE;
      }
   }
   
   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Pdate_mod(JSContext *cx, uintN argc, jsval *vp)
{   
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);
 
   return priv ? Shared_DateMod(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_DateMod(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_DateMod(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_NameOfDay(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   JSString *jstr = AssertJSNewStringCopyZ(cx, date->NameOfDay().c_str());
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

static JSBool Pdate_NameOfDay(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_NameOfDay(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_NameOfDay(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_NameOfDay(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_NameOfMonth(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   JSString *jstr = AssertJSNewStringCopyZ(cx, date->NameOfMonth().c_str());
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

static JSBool Pdate_NameOfMonth(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_NameOfMonth(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_NameOfMonth(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_NameOfMonth(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_DateIncrement(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   (*date)++;
   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Pdate_Increment(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_DateIncrement(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_DateIncrement(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_DateIncrement(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_DateDecrement(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   (*date)--;
   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Pdate_Decrement(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_DateDecrement(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_DateDecrement(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_DateDecrement(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_DateIncrAssign(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "Pdate::increment");

   int32 rhs = 0;
   JS_ValueToECMAInt32(cx, argv[0], &rhs);
   (*date) += rhs;

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Pdate_IncrAssign(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_DateIncrAssign(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_DateIncrAssign(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_DateIncrAssign(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_DateDecrAssign(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "Pdate::decrAssign");
   int32 rhs = 0;
   JS_ValueToECMAInt32(cx, argv[0], &rhs);
   (*date) -= rhs;

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Pdate_DecrAssign(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_DateDecrAssign(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_DateDecrAssign(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_DateDecrAssign(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_DateLessThan(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   jsval *argv = JS_ARGV(cx, vp);
   Pdate *other;

   if(argc < 1 || !(other = AllowDateOrStamp(cx, argv[0])))
   {
      JS_ReportError(cx, "Too few or invalid arguments");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, *date < *other ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Pdate_LessThan(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_DateLessThan(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_DateLessThan(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_DateLessThan(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_DateLessThanOrEq(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   jsval *argv = JS_ARGV(cx, vp);
   Pdate *other;

   if(argc < 1 || !(other = AllowDateOrStamp(cx, argv[0])))
   {
      JS_ReportError(cx, "Too few or invalid arguments");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, *date <= *other ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Pdate_LessThanOrEq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_DateLessThanOrEq(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_DateLessThanOrEq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_DateLessThanOrEq(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_DateEq(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   jsval *argv = JS_ARGV(cx, vp);
   Pdate *other;

   if(argc < 1 || !(other = AllowDateOrStamp(cx, argv[0])))
   {
      JS_ReportError(cx, "Too few or invalid arguments");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, *date == *other ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Pdate_Eq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_DateEq(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_DateEq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_DateEq(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_DateGreaterThan(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   jsval *argv = JS_ARGV(cx, vp);
   Pdate *other;

   if(argc < 1 || !(other = AllowDateOrStamp(cx, argv[0])))
   {
      JS_ReportError(cx, "Too few or invalid arguments");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, *date > *other ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Pdate_GreaterThan(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_DateGreaterThan(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_DateGreaterThan(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_DateGreaterThan(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_DateGreaterThanOrEq(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   jsval *argv = JS_ARGV(cx, vp);
   Pdate *other;

   if(argc < 1 || !(other = AllowDateOrStamp(cx, argv[0])))
   {
      JS_ReportError(cx, "Too few or invalid arguments");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, *date >= *other ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Pdate_GreaterThanOrEq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_DateGreaterThanOrEq(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_DateGreaterThanOrEq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_DateGreaterThanOrEq(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_DateNotEq(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   jsval *argv = JS_ARGV(cx, vp);
   Pdate *other;

   if(argc < 1 || !(other = AllowDateOrStamp(cx, argv[0])))
   {
      JS_ReportError(cx, "Too few or invalid arguments");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, *date != *other ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Pdate_NotEq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_DateNotEq(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_DateNotEq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_DateNotEq(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_OutDate(JSContext *cx, uintN argc, jsval *vp, Pdate *date)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "OutDate");

   if(!date->IsValidDate())
      throw JSEngineError("Date is invalid!");

   int32 mode;
   JS_ValueToECMAInt32(cx, argv[0], &mode);

   std::string outstr = date->OutDate(mode);
   JSString *jstr = AssertJSNewStringCopyZ(cx, outstr.c_str()); // FIXME/TODO: auto string?
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

static JSBool Pdate_OutDate(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

   return priv ? Shared_OutDate(cx, argc, vp, &priv->date) : JS_FALSE;
}

static JSBool Pstamp_OutDate(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_OutDate(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Pdate_ToString(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::MustGetFromThis<PrivatePdate>(cx, vp);

   std::string dateStr;
   if(priv->date.IsValidDate())
      dateStr = priv->date.OutDate(global_mode);
   else
      dateStr = "invalid date";

   JSString *jstr = AssertJSNewStringCopyZ(cx, dateStr.c_str());
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

// Ptime Methods

static JSBool Shared_IsValidTime(JSContext *cx, uintN argc, jsval *vp, Ptime *time)
{
   JS_SET_RVAL(cx, vp, time->IsValidTime() ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Ptime_IsValidTime(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePtime>(cx, vp);

   return priv ? Shared_IsValidTime(cx, argc, vp, &priv->time) : JS_FALSE;
}

static JSBool Pstamp_IsValidTime(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_IsValidTime(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_InTime(JSContext *cx, uintN argc, jsval *vp, Ptime *time)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc == 2)
   {
      const char *timeStr = SafeGetStringBytes(cx, argv[0], &argv[0]);
      int32 flags = 0;
      JS_ValueToECMAInt32(cx, argv[1], &flags);

      time->InTime(timeStr, flags);
   }
   else if (argc == 1)
   {
      auto pt = AllowTimeOrStamp(cx, argv[0]);
      if(pt)
         time->InTime(*pt);
      else
         ; // TODO: time_t ?
   }

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Ptime_InTime(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePtime>(cx, vp);

   return priv ? Shared_InTime(cx, argc, vp, &priv->time) : JS_FALSE;
}

static JSBool Pstamp_InTime(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_InTime(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_OutTime(JSContext *cx, uintN argc, jsval *vp, Ptime *time)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "OutTime");

   if(!time->IsValidTime())
      throw JSEngineError("Time is invalid!");

   int32 mode;
   JS_ValueToECMAInt32(cx, argv[0], &mode);

   std::string outstr = time->OutTime(mode);
   JSString *jstr = AssertJSNewStringCopyZ(cx, outstr.c_str());
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

static JSBool Ptime_OutTime(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePtime>(cx, vp);

   return priv ? Shared_OutTime(cx, argc, vp, &priv->time) : JS_FALSE;
}

static JSBool Pstamp_OutTime(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_OutTime(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_TimeMod(JSContext *cx, uintN argc, jsval *vp, Ptime *time)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc == 2)
   {
      int32 dist, metric;
      JS_ValueToECMAInt32(cx, argv[0], &dist);
      JS_ValueToECMAInt32(cx, argv[1], &metric);
      time->mod(dist, metric);
   }
   else if(argc == 1)
   {
      auto pt = AllowTimeOrStamp(cx, argv[0]);
      if(pt)
         time->mod(*pt);
      else
      {
         JS_ReportError(cx, "Parameter 1 is not an instance of Ptime");
         return JS_FALSE;
      }
   }
   
   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Ptime_mod(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePtime>(cx, vp);

   return priv ? Shared_TimeMod(cx, argc, vp, &priv->time) : JS_FALSE;
}

static JSBool Pstamp_TimeMod(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_TimeMod(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_TimeLessThan(JSContext *cx, uintN argc, jsval *vp, Ptime *time)
{
   jsval *argv = JS_ARGV(cx, vp);
   Ptime *other;

   if(argc < 1 || !(other = AllowTimeOrStamp(cx, argv[0])))
   {
      JS_ReportError(cx, "Too few or invalid arguments");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, *time < *other ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Ptime_LessThan(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePtime>(cx, vp);

   return priv ? Shared_TimeLessThan(cx, argc, vp, &priv->time) : JS_FALSE;
}

static JSBool Pstamp_TimeLessThan(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_TimeLessThan(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_TimeLessThanOrEq(JSContext *cx, uintN argc, jsval *vp, Ptime *time)
{
   jsval *argv = JS_ARGV(cx, vp);
   Ptime *other;

   if(argc < 1 || !(other = AllowTimeOrStamp(cx, argv[0])))
   {
      JS_ReportError(cx, "Too few or invalid arguments");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, *time <= *other ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Ptime_LessThanOrEq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePtime>(cx, vp);

   return priv ? Shared_TimeLessThanOrEq(cx, argc, vp, &priv->time) : JS_FALSE;
}

static JSBool Pstamp_TimeLessThanOrEq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_TimeLessThanOrEq(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_TimeEq(JSContext *cx, uintN argc, jsval *vp, Ptime *time)
{
   jsval *argv = JS_ARGV(cx, vp);
   Ptime *other;

   if(argc < 1 || !(other = AllowTimeOrStamp(cx, argv[0])))
   {
      JS_ReportError(cx, "Too few or invalid arguments");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, *time == *other ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Ptime_Eq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePtime>(cx, vp);

   return priv ? Shared_TimeEq(cx, argc, vp, &priv->time) : JS_FALSE;
}

static JSBool Pstamp_TimeEq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_TimeEq(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_TimeGreaterThan(JSContext *cx, uintN argc, jsval *vp, Ptime *time)
{
   jsval *argv = JS_ARGV(cx, vp);
   Ptime *other;

   if(argc < 1 || !(other = AllowTimeOrStamp(cx, argv[0])))
   {
      JS_ReportError(cx, "Too few or invalid arguments");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, *time > *other ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Ptime_GreaterThan(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePtime>(cx, vp);

   return priv ? Shared_TimeGreaterThan(cx, argc, vp, &priv->time) : JS_FALSE;
}

static JSBool Pstamp_TimeGreaterThan(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_TimeGreaterThan(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_TimeGreaterThanOrEq(JSContext *cx, uintN argc, jsval *vp, Ptime *time)
{
   jsval *argv = JS_ARGV(cx, vp);
   Ptime *other;

   if(argc < 1 || !(other = AllowTimeOrStamp(cx, argv[0])))
   {
      JS_ReportError(cx, "Too few or invalid arguments");
      return JS_FALSE;
   }

      JS_SET_RVAL(cx, vp, *time >= *other ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Ptime_GreaterThanOrEq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePtime>(cx, vp);
   
   return priv ? Shared_TimeGreaterThanOrEq(cx, argc, vp, &priv->time) : JS_FALSE;
}

static JSBool Pstamp_TimeGreaterThanOrEq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);
   
   return priv ? Shared_TimeGreaterThanOrEq(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_TimeNotEq(JSContext *cx, uintN argc, jsval *vp, Ptime *time)
{
   jsval *argv = JS_ARGV(cx, vp);
   Ptime *other;

   if(argc < 1 || !(other = AllowTimeOrStamp(cx, argv[0])))
   {
      JS_ReportError(cx, "Too few or invalid arguments");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, *time != *other ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Ptime_NotEq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePtime>(cx, vp);

   return priv ? Shared_TimeNotEq(cx, argc, vp, &priv->time) : JS_FALSE;
}

static JSBool Pstamp_TimeNotEq(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_TimeNotEq(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_TimeIncr(JSContext *cx, uintN argc, jsval *vp, Ptime *time)
{
   (*time)++;
   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Ptime_Increment(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePtime>(cx, vp);

   return priv ? Shared_TimeIncr(cx, argc, vp, &priv->time) : JS_FALSE;
}

static JSBool Pstamp_TimeIncrement(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_TimeIncr(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Shared_TimeDecr(JSContext *cx, uintN argc, jsval *vp, Ptime *time)
{
   (*time)--;
   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Ptime_Decrement(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePtime>(cx, vp);

   return priv ? Shared_TimeDecr(cx, argc, vp, &priv->time) : JS_FALSE;
}

static JSBool Pstamp_TimeDecrement(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);

   return priv ? Shared_TimeDecr(cx, argc, vp, &priv->stamp) : JS_FALSE;
}

static JSBool Ptime_ToString(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::MustGetFromThis<PrivatePtime>(cx, vp);

   std::string dateStr;
   if(priv->time.IsValidTime())
      dateStr = priv->time.OutTime(global_mode);
   else
      dateStr = "invalid time";

   JSString *jstr = AssertJSNewStringCopyZ(cx, dateStr.c_str());
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

// Pstamp-exclusive Methods

static JSBool Pstamp_IsValidStamp(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::MustGetFromThis<PrivatePstamp>(cx, vp);
   JS_SET_RVAL(cx, vp, priv->stamp.IsValidStamp() ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Pstamp_InStamp(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::MustGetFromThis<PrivatePstamp>(cx, vp);
   jsval *argv = JS_ARGV(cx, vp);

   if(argc == 2)
   {
      const char *stampStr = SafeGetStringBytes(cx, argv[0], &argv[0]);
      int32 flags = 0;
      JS_ValueToECMAInt32(cx, argv[1], &flags);

      priv->stamp.InStamp(stampStr, flags);
   }

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Pstamp_OutStamp(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::MustGetFromThis<PrivatePstamp>(cx, vp);
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "OutStamp");

   if(!priv->stamp.IsValidStamp())
      throw JSEngineError("Stamp is invalid!");

   int32 mode;
   JS_ValueToECMAInt32(cx, argv[0], &mode);

   std::string outstr = priv->stamp.OutStamp(mode);
   JSString *jstr = AssertJSNewStringCopyZ(cx, outstr.c_str()); // FIXME/TODO: auto string?
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

static JSBool Pstamp_ToJSDateString(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::MustGetFromThis<PrivatePstamp>(cx, vp);
   jsval *argv = JS_ARGV(cx, vp);

   if(!priv->stamp.IsValidStamp())
      throw JSEngineError("Stamp is invalid!");

   JSString *jstr = AssertJSNewStringCopyZ(cx, priv->stamp.OutStamp(global_mode | withoutmillis).c_str());
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

static JSBool Pstamp_Mod(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::MustGetFromThis<PrivatePstamp>(cx, vp);
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "Pstamp::mod");
   ASSERT_IS_OBJECT(argv[0], "Argument 0");

   JSObject *param = JSVAL_TO_OBJECT(argv[0]);
   int result = 0;

   if(argc == 2)
   {
      int32 dist, metric;
      JS_ValueToECMAInt32(cx, argv[0], &dist);
      JS_ValueToECMAInt32(cx, argv[1], &metric);
      result = priv->stamp.mod(dist, metric);
   }
   else 
   {
      if(JS_InstanceOf(cx, param, &pStampClass, nullptr))
      {
         auto thatPriv = PrivateData::GetFromJSObject<PrivatePstamp>(cx, param);

         result = priv->stamp.mod(thatPriv->stamp);
      }
      else if(JS_InstanceOf(cx, param, &pDateClass, nullptr))
      {
         auto thatPriv = PrivateData::GetFromJSObject<PrivatePdate>(cx, param);

         result = priv->stamp.mod(thatPriv->date);
      }
      else if(JS_InstanceOf(cx, param, &pTimeClass, nullptr))
      {
         auto thatPriv = PrivateData::GetFromJSObject<PrivatePtime>(cx, param);

         result = priv->stamp.mod(thatPriv->time);
      }
      else
         throw JSEngineError("Pstamp::mod argument must be a Pstamp, Pdate, or Ptime instance");
   }

   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(result));
   return JS_TRUE;
}

static JSBool Pstamp_ToString(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::MustGetFromThis<PrivatePstamp>(cx, vp);

   std::string ts;
   if(priv->stamp.IsValidStamp())
      ts = priv->stamp.OutStamp(global_mode);
   else
      ts = "invalid timestamp";

   JSString *jstr = AssertJSNewStringCopyZ(cx, ts.c_str());
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

// Cross-walk to JS Date

static JSBool DateToPtype(JSContext *cx, uintN argc, jsval *vp)
{
   JSObject *obj  = JS_THIS_OBJECT(cx, vp);
   jsval    *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "DateToPtype");
   ASSERT_IS_OBJECT(argv[0], "Argument 0");

   JSObject *dateArg = JSVAL_TO_OBJECT(argv[0]);

   jsval month = JSVAL_VOID, day = JSVAL_VOID, year = JSVAL_VOID, hour = JSVAL_VOID, 
      minute = JSVAL_VOID, second = JSVAL_VOID, millisecond = JSVAL_VOID;

   AssertJSCallFunctionName(cx, dateArg, "getMonth",        0, nullptr, &month);
   AssertJSCallFunctionName(cx, dateArg, "getDate",         0, nullptr, &day);
   AssertJSCallFunctionName(cx, dateArg, "getFullYear",     0, nullptr, &year);
   AssertJSCallFunctionName(cx, dateArg, "getHours",        0, nullptr, &hour);
   AssertJSCallFunctionName(cx, dateArg, "getMinutes",      0, nullptr, &minute);
   AssertJSCallFunctionName(cx, dateArg, "getSeconds",      0, nullptr, &second);
   AssertJSCallFunctionName(cx, dateArg, "getMilliseconds", 0, nullptr, &millisecond);

   int32 monthInt, dayInt, yearInt, hourInt, minuteInt, secondInt, millisecondInt;
   JS_ValueToECMAInt32(cx, month,       &monthInt);
   JS_ValueToECMAInt32(cx, day,         &dayInt);
   JS_ValueToECMAInt32(cx, year,        &yearInt);
   JS_ValueToECMAInt32(cx, hour,        &hourInt);
   JS_ValueToECMAInt32(cx, minute,      &minuteInt);
   JS_ValueToECMAInt32(cx, second,      &secondInt);
   JS_ValueToECMAInt32(cx, millisecond, &millisecondInt);

   if(JS_InstanceOf(cx, obj, &pDateClass, nullptr))
   {
      auto priv = PrivateData::GetFromThis<PrivatePdate>(cx, vp);

      if(priv)
      {
         priv->date.month = monthInt + 1;
         priv->date.day   = dayInt;
         priv->date.year  = yearInt;
      }
   }
   else if(JS_InstanceOf(cx, obj, &pTimeClass, nullptr))
   {
      auto priv = PrivateData::GetFromThis<PrivatePtime>(cx, vp);

      if(priv)
      {
         priv->time.hour        = hourInt;
         priv->time.minute      = minuteInt;
         priv->time.second      = secondInt;
         priv->time.millisecond = millisecondInt;
      }
   }
   else if(JS_InstanceOf(cx, obj, &pStampClass, nullptr))
   {
      auto priv = PrivateData::GetFromThis<PrivatePstamp>(cx, vp);
      if(priv)
      {
         priv->stamp.month       = monthInt + 1;
         priv->stamp.day         = dayInt;
         priv->stamp.year        = yearInt;
         priv->stamp.hour        = hourInt;
         priv->stamp.minute      = minuteInt;
         priv->stamp.second      = secondInt;
         priv->stamp.millisecond = millisecondInt;
      }
   }
   else
      throw JSEngineError("Not a Pstamp, Pdate, or Ptime instance");

   JS_SET_RVAL(cx, vp, JS_THIS(cx, vp));
   return JS_TRUE;
}

// GetTimeSince functions

static JSBool Pdate_GetTimeSince(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::MustGetFromThis<PrivatePdate>(cx, vp);
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "GetTimeSince");
   AssertInstanceOf(cx, &pDateClass, argv[0]);

   JSObject *param = JSVAL_TO_OBJECT(argv[0]);
   auto thatPriv = PrivateData::MustGetFromJSObject<PrivatePdate>(cx, param);

   JSString *jstr = AssertJSNewStringCopyZ(cx, GetTimeSince(priv->date, thatPriv->date).c_str());
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

static JSBool Ptime_GetTimeSince(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::MustGetFromThis<PrivatePtime>(cx, vp);
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "GetTimeSince");
   AssertInstanceOf(cx, &pTimeClass, argv[0]);

   JSObject *param = JSVAL_TO_OBJECT(argv[0]);
   auto thatPriv = PrivateData::MustGetFromJSObject<PrivatePtime>(cx, param);

   JSString *jstr = AssertJSNewStringCopyZ(cx, GetTimeSince(priv->time, thatPriv->time).c_str());
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

static JSBool Pstamp_GetTimeSince(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::MustGetFromThis<PrivatePstamp>(cx, vp);
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "GetTimeSince");
   AssertInstanceOf(cx, &pStampClass, argv[0]);

   JSObject *param = JSVAL_TO_OBJECT(argv[0]);
   auto thatPriv = PrivateData::MustGetFromJSObject<PrivatePstamp>(cx, param);

   JSString *jstr = AssertJSNewStringCopyZ(cx, GetTimeSince(priv->stamp, thatPriv->stamp).c_str());
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

// CalcAge

static JSBool Pdate_CalcAge(JSContext *cx, uintN argc, jsval *vp)
{
   auto priv = PrivateData::MustGetFromThis<PrivatePdate>(cx, vp);
   jsval *argv = JS_ARGV(cx, vp);
   int age;

   if(!argc)
      age = CalcAge(priv->date);
   else
   {
      AssertInstanceOf(cx, &pDateClass, argv[0]);
      JSObject *param = JSVAL_TO_OBJECT(argv[0]);
      auto thatPriv = PrivateData::MustGetFromJSObject<PrivatePdate>(cx, param);
      age = CalcAge(priv->date, thatPriv->date);
   }

   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(age));
   return JS_TRUE;
}

// Method Tables

static JSFunctionSpec PdateMethods[] =
{
   JSE_FN("calcAge",         Pdate_CalcAge,         0, 0, 0),
   JSE_FN("dayOfWeek",       Pdate_DayOfWeek,       0, 0, 0),
   JSE_FN("inDate",          Pdate_InDate,          1, 0, 0),
   JSE_FN("isValidDate",     Pdate_IsValidDate,     0, 0, 0),
   JSE_FN("mod",             Pdate_mod,             1, 0, 0),
   JSE_FN("nameOfDay",       Pdate_NameOfDay,       0, 0, 0),
   JSE_FN("nameOfMonth",     Pdate_NameOfMonth,     0, 0, 0),
   JSE_FN("increment",       Pdate_Increment,       0, 0, 0),
   JSE_FN("decrement",       Pdate_Decrement,       0, 0, 0),
   JSE_FN("addAssign",       Pdate_IncrAssign,      1, 0, 0),
   JSE_FN("subAssign",       Pdate_DecrAssign,      1, 0, 0),
   JSE_FN("lessThan",        Pdate_LessThan,        1, 0, 0),
   JSE_FN("lessOrEqual",     Pdate_LessThanOrEq,    1, 0, 0),
   JSE_FN("getTimeSince",    Pdate_GetTimeSince,    1, 0, 0),
   JSE_FN("greaterThan",     Pdate_GreaterThan,     1, 0, 0),
   JSE_FN("greaterOrEqual",  Pdate_GreaterThanOrEq, 1, 0, 0),
   JSE_FN("equalTo",         Pdate_Eq,              1, 0, 0),
   JSE_FN("notEqualTo",      Pdate_NotEq,           1, 0, 0),
   JSE_FN("outDate",         Pdate_OutDate,         1, 0, 0),
   JSE_FN("toString",        Pdate_ToString,        0, 0, 0),
   JSE_FN("fromDate",        DateToPtype,           1, 0, 0),
   JS_FS_END
};

static JSFunctionSpec PtimeMethods[] =
{
   JSE_FN("inTime",          Ptime_InTime,          1, 0, 0),
   JSE_FN("isValidTime",     Ptime_IsValidTime,     0, 0, 0),
   JSE_FN("mod",             Ptime_mod,             1, 0, 0),
   JSE_FN("increment",       Ptime_Increment,       0, 0, 0),
   JSE_FN("decrement",       Ptime_Decrement,       0, 0, 0),
   JSE_FN("lessThan",        Ptime_LessThan,        1, 0, 0),
   JSE_FN("lessOrEqual",     Ptime_LessThanOrEq,    1, 0, 0),
   JSE_FN("getTimeSince",    Ptime_GetTimeSince,    1, 0, 0),
   JSE_FN("greaterThan",     Ptime_GreaterThan,     1, 0, 0),
   JSE_FN("greaterOrEqual",  Ptime_GreaterThanOrEq, 1, 0, 0),
   JSE_FN("equalTo",         Ptime_Eq,              1, 0, 0),
   JSE_FN("notEqualTo",      Ptime_NotEq,           1, 0, 0),
   JSE_FN("outTime",         Ptime_OutTime,         1, 0, 0),
   JSE_FN("toString",        Ptime_ToString,        0, 0, 0),
   JSE_FN("fromDate",        DateToPtype,           1, 0, 0),
   JS_FS_END
};

static JSFunctionSpec PstampMethods[] =
{
   // Pdate-Inherited Methods
   JSE_FN("dayOfWeek",          Pstamp_DayOfWeek,           0, 0, 0),
   JSE_FN("inDate",             Pstamp_InDate,              1, 0, 0),
   JSE_FN("isValidDate",        Pstamp_IsValidDate,         0, 0, 0),
   JSE_FN("dateMod",            Pstamp_DateMod,             1, 0, 0),
   JSE_FN("nameOfDay",          Pstamp_NameOfDay,           0, 0, 0),
   JSE_FN("nameOfMonth",        Pstamp_NameOfMonth,         0, 0, 0),
   JSE_FN("dateIncrement",      Pstamp_DateIncrement,       0, 0, 0),
   JSE_FN("dateDecrement",      Pstamp_DateDecrement,       0, 0, 0),
   JSE_FN("dateAddAssign",      Pstamp_DateIncrAssign,      1, 0, 0),
   JSE_FN("dateSubAssign",      Pstamp_DateDecrAssign,      1, 0, 0),
   JSE_FN("dateLessThan",       Pstamp_DateLessThan,        1, 0, 0),
   JSE_FN("dateLessOrEqual",    Pstamp_DateLessThanOrEq,    1, 0, 0),
   JSE_FN("dateGreaterThan",    Pstamp_DateGreaterThan,     1, 0, 0),
   JSE_FN("dateGreaterOrEqual", Pstamp_DateGreaterThanOrEq, 1, 0, 0),
   JSE_FN("dateEqualTo",        Pstamp_DateEq,              1, 0, 0),
   JSE_FN("dateNotEqualTo",     Pstamp_DateNotEq,           1, 0, 0),
   JSE_FN("outDate",            Pstamp_OutDate,             1, 0, 0),

   // Ptime-Inherited Methods
   JSE_FN("inTime",             Pstamp_InTime,              1, 0, 0),
   JSE_FN("isValidTime",        Pstamp_IsValidTime,         0, 0, 0),
   JSE_FN("timeMod",            Pstamp_TimeMod,             1, 0, 0),
   JSE_FN("timeIncrement",      Pstamp_TimeIncrement,       0, 0, 0),
   JSE_FN("timeDecrement",      Pstamp_TimeDecrement,       0, 0, 0),
   JSE_FN("timeLessThan",       Pstamp_TimeLessThan,        1, 0, 0),
   JSE_FN("timeLessOrEqual",    Pstamp_TimeLessThanOrEq,    1, 0, 0),
   JSE_FN("timeGreaterThan",    Pstamp_TimeGreaterThan,     1, 0, 0),
   JSE_FN("timeGreaterOrEqual", Pstamp_TimeGreaterThanOrEq, 1, 0, 0),
   JSE_FN("timeEqualTo",        Pstamp_TimeEq,              1, 0, 0),
   JSE_FN("timeNotEqualTo",     Pstamp_TimeNotEq,           1, 0, 0),
   JSE_FN("outTime",            Pstamp_OutTime,             1, 0, 0),

   // Original Pstamp methods
   JSE_FN("isValidStamp",       Pstamp_IsValidStamp,        0, 0, 0),
   JSE_FN("inStamp",            Pstamp_InStamp,             2, 0, 0),
   JSE_FN("getTimeSince",       Pstamp_GetTimeSince,        1, 0, 0),
   JSE_FN("outStamp",           Pstamp_OutStamp,            1, 0, 0),
   JSE_FN("mod",                Pstamp_Mod,                 1, 0, 0),
   JSE_FN("toString",           Pstamp_ToString,            0, 0, 0),
   JSE_FN("fromDate",           DateToPtype,                1, 0, 0),
   JSE_FN("toJSDateString",     Pstamp_ToJSDateString,      0, 0, 0),

   JS_FS_END
};

//
// Properties
//

// Pdate

static Pdate *PdateFromPrivate(JSContext *cx, JSObject *obj)
{
   if(JS_InstanceOf(cx, obj, &pStampClass, nullptr))
      return &(PrivateData::GetFromJSObject<PrivatePstamp>(cx, obj)->stamp);

   if(JS_InstanceOf(cx, obj, &pDateClass, nullptr))
      return &(PrivateData::GetFromJSObject<PrivatePdate>(cx, obj)->date);

   JS_ReportError(cx, "Not a Pdate");
   return nullptr;
}

static Ptime *PtimeFromPrivate(JSContext *cx, JSObject *obj)
{
   if(JS_InstanceOf(cx, obj, &pStampClass, nullptr))
      return &(PrivateData::GetFromJSObject<PrivatePstamp>(cx, obj)->stamp);

   if(JS_InstanceOf(cx, obj, &pTimeClass, nullptr))
      return &(PrivateData::GetFromJSObject<PrivatePtime>(cx, obj)->time);

   JS_ReportError(cx, "Not a Ptime");
   return nullptr;
}

static JSBool Pdate_GetDay(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PdateFromPrivate(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint jsDay = static_cast<jsint>(priv->day);
   *vp = INT_TO_JSVAL(jsDay);
   return JS_TRUE;
}

static JSBool Pdate_SetDay(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PdateFromPrivate(cx, obj);
   if(!priv)
      return JS_FALSE;

   int32 dayVal = priv->day;
   if(JS_ValueToECMAInt32(cx, *vp, &dayVal))
   {
      priv->day = static_cast<int>(dayVal);
      return JS_TRUE;
   }
   else
   {
      JS_ReportError(cx, "Could not set day from value");
      return JS_FALSE;
   }
}

static JSBool Pdate_GetMonth(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PdateFromPrivate(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint jsMonth = static_cast<jsint>(priv->month);
   *vp = INT_TO_JSVAL(jsMonth);
   return JS_TRUE;
}

static JSBool Pdate_SetMonth(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PdateFromPrivate(cx, obj);
   if(!priv)
      return JS_FALSE;

   int32 monthVal = priv->month;
   if(JS_ValueToECMAInt32(cx, *vp, &monthVal))
   {
      priv->month = static_cast<int>(monthVal);
      return JS_TRUE;
   }
   else
   {
      JS_ReportError(cx, "Could not set month from value");
      return JS_FALSE;
   }
}

static JSBool Pdate_GetYear(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PdateFromPrivate(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint jsYear = static_cast<jsint>(priv->year);
   *vp = INT_TO_JSVAL(jsYear);
   return JS_TRUE;
}

static JSBool Pdate_SetYear(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PdateFromPrivate(cx, obj);
   if(!priv)
      return JS_FALSE;

   int32 yearVal = priv->year;
   if(JS_ValueToECMAInt32(cx, *vp, &yearVal))
   {
      priv->year = static_cast<int>(yearVal);
      return JS_TRUE;
   }
   else
   {
      JS_ReportError(cx, "Could not set year from value");
      return JS_FALSE;
   }
}

// Ptime

static JSBool Ptime_GetHour(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PtimeFromPrivate(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint val = static_cast<jsint>(priv->hour);
   *vp = INT_TO_JSVAL(val);
   return JS_TRUE;
}

static JSBool Ptime_SetHour(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PtimeFromPrivate(cx, obj);
   if(!priv)
      return JS_FALSE;

   int32 val = priv->hour;
   if(JS_ValueToECMAInt32(cx, *vp, &val))
   {
      priv->hour = static_cast<int>(val);
      return JS_TRUE;
   }
   else
   {
      JS_ReportError(cx, "Could not set hour from value");
      return JS_FALSE;
   }
}

static JSBool Ptime_GetMinute(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PtimeFromPrivate(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint val = static_cast<jsint>(priv->minute);
   *vp = INT_TO_JSVAL(val);
   return JS_TRUE;
}

static JSBool Ptime_SetMinute(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PtimeFromPrivate(cx, obj);
   if(!priv)
      return JS_FALSE;

   int32 val = priv->minute;
   if(JS_ValueToECMAInt32(cx, *vp, &val))
   {
      priv->minute = static_cast<int>(val);
      return JS_TRUE;
   }
   else
   {
      JS_ReportError(cx, "Could not set minute from value");
      return JS_FALSE;
   }
}

static JSBool Ptime_GetSecond(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PtimeFromPrivate(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint val = static_cast<jsint>(priv->second);
   *vp = INT_TO_JSVAL(val);
   return JS_TRUE;
}

static JSBool Ptime_SetSecond(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PtimeFromPrivate(cx, obj);
   if(!priv)
      return JS_FALSE;

   int32 val = priv->second;
   if(JS_ValueToECMAInt32(cx, *vp, &val))
   {
      priv->second = static_cast<int>(val);
      return JS_TRUE;
   }
   else
   {
      JS_ReportError(cx, "Could not set second from value");
      return JS_FALSE;
   }
}

static JSBool Ptime_GetMillisecond(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PtimeFromPrivate(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint val = static_cast<jsint>(priv->millisecond);
   *vp = INT_TO_JSVAL(val);
   return JS_TRUE;
}

static JSBool Ptime_SetMillisecond(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PtimeFromPrivate(cx, obj);
   if(!priv)
      return JS_FALSE;

   int32 val = priv->millisecond;
   if(JS_ValueToECMAInt32(cx, *vp, &val))
   {
      priv->millisecond = static_cast<int>(val);
      return JS_TRUE;
   }
   else
   {
      JS_ReportError(cx, "Could not set millisecond from value");
      return JS_FALSE;
   }
}

//
// Property Tables
//

static JSPropertySpec PdateProps[] =
{
   { 
      "day", 0, 
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED,
      Pdate_GetDay, Pdate_SetDay
   },

   {
      "month", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED,
      Pdate_GetMonth, Pdate_SetMonth
   },

   {
      "year", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED,
      Pdate_GetYear, Pdate_SetYear
   },

   { nullptr }
};

static JSPropertySpec PtimeProps[] =
{
   {
      "hour", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED,
      Ptime_GetHour, Ptime_SetHour 
   },

   {
      "minute", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED,
      Ptime_GetMinute, Ptime_SetMinute 
   },

   {
      "second", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED,
      Ptime_GetSecond, Ptime_SetSecond
   },

   {
      "millisecond", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED,
      Ptime_GetMillisecond, Ptime_SetMillisecond
   },

   { nullptr }
};

static JSPropertySpec PstampProps[] =
{
   { 
      "day", 0, 
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED,
      Pdate_GetDay, Pdate_SetDay
   },

   {
      "month", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED,
      Pdate_GetMonth, Pdate_SetMonth
   },

   {
      "year", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED,
      Pdate_GetYear, Pdate_SetYear
   },

   {
      "hour", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED,
      Ptime_GetHour, Ptime_SetHour 
   },

   {
      "minute", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED,
      Ptime_GetMinute, Ptime_SetMinute 
   },

   {
      "second", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED,
      Ptime_GetSecond, Ptime_SetSecond
   },

   {
      "millisecond", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED,
      Ptime_GetMillisecond, Ptime_SetMillisecond
   },

   { nullptr }
};

//
// Create Functions and Native Objects
//

static NativeInitCode Pdate_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &pDateClass, 
                           JSEngineNativeWrapper<Pdate_New>,
                           0, PdateProps, PdateMethods, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;  
}

static Native pDateGlobalNative("Pdate", Pdate_Create);

static NativeInitCode Ptime_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &pTimeClass, 
                           JSEngineNativeWrapper<Ptime_New>,
                           0, PtimeProps, PtimeMethods, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native pTimeGlobalNative("Ptime", Ptime_Create);

static NativeInitCode Pstamp_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &pStampClass, 
                           JSEngineNativeWrapper<Pstamp_New>,
                           0, PstampProps, PstampMethods, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native pStampGlobalNative("Pstamp", Pstamp_Create);

// EOF

