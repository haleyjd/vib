/*

  JS Bindings for the Simple DirectMedia Library and satellites.

*/

#ifndef VIBC_NO_SDL

#include "SDL.h"
#include "SDL_opengl.h"
#include "main.h"
#include "jsengine2.h"
#include "jsnatives.h"

//=============================================================================
//
// SDL Utilities
//

static bool sdlLoaded = false;

//
// ShutdownSDL
//
static void ShutdownSDL()
{
   if(sdlLoaded)
      SDL_Quit();
   sdlLoaded = false;
}

//
// InitSDL
//
// Will initialize the SDL library the first time it is called. Returns true
// or false to indicate success or failure to initialize.
//
static bool InitSDL()
{
   if(!sdlLoaded)
   {
      if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_NOPARACHUTE) == -1)
         return false;
      sdlLoaded = true;
      new ShutdownAction(ShutdownSDL);

      // Enable Unicode keysym translation
      SDL_EnableUNICODE(1);
   }

   return true;
}

static void AssertInitSDL()
{
   if(!InitSDL())
      throw JSEngineError("Could not initialize SDL");
}

//=============================================================================
//
// OpenGL Video
//

// Surface returned from SDL_SetVideoMode; not really useful for anything.
static SDL_Surface *surface;

struct vidmode_t
{
   bool wantfullscreen;
   bool wantvsync;
   bool wantframe;
   int  v_w;
   int  v_h;
   int  colordepth;
};

static bool InitGraphics(const vidmode_t &params)
{
   int flags = SDL_OPENGL;
   
   // make sure SDL is initialized
   if(!InitSDL())
      return false;

   if(params.wantfullscreen)
      flags |= SDL_FULLSCREEN;

   if(!params.wantframe)
      flags |= SDL_NOFRAME;

   // Set GL attributes through SDL
   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
   SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     params.colordepth >= 24 ? 8 : 5);
   SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   params.colordepth >= 24 ? 8 : 5);
   SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    params.colordepth >= 24 ? 8 : 5);
   SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   params.colordepth >= 32 ? 8 : 0);
   SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, params.wantvsync ? 1 : 0);
   
   // allow anti-aliasing
   SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
   SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  4);
   
   // Set GL video mode
   if(!(surface = SDL_SetVideoMode(params.v_w, params.v_h, params.colordepth, flags)))
      return false;

   // Wait for a bit so the screen can settle
   if(flags & SDL_FULLSCREEN)
      Sleep(500);

   // TODO: Load PBO extension?

   // Set initial viewport
   glViewport(0, 0, (GLsizei)params.v_w, (GLsizei)params.v_h);

   // TODO: other ops?

   return true;
}

//=============================================================================
//
// JS Bindings
//

static vidmode_t video;

//
// Video Object
//
// Provides a wrapper around the application window / rendering context.
//

static JSBool Video_OpenWindow(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);

   int32  w = 640, h = 480, colordepth = 32;
   JSBool fullscreen = JS_FALSE, vsync = JS_TRUE, frame = JS_TRUE;

   if(argc >= 1)
   {
      JS_ValueToECMAInt32(cx, argv[0], &w);
      if(argc >= 2)
      {
         JS_ValueToECMAInt32(cx, argv[1], &h);
         if(argc >= 3)
         {
            JS_ValueToECMAInt32(cx, argv[2], &colordepth);
            if(argc >= 4)
            {
               JS_ValueToBoolean(cx, argv[3], &fullscreen);
               if(argc >= 5)
               {
                  JS_ValueToBoolean(cx, argv[4], &vsync);
                  if(argc >= 6)
                     JS_ValueToBoolean(cx, argv[5], &frame);
               }
            }
         }
      }
   }

   video.v_w            = w;
   video.v_h            = h;
   video.colordepth     = colordepth;
   video.wantfullscreen = !!fullscreen;
   video.wantvsync      = !!vsync;
   video.wantframe      = !!frame;

   bool res = InitGraphics(video);

   JS_SET_RVAL(cx, vp, res ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSBool Video_SwapBuffers(JSContext *cx, uintN argc, jsval *vp)
{
   SDL_GL_SwapBuffers();

   return JS_TRUE;
}

static JSBool Video_SetCaption(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   const char *title = "vibconsole";
   const char *icon  = "vibconsole";

   if(argc >= 1)
   {
      title = icon = SafeGetStringBytes(cx, argv[0], &argv[0]);
      if(argc >= 2)
         icon = SafeGetStringBytes(cx, argv[1], &argv[1]);
   }

   if(InitSDL())
      SDL_WM_SetCaption(title, icon);

   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static unsigned int GL_MakeTextureDimension(unsigned int i)
{
   if(i)
   {
      --i;
      i |= i >> 1;
      i |= i >> 2;
      i |= i >> 4;
      i |= i >> 8;
      i |= i >> 16;
      ++i;
   }

   return i;
}

int curboundtexture;

// Framebuffer texture data
static unsigned int framebuffer_umax;
static unsigned int framebuffer_vmax;
static unsigned int texturesize;
static GLfloat      texcoord_smax;
static GLfloat      texcoord_tmax;
static GLuint       textureid;
static Uint32      *framebuffer;

static JSBool Video_CreateFramebufferTexture(JSContext *cx, uintN argc, jsval *vp)
{
   if(textureid) // already created one?
      return JS_TRUE;

   framebuffer_umax = GL_MakeTextureDimension((unsigned int)video.v_w);
   framebuffer_vmax = GL_MakeTextureDimension((unsigned int)video.v_h);
   texcoord_smax    = (GLfloat)video.v_w / framebuffer_umax;
   texcoord_tmax    = (GLfloat)video.v_h / framebuffer_vmax;

   glGenTextures(1, &textureid);

   texturesize = framebuffer_umax * framebuffer_vmax * 4;
   GLvoid *tempbuffer = calloc(framebuffer_umax * 4, framebuffer_vmax);
   glBindTexture(GL_TEXTURE_2D, textureid);
   curboundtexture = textureid;

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (GLsizei)framebuffer_umax,
                (GLsizei)framebuffer_vmax, 0, GL_BGRA, GL_UNSIGNED_BYTE, tempbuffer);
   free(tempbuffer);

   framebuffer = (Uint32 *)calloc(video.v_w * 4, video.v_h);

   return JS_TRUE;
}

static JSBool Video_PutFramebufferPixel(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   if(argc < 3 || !framebuffer)
      return JS_TRUE; // not enough parameters, or no framebuffer
   uint32 x, y, color;
   JS_ValueToECMAUint32(cx, argv[0], &x);
   JS_ValueToECMAUint32(cx, argv[1], &y);
   JS_ValueToECMAUint32(cx, argv[2], &color);

   if(x >= (uint32)video.v_w || y >= (uint32)video.v_h)
      return JS_TRUE; // bounds check

   framebuffer[y*video.v_w + x] = color;
   return JS_TRUE;
}

static JSBool Video_UploadFramebuffer(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   if(argc < 1 || !framebuffer)
      return JS_TRUE; // not enough parameters, or no framebuffer
   
   JSObject *arrayObj;
   if(JSVAL_IS_OBJECT(argv[0]) && JS_IsArrayObject(cx, (arrayObj = JSVAL_TO_OBJECT(argv[0]))))
   {
      jsuint arrayLen = 0;
      JS_GetArrayLength(cx, arrayObj, &arrayLen);

      for(jsuint i = 0; i < arrayLen && i < (jsuint)(video.v_w * video.v_h); i++)
      {
         jsval valAtIndex = JSVAL_VOID;
         if(!JS_LookupElement(cx, arrayObj, (jsint)i, &valAtIndex))
            break;

         uint32 intVal = 0;
         JS_ValueToECMAUint32(cx, valAtIndex, &intVal);
         framebuffer[i] = intVal;
      }
   }

   return JS_TRUE;
}

static JSBool Video_RenderFramebuffer(JSContext *cx, uintN argc, jsval *vp)
{
   if(!textureid || !framebuffer)
      return JS_TRUE; // no framebuffer

   if(curboundtexture != textureid)
      glBindTexture(GL_TEXTURE_2D, textureid);

   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (GLsizei)video.v_w, (GLsizei)video.v_h,
      GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)framebuffer);

   float x = 0.0f;
   float y = 0.0f;
   float w = (float)video.v_w;
   float h = (float)video.v_h;
   float smax = texcoord_smax;
   float tmax = texcoord_tmax;

   glBegin(GL_QUADS);
   glTexCoord2f(0.0f, 0.0f);
   glVertex2f(x, y);
   glTexCoord2f(0.0f, tmax);     
   glVertex2f(x, y + h);
   glTexCoord2f(smax, tmax);
   glVertex2f(x + w, y + h);
   glTexCoord2f(smax, 0.0f);
   glVertex2f(x + w, y);
   glEnd();

   return JS_TRUE;
}

static JSClass videoClass =
{
   "VideoClass",
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

static JSFunctionSpec videoJSMethods[] =
{
   JSE_FN("openWindow",  Video_OpenWindow,  0, 0, 0),
   JSE_FN("setCaption",  Video_SetCaption,  0, 0, 0),
   JSE_FN("swapBuffers", Video_SwapBuffers, 0, 0, 0),

   JSE_FN("createFramebufferTexture", Video_CreateFramebufferTexture, 0, 0, 0),
   JSE_FN("putFramebufferPixel",      Video_PutFramebufferPixel,      3, 0, 0),
   JSE_FN("uploadFramebuffer",        Video_UploadFramebuffer,        1, 0, 0),
   JSE_FN("renderFramebuffer",        Video_RenderFramebuffer,        0, 0, 0),

   JS_FS_END
};

static NativeInitCode Video_Create(JSContext *cx, JSObject *global)
{
   JSObject *obj;

   if(!(obj = JS_DefineObject(cx, global, "Video", &videoClass, nullptr, JSPROP_PERMANENT)))
      return RESOLUTIONERROR;

   if(!JS_DefineFunctions(cx, obj, videoJSMethods))
      return RESOLUTIONERROR;

   return RESOLVED;
}

static Native videoGlobalNative("Video", Video_Create);

//
// Timer
//

static JSBool Timer_Delay(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   int32 ms = 1;

   ASSERT_ARGC_GE(argc, 1, "delay");
   JS_ValueToECMAInt32(cx, argv[0], &ms);

   SDL_Delay(static_cast<Uint32>(ms));
   JS_SET_RVAL(cx, vp, JSVAL_VOID);
   return JS_TRUE;
}

static JSBool Timer_GetTicks(JSContext *cx, uintN argc, jsval *vp)
{
   Uint32 ticks = SDL_GetTicks();

   jsdouble *nd = JS_NewDouble(cx, ticks); // FIXME/TODO: auto root
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


//
// SDLEvent Object
//
// Wrapper around an SDL_Event C struct.
//

class PrivateEvent : public PrivateData
{
   DECLARE_PRIVATE_DATA()

public:
   SDL_Event evt;

   PrivateEvent() : PrivateData(), evt() {}
};

static void SDLEvent_Finalize(JSContext *cx, JSObject *obj);

static JSClass sdlEventClass =
{
   "SDLEvent",
   JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_PropertyStub,
   JS_EnumerateStub,
   JS_ResolveStub,
   JS_ConvertStub,
   SDLEvent_Finalize,
   JSCLASS_NO_OPTIONAL_MEMBERS
};

DEFINE_PRIVATE_DATA(PrivateEvent, sdlEventClass)

static JSBool SDLEvent_New(JSContext *cx, JSObject *obj, uintN argc,
                           jsval *argv, jsval *rval)
{
   if(!JS_IsConstructing(cx))
   {
      // TODO: construct and return a new instance of SDLEvent
      return JS_TRUE;
   }
   else
   {
      std::unique_ptr<PrivateEvent> pe(new PrivateEvent());
      pe->setToJSObjectAndRelease(cx, obj, pe);
      *rval = JSVAL_VOID;
      return JS_TRUE;
   }
}

static void SDLEvent_Finalize(JSContext *cx, JSObject *obj)
{
   auto priv = PrivateData::GetFromJSObject<PrivateEvent>(cx, obj);
   if(priv)
      delete priv;
}

static JSBool SDLEvent_GetType(JSContext *cx, JSObject *obj, jsval idval, jsval *vp)
{
   auto priv = PrivateData::GetFromJSObject<PrivateEvent>(cx, obj);
   if(!priv)
      return JS_FALSE;

   jsint val = static_cast<jsint>(priv->evt.type);
   *vp = INT_TO_JSVAL(val);
   return JS_TRUE;
}

static JSPropertySpec sdlEventProps[] =
{
   {
      "type", 0,
      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED|JSPROP_READONLY, // FIXME: make read/write?
      SDLEvent_GetType, nullptr
   },

   { nullptr }
};

static JSClass sdlEventDataClass =
{
   "SDLEventData",
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

//
// NewKBEvent
//
// Add keyboard event properties to an SDL_Event object
//
static JSBool NewKBEvent(JSContext *cx, const SDL_Event &ev, JSObject *parent)
{
   JSObject *newObj = JS_DefineObject(cx, parent, "key", &sdlEventDataClass, nullptr,
                                      JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
   if(!newObj)
      return JS_FALSE;

   jsint type, state;

   type  = ev.key.type;
   state = ev.key.state;

   JS_DefineProperty(cx, newObj, "type",  INT_TO_JSVAL(type),  nullptr, nullptr, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
   JS_DefineProperty(cx, newObj, "state", INT_TO_JSVAL(state), nullptr, nullptr, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);

   JSObject *keysym = JS_DefineObject(cx, newObj, "keysym", &sdlEventDataClass, nullptr,
                                      JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);

   if(!keysym)
      return JS_FALSE;

   jsint scancode, sym, mod, unicode;
   scancode = ev.key.keysym.scancode;
   sym      = ev.key.keysym.sym;
   mod      = ev.key.keysym.mod;
   unicode  = ev.key.keysym.unicode;

   JS_DefineProperty(cx, keysym, "scancode", INT_TO_JSVAL(scancode), nullptr, nullptr, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
   JS_DefineProperty(cx, keysym, "sym",      INT_TO_JSVAL(sym),      nullptr, nullptr, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
   JS_DefineProperty(cx, keysym, "mod",      INT_TO_JSVAL(mod),      nullptr, nullptr, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);
   JS_DefineProperty(cx, keysym, "unicode",  INT_TO_JSVAL(unicode),  nullptr, nullptr, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);

   return JS_TRUE;
}

typedef JSBool (*ev_func_ptr)(JSContext *, const SDL_Event &, JSObject *);

struct ev_func_t
{
   Uint8 eventType;
   ev_func_ptr func;
};

static ev_func_t eventFuncs[] =
{
   { SDL_KEYDOWN, NewKBEvent },
   { SDL_KEYUP,   NewKBEvent },
   { SDL_NOEVENT, nullptr    } // must be last.
};

//
// NewSDLEvent
//
// Create a new SDLEvent JSObject.
//
static JSObject *NewSDLEvent(JSContext *cx, const SDL_Event &ev)
{
   std::unique_ptr<PrivateEvent> pe(new PrivateEvent());

   JSObject *newObj = AssertJSNewObject(cx, &sdlEventClass, nullptr, nullptr);
   AutoNamedRoot root(cx, newObj, "NewSDLEvent");

   // TODO: Add methods

   // Add properties
   AssertJSDefineProperties(cx, newObj, sdlEventProps);

   // Add sub-object properties depending on event type
   ev_func_t *evfn = eventFuncs;
   while(evfn->eventType != SDL_NOEVENT)
   {
      if(evfn->eventType == ev.type)
      {
         evfn->func(cx, ev, newObj);
         break;
      }
      ++evfn;
   }

   // Attach private data
   pe->evt = ev;
   pe->setToJSObjectAndRelease(cx, newObj, pe);

   return newObj;
}

//
// Events Object
//
// Provides a wrapper around the event queue.
//

//
// Events_WaitEvent
//
// Wait until an event is available.
//
static JSBool Events_WaitEvent(JSContext *cx, uintN argc, jsval *vp)
{
   SDL_Event evt;

   AssertInitSDL();
   SDL_WaitEvent(&evt);

   JSObject *newEvt = NewSDLEvent(cx, evt);
   JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newEvt));
   return JS_TRUE;
}

//
// Events_PollEvent
// 
// Get an event if one is available.
//
static JSBool Events_PollEvent(JSContext *cx, uintN argc, jsval *vp)
{
   SDL_Event evt;

   AssertInitSDL();

   int res = SDL_PollEvent(&evt);
   if(res)
   {
      JSObject *newEvt = NewSDLEvent(cx, evt);
      JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(newEvt));
   }
   else
      JS_SET_RVAL(cx, vp, JSVAL_NULL);

   return JS_TRUE;
}

//
// Events_EnableKeyRepeat
// 
// Enable keyboard key repeat.
//
static JSBool Events_EnableKeyRepeat(JSContext *cx, uintN argc, jsval *vp)
{
   jsval *argv = JS_ARGV(cx, vp);
   int32 delay = SDL_DEFAULT_REPEAT_DELAY, interval = SDL_DEFAULT_REPEAT_INTERVAL;

   if(argc >= 1)
   {
      JS_ValueToECMAInt32(cx, argv[0], &delay);
      if(argc >= 2)
         JS_ValueToECMAInt32(cx, argv[1], &interval);
   }

   int res = SDL_EnableKeyRepeat(delay, interval);

   JS_SET_RVAL(cx, vp, res == 0 ? JSVAL_TRUE : JSVAL_FALSE);
   return JS_TRUE;
}

static JSClass eventsClass =
{
   "EventsClass",
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

static JSFunctionSpec eventsJSMethods[] =
{
   JSE_FN("enableKeyRepeat", Events_EnableKeyRepeat, 0, 0, 0),
   JSE_FN("pollEvent",       Events_PollEvent,       0, 0, 0),
   JSE_FN("waitEvent",       Events_WaitEvent,       0, 0, 0),
   JS_FS_END
};

static NativeInitCode Events_Create(JSContext *cx, JSObject *global)
{
   JSObject *obj;

   if(!(obj = JS_DefineObject(cx, global, "Events", &eventsClass, nullptr, JSPROP_PERMANENT)))
      return RESOLUTIONERROR;

   if(!JS_DefineFunctions(cx, obj, eventsJSMethods))
      return RESOLUTIONERROR;

   return RESOLVED;
}

static Native eventsGlobalNative("Events", Events_Create);

//
// Enumerations
//

static EnumEntry sdlEnum[] =
{
   // Event Types
   { SDL_ACTIVEEVENT,     "ACTIVEEVENT"     },
   { SDL_MOUSEMOTION,     "MOUSEMOTION"     },
   { SDL_MOUSEBUTTONDOWN, "MOUSEBUTTONDOWN" },
   { SDL_MOUSEBUTTONUP,   "MOUSEBUTTONUP"   },
   { SDL_JOYAXISMOTION,   "JOYAXISMOTION"   },
   { SDL_JOYBALLMOTION,   "JOYBALLMOTION"   },
   { SDL_JOYHATMOTION,    "JOYHATMOTION"    },
   { SDL_JOYBUTTONDOWN,   "JOYBUTTONDOWN"   },
   { SDL_JOYBUTTONUP,     "JOYBUTTONUP"     },
   { SDL_VIDEORESIZE,     "VIDEORESIZE"     },
   { SDL_VIDEOEXPOSE,     "VIDEOEXPOSE"     },
   { SDL_QUIT,            "QUIT"            },
   { SDL_USEREVENT,       "USEREVENT"       },
   { SDL_SYSWMEVENT,      "SYSWMEVENT"      },

   // Key Types
   { SDL_KEYDOWN,         "KEYDOWN"         },
   { SDL_KEYUP,           "KEYUP"           },

   // Key States
   { SDL_PRESSED,         "PRESSED"         },
   { SDL_RELEASED,        "RELEASED"        },

   ENUMEND()
};

// methods
static JSBool JSSDL_Quit(JSContext *cx, uintN argc, jsval *vp)
{
   if(sdlLoaded)
      SDL_Quit();
   sdlLoaded = false;
   return JS_TRUE;
}

static JSFunctionSpec sdlFuncs[] =
{
   JSE_FN("quit", JSSDL_Quit, 0, 0, 0),
   JS_FS_END
};

static NativeInitCode SDLEnum_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_DefineObject(cx, global, "SDL", &enumClass, nullptr, 0);
   if(obj)
   {
      AddEnumerationProperties(cx, obj, sdlEnum);
      JS_DefineFunctions(cx, obj, sdlFuncs);
      return RESOLVED;
   }
   else
      return RESOLUTIONERROR;
}

static Native sdlEnumNative("SDL", SDLEnum_Create);

static EnumEntry sdlkEnum[] =
{
   // Keysyms (selected)
   { SDLK_BACKSPACE,     "BACKSPACE"     },
   { SDLK_TAB,           "TAB"           },
   { SDLK_RETURN,        "RETURN"        },
   { SDLK_PAUSE,         "PAUSE"         },
   { SDLK_ESCAPE,        "ESCAPE"        },
   { SDLK_SPACE,         "SPACE"         },
   { SDLK_EXCLAIM,       "EXCLAIM"       },
   { SDLK_QUOTEDBL,      "QUOTEDBL"      },
   { SDLK_HASH,          "HASH"          },
   { SDLK_DOLLAR,        "DOLLAR"        },
   { SDLK_AMPERSAND,     "AMPERSAND"     },
   { SDLK_QUOTE,         "QUOTE"         },
   { SDLK_LEFTPAREN,     "LEFTPAREN"     },
   { SDLK_RIGHTPAREN,    "RIGHTPAREN"    },
   { SDLK_ASTERISK,      "ASTERISK"      },
   { SDLK_PLUS,          "PLUS"          },
   { SDLK_COMMA,         "COMMA"         },
   { SDLK_MINUS,         "MINUS"         },
   { SDLK_PERIOD,        "PERIOD"        },
   { SDLK_SLASH,         "SLASH"         },
   { SDLK_0,             "ZERO"          },
   { SDLK_1,             "ONE"           },
   { SDLK_2,             "TWO"           },
   { SDLK_3,             "THREE"         },
   { SDLK_4,             "FOUR"          },
   { SDLK_5,             "FIVE"          },
   { SDLK_6,             "SIX"           },
   { SDLK_7,             "SEVEN"         },
   { SDLK_8,             "EIGHT"         },
   { SDLK_9,             "NINE"          },
   { SDLK_COLON,         "COLON"         },
   { SDLK_SEMICOLON,     "SEMICOLON"     },
   { SDLK_LESS,          "LESS"          },
   { SDLK_EQUALS,        "EQUALS"        },
   { SDLK_GREATER,       "GREATER"       },
   { SDLK_QUESTION,      "QUESTION"      },
   { SDLK_AT,            "AT"            },
   { SDLK_LEFTBRACKET,   "LEFTBRACKET"   },
   { SDLK_RIGHTBRACKET,  "RIGHTBRACKET"  },
   { SDLK_CARET,         "CARET"         },
   { SDLK_UNDERSCORE,    "UNDERSCORE"    },
   { SDLK_BACKQUOTE,     "BACKQUOTE"     },
   { SDLK_a,             "a"             },
   { SDLK_b,             "b"             },
   { SDLK_c,             "c"             },
   { SDLK_d,             "d"             },
   { SDLK_e,             "e"             },
   { SDLK_f,             "f"             },
   { SDLK_g,             "g"             },
   { SDLK_h,             "h"             },
   { SDLK_i,             "i"             },
   { SDLK_j,             "j"             },
   { SDLK_k,             "k"             },
   { SDLK_l,             "l"             },
   { SDLK_m,             "m"             },
   { SDLK_n,             "n"             },
   { SDLK_o,             "o"             },
   { SDLK_p,             "p"             },
   { SDLK_q,             "q"             },
   { SDLK_r,             "r"             },
   { SDLK_s,             "s"             },
   { SDLK_t,             "t"             },
   { SDLK_u,             "u"             },
   { SDLK_v,             "v"             },
   { SDLK_w,             "w"             },
   { SDLK_x,             "x"             },
   { SDLK_y,             "y"             },
   { SDLK_z,             "z"             },
   { SDLK_DELETE,        "DELETE"        },
   // TODO: KP
   { SDLK_UP,            "UP"            },
   { SDLK_DOWN,          "DOWN"          },
   { SDLK_RIGHT,         "RIGHT"         },
   { SDLK_LEFT,          "LEFT"          },
   { SDLK_INSERT,        "INSERT"        },
   { SDLK_HOME,          "HOME"          },
   { SDLK_END,           "END"           },
   { SDLK_PAGEUP,        "PAGEUP"        },
   { SDLK_PAGEDOWN,      "PAGEDOWN"      },
   // TODO: F keys etc.

   ENUMEND()
};

static NativeInitCode SDLKeyEnum_Create(JSContext *cx, JSObject *global)
{
   auto obj = JS_DefineObject(cx, global, "SDLKey", &enumClass, nullptr, 0);
   if(obj)
   {
      AddEnumerationProperties(cx, obj, sdlkEnum);
      return RESOLVED;
   }
   else
      return RESOLUTIONERROR;
}

static Native sdlKeyEnumNative("SDLKey", SDLKeyEnum_Create);

#endif // VIBC_NO_SDL

// EOF

