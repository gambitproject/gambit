//
// FILE: gel.cc -- Supplies main() for the GEL standalone
//
// $Id$
//

#include <signal.h>
#include <values.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>

#include "gmisc.h"
#include "gstream.h"
#include "gtext.h"
#include "gblock.h"
#include "gstack.h"
#include "tristate.h"
#include "gnumber.h"
#include "exprtree.h"
#include "match.h"
#include "gel.h"
#include "gcmdline.h"
#include "gpreproc.h"


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
/*
#define MATH_CONTINUE    0
#define	MATH_IGNORE	 1
#define	MATH_QUIT	 2

#ifdef __BORLANDC__ // this handler is defined differently windows
extern "C" int winio_ari(const char *msg);
extern "C" void winio_closeall(void);
int _RTLENTRY _matherr (struct exception *e)
#else
#ifdef __linux__ // kludge to make it compile.  Seems linux does not support.
  struct exception {int type;char *name;double arg1,arg2,retval; }; 
#define LN_MINDOUBLE 1e-20
#define SING 0
#endif
int matherr(struct exception *e)
#endif
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
*/
char* _SourceDir = NULL;
char* _ExePath = NULL;

#ifdef __BORLANDC__
int gcl_main(int /*argc*/, char *argv[])
#else
int main( int /*argc*/, char* argv[] )
#endif    // __BORLANDC__
{
  _ExePath = new char[strlen(argv[0]) + 2];
  strcpy(_ExePath, argv[0]);

#ifdef __GNUG__
  const char SLASH = '/';
#elif defined __BORLANDC__
  const char SLASH = '\\';
#endif   // __GNUG__


  char *c = strrchr( argv[0], SLASH );

  _SourceDir = new char[256];
  if (c != NULL)  {
    int len = strlen(argv[0]) - strlen(c);
    assert(len < 256);
    strncpy(_SourceDir, argv[0], len);
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
#endif  

  gelVariableTable *vt = new gelVariableTable; 

  GELCompiler C(vt);
  gPreprocessor P(&gcmdline);

  while (!P.eof())  { 
#ifdef USE_EXCEPTIONS
    try   {
#endif // USE_EXCEPTIONS
      gelExpr *expr = C.Compile(P.GetLine(), P.GetFileName(),
                                P.GetLineNumber(), P.GetRawLine());
      if (expr)   {
        expr->Execute(vt);
        delete expr;
        gout << '\n';
      }
      else  
        gout << "A compile error occurred\n";
#ifdef USE_EXCEPTIONS
    }
    catch (gException &e)  {
      gout << "EXCEPTION: " << e.Description() << '\n';
    }
    catch (...)   {
      gout << "Whoa... exception!!!!\n";
    }
#endif // USE_EXCEPTIONS
  }

  delete[] _SourceDir;
  delete vt;

  // this is normally done in destructor for gCmdLineInput,
  //   in gcmdline.cc, but apparently the destructors for
  //   global static objects are not called, hence this
  gCmdLineInput::RestoreTermAttr();
  
  return 0;
}



