//
// FILE: gmisc.h -- Miscellaneous generally-useful functions
//
// $Id$
//

#ifndef GMISC_H
#define GMISC_H

//
// Defining shorthand names for some types
//
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;


typedef enum { triFALSE = 0, triTRUE = 1, triUNKNOWN = 2 } gTriState;

class gText;
class gOutput;
gOutput &operator<<(gOutput &, gTriState);

//
// A few mathematically-related functions which appear frequently
//
template <class T> class gArray;

template <class T> T gmin(const T &a, const T &b);
template <class T> T gmax(const T &a, const T &b);
template <class T> T gmax(const gArray<T> &);

class gNumber;
double abs(double a);
gNumber abs(const gNumber &);

int sign(const double &a);

//
// Generation of random numbers
//

// Set the seed of the random number generator
void SetSeed(unsigned int);

// Generate a random variable from the distribution U[0..1]
double Uniform(void);

// Generates a random number between 0 and IM exclusive of endpoints
// Adapted from _Numerical_Recipes_for_C_
#define IM 2147483647
long ran1(long* idum);

//
// Converting between strings and other datatypes
//

class gInteger;
class gRational;

void ToTextWidth(int); // Set # of decimal places for floating point
int  ToTextWidth(void); // Get the current value of the above
void ToTextPrecision(int); // Set # of decimal places for floating point
int  ToTextPrecision(void); // Get the current value of the above

gText ToText(int);
gText ToText(long);
gText ToText(double);
gText ToText(double p_number, int p_precision);
gText ToText(const gInteger &);
gText ToText(const gRational &, bool approx = true);
gText ToText(const gNumber &);
gText ToText(const gNumber &p_number, int p_precision);
gText ToText(gTriState);

double FromText(const gText &, double &);
gRational FromText(const gText &, gRational &);
gNumber	FromText(const gText &, gNumber &);
double ToDouble(const gText &);
gNumber ToNumber(const gText &);    

//
/// Return a copy of the string with all quotes preceded by a backslash
//
gText EscapeQuotes(const gText &);

//
// Type dependent epsilon
//

void gEpsilon(double &v, int i = 8);
void gEpsilon(gRational &v, int i = 8);
void gEpsilon(gNumber &v, int i=8);

double pow(int,long);
double pow(double,long);

//
// Simple class for compact reference to pairs of indices
//

class index_pair {
private:
  const int first;
  const int second;
  
public:
  index_pair(const int&, const int&);
  ~index_pair();
  
  bool operator == (const index_pair&) const;
  bool operator != (const index_pair&) const;
  int operator [] (const int&) const; 

  friend gOutput& operator << (gOutput& output, const index_pair& x);  
};

class gException   {
public:
  virtual ~gException();
  
  virtual gText Description(void) const = 0;
};

class gNewFailed : public gException   {
public:
  virtual ~gNewFailed()   { }
  gText Description(void) const;
}; 

#ifdef __BCC55__
#define TEMPLATE_SPECIALIZATION()   template<>
#else
#define TEMPLATE_SPECIALIZATION()
#endif  // __BCC55__


#endif    // GMISC_H

