//
// $Source$
// $Date$
// $Revision$
// 
// DESCRIPTION:
// Implementation of I/O streaming functions
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#if defined(__GNUG__) && !defined(__APPLE_CC__)
#pragma implementation "gstream.h"
#endif   // __GNUG__

#include "base.h"

//--------------------------------------------------------------------------
//                         gbtInput member functions
//--------------------------------------------------------------------------

gbtInput::gbtInput(void)   { }

gbtInput::~gbtInput()   { }


//--------------------------------------------------------------------------
//                       gbtFileInput member functions
//--------------------------------------------------------------------------

gbtText gbtFileInput::OpenFailed::Description(void) const
{
  return "Open failed in gbtFileInput";
}

gbtText gbtFileInput::ReadFailed::Description(void) const
{
  return "Read failed in gbtFileInput";
}

gbtFileInput::gbtFileInput(const char *in) : f(fopen(in, "r"))
{
  if (!f)   throw OpenFailed();
}

gbtFileInput::gbtFileInput(FILE *in) : f(in)
{
  if (!f)   throw OpenFailed();
}

gbtFileInput::~gbtFileInput()
{
  if (f)   fclose(f);
}

gbtInput &gbtFileInput::operator>>(int &x)
{
  if (fscanf(f, "%d", &x) != 1)   throw ReadFailed();
  return *this;
}

gbtInput &gbtFileInput::operator>>(unsigned int &x)
{
  if (fscanf(f, "%d", &x) != 1)   throw ReadFailed();
  return *this;
}

gbtInput &gbtFileInput::operator>>(long &x)
{
  if (fscanf(f, "%ld", &x) != 1)   throw ReadFailed();
  return *this;
}

gbtInput &gbtFileInput::operator>>(char &x)
{
  x = (char) fgetc(f);
  return *this;
}

gbtInput &gbtFileInput::operator>>(double &x)
{
  if (fscanf(f, "%lf", &x) != 1)   throw ReadFailed();
  return *this;
}

gbtInput &gbtFileInput::operator>>(float &x)
{
  if (fscanf(f, "%f", &x) != 1)   throw ReadFailed();
  return *this;
}

gbtInput &gbtFileInput::operator>>(char *x)
{
  if (fscanf(f, "%s", x) != 1)   throw ReadFailed();
  return *this;
}

int gbtFileInput::get(char &c)
{
  c = (char) fgetc(f);
  return (!feof(f));
}

void gbtFileInput::unget(char c)
{
  ::ungetc(c, f);
}

bool gbtFileInput::eof(void) const
{
  return feof(f);
}

void gbtFileInput::seekp(long pos) const
{
  fseek(f, pos, 0);
}

long gbtFileInput::getpos(void) const
{
  return ftell(f);
}

void gbtFileInput::setpos(long x) const
{
  fseek(f, x, 0);
}

//--------------------------------------------------------------------------
//                   gbtStandardInput member functions
//--------------------------------------------------------------------------

gbtText gbtStandardInput::ReadFailed::Description(void) const
{
  return "Read failed in gbtStandardInput";
}

gbtStandardInput::gbtStandardInput(void)
{ }

gbtStandardInput::~gbtStandardInput()
{ }

gbtInput &gbtStandardInput::operator>>(int &x)
{
  if (scanf("%d", &x) != 1)   throw ReadFailed();
  return *this;
}

gbtInput &gbtStandardInput::operator>>(unsigned int &x)
{
  if (scanf("%d", &x) != 1)   throw ReadFailed();
  return *this;
}

gbtInput &gbtStandardInput::operator>>(long &x)
{
  if (scanf("%ld", &x) != 1)   throw ReadFailed();
  return *this;
}

gbtInput &gbtStandardInput::operator>>(char &x)
{
  x = (char) fgetc(stdin);
  return *this;
}

gbtInput &gbtStandardInput::operator>>(double &x)
{
  if (scanf("%lf", &x) != 1)   throw ReadFailed();
  return *this;
}

gbtInput &gbtStandardInput::operator>>(float &x)
{
  if (scanf("%f", &x) != 1)   throw ReadFailed();
  return *this;
}

gbtInput &gbtStandardInput::operator>>(char *x)
{
  if (scanf("%s", x) != 1)   throw ReadFailed();
  return *this;
}

int gbtStandardInput::get(char &c)
{
  c = (char) fgetc(stdin);
  return (!feof(stdin));
}

void gbtStandardInput::unget(char c)
{
  ::ungetc(c, stdin);
}

bool gbtStandardInput::eof(void) const
{
  return feof(stdin);
}

void gbtStandardInput::seekp(long pos) const
{
  fseek(stdin, pos, 0);
}

long gbtStandardInput::getpos(void) const
{
  return ftell(stdin);
}

void gbtStandardInput::setpos(long x) const
{ }


//--------------------------------------------------------------------------
//                         gbtNullInput member functions
//--------------------------------------------------------------------------

gbtNullInput::gbtNullInput(void)    { }

gbtNullInput::~gbtNullInput()    { }

gbtInput &gbtNullInput::operator>>(int &x)
{
  x = 0;
  return *this;
}

gbtInput &gbtNullInput::operator>>(unsigned int &x)
{
  x = 0;
  return *this;
}

gbtInput &gbtNullInput::operator>>(long &x)
{
  x = 0L;
  return *this;
}

gbtInput &gbtNullInput::operator>>(char &x)
{
  x = '\0';
  return *this;
}

gbtInput &gbtNullInput::operator>>(double &x)
{
  x = 0.0;
  return *this;
}

gbtInput &gbtNullInput::operator>>(float &x)
{
  x = 0.0;
  return *this;
}

gbtInput &gbtNullInput::operator>>(char *x)
{
  if (x)   *x = '\0';
  return *this;
}

int gbtNullInput::get(char &)   { return 0; }

void gbtNullInput::unget(char)  { }

bool gbtNullInput::eof(void) const   { return true; }

void gbtNullInput::seekp(long) const   { }

long gbtNullInput::getpos(void) const { return 0; }

void gbtNullInput::setpos(long) const { }


//--------------------------------------------------------------------------
//                          gbtOutput member functions
//--------------------------------------------------------------------------

gbtOutput::gbtOutput(void)   { }

gbtOutput::~gbtOutput()   { }

//--------------------------------------------------------------------------
//                         gbtStandardOutput member functions
//--------------------------------------------------------------------------

gbtText gbtStandardOutput::OpenFailed::Description(void) const
{
  return "Open failed in gbtStandardOutput";
}

gbtText gbtStandardOutput::WriteFailed::Description(void) const
{
  return "Write failed in gbtStandardOutput";
}

gbtStandardOutput::gbtStandardOutput(void)
  : Width(0), Prec(6), Represent('f')
{ }

gbtStandardOutput::~gbtStandardOutput()
{ }

int gbtStandardOutput::GetWidth(void) const
{
  return Width;
}

gbtOutput &gbtStandardOutput::SetWidth(int w) 
{
  Width = w;
  return *this;
}

int gbtStandardOutput::GetPrec(void) const 
{
  return Prec;
}

gbtOutput &gbtStandardOutput::SetPrec(int p) 
{
  Prec = p;
  return *this;
}

gbtOutput &gbtStandardOutput::SetExpMode(void) 
{
  Represent = 'e';
  return *this;
}

gbtOutput &gbtStandardOutput::SetFloatMode(void) 
{
  Represent = 'f';
  return *this;
}

char gbtStandardOutput::GetRepMode(void) const
{
  return Represent;
}

gbtOutput &gbtStandardOutput::operator<<(int x)
{
  if (printf("%*d", Width, x) < 0)   throw WriteFailed();
  return *this;
}

gbtOutput &gbtStandardOutput::operator<<(unsigned int x)
{
  if (printf("%*d", Width, x) < 0)   throw WriteFailed();
  return *this;
}

gbtOutput &gbtStandardOutput::operator<<(bool x)
{
  if (printf("%c", (x) ? 'T' : 'F') < 0)   throw WriteFailed();
  return *this;
}

gbtOutput &gbtStandardOutput::operator<<(long x)
{
  if (printf("%*ld", Width, x) < 0)   throw WriteFailed();
  return *this;
}

gbtOutput &gbtStandardOutput::operator<<(char x)
{
  if (printf("%c", x) < 0)   throw WriteFailed();
  return *this;
}

gbtOutput &gbtStandardOutput::operator<<(double x)
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

gbtOutput &gbtStandardOutput::operator<<(float x)
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

gbtOutput &gbtStandardOutput::operator<<(const char *x)
{
  if (printf("%s", x) < 0)   throw WriteFailed();
  return *this;
}

gbtOutput &gbtStandardOutput::operator<<(const void *x)
{
  if (printf("%p", x) < 0)   throw WriteFailed();
  return *this;
}


//--------------------------------------------------------------------------
//                         gbtFileOutput member functions
//--------------------------------------------------------------------------

gbtText gbtFileOutput::OpenFailed::Description(void) const
{
  return "Open failed in gbtFileOutput";
}

gbtText gbtFileOutput::WriteFailed::Description(void) const
{
  return "Write failed in gbtFileOutput";
}

gbtFileOutput::gbtFileOutput(const char *out, bool append /* = false */)
  : f(fopen(out, (append) ? "a" : "w")),
    Width(0), Prec(6), Represent('f')
{
  if (!f)   throw OpenFailed();
}

gbtFileOutput::~gbtFileOutput()
{
  if (f)   fclose(f);
}

int gbtFileOutput::GetWidth(void) const
{
  return Width;
}

gbtOutput &gbtFileOutput::SetWidth(int w) 
{
  Width = w;
  return *this;
}

int gbtFileOutput::GetPrec(void) const 
{
  return Prec;
}

gbtOutput &gbtFileOutput::SetPrec(int p) 
{
  Prec = p;
  return *this;
}

gbtOutput &gbtFileOutput::SetExpMode(void) 
{
  Represent = 'e';
  return *this;
}

gbtOutput &gbtFileOutput::SetFloatMode(void) 
{
  Represent = 'f';
  return *this;
}

char gbtFileOutput::GetRepMode(void) const
{
  return Represent;
}

gbtOutput &gbtFileOutput::operator<<(int x)
{
  if (fprintf(f, "%*d", Width, x) < 0)   throw WriteFailed();
  return *this;
}

gbtOutput &gbtFileOutput::operator<<(unsigned int x)
{
  if (fprintf(f, "%*d", Width, x) < 0)   throw WriteFailed();
  return *this;
}

gbtOutput &gbtFileOutput::operator<<(bool x)
{
  if (fprintf(f, "%c", (x) ? 'T' : 'F') < 0)   throw WriteFailed();
  return *this;
}

gbtOutput &gbtFileOutput::operator<<(long x)
{
  if (fprintf(f, "%*ld", Width, x) < 0)   throw WriteFailed();
  return *this;
}

gbtOutput &gbtFileOutput::operator<<(char x)
{
  if (fprintf(f, "%c", x) < 0)   throw WriteFailed();
  return *this;
}

gbtOutput &gbtFileOutput::operator<<(double x)
{
  if (Represent == 'f')   {
    if (fprintf(f, "%*.*f", Width, Prec, x) < 0)  
      throw WriteFailed();
  }
  else   {   // Represent == 'e'
    if (fprintf(f, "%*.*e", Width, Prec, x) < 0) 
      throw WriteFailed();
  }
  return *this;
}

gbtOutput &gbtFileOutput::operator<<(float x)
{
  if (Represent == 'f')   {
    if (fprintf(f, "%*.*f", Width, Prec, x) < 0) 
      throw WriteFailed();
  }
  else   {   // Represent == 'e'
    if (fprintf(f, "%*.*e", Width, Prec, x) < 0) 
      throw WriteFailed();
  }
  return *this;
}

gbtOutput &gbtFileOutput::operator<<(const char *x)
{
  if (fprintf(f, "%s", x) < 0)   throw WriteFailed();
  return *this;
}

gbtOutput &gbtFileOutput::operator<<(const void *x)
{
  if (fprintf(f, "%p", x) < 0)   throw WriteFailed();
  return *this;
}


//--------------------------------------------------------------------------
//                         gbtNullOutput member functions
//--------------------------------------------------------------------------

gbtNullOutput::gbtNullOutput(void)   { }

gbtNullOutput::~gbtNullOutput()   { }

int gbtNullOutput::GetWidth(void) const  { return 0; }

gbtOutput &gbtNullOutput::SetWidth(int) { return *this; }

int gbtNullOutput::GetPrec(void) const { return 6; }

gbtOutput &gbtNullOutput::SetPrec(int) { return *this; }

gbtOutput &gbtNullOutput::SetExpMode(void) { return *this; }

gbtOutput &gbtNullOutput::SetFloatMode(void) { return *this; }

char gbtNullOutput::GetRepMode(void) const { return 'f'; }

gbtOutput &gbtNullOutput::operator<<(int)    { return *this; }

gbtOutput &gbtNullOutput::operator<<(unsigned int)   { return *this; }

gbtOutput &gbtNullOutput::operator<<(bool)    { return *this; }

gbtOutput &gbtNullOutput::operator<<(long)   { return *this; }

gbtOutput &gbtNullOutput::operator<<(char)   { return *this; }

gbtOutput &gbtNullOutput::operator<<(double)   { return *this; }

gbtOutput &gbtNullOutput::operator<<(float)    { return *this; }

gbtOutput &gbtNullOutput::operator<<(const char *)  { return *this; }

gbtOutput &gbtNullOutput::operator<<(const void *)  { return *this; }
