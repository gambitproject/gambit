//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Definition of gText class
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

#ifdef __GNUG__
#pragma interface
#endif    // __GNUG__

#include <string.h>
#include "gmisc.h"

class gInput;
class gOutput;

class gText   {
  friend gInput &operator>>(gInput &, gText &);
  friend gOutput &operator<<(gOutput &, const gText &);
  protected:
    char *storage;

    // PRIVATE CONSTRUCTORS
    gText(int len);

  public:
    // INDEX ERROR HANDLER CLASS
    class BadIndex : public gException  {
      public:
      virtual ~BadIndex()  { }
      gText Description(void) const;
     };

    // CONSTRUCTORS, DESTRUCTORS, AND CONSTRUCTIVE OPERATORS
    gText(void);
    gText(char c);
    gText(const char *s);
    gText(const gText &s);
    ~gText()     { delete [] storage; }
    gText &operator=(const char *s);
    gText &operator=(const gText &s);

	  // MEMBER FUNCTIONS
    unsigned int Length(void) const    { return strlen(storage); }
    gText Right(int len) const;
    gText Left(int len) const;
    gText Mid(int len, int where) const;
    gText Upcase(void) const;
    gText Dncase(void) const;
    void Insert(char c, unsigned int n);
    void Remove(unsigned int n);
    int LastOccur(char c);

	  // CONCATENATORS
    gText &operator+=(char c);
    gText &operator+=(const char *s);
    gText &operator+=(const gText &s)   { *this += s.storage; return *this; }
    gText operator+(char c) const;
    gText operator+(const char *s) const;
    gText operator+(const gText &s) const { return *this + s.storage; }

	  // RELATIONAL OPERATORS
    bool operator==(const gText &s) const
      { return strcmp(storage, s.storage) == 0; }
    bool operator!=(const gText &s) const
      { return strcmp(storage, s.storage) != 0; }
    bool operator< (const gText &s) const
      { return strcmp(storage, s.storage) <  0; }
    bool operator> (const gText &s) const
      { return strcmp(storage, s.storage) >  0; }
    bool operator<=(const gText &s) const
      { return strcmp(storage, s.storage) <= 0; }
    bool operator>=(const gText &s) const
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

gText operator+(const char *c, const gText &s);

#endif   // GTEXT_H


