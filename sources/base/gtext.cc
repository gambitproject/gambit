//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of Text class
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
#pragma implementation "gtext.h"
#endif  // __GNUG__

#include <stdlib.h>
#include <ctype.h>
#include "base.h"

gbtText gbtText::BadIndex::Description(void) const
{ return "Bad index exception in gbtText"; }

gbtText::gbtText(void)
{
  storage = new char[1];
  *storage = '\0';
}

gbtText::gbtText(char c)
{
  storage = new char[2];
  storage[0] = c;
  storage[1] = '\0';
}

gbtText::gbtText(const char *s)
{
  if (s != 0)   {
    storage = new char[strlen(s) + 1];
    strcpy(storage, s);
  }
  else  {
    storage = new char[1];
    storage[0] = '\0';
  }
}

gbtText::gbtText(const gbtText &s)
{
  storage = new char[strlen(s.storage) + 1];
  strcpy(storage, s.storage);
}

gbtText::gbtText(int len)
{
  storage = new char[len + 1];
  memset(storage, 0, len + 1);
}


gbtText &gbtText::operator=(const gbtText &s)
{ 
  if (this != &s) {
    *this = s.storage; 
  }  
  return *this; 
}

gbtText &gbtText::operator=(const char *s)
{
  delete [] storage;
  if (s != 0)   {
    storage = new char[strlen(s) + 1];
    strcpy(storage, s);
  }
  else  {
    storage = new char[1];
    storage[0] = '\0';
  }
  return *this;
}

gbtText &gbtText::operator+=(char c)
{
  char *sp = new char[strlen(storage) + 2];
  strcpy(sp, storage);
  sp[strlen(storage)] = c;
  sp[strlen(storage) + 1] = '\0';
  delete [] storage;
  storage = sp;
  return *this;
}

gbtText &gbtText::operator+=(const char *s)
{
  char *sp = new char[strlen(storage) + strlen(s) + 1];
  strcpy(sp, storage);
  strcat(sp, s);
  delete [] storage;
  storage = sp;
  return *this;
}

gbtText gbtText::operator+(char c) const
{
  gbtText tmp(*this);
  tmp += c;
  return tmp;
}

gbtText gbtText::operator+(const char *s) const
{
  gbtText tmp(*this);
  tmp += s;
  return tmp;
}

gbtText gbtText::Right(int len) const
{
  int pos = strlen( storage ) - len;
  return gbtText(storage + ( pos > 0 ? pos : 0 ) );
}

gbtText gbtText::Left(int len) const
{
  gbtText tmp(len + 1);
  strncpy(tmp.storage, storage, len);
  return tmp;
}

gbtText gbtText::Mid(int len, int where) const
{
  gbtText tmp(len + 1);
  strncpy(tmp.storage, storage + where - 1, len);
  return tmp;
}


gbtText gbtText::Upcase(void) const
{
  gbtText tmp = *this;
  for (unsigned int i = 0; i < strlen(storage); i++)
    tmp.storage[i] = (char) toupper(tmp.storage[i]);
  return tmp;
}

gbtText gbtText::Dncase(void) const
{
  gbtText tmp = *this;
  for (unsigned int i = 0; i < strlen(storage); i++)
    tmp.storage[i] = (char) tolower(tmp.storage[i]);
  return tmp;
}

void gbtText::Insert(char c, unsigned int n)
{
  if (n > strlen(storage))  throw BadIndex();

  char *temp = new char[strlen(storage)+2];
  strncpy(temp, storage, n);
  temp[n] = c;
  strcpy(temp + n + 1, storage + n);

  delete [] storage;
  storage = temp;
}

void gbtText::Remove(unsigned int n)
{
  if (n > strlen(storage) - 1)  throw BadIndex();

  char *temp = new char[strlen(storage)];
  strncpy(temp, storage, n);
  strcpy(temp + n, storage + n + 1);

  delete [] storage;
  storage = temp;
}

int gbtText::LastOccur(char c)
{
  int result = -1;

  for (unsigned int i = 0; i < strlen(storage); i++)
    if (storage[i] == c)
      result = i;
  return (result + 1);
}


gbtInput& operator>>(gbtInput &from, gbtText &s)
{
  char a;
  
  s = "";
  
  do  {
    from >> a;
  }  while (isspace(a));

  if (a == '\"')  {
    bool lastslash = false;

    from >> a;
    while  (a != '\"' || lastslash)  {
      if (lastslash && a == '"')  
        s += '"';
      else if (lastslash)  {
        s += '\\';
	s += a;
      }
      else if (a != '\\')
	s += a;

      lastslash = (a == '\\');
      from >> a;
    }
  }
  else  {
    do  {
      s += a;
      from >> a;
    }  while (!isspace(a));
  }

  return from;
}

gbtOutput& operator<<(gbtOutput &to, const gbtText &s)
{
  to << s.storage; return to;
}

gbtText operator+(const char *c, const gbtText &s)
{
  return gbtText(c) + s;
}
