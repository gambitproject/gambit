//
// FILE: gstring.cc -- Implementation of String class
//
// $Id$
//

#ifdef __GNUG__
#pragma implementation "gstring.h"
#endif  // __GNUG__

#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include "gambitio.h"
#include "gstring.h"


gString::gString(void)
{
  storage = new char[1];
  assert(storage != 0);
  *storage = '\0';
}

gString::gString(char c)
{
  storage = new char[2];
  assert(storage != 0);
  storage[0] = c;
  storage[1] = '\0';
}

gString::gString(const char *s)
{
  if (s != 0)   {
    storage = new char[strlen(s) + 1];
    assert(storage != 0);
    strcpy(storage, s);
  }
  else  {
    storage = new char[1];
    assert(storage != 0);
    storage[0] = '\0';
  }
}

gString::gString(const gString &s)
{
  storage = new char[strlen(s.storage) + 1];
  assert(storage != 0);
  strcpy(storage, s.storage);
}

gString::gString(int len)
{
  storage = new char[len + 1];
  assert(storage != 0);
  memset(storage, 0, len + 1);
}


gString &gString::operator=(const char *s)
{
  delete [] storage;
  if (s != 0)   {
    storage = new char[strlen(s) + 1];
    assert(storage != 0);
    strcpy(storage, s);
  }
  else  {
    storage = new char[1];
    assert(storage != 0);
    storage[0] = '\0';
  }
  return *this;
}

void gString::operator+=(char c)
{
  char *sp = new char[strlen(storage) + 2];
  assert (sp != 0);
  strcpy(sp, storage);
  sp[strlen(storage)] = c;
  sp[strlen(storage) + 1] = '\0';
  delete [] storage;
  storage = sp;
}

void gString::operator+=(const char *s)
{
  char *sp = new char[strlen(storage) + strlen(s) + 1];
  assert (sp != 0);
  strcpy(sp, storage);
  strcat(sp, s);
  delete [] storage;
  storage = sp;
}

gString gString::operator+(char c) const
{
  gString tmp(*this);
  tmp += c;
  return tmp;
}

gString gString::operator+(const char *s) const
{
  gString tmp(*this);
  tmp += s;
  return tmp;
}

gString gString::right(int len) const
{
  int pos = strlen( storage ) - len;
  return gString(storage + ( pos > 0 ? pos : 0 ) );
}

gString gString::left(int len) const
{
  gString tmp(len + 1);
  strncpy(tmp.stradr(), storage, len);
  return tmp;
}

gString gString::mid(int len, int where) const
{
  gString tmp(len + 1);
  strncpy(tmp.stradr(), storage + where - 1, len);
  return tmp;
}


gString gString::upcase(void) const
{
  gString tmp = *this;
  for (unsigned int i = 0; i < strlen(storage); i++)    tmp.storage[i] = toupper(tmp.storage[i]);
  return tmp;
}

gString gString::dncase(void) const
{
  gString tmp = *this;
  for (unsigned int i = 0; i < strlen(storage); i++)    tmp.storage[i] = tolower(tmp.storage[i]);
  return tmp;
}

void gString::insert(char c, unsigned int n)
{
    if (n > strlen(storage)) return;    // out of bounds
    char *temp = new char[strlen(storage)+2];
    assert(temp);

    strncpy(temp, storage, n);
    temp[n] = c;
    strcpy(temp + n + 1, storage + n);

    delete [] storage;
    storage = temp;
}

void gString::remove(unsigned int n)
{
    if (n > strlen(storage)-1) return;
    char *temp = new char[strlen(storage)];

    assert(temp);

    strncpy(temp, storage, n);
    strcpy(temp + n, storage + n + 1);

    delete [] storage;
    storage = temp;
}

int gString::lastOccur(char c)
{
  int result = -1;

  for (unsigned int i = 0; i < strlen(storage); i++)
    { if (storage[i] == c) {result = i;}; }
  return (result + 1);
}


gInput& operator>>(gInput &from, gString &s)
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

gOutput& operator<<(gOutput &to, const gString &A)
{
  to << A.storage; return to;
}





