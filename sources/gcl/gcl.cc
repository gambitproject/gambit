//
// FILE: gcl.cc -- top level of the Gambit Command Line
//
// $Id$
//

#include <signal.h>
#include <values.h>
#include <math.h>
#include <stdlib.h>
#include <new.h>

#include "base/base.h"
#include "math/rational.h"
#include "gcl/gsmconsole.h"
#include "gcl/gcmdline.h"
#include "gcl/gpreproc.h"

extern int GCLParse(GSM *p_gsm,
		    const gText& line, const gText &file, int lineno,
		    const gText& rawline);

void gclNewHandler(void)
{
  throw gNewFailed();
}


typedef void (*fptr)(int);

void SigFPEHandler(int a)
{
  if (a==SIGFPE) {
    gStandardOutput gout;
    gout << "A floating point error has occured! "
         << "The results returned may be invalid\n";
  }
  signal(SIGFPE, (fptr) SigFPEHandler);  //  reinstall signal handler
}

void SigSegFaultHandler(int)
{
  gStandardOutput gout;
  gout << "A segmentation fault has occurred\n";
#ifdef __BORLANDC__
  gCmdLineInput::RestoreTermAttr();
#endif  // __BORLANDC__
  exit(1);
}

#ifdef __BORLANDC__
//
// A "vanilla" command line for MSW console application, without
// terminal-control-type features.
//
class VanillaCommandLine : public GCL::CommandLine {
protected:
  char GetNextChar(void);

public:
  VanillaCommandLine(int p_historyDepth) : GCL::CommandLine(p_historyDepth) { }

  void SetPrompt(bool) { }

  virtual gInput& operator>>(int &x);
  virtual gInput& operator>>(unsigned int &x) { return *this; }
  virtual gInput& operator>>(long &x) { return *this; }
  virtual gInput& operator>>(char &x) { return *this; }
  virtual gInput& operator>>(double &x) { return *this; }
  virtual gInput& operator>>(float &x) { return *this; }
  virtual gInput& operator>>(char *x) { return *this; }
  
  int get(char &c) {
    c = GetNextChar();
    return !eof(); 
  }

  virtual void unget(char c) { }

  virtual bool eof(void) const { return gin.eof(); }
  virtual void seekp(long /* x */) const { }
  virtual long getpos(void) const { return 0; }
  virtual void setpos(long /* x */) const { }
  
  virtual bool IsValid(void) const { return !eof(); }
};

gInput &VanillaCommandLine::operator>>(int &x)
{
  gin >> x;
  return *this;
}

char VanillaCommandLine::GetNextChar(void)
{
  char c;
  gin.get(c);
  return c;
}

#endif  // __BORLANDC__

char* _SourceDir = NULL;
char* _ExePath = NULL;

int main(int argc, char *argv[])
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

    gStandardInput gin;
    gStandardOutput gout;
    gStandardOutput gerr;
    GSM *gsm = new gsmConsole(gin, gout, gerr);

#ifndef __BORLANDC__
    // Set up the error handling functions:
    signal(SIGFPE, (fptr) SigFPEHandler);

    signal(SIGTSTP, SIG_IGN);

    signal(SIGSEGV, (fptr) SigSegFaultHandler);
    signal(SIGABRT, (fptr) SigSegFaultHandler);
    signal(SIGBUS,  (fptr) SigSegFaultHandler);
    signal(SIGKILL, (fptr) SigSegFaultHandler);
    signal(SIGILL,  (fptr) SigSegFaultHandler);

    signal(SIGINT, gsmConsole::gclStatusHandler);
#endif

#ifdef __BORLANDC__
    VanillaCommandLine gcmdline(20);
#else
    gCmdLineInput gcmdline(20);
#endif   // __BORLANDC__

    gText initString = "Include[\"gclini.gcl\"];\n";
    for (int i = 1; i < argc; i++) {
      initString += "Include[\"";
      initString += argv[i];
      initString += "\"];\n";
    }

    gPreprocessor P(*gsm, &gcmdline, initString);
    
    while (!P.eof()) {
      gText line = P.GetLine();
      gText fileName = P.GetFileName();
      int lineNumber = P.GetLineNumber();
      gText rawLine = P.GetRawLine();
      GCLParse(gsm, line, fileName, lineNumber, rawLine );
    }

    delete[] _SourceDir;
    delete gsm;


#ifndef __BORLANDC__
    // this is normally done in destructor for gCmdLineInput,
    //   in gcmdline.cc, but apparently the destructors for
    //   global static objects are not called, hence this
    gCmdLineInput::RestoreTermAttr();
#endif   // __BORLANDC__
  }

  // AIX has some problem with exception handling
#ifndef _AIX
  catch (gclQuitOccurred &E) {
#ifndef __BORLANDC__
    gCmdLineInput::RestoreTermAttr();
#endif   // __BORLANDC__
    return E.Value();
  }
  // The last line of defense for exceptions:
  catch (gException &w)  {
    gStandardOutput gout;
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



