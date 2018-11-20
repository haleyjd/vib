// This is the main DLL file.

#include "stdafx.h"

#include "PSProxyCLR.h"

PSProxyCLRClass::PSProxyCLRClass()
{
   priv = new PSProxyCLRPrivate();
   priv->psProxy = gcnew PowerShellProxy::PSProxyClass();
}

PSProxyCLRClass::~PSProxyCLRClass()
{
   delete priv;
}

void PSProxyCLRClass::addArgument(const char *arg)
{
   priv->psProxy->addArgument(gcnew System::String(arg));
}

void PSProxyCLRClass::addCommand(const char *cmd)
{
   priv->psProxy->addCommand(gcnew System::String(cmd));
}

void PSProxyCLRClass::addParameter(const char *param)
{
   priv->psProxy->addParameter(gcnew System::String(param));
}

void PSProxyCLRClass::addScript(const char *script)
{
   priv->psProxy->addScript(gcnew System::String(script));
}

char *PSProxyCLRClass::invoke()
{
   System::String ^res = priv->psProxy->invoke();
   return (char *)(Marshal::StringToHGlobalAnsi(res).ToPointer());
}

void PSProxyCLRClass::freeResult(char *res)
{
   Marshal::FreeHGlobal(IntPtr(res));
}