//#
//# FILE: gambitio.cc -- Implementation of I/O streaming functions
//#
//# @(#)gambitio.cc	1.4 11/3/94
//#

#ifdef __GNUG__
#pragma implementation "gambitio.h"
#endif   // __GNUG__

#include <assert.h>
#include "gambitio.h"

gFileInput::gFileInput(void)
{
  f = 0;valid=0;
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
	int c=fscanf(f, "%c", &x);valid=(c==1) ? 1 : 0;
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
  return valid;
}


gFileOutput::gFileOutput(void)
{
  f = 0;valid=0;
}

gFileOutput::gFileOutput(const char *out)
{
	f =(out) ? fopen(out, "w") : 0;valid=(f==0) ? 0 : 1;
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
	int c=fprintf(f, "%d", x);
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
	int c=fprintf(f, "%6.6g", x);valid=(c==1) ? 1 : 0;
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

int gFileOutput::IsValid(void) const
{
	return valid;
}


gFileInput gin(stdin);

gFileOutput gout(stdout);
gFileOutput gerr(stderr);



