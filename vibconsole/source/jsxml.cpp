/*
   JS Bindings for libxml2 functionality
*/

#ifndef VIBC_NO_LIBXML2

#include "libxml/xmlmemory.h"
#include "libxml/parser.h"
#include "libxml/HTMLparser.h"
#include "libxml/xpath.h"
#include "libxml/xpathInternals.h"
#include "libxml/uri.h"

#include "jsengine2.h"
#include "jsnatives.h"
#include "main.h"
#include "utfconv.h"

//=============================================================================
//
// Initialization and Shutdown
//

static void ShutdownLibxml()
{
   xmlCleanupParser();
   //xmlMemoryDump();
}

//
// InitLibxml
//
// Will initialize the libxml2 library the first time it is called.
//
static void InitLibxml()
{
   static bool loaded = false;

   if(!loaded)
   {
      xmlInitParser();
      LIBXML_TEST_VERSION
      new ShutdownAction(ShutdownLibxml);
      loaded = true;
   }
}

//=============================================================================
//
// String Utilities
//

template<typename S, typename D>
D safe_char_conversion(S str)
{
   return reinterpret_cast<D>(str);
}

// Safe casts (types are basically equivalent)

#define CCFROMXML(str) safe_char_conversion<const xmlChar  *, const char     *>(str)
#define XMLFROMCC(str) safe_char_conversion<const char     *, const xmlChar  *>(str)
#define JSFROMC16(str) safe_char_conversion<const char16_t *, const jschar   *>(str)
#define C16FROMJS(str) safe_char_conversion<const jschar   *, const char16_t *>(str)


static JSString *JSStringFromXMLChars(JSContext *cx, const xmlChar *str)
{
   size_t size = (size_t)(xmlUTF8Strsize(str, xmlStrlen(str)));

   char16_t *conv = UTF8toUTF16(CCFROMXML(str), size);
   JSString *jstr = JS_NewUCStringCopyZ(cx, JSFROMC16(conv));

   delete [] conv;

   if(!jstr)
      throw JSEngineError("Out of memory", true);

   return jstr;
}

static xmlChar *XMLCharsFromJSString(JSContext *cx, JSString *jstr)
{
   jschar *chars = JS_GetStringChars(jstr);
   size_t  size  = JS_GetStringLength(jstr);

   char *res = UTF16toUTF8(C16FROMJS(chars), size);

   if(!res)
      throw JSEngineError("Cannot convert UTF16 to UTF8");

   return const_cast<xmlChar *>(XMLFROMCC(res));
}

// For return values from XMLCharsFromJSString
typedef std::unique_ptr<xmlChar []> AutoXMLStr;

// For return values from the libxml API
class AutoXMLChar
{
protected:
   xmlChar *str;
public:
   AutoXMLChar(xmlChar *s) : str(s) {}
   ~AutoXMLChar()
   {
      if(str)
         xmlFree(str);
      str = nullptr;
   }
   xmlChar *get() const { return str; }
};

//=============================================================================
//
// Enumerations
//

static EnumEntry xmlElementTypeEnumValues[] =
{
   { XML_ELEMENT_NODE,       "ELEMENT_NODE"       },
   { XML_ATTRIBUTE_NODE,     "ATTRIBUTE_NODE"     },
   { XML_TEXT_NODE,          "TEXT_NODE"          },
   { XML_CDATA_SECTION_NODE, "CDATA_SECTION_NODE" },
   { XML_ENTITY_REF_NODE,    "ENTITY_REF_NODE"    },
   { XML_ENTITY_NODE,        "ENTITY_NODE"        },
   { XML_PI_NODE,            "PI_NODE"            },
   { XML_COMMENT_NODE,       "COMMENT_NODE"       },
   { XML_DOCUMENT_NODE,      "DOCUMENT_NODE"      },
   { XML_DOCUMENT_TYPE_NODE, "DOCUMENT_TYPE_NODE" },
   { XML_DOCUMENT_FRAG_NODE, "DOCUMENT_FRAG_NODE" },
   { XML_NOTATION_NODE,      "NOTATION_NODE"      },
   { XML_HTML_DOCUMENT_NODE, "HTML_DOCUMENT_NODE" },
   { XML_DTD_NODE,           "DTD_NODE"           },
   { XML_ELEMENT_DECL,       "ELEMENT_DECL"       },
   { XML_ATTRIBUTE_DECL,     "ATTRIBUTE_DECL"     },
   { XML_ENTITY_DECL,        "ENTITY_DECL"        },
   { XML_NAMESPACE_DECL,     "NAMESPACE_DECL"     },
   { XML_XINCLUDE_START,     "XINCLUDE_START"     },
   { XML_XINCLUDE_END,       "XINCLUDE_END"       },
   { XML_DOCB_DOCUMENT_NODE, "DOCB_DOCUMENT_NODE" },
   ENUMEND()
};

static EnumEntry xmlAttributeTypeValues[] =
{
   { XML_ATTRIBUTE_CDATA,        "ATTRIBUTE_CDATA"       },
   { XML_ATTRIBUTE_ID,           "ATTRIBUTE_ID"          },
   { XML_ATTRIBUTE_IDREF,        "ATTRIBUTE_IDREF"       },
   { XML_ATTRIBUTE_IDREFS,       "ATTRIBUTE_IDREFS"      },
   { XML_ATTRIBUTE_ENTITY,       "ATTRIBUTE_ENTITY"      },
   { XML_ATTRIBUTE_ENTITIES,     "ATTRIBUTE_ENTITIES"    },
   { XML_ATTRIBUTE_NMTOKEN,      "ATTRIBUTE_NMTOKEN"     },
   { XML_ATTRIBUTE_NMTOKENS,     "ATTRIBUTE_NMTOKENS"    },
   { XML_ATTRIBUTE_ENUMERATION,  "ATTRIBUTE_ENUMERATION" },
   { XML_ATTRIBUTE_NOTATION,     "ATTRIBUTE_NOTATION"    },
   ENUMEND()
};

//=============================================================================
//
// Wrapper Classes
//

//
// PrivateXMLURI
//
// Wraps an xmlURI object
//
class PrivateXMLURI : public PrivateData
{
   DECLARE_PRIVATE_DATA()

protected:
   xmlURIPtr uri;

public:
   PrivateXMLURI(xmlURIPtr pUri) : PrivateData(), uri(pUri)
   {
   }

   ~PrivateXMLURI()
   {
      if(uri)
      {
         xmlFreeURI(uri);
         uri = nullptr;
      }
   }

   xmlURIPtr getURI() const { return uri; }

   JSString *toString(JSContext *cx) const
   {
      xmlChar *chars;
      AutoXMLChar chp((chars = xmlSaveUri(uri)));
      return JSStringFromXMLChars(cx, chars ? chars : XMLFROMCC(""));
   }

   template<char *xmlURI::*str>
   JSString *jsStrFromProperty(JSContext *cx) const
   {
      auto field = uri->*str;
      return AssertJSNewStringCopyZ(cx, field ? field : "");
   }

   static PrivateXMLURI *FromCC(const char *str)
   {
      auto xuri = xmlParseURI(str);
      return xuri ? new PrivateXMLURI(xuri) : nullptr;
   }
};

//
// PrivateXMLAttr
//
// Wraps an xmlAttr node property object
//
class PrivateXMLAttr : public PrivateData
{
   DECLARE_PRIVATE_DATA()

protected:
   xmlAttrPtr attr;
   JSObject *docObj;

public:
   PrivateXMLAttr(xmlAttrPtr pAttr) : PrivateData(), attr(pAttr)
   {
   }

   ~PrivateXMLAttr()
   {
   }

   xmlAttrPtr getAttr() const { return attr; }

   xmlElementType getType() const { return attr->type; }

   JSString *getName(JSContext *cx) const
   {
      const xmlChar *name = XMLFROMCC("");
      if(attr->name)
         name = attr->name;
      return JSStringFromXMLChars(cx, name);
   }

   JSString *getValue(JSContext *cx) const
   {
      const xmlChar *content = XMLFROMCC("");
      if(attr->children && attr->children->content)
         content = attr->children->content;
      return JSStringFromXMLChars(cx, content);
   }

   xmlAttributeType getAttributeType() const { return attr->atype; }

   void setDocObj(JSObject *pDocObj) { docObj = pDocObj; }
   JSObject *getDocObj() const { return docObj; }

   // Statics

   static PrivateXMLAttr *FromNodeProperties(xmlNodePtr node)
   {
      auto attr = node->properties;
      return attr ? new PrivateXMLAttr(attr) : nullptr;
   }

   static PrivateXMLAttr *FromHasProp(xmlNodePtr node, const xmlChar *name)
   {
      auto attr = xmlHasProp(node, name);
      return attr ? new PrivateXMLAttr(attr) : nullptr;
   }

   static PrivateXMLAttr *FromNext(xmlAttrPtr attr)
   {
      auto next = attr->next;
      return next ? new PrivateXMLAttr(next) : nullptr;
   }
   
   static PrivateXMLAttr *FromPrev(xmlAttrPtr attr)
   {
      auto prev = attr->prev;
      return prev ? new PrivateXMLAttr(prev) : nullptr;
   }

   static PrivateXMLAttr *FromGenericAttr(xmlAttrPtr attr)
   {
      return attr ? new PrivateXMLAttr(attr) : nullptr;
   }
};

//
// PrivateXMLNode
//
// Wraps the xmlNode object
//
class PrivateXMLNode : public PrivateData
{
   DECLARE_PRIVATE_DATA()

protected:
   xmlNodePtr node;
   JSObject *docObj;

public:
   PrivateXMLNode(xmlNodePtr pNode) : PrivateData(), node(pNode)
   {
   }

   ~PrivateXMLNode()
   {
   }

   // Methods

   xmlNodePtr getNode() const { return node; }

   xmlElementType getType() const { return node->type; }

   JSString *getName(JSContext *cx) const
   {
      const xmlChar *name = node->name ? node->name : XMLFROMCC("");
      return JSStringFromXMLChars(cx, name);
   }

   JSString *getContent(JSContext *cx) const
   {
      AutoXMLChar str(xmlNodeGetContent(node));
      return JSStringFromXMLChars(cx, str.get() ? str.get() : XMLFROMCC(""));
   }

   JSString *getNodePath(JSContext *cx) const
   {
      AutoXMLChar str(xmlGetNodePath(node));
      return JSStringFromXMLChars(cx, str.get() ? str.get() : XMLFROMCC(""));
   }

   // child element count
   unsigned long getChildElementCount() const 
   {
      return xmlChildElementCount(node);
   }

   // line number
   long getLineNo() const
   {
      return xmlGetLineNo(node);
   }

   // is blank node?
   bool isBlankNode() const
   {
      return !!xmlIsBlankNode(node);
   }

   // is text?
   bool isTextNode() const
   {
      return !!xmlNodeIsText(node);
   }

   // get space preserve property (-1, 0, or 1)
   int getSpacePreserve() const 
   {
      return xmlNodeGetSpacePreserve(node);
   }

   // Build an array of child nodes
   JSObject *childNodeArray(JSContext *cx, xmlNodePtr node);

   // Build an array of child attributes   
   JSObject *childAttribArray(JSContext *cx, xmlNodePtr node);

   void setDocObj(JSObject *pDocObj) { docObj = pDocObj; }
   JSObject *getDocObj() const { return docObj; }

   // Statics

   // From doc root node
   static PrivateXMLNode *FromDocRoot(xmlDocPtr doc)
   {
      auto node = xmlDocGetRootElement(doc);
      return node ? new PrivateXMLNode(node) : nullptr;
   }

   // From first child node
   static PrivateXMLNode *FromFirstChild(xmlNodePtr parent)
   {
      auto node = parent->children;
      return node ? new PrivateXMLNode(node) : nullptr;
   }

   static PrivateXMLNode *FromLastChild(xmlNodePtr parent)
   {
      auto node = xmlGetLastChild(parent);
      return node ? new PrivateXMLNode(node) : nullptr;
   }

   // From last element child node
   static PrivateXMLNode *FromLastElementChild(xmlNodePtr parent)
   {
      auto node = xmlLastElementChild(parent);
      return node ? new PrivateXMLNode(node) : nullptr;
   }

   // From previous element sibling
   static PrivateXMLNode *FromPrevLastSibling(xmlNodePtr xnode)
   {
      auto node = xmlPreviousElementSibling(xnode);
      return node ? new PrivateXMLNode(node) : nullptr;
   }

   // From next sibling node
   static PrivateXMLNode *FromNext(xmlNodePtr prev)
   {
      return prev->next ? new PrivateXMLNode(prev->next) : nullptr;
   }

   // From previous sibling node
   static PrivateXMLNode *FromPrev(xmlNodePtr next)
   {
      return next->prev ? new PrivateXMLNode(next->prev) : nullptr;
   }

   // From parent node
   static PrivateXMLNode *FromParent(xmlNodePtr child)
   {
      return child->parent ? new PrivateXMLNode(child->parent) : nullptr;
   }

   // From generic node
   static PrivateXMLNode *FromGenericNode(xmlNodePtr node)
   {
      return node ? new PrivateXMLNode(node) : nullptr;
   }
};

#define ASSERT_DOC(doc, msg)   \
   if(!(doc))                  \
      throw JSEngineError(msg)

//
// PrivateXMLDocument
//
// Wraps the xmlDoc object
//
class PrivateXMLDocument : public PrivateData
{
   DECLARE_PRIVATE_DATA()

protected:
   xmlDocPtr doc;

public:
   PrivateXMLDocument(xmlDocPtr pDoc) : PrivateData(), doc(pDoc)
   {
      InitLibxml();
   }

   void release()
   {
      if(doc)
      {
         xmlFreeDoc(doc);
         doc = nullptr;
      }
   }

   ~PrivateXMLDocument()
   {
      release();
   }

   // Methods

   xmlDocPtr getDoc() const { return doc; }

   bool isEmpty() const
   {
      return (xmlDocGetRootElement(doc) == nullptr);
   }

   PrivateXMLNode *getRootNode() const 
   {
      return PrivateXMLNode::FromDocRoot(doc);
   }

   JSString *getNodeListString(JSContext *cx, xmlNodePtr node, bool inLine)
   {
      AutoXMLChar str(xmlNodeListGetString(doc, node, inLine));
      return JSStringFromXMLChars(cx, str.get() ? str.get() : XMLFROMCC(""));
   }

   int saveFile(const char *filename) const
   {
      return xmlSaveFile(filename, doc);
   }

   int saveFileEnc(const char *filename, const char *encoding) const
   {
      return xmlSaveFileEnc(filename, doc, encoding);
   }

   int saveFormatFile(const char *filename, int format) const
   {
      return xmlSaveFormatFile(filename, doc, format);
   }

   int saveFormatFileEnc(const char *filename, const char *encoding, int format) const
   {
      return xmlSaveFormatFileEnc(filename, doc, encoding, format);
   }

   // Statics

   // File reading factory
   static PrivateXMLDocument *FromFile(const char *fn)
   {
      auto doc = xmlParseFile(fn);
      ASSERT_DOC(doc, "Could not open XML document from file");
      return new PrivateXMLDocument(doc);
   }

   // File w/encoding and options
   static PrivateXMLDocument *FromFileOpt(const char *fn, const char *enc, int opt)
   {
      auto doc = xmlReadFile(fn, enc, opt);
      ASSERT_DOC(doc, "Could not open XML document from file");
      return new PrivateXMLDocument(doc);
   }

   // String-reading factory
   static PrivateXMLDocument *FromString(const char *buffer, int size)
   {
      auto doc = xmlParseMemory(buffer, size);
      ASSERT_DOC(doc, "Could not open XML document from string");
      return new PrivateXMLDocument(doc);
   }

   // From xmlChar String
   static PrivateXMLDocument *FromXMLChars(const xmlChar *cur)
   {
      auto doc = xmlParseDoc(cur);
      ASSERT_DOC(doc, "Could not open XML document from string");
      return new PrivateXMLDocument(doc);
   }

   // From xmlChar memory
   static PrivateXMLDocument *FromXMLCharMem(const xmlChar *cur, const char *URL,
                                             const char *encoding, int options)
   {
      auto doc = xmlReadDoc(cur, URL, encoding, options);
      ASSERT_DOC(doc, "Could not open XML document from memory");
      return new PrivateXMLDocument(doc);
   }

   // Memory-reading factory
   static PrivateXMLDocument *FromMemory(const char *buffer, int size, 
                                         const char *URL, const char *encoding,
                                         int options)
   {
      auto doc = xmlReadMemory(buffer, size, URL, encoding, options);
      ASSERT_DOC(doc, "Could not open XML document from memory");
      return new PrivateXMLDocument(doc);
   }

   //
   // HTML parsing
   //

   static PrivateXMLDocument *FromHTMLFile(const char *filename, const char *encoding)
   {
      auto doc = htmlParseFile(filename, encoding);
      ASSERT_DOC(doc, "Could not open HTML document from file");
      return new PrivateXMLDocument(doc);
   }

   static PrivateXMLDocument *FromHTMLFileOpt(const char *URL, const char *encoding, 
                                              int options)
   {
      auto doc = htmlReadFile(URL, encoding, options);
      ASSERT_DOC(doc, "Could not open HTML document from file");
      return new PrivateXMLDocument(doc);
   }

   static PrivateXMLDocument *FromHTMLString(xmlChar *cur, const char *encoding)
   {
      auto doc = htmlParseDoc(cur, encoding);
      ASSERT_DOC(doc, "Could not open HTML document from string");
      return new PrivateXMLDocument(doc);
   }

   static PrivateXMLDocument *FromHTMLStringOpt(xmlChar *cur, const char *URL, 
                                                const char *encoding, int options)
   {
      auto doc = htmlReadDoc(cur, URL, encoding, options);
      ASSERT_DOC(doc, "Could not open HTML document from string");
      return new PrivateXMLDocument(doc);
   }
};

class PrivateXMLXPathObj : public PrivateData
{
   DECLARE_PRIVATE_DATA()

protected:
   xmlXPathContextPtr ctx;
   xmlXPathObjectPtr  xpathobj;
   JSObject          *docObj;

public:
   PrivateXMLXPathObj(xmlXPathObjectPtr aXPathObj, xmlXPathContextPtr actx) 
      : PrivateData(), ctx(actx), xpathobj(aXPathObj)
   {
   }

   ~PrivateXMLXPathObj()
   {
      if(xpathobj)
      {
         xmlXPathFreeObject(xpathobj);
         xpathobj = nullptr;
      }
      if(ctx)
      {
         xmlXPathFreeContext(ctx);
         ctx = nullptr;
      }
   }

   //
   // Methods
   //

   xmlXPathObjectPtr getXPathObj() const { return xpathobj; }

   bool hasNodeSet()  const { return (xpathobj && xpathobj->nodesetval != nullptr); }
   int  nodeSetSize() const { return ((xpathobj && xpathobj->nodesetval) ? xpathobj->nodesetval->nodeNr : 0); }

   xmlNodePtr getNodeAt(int idx) const
   {
      return 
       ((xpathobj && xpathobj->nodesetval && idx >= 0 && idx < xpathobj->nodesetval->nodeNr) ?
         xpathobj->nodesetval->nodeTab[idx] : nullptr);
   }

   void setDocObj(JSObject *pDocObj) { docObj = pDocObj; }
   JSObject *getDocObj() const { return docObj; }

   //
   // Statics
   //

   static PrivateXMLXPathObj *FromExpression(xmlDocPtr doc, const xmlChar *expr, const std::map<std::string, std::string> &nsMap)
   {
      xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
      if(!ctx)
         throw JSEngineError("Could not create xpath context object");

      // register namespaces if provided
      for(auto itr = nsMap.cbegin(); itr != nsMap.cend(); ++itr)
      {
         xmlXPathRegisterNs(ctx, XMLFROMCC(itr->first.c_str()), XMLFROMCC(itr->second.c_str()));
      }

      xmlXPathObjectPtr xpathobj = xmlXPathEvalExpression(expr, ctx);
      if(!xpathobj)
      {
         xmlXPathFreeContext(ctx);
         throw JSEngineError("Could not evaluate xpath expression");
      }

      return new PrivateXMLXPathObj(xpathobj, ctx);
   }      
};

//=============================================================================
//
// XMLURI
//

// Constructor
static JSBool XMLURI_New(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                         jsval *rval)
{
   ASSERT_IS_CONSTRUCTING(cx, "XMLURI");

   const char *str = "";
   if(argc >= 1)
      str = SafeGetStringBytes(cx, argv[0], &argv[0]);

   std::unique_ptr<PrivateXMLURI> privuri(PrivateXMLURI::FromCC(str));
   privuri->setToJSObjectAndRelease(cx, obj, privuri);

   *rval = JSVAL_VOID;
   return JS_TRUE;
}

// Finalizer
static void XMLURI_Finalize(JSContext *cx, JSObject *obj)
{
   auto uri = PrivateData::GetFromJSObject<PrivateXMLURI>(cx, obj);

   if(uri)
   {
      delete uri;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

static JSClass xmluri_class =
{
   "XMLURI",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   XMLURI_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivateXMLURI, xmluri_class)

//
// XMLURI Methods
//

static JSBool XMLURI_toString(JSContext *cx, uintN argc, jsval *vp)
{
   auto uri = PrivateData::MustGetFromThis<PrivateXMLURI>(cx, vp);
   auto str = uri->toString(cx);
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(str));
   return JS_TRUE;
}

static JSFunctionSpec xmluriJSMethods[] =
{
   JSE_FN("toString", XMLURI_toString, 0, 0, 0),
   JS_FS_END
};

//
// XMLURI Properties
//

template<char *xmlURI::* str>
static JSBool XMLURI_GetStringProp(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   try
   {
      auto priv = PrivateData::MustGetFromJSObject<PrivateXMLURI>(cx, obj);
      auto strval = priv->jsStrFromProperty<str>(cx);
      *vp = STRING_TO_JSVAL(strval);
      return JS_TRUE;
   }
   catch(const JSEngineError &err)
   {
      return err.propagateToJS(cx);
   }
}

template<int xmlURI::* intfield>
static JSBool XMLURI_GetIntProp(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PrivateData::GetFromJSObject<PrivateXMLURI>(cx, obj);
   if(!priv)
      return JS_FALSE;
   auto intval = priv->getURI()->*intfield;
   *vp = INT_TO_JSVAL(intval);
   return JS_TRUE;
}

#define URISPROP(name, field)                         \
{                                                     \
   name, 0,                                           \
   JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY, \
   XMLURI_GetStringProp<field>, nullptr               \
}

#define URIIPROP(name, field)                         \
{                                                     \
   name, 0,                                           \
   JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY, \
   XMLURI_GetIntProp<field>, nullptr                  \
}

static JSPropertySpec xmluriJSProps[] =
{
   URISPROP("scheme",     &xmlURI::scheme),
   URISPROP("opaque",     &xmlURI::opaque),
   URISPROP("authority",  &xmlURI::authority),
   URISPROP("server",     &xmlURI::server),
   URISPROP("user",       &xmlURI::user),
   URISPROP("path",       &xmlURI::path),
   URISPROP("query",      &xmlURI::query),
   URISPROP("fragment",   &xmlURI::fragment),
   URISPROP("query_raw",  &xmlURI::query_raw),
   URIIPROP("port",       &xmlURI::port),
   URIIPROP("cleanup",    &xmlURI::cleanup),   
   { nullptr }
};

static NativeInitCode XMLURI_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &xmluri_class, 
                           JSEngineNativeWrapper<XMLURI_New>,
                           1, xmluriJSProps, xmluriJSMethods, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native xmluriGlobalNative("XMLURI", XMLURI_Create);

//=============================================================================
//
// XMLAttrib
//

static JSBool XMLAttr_ReturnObject(JSContext *cx, jsval *vp, JSObject *docObj, PrivateXMLAttr *attr);

// Finalizer
static void XMLAttr_Finalize(JSContext *cx, JSObject *obj)
{
   auto attr = PrivateData::GetFromJSObject<PrivateXMLAttr>(cx, obj);

   if(attr)
   {
      delete attr;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

static JSClass xmlattr_class =
{
   "XMLAttr",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   XMLAttr_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivateXMLAttr, xmlattr_class)

//
// XMLAttr Methods
//

static JSBool XMLAttr_getType(JSContext *cx, uintN argc, jsval *vp)
{
   auto attr = PrivateData::GetFromThis<PrivateXMLAttr>(cx, vp);
   if(!attr)
   {
      JS_ReportError(cx, "Invalid XMLAttr instance");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, INT_TO_JSVAL((int)attr->getType()));
   return JS_TRUE;
}

static JSBool XMLAttr_getName(JSContext *cx, uintN argc, jsval *vp)
{
   auto attr = PrivateData::MustGetFromThis<PrivateXMLAttr>(cx, vp);
   JSString *jstr = attr->getName(cx);
   JS_SET_RVAL(cx, vp, jstr ? STRING_TO_JSVAL(jstr) : JSVAL_NULL);
   return JS_TRUE;
}

static JSBool XMLAttr_getValue(JSContext *cx, uintN argc, jsval *vp)
{
   auto attr = PrivateData::MustGetFromThis<PrivateXMLAttr>(cx, vp);
   JSString *jstr = attr->getValue(cx);
   JS_SET_RVAL(cx, vp, jstr ? STRING_TO_JSVAL(jstr) : JSVAL_NULL);
   return JS_TRUE;
}

static JSBool XMLAttr_getAttributeType(JSContext *cx, uintN argc, jsval *vp)
{
   auto attr = PrivateData::GetFromThis<PrivateXMLAttr>(cx, vp);
   if(!attr)
   {
      JS_ReportError(cx, "Invalid XMLAttr instance");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, INT_TO_JSVAL((int)attr->getAttributeType()));
   return JS_TRUE;
}

// From next attribute
static JSBool XMLAttr_getNext(JSContext *cx, uintN argc, jsval *vp)
{
   PrivateXMLAttr *newAttr;
   auto thisAttr = PrivateData::GetFromThis<PrivateXMLAttr>(cx, vp);
   if(!thisAttr)
   {
      JS_ReportError(cx, "Invalid XMLAttr instance");
      return JS_FALSE;
   }
   if(!(newAttr = PrivateXMLAttr::FromNext(thisAttr->getAttr())))
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }

   return XMLAttr_ReturnObject(cx, vp, thisAttr->getDocObj(), newAttr);
}

// From previous attribute
static JSBool XMLAttr_getPrev(JSContext *cx, uintN argc, jsval *vp)
{
   PrivateXMLAttr *newAttr;
   auto thisAttr = PrivateData::GetFromThis<PrivateXMLAttr>(cx, vp);
   if(!thisAttr)
   {
      JS_ReportError(cx, "Invalid XMLAttr instance");
      return JS_FALSE;
   }
   if(!(newAttr = PrivateXMLAttr::FromPrev(thisAttr->getAttr())))
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }

   return XMLAttr_ReturnObject(cx, vp, thisAttr->getDocObj(), newAttr);
}

static JSFunctionSpec xmlattrJSMethods[] =
{
   JSE_FN("getType",                   XMLAttr_getType,          0, 0, 0),
   JSE_FN("getName",                   XMLAttr_getName,          0, 0, 0),
   JSE_FN("getValue",                  XMLAttr_getValue,         0, 0, 0),   
   JSE_FN("getAttributeType",          XMLAttr_getAttributeType, 0, 0, 0),
   JSE_FN("getNext",                   XMLAttr_getNext,          0, 0, 0),
   JSE_FN("getPrev",                   XMLAttr_getPrev,          0, 0, 0),
   JS_FS_END
};

// Create a new XMLAttr JS Object and set it as the return value. If the creation
// fails, out-of-memory is reported and the PrivateXMLAttr is destroyed.
static JSBool XMLAttr_ReturnObject(JSContext *cx, jsval *vp, JSObject *docObj, PrivateXMLAttr *attr)
{
   try
   {
      JSObject *newObj = AssertJSNewObject(cx, &xmlattr_class, nullptr, nullptr);
      AutoNamedRoot anr(cx, newObj, "NewXMLAttr");
      AssertJSDefineFunctions(cx, newObj, xmlattrJSMethods);
      AssertJSDefineProperty(cx, newObj, "document", OBJECT_TO_JSVAL(docObj), nullptr, nullptr, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
      attr->setToJSObject(cx, newObj);
      attr->setDocObj(docObj);
      JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newObj));
      return JS_TRUE;
   }
   catch(const JSEngineError &err)
   {
      delete attr;
      return err.propagateToJS(cx);
   }
}

static NativeInitCode XMLAttr_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &xmlattr_class, nullptr,
                           0, nullptr, xmlattrJSMethods, nullptr, nullptr);
   
   if(obj)
      AddEnumerationProperties(cx, obj, xmlAttributeTypeValues);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native xmlAttrGlobalNative("XMLAttr", XMLAttr_Create);

//=============================================================================
//
// XMLNode
//

// Finalizer
static void XMLNode_Finalize(JSContext *cx, JSObject *obj)
{
   auto node = PrivateData::GetFromJSObject<PrivateXMLNode>(cx, obj);

   if(node)
   {
      delete node;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

static JSClass xmlnode_class =
{
   "XMLNode",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   XMLNode_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivateXMLNode, xmlnode_class)

static JSBool XMLNode_ReturnObject(JSContext *cx, jsval *vp, JSObject *docObj, PrivateXMLNode *node);

//
// XMLNode Methods
//

static JSBool XMLNode_getType(JSContext *cx, uintN argc, jsval *vp)
{
   auto pnode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!pnode)
   {
      JS_ReportError(cx, "Invalid XMLNode object");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, INT_TO_JSVAL((int)pnode->getType()));
   return JS_TRUE;
}

static JSBool XMLNode_getName(JSContext *cx, uintN argc, jsval *vp)
{
   auto pnode = PrivateData::MustGetFromThis<PrivateXMLNode>(cx, vp);
   JSString *jstr = pnode->getName(cx);
   JS_SET_RVAL(cx, vp, jstr ? STRING_TO_JSVAL(jstr) : JSVAL_NULL);
   return JS_TRUE;
}

static JSBool XMLNode_getContent(JSContext *cx, uintN argc, jsval *vp)
{
   auto pnode = PrivateData::MustGetFromThis<PrivateXMLNode>(cx, vp);
   JSString *jstr = pnode->getContent(cx);
   JS_SET_RVAL(cx, vp, jstr ? STRING_TO_JSVAL(jstr) : JSVAL_NULL);
   return JS_TRUE;
}

static JSBool XMLNode_getNodePath(JSContext *cx, uintN argc, jsval *vp)
{
   auto pnode = PrivateData::MustGetFromThis<PrivateXMLNode>(cx, vp);
   JSString *jstr = pnode->getNodePath(cx);
   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(jstr));
   return JS_TRUE;
}

static JSBool XMLNode_getChildElementCount(JSContext *cx, uintN argc, jsval *vp)
{
   auto pnode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!pnode)
   {
      JS_ReportError(cx, "Invalid XMLNode object");
      return JS_FALSE;
   }

   auto cec = pnode->getChildElementCount();
   JS_SET_RVAL(cx, vp, INT_TO_JSVAL((jsint)cec));
   return JS_TRUE;
}

static JSBool XMLNode_getLineNo(JSContext *cx, uintN argc, jsval *vp)
{
   auto pnode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!pnode)
   {
      JS_ReportError(cx, "Invalid XMLNode object");
      return JS_FALSE;
   }

   auto cec = pnode->getLineNo();
   JS_SET_RVAL(cx, vp, INT_TO_JSVAL((jsint)cec));
   return JS_TRUE;
}

static JSBool XMLNode_isBlankNode(JSContext *cx, uintN argc, jsval *vp)
{
   auto pnode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!pnode)
   {
      JS_ReportError(cx, "Invalid XMLNode object");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, pnode->isBlankNode() ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool XMLNode_isTextNode(JSContext *cx, uintN argc, jsval *vp)
{
   auto pnode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!pnode)
   {
      JS_ReportError(cx, "Invalid XMLNode object");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, pnode->isTextNode() ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool XMLNode_getSpacePreserve(JSContext *cx, uintN argc, jsval *vp)
{
   auto pnode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!pnode)
   {
      JS_ReportError(cx, "Invalid XMLNode object");
      return JS_FALSE;
   }

   auto cec = pnode->getSpacePreserve();
   JS_SET_RVAL(cx, vp, INT_TO_JSVAL((jsint)cec));
   return JS_TRUE;
}

// From first child node
static JSBool XMLNode_getFirstChild(JSContext *cx, uintN argc, jsval *vp)
{
   PrivateXMLNode *newNode;
   auto thisNode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!thisNode)
   {
      JS_ReportError(cx, "Invalid XMLNode instance");
      return JS_FALSE;
   }
   if(!(newNode = PrivateXMLNode::FromFirstChild(thisNode->getNode())))
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }

   return XMLNode_ReturnObject(cx, vp, thisNode->getDocObj(), newNode);
}

// From last child node
static JSBool XMLNode_getLastChild(JSContext *cx, uintN argc, jsval *vp)
{
   PrivateXMLNode *newNode;
   auto thisNode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!thisNode)
   {
      JS_ReportError(cx, "Invalid XMLNode instance");
      return JS_FALSE;
   }
   if(!(newNode = PrivateXMLNode::FromLastChild(thisNode->getNode())))
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }

   return XMLNode_ReturnObject(cx, vp, thisNode->getDocObj(), newNode);
}

// From last element child
static JSBool XMLNode_getLastElementChild(JSContext *cx, uintN argc, jsval *vp)
{
   PrivateXMLNode *newNode;
   auto thisNode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!thisNode)
   {
      JS_ReportError(cx, "Invalid XMLNode instance");
      return JS_FALSE;
   }
   if(!(newNode = PrivateXMLNode::FromLastElementChild(thisNode->getNode())))
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }

   return XMLNode_ReturnObject(cx, vp, thisNode->getDocObj(), newNode);
}

// From previous element sibling
static JSBool XMLNode_getPreviousElementSibling(JSContext *cx, uintN argc, jsval *vp)
{
   PrivateXMLNode *newNode;
   auto thisNode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!thisNode)
   {
      JS_ReportError(cx, "Invalid XMLNode instance");
      return JS_FALSE;
   }
   if(!(newNode = PrivateXMLNode::FromPrevLastSibling(thisNode->getNode())))
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }

   return XMLNode_ReturnObject(cx, vp, thisNode->getDocObj(), newNode);
}

// From next sibling node
static JSBool XMLNode_getNext(JSContext *cx, uintN argc, jsval *vp)
{
   PrivateXMLNode *newNode;
   auto thisNode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!thisNode)
   {
      JS_ReportError(cx, "Invalid XMLNode instance");
      return JS_FALSE;
   }
   if(!(newNode = PrivateXMLNode::FromNext(thisNode->getNode())))
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }

   return XMLNode_ReturnObject(cx, vp, thisNode->getDocObj(), newNode);
}

// From previous sibling node
static JSBool XMLNode_getPrev(JSContext *cx, uintN argc, jsval *vp)
{
   PrivateXMLNode *newNode;
   auto thisNode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!thisNode)
   {
      JS_ReportError(cx, "Invalid XMLNode instance");
      return JS_FALSE;
   }
   if(!(newNode = PrivateXMLNode::FromPrev(thisNode->getNode())))
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }

   return XMLNode_ReturnObject(cx, vp, thisNode->getDocObj(), newNode);
}

// From parent node
static JSBool XMLNode_getParent(JSContext *cx, uintN argc, jsval *vp)
{
   PrivateXMLNode *newNode;
   auto thisNode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!thisNode)
   {
      JS_ReportError(cx, "Invalid XMLNode instance");
      return JS_FALSE;
   }
   if(!(newNode = PrivateXMLNode::FromParent(thisNode->getNode())))
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }

   return XMLNode_ReturnObject(cx, vp, thisNode->getDocObj(), newNode);
}

// Get first attribute node
static JSBool XMLNode_getFirstAttribute(JSContext *cx, uintN argc, jsval *vp)
{
   PrivateXMLAttr *attr;
   auto thisNode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!thisNode)
   {
      JS_ReportError(cx, "Invalid XMLNode instance");
      return JS_FALSE;
   }
   if(!(attr = PrivateXMLAttr::FromNodeProperties(thisNode->getNode())))
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }

   return XMLAttr_ReturnObject(cx, vp, thisNode->getDocObj(), attr);
}

static JSBool XMLNode_getChildNodes(JSContext *cx, uintN argc, jsval *vp)
{
   auto thisNode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!thisNode)
   {
      JS_ReportError(cx, "Invalid XMLNode instance");
      return JS_FALSE;
   }
   JSObject *arr = thisNode->childNodeArray(cx, thisNode->getNode());
   JS_SET_RVAL(cx, vp, arr ? OBJECT_TO_JSVAL(arr) : JSVAL_NULL);
   return JS_TRUE;
}

static JSBool XMLNode_getAttributes(JSContext *cx, uintN argc, jsval *vp)
{
   auto thisNode = PrivateData::GetFromThis<PrivateXMLNode>(cx, vp);
   if(!thisNode)
   {
      JS_ReportError(cx, "Invalid XMLNode instance");
      return JS_FALSE;
   }
   JSObject *arr = thisNode->childAttribArray(cx, thisNode->getNode());
   JS_SET_RVAL(cx, vp, arr ? OBJECT_TO_JSVAL(arr) : JSVAL_NULL);
   return JS_TRUE;
}

// static PrivateXMLAttr *FromHasProp(xmlNodePtr node, const xmlChar *name)
static JSBool XMLNode_hasProp(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto thisNode = PrivateData::MustGetFromThis<PrivateXMLNode>(cx, vp);
   ASSERT_ARGC_GE(argc, 1, "hasProp");

   AutoJSValueToStringRooted jstr(cx, argv[0]);

   xmlChar *prop = nullptr;
   AutoXMLStr propptr((prop = XMLCharsFromJSString(cx, jstr)));

   PrivateXMLAttr *attr = nullptr;
   if(!(attr = PrivateXMLAttr::FromHasProp(thisNode->getNode(), prop)))
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }

   return XMLAttr_ReturnObject(cx, vp, thisNode->getDocObj(), attr);
}

static JSFunctionSpec xmlnodeJSMethods[] =
{
   JSE_FN("getType",                   XMLNode_getType,                   0, 0, 0),
   JSE_FN("getName",                   XMLNode_getName,                   0, 0, 0),
   JSE_FN("getContent",                XMLNode_getContent,                0, 0, 0),
   JSE_FN("getNodePath",               XMLNode_getNodePath,               0, 0, 0),
   JSE_FN("getChildElementCount",      XMLNode_getChildElementCount,      0, 0, 0),
   JSE_FN("getLineNo",                 XMLNode_getLineNo,                 0, 0, 0),
   JSE_FN("isBlankNode",               XMLNode_isBlankNode,               0, 0, 0),
   JSE_FN("isTextNode",                XMLNode_isTextNode,                0, 0, 0),
   JSE_FN("getSpacePreserve",          XMLNode_getSpacePreserve,          0, 0, 0),
   JSE_FN("getFirstChild",             XMLNode_getFirstChild,             0, 0, 0),
   JSE_FN("getLastChild",              XMLNode_getLastChild,              0, 0, 0),
   JSE_FN("getLastElementChild",       XMLNode_getLastElementChild,       0, 0, 0),
   JSE_FN("getPreviousElementSibling", XMLNode_getPreviousElementSibling, 0, 0, 0),
   JSE_FN("getNext",                   XMLNode_getNext,                   0, 0, 0),
   JSE_FN("getPrev",                   XMLNode_getPrev,                   0, 0, 0),
   JSE_FN("getParent",                 XMLNode_getParent,                 0, 0, 0),
   JSE_FN("getFirstAttribute",         XMLNode_getFirstAttribute,         0, 0, 0),
   JSE_FN("getChildNodes",             XMLNode_getChildNodes,             0, 0, 0),
   JSE_FN("getAttributes",             XMLNode_getAttributes,             0, 0, 0),
   JSE_FN("hasProp",                   XMLNode_hasProp,                   1, 0, 0),
   JS_FS_END
};

JSObject *PrivateXMLNode::childNodeArray(JSContext *cx, xmlNodePtr node)
{
   JSObject *newArray = nullptr;

   try
   {
      JSObject *newArray = AssertJSNewArrayObject(cx, 0, nullptr);
      AutoNamedRoot anr(cx, newArray, "NewChildNodeArray");

      jsint count = 0;
      xmlNodePtr child = node->children;
      while(child)
      {
         JSObject *nodeObj = AssertJSNewObject(cx, &xmlnode_class, nullptr, nullptr);
         AutoNamedRoot cr(cx, nodeObj, "NewChildNode");

         std::unique_ptr<PrivateXMLNode> newNode(new PrivateXMLNode(child));
         AssertJSDefineFunctions(cx, nodeObj, xmlnodeJSMethods);
         AssertJSDefineProperty(cx, nodeObj, "document", OBJECT_TO_JSVAL(getDocObj()), nullptr, nullptr, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
         newNode->setDocObj(getDocObj());
         newNode->setToJSObjectAndRelease(cx, nodeObj, newNode);

         jsval v = OBJECT_TO_JSVAL(nodeObj);
         AssertJSSetElement(cx, newArray, count, &v);

         ++count;
         child = child->next;
      }
   }
   catch(const JSEngineError &)
   {
      newArray = nullptr;
   }

   return newArray;
}

JSObject *PrivateXMLNode::childAttribArray(JSContext *cx, xmlNodePtr node)
{
   JSObject *newArray = nullptr;
   try
   {
      newArray = AssertJSNewArrayObject(cx, 0, nullptr);
      AutoNamedRoot anr(cx, newArray, "NewChildAttribArray");

      jsint count = 0;
      xmlAttrPtr attr = node->properties;
      while(attr)
      {
         JSObject *attrObj = AssertJSNewObject(cx, &xmlattr_class, nullptr, nullptr);
         AutoNamedRoot cr(cx, attrObj, "NewChildAttr");

         std::unique_ptr<PrivateXMLAttr> newAttr(new PrivateXMLAttr(attr));
         AssertJSDefineFunctions(cx, attrObj, xmlattrJSMethods);
         AssertJSDefineProperty(cx, attrObj, "document", OBJECT_TO_JSVAL(getDocObj()), nullptr, nullptr, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
         newAttr->setDocObj(getDocObj());
         newAttr->setToJSObjectAndRelease(cx, attrObj, newAttr);

         jsval v = OBJECT_TO_JSVAL(attrObj);
         AssertJSSetElement(cx, newArray, count, &v);

         ++count;
         attr = attr->next;
      }
   }
   catch(const JSEngineError &)
   {
      newArray = nullptr;
   }

   return newArray;
}

// Create a new XMLNode JS Object and set it as the return value. If the creation
// fails, out-of-memory is reported and the PrivateXMLNode is destroyed.
static JSBool XMLNode_ReturnObject(JSContext *cx, jsval *vp, JSObject *docObj, PrivateXMLNode *node)
{
   try
   {
      JSObject *newObj = AssertJSNewObject(cx, &xmlnode_class, nullptr, nullptr);
      AutoNamedRoot anr(cx, newObj, "NewXMLNode");
      AssertJSDefineFunctions(cx, newObj, xmlnodeJSMethods);
      AssertJSDefineProperty(cx, newObj, "document", OBJECT_TO_JSVAL(docObj), nullptr, nullptr, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
      node->setToJSObject(cx, newObj);
      node->setDocObj(docObj);
      JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newObj));
      return JS_TRUE;
   }
   catch(const JSEngineError &err)
   {
      delete node;
      return err.propagateToJS(cx);
   }
}

static NativeInitCode XMLNode_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &xmlnode_class, nullptr,
                           0, nullptr, xmlnodeJSMethods, nullptr, nullptr);

   if(obj)
      AddEnumerationProperties(cx, obj, xmlElementTypeEnumValues);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native xmlNodeGlobalNative("XMLNode", XMLNode_Create);

//=============================================================================
//
// XMLXPathObject
//

// Finalizer
static void XMLXPathObject_Finalize(JSContext *cx, JSObject *obj)
{
   auto xobj = PrivateData::GetFromJSObject<PrivateXMLXPathObj>(cx, obj);

   if(xobj)
   {
      delete xobj;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

static JSClass xmlxobj_class =
{
   "XMLXPathObject",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   XMLXPathObject_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivateXMLXPathObj, xmlxobj_class)

//
// XMLXPathObject Methods
//

static JSBool XMLXPathObject_hasNodeSet(JSContext *cx, uintN argc, jsval *vp)
{
   auto xobj = PrivateData::GetFromThis<PrivateXMLXPathObj>(cx, vp);
   if(!xobj)
   {
      JS_ReportError(cx, "Invalid XMLXPathObject instance");
      return JS_FALSE;
   }
   JS_SET_RVAL(cx, vp, xobj->hasNodeSet() ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool XMLXPathObject_nodeSetSize(JSContext *cx, uintN argc, jsval *vp)
{
   auto xobj = PrivateData::GetFromThis<PrivateXMLXPathObj>(cx, vp);
   if(!xobj)
   {
      JS_ReportError(cx, "Invalid XMLXPathObject instance");
      return JS_FALSE;
   }
   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(xobj->nodeSetSize()));
   return JS_TRUE;
}

static JSBool XMLXPathObject_getNodeAt(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto xobj = PrivateData::MustGetFromThis<PrivateXMLXPathObj>(cx, vp);
   PrivateXMLNode *node;
   ASSERT_ARGC_GE(argc, 1, "getNodeAt");

   int32 idx = 0;
   JS_ValueToECMAInt32(cx, argv[0], &idx);

   int maxIdx = xobj->nodeSetSize();
   if(idx < 0 || idx >= maxIdx)
   {
      JS_SET_RVAL(cx, vp, JSVAL_VOID); // undefined
      return JS_TRUE;
   }
   if(!(node = PrivateXMLNode::FromGenericNode(xobj->getNodeAt(idx))))
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }

   return XMLNode_ReturnObject(cx, vp, xobj->getDocObj(), node);
}

static JSBool XMLXPathObject_getNodes(JSContext *cx, uintN argc, jsval *vp)
{
   auto xobj = PrivateData::MustGetFromThis<PrivateXMLXPathObj>(cx, vp);
   int numNodes = xobj->nodeSetSize();

   JSObject *newArray = AssertJSNewArrayObject(cx, numNodes, nullptr);
   AutoNamedRoot anr(cx, newArray, "NewXPathArray");

   for(int i = 0; i < numNodes; i++)
   {
      JSObject *subObj = AssertJSNewObject(cx, &xmlnode_class, nullptr, nullptr);
      AutoNamedRoot cr(cx, subObj, "NewXMLArrayNode");

      std::unique_ptr<PrivateXMLNode> node(new PrivateXMLNode(xobj->getNodeAt(i)));
      AssertJSDefineFunctions(cx, subObj, xmlnodeJSMethods);
      AssertJSDefineProperty(cx, subObj, "document", OBJECT_TO_JSVAL(xobj->getDocObj()), nullptr, nullptr, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
      node->setDocObj(xobj->getDocObj());
      node->setToJSObjectAndRelease(cx, subObj, node);

      jsval v = OBJECT_TO_JSVAL(subObj);
      AssertJSSetElement(cx, newArray, i, &v);
   }

   JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newArray));
   return JS_TRUE;
}

static JSFunctionSpec xmlxobjJSMethods[] =
{
   JSE_FN("hasNodeSet",  XMLXPathObject_hasNodeSet,  0, 0, 0),
   JSE_FN("nodeSetSize", XMLXPathObject_nodeSetSize, 0, 0, 0),
   JSE_FN("getNodeAt",   XMLXPathObject_getNodeAt,   1, 0, 0),
   JSE_FN("getNodes",    XMLXPathObject_getNodes,    0, 0, 0),
   JS_FS_END
};

//
// Properties
//

static JSBool XMLXPathObject_GetLength(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PrivateData::GetFromJSObject<PrivateXMLXPathObj>(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint jsLen = static_cast<jsint>(priv->nodeSetSize());
   *vp = INT_TO_JSVAL(jsLen);
   return JS_TRUE;
}

static JSPropertySpec xmlxobjProps[] =
{
   { 
      "length", 0, 
      JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED,
      XMLXPathObject_GetLength, nullptr
   },

   { nullptr }
};

// Create a new XMLXPathObject JS object and set it as the return value. If the creation
// fails, out-of-memory is reported and the PrivateXMLXPathObj is destroyed.
static JSBool XMLXPathObject_ReturnObject(JSContext *cx, jsval *vp, JSObject *docObj, PrivateXMLXPathObj *xobj)
{
   try
   {
      JSObject *newObj = AssertJSNewObject(cx, &xmlxobj_class, nullptr, nullptr);
      AutoNamedRoot anr(cx, newObj, "NewXMLXPathObj");
      AssertJSDefineFunctions(cx, newObj, xmlxobjJSMethods);
      AssertJSDefineProperties(cx, newObj, xmlxobjProps);
      AssertJSDefineProperty(cx, newObj, "document", OBJECT_TO_JSVAL(docObj), nullptr, nullptr, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
      xobj->setToJSObject(cx, newObj);
      xobj->setDocObj(docObj);
      JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newObj));
      return JS_TRUE;
   }
   catch(const JSEngineError &err)
   {
      delete xobj;
      return err.propagateToJS(cx);
   }
}

static NativeInitCode XMLXPathObject_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &xmlxobj_class, nullptr,
                           0, nullptr, xmlxobjJSMethods, nullptr, nullptr);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native xmlXPathObjectGlobalNative("XMLXPathObject", XMLXPathObject_Create);

//=============================================================================
//
// XMLDocument
//

// Finalizer
static void XMLDocument_Finalize(JSContext *cx, JSObject *obj)
{
   auto doc = PrivateData::GetFromJSObject<PrivateXMLDocument>(cx, obj);

   if(doc)
   {
      delete doc;
      JS_SetPrivate(cx, obj, nullptr);
   }
}

static JSClass xmldoc_class =
{
   "XMLDocument",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   XMLDocument_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivateXMLDocument, xmldoc_class)

//
// XMLDocument Methods
//

// Test if document is empty
static JSBool XMLDocument_isEmpty(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   auto doc = PrivateData::GetFromThis<PrivateXMLDocument>(cx, vp);
   if(!doc)
   {
      JS_ReportError(cx, "Invalid XMLDocument instance");
      return JS_FALSE;
   }

   JS_SET_RVAL(cx, vp, doc->isEmpty() ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

// Get document root node
static JSBool XMLDocument_getRootNode(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   PrivateXMLDocument *doc;
   PrivateXMLNode     *node;

   if(!(doc = PrivateData::GetFromThis<PrivateXMLDocument>(cx, vp)))
   {
      JS_ReportError(cx, "Invalid XMLDocument instance");
      return JS_FALSE;
   }
   if(!(node = PrivateXMLNode::FromDocRoot(doc->getDoc())))
   {
      JS_SET_RVAL(cx, vp, JSVAL_NULL);
      return JS_TRUE;
   }

   return XMLNode_ReturnObject(cx, vp, JS_THIS_OBJECT(cx, vp), node);
}

static JSBool XMLDocument_getNodeListString(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   bool inLine = false;

   auto doc = PrivateData::MustGetFromThis<PrivateXMLDocument>(cx, vp);
   ASSERT_ARGC_GE(argc, 1, "getNodeListString");
   AssertInstanceOf(cx, &xmlnode_class, argv[0]);
   auto node = PrivateData::MustGetFromJSObject<PrivateXMLNode>(cx, JSVAL_TO_OBJECT(argv[0]));

   if(argc >= 2)
   {
      JSBool foo = JS_FALSE;
      JS_ValueToBoolean(cx, argv[1], &foo);
      inLine = !!foo;
   }

   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(doc->getNodeListString(cx, node->getNode(), inLine)));
   return JS_TRUE;
}

static JSBool XMLDocument_evalXPathExpression(JSContext *cx, uintN argc, jsval *vp)
{   
   jsval *argv = JS_ARGV(cx, vp);
   PrivateXMLDocument *doc;
   std::map<std::string, std::string> nsMap;

   doc = PrivateData::MustGetFromThis<PrivateXMLDocument>(cx, vp);
   ASSERT_ARGC_GE(argc, 1, "evalXPathExpression");

   AutoJSValueToStringRooted jstr(cx, argv[0]);

   if(argc >= 2 && JSVAL_IS_OBJECT(argv[1]))
      AssertJSObjectToStringMap(cx, JSVAL_TO_OBJECT(argv[1]), nsMap);

   AutoXMLStr expr(XMLCharsFromJSString(cx, jstr));
   xmlChar *exprStr = expr.get();

   JSObject *docObj = JS_THIS_OBJECT(cx, vp);
   return XMLXPathObject_ReturnObject(cx, vp, docObj, PrivateXMLXPathObj::FromExpression(doc->getDoc(), exprStr, nsMap));
}

static JSBool XMLDocument_saveFile(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   PrivateXMLDocument *doc;

   doc = PrivateData::MustGetFromThis<PrivateXMLDocument>(cx, vp);
   ASSERT_ARGC_GE(argc, 1, "saveFile");

   const char *fn = SafeGetStringBytes(cx, argv[0], &argv[0]);
   int ret = doc->saveFile(fn);

   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ret));
   return JS_TRUE;
}

static JSBool XMLDocument_saveFileEnc(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   PrivateXMLDocument *doc;

   doc = PrivateData::MustGetFromThis<PrivateXMLDocument>(cx, vp);
   ASSERT_ARGC_GE(argc, 2, "saveFileEnc");

   const char *fn  = SafeGetStringBytes(cx, argv[0], &argv[0]);
   const char *enc = SafeGetStringBytes(cx, argv[1], &argv[1]);
   int ret = doc->saveFileEnc(fn, enc);

   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ret));
   return JS_TRUE;
}

static JSBool XMLDocument_saveFormatFile(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   
   auto doc = PrivateData::MustGetFromThis<PrivateXMLDocument>(cx, vp);
   ASSERT_ARGC_GE(argc, 2, "saveFormatFile");

   const char *fn = SafeGetStringBytes(cx, argv[0], &argv[0]);
   int32 format = 0;
   JS_ValueToECMAInt32(cx, argv[1], &format);
   int ret = doc->saveFormatFile(fn, format);

   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ret));
   return JS_TRUE;
}

static JSBool XMLDocument_saveFormatFileEnc(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   auto doc = PrivateData::MustGetFromThis<PrivateXMLDocument>(cx, vp);
   ASSERT_ARGC_GE(argc, 3, "saveFormatFileEnc");

   const char *fn  = SafeGetStringBytes(cx, argv[0], &argv[0]);
   const char *enc = SafeGetStringBytes(cx, argv[1], &argv[1]);
   int32 format = 0;
   JS_ValueToECMAInt32(cx, argv[2], &format);
   int ret = doc->saveFormatFileEnc(fn, enc, format);

   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(ret));
   return JS_TRUE;
}

static JSFunctionSpec xmldocJSMethods[] =
{
   JSE_FN("isEmpty",             XMLDocument_isEmpty,             0, 0, 0),
   JSE_FN("getRootNode",         XMLDocument_getRootNode,         0, 0, 0),
   JSE_FN("getNodeListString",   XMLDocument_getNodeListString,   1, 0, 0),
   JSE_FN("evalXPathExpression", XMLDocument_evalXPathExpression, 1, 0, 0),
   JSE_FN("saveFile",            XMLDocument_saveFile,            1, 0, 0),
   JSE_FN("saveFileEnc",         XMLDocument_saveFileEnc,         2, 0, 0),
   JSE_FN("saveFormatFile",      XMLDocument_saveFormatFile,      2, 0, 0),
   JSE_FN("saveFormatFileEnc",   XMLDocument_saveFormatFileEnc,   3, 0, 0),
   JS_FS_END
};

//
// XMLDocument Statics
//

// Create a new XMLDocument JS Object and set it as the return value. If the creation
// fails, out-of-memory is reported and the PrivateXMLDocument is destroyed.
static JSBool XMLDocument_ReturnObject(JSContext *cx, jsval *vp, PrivateXMLDocument *doc)
{
   try
   {
      JSObject *newObj = AssertJSNewObject(cx, &xmldoc_class, nullptr, nullptr);
      AutoNamedRoot anr(cx, newObj, "NewXMLDoc");
      AssertJSDefineFunctions(cx, newObj, xmldocJSMethods);
      doc->setToJSObject(cx, newObj);
      JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newObj));
      return JS_TRUE;
   }
   catch(const JSEngineError &err)
   {
      delete doc;
      return err.propagateToJS(cx);
   }
}

// File reading factory
static JSBool XMLDocument_FromFile(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "XMLDocument.FromFile");

   const char *fn = SafeGetStringBytes(cx, argv[0], &argv[0]);
   return XMLDocument_ReturnObject(cx, vp, PrivateXMLDocument::FromFile(fn));
}

// File w/encoding and options
static JSBool XMLDocument_FromFileOpt(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 3, "XMLDocument.FromFileOpt");

   const char *fn  = SafeGetStringBytes(cx, argv[0], &argv[0]);
   const char *enc = SafeGetStringBytes(cx, argv[1], &argv[1]);
   int32 opt = 0;
   JS_ValueToECMAInt32(cx, argv[2], &opt);

   return XMLDocument_ReturnObject(cx, vp, PrivateXMLDocument::FromFileOpt(fn, enc, opt));
}

// String-reading factory
static JSBool XMLDocument_FromString(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   
   ASSERT_ARGC_GE(argc, 1, "XMLDocument.FromString");
   AutoJSValueToStringRooted jstr(cx, argv[0]);
   AutoXMLStr chars(XMLCharsFromJSString(cx, jstr));
   return XMLDocument_ReturnObject(cx, vp, PrivateXMLDocument::FromXMLChars(chars.get()));
}

// From xmlChar memory
static JSBool XMLDocument_FromCharMem(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   
   ASSERT_ARGC_GE(argc, 4, "XMLDocument.FromCharMem");
   AutoJSValueToStringRooted jstr(cx, argv[0]);

   const char *URL = SafeGetStringBytes(cx, argv[1], &argv[1]);
   const char *enc = SafeGetStringBytes(cx, argv[2], &argv[2]);
   int32 opt = 0;
   JS_ValueToECMAInt32(cx, argv[3], &opt);

   AutoXMLStr chars(XMLCharsFromJSString(cx, jstr));
   return XMLDocument_ReturnObject(cx, vp, PrivateXMLDocument::FromXMLCharMem(chars.get(), URL, enc, opt));
}

static JSBool XMLDocument_FromMemory(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   
   ASSERT_ARGC_GE(argc, 4, "XMLDocument.FromMemory");
   AssertInstanceOf(cx, NativeByteBuffer::GetJSClass(), argv[0]);
   NativeByteBuffer *nbb = PrivateData::MustGetFromJSObject<NativeByteBuffer>(cx, JSVAL_TO_OBJECT(argv[0]));

   std::string nbbstr;
   nbb->toString(nbbstr);
   const char *URL = SafeGetStringBytes(cx, argv[1], &argv[1]);
   const char *enc = SafeGetStringBytes(cx, argv[2], &argv[2]);
   int32 opt = 0;
   JS_ValueToECMAInt32(cx, argv[3], &opt);
   return XMLDocument_ReturnObject(cx, vp, PrivateXMLDocument::FromMemory(nbbstr.c_str(), nbbstr.length(), URL, enc, opt));
}

// HTML file reading factory
static JSBool XMLDocument_FromHTMLFile(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 1, "XMLDocument.FromHTMLFile");

   const char *fn  = SafeGetStringBytes(cx, argv[0], &argv[0]);
   const char *enc = nullptr;
   if(argc >= 2)
      enc = SafeGetStringBytes(cx, argv[1], &argv[1]);

   return XMLDocument_ReturnObject(cx, vp, PrivateXMLDocument::FromHTMLFile(fn, enc));
}

// HTML file w/encoding and options
static JSBool XMLDocument_FromHTMLFileOpt(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   ASSERT_ARGC_GE(argc, 3, "XMLDocument.FromHTMLFileOpt");

   const char *fn  = SafeGetStringBytes(cx, argv[0], &argv[0]);
   const char *enc = SafeGetStringBytes(cx, argv[1], &argv[1]);
   int32 opt = 0;
   JS_ValueToECMAInt32(cx, argv[2], &opt);
   return XMLDocument_ReturnObject(cx, vp, PrivateXMLDocument::FromHTMLFileOpt(fn, enc, opt));
}

// HTML string-reading factory
static JSBool XMLDocument_FromHTMLString(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   
   ASSERT_ARGC_GE(argc, 1, "XMLDocument.FromHTMLString");
   AutoJSValueToStringRooted jstr(cx, argv[0]);

   AutoXMLStr chars(XMLCharsFromJSString(cx, jstr));
   const char *enc = nullptr;
   if(argc >= 2)
      enc = SafeGetStringBytes(cx, argv[1], &argv[1]);

   return XMLDocument_ReturnObject(cx, vp, PrivateXMLDocument::FromHTMLString(chars.get(), enc));
}

// HTML from xmlChar memory
static JSBool XMLDocument_FromHTMLStringOpt(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   
   ASSERT_ARGC_GE(argc, 4, "XMLDocument.FromHTMLStringOpt");
   AutoJSValueToStringRooted jstr(cx, argv[0]);

   const char *URL = SafeGetStringBytes(cx, argv[1], &argv[1]);
   const char *enc = SafeGetStringBytes(cx, argv[2], &argv[2]);
   int32 opt = 0;
   JS_ValueToECMAInt32(cx, argv[3], &opt);

   AutoXMLStr chars(XMLCharsFromJSString(cx, jstr));
   return XMLDocument_ReturnObject(cx, vp, PrivateXMLDocument::FromHTMLStringOpt(chars.get(), URL, enc, opt));
}

static JSFunctionSpec xmldocStatics[] =
{
   JSE_FN("FromFile",          XMLDocument_FromFile,          1, 0, 0),
   JSE_FN("FromFileOpt",       XMLDocument_FromFileOpt,       3, 0, 0),
   JSE_FN("FromString",        XMLDocument_FromString,        1, 0, 0),
   JSE_FN("FromCharMem",       XMLDocument_FromCharMem,       4, 0, 0),
   JSE_FN("FromMemory",        XMLDocument_FromMemory,        4, 0, 0),
   JSE_FN("FromHTMLFile",      XMLDocument_FromHTMLFile,      1, 0, 0),
   JSE_FN("FromHTMLFileOpt",   XMLDocument_FromHTMLFileOpt,   3, 0, 0),
   JSE_FN("FromHTMLString",    XMLDocument_FromHTMLString,    1, 0, 0),
   JSE_FN("FromHTMLStringOpt", XMLDocument_FromHTMLStringOpt, 4, 0, 0),
   JS_FS_END
};

static NativeInitCode XMLDocument_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_InitClass(cx, global, nullptr, &xmldoc_class, nullptr,
                           0, nullptr, xmldocJSMethods, nullptr, xmldocStatics);

   return obj ? RESOLVED : RESOLUTIONERROR;
}

static Native xmlDocumentGlobalNative("XMLDocument", XMLDocument_Create);

#endif VIBC_NO_LIBXML2

// EOF

