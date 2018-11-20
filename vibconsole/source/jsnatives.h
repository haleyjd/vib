
#ifndef JSNATIVES_H__
#define JSNATIVES_H__

#include <map>
#include <vector>
#include <string>

/**
 * Automatically add a native function to the JavaScript Utils class.
 * Instantiate a persistent (ie. global, or class-, module-, or function-level
 * static) instance of this object anywhere. It'll add itself into the linked
 * list this class maintains at program startup, and then AddNatives will put
 * all of those wrapped native functions into the JavaScript "Utils" class.
 *
 * @todo Would be cool if the class could be specified too :P
 */
class NativeWrapper
{
protected:
   static NativeWrapper *wrappers; //!< List of all instances
   NativeWrapper *next;            //!< Next instance in global list
   const char *name;               //!< Name to use for the function in JS
   JSFastNative func;              //!< The FastNative callback
   int numargs;                    //!< Number of expected arguments
   
public:
   /** 
    * Constructor. 
    * Adds itself to the class's global instance list.
    * @param pName The name this function will have in JavaScript.
    * @param pFunc Pointer to the JSFastNative wrapper function.
    * @param pNumArgs Minimum number of arguments the JS function expects.
    */
   NativeWrapper(const char *pName, JSFastNative pFunc, int pNumArgs)
   {
      // Add self to the static list of instances
      next = wrappers;
      wrappers = this;

      name    = pName;
      func    = pFunc;
      numargs = pNumArgs;
   }

   static NativeInitCode Utils_Create(JSContext *cx, JSObject *global);
};

// Enumerations
extern JSClass enumClass;

/**
 * Create an EnumEntry for every value in the enumeration.
 */
struct EnumEntry
{
   int value;        //!< Enumeration value
   const char *name; //!< Name of the enumeration value
};

void AddEnumerationProperties(JSContext *cx, JSObject *obj, EnumEntry *enumEntries);

/** Shorthand macro for creating EnumEntry structures in an array. */
#define ENUMENTRY(e) { e, #e   }

/** Use this to terminate the list of EnumEntry structures. */
#define ENUMEND()    { 0, nullptr }

extern std::ofstream echoFile;

void LazyStringMap_ReturnObject(JSContext *cx, jsval *vp, std::map<std::string, std::string> &sm);
void LazyVecMap_ReturnObject(JSContext *cx, jsval *vp, std::vector<std::map<std::string, std::string>> &vm);

class NativeByteBuffer : public PrivateData
{
   DECLARE_PRIVATE_DATA()

protected:
   unsigned char *memory;
   size_t size;

public:
   NativeByteBuffer(size_t pSize);
   NativeByteBuffer(unsigned char *pMemory, size_t pSize);
   NativeByteBuffer(const NativeByteBuffer &other);
   ~NativeByteBuffer();

   unsigned char *getBuffer() const { return memory; }
   size_t getSize() const { return size; }

   void resize(size_t newSize);

   unsigned char &operator [] (size_t index)
   {
      return memory[index];
   }

   unsigned char operator [] (size_t index) const 
   {
      return memory[index];
   }

   void toString(std::string &out) const;

   void fromString(const std::string &in);

   JSString *toUCString(JSContext *cx);

   static JSObject *ExternalCreate(JSContext *cx, NativeByteBuffer *nbb, AutoNamedRoot &anr);
};


#endif

// EOF

