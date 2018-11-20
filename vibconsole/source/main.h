/** @file main.h
 *
 * VIB Console - Main Module
 *
 */

#ifndef MAIN_H__
#define MAIN_H__

extern bool MainLoopRunning;
extern bool NonInteractive;    // set to true to put the program into non-interactive mode
extern int  ProcessReturnCode;

/**
 * Class to manage shutdown actions that doesn't depend on atexit(), which is 
 * unfriendly with certain libraries.
 */
class ShutdownAction
{
public:
   typedef void (*ShutdownFn)(); /** Shutdown callback prototype */

protected:   
   static ShutdownAction *list; /** Static linked list of all instances */

   // Instance variables
   ShutdownAction *next; /** Next on linked list */
   ShutdownFn      func; /** Function to run on shutdown */

public:
   ShutdownAction(ShutdownFn funcToRun) : func(funcToRun)
   {
      next = list; 
      list = this; // add to static linked list
   }

   /**
    * Run all shutdown actions
    */
   static void RunActions()
   {
      for(auto sf = list; sf; sf = sf->next)
         sf->func();
   }
};

extern const char *const *myenvp;

#endif

// EOF

