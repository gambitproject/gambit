
#ifndef GAMBITIO_H
#define GAMBITIO_H

#include <stdio.h>
#include <assert.h>

class input  {
  private:
    FILE *f;

  public:
	// CONSTRUCTORS AND DESTRUCTOR
    input(void)   { f = 0; }
    input(const char *in)   { f = fopen(in, "r"); }
    input(FILE *in)   { f = in; }
    input(const input &ip)   { assert(0); }
    ~input()   { if (f)   fclose(f); }

	// OPERATOR OVERLOADING
    input& operator=(FILE *in)
      { if (f)  fclose(f);   f = in;  return *this; }
    input& operator=(const input &)
	  // we don't want to allow this operation at all...
      { assert(0);   return *this; }
    int operator==(const input &ip) const   { return (f == ip.f); }
    int operator!=(const input &ip) const   { return (f != ip.f); }

	//defining the Input operators for basic types
    input& operator>>(int &x)
      { fscanf(f, "%d", &x);   return *this; }
    input& operator>>(unsigned int &x)
      { fscanf(f, "%d", &x);   return *this; }
    input& operator>>(long &x)
      { fscanf(f, "%ld", &x);   return *this; }
    input& operator>>(char &x)
      { fscanf(f, "%c", &x);   return *this; }
    input& operator>>(double &x)
      { fscanf(f, "%lf", &x);  return *this; }
    input& operator>>(float &x)
      { fscanf(f, "%f", &x);   return *this; }
    input& operator>>(char *x)
      { fscanf(f, "%s", x);    return *this; }

	// other useful operations
    void unget(char c)
      { ::ungetc(c, f); }
};

extern input gin;

class output  {
  private:
    FILE *f;

  public:
	// CONSTRUCTORS AND DESTRUCTOR
    output(void)   { f = 0; }
    output(const char *out)   { f = fopen(out, "w"); }
    output(FILE *out)   { f = out; } 
    output(const output &op)   { assert(0); }
    ~output()   { if (f)   fclose(f); }

	// OPERATOR OVERLOADING
    output& operator=(FILE *out)
      { if (f)  fclose(f);  f = out;  return *this; }
    output& operator=(const output &)
	  // we never want to allow this operation!!
      { assert(0);  return *this; }
    int operator==(const output &out) const   { return (f == out.f); }
    int operator!=(const output &out) const   { return (f != out.f); }

	//defining the Output operator for basic types
    output& operator<<(int x)
      { fprintf(f, "%d", x);  return *this; }
    output& operator<<(unsigned int x)
      { fprintf(f, "%d", x);  return *this; }
    output& operator<<(long x)
      { fprintf(f, "%ld", x);  return *this; }
    output& operator<<(char x)
      { fprintf(f, "%c", x);  return *this; }
    output& operator<<(double x)
      { fprintf(f, "%lf", x);  return *this; }
    output& operator<<(float x)
      { fprintf(f, "%f", x);  return *this; }
    output& operator<<(char *x)
      { fprintf(f, "%s", x);  return *this; }
    output& operator<<(void *x)
      { fprintf(f, "%p", x); return *this; }
};

extern output gout, gerr;

#endif   // GAMBITIO_H
