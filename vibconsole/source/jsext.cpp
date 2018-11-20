/*
  Native extensions for built-in JS Objects
*/

#include "jsengine2.h"
#include "jsnatives.h"
#include "util.h"

//=============================================================================
//
// ECMAScript 5 property descriptors
//

//
// Object_defineProperty
//
// Implements an approximation of ECMAScript 5 Object.defineProperty; there are
// some subtle non-compliances due to limitations of SpiderMonkey 1.8.0.
//
static JSBool Object_defineProperty(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 3, "Object.defineProperty");
   ASSERT_IS_OBJECT(argv[0], "obj argument");
   ASSERT_IS_OBJECT(argv[2], "descriptor argument");

   JSObject *obj = JSVAL_TO_OBJECT(argv[0]);
   JSObject *dsc = JSVAL_TO_OBJECT(argv[2]);

   AutoJSValueToStringRooted str(cx, argv[1]);
   bool configurable = false, enumerable = false, writable = false;

   // get shared properties
   JSBool boolValue;
   jsval  value = JSVAL_VOID;
   if(JS_GetProperty(cx, dsc, "configurable", &value))
   {
      if(JS_ValueToBoolean(cx, value, &boolValue))
         configurable = (boolValue == JS_TRUE);
   }
   if(JS_GetProperty(cx, dsc, "enumerable", &value))
   {
      if(JS_ValueToBoolean(cx, value, &boolValue))
         enumerable = (boolValue == JS_TRUE);
   }

   // Figure out what type of descriptor this is...
   bool hasValue = false, hasWritable = false, hasGet = false, hasSet = false;
   boolValue = JS_FALSE;
   if(JS_HasProperty(cx, dsc, "value", &boolValue) && boolValue)
      hasValue = true;
   if(JS_HasProperty(cx, dsc, "writable", &boolValue) && boolValue)
      hasWritable = true;
   if(JS_HasProperty(cx, dsc, "get", &boolValue) && boolValue)
      hasGet = true;
   if(JS_HasProperty(cx, dsc, "set", &boolValue) && boolValue)
      hasSet = true;

   // Check for invalid combinations
   if((hasValue || hasWritable) && (hasGet || hasSet))
      throw JSEngineError("Invalid property descriptor; mixed state");

   boolValue = JS_FALSE;
   if((hasValue || hasWritable) || !(hasGet || hasSet)) // data descriptor
   {
      // get writable property
      if(JS_GetProperty(cx, dsc, "writable", &value))
      {
         if(JS_ValueToBoolean(cx, value, &boolValue))
            writable = (boolValue == JS_TRUE);
      }

      // define the property
      uintN attrs = 0;
      if(!configurable)
         attrs |= JSPROP_PERMANENT;
      if(enumerable)
         attrs |= JSPROP_ENUMERATE;
      if(!writable)
         attrs |= JSPROP_READONLY;

      jsval value = JSVAL_VOID;
      JS_GetProperty(cx, dsc, "value", &value);

      jschar *jschars = JS_GetStringChars(str);
      size_t  jslen   = JS_GetStringLength(str);

      AssertJSDefineUCProperty(cx, obj, jschars, jslen, value, nullptr, nullptr, attrs);
   }
   else
   {
      JSObject *get = nullptr, *set = nullptr;

      // get "get" property
      jsval value = JSVAL_VOID;
      if(JS_GetProperty(cx, dsc, "get", &value) && JSVAL_IS_OBJECT(value))
      {
         JSObject *fobj = JSVAL_TO_OBJECT(value);
         if(JS_ObjectIsFunction(cx, fobj))
            get = fobj;
      }

      // get "set" property
      value = JSVAL_VOID;
      if(JS_GetProperty(cx, dsc, "set", &value) && JSVAL_IS_OBJECT(value))
      {
         JSObject *fobj = JSVAL_TO_OBJECT(value);
         if(JS_ObjectIsFunction(cx, fobj))
            set = fobj;
      }

      uintN attrs = JSPROP_SHARED;
      if(!configurable)
         attrs |= JSPROP_PERMANENT;
      if(enumerable)
         attrs |= JSPROP_ENUMERATE;
      if(get)
         attrs |= JSPROP_GETTER;
      if(set)
         attrs |= JSPROP_SETTER;

      jschar *jschars = JS_GetStringChars(str);
      size_t  jslen   = JS_GetStringLength(str);

      AssertJSDefineUCProperty(cx, obj, jschars, jslen, value, (JSPropertyOp)get, (JSPropertyOp)set, attrs);
   }

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

//
// Object_getOwnPropertyDescriptorInt
//
// Implements approximate internal logic for ECMAScript 5 Object.getOwnPropertyDescriptor; 
// there are some subtle non-compliances due to limitations of SpiderMonkey 1.8.0. This
// is not a standalone function; it must be invoked from inside a JavaScript wrapper.
//
static JSBool Object_getOwnPropertyDescriptorInt(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 2, "Extensions.objectGetOwnPropertyDescriptorInt");
   ASSERT_IS_OBJECT(argv[0], "obj argument");

   AutoJSValueToStringRooted str(cx, argv[1]);

   JSObject *obj     = JSVAL_TO_OBJECT(argv[0]);
   jschar   *jschars = JS_GetStringChars(str);
   size_t    jslen   = JS_GetStringLength(str);

   jsval prop;
   if(!JS_GetUCProperty(cx, obj, jschars, jslen, &prop))
   {
      JS_SET_RVAL(cx, vp, JSVAL_VOID); // undefined
      return JS_TRUE;
   }

   uintN  attrs = 0;
   JSBool found = JS_FALSE;
   JSPropertyOp getter = nullptr, setter = nullptr;
   if(!JS_GetUCPropertyAttrsGetterAndSetter(cx, obj, jschars, jslen, &attrs, &found, &getter, &setter) || !found)
   {
      /*
      For whatever reason, JS_GetPropertyAttributes and friends have a different idea about the source of 
      properties than does the language itself. We have already checked that obj.hasOwnProperty(prop) is true, 
      and have gotten its value, but due to an unsophisticated check in the GetPropertyAttributes function in 
      jsapi.c, the function will say the property is not found. In this case, we have to walk up the prototype 
      chain, since natively specified properties are somehow positioned a step up from the object itself.
      */
      JSObject *prev  = obj;
      JSObject *proto = obj;
      while((proto = JS_GetPrototype(cx, proto)) && proto != prev)
      {
         if(JS_GetUCPropertyAttrsGetterAndSetter(cx, proto, jschars, jslen, &attrs, &found, &getter, &setter) && found)
            break;
         prev = proto;
      }
      if(!found)
         throw JSEngineError("Could not retrieve property attributes, internal JSAPI error");
   }

   // native objects w/cached prototypes do not behave correctly in this regard; they
   // return a getter function for anything assigned to them. If the getter/setter are not
   // actual JS functions for these objects, create a value descriptor instead.
   JSClass *jsclass = JS_GetClass(obj);
   if(jsclass && JSCLASS_CACHED_PROTO_KEY(jsclass) != JSProto_Null) 
   {
      if(getter && !(attrs & JSPROP_GETTER))
         getter = nullptr;
      if(setter && !(attrs & JSPROP_SETTER))
         setter = nullptr;
   }

   JSObject *descriptor = AssertJSNewObject(cx, nullptr, nullptr, nullptr);
   AutoNamedRoot anr(cx, obj, "GetPropertyDescRoot");

   if(getter || setter) // accessor descriptor needed
   {
      if(getter && !(attrs & JSPROP_GETTER)) // native getter, must use wrapper
      {
         ASSERT_ARGC_GE(argc, 3, "Object.getOwnPropertyDescriptor");
         ASSERT_VALUE_IS_FUNCTION(cx, argv[2]);
         getter = reinterpret_cast<JSPropertyOp>(JSVAL_TO_OBJECT(argv[2]));
      }
      if(setter && !(attrs & JSPROP_SETTER)) // native setter, must use wrapper
      {
         ASSERT_ARGC_GE(argc, 4, "Object.getOwnPropertyDescriptor");
         ASSERT_VALUE_IS_FUNCTION(cx, argv[3]);
         setter = reinterpret_cast<JSPropertyOp>(JSVAL_TO_OBJECT(argv[3]));
      }

      jsval getval = getter ? OBJECT_TO_JSVAL(reinterpret_cast<JSObject *>(getter)) : JSVAL_VOID;
      AssertJSDefineProperty(cx, descriptor, "get", getval, nullptr, nullptr, JSPROP_ENUMERATE);

      jsval setval = setter ? OBJECT_TO_JSVAL(reinterpret_cast<JSObject *>(setter)) : JSVAL_VOID;
      AssertJSDefineProperty(cx, descriptor, "set", setval, nullptr, nullptr, JSPROP_ENUMERATE);
   }
   else // data descriptor needed
   {
      AssertJSDefineProperty(cx, descriptor, "value", prop, nullptr, nullptr, JSPROP_ENUMERATE);

      JSBool writable = (attrs & JSPROP_READONLY) ? JS_FALSE : JS_TRUE;
      AssertJSDefineProperty(cx, descriptor, "writable", BOOLEAN_TO_JSVAL(writable), nullptr, nullptr, JSPROP_ENUMERATE);
   }

   // shared properties

   // NB: the concept of type mutability is not reflected here, only capability of deletion.
   JSBool configurable = (attrs & JSPROP_PERMANENT) ? JS_FALSE : JS_TRUE;
   AssertJSDefineProperty(cx, descriptor, "configurable", BOOLEAN_TO_JSVAL(configurable), nullptr, nullptr, JSPROP_ENUMERATE);

   JSBool enumerable = (attrs & JSPROP_ENUMERATE) ? JS_TRUE : JS_FALSE;
   AssertJSDefineProperty(cx, descriptor, "enumerable", BOOLEAN_TO_JSVAL(enumerable), nullptr, nullptr, JSPROP_ENUMERATE);

   // got it!
   JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(descriptor));
   return JS_TRUE;
}

//
// If your mind is blown by the necessity of walking down the prototype chain to find all of an
// object's "own" properties, well, join the party. The JSAPI places natively defined "shared"
// properties on an object's prototype, and then effectively lies through its teeth anywhere the
// *language itself* asks questions such as "hasOwnProperty" about an object, using a hack on the
// combination of the JSPROP_SHARED|JSPROP_PERMANENT flags.
//
// Since the API was designed to be suitable for monkeys banging on keyboards, however, it neglects
// to allow for the fact entirely and ignores that such properties exist at all. Not even the 
// JS_EnumerateAll API I added to make this possible at all can see them *on the object itself*.
//
// So, we walk down the prototype chain in the function below and call this on every object up
// the chain. Any objects on the chain defining SHARED|PERMANENT properties get those concatenated
// to the array of property names.
//
// We stop below on a class having JSProto_Object as its JSClass tag, however, because that class
// only contains the three non-standard Mozilla "magic" properties, __proto__, __parent__, and
// __count__, which we do not want showing up for the sake of compatibility.
//
static void AddOwnPropNames(JSContext *cx, JSObject *obj, JSObject *arr, bool isProto, jsint &idx)
{
   JSIdArray *ids = AssertJSEnumerateAll(cx, obj);
   AutoIdArray autoArr(cx, ids);

   for(jsint i = 0; i < ids->length; i++)
   {
      jsval prop;
      AssertJSIdToValue(cx, ids->vector[i], &prop);
      AutoJSValueToStringRooted astr;
      JSString *jstr;

      if(!JSVAL_IS_STRING(prop))
      {
         astr.init(cx, prop);
         jstr = astr;
      }
      else
         jstr = JSVAL_TO_STRING(prop);

      // Again, due to stupid API design and complete inconsistency between the API
      // and the actual language, we must scan object prototypes for natively defined
      // properties.
      if(isProto)
      {
         jschar *name  = JS_GetStringChars(jstr);
         size_t  len   = JS_GetStringLength(jstr);
         uintN   attrs = 0;
         JSBool  found = JS_FALSE;
         if(JS_GetUCPropertyAttributes(cx, obj, name, len, &attrs, &found) && found)
         {
            // only add prototype properties that are shared and permanent
            if((~attrs & (JSPROP_SHARED|JSPROP_PERMANENT)) == 0)
            {
               prop = STRING_TO_JSVAL(jstr);
               AssertJSSetElement(cx, arr, idx, &prop);
               ++idx;
            }
         }
      }
      else
      {
         prop = STRING_TO_JSVAL(jstr);
         AssertJSSetElement(cx, arr, idx, &prop);
         ++idx;
      }
   }
}

//
// Object_getOwnPropertyNamesInt
//
// Implements ECMAScript 5 Object.getOwnPropertyNames
//
static JSBool Object_getOwnPropertyNamesInt(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "Object.getOwnPropertyNames");
   ASSERT_IS_OBJECT(argv[0], "obj argument");

   JSObject *arr = AssertJSNewArrayObject(cx, 0, nullptr);
   AutoNamedRoot anr(cx, arr, "Object_getOwnPropertyNames");

   JSObject *obj = JSVAL_TO_OBJECT(argv[0]);
   jsint idx = 0;

   AddOwnPropNames(cx, obj, arr, false, idx); // add own props
   while((obj = JS_GetPrototype(cx, obj)))
   {
      JSClass *jsclass = JS_GetClass(obj);         
      if(jsclass && JSCLASS_CACHED_PROTO_KEY(jsclass) == JSProto_Object) // do not do for Object class.
         break;
      AddOwnPropNames(cx, obj, arr, true, idx); // add idiotic native shared properties
   }

   JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(arr));
   return JS_TRUE;
}


//=============================================================================
//
// Extensions class
//

// Class for containing native extension functions; they must be assigned to their built-in objects as
// properties in a script, such as in autoexec.js
static JSClass extensions_class =
{
   "Extensions",
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

static JSFunctionSpec extMethods[] =
{
   JSE_FN("objectDefineProperty",              Object_defineProperty,              3, 0, 0),
   JSE_FN("objectGetOwnPropertyDescriptorInt", Object_getOwnPropertyDescriptorInt, 2, 0, 0),
   JSE_FN("objectGetOwnPropertyNamesInt",      Object_getOwnPropertyNamesInt,      1, 0, 0),
   JS_FS_END
};

static NativeInitCode Extensions_Create(JSContext *cx, JSObject *global)
{
   JSObject *obj;

   if(!(obj = JS_DefineObject(cx, global, "Extensions", &extensions_class, nullptr, JSPROP_PERMANENT)))
      return RESOLUTIONERROR;

   if(!JS_DefineFunctions(cx, obj, extMethods))
      return RESOLUTIONERROR;

   return RESOLVED;
}

static Native extensionsNative("Extensions", Extensions_Create);

// EOF

