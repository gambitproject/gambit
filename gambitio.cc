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

gFileInput::gFileInput(void)
{
  f = 0;
}

gFileInput::gFileInput(const char *in)
{
  f = fopen(in, "r");
}

gFileInput::gFileInput(FILE *in)
{
  f = in;
}

gFileInput::~gFileInput()
{
  if (f)   fclose(f);
}

gFileInput &gFileInput::operator=(FILE *in)
{
  if (f)  fclose(f);
  f = in;
  return *this;
}

gFileInput &gFileInput::operator=(const char *in)
{
  if (f)  fclose(f);
  f = fopen(in, "r");
  return *this;
}

gInput &gFileInput::operator>>(int &x)
{
  assert(f);
  fscanf(f, "%d", &x);
  return *this;
}

gInput &gFileInput::operator>>(unsigned int &x)
{
  assert(f);
  fscanf(f, "%d", &x);
  return *this;
}

gInput &gFileInput::operator>>(long &x)
{
  assert(f);
  fscanf(f, "%ld", &x);
  return *this;
}

gInput &gFileInput::operator>>(char &x)
{
  assert(f);
  fscanf(f, "%c", &x);
  return *this;
}

gInput &gFileInput::operator>>(double &x)
{
  assert(f);
  fscanf(f, "%lf", &x);
  return *this;
}

gInput &gFileInput::operator>>(float &x)
{
  assert(f);
  fscanf(f, "%f", &x);
  return *this;
}

gInput &gFileInput::operator>>(char *x)
{
  assert(f);
  fscanf(f, "%s", x);
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

int gFileInput::eof(void) const
{
  return feof(f);
}

void gFileInput::seekp(long pos) const
{
  assert(f);
  fseek(f, pos, 0);
}

int gFileInput::IsValid(void) const
{
  return (f != 0);
}


gFileOutput::gFileOutput(void)
{
  f = 0;
}

gFileOutput::gFileOutput(const char *out)
{
  f = fopen(out, "w");
}

gFileOutput::gFileOutput(FILE *out)
{
  f = out;
}

gFileOutput::~gFileOutput()
{
  if (f)   fclose(f);
}

gFileOutput &gFileOutput::operator=(FILE *out)
{
  if (f)   fclose(f);
  f = out;
  return *this;
}

gFileOutput &gFileOutput::operator=(const char *out)
{
  if (f)   fclose(f);
  f = fopen(out, "w");
  return *this;
}

gOutput &gFileOutput::operator<<(int x)
{
  assert(f);
  fprintf(f, "%d", x);
  return *this;
}

gOutput &gFileOutput::operator<<(unsigned int x)
{
  assert(f);
  fprintf(f, "%d", x);
  return *this;
}

gOutput &gFileOutput::operator<<(long x)
{
  assert(f);
  fprintf(f, "%ld", x);
  return *this;
}

gOutput &gFileOutput::operator<<(char x)
{
  assert(f);
  fprintf(f, "%c", x);
  return *this;
}

gOutput &gFileOutput::operator<<(double x)
{
  assert(f);
  fprintf(f, "%lf", x);
  return *this;
}

gOutput &gFileOutput::operator<<(float x)
{
  assert(f);
  fprintf(f, "%f", x);
  return *this;
}

gOutput &gFileOutput::operator<<(const char *x)
{
  assert(f);
  fprintf(f, "%s", x);
  return *this;
}

gOutput &gFileOutput::operator<<(const void *x)
{
  assert(f);
  fprintf(f, "%p", x);
  return *this;
}

int gFileOutput::IsValid(void) const
{
  return (f != 0);
}


gFileInput gin(stdin);

gFileOutput gout(stdout);
gFileOutput gerr(stderr);



