/*
   JS OpenGL Bindings
*/

#ifndef VIBC_NO_SDL

#include "SDL.h"
#include "SDL_opengl.h"
#include "jsengine2.h"
#include "jsnatives.h"

//
// Methods
//

//
// JSGL_Begin
//
// Wraps glBegin()
//
static JSBool JSGL_Begin(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   GLenum mode = GL_POINTS;

   if(argc >= 1)
   {
      int32 mode_int = GL_POINTS;
      JS_ValueToECMAInt32(cx, argv[0], &mode_int);
      mode = static_cast<GLenum>(mode_int);
   }

   glBegin(mode);

   return JS_TRUE;
}

//
// JSGL_BindTexture
//
static JSBool JSGL_BindTexture(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 2)
   {
      JS_ReportError(cx, "Insufficient parameters to glBindBuffer");
      return JS_FALSE;
   }

   int32 target, texture;
   JS_ValueToECMAInt32(cx, argv[0], &target);
   JS_ValueToECMAInt32(cx, argv[0], &texture);

   glBindTexture(static_cast<GLenum>(target), static_cast<GLuint>(texture));
   return JS_TRUE;
}

//
// JSGL_BlendFunc
//
static JSBool JSGL_BlendFunc(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 2)
   {
      JS_ReportError(cx, "Insufficient arguments to glBlendFunc");
      return JS_FALSE;
   }

   int32 sfactor, dfactor;
   JS_ValueToECMAInt32(cx, argv[0], &sfactor);
   JS_ValueToECMAInt32(cx, argv[1], &dfactor);

   glBlendFunc(static_cast<GLenum>(sfactor), static_cast<GLenum>(dfactor));
   return JS_TRUE;
}

//
// JSGL_Clear
//
static JSBool JSGL_Clear(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 1)
   {
      JS_ReportError(cx, "Insufficient arguments to glClear");
      return JS_FALSE;
   }

   int32 buf;
   JS_ValueToECMAInt32(cx, argv[0], &buf);

   glClear(static_cast<GLbitfield>(buf));
   return JS_TRUE;
}

//
// JSGL_ClearColor
//
static JSBool JSGL_ClearColor(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 4)
   {
      JS_ReportError(cx, "Insufficient arguments to glClearColor");
      return JS_FALSE;
   }

   jsdouble r, g, b, a;
   JS_ValueToNumber(cx, argv[0], &r);
   JS_ValueToNumber(cx, argv[1], &g);
   JS_ValueToNumber(cx, argv[2], &b);
   JS_ValueToNumber(cx, argv[3], &a);

   glClearColor((GLclampf)r, (GLclampf)g, (GLclampf)b, (GLclampf)a);
   return JS_TRUE;
}

//
// JSGL_Color3d
//
static JSBool JSGL_Color3d(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 3)
   {
      JS_ReportError(cx, "Insufficient parameters to glColor3d");
      return JS_FALSE;
   }

   jsdouble r, g, b;
   JS_ValueToNumber(cx, argv[0], &r);
   JS_ValueToNumber(cx, argv[1], &g);
   JS_ValueToNumber(cx, argv[2], &b);

   glColor3d(r, g, b);
   return JS_TRUE;
}

//
// JSGL_Color4d
//
static JSBool JSGL_Color4d(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 4)
   {
      JS_ReportError(cx, "Insufficient parameters to glColor4d");
      return JS_FALSE;
   }

   jsdouble r, g, b, a;
   JS_ValueToNumber(cx, argv[0], &r);
   JS_ValueToNumber(cx, argv[1], &g);
   JS_ValueToNumber(cx, argv[2], &b);
   JS_ValueToNumber(cx, argv[3], &a);

   glColor4d(r, g, b, a);
   return JS_TRUE;
}

//
// JSGL_Disable
//
static JSBool JSGL_Disable(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   int32 en;

   if(argc < 1)
   {
      JS_ReportError(cx, "Insufficient arguments to glDisable");
      return JS_FALSE;
   }
   JS_ValueToECMAInt32(cx, argv[0], &en);

   glDisable(static_cast<GLenum>(en));
   return JS_TRUE;
}

//
// JSGL_Enable
//
static JSBool JSGL_Enable(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   int32 en;

   if(argc < 1)
   {
      JS_ReportError(cx, "Insufficient arguments to glEnable");
      return JS_FALSE;
   }
   JS_ValueToECMAInt32(cx, argv[0], &en);

   glEnable(static_cast<GLenum>(en));
   return JS_TRUE;
}

//
// JSGL_LoadIdentity
//
static JSBool JSGL_LoadIdentity(JSContext *cx, uintN argc, jsval *vp)
{
   glLoadIdentity();
   return JS_TRUE;
}

//
// JSGL_MatrixMode
//
static JSBool JSGL_MatrixMode(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   int32 mode;
   if(argc < 1)
   {
      JS_ReportError(cx, "Insufficient arguments to glMatrixMode");
      return JS_FALSE;
   }
   JS_ValueToECMAInt32(cx, argv[0], &mode);

   glMatrixMode(static_cast<GLenum>(mode));
   return JS_TRUE;
}

//
// JSGL_Ortho
//
static JSBool JSGL_Ortho(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 6)
   {
      JS_ReportError(cx, "Insufficient arguments to glOrtho");
      return JS_FALSE;
   }

   jsdouble l, r, b, t, n, f;
   JS_ValueToNumber(cx, argv[0], &l);
   JS_ValueToNumber(cx, argv[1], &r);
   JS_ValueToNumber(cx, argv[2], &b);
   JS_ValueToNumber(cx, argv[3], &t);
   JS_ValueToNumber(cx, argv[4], &n);
   JS_ValueToNumber(cx, argv[5], &f);

   glOrtho(l, r, b, t, n, f);
   return JS_TRUE;
}

//
// JSGL_PopMatrix
//
static JSBool JSGL_PopMatrix(JSContext *cx, uintN argc, jsval *vp)
{
   glPopMatrix();
   return JS_TRUE;
}

//
// JSGL_PushMatrix
//
static JSBool JSGL_PushMatrix(JSContext *cx, uintN argc, jsval *vp)
{
   glPushMatrix();
   return JS_TRUE;
}

//
// JSGL_Rotated
//
static JSBool JSGL_Rotated(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   if(argc < 4)
   {
      JS_ReportError(cx, "Insufficient arguments to glRotated");
      return JS_FALSE;
   }

   jsdouble angle, x, y, z;
   JS_ValueToNumber(cx, argv[0], &angle);
   JS_ValueToNumber(cx, argv[1], &x);
   JS_ValueToNumber(cx, argv[2], &y);
   JS_ValueToNumber(cx, argv[3], &z);

   glRotated(angle, x, y, z);
   return JS_TRUE;
}

//
// JSGL_TexParameteri
//
static JSBool JSGL_TexParameteri(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   if(argc < 3)
   {
      JS_ReportError(cx, "Insufficient arguments to glTexParameteri");
      return JS_FALSE;
   }
   int32 target, pname, param;
   JS_ValueToECMAInt32(cx, argv[0], &target);
   JS_ValueToECMAInt32(cx, argv[1], &pname);
   JS_ValueToECMAInt32(cx, argv[2], &param);

   glTexParameteri(target, pname, param);
   return JS_TRUE;
}

//
// JSGL_Translated
//
static JSBool JSGL_Translated(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   if(argc < 3)
   {
      JS_ReportError(cx, "Insufficient arguments to glTranslated");
      return JS_FALSE;
   }

   jsdouble x, y, z;
   JS_ValueToNumber(cx, argv[0], &x);
   JS_ValueToNumber(cx, argv[1], &y);
   JS_ValueToNumber(cx, argv[2], &z);

   glTranslated(x, y, z);
   return JS_TRUE;
}

//
// JSGL_Vertex2d
//
static JSBool JSGL_Vertex2d(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 2)
   {
      JS_ReportError(cx, "Insufficient arguments to glVertex2d");
      return JS_FALSE;
   }

   jsdouble x, y;
   JS_ValueToNumber(cx, argv[0], &x);
   JS_ValueToNumber(cx, argv[1], &y);

   glVertex2d(x, y);
   return JS_TRUE;
}

//
// JSGL_Vertex3d
//
static JSBool JSGL_Vertex3d(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 3)
   {
      JS_ReportError(cx, "Insufficient parameters to glVertex3d");
      return JS_FALSE;
   }

   if(!JSVAL_IS_DOUBLE(argv[0]) || !JSVAL_IS_DOUBLE(argv[1]) || !JSVAL_IS_DOUBLE(argv[2]))
   {
      JS_ReportError(cx, "Invalid paremeters to glVertex3d");
      return JS_FALSE;
   }

   jsdouble x, y, z;

   JS_ValueToNumber(cx, argv[0], &x);
   JS_ValueToNumber(cx, argv[1], &y);
   JS_ValueToNumber(cx, argv[2], &z);

   glVertex3d(x, y, z);
   return JS_TRUE;
}

//
// JSGL_Viewport
//
static JSBool JSGL_Viewport(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   if(argc < 4)
   {
      JS_ReportError(cx, "Insufficient arguments to glViewport");
      return JS_FALSE;
   }

   int32 x, y, w, h;
   JS_ValueToECMAInt32(cx, argv[0], &x);
   JS_ValueToECMAInt32(cx, argv[1], &y);
   JS_ValueToECMAInt32(cx, argv[2], &w);
   JS_ValueToECMAInt32(cx, argv[3], &h);

   glViewport(x, y, w, h);
   return JS_TRUE;
}

//
// JSGL_End
//
// Wraps glEnd()
//
static JSBool JSGL_End(JSContext *cx, uintN argc, jsval *vp)
{
   glEnd();
   return JS_TRUE;
}

//
// Enumerations
//

#define GLENUM(name) { GL_ ## name, #name }

static EnumEntry jsglEnumEntries[] =
{
   GLENUM(ACCUM_BUFFER_BIT),
   GLENUM(ALWAYS),
   GLENUM(BLEND),
   GLENUM(BGRA),
   GLENUM(CLAMP),
   GLENUM(CLAMP_TO_BORDER),
   GLENUM(CLAMP_TO_EDGE),
   GLENUM(COLOR),
   GLENUM(COLOR_BUFFER_BIT),
   GLENUM(CULL_FACE),
   GLENUM(DEPTH_BUFFER_BIT),
   GLENUM(DEPTH_TEST),
   GLENUM(EQUAL),
   GLENUM(GEQUAL),
   GLENUM(GREATER),
   GLENUM(LEQUAL),
   GLENUM(LESS),
   GLENUM(LINEAR),
   GLENUM(LINEAR_MIPMAP_LINEAR),
   GLENUM(LINEAR_MIPMAP_NEAREST),
   GLENUM(LINES),
   GLENUM(LINE_STRIP),
   GLENUM(MIRRORED_REPEAT),
   GLENUM(MODELVIEW),
   GLENUM(NEAREST),
   GLENUM(NEAREST_MIPMAP_LINEAR),
   GLENUM(NEAREST_MIPMAP_NEAREST),
   GLENUM(NEVER),
   GLENUM(NONE),
   GLENUM(NOTEQUAL),
   GLENUM(ONE),
   GLENUM(POINTS),
   GLENUM(POLYGON),
   GLENUM(POLYGON_SMOOTH),
   GLENUM(PROJECTION),
   GLENUM(QUADS),
   GLENUM(REPEAT),
   GLENUM(SRC_ALPHA_SATURATE),
   GLENUM(TEXTURE),
   GLENUM(TEXTURE_2D),
   GLENUM(TEXTURE_BASE_LEVEL),
   GLENUM(TEXTURE_BORDER_COLOR),
   GLENUM(TEXTURE_COMPARE_FUNC),
   GLENUM(TEXTURE_COMPARE_MODE),
   GLENUM(TEXTURE_LOD_BIAS),
   GLENUM(TEXTURE_MIN_FILTER),
   GLENUM(TEXTURE_MAG_FILTER),
   GLENUM(TEXTURE_MIN_LOD),
   GLENUM(TEXTURE_MAX_LOD),
   GLENUM(TEXTURE_MAX_LEVEL),
   GLENUM(TEXTURE_WRAP_S),
   GLENUM(TEXTURE_WRAP_T),
   GLENUM(TEXTURE_WRAP_R),
   GLENUM(TRIANGLES),
   GLENUM(TRIANGLE_STRIP),
   GLENUM(UNSIGNED_BYTE),
   ENUMEND()
};

//
// GL CLASS
//

static JSClass jsglClass = 
{
   "GLClass",
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

// Methods
static JSFunctionSpec jsglMethods[] =
{
   JSE_FN("Begin",         JSGL_Begin,         0, 0, 0),
   JSE_FN("BindTexture",   JSGL_BindTexture,   2, 0, 0),
   JSE_FN("BlendFunc",     JSGL_BlendFunc,     2, 0, 0),
   JSE_FN("Clear",         JSGL_Clear,         1, 0, 0),
   JSE_FN("ClearColor",    JSGL_ClearColor,    4, 0, 0),
   JSE_FN("Color3d",       JSGL_Color3d,       3, 0, 0),
   JSE_FN("Color4d",       JSGL_Color4d,       4, 0, 0),
   JSE_FN("Disable",       JSGL_Disable,       1, 0, 0),
   JSE_FN("Enable",        JSGL_Enable,        1, 0, 0),
   JSE_FN("End",           JSGL_End,           0, 0, 0),
   JSE_FN("LoadIdentity",  JSGL_LoadIdentity,  0, 0, 0),
   JSE_FN("MatrixMode",    JSGL_MatrixMode,    1, 0, 0),
   JSE_FN("Ortho",         JSGL_Ortho,         6, 0, 0),
   JSE_FN("PushMatrix",    JSGL_PushMatrix,    0, 0, 0),
   JSE_FN("Rotated",       JSGL_Rotated,       4, 0, 0),
   JSE_FN("TexParameteri", JSGL_TexParameteri, 3, 0, 0),
   JSE_FN("Translated",    JSGL_Translated,    3, 0, 0),
   JSE_FN("Vertex2d",      JSGL_Vertex2d,      2, 0, 0),
   JSE_FN("Vertex3d",      JSGL_Vertex3d,      3, 0, 0),
   JSE_FN("Viewport",      JSGL_Viewport,      4, 0, 0),
   JS_FS_END
};

static NativeInitCode JSGL_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_DefineObject(cx, global, "gl", &jsglClass, nullptr, 0);
   if(obj)
   {
      AddEnumerationProperties(cx, obj, jsglEnumEntries);

      JS_DefineFunctions(cx, obj, jsglMethods);

      return RESOLVED;
   }
   else
      return RESOLUTIONERROR;
}

static Native jsglNative("gl", JSGL_Create);

#endif // VIBC_NO_SDL

// EOF

