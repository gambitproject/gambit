//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Definition of gbtText class
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

#ifndef GTEXT_H
#define GTEXT_H

#if defined(__GNUG__) && !defined(__APPLE_CC__)
#pragma interface
#endif    // __GNUG__

#include <string.h>
#include "gmisc.h"

class gbtInput;
class gbtOutput;

class gbtText   {
  friend gbtInput &operator>>(gbtInput &, gbtText &);
  friend gbtOutput &operator<<(gbtOutput &, const gbtText &);
  protected:
    char *storage;

    // PRIVATE CONSTRUCTORS
    gbtText(int len);

  public:
    // INDEX ERROR HANDLER CLASS
    class BadIndex : public gbtException  {
      public:
      virtual ~BadIndex()  { }
      gbtText Description(void) const;
     };

    // CONSTRUCTORS, DESTRUCTORS, AND CONSTRUCTIVE OPERATORS
    gbtText(void);
    gbtText(char c);
    gbtText(const char *s);
    gbtText(const gbtText &s);
    ~gbtText()     { delete [] storage; }
    gbtText &operator=(const char *s);
    gbtText &operator=(const gbtText &s);

	  // MEMBER FUNCTIONS
    unsigned int Length(void) const    { return strlen(storage); }
    gbtText Right(int len) const;
    gbtText Left(int len) const;
    gbtText Mid(int len, int where) const;
    gbtText Upcase(void) const;
    gbtText Dncase(void) const;
    void Insert(char c, unsigned int n);
    void Remove(unsigned int n);
    int LastOccur(char c);

	  // CONCATENATORS
    gbtText &operator+=(char c);
    gbtText &operator+=(const char *s);
    gbtText &operator+=(const gbtText &s)   { *this += s.storage; return *this; }
    gbtText operator+(char c) const;
    gbtText operator+(const char *s) const;
    gbtText operator+(const gbtText &s) const { return *this + s.storage; }

	  // RELATIONAL OPERATORS
    bool operator==(const gbtText &s) const
      { return strcmp(storage, s.storage) == 0; }
    bool operator!=(const gbtText &s) const
      { return strcmp(storage, s.storage) != 0; }
    bool operator< (const gbtText &s) const
      { return strcmp(storage, s.storage) <  0; }
    bool operator> (const gbtText &s) const
      { return strcmp(storage, s.storage) >  0; }
    bool operator<=(const gbtText &s) const
      { return strcmp(storage, s.storage) <= 0; }
    bool operator>=(const gbtText &s) const
      { return strcmp(storage, s.storage) >= 0; }
    
    bool operator==(const char *s) const     { return strcmp(storage, s) == 0; }
    bool operator!=(const char *s) const     { return strcmp(storage, s) != 0; }
    bool operator< (const char *s) const     { return strcmp(storage, s) <  0; }
    bool operator> (const char *s) const     { return strcmp(storage, s) >  0; }
    bool operator<=(const char *s) const     { return strcmp(storage, s) <= 0; }
    bool operator>=(const char *s) const     { return strcmp(storage, s) >= 0; }

	  // SUBSCRIPTORS
    char &operator[](unsigned int n)
      { if (n > strlen(storage))   throw BadIndex();
        return *(storage + n);
      }

    const char &operator[](unsigned int n) const
      { if (n > strlen(storage))   throw BadIndex();
        return *(storage + n);
      }

    char *operator+(int n)        { return storage + n; }

   	// CONVERSIONS
    operator char*() const   { return storage; }
    operator const char*() const { return storage; }

};

gbtText operator+(const char *c, const gbtText &s);

#endif   // GTEXT_H
