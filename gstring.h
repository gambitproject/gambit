//
// FILE: string.h -- Definition of String class
//
// $Id$
//

#ifndef GSTRING_H
#define GSTRING_H

#include <stdio.h>
#include <string.h>

class gString   {
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
    char *stradr(void)   { return storage; }
    int length(void) const    { return strlen(storage); }
    gString right(int len) const;
    gString left(int len) const;
    gString mid(int len, int where) const;
    gString upcase(void) const;
    gString dncase(void) const;
    void insert(char c, int n);
    void remove(int n);

	// ASSIGNMENTS
    void operator=(const char *s);
    void operator=(const gString& s)   { *this = s.storage; }

	// CONCATENATORS
    void operator+=(const char *s);
    void operator+=(const gString& s)   { *this += s.storage; }
    gString operator+(const char *s);
    gString operator+(const gString& s)  { return *this + s.storage; }

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
    char& operator[](int n)       { return *(storage + n); }
    char* operator+(int n)       { return storage + n; }

	// CONVERSIONS
    operator const char*() const { return storage; }

	// DESTRUCTOR
    virtual ~gString()     { delete [] storage; }
};

inline gString operator+(const char *c, const gString &s)
{
    return gString(c) + s;
}

#endif   // STRING_H


