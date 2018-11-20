/*
  Native C++ Consumable Header for PSProxyCLR
*/

#ifndef PSPROXYCLR_H__
#define PSPROXYCLR_H__

#ifndef VIBC_NO_POWERSHELL

class PSProxyCLRPrivate;

class __declspec(dllexport) PSProxyCLRClass
{
private:
   PSProxyCLRPrivate *priv;

public:
   PSProxyCLRClass();
   ~PSProxyCLRClass();

   void addArgument(const char *arg);
   void addCommand(const char *cmd);
   void addParameter(const char *param);
   void addScript(const char *script);
   char *invoke();
   void freeResult(char *res);
};

#endif

#endif