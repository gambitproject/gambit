//
// FILE: gstream.cc -- Implementation of I/O streaming functions
//
// $Id$
//

#ifdef __GNUG__
#pragma implementation "gstream.h"
#endif   // __GNUG__

#include <assert.h>
#include "gstream.h"

//--------------------------------------------------------------------------
//                         gInput member functions
//--------------------------------------------------------------------------

gInput::gInput(void)   { }

gInput::~gInput()   { }


//--------------------------------------------------------------------------
//                       gFileInput member functions
//--------------------------------------------------------------------------

#ifdef USE_EXCEPTIONS
#include "gtext.h"

gText gFileInput::OpenFailed::Description(void) const
{
  return "Open failed in gFileInput";
}

gText gFileInput::ReadFailed::Description(void) const
{
  return "Read failed in gFileInput";
}
#endif   // USE_EXCEPTIONS

gFileInput::gFileInput(const char *in) : f(fopen(in, "r"))
{
#ifdef USE_EXCEPTIONS
  if (!f)  throw OpenFailed();
#else
  valid = (f != 0);
#endif   // USE_EXCEPTIONS
}

gFileInput::gFileInput(FILE *in) : f(in)
{
#ifdef USE_EXCEPTIONS
  if (!f)  throw OpenFailed();
#else
  valid = (f != 0);
#endif   // USE_EXCEPTIONS
}

gFileInput::~gFileInput()
{
  if (f)   fclose(f);
}

gInput &gFileInput::operator>>(int &x)
{
#ifdef USE_EXCEPTIONS
  if (fscanf(f, "%d", &x) != 1)   throw ReadFailed();
#else
  valid = (fscanf(f, "%d", &x) == 1);
#endif   // USE_EXCEPTIONS
  return *this;
}

gInput &gFileInput::operator>>(unsigned int &x)
{
#ifdef USE_EXCEPTIONS
  if (fscanf(f, "%d", &x) != 1)   throw ReadFailed();
#else
  valid = (fscanf(f, "%d", &x) == 1);
#endif   // USE_EXCEPTIONS
  return *this;
}

gInput &gFileInput::operator>>(long &x)
{
#ifdef USE_EXCEPTIONS
  if (fscanf(f, "%ld", &x) != 1)   throw ReadFailed();
#else
  valid = (fscanf(f, "%ld", &x) == 1);
#endif   // USE_EXCEPTIONS
  return *this;
}

gInput &gFileInput::operator>>(char &x)
{
  x = fgetc(f);
  return *this;
}

gInput &gFileInput::operator>>(double &x)
{
#ifdef USE_EXCEPTIONS
  if (fscanf(f, "%lf", &x) != 1)   throw ReadFailed();
#else
  valid = (fscanf(f, "%lf", &x) == 1);
#endif   // USE_EXCEPTIONS
  return *this;
}

gInput &gFileInput::operator>>(float &x)
{
#ifdef USE_EXCEPTIONS
  if (fscanf(f, "%f", &x) != 1)   throw ReadFailed();
#else
  valid = (fscanf(f, "%f", &x) == 1);
#endif   // USE_EXCEPTIONS
  return *this;
}

gInput &gFileInput::operator>>(char *x)
{
#ifdef USE_EXCEPTIONS
  if (fscanf(f, "%s", x) != 1)   throw ReadFailed();
#else
  valid = (fscanf(f, "%s", x) == 1);
#endif   // USE_EXCEPTIONS
  return *this;
}

int gFileInput::get(char &c)
{
  c = fgetc(f);
  return (!feof(f));
}

void gFileInput::unget(char c)
{
  ::ungetc(c, f);
}

bool gFileInput::eof(void) const
{
  return feof(f);
}

void gFileInput::seekp(long pos) const
{
  fseek(f, pos, 0);
}

long gFileInput::getpos(void) const
{
  return ftell(f);
}

void gFileInput::setpos(long x) const
{
  fseek(f, x, 0);
}

#ifdef USE_EXCEPTIONS
bool gFileInput::IsValid(void) const
{
  return true;
}
#else
bool gFileInput::IsValid(void) const
{
  return valid;
}
#endif   // USE_EXCEPTIONS

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

int gNullInput::get(char &)   { return 0; }

void gNullInput::unget(char)  { }

bool gNullInput::eof(void) const   { return true; }

void gNullInput::seekp(long) const   { }

long gNullInput::getpos(void) const { return 0; }

void gNullInput::setpos(long) const { }

bool gNullInput::IsValid(void) const   { return true; }


//--------------------------------------------------------------------------
//                          gOutput member functions
//--------------------------------------------------------------------------

gOutput::gOutput(void)   { }

gOutput::~gOutput()   { }

//--------------------------------------------------------------------------
//                         gFileOutput member functions
//--------------------------------------------------------------------------

#ifdef USE_EXCEPTIONS
gText gFileOutput::OpenFailed::Description(void) const
{
  return "Open failed in gFileOutput";
}

gText gFileOutput::WriteFailed::Description(void) const
{
  return "Write failed in gFileOutput";
}
#endif   // USE_EXCEPTIONS

gFileOutput::gFileOutput(const char *out, bool append /* = false */)
  : f(fopen(out, (append) ? "a" : "w")), Width(0), Prec(6), Represent('f')
{
#ifdef USE_EXCEPTIONS
  if (!f)   throw OpenFailed();
#else
  valid = (f != 0);
#endif   // USE_EXCEPTIONS
}

gFileOutput::gFileOutput(FILE *out)
  : f(out), Width(0), Prec(6), Represent('f')
{
#ifdef USE_EXCEPTIONS
  if (!f)   throw OpenFailed();
#else
  valid = (f != 0);
#endif   // USE_EXCEPTIONS
}

gFileOutput::~gFileOutput()
{
  if (f)   fclose(f);
}

int gFileOutput::GetWidth(void) 
{
  return Width;
}

gOutput &gFileOutput::SetWidth(int w) 
{
  Width = w;
  return *this;
}

int gFileOutput::GetPrec(void) 
{
  return Prec;
}

gOutput &gFileOutput::SetPrec(int p) 
{
  Prec = p;
  return *this;
}

gOutput &gFileOutput::SetExpMode(void) 
{
  Represent = 'e';
  return *this;
}

gOutput &gFileOutput::SetFloatMode(void) 
{
  Represent = 'f';
  return *this;
}

char gFileOutput::GetRepMode(void)
{
  return Represent;
}

gOutput &gFileOutput::operator<<(int x)
{
#ifdef USE_EXCEPTIONS
  if (fprintf(f, "%*d", Width, x) < 0)   throw WriteFailed();
#else
  valid = (fprintf(f, "%*d", Width, x) >= 0);
#endif   // USE_EXCEPTIONS
  return *this;
}

gOutput &gFileOutput::operator<<(unsigned int x)
{
#ifdef USE_EXCEPTIONS
  if (fprintf(f, "%*d", Width, x) < 0)   throw WriteFailed();
#else
  valid = (fprintf(f, "%*d", Width, x) >= 0);
#endif   // USE_EXCEPTIONS
  return *this;
}

gOutput &gFileOutput::operator<<(bool x)
{
#ifdef USE_EXCEPTIONS
  if (fprintf(f, "%c", (x) ? 'T' : 'F') < 0)   throw WriteFailed();
#else
  valid = (fprintf(f, "%c", (x) ? 'T' : 'F') >= 0);
#endif   // USE_EXCEPTIONS
  return *this;
}

gOutput &gFileOutput::operator<<(long x)
{
#ifdef USE_EXCEPTIONS
  if (fprintf(f, "%*ld", Width, x) < 0)   throw WriteFailed();
#else
  valid = (fprintf(f, "%*ld", Width, x) >= 0);
#endif   // USE_EXCEPTIONS
  return *this;
}

gOutput &gFileOutput::operator<<(char x)
{
#ifdef USE_EXCEPTIONS
  if (fprintf(f, "%c", x) < 0)   throw WriteFailed();
#else
  valid = (fprintf(f, "%c", x) >= 0);
#endif   // USE_EXCEPTIONS
  return *this;
}

gOutput &gFileOutput::operator<<(double x)
{
  if (Represent == 'f')   {
#ifdef USE_EXCEPTIONS
    if (fprintf(f, "%*.*f", Width, Prec, x) < 0)   throw WriteFailed();
#else
    valid = (fprintf(f, "%*.*f", Width, Prec, x) >= 0);
#endif   // USE_EXCEPTIONS
  }
  else   {   // Represent == 'e'
#ifdef USE_EXCEPTIONS
    if (fprintf(f, "%*.*e", Width, Prec, x) < 0)   throw WriteFailed();
#else
    valid = (fprintf(f, "%*.*e", Width, Prec, x) >= 0);
#endif   // USE_EXCEPTIONS
  }
  return *this;
}

gOutput &gFileOutput::operator<<(float x)
{
  if (Represent == 'f')   {
#ifdef USE_EXCEPTIONS
    if (fprintf(f, "%*.*f", Width, Prec, x) < 0)   throw WriteFailed();
#else
    valid = (fprintf(f, "%*.*f", Width, Prec, x) >= 0);
#endif   // USE_EXCEPTIONS
  }
  else   {   // Represent == 'e'
#ifdef USE_EXCEPTIONS
    if (fprintf(f, "%*.*e", Width, Prec, x) < 0)   throw WriteFailed();
#else
    valid = (fprintf(f, "%*.*e", Width, Prec, x) >= 0);
#endif   // USE_EXCEPTIONS
  }
  return *this;
}

gOutput &gFileOutput::operator<<(const char *x)
{
#ifdef USE_EXCEPTIONS
  if (fprintf(f, "%s", x) < 0)   throw WriteFailed();
#else
  valid = (fprintf(f, "%s", x) >= 0);
#endif   // USE_EXCEPTIONS
  return *this;
}

gOutput &gFileOutput::operator<<(const void *x)
{
#ifdef USE_EXCEPTIONS
  if (fprintf(f, "%p", x) < 0)   throw WriteFailed();
#else
  valid = (fprintf(f, "%p", x) >= 0);
#endif   // USE_EXCEPTIONS
  return *this;
}

#ifdef USE_EXCEPTIONS
bool gFileOutput::IsValid(void) const
{
  return true;
}
#else
bool gFileOutput::IsValid(void) const
{
  return valid;
}
#endif   // USE_EXCEPTIONS


//--------------------------------------------------------------------------
//                         gNullOutput member functions
//--------------------------------------------------------------------------

gNullOutput::gNullOutput(void)   { }

gNullOutput::~gNullOutput()   { }

int gNullOutput::GetWidth(void) { return true; }

gOutput &gNullOutput::SetWidth(int) { return *this; }

int gNullOutput::GetPrec(void) { return true; }

gOutput &gNullOutput::SetPrec(int) { return *this; }

gOutput &gNullOutput::SetExpMode(void) { return *this; }

gOutput &gNullOutput::SetFloatMode(void) { return *this; }

char gNullOutput::GetRepMode(void) { return true; }

gOutput &gNullOutput::operator<<(int)    { return *this; }

gOutput &gNullOutput::operator<<(unsigned int)   { return *this; }

gOutput &gNullOutput::operator<<(bool)    { return *this; }

gOutput &gNullOutput::operator<<(long)   { return *this; }

gOutput &gNullOutput::operator<<(char)   { return *this; }

gOutput &gNullOutput::operator<<(double)   { return *this; }

gOutput &gNullOutput::operator<<(float)    { return *this; }

gOutput &gNullOutput::operator<<(const char *)  { return *this; }

gOutput &gNullOutput::operator<<(const void *)  { return *this; }

bool gNullOutput::IsValid(void) const   { return true; }


gNullInput _gzero;
gInput &gzero = _gzero;

gNullOutput _gnull;
gOutput &gnull = _gnull;


