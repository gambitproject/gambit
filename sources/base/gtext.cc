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

#ifdef __GNUG__
#pragma implementation "gtext.h"
#endif  // __GNUG__

#include <stdlib.h>
#include <ctype.h>
#include "base.h"

gText gText::BadIndex::Description(void) const
{ return "Bad index exception in gText"; }

gText::gText(void)
{
  storage = new char[1];
  *storage = '\0';
}

gText::gText(char c)
{
  storage = new char[2];
  storage[0] = c;
  storage[1] = '\0';
}

gText::gText(const char *s)
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

gText::gText(const gText &s)
{
  storage = new char[strlen(s.storage) + 1];
  strcpy(storage, s.storage);
}

gText::gText(int len)
{
  storage = new char[len + 1];
  memset(storage, 0, len + 1);
}


gText &gText::operator=(const gText &s)
{ 
  if (this != &s) {
    *this = s.storage; 
  }  
  return *this; 
}

gText &gText::operator=(const char *s)
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

gText &gText::operator+=(char c)
{
  char *sp = new char[strlen(storage) + 2];
  strcpy(sp, storage);
  sp[strlen(storage)] = c;
  sp[strlen(storage) + 1] = '\0';
  delete [] storage;
  storage = sp;
  return *this;
}

gText &gText::operator+=(const char *s)
{
  char *sp = new char[strlen(storage) + strlen(s) + 1];
  strcpy(sp, storage);
  strcat(sp, s);
  delete [] storage;
  storage = sp;
  return *this;
}

gText gText::operator+(char c) const
{
  gText tmp(*this);
  tmp += c;
  return tmp;
}

gText gText::operator+(const char *s) const
{
  gText tmp(*this);
  tmp += s;
  return tmp;
}

gText gText::Right(int len) const
{
  int pos = strlen( storage ) - len;
  return gText(storage + ( pos > 0 ? pos : 0 ) );
}

gText gText::Left(int len) const
{
  gText tmp(len + 1);
  strncpy(tmp.storage, storage, len);
  return tmp;
}

gText gText::Mid(int len, int where) const
{
  gText tmp(len + 1);
  strncpy(tmp.storage, storage + where - 1, len);
  return tmp;
}


gText gText::Upcase(void) const
{
  gText tmp = *this;
  for (unsigned int i = 0; i < strlen(storage); i++)
    tmp.storage[i] = (char) toupper(tmp.storage[i]);
  return tmp;
}

gText gText::Dncase(void) const
{
  gText tmp = *this;
  for (unsigned int i = 0; i < strlen(storage); i++)
    tmp.storage[i] = (char) tolower(tmp.storage[i]);
  return tmp;
}

void gText::Insert(char c, unsigned int n)
{
  if (n > strlen(storage))  throw BadIndex();

  char *temp = new char[strlen(storage)+2];
  strncpy(temp, storage, n);
  temp[n] = c;
  strcpy(temp + n + 1, storage + n);

  delete [] storage;
  storage = temp;
}

void gText::Remove(unsigned int n)
{
  if (n > strlen(storage) - 1)  throw BadIndex();

  char *temp = new char[strlen(storage)];
  strncpy(temp, storage, n);
  strcpy(temp + n, storage + n + 1);

  delete [] storage;
  storage = temp;
}

int gText::LastOccur(char c)
{
  int result = -1;

  for (unsigned int i = 0; i < strlen(storage); i++)
    if (storage[i] == c)
      result = i;
  return (result + 1);
}


gInput& operator>>(gInput &from, gText &s)
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

gOutput& operator<<(gOutput &to, const gText &s)
{
  to << s.storage; return to;
}

gText operator+(const char *c, const gText &s)
{
  return gText(c) + s;
}





