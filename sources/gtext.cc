//
// FILE: gtext.cc -- Implementation of Text class
//
// $Id$
//

#ifdef __GNUG__
#pragma implementation "gtext.h"
#endif  // __GNUG__

#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include "gambitio.h"
#include "gtext.h"

#ifdef USE_EXCEPTIONS
gText gText::BadIndex::Description(void) const
{ return "Bad index exception in gText"; }
#endif   // USE_EXCEPTIONS

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
#ifdef USE_EXCEPTIONS
  if (n > strlen(storage))  throw BadIndex();
#else
  if (n > strlen(storage)) return;    // out of bounds
#endif   // USE_EXCEPTIONS

  char *temp = new char[strlen(storage)+2];
  strncpy(temp, storage, n);
  temp[n] = c;
  strcpy(temp + n + 1, storage + n);

  delete [] storage;
  storage = temp;
}

void gText::Remove(unsigned int n)
{
#ifdef USE_EXCEPTIONS
  if (n > strlen(storage) - 1)  throw BadIndex();
#else
  if (n > strlen(storage) - 1)  return;
#endif  // USE_EXCEPTIONS

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





