//
// FILE: gtext.h -- Definition of gText class
//
// $Id$
//

#ifndef GTEXT_H
#define GTEXT_H

#ifdef __GNUG__
#pragma interface
#endif    // __GNUG__

#include <string.h>
#include <assert.h>
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
#ifdef USE_EXCEPTIONS
    // INDEX ERROR HANDLER CLASS
    class BadIndex : public gException  {
      public:
      BadIndex(int, char *);

      virtual ~BadIndex()  { }
      gText Description(void) const;
     };
#endif   // USE_EXCEPTIONS

    // CONSTRUCTORS, DESTRUCTORS, AND CONSTRUCTIVE OPERATORS
    gText(void);
    gText(char c);
    gText(const char *s);
    gText(const gText &s);
    ~gText()     { delete [] storage; }
    gText &operator=(const char *s);
    gText &operator=(const gText &s)   { *this = s.storage; return *this; }

	  // MEMBER FUNCTIONS
    int Length(void) const    { return strlen(storage); }
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
#ifdef USE_EXCEPTIONS
    char &operator[](unsigned int n)
      { if (n > strlen(storage))   throw BadIndex(__LINE__, __FILE__);
        return *(storage + n);
      }
#else
    char &operator[](unsigned int n)      
      { assert(n <= strlen(storage));
	return *(storage + n);
      }
#endif   // USE_EXCEPTIONS

#ifdef USE_EXCEPTIONS
    char &operator[](unsigned int n) const
      { if (n > strlen(storage))   throw BadIndex(__LINE__, __FILE__);
        return *(storage + n);
      }
#else
    char &operator[](unsigned int n) const      
      { assert(n <= strlen(storage));
	return *(storage + n);
      }
#endif   // USE_EXCEPTIONS

    char *operator+(int n)        { return storage + n; }

   	// CONVERSIONS
    operator char*() const   { return storage; }
    operator const char*() const { return storage; }

};

gText operator+(const char *c, const gText &s);

#endif   // GTEXT_H


