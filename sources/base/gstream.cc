//
// FILE: gstream.cc -- Implementation of I/O streaming functions
//
// $Id$
//

#ifdef __GNUG__
#pragma implementation "gstream.h"
#endif   // __GNUG__

#include "base/base.h"

//--------------------------------------------------------------------------
//                         gInput member functions
//--------------------------------------------------------------------------

gInput::gInput(void)   { }

gInput::~gInput()   { }


//--------------------------------------------------------------------------
//                       gFileInput member functions
//--------------------------------------------------------------------------

gText gFileInput::OpenFailed::Description(void) const
{
  return "Open failed in gFileInput";
}

gText gFileInput::ReadFailed::Description(void) const
{
  return "Read failed in gFileInput";
}

gFileInput::gFileInput(const char *in) : f(fopen(in, "r"))
{
  if (!f)   throw OpenFailed();
}

gFileInput::gFileInput(FILE *in) : f(in)
{
  if (!f)   throw OpenFailed();
}

gFileInput::~gFileInput()
{
  if (f)   fclose(f);
}

gInput &gFileInput::operator>>(int &x)
{
  if (fscanf(f, "%d", &x) != 1)   throw ReadFailed();
  return *this;
}

gInput &gFileInput::operator>>(unsigned int &x)
{
  if (fscanf(f, "%d", &x) != 1)   throw ReadFailed();
  return *this;
}

gInput &gFileInput::operator>>(long &x)
{
  if (fscanf(f, "%ld", &x) != 1)   throw ReadFailed();
  return *this;
}

gInput &gFileInput::operator>>(char &x)
{
  x = (char) fgetc(f);
  return *this;
}

gInput &gFileInput::operator>>(double &x)
{
  if (fscanf(f, "%lf", &x) != 1)   throw ReadFailed();
  return *this;
}

gInput &gFileInput::operator>>(float &x)
{
  if (fscanf(f, "%f", &x) != 1)   throw ReadFailed();
  return *this;
}

gInput &gFileInput::operator>>(char *x)
{
  if (fscanf(f, "%s", x) != 1)   throw ReadFailed();
  return *this;
}

int gFileInput::get(char &c)
{
  c = (char) fgetc(f);
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

//--------------------------------------------------------------------------
//                   gStandardInput member functions
//--------------------------------------------------------------------------

gText gStandardInput::ReadFailed::Description(void) const
{
  return "Read failed in gStandardInput";
}

gStandardInput::gStandardInput(void)
{ }

gStandardInput::~gStandardInput()
{ }

gInput &gStandardInput::operator>>(int &x)
{
  if (scanf("%d", &x) != 1)   throw ReadFailed();
  return *this;
}

gInput &gStandardInput::operator>>(unsigned int &x)
{
  if (scanf("%d", &x) != 1)   throw ReadFailed();
  return *this;
}

gInput &gStandardInput::operator>>(long &x)
{
  if (scanf("%ld", &x) != 1)   throw ReadFailed();
  return *this;
}

gInput &gStandardInput::operator>>(char &x)
{
  x = (char) fgetc(stdin);
  return *this;
}

gInput &gStandardInput::operator>>(double &x)
{
  if (scanf("%lf", &x) != 1)   throw ReadFailed();
  return *this;
}

gInput &gStandardInput::operator>>(float &x)
{
  if (scanf("%f", &x) != 1)   throw ReadFailed();
  return *this;
}

gInput &gStandardInput::operator>>(char *x)
{
  if (scanf("%s", x) != 1)   throw ReadFailed();
  return *this;
}

int gStandardInput::get(char &c)
{
  c = (char) fgetc(stdin);
  return (!feof(stdin));
}

void gStandardInput::unget(char c)
{
  ::ungetc(c, stdin);
}

bool gStandardInput::eof(void) const
{
  return feof(stdin);
}

void gStandardInput::seekp(long pos) const
{
  fseek(stdin, pos, 0);
}

long gStandardInput::getpos(void) const
{
  return ftell(stdin);
}

void gStandardInput::setpos(long x) const
{ }


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


//--------------------------------------------------------------------------
//                          gOutput member functions
//--------------------------------------------------------------------------

gOutput::gOutput(void)   { }

gOutput::~gOutput()   { }

//--------------------------------------------------------------------------
//                         gStandardOutput member functions
//--------------------------------------------------------------------------

gText gStandardOutput::OpenFailed::Description(void) const
{
  return "Open failed in gStandardOutput";
}

gText gStandardOutput::WriteFailed::Description(void) const
{
  return "Write failed in gStandardOutput";
}

gStandardOutput::gStandardOutput(void)
  : Width(0), Prec(6), Represent('f')
{ }

gStandardOutput::~gStandardOutput()
{ }

int gStandardOutput::GetWidth(void) const
{
  return Width;
}

gOutput &gStandardOutput::SetWidth(int w) 
{
  Width = w;
  return *this;
}

int gStandardOutput::GetPrec(void) const 
{
  return Prec;
}

gOutput &gStandardOutput::SetPrec(int p) 
{
  Prec = p;
  return *this;
}

gOutput &gStandardOutput::SetExpMode(void) 
{
  Represent = 'e';
  return *this;
}

gOutput &gStandardOutput::SetFloatMode(void) 
{
  Represent = 'f';
  return *this;
}

char gStandardOutput::GetRepMode(void) const
{
  return Represent;
}

gOutput &gStandardOutput::operator<<(int x)
{
  if (printf("%*d", Width, x) < 0)   throw WriteFailed();
  return *this;
}

gOutput &gStandardOutput::operator<<(unsigned int x)
{
  if (printf("%*d", Width, x) < 0)   throw WriteFailed();
  return *this;
}

gOutput &gStandardOutput::operator<<(bool x)
{
  if (printf("%c", (x) ? 'T' : 'F') < 0)   throw WriteFailed();
  return *this;
}

gOutput &gStandardOutput::operator<<(long x)
{
  if (printf("%*ld", Width, x) < 0)   throw WriteFailed();
  return *this;
}

gOutput &gStandardOutput::operator<<(char x)
{
  if (printf("%c", x) < 0)   throw WriteFailed();
  return *this;
}

gOutput &gStandardOutput::operator<<(double x)
{
  if (Represent == 'f')   {
    if (printf("%*.*f", Width, Prec, x) < 0)  
      throw WriteFailed();
  }
  else   {   // Represent == 'e'
    if (printf("%*.*e", Width, Prec, x) < 0) 
      throw WriteFailed();
  }
  return *this;
}

gOutput &gStandardOutput::operator<<(long double x)
{
  if (Represent == 'f')   {
    if (printf("%*.*Lf", Width, Prec, x) < 0)  
      throw WriteFailed();
  }
  else   {   // Represent == 'e'
    if (printf("%*.*Le", Width, Prec, x) < 0) 
      throw WriteFailed();
  }
  return *this;
}

gOutput &gStandardOutput::operator<<(float x)
{
  if (Represent == 'f')   {
    if (printf("%*.*f", Width, Prec, x) < 0) 
      throw WriteFailed();
  }
  else   {   // Represent == 'e'
    if (printf("%*.*e", Width, Prec, x) < 0) 
      throw WriteFailed();
  }
  return *this;
}

gOutput &gStandardOutput::operator<<(const char *x)
{
  if (printf("%s", x) < 0)   throw WriteFailed();
  return *this;
}

gOutput &gStandardOutput::operator<<(const void *x)
{
  if (printf("%p", x) < 0)   throw WriteFailed();
  return *this;
}


//--------------------------------------------------------------------------
//                         gFileOutput member functions
//--------------------------------------------------------------------------

gText gFileOutput::OpenFailed::Description(void) const
{
  return "Open failed in gFileOutput";
}

gText gFileOutput::WriteFailed::Description(void) const
{
  return "Write failed in gFileOutput";
}

gFileOutput::gFileOutput(const char *out, bool append /* = false */, bool close /* = true */)
  : f(fopen(out, (append) ? "a" : "w")), filename(out), 
    keepClosed(close), Width(0), 
    Prec(6), Represent('f')
{
  if (!f)   throw OpenFailed();
}

gFileOutput::~gFileOutput()
{
  if (f)   fclose(f);
}

void gFileOutput::Open(void)
{
  if(!f) { 
    f=fopen(filename,"a");
    if (!f) throw OpenFailed();
  }
}

void gFileOutput::Close(void)
{
  if(keepClosed) { fclose(f);f=0;} 
}

int gFileOutput::GetWidth(void) const
{
  return Width;
}

gOutput &gFileOutput::SetWidth(int w) 
{
  Width = w;
  return *this;
}

int gFileOutput::GetPrec(void) const 
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

char gFileOutput::GetRepMode(void) const
{
  return Represent;
}

gOutput &gFileOutput::operator<<(int x)
{
  Open();
  if (fprintf(f, "%*d", Width, x) < 0)   throw WriteFailed();
  Close();
  return *this;
}

gOutput &gFileOutput::operator<<(unsigned int x)
{
  Open();
  if (fprintf(f, "%*d", Width, x) < 0)   throw WriteFailed();
  Close();
  return *this;
}

gOutput &gFileOutput::operator<<(bool x)
{
  Open();
  if (fprintf(f, "%c", (x) ? 'T' : 'F') < 0)   throw WriteFailed();
  Close();
  return *this;
}

gOutput &gFileOutput::operator<<(long x)
{
  Open();
  if (fprintf(f, "%*ld", Width, x) < 0)   throw WriteFailed();
  Close();
  return *this;
}

gOutput &gFileOutput::operator<<(char x)
{
  Open();
  if (fprintf(f, "%c", x) < 0)   throw WriteFailed();
  Close();
  return *this;
}

gOutput &gFileOutput::operator<<(double x)
{
  Open();
  if (Represent == 'f')   {
    if (fprintf(f, "%*.*f", Width, Prec, x) < 0)  
      throw WriteFailed();
  }
  else   {   // Represent == 'e'
    if (fprintf(f, "%*.*e", Width, Prec, x) < 0) 
      throw WriteFailed();
  }
  Close();
  return *this;
}

gOutput &gFileOutput::operator<<(long double x)
{
  Open();
  if (Represent == 'f')   {
    if (fprintf(f, "%*.*Lf", Width, Prec, x) < 0)  
      throw WriteFailed();
  }
  else   {   // Represent == 'e'
    if (fprintf(f, "%*.*Le", Width, Prec, x) < 0) 
      throw WriteFailed();
  }
  Close();
  return *this;
}

gOutput &gFileOutput::operator<<(float x)
{
  Open();
  if (Represent == 'f')   {
    if (fprintf(f, "%*.*f", Width, Prec, x) < 0) 
      throw WriteFailed();
  }
  else   {   // Represent == 'e'
    if (fprintf(f, "%*.*e", Width, Prec, x) < 0) 
      throw WriteFailed();
  }
  Close();
  return *this;
}

gOutput &gFileOutput::operator<<(const char *x)
{
  Open();
  if (fprintf(f, "%s", x) < 0)   throw WriteFailed();
  Close();
  return *this;
}

gOutput &gFileOutput::operator<<(const void *x)
{
  Open();
  if (fprintf(f, "%p", x) < 0)   throw WriteFailed();
  Close();
  return *this;
}


//--------------------------------------------------------------------------
//                         gNullOutput member functions
//--------------------------------------------------------------------------

gNullOutput::gNullOutput(void)   { }

gNullOutput::~gNullOutput()   { }

int gNullOutput::GetWidth(void) const  { return 0; }

gOutput &gNullOutput::SetWidth(int) { return *this; }

int gNullOutput::GetPrec(void) const { return 6; }

gOutput &gNullOutput::SetPrec(int) { return *this; }

gOutput &gNullOutput::SetExpMode(void) { return *this; }

gOutput &gNullOutput::SetFloatMode(void) { return *this; }

char gNullOutput::GetRepMode(void) const { return 'f'; }

gOutput &gNullOutput::operator<<(int)    { return *this; }

gOutput &gNullOutput::operator<<(unsigned int)   { return *this; }

gOutput &gNullOutput::operator<<(bool)    { return *this; }

gOutput &gNullOutput::operator<<(long)   { return *this; }

gOutput &gNullOutput::operator<<(char)   { return *this; }

gOutput &gNullOutput::operator<<(double)   { return *this; }

gOutput &gNullOutput::operator<<(long double) { return *this; }

gOutput &gNullOutput::operator<<(float)    { return *this; }

gOutput &gNullOutput::operator<<(const char *)  { return *this; }

gOutput &gNullOutput::operator<<(const void *)  { return *this; }


gNullInput _gzero;
gInput &gzero = _gzero;

gNullOutput _gnull;
gOutput &gnull = _gnull;


