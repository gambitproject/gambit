//#
//# FILE: gambitio.h -- Declaration of input and output classes
//#
//# $Id$
//#

#ifndef GAMBITIO_H
#define GAMBITIO_H

#include <stdio.h>
#include <assert.h>

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

//
// <category lib=glib sect="Input/Output">
//
// This class is a (simple) implementation of an input stream.  It is built
// around the C stdio library in preference to the C++ iostream library
// since some compilers (notable GNU C++) suffer from executable bloat
// when iostreams are used.
//
// This class should be used preferentially over a pointer to FILE.  In
// particular, the destructor is responsible for closing the file, thus
// avoiding dangling open files.
//
// <note> The copy constructor and assignment operator are declared private
//        to the class so that it is not possible to get multiple instances
//        of gInput referencing the same file, which would be a Bad Thing (tm)
//        in view of the semantics of the destructor.  When needing to pass
//        a stream as a parameter to a function, use a parameter of type
//        class gInput &.
//
class gInput  {
  private:
    FILE *f;

//
// The copy constructor and assignment operator are declared private to
// override the default meanings of these functions.  They are never
// implemented.
//+grp
    gInput(const gInput &);
    gInput &operator=(const gInput &);
//-grp

  public:
//
// The default constructor, initializing the instance to point to no stream
//
    gInput(void)   { f = 0; }
//
// Initialize the instance to point to the stream with a certain filename.
//
// <note> This is the preferred way to initialize an instance of this class.
//
    gInput(const char *in)   { f = fopen(in, "r"); }
//
// Initialize the instance to point to a given file.  This is provided
// principally for compatibility with existing code.
//
    gInput(FILE *in)   { f = in; }

//
// Close the file pointed to, if any.
//
    ~gInput()   { if (f)   fclose(f); }

//
// Set the current file to the given file.  Closes the currently referenced
// file, if any.
//
    gInput& operator=(FILE *in)
      { if (f)  fclose(f);   f = in;  return *this; }
//
// Set the current file to the file with a certain filename.  Closes the
// currently referenced file, if any.
//
// <note> This is the preferred way to change the file to which an instance
// points.
//
    gInput& operator=(const char *in)
      { if (f)  fclose(f);   f = fopen(in, "r");  return *this; }

//
// Tests for the equality of two instantiations.  They are considered
// equal if the file buffer they point to is the same one.
//+grp
    int operator==(const gInput &ip) const   { return (f == ip.f); }
    int operator!=(const gInput &ip) const   { return (f != ip.f); }
//-grp

//
// Input primitives for the builtin types.
//
// <note> The operator for char * assumes that the area in memory pointed
//        to by the pointer is large enough to hold the string read.
//+grp
    gInput& operator>>(int &x)
      { assert(f);  fscanf(f, "%d", &x);   return *this; }
    gInput& operator>>(unsigned int &x)
      { assert(f);  fscanf(f, "%d", &x);   return *this; }
    gInput& operator>>(long &x)
      { assert(f);  fscanf(f, "%ld", &x);   return *this; }
    gInput& operator>>(char &x)
      { assert(f);  fscanf(f, "%c", &x);   return *this; }
    gInput& operator>>(double &x)
      { assert(f);  fscanf(f, "%lf", &x);  return *this; }
    gInput& operator>>(float &x)
      { assert(f);  fscanf(f, "%f", &x);   return *this; }
    gInput& operator>>(char *x)
      { assert(f);  fscanf(f, "%s", x);    return *this; }
//-grp

    int get(char &c)   { c = fgetc(f);  return (!feof(f)); }

//
// Unget the character given.  Only one character of pushback is guaranteed.
//
    void unget(char c)
      { ::ungetc(c, f); }

//
// Returns nonzero if the end-of-file marker has been reached.
//
    int eof(void) const
      { return feof(f); }
};

extern gInput gin;

//
// This class is a (simple) implementation of an output stream.  It is built
// around the C stdio library in preference to the C++ iostream library
// since some compilers (notable GNU C++) suffer from executable bloat
// when iostreams are used.
//
// This class should be used preferentially over a pointer to FILE.  In
// particular, the destructor is responsible for closing the file, thus
// avoiding dangling open files.
//
// <note> The copy constructor and assignment operator are declared private
//        to the class so that it is not possible to get multiple instances
//        of gOutput referencing the same file, which would be a Bad Thing (tm)
//        in view of the semantics of the destructor.  When needing to pass
//        a stream as a parameter to a function, use a parameter of type
//        class gOutput &.
//
class gOutput  {
  private:
    FILE *f;

//
// The copy constructor and assignment operator are declared private to
// override the default meanings of these functions.  They are never
// implemented.
//+grp
    gOutput(const gOutput &);
    gOutput &operator=(const gOutput &);
//-grp

  public:
//
// The default constructor, initializing the instance to point to no stream
//
    gOutput(void)   { f = 0; }
//
// Initialize the instance to point to the stream with a certain filename.
//
// <note> This is the preferred way to initialize an instance of this class.
//
    gOutput(const char *out)   { f = fopen(out, "w"); }
//
// Initialize the instance to point to a given file.  This is provided
// principally for compatibility with existing code.
//
    gOutput(FILE *out)   { f = out; } 
//
// Close the file pointed to, if any.
//
    ~gOutput()   { if (f)   fclose(f); }


//
// Set the current file to the given file.  Closes the currently referenced
// file, if any.
//
    gOutput& operator=(FILE *out)
      { if (f)  fclose(f);  f = out;  return *this; }

//
// Tests for the equality of two instantiations.  They are considered
// equal if the file buffer they point to is the same one.
//+grp
    int operator==(const gOutput &out) const   { return (f == out.f); }
    int operator!=(const gOutput &out) const   { return (f != out.f); }
//-grp

//
// Output primitives for the basic types
//+grp
    gOutput& operator<<(int x)
      { assert(f);  fprintf(f, "%d", x);  return *this; }
    gOutput& operator<<(unsigned int x)
      { assert(f);  fprintf(f, "%d", x);  return *this; }
    gOutput& operator<<(long x)
      { assert(f);  fprintf(f, "%ld", x);  return *this; }
    gOutput& operator<<(char x)
      { assert(f);  fprintf(f, "%c", x);  return *this; }
    gOutput& operator<<(double x)
      { assert(f);  fprintf(f, "%lf", x);  return *this; }
    gOutput& operator<<(float x)
      { assert(f);  fprintf(f, "%f", x);  return *this; }
    gOutput& operator<<(const char *x)
      { assert(f);  fprintf(f, "%s", x);  return *this; }
    gOutput& operator<<(const void *x)
      { assert(f);  fprintf(f, "%p", x); return *this; }
//-grp
};

extern gOutput gout, gerr;

#endif   // GAMBITIO_H
