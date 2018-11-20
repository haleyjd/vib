/*
  
  Internal helpers for Symbol polyfill

*/

#include <map>
#include <string>
#include "jsengine2.h"
#include "jsnatives.h"

//
// NativeSymbolRef maintains a reference to an object onto which Symbol
// properties are being defined in JavaScript (this will be the global
// Object.prototype, in practice), and removes the property corresponding
// to a given Symbol instance when that instance's properties (including
// the reference to this NativeSymbolRef object) are finalized. This
// prevents accumulation of dead Symbol properties on Object.prototype.
//
class NativeSymbolRef : public PrivateData
{
   DECLARE_PRIVATE_DATA()

protected:
   std::string    m_propName;
   JSObject      *m_protoObj;
   AutoNamedRoot  prototype;
   int            references;

public:
   NativeSymbolRef(JSContext *cx, JSObject *protoObj, const char *propName) 
      : PrivateData(), m_propName(propName), m_protoObj(protoObj),
        prototype(cx, protoObj, "NativeSymbolRef"), references(1)
   {
   }

   void addReference() { ++references; }
   void remReference() { --references; }
   bool hasReferences() const { return references > 0; }

   const char *getPropName()  const { return m_propName.c_str(); }
   JSObject   *getPrototype() const { return m_protoObj;         }
};

typedef std::map<std::string, NativeSymbolRef *> NSRMap;
static NSRMap refForName;

//
// Constructor
//
static JSBool NativeSymbolRef_New(JSContext *cx, JSObject *obj, uintN argc,
                                  jsval *argv, jsval *rval)
{
   ASSERT_IS_CONSTRUCTING(cx, "NativeSymbolRef");
   ASSERT_ARGC_GE(argc, 2, "NativeSymbolRef");
   ASSERT_IS_OBJECT(argv[1], "prototype");
   const char *propName = SafeGetStringBytes(cx, argv[0], &argv[0]);
   JSObject   *protoObj = JSVAL_TO_OBJECT(argv[1]);

   NSRMap::const_iterator itr;
   if((itr = refForName.find(propName)) != refForName.end())
   {
      itr->second->setToJSObject(cx, obj);
      itr->second->addReference();
   }
   else
   {
      std::unique_ptr<NativeSymbolRef> newPriv(new NativeSymbolRef(cx, protoObj, propName));
      NativeSymbolRef *nsr = newPriv.get();
      newPriv->setToJSObjectAndRelease(cx, obj, newPriv);
      refForName[propName] = nsr;
   }

   *rval = JSVAL_VOID;
   return JS_TRUE;
}

//
// Finalizer
//
static void NativeSymbolRef_Finalize(JSContext *cx, JSObject *obj)
{
   auto priv = PrivateData::GetFromJSObject<NativeSymbolRef>(cx, obj);

   if(priv)
   {
      priv->remReference();
      if(!priv->hasReferences())
      {
         refForName.erase(priv->getPropName());
         JS_DeleteProperty(cx, priv->getPrototype(), priv->getPropName());
         delete priv;
      }
      JS_SetPrivate(cx, obj, nullptr);
   }
}

/**
 * JSClass for NativeSymbolRef
 */
static JSClass nativeSymbolRefClass =
{
   "NativeSymbolRef",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   NativeSymbolRef_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(NativeSymbolRef, nativeSymbolRefClass)

static NativeInitCode NativeSymbolRef_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &nativeSymbolRefClass, 
                           JSEngineNativeWrapper<NativeSymbolRef_New>,
                           0, nullptr, nullptr, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native symbolRefNative("NativeSymbolRef", NativeSymbolRef_Create);

// EOF

