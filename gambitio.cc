//#
//# FILE: gambitio.cc -- Implementation of I/O streaming functions
//#
//# $Id$
//#

#ifdef __GNUG__
#pragma implementation "gambitio.h"
#endif   // __GNUG__

#include <assert.h>
#include "gambitio.h"

//--------------------------------------------------------------------------
//                         gInput member functions
//--------------------------------------------------------------------------

gInput::gInput(void)   { }

gInput::~gInput()   { }


//--------------------------------------------------------------------------
//                       gFileInput member functions
//--------------------------------------------------------------------------

gFileInput::gFileInput(void)
{
  f = 0; valid=0;
}

gFileInput::gFileInput(const char *in)
{
  f = fopen(in, "r");valid=(f==NULL) ? 0 : 1;
}

gFileInput::gFileInput(FILE *in)
{
  f = in;valid=(f==NULL) ? 0 : 1;
}

gFileInput::~gFileInput()
{
  if (f)   fclose(f);valid=0;
}

gFileInput &gFileInput::operator=(FILE *in)
{
  if (f)  fclose(f);
  f = in;valid=(f==NULL) ? 0 : 1;
  return *this;
}

gFileInput &gFileInput::operator=(const char *in)
{
  if (f)  fclose(f);
  f = fopen(in, "r");valid=(f==NULL) ? 0 : 1;
  return *this;
}

gInput &gFileInput::operator>>(int &x)
{
  assert(f);
  int c=fscanf(f, "%d", &x);valid=(c==1) ? 1 : 0;
  return *this;
}

gInput &gFileInput::operator>>(unsigned int &x)
{
  assert(f);
  int c=fscanf(f, "%d", &x);valid=(c==1) ? 1 : 0;
  return *this;
}

gInput &gFileInput::operator>>(long &x)
{
  assert(f);
  int c=fscanf(f, "%ld", &x);valid=(c==1) ? 1 : 0;
  return *this;
}

gInput &gFileInput::operator>>(char &x)
{
  assert(f);
//  int c=fscanf(f, "%c", &x);valid=(c==1) ? 1 : 0;
  x = fgetc(f);
  return *this;
}

gInput &gFileInput::operator>>(double &x)
{
  assert(f);
  int c=fscanf(f, "%lf", &x);valid=(c==1) ? 1 : 0;
  return *this;
}

gInput &gFileInput::operator>>(float &x)
{
  assert(f);
  int c=fscanf(f, "%f", &x);valid=(c==1) ? 1 : 0;
  return *this;
}

gInput &gFileInput::operator>>(char *x)
{
  assert(f);
  int c=fscanf(f, "%s", x);valid=(c==1) ? 1 : 0;
  return *this;
}

int gFileInput::get(char &c)
{
  assert(f);
  c = fgetc(f);
  return (!feof(f));
}

void gFileInput::unget(char c)
{
  assert(f);
  ::ungetc(c, f);
}

bool gFileInput::eof(void) const
{
  return feof(f);
}

void gFileInput::seekp(long pos) const
{
  assert(f);
  fseek(f, pos, 0);
}

bool gFileInput::IsValid(void) const
{
  return valid;
}

//--------------------------------------------------------------------------
//                         gNullInput member functions
//--------------------------------------------------------------------------

gNullInput::gNullInput(void)    { }

gNullInput::~gNullInput()    { }

gInput &gNullInput::operator>>(int &x)
{
  x = 0;
  return *this;
}

gInput &gNullInput::operator>>(unsigned int &x)
{
  x = 0;
  return *this;
}

gInput &gNullInput::operator>>(long &x)
{
  x = 0L;
  return *this;
}

gInput &gNullInput::operator>>(char &x)
{
  x = '\0';
  return *this;
}

gInput &gNullInput::operator>>(double &x)
{
  x = 0.0;
  return *this;
}

gInput &gNullInput::operator>>(float &x)
{
  x = 0.0;
  return *this;
}

gInput &gNullInput::operator>>(char *x)
{
  if (x)   *x = '\0';
  return *this;
}

int gNullInput::get(char &c)   { return 0; }

void gNullInput::unget(char c)  { }

bool gNullInput::eof(void) const   { return true; }

void gNullInput::seekp(long x) const   { }

bool gNullInput::IsValid(void) const   { return true; }


//--------------------------------------------------------------------------
//                          gOutput member functions
//--------------------------------------------------------------------------

gOutput::gOutput(void)   { }

gOutput::~gOutput()   { }


//--------------------------------------------------------------------------
//                         gFileOutput member functions
//--------------------------------------------------------------------------


gFileOutput::gFileOutput(void)
{
  f = 0;valid=0;
}

gFileOutput::gFileOutput(const char *out)
{
  f = fopen(out, "w");valid=(f==NULL) ? 0 : 1;
}

gFileOutput::gFileOutput(FILE *out)
{
  f = out;valid=(f==NULL) ? 0 : 1;
}

gFileOutput::~gFileOutput()
{
  if (f)   fclose(f);valid=0;
}

gFileOutput &gFileOutput::operator=(FILE *out)
{
  if (f)   fclose(f);
  f = out;valid=(f==NULL) ? 0 : 1;
  return *this;
}

gFileOutput &gFileOutput::operator=(const char *out)
{
  if (f)   fclose(f);
  f = fopen(out, "w");valid=(f==NULL) ? 0 : 1;
  return *this;
}

gOutput &gFileOutput::operator<<(int x)
{
  assert(f);
  int c=fprintf(f, "%d", x);  valid = (c == 1) ? 1 : 0;
  return *this;
}

gOutput &gFileOutput::operator<<(unsigned int x)
{
  assert(f);
  int c=fprintf(f, "%d", x);valid=(c==1) ? 1 : 0;
  return *this;
}

gOutput &gFileOutput::operator<<(long x)
{
  assert(f);
  int c=fprintf(f, "%ld", x);valid=(c==1) ? 1 : 0;
  return *this;
}

gOutput &gFileOutput::operator<<(char x)
{
  assert(f);
  int c=fprintf(f, "%c", x);valid=(c==1) ? 1 : 0;
  return *this;
}

gOutput &gFileOutput::operator<<(double x)
{
  assert(f);
  int c=fprintf(f, "%lf", x);valid=(c==1) ? 1 : 0;
  return *this;
}

gOutput &gFileOutput::operator<<(float x)
{
  assert(f);
  int c=fprintf(f, "%f", x);valid=(c==1) ? 1 : 0;
  return *this;
}

gOutput &gFileOutput::operator<<(const char *x)
{
  assert(f);
  int c=fprintf(f, "%s", x);valid=(c==1) ? 1 : 0;
  return *this;
}

gOutput &gFileOutput::operator<<(const void *x)
{
  assert(f);
  int c=fprintf(f, "%p", x);valid=(c==1) ? 1 : 0;
  return *this;
}

bool gFileOutput::IsValid(void) const
{
  return valid;
}


//--------------------------------------------------------------------------
//                         gNullOutput member functions
//--------------------------------------------------------------------------

gNullOutput::gNullOutput(void)   { }

gNullOutput::~gNullOutput()   { }

gOutput &gNullOutput::operator<<(int)    { return *this; }

gOutput &gNullOutput::operator<<(unsigned int)   { return *this; }

gOutput &gNullOutput::operator<<(long)   { return *this; }

gOutput &gNullOutput::operator<<(char)   { return *this; }

gOutput &gNullOutput::operator<<(double)   { return *this; }

gOutput &gNullOutput::operator<<(float)    { return *this; }

gOutput &gNullOutput::operator<<(const char *)  { return *this; }

gOutput &gNullOutput::operator<<(const void *)  { return *this; }

bool gNullOutput::IsValid(void) const   { return true; }


gFileInput _gin(stdin);
gInput &gin = _gin;

gNullInput _gzero;
gInput &gzero = _gzero;

gFileOutput _gout(stdout);
gOutput &gout = _gout;

gFileOutput _gerr(stderr);
gOutput &gerr = _gerr;

gNullOutput _gnull;
gOutput &gnull = _gnull;


