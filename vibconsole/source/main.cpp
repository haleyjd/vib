/** @file main.cpp
 *
 * VIB Console - Main Module
 *
 */

#ifndef VIBC_NO_WIN32
#include <windows.h>
#endif

#include <iostream>
#include <fstream>
#include <string>

#include "inifile.h"
#include "main.h"
#include "misc.h"
#include "jsengine2.h"
#include "jsnatives.h"

// Globals
bool MainLoopRunning;
bool NonInteractive; // does not take input at the console
int  ProcessReturnCode = 0;

// Statics

// Linked list of ShutdownAction instances
ShutdownAction *ShutdownAction::list;

#ifndef VIBC_NO_WIN32
/** 
 * This event is used to orchestrate synchronization with the main thread when
 * a console event is received.
 */
static HANDLE stopAppEvent;

/**
 * Win32 console exception handler which is setup during initialization only
 * when running as an application. This will get called whenever an important
 * event is caught by the application's console window.
 * @param CEvent Win32 console event code
 * @return Always returns TRUE.
 */
static BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
   switch(CEvent)
   {
   case CTRL_CLOSE_EVENT:
   case CTRL_BREAK_EVENT:
   case CTRL_C_EVENT:
      SetEvent(stopAppEvent); // signal the stop event
      break;
   }

   return TRUE;
}
#endif

/**
 * Tweak properties of the console window
 */
static void SetupConsoleWindow()
{
#ifndef VIBC_NO_WIN32
   // Disable the close button; program should be exited via Core.exit()
   HWND wnd = GetConsoleWindow();
   if(wnd)
   {
      HMENU menu = GetSystemMenu(wnd, FALSE);
      DeleteMenu(menu, SC_CLOSE, MF_BYCOMMAND);
   }

   SetConsoleTitle(L"vibconsole");
#endif
}

static int myargc;
static const char *const *myargv;

const char *const *myenvp;

static int CheckArg(const char *arg)
{
   for(int i = 1; i < myargc; i++)
   {
      if(!_stricmp(arg, myargv[i]))
         return i;
   }

   return 0;
}

static bool IsCommandArg(const char *const argv)
{
   return (*argv == '-' || *argv == '+' || *argv == '@');
}

/**
 * Process command line options
 */
static void ProcessCmdLine()
{
   bool exitImm = false;
   int  code = 0;
   int p;

   // -exit: exit immediately after performing command line tasks
   if(CheckArg("-exit"))
      exitImm = true;

   // -noninteractive: disables console input in the main loop
   if(CheckArg("-noninteractive"))
      NonInteractive = true;

   // -file: execute the following arguments as script files in order provided
   if((p = CheckArg("-file")) && p < myargc - 1)
   {
      for(int i = p + 1; i < myargc; i++)
      {
         if(IsCommandArg(myargv[i]))
            break;
         jsval res;
         JSEngine_EvaluateFile(myargv[i], &res);
         code = JSEngine_ValueToInteger(&res);
      }
   }

   // -eval: evaluate the following arguments as expressions in order provided
   if((p = CheckArg("-eval")) && p < myargc - 1)
   {
      for(int i = p + 1; i < myargc; i++)
      {
         if(IsCommandArg(myargv[i]))
            break;
         jsval res;
         JSEngine_EvaluateScript("command line", myargv[i], &res);
         code = JSEngine_ValueToInteger(&res);
      }
   }

   if(exitImm)
      exit(code);
}

/**
 * Program Initialization
 * @return True if all setup operations were successful, or the user chose to
 *  ignore all non-fatal warnings. False otherwise.
 */
static bool InitProgram()
{
   SetupConsoleWindow();

   // Ensure ios is in sync with stdio
   std::ios::sync_with_stdio();
   
#ifndef VIBC_NO_WIN32
   // Setup console event handler
   if(!SetConsoleCtrlHandler(ConsoleHandler, TRUE))
   {
      if(!ReportError("Could not set console handler."))
         return false;
   }
   else
      stopAppEvent = CreateEvent(0, FALSE, FALSE, 0);
#endif

   // Load ini file
   IniFile &ini = IniFile::GetIniFile();
   ini.loadOptionsFromFile("options.ini");
   
   // Initialize JSAPI
   if(!JSEngine_Init())
      return false;

   // Process command line
   ProcessCmdLine();
   
   return true;
}

/**
 * Main Loop
 */
static void MainLoop()
{
   MainLoopRunning = true;

   while(MainLoopRunning)
   {
      // if in non-interactive mode, run non-interactive features instead
      if(NonInteractive)
      {
      }
      else
      {
         std::string prompt;
         JSEngine_GetInputPrompt(prompt);
         std::cout << prompt;

         if(echoFile)
         {
            echoFile << prompt;
            echoFile.flush();
         }

         // Get a line of input
         std::string input;
         std::getline(std::cin, input);

         if(echoFile)
         {
            echoFile << input << std::endl;
            echoFile.flush();
         }

         // Feed it to the JS interpreter
         JSEngine_AddInputLine(input);
      }

#ifndef VIBC_NO_WIN32
      if(WaitForSingleObject(stopAppEvent, 0) == WAIT_OBJECT_0)
         break; // console event wants us to exit
#endif
   }
}

/**
 * Shutdown tasks
 */
void Shutdown()
{
   // Run all shutdown actions
   ShutdownAction::RunActions();

   // Shutdown JS
   JSEngine_Shutdown();
}

/**
 * Main Program
 */
int main(int argc, const char *argv[], const char *envp[])
{
   myargc = argc;
   myargv = argv;
   myenvp = envp;

   if(InitProgram())
      MainLoop();

   Shutdown();
      
   return ProcessReturnCode;
}

// EOF

