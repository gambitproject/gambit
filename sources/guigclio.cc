// File: wxgclio.cc -- Creates a GUI shell to simulate console IO for the Gambit
// command line.  The class gWxGclIo may come in useful anywhere we need to use
// console IO in a gui enviroment.  Note that you must define NO_GIO when compiling
// gambitio.cc to avoid duplicate symbols.
//  $Id$
#include "guigclio.h"


gOutput &GuiGclIo::operator<<(int x)
{

  int c=printf( "%*d", Width,  x);  valid = (c == 1) ? 1 : 0;
  return *this;
}

gOutput &GuiGclIo::operator<<(unsigned int x)
{

  int c=printf( "%*d", Width,  x);valid=(c==1) ? 1 : 0;
  return *this;
}

gOutput &GuiGclIo::operator<<(bool x)
{

  int c=printf( "%c",(x) ? 'T' : 'F');  valid = (c == 1) ? 1 : 0;
  return *this;
}

gOutput &GuiGclIo::operator<<(long x)
{
  
  int c=printf( "%*ld", Width, x);valid=(c==1) ? 1 : 0;
  return *this;
}

gOutput &GuiGclIo::operator<<(char x)
{
  
  int c=printf( "%c", x);valid=(c==1) ? 1 : 0;
  return *this;
}

gOutput &GuiGclIo::operator<<(double x)
{
  int c = 0;

  
  switch (Represent) { 
    case 'f':
      c = printf( "%*.*f", Width, Prec, x);
      break;
    case 'e':
      c = printf( "%*.*e", Width, Prec, x);
      break;
    }
  valid=(c==1) ? 1 : 0;
  return *this;
}

gOutput &GuiGclIo::operator<<(float x)
{
  int c;

  
  switch (Represent) {
    case 'f':
      c=printf( "%*.*f", Width, Prec, x);
      break;
    case 'e':
      c=printf( "%*.*e", Width, Prec, x);
      break;
    }
  valid=(c==1) ? 1 : 0;
  return *this;
}

gOutput &GuiGclIo::operator<<(const char *x)
{
  
  int c=printf( "%s", x);valid=(c==1) ? 1 : 0;
  return *this;
}

gOutput &GuiGclIo::operator<<(const void *x)
{

	int c=printf( "%p", x);valid=(c==1) ? 1 : 0;
  return *this;
}

/*
GuiGclIo gclio;

gInput &gin=gclio;
gOutput &gout=gclio;
gOutput &gerr=gclio;
  */

gFileInput _gin(stdin);
gInput &gin = _gin;

gWinOutput _gout;
gOutput &gout = _gout;

gWinOutput _gerr;
gOutput &gerr = _gerr;


