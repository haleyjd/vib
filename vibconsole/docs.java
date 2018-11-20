/* Connection to an ADO database */
class ADOReadOnlyDB
{
  bool open(string connectStr);
  bool close();
  ADORecordSet getRecordSet();
  ADOCommand getCommand(string sql, bool prepared = false);
  ADOCommand getCommandFile(string filename, bool prepared = false);
};

/* Command to execute against an ADO database */
class ADOCommand
{
  ADORecordSet execute();
};

/* ADO query results */
class ADORecordSet
{
  bool open(string query);
  bool close();
  bool atEOF();
  bool next();
  string getValue();
  LazyStringMap getMap();
  LazyVecMap getVecMap();
};

/* cURL HTTP connection file IO abstraction */
class CURLFile
{
  CURLFile(string filename = "", string mode = "r");
  
  bool open(string filename, string mode = "r");
  bool openPost(string filename, string postArgs = "");
  bool isOpen();
  bool close();
  bool eof();
  ByteBuffer read();
  string gets(int size);
  bool rewind();
};

/* Date, time, and timestamp enumeration flags */
class DateTimeFlagsEnum
{
  const int unknown       = 0;
  const int yy            = 0;
  const int yyyy          = 1;
  const int mdy           = 0;
  const int ymd           = 2;
  const int dmy           = 4;
  const int ampm          = 0;
  const int military      = 8;
  const int timefirst     = 0;
  const int datefirst     = 16;
  const int withsec       = 0;
  const int withoutsec    = 32;
  const int no_zeros      = 0;
  const int left_zeros    = 64;
  const int separators    = 0;
  const int no_separators = 128;
  const int withmillis    = 0;
  const int withoutmillis = 256;
  const int global_mode   = yyyy|mdy|military|datefirst;
};

/* Prometheus date class */
class Pdate
{
  int day;
  int month;
  int year;
  
  Pdate(Pdate other);
  Pdate(string date);
  Pdate();
  
  int calcAge(Pdate rhs = null);
  int dayOfWeek();
  void inDate(string incoming, int mode);
  void inDate(Object dateOrStamp);
  bool isValidDate();
  void mod(int dist, int metric);
  void mod(Object dateOrStamp);
  string nameOfDay();
  string nameOfMonth();
  void increment();
  void decrement();
  void addAssign(int amount);
  void subAssign(int amount);
  bool lessThan(Object dateOrStamp);
  bool lessOrEqual(Object dateOrStamp);
  string getTimeSince(Pdate date);
  bool greaterThan(Object dateOrStamp);
  bool greaterOrEqual(Object dateOrStamp);
  bool equalTo(Object dateOrStamp);
  bool notEqualTo(Object dateOrStamp);
  string outDate(int mode);
  string toString();
  void fromDate(Date jsDate);
};

/* Prometheus time class */
class Ptime
{
  int hour;
  int minute;
  int second;
  int millisecond;
  
  Ptime(Ptime other);
  Ptime(string time);
  Ptime();
  
  void inTime(string time, int mode);
  void inTime(Object timeOrStamp);
  bool isValidTime();
  void mod(int dist, int metric);
  void mod(Object timeOrStamp);
  void increment();
  void decrement();
  bool lessThan(Object timeOrStamp);
  bool lessOrEqual(Object timeOrStamp);
  string getTimeSince(Ptime time);
  bool greaterThan(Object timeOrStamp);
  bool greaterOrEqual(Object timeOrStamp);
  bool equalTo(Object timeOrStamp);
  bool notEqualTo(Object timeOrStamp);
  string outTime(int mode);
  string toString();
  void fromDate(Date jsDate);
};

/* Prometheus timestamp class */
class Pstamp
{
  int day;
  int month;
  int year;
  int hour;
  int minute;
  int second;
  int millisecond;
  
  Pstamp(Pstamp other);
  Pstamp(string stamp, int mode = DateTimeFlags.global_mode);
  Pstamp();
   
  int dayOfWeek();
  void inDate(string incoming, int mode);
  void inDate(Object dateOrStamp);
  bool isValidDate();
  void dateMod(int dist, int metric);
  void dateMod(Object dateOrStamp);
  string nameOfDay();
  string nameOfMonth();
  void dateIncrement();
  void dateDecrement();
  void dateAddAssign(int amount);
  void dateSubAssign(int amount);
  bool dateLessThan(Object dateOrStamp);
  bool dateLessOrEqual(Object dateOrStamp);
  bool dateGreaterThan(Object dateOrStamp);
  bool dateGreaterOrEqual(Object dateOrStamp);
  bool dateEqualTo(Object dateOrStamp);
  bool dateNotEqualTo(Object dateOrStamp);
  string outDate(int mode);
   
  void inTime(string time, int mode);
  void inTime(Object timeOrStamp);
  bool isValidTime();
  void timeMod(int dist, int metric);
  void timeMod(Object timeOrStamp);
  void timeIncrement();
  void timeDecrement();
  bool timeLessThan(Object timeOrStamp);
  bool timeLessOrEqual(Object timeOrStamp);
  bool timeGreaterThan(Object timeOrStamp);
  bool timeGreaterOrEqual(Object timeOrStamp);
  bool timeEqualTo(Object timeOrStamp);
  bool timeNotEqualTo(Object timeOrStamp);
  string outTime(int mode);
   
  bool isValidStamp();
  void inStamp(string stamp, int mode);
  void inStamp(Pstamp stamp);
  string getTimeSince(Pstamp stamp);
  string outStamp(int mode);
  void mod(int dist, int metric);
  void mod(Pstamp stamp);
  string toString();
  void fromDate(Date jsDate);
  string toJSDateString();
};

/* Global object for default console execution context */
class global
{
  ConsoleClass      Console;
  CoreClass         Core;
  DateTimeFlagsEnum DateTimeFlags;
  EventsClass       Events;
  GLClass           gl;
  IniFile           iniFile;
  SDLEnum           SDL;
  SDLKeyEnum        SDLKey;
  SQLOptionsEnum    SQLOptions;
  TimerClass        Timer;
  UtilsClass        Utils;
  VideoClass        Video;
  WinClass          Win;
  
  PrometheusLookupsClass PrometheusLookups;
};

/* Sandbox global object, created via Core.loadModule or Core.evalSandbox */
class sandbox
{
  ConsoleClass      Console;
  CoreClass         Core;
  DateTimeFlagsEnum DateTimeFlags;
  EventsClass       Events;
  GLClass           gl;
  IniFile           iniFile;
  SDLEnum           SDL;
  SDLKeyEnum        SDLKey;
  SQLOptionsEnum    SQLOptions;
  TimerClass        Timer;
  UtilsClass        Utils;
  VideoClass        Video;
  WinClass          Win;

  PrometheusLookupsClass PrometheusLookups;
};

/* Restricted global for execution of untrusted code. Created via
   Core.loadJSON or Core.evalUntrustedString */
class RestrictedContext
{
};

/* Holds natively implemented ECMA 5 extensions */
class Extensions
{
  /* Assigned to Object.defineProperty in autoexec.js */
  void objectDefineProperty(Object obj, string prop, Object descriptor);
};

/* OpenGL Bindings */
class GLClass
{
  const int ACCUM_BUFFER_BIT;
  const int ALWAYS;
  const int BLEND;
  const int BGRA;
  const int CLAMP;
  const int CLAMP_TO_BORDER;
  const int CLAMP_TO_EDGE;
  const int COLOR;
  const int COLOR_BUFFER_BIT;
  const int CULL_FACE;
  const int DEPTH_BUFFER_BIT;
  const int DEPTH_TEST;
  const int EQUAL;
  const int GEQUAL;
  const int GREATER;
  const int LEQUAL;
  const int LESS;
  const int LINEAR;
  const int LINEAR_MIPMAP_LINEAR;
  const int LINEAR_MIPMAP_NEAREST;
  const int LINES;
  const int LINE_STRIP;
  const int MIRRORED_REPEAT;
  const int MODELVIEW;
  const int NEAREST;
  const int NEAREST_MIPMAP_LINEAR;
  const int NEAREST_MIPMAP_NEAREST;
  const int NEVER;
  const int NONE;
  const int NOTEQUAL;
  const int ONE;
  const int POINTS;
  const int POLYGON;
  const int POLYGON_SMOOTH;
  const int PROJECTION;
  const int QUADS;
  const int REPEAT;
  const int SRC_ALPHA_SATURATE;
  const int TEXTURE;
  const int TEXTURE_2D;
  const int TEXTURE_BASE_LEVEL;
  const int TEXTURE_BORDER_COLOR;
  const int TEXTURE_COMPARE_FUNC;
  const int TEXTURE_COMPARE_MODE;
  const int TEXTURE_LOD_BIAS;
  const int TEXTURE_MIN_FILTER;
  const int TEXTURE_MAG_FILTER;
  const int TEXTURE_MAX_LEVEL;
  const int TEXTURE_WRAP_S;
  const int TEXTURE_WRAP_T;
  const int TEXTURE_WRAP_R;
  const int TRIANGLES;
  const int TRIANGLE_STRIP;
  const int UNSIGNED_BYTE;
  
  void Begin(int mode = POINTS);
  void BindTexture(int target, int texture);
  void BlendFunc(int sfactor, int dfactor);
  void Clear(int buf);
  void ClearColor(double r, double g, double b, double a);
  void Color3d(double r, double g, double b);
  void Color4d(double r, double g, double b, double a);
  void Disable(int en);
  void Enable(int en);
  void End();
  void LoadIdentity();
  void MatrixMode(int mode);
  void Ortho(double l, double r, double b, double t, double n, double f);
  void PopMatrix();
  void PushMatrix();
  void Rotated(double angle, double x, double y, double z);
  void TexParameteri(int target, int pname, int param);
  void Translated(double x, double y, double z);
  void Vertex2d(double x, double y);
  void Vertex3d(double x, double y, double z);
  void Viewport(double x, double y, double w, double h);
};

/* Core intepreter interface */
class CoreClass
{
  void GC();
  void maybeGC();
  Object loadScript(string filename);
  Object loadModule(string filename);
  Object loadMixin(string filename, Object obj);
  Object loadJSON(string filename);
  Object evalString(string str);
  Object evalUntrustedString(string str);
  Object evalSandbox(string str);
  void exit();
};

/* Console IO */
class ConsoleClass
{
  void print(Object str, ...);
  void println(Object str, ...);
  string getline();
  bool startEcho(string filename);
  bool stopEcho();
  void resize(int width, int height);
  void setTextAttrib(int attrib = 0x07);
  void fillOutputAttrib(int attrib = 0x07);
  void setTitle(string title = "vibconsole");
};

/* Native byte buffer object */
class ByteBuffer
{
  /*readonly*/ int size;
  
  ByteBuffer(ByteBuffer other);
  ByteBuffer(int size);
  
  bool resize(int newSize);
  string toString();
  string toUCString();
  void fromString(string str);
  void setBytesAt(int index, Array values);
  void setBytesAt(int index, int value);
};

/* Native file system interaction */
class File
{
  File(string filename, string mode = "r");
  
  int size();
  int puts(Object str);
  int read(ByteBuffer dest, int sizeToRead = 0);
  int write(ByteBuffer src, int sizeToWrite = 0);
  bool isOpen();
  int flush();
  void close();
};

/* Lazy-evaluating wrapper for C++ STL map<string, string> */
class LazyStringMap
{
  /* has arbitrary properties, get with operator [] */
  
  string toCommaString();
};

/* Lazy-evaluating wrapper for C++ STL vector<map<string, string>> */
class LazyVecMap
{
  /* has size() number of LazyStringMap instances keyed by index (use operator []) */
  
  int size();
  string toCSV();
  
  static LazyVecMap FromCSV(string filename);
};

/* Lazy-evaluating wrapper for C++ STL map<string, map<string, string>> */
class LazyMapStringMap
{
  /* has arbitrary LazyStringMap instances keyed by name (use operator []) */
};

/* Initialization file global singleton */
class IniFile
{
  LazyMapStringMap getIniOptions();
};

/* Connection to a Firebird database */
class PrometheusDB
{
  PrometheusDB();
  
  bool connect(string addr, string user, string pwd);
  bool connect(string iniName);
  bool isConnected();
  void disconnect();
  bool executeStatement(string sql);
  string getOneField(string sql);
  LazyVecMap sqlToVecMap(string sql);
};

/* Parameters to fieldOpts for insert and update statements */
class SQLOptionsEnum
{
  const int sql_full_processing;
  const int sql_no_uppercasing;
  const int sql_no_quoting;
  const int sql_no_left_trimming;
  const int sql_no_right_trimming;
  const int sql_no_trimming;
  const int sql_no_processing;
};

/* Transaction against a Firebird database */
class PrometheusTransaction
{
  PrometheusTransaction();
  
  bool isActive();
  bool commit();
  bool rollback();
  bool stdTransaction(PrometheusDB db);
  string getNextId(string tableName);
  bool executeStatement(string sql);
  bool executeInsertStatement(string tableName, Object fieldMap, Object fieldOpts);
  bool executeUpdateStatement(string tableName, Object fieldMap, Object fieldOpts);
  string getOneField(string sql);
  LazyVecMap sqlToVecMap(string sql);
  bool lockForUpdate(string tableName, string id);
  LazyStringMap sqlToMap(string sql);
  LazyStringMap getFullRecord(string tableName, string id);
};

/* Support for loading and resolving symbols from a lookup table
   stored in a Firebird database */
class PrometheusLookupsClass
{
  void LoadLookup(PrometheusDB db, string table);
  void LoadCustom(PrometheusDB db, string table, string sql);
  void PurgeLookups();
  string IdOf(string table, string value);
  string ValueOf(string table, string id);
  bool IsValidID(string table, string id);
  bool IsValidValue(string table, string value);
};

/* Prometheus utility function wrapper class */
class UtilsClass
{
  string AcronymString(string str);
  string CollapseSpaces(string str);
  string ConvertBinary(string str);
  string ConvertBool(string str);
  string ConvertTime(string str);
  string CurrentDate();
  string CurrentTime();
  string FormatAddress(string str);
  string FormatPhoneNumber(string str);
  string FormatZipCode(string str);
  string InitialsSearchString1(string str);
  string InitialsSearchString2(string str);
  bool IsBlankOrZero(string str);
  bool IsBracketed(string str);
  bool IsFloat(string str);
  bool IsIn(string str, ...);
  bool IsInt(string str);
  bool IsLeapYear(int year);
  int LastDayOf(int month, int year);
  bool LikeString(string regexp, string input);
  string LoadTextFile(string filename);
  string LowercaseFirstChar(string str);
  string LowercaseString(string str);
  string PowerSearchString(string str);
  string PrettyDate(string str);
  string PrettySql(string str);
  string PrettySSN(string str);
  string PrettyString(string str);
  string RightNow(bool compact);
  string SafeSQLString(string str, bool preserve_fieldname_case);
  string SentenceString(string str);
  string StripAll(string str, string chars);
  string StripAllExcept(string str, string chars);
  string StripFirst(string str);
  string StripLast(string str);
  string StripLeading(string str, string chars);
  string StripSurrounding(string str, string chars);
  string StripTrailing(string str, string chars);
  string TwoDecimals(string str);
  bool UppercaseCompare(string str1, string str2);
  string UppercaseFirstChar(string str);
  string UppercaseString(string str);
};

/* SDL screen / framebuffer wrapper (GL 2D-in-3D) */
class VideoClass
{
  bool openWindow(int w = 640, int h = 480, int bpp = 32, 
                  bool fullscreen = false, bool vsync = true,
                  bool frame = true);
  void setCaption(string title = "vibconsole", string icon = "vibconsole");
  void swapBuffers();
  
  void createFramebufferTexture();
  void putFramebufferPixel(int x, int y, int color);
  void uploadFramebuffer();
  void renderFramebuffer();
};

/* SDL Timer wrapper */
class TimerClass
{
  void delay(int ms);
  double getTicks();
};

/* SDL event data */
class SDLEventData
{
  /* properties depend on type of event */
};

/* SDL event object wrapper */
class SDLEvent
{
  /* has instance of SDLEventData w/name depending on type */
  const int type;
};

/* SDL event loop wrapper */
class EventsClass
{
  bool enableKeyRepeat(int delay = SDL_DEFAULT_REPEAT_DELAY,
                       int interval = SDL_DEFAULT_REPEAT_INTERVAL);
  SDLEvent pollEvent();
  SDLEvent waitEvent();
};

/* SDL enumeration values */
class SDLEnum
{
  const int ACTIVEEVENT;
  const int MOUSEMOTION;
  const int MOUSEBUTTONDOWN;
  const int MOUSEBUTTONUP;
  const int JOYAXISMOTION;
  const int JOYBALLMOTION;
  const int JOYHATMOTION;
  const int JOYBUTTONDOWN;
  const int JOYBUTTONUP;
  const int KEYDOWN;
  const int KEYUP;
  const int PRESSED;
  const int QUIT;
  const int RELEASED;
  const int SYSWMEVENT;
  const int USEREVENT;
  const int VIDEOEXPOSE;
  const int VIDEORESIZE;
};

/* SDLKey enumeration values */
class SDLKeyEnum
{
  const int BACKSPACE;
  const int TAB;
  const int RETURN;
  const int PAUSE;
  const int ESCAPE;
  const int SPACE;
  const int EXCLAIM;
  const int QUOTEDBL;
  const int HASH;
  const int DOLLAR;
  const int AMPERSAND;
  const int QUOTE;
  const int LEFTPAREN;
  const int RIGHTPAREN;
  const int ASTERISK;
  const int PLUS;
  const int COMMA;
  const int MINUS;
  const int PERIOD;
  const int SLASH;
  const int ZERO;
  const int ONE;
  const int TWO;
  const int THREE;
  const int FOUR;
  const int FIVE;
  const int SIX;
  const int SEVEN;
  const int EIGHT;
  const int NINE;
  const int COLON;
  const int SEMICOLON;
  const int LESS;
  const int EQUALS;
  const int GREATER;
  const int QUESTION;
  const int AT;
  const int LEFTBRACKET;
  const int RIGHTBRACKET;
  const int CARET;
  const int UNDERSCORE;
  const int BACKQUOTE;
  const int a;
  const int b;
  const int c;
  const int d;
  const int e;
  const int f;
  const int g;
  const int h;
  const int i;
  const int j;
  const int k;
  const int l;
  const int m;
  const int n;
  const int o;
  const int p;
  const int q;
  const int r;
  const int s;
  const int t;
  const int u;
  const int v;
  const int w;
  const int x;
  const int y;
  const int z;
  const int DELETE;
  const int UP;
  const int DOWN;
  const int RIGHT;
  const int LEFT;
  const int INSERT;
  const int HOME;
  const int END;
  const int PAGEUP;
  const int PAGEDOWN;
};

/* Win32 native dialog */
class Dialog
{
  void destroy();
  bool valid();
};

/* Win32 MB_ enumeration values */
class MBEnum
{
  const int ABORTRETRYIGNORE;
  const int CANCELTRYCONTINUE;
  const int HELP;
  const int OK;
  const int CANCEL;
  const int RETRYCANCEL;
  const int YESNO;
  const int YESNOCANCEL;
  const int ICONEXCLAMATION;
  const int ICONWARNING;
  const int ICONINFORMATION;
  const int ICONASTERISK;
  const int ICONQUESTION;
  const int ICONSTOP;
  const int ICONERROR;
  const int ICONHAND;
};

/* Win32 ID_ enumeration values */
class IDEnum
{
  const int ABORT;
  const int CANCEL;
  const int CONTINUE;
  const int IGNORE;
  const int NO;
  const int OK;
  const int RETRY;
  const int TRYAGAIN;
  const int YES;
};

/* Win32 WM_ enumeration values */
class WMEnum
{
  const int CLOSE;
  const int COMMAND;
  const int CREATE;
  const int DESTROY;
};

/* Win32 API wrapper */
class WinClass
{
  MBEnum MB;
  IDEnum ID;
  WMEnum WM;
  
  Dialog CreateDialog(string moduleName, Object resourceID, Function dlgProc);
  bool exec(string appName, string cmdLine, bool newConsole = false, bool waitOnProcess = false);
  int MessageBox(string text, string caption, int flags);
  void PumpEvents();
};

/* libxml2 URI wrapper */
class XMLURI
{
  string scheme;
  string opaque;
  string authority;
  string server;
  string user;
  string path;
  string query;
  string fragment;
  string query_raw;
  int    port;
  int    cleanup;
  
  XMLURI(string str);
  string toString();
};

/* libxml2 XML attribute wrapper */
class XMLAttr
{
  static const int ATTRIBUTE_CDATA;
  static const int ATTRIBUTE_ID;
  static const int ATTRIBUTE_IDREF;
  static const int ATTRIBUTE_IDREFS;
  static const int ATTRIBUTE_ENTITY;
  static const int ATTRIBUTE_ENTITIES;
  static const int ATTRIBUTE_NMTOKEN;
  static const int ATTRIBUTE_NMTOKENS;
  static const int ATTRIBUTE_ENUMERATION;
  static const int ATTRIBUTE_NOTATION;

  XMLDocument document;

  int getType();
  string getName();
  string getValue();
  int getAttributeType();
  XMLAttr getNext();
  XMLAttr getPrev();
};

/* libxml2 XML node wrapper */
class XMLNode
{
  static const int ELEMENT_NODE;
  static const int ATTRIBUTE_NODE;
  static const int TEXT_NODE;
  static const int CDATA_SECTION_NODE;
  static const int ENTITY_REF_NODE;
  static const int ENTITY_NODE;
  static const int PI_NODE;
  static const int COMMENT_NODE;
  static const int DOCUMENT_NODE;
  static const int DOCUMENT_TYPE_NODE;
  static const int DOCUMENT_FRAG_NODE;
  static const int NOTATION_NODE;
  static const int HTML_DOCUMENT_NODE;
  static const int DTD_NODE;
  static const int ELEMENT_DECL;
  static const int ATTRIBUTE_DECL;
  static const int ENTITY_DECL;
  static const int NAMESPACE_DECL;
  static const int XINCLUDE_START;
  static const int XINCLUDE_END;
  static const int DOCB_DOCUMENT_NODE;
  
  XMLDocument document;
  
  int getType();
  string getName();
  string getContent();
  string getNodePath();
  int getChildElementCount();
  int getLineNo();
  bool isBlankNode();
  bool isTextNode();
  int getSpacePreserve();
  XMLNode getFirstChild();
  XMLNode getLastChild();
  XMLNode getLastElementChild();
  XMLNode getPreviousElementSibling();
  XMLNode getNext();
  XMLNode getPrev();
  XMLNode getParent();
  XMLAttr getFirstAttribute();
  Array getChildNodes();
  Array getAttributes();
  XMLAttr hasProp(string propName);
};

/* libxml2 XPath object wrapper */
class XMLXPathObject
{
  XMLDocument document;
  int length;
  
  bool hasNodeSet();
  int nodeSetSize();
  XMLNode getNodeAt(int index);
  Array getNodes();
};

/* libxml2 XML document wrapper */
class XMLDocument
{
  static XMLDocument FromFile(string filename);
  static XMLDocument FromFileOpt(string filename, string encoding, int options);
  static XMLDocument FromString(string str);
  static XMLDocument FromCharMem(string str, string url, string encoding, int options);
  static XMLDocument FromMemory(ByteBuffer buf, string url, string encoding, int options);
  static XMLDocument FromHTMLFile(string filename, string encoding = null);
  static XMLDocument FromHTMLFileOpt(string filename, string encoding, int options);
  static XMLDocument FromHTMLString(string html, string encoding = null);
  static XMLDocument FromHTMLStringOpt(string html, string url, string encoding, int options);
  
  bool isEmpty();
  XMLNode getRootNode();
  string getNodeListString(XMLNode node);
  XMLXPathObject evalXPathExpression(string xpathExpr, Object nsmap = null);
  int saveFile(string filename);
  int saveFileEnc(string filename, string encoding);
  int saveFormatFile(string filename, int format);
  int saveFormatFileEnc(string filename, string encoding, int format);
};
