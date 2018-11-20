// PSProxyCLR.h

#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;

class PSProxyCLRPrivate
{
public:
   msclr::auto_gcroot<PowerShellProxy::PSProxyClass^> psProxy;
};

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
