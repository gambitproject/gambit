//
// FILE: gstring.h -- Definition of gString class
//
// $Id$
//

#ifndef GSTRING_H
#define GSTRING_H

#ifdef __GNUG__
#pragma interface
#endif    // __GNUG__

#include <string.h>
#include <assert.h>

class gInput;
class gOutput;

class gString   {
  friend gInput& operator>>(gInput&, gString&);
  friend gOutput& operator<<(gOutput&, const gString&);
  protected:
    char *storage;

    // PRIVATE CONSTRUCTORS
      gString(int len);

  public:
    // CONSTRUCTORS
    gString(void);
    gString(char c);
    gString(const char *s);
    gString(const gString& s);

	// MEMBER FUNCTIONS
    char *stradr(void)      { return storage; }
    int length(void) const    { return strlen(storage); }
    gString right(int len) const;
    gString left(int len) const;
    gString mid(int len, int where) const;
    gString upcase(void) const;
    gString dncase(void) const;
    void insert(char c, unsigned int n);
    void remove(unsigned int n);
    int lastOccur(char c);

	// ASSIGNMENTS
    gString &operator=(const char *s);
    gString &operator=(const gString& s)   { *this = s.storage; return *this; }

	// CONCATENATORS
    void operator+=(char c);
    void operator+=(const char *s);
    void operator+=(const gString& s)   { *this += s.storage; }
    gString operator+(char c) const;
    gString operator+(const char *s) const;
    gString operator+(const gString& s) const { return *this + s.storage; }

	// RELATIONAL OPERATORS
    int operator==(const gString& s) const
      { return strcmp(storage, s.storage) == 0; }
    int operator!=(const gString& s) const
      { return strcmp(storage, s.storage) != 0; }
    int operator< (const gString& s) const
      { return strcmp(storage, s.storage) <  0; }
    int operator> (const gString& s) const
      { return strcmp(storage, s.storage) >  0; }
    int operator<=(const gString& s) const
      { return strcmp(storage, s.storage) <= 0; }
    int operator>=(const gString& s) const
      { return strcmp(storage, s.storage) >= 0; }
    
    int operator==(const char *s) const     { return strcmp(storage, s) == 0; }
    int operator!=(const char *s) const     { return strcmp(storage, s) != 0; }
    int operator< (const char *s) const     { return strcmp(storage, s) <  0; }
    int operator> (const char *s) const     { return strcmp(storage, s) >  0; }
    int operator<=(const char *s) const     { return strcmp(storage, s) <= 0; }
    int operator>=(const char *s) const     { return strcmp(storage, s) >= 0; }

	// SUBSCRIPTORS
    char& operator[](unsigned int n)    
     { assert(n <= strlen(storage));  return *(storage + n); }
    char operator[](unsigned int n) const 
     { assert(n <= strlen(storage));  return *(storage + n); }
    char* operator+(int n)       { return storage + n; }

	// CONVERSIONS
    operator char*() const   { return storage; }
    operator const char*() const { return storage; }

	// DESTRUCTOR
    ~gString()     { delete [] storage; }
};

inline gString operator+(const char *c, const gString &s)
{
    return gString(c) + s;
}

#endif   // STRING_H


