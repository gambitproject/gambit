//
// FILE: gpoly.cc -- Instantiation of gPoly class
//
// $Id$
//

#include "gpoly.imp"
#ifdef GDOUBLE
#include "double.h"
#endif   // GDOUBLE
#include "rational.h"
#include "gnumber.h"

double gPoly<double>::String_Coeff(double nega)
{
  double doub;
  gText Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * FromText(Coeff,doub));  
}

#ifdef GDOUBLE
gDouble gPoly<gDouble>::String_Coeff(gDouble nega)
{
  double doub;
  gText Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * (gDouble)FromText(Coeff,doub));  
}
#endif   // GDOUBLE

gRational gPoly<gRational>::String_Coeff(gRational nega)
{
  gRational rat;
  gText Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '/' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * FromText(Coeff,rat));  
}


gNumber gPoly<gNumber>::String_Coeff(gNumber nega)
{
  gNumber num;
  gText Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '/' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }

  if (Coeff == "") return (nega);
  else return (nega * gNumber(FromText(Coeff, num)));  
}

template class gPoly<gRational>;
template gPoly<gRational> operator*(const gRational val, const gPoly<gRational> &poly);
template gPoly<gRational> operator*(const gPoly<gRational> &poly, const gRational val);

#ifdef GDOUBLE
template gPoly<gDouble> TogDouble(const gPoly<gRational>&);
template gPoly<gDouble> NormalizationOfPoly(const gPoly<gRational>&);
#endif   // GDOUBLE
template gOutput &operator<<(gOutput &f, const gPoly<gRational> &y);
template gText &operator<<(gText &, const gPoly<gRational> &);

template class gPoly<gNumber>;
template gPoly<gNumber> operator*(const gNumber val, const gPoly<gNumber> &poly);
template gPoly<gNumber> operator*(const gPoly<gNumber> &poly, const gNumber val);

#ifdef GDOUBLE
template gPoly<gDouble> TogDouble(const gPoly<gNumber>&);
template gPoly<gDouble> NormalizationOfPoly(const gPoly<gNumber>&);
#endif   // GDOUBLE
template gOutput &operator<<(gOutput &f, const gPoly<gNumber> &y);
template gText &operator<<(gText &, const gPoly<gNumber> &);

template class gPoly<double>;
template gPoly<double> operator*(const double val, const gPoly<double> &poly);
template gPoly<double> operator*(const gPoly<double> &poly, const double val);

#ifdef GDOUBLE
template gPoly<gDouble> TogDouble(const gPoly<double>&);
template gPoly<gDouble> NormalizationOfPoly(const gPoly<double>&);
#endif   // GDOUBLE
template gOutput &operator<<(gOutput &f, const gPoly<double> &y);
template gText &operator<<(gText &, const gPoly<double> &);

#ifdef GDOUBLE
template class gPoly<gDouble>;
template gPoly<gDouble> operator*(const gDouble val, const gPoly<gDouble> &poly);
template gPoly<gDouble> operator*(const gPoly<gDouble> &poly, const gDouble val);
template gPoly<gDouble> operator+(const gDouble val, const gPoly<gDouble> &poly);
template gPoly<gDouble> operator+(const gPoly<gDouble> &poly, const gDouble val);

template gPoly<gDouble> TogDouble(const gPoly<gDouble>&);
template gPoly<gDouble> NormalizationOfPoly(const gPoly<gDouble>&);
template gOutput &operator<<(gOutput &f, const gPoly<gDouble> &y);
template gText &operator<<(gText &, const gPoly<gDouble> &);
#endif   // GDOUBLE
/*
template class gPoly<long>;
template gPoly<long> operator*(const long val, const gPoly<long> poly);
template gPoly<long> operator*(const gPoly<long> poly, const long val);
template gPoly<gDouble> TogDouble(const gPoly<long>&);
template gPoly<gDouble> NormalizationOfPoly(const gPoly<long>&);
template gOutput &operator<<(gOutput &f, const gPoly<long> &y);
*/

/*
int gPoly<int>::String_Coeff(int nega)
{
  gText Coeff = "";
  while (charc >= '0' && charc <= '9'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * atoi(Coeff.stradr()));  
}

long gPoly<long>::String_Coeff(long nega)
{
  gText Coeff = "";
  while (charc >= '0' && charc <= '9'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * atol(Coeff.stradr()));  
}
*/

gText ToText(const gPoly<gNumber> &p)
{
  gText t;
  t << p;
  return t;
}


#include "glist.imp"
#include "garray.imp"
#include "gblock.imp"

template class gArray< gPoly< int > * >;
template class gArray< gPoly< double > * >;
template class gArray< gPoly< gRational > *>;
template class gArray< Variable * >;

template class gBlock<Variable *>;

template class gList< gPoly<int> * >;
template class gList< gPoly<gRational> * >;
template class gList< gPoly<double> * >;
template class gList<gPoly<gRational> >;
template class gList<gPoly<gNumber> >;
template class gList<gPoly<gNumber> * >;
template gOutput& operator << (gOutput& output, 
			       const gList<gPoly<gRational> >&);
template gOutput& operator << (gOutput& output, 
			       const gList<gPoly<gNumber> >&);

template class gList<gPoly<double> >;
template gOutput& operator << (gOutput& output, 
			       const gList<gPoly<double> >&);

#ifdef GDOUBLE
template class gList< gPoly<gDouble> * >;
template class gList< gPoly<gDouble> >;
template class gList<gVector<gDouble> >;
template gOutput& operator << (gOutput& output, 
			       const gList<gVector<gDouble> >&);

#endif   // GDOUBLE



