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

template class gPoly<gRational>;
template gPoly<gRational> operator*(const gRational val, const gPoly<gRational> &poly);
template gPoly<gRational> operator*(const gPoly<gRational> &poly, const gRational val);

#ifdef GDOUBLE
template gPoly<gDouble> TogDouble(const gPoly<gRational>&);
#endif   // GDOUBLE
template gOutput &operator<<(gOutput &f, const gPoly<gRational> &y);
template gString &operator<<(gString &, const gPoly<gRational> &);

template class gPoly<gNumber>;
template gPoly<gNumber> operator*(const gNumber val, const gPoly<gNumber> &poly);
template gPoly<gNumber> operator*(const gPoly<gNumber> &poly, const gNumber val);

#ifdef GDOUBLE
template gPoly<gDouble> TogDouble(const gPoly<gNumber>&);
#endif   // GDOUBLE
template gOutput &operator<<(gOutput &f, const gPoly<gNumber> &y);
template gString &operator<<(gString &, const gPoly<gNumber> &);

template class gPoly<double>;
template gPoly<double> operator*(const double val, const gPoly<double> &poly);
template gPoly<double> operator*(const gPoly<double> &poly, const double val);

#ifdef GDOUBLE
template gPoly<gDouble> TogDouble(const gPoly<double>&);
#endif   // GDOUBLE
template gOutput &operator<<(gOutput &f, const gPoly<double> &y);
template gString &operator<<(gString &, const gPoly<double> &);

#ifdef GDOUBLE
template class gPoly<gDouble>;
template gPoly<gDouble> operator*(const gDouble val, const gPoly<gDouble> poly);
template gPoly<gDouble> operator*(const gPoly<gDouble> poly, const gDouble val);

template gPoly<gDouble> TogDouble(const gPoly<gDouble>&);
template gOutput &operator<<(gOutput &f, const gPoly<gDouble> &y);
template gString &operator<<(gString &, const gPoly<gDouble> &);
#endif   // GDOUBLE
/*
template class gPoly<long>;
template gPoly<long> operator*(const long val, const gPoly<long> poly);
template gPoly<long> operator*(const gPoly<long> poly, const long val);
template gPoly<gDouble> TogDouble(const gPoly<long>&);
template gOutput &operator<<(gOutput &f, const gPoly<long> &y);
*/

/*
int gPoly<int>::String_Coeff(int nega)
{
  gString Coeff = "";
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
  gString Coeff = "";
  while (charc >= '0' && charc <= '9'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * atol(Coeff.stradr()));  
}
*/

double gPoly<double>::String_Coeff(double nega)
{
  double doub;
  gString Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * FromString(Coeff,doub));  
}

#ifdef GDOUBLE
gDouble gPoly<gDouble>::String_Coeff(gDouble nega)
{
  double doub;
  gString Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * (gDouble)FromString(Coeff,doub));  
}
#endif   // GDOUBLE

gRational gPoly<gRational>::String_Coeff(gRational nega)
{
  gRational rat;
  gString Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '/' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * FromString(Coeff,rat));  
}


gNumber gPoly<gNumber>::String_Coeff(gNumber nega)
{
  gNumber num;
  gString Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '/' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }

  if (Coeff == "") return (nega);
  else return (nega * gNumber(FromString(Coeff, num)));  
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
template class gNode< gPoly<int> * >;
template class gList< gPoly<gRational> * >;
template class gNode< gPoly<gRational> * >;
template class gList< gPoly<double> * >;
template class gNode< gPoly<double> * >;
template class gList<gPoly<gRational> >;
template class gNode<gPoly<gRational> >;
template gOutput& operator << (gOutput& output, 
			       const gList<gPoly<gRational> >&);

//template class gList<gDouble>;
//template class gNode<gDouble>;
#ifdef GDOUBLE
template class gList< gPoly<gDouble> * >;
template class gNode< gPoly<gDouble> * >;
template class gList< gPoly<gDouble> >;
template class gNode< gPoly<gDouble> >;
template class gList<gVector<gDouble> >;
template class gNode<gVector<gDouble> >;
template gOutput& operator << (gOutput& output, 
			       const gList<gVector<gDouble> >&);

#endif   // GDOUBLE

//template class gList< gPoly<long> * >;
//template class gNode< gPoly<long> * >;




