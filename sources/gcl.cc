//
// FILE: gcl.cc -- top level of the Gambit Command Line
//
//  $Id$
//

#include <signal.h>
#include <values.h>
#include <math.h>
#include <stdlib.h>
#include <new.h>

#include "rational.h"
#include "gtext.h"
#include "glist.h"
#include "gsm.h"
#include "gstack.h"
#include "gcompile.h"
#include "gcmdline.h"
#include "gpreproc.h"


void gclNewHandler(void)
{
  throw gNewFailed();
}


typedef void (*fptr)(int);

void SigFPEHandler(int a)
{
  if (a==SIGFPE)
    gerr << "A floating point error has occured! "
         << "The results returned may be invalid\n";
  signal(SIGFPE, (fptr) SigFPEHandler);  //  reinstall signal handler
}

void SigSegFaultHandler(int)
{
  gerr << "A segmentation fault has occurred\n";
  gCmdLineInput::RestoreTermAttr();
  exit(1);
}

#ifdef __BORLANDC__ // this handler is defined differently windows
#define MATH_CONTINUE    0
#define	MATH_IGNORE	 1
#define	MATH_QUIT	 2
extern "C" int winio_ari(const char *msg);
extern "C" void winio_closeall(void);
int _RTLENTRY _matherr (struct exception *e)
  /*
#else
#ifdef __linux__ // kludge to make it compile.  Seems linux does not support.
  struct exception {int type;char *name;double arg1,arg2,retval; };
#define LN_MINDOUBLE 1e-20
#define SING 0
#endif
int matherr(struct exception *e)
#endif
  */
{
  char *whyS [] = { "argument domain error",
		    "argument singularity ",
		    "overflow range error ",
		    "underflow range error",
		    "total loss of significance",
		    "partial loss of significance" };
  static option = MATH_CONTINUE;
  char errMsg[80];
  if (option != MATH_IGNORE)  {
    sprintf (errMsg, "%s (%8g,%8g): %s\n",
	     e->name, e->arg1, e->arg2, whyS [e->type - 1]);
    gerr << errMsg;
// define this to pop up a dialog for math errors under windows. 
// allows to quit.
#if defined(__WIN32__) && defined(MATH_ERROR_DIALOG)
    option = winio_ari(errMsg);
    if (option == MATH_QUIT) {
      winio_closeall(); 
      exit(1);
    }
#endif
  }

  if (e->type == SING)
    if (!strcmp(e->name,"log"))
      if(e->arg1 == 0.0)  {
	e->retval = LN_MINDOUBLE;
	return 1;
      }

  return 1;	// we did not really fix anything, but want no more warnings
}
#endif


char* _SourceDir = NULL;
char* _ExePath = NULL;

#ifdef __BORLANDC__
#include "stdafx.h"
#include "mainfrm.h"
#include "wineditdoc.h"
#include "wineditview.h"
#endif // __BORLANDC__

void BeginWaitCursor()
{
#ifdef __BORLANDC__
((CMainFrame*) AfxGetMainWnd())->BeginWaitCursor();
#endif // __BORLANDC__
}

void EndWaitCursor()
{
#ifdef __BORLANDC__
((CMainFrame*) AfxGetMainWnd())->EndWaitCursor();
#endif // __BORLANDC__
}


#ifdef __BORLANDC__
int gcl_main( int /*argc*/, char* argv[] )
#else
int main( int /*argc*/, char* argv[] )
#endif // __BORLANDC__

{
  set_new_handler(gclNewHandler);

  try {
    _ExePath = new char[strlen(argv[0]) + 2];
#ifdef __BORLANDC__
    // Apparently, Win95 surrounds the program name with explicit quotes;
    // if this occurs, special case code
    if (argv[0][0] == '"') {
      strncpy(_ExePath, argv[0] + 1, strlen(argv[0]) - 2);
    }
    else {
      strcpy(_ExePath, argv[0]);
    }
#else
    strcpy(_ExePath, argv[0]);
#endif  // __BORLANDC__
#ifdef __GNUG__
    const char SLASH = '/';
#elif defined __BORLANDC__
    const char SLASH = '\\';
#endif   // __GNUG__


    char *c = strrchr( _ExePath, SLASH );

    _SourceDir = new char[256];
    if (c != NULL)  {
      int len = strlen(_ExePath) - strlen(c);
      if (len >= 256)  len = 255;
      strncpy(_SourceDir, _ExePath, len);
    }
    else   {
      strcpy(_SourceDir, "");
    }

    // Set up the error handling functions:
#ifndef __BORLANDC__
    signal(SIGFPE, (fptr) SigFPEHandler);

    signal(SIGTSTP, SIG_IGN);

    signal(SIGSEGV, (fptr) SigSegFaultHandler);
    signal(SIGABRT, (fptr) SigSegFaultHandler);
    signal(SIGBUS,  (fptr) SigSegFaultHandler);
    signal(SIGKILL, (fptr) SigSegFaultHandler);
    signal(SIGILL,  (fptr) SigSegFaultHandler);

    // This is added here since for some reason, the handler gets
    // uninstalled on linux.  This shouldn't *hurt* anything to
    // install it here as well.
    extern void gGCLStatusHandler(int);
    signal(SIGINT, gGCLStatusHandler);
#endif

    GSM *gsm = new GSM;

    GCLCompiler C(*gsm);
    gCmdLineInput gcmdline(20);
    gPreprocessor P(*gsm, &gcmdline, "Include[\"gclini.gcl\"]");
    while (!P.eof()) {
      gText line = P.GetLine();
      gText fileName = P.GetFileName();
      int lineNumber = P.GetLineNumber();
      gText rawLine = P.GetRawLine();
      BeginWaitCursor();  // for BC
      C.Parse(line, fileName, lineNumber, rawLine );
      EndWaitCursor();  // for BC
    }

    delete[] _SourceDir;
    delete gsm;


    // this is normally done in destructor for gCmdLineInput,
    //   in gcmdline.cc, but apparently the destructors for
    //   global static objects are not called, hence this
    gCmdLineInput::RestoreTermAttr();
  }

  // AIX has some problem with exception handling
#ifndef _AIX
  catch (gclQuitOccurred &E) {
    gCmdLineInput::RestoreTermAttr();
    return E.Value();
  }
  // The last line of defense for exceptions:
  catch (gException &w)  {
    gout << "GCL EXCEPTION:" << w.Description() << "; Caught in gcl.cc, main()\n";
    return 1;
  }
#else
  catch (...) {
    gCmdLineInput::RestoreTermAttr();
    return 0;
  }
#endif // _AIX

  return 0;
}



