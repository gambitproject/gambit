%module rational
%include typemaps.i
  
%{
#include "../math/rational.h"
%}

%name(Rational) class gbtRational {
public:
  gbtRational(void);
  %name(Rational_Int) gbtRational(int);
  %name(Rational_Float) gbtRational(float);
  %name(Rational_Ratio) gbtRational(const gbtInteger &,
				       const gbtInteger &);
  ~gbtRational();
};

%extend gbtRational {
  std::string __str__(void)   { return ToText(*self); }  
  std::string __repr__(void)  { return ToText(*self); }   

  bool __lt__(const gbtRational &x) { return *self < x; }
  bool __le__(const gbtRational &x) { return *self <= x; }
  bool __eq__(const gbtRational &x) { return *self == x; }
  bool __ne__(const gbtRational &x) { return *self != x; }
  bool __gt__(const gbtRational &x) { return *self > x; }
  bool __ge__(const gbtRational &x) { return *self >= x; }

  gbtRational __add__(const gbtRational &x)  { return *self + x; }
  gbtRational __sub__(const gbtRational &x)  { return *self - x; }
  gbtRational __mul__(const gbtRational &x)  { return *self * x; }
  gbtRational __div__(const gbtRational &x)  { return *self / x; }
  
  double __float__(void)
  { return (double) *self; }
             
}

