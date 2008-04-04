%module libgambit
%include file.i

// Globally turn on the autodoc feature
//%feature("autodoc", "1");  // 0 == no param types, 1 == show param types

// Borrowed from the wxPython distribution
%define DocStr(decl, docstr)
  %feature("docstring") decl docstr;
%enddef

//========================================================================
//                         Our custom typemaps
//========================================================================

%typemap(in) const Gambit::Array<int> & {
  int i;
  if (!PySequence_Check($input)) {
    PyErr_SetString(PyExc_ValueError, "Expected a sequence");
    return NULL;
  }
  $1 = new Array<int>(PySequence_Length($input));
  for (i = 0; i < PySequence_Length($input); i++) {
    PyObject *o = PySequence_GetItem($input, i);
    if (PyNumber_Check(o)) {
      (*$1)[i+1] = (int) PyInt_AsLong(o);
    }
    else {
      PyErr_SetString(PyExc_ValueError, "Sequence elements must be numbers");
      return NULL;
    }
  }
}

%typemap(freearg) const Array<int> & {
  if ($1) free($1);
}

%include exception.i
%include python/std_string.i
%include python/std_iostream.i

%{
#include "libgambit/libgambit.h"

using namespace Gambit;

%}

%exception {
  try {
    $action
  }
  catch (Gambit::IndexException &) {
    SWIG_exception(SWIG_IndexError, "index out of range");
  }
  catch (Gambit::NullException &) {
    SWIG_exception(SWIG_RuntimeError, "operating on null object");
  }
  catch (...) {
    SWIG_exception(SWIG_RuntimeError, "uncaught runtime error");
  }
}

%ignore Exception;

// A boatload of functions, mostly from integer.h, that we don't want to
// bother with (and which cause newer versions of SWIG to complain)
%ignore pow;
%ignore sign;
%ignore Gambit::pow;
%ignore Gambit::sign;
%ignore Gambit::lg;
%ignore Gambit::ratio;
%ignore Gambit::gcd;
%ignore Gambit::even;
%ignore Gambit::odd;
%ignore Gambit::setbit;
%ignore Gambit::clearbit;
%ignore Gambit::testbit;
%ignore Gambit::abs;
%ignore Gambit::negate;
%ignore Gambit::complement;
%ignore Gambit::compare;
%ignore Gambit::ucompare;
%ignore Gambit::add;
%ignore Gambit::sub;
%ignore Gambit::mul;
%ignore Gambit::div;
%ignore Gambit::mod;
%ignore Gambit::divide;
%ignore Gambit::lshift;
%ignore Gambit::rshift;
%ignore Gambit::Itoa;
%ignore Gambit::atoI;
%ignore Gambit::operator>>;
%ignore Gambit::operator<<;
%ignore Gambit::sqr;
%ignore Gambit::floor;
%ignore Gambit::ceil;
%ignore Gambit::trunc;
%ignore Gambit::round;
%ignore *::operator=;
%ignore *::operator double;
%rename(plusplus) *::operator++;
%ignore *::operator--;
%ignore *::Itoa;
%ignore *::atoIntegerRep;
%ignore *::operator[];
%ignore operator<<;
%ignore operator>>;
%ignore operator Gambit::GameAction;
%ignore operator Gambit::GameInfoset;
%ignore operator Gambit::GameStrategy;
%ignore operator Gambit::GamePlayer;
%ignore operator const Gambit::GameStrategy &;



%include <libgambit/libgambit.h>

%include <libgambit/array.h>
%template(ArrayDouble) Gambit::Array<double>;
%template(ArrayRational) Gambit::Array<Gambit::Rational>;

%include <libgambit/vector.h>
%template(VectorDouble) Gambit::Vector<double>;
%template(VectorRational) Gambit::Vector<Gambit::Rational>;

%include <libgambit/pvector.h>
%template(PVectorDouble) Gambit::PVector<double>;
%template(PVectorRational) Gambit::PVector<Gambit::Rational>;

%include <libgambit/dvector.h>
%template(DVectorDouble) Gambit::DVector<double>;
%template(DVectorRational) Gambit::DVector<Gambit::Rational>;

//========================================================================
//                               Integer
//========================================================================

%include <libgambit/integer.h>
%extend Gambit::Integer {
  std::string __str__(void)   { return ToText(*self); }
  std::string __repr__(void)  { return ToText(*self); }

  // Right-side operators (a nifty Python feature!)
  Integer __radd__(long lhs) const { return Integer(lhs) + *self; }
  Integer __rsub__(long lhs) const { return Integer(lhs) - *self; }
  Integer __rmul__(long lhs) const { return Integer(lhs) * *self; }
  Integer __rdiv__(long lhs) const { return Integer(lhs) / *self; } 

  long __int__(void)   { return self->as_long(); }
};

//========================================================================
//                               Rational
//========================================================================

%include <libgambit/rational.h>

%extend Gambit::Rational {
  std::string __str__(void)   { return ToText(*self); }  
  std::string __repr__(void)  { return ToText(*self); }   

  // Explicit overloads of some operators
  // This combines with SWIG's overloading-handling features to
  // allow operations between rationals and other types
  double operator+(double y) const { return (double) *self + y; }
  double operator-(double y) const { return (double) *self - y; }
  double operator*(double y) const { return (double) *self * y; }
  double operator/(double y) const { return (double) *self / y; }

  Rational operator+=(double y)   { return *self += Rational(y); }
  Rational operator-=(double y)   { return *self -= Rational(y); }
  Rational operator*=(double y)   { return *self *= Rational(y); }
  Rational operator/=(double y)   { return *self /= Rational(y); }


  // Additional Python operators
  Rational __pos__(void)  { return *self; }

  // Right-side operators (a nifty Python feature!)
  double __radd__(double lhs) const { return lhs + (double) *self; }
  double __rsub__(double lhs) const { return lhs - (double) *self; }
  double __rmul__(double lhs) const { return lhs * (double) *self; }
  double __rdiv__(double lhs) const { return lhs / (double) *self; } 

  double __float__(void)   { return (double) *self; }
  Rational __abs__(void)  { return fabs(*self); }
}


//========================================================================
//                             Game classes
//========================================================================

%ignore *::operator!;

%include <libgambit/game.h>

%ignore Gambit::GameObjectPtr<Gambit::GameOutcomeRep>::operator Gambit::GameOutcomeRep *;
%template(GameOutcome) Gambit::GameObjectPtr<Gambit::GameOutcomeRep>;

%extend Gambit::GameOutcomeRep {
  Rational GetPayoff(int pl) const { return self->GetPayoff<Rational>(pl); }
  void SetPayoff(int pl, const Rational &payoff)
  { self->SetPayoff(pl, Gambit::ToText(payoff)); }
  void SetPayoff(int pl, double payoff)
  { self->SetPayoff(pl, Gambit::ToText(payoff)); }
  void SetPayoff(int pl, const std::string &payoff)
  { self->SetPayoff(pl, payoff); }
};

%ignore Gambit::GameObjectPtr<Gambit::GameStrategyRep>::operator Gambit::GameStrategyRep *;
%template(GameStrategy) Gambit::GameObjectPtr<Gambit::GameStrategyRep>;

%ignore Gambit::GameObjectPtr<Gambit::GameActionRep>::operator Gambit::GameActionRep *;
%template(GameAction) Gambit::GameObjectPtr<Gambit::GameActionRep>;

%ignore Gambit::GameObjectPtr<Gambit::GameInfosetRep>::operator Gambit::GameInfosetRep *;
%template(GameInfoset) Gambit::GameObjectPtr<Gambit::GameInfosetRep>;

%ignore Gambit::GameObjectPtr<Gambit::GameNodeRep>::operator Gambit::GameNodeRep *;
%template(GameNode) Gambit::GameObjectPtr<Gambit::GameNodeRep>;

%ignore Gambit::GameObjectPtr<Gambit::GamePlayerRep>::operator Gambit::GamePlayerRep *;
%template(GamePlayer) Gambit::GameObjectPtr<Gambit::GamePlayerRep>;

%extend Gambit::PureStrategyProfile {
  Rational GetPayoff(const Gambit::GamePlayer &p_player) const
   { return self->GetPayoff<Rational>(p_player); }
  Rational GetStrategyValue(const Gambit::GameStrategy &p_strategy) const
   { return self->GetStrategyValue<Rational>(p_strategy); }
};

//========================================================================
//                            Iterators
//========================================================================

%extend Gambit::GameObjectIterator {
  %pythoncode %{
    def __iter__(self): return self
    def next(self):
      if self.AtEnd(): raise StopIteration
      value = self.__deref__()
      self.plusplus()       
      return value
  %}
}

%ignore *::operator Gambit::GameObjectPtr<Gambit::GameActionRep >;
%template(GameActionIterator) Gambit::GameObjectIterator<Gambit::GameActionRep, Gambit::GameAction>;

%ignore *::operator Gambit::GameObjectPtr<Gambit::GameInfosetRep >;
%template(GameInfosetIterator) Gambit::GameObjectIterator<Gambit::GameInfosetRep, Gambit::GameInfoset>;

%ignore *::operator Gambit::GameObjectPtr<Gambit::GameStrategyRep >;
%template(GameStrategyIterator) Gambit::GameObjectIterator<Gambit::GameStrategyRep, Gambit::GameStrategy>;

%ignore *::operator Gambit::GameObjectPtr<Gambit::GamePlayerRep >;
%template(GamePlayerIterator) Gambit::GameObjectIterator<Gambit::GamePlayerRep, Gambit::GamePlayer>;
 

%ignore Gambit::GameObjectPtr<Gambit::GameRep>::operator Gambit::GameRep *;
%ignore Gambit::GameRep::GameRep(void);
%ignore Gambit::GameRep::GameRep(const Array<int> &);
%ignore Gambit::GameRep::WriteEfgFile(std::ostream &);
%ignore Gambit::GameRep::WriteNfgFile(std::ostream &);

%extend Gambit::GameRep {
  std::string efg_file(void) const
  {
    std::ostringstream s;
    self->WriteEfgFile(s);
    return s.str();
  }

  std::string nfg_file(void) const
  {
    std::ostringstream s;
    self->WriteNfgFile(s);
    return s.str();
  }

  MixedStrategyProfile<double> NewMixedStrategyDouble(void)
  { return MixedStrategyProfile<double>(StrategySupport(self)); }

  MixedStrategyProfile<Gambit::Rational> NewMixedStrategyRational(void)
  { return MixedStrategyProfile<Gambit::Rational>(StrategySupport(self)); }

  MixedBehavProfile<double> NewMixedBehavDouble(void)
  { return MixedBehavProfile<double>(BehavSupport(self)); }

  MixedBehavProfile<Gambit::Rational> NewMixedBehavRational(void)
  { return MixedBehavProfile<Gambit::Rational>(BehavSupport(self)); }
}

%template(Game) Gambit::GameObjectPtr<Gambit::GameRep>;

%extend Gambit::GameObjectPtr<Gambit::GameRep> {
%pythoncode %{
  def mixed_strategy(self, rational=False):
    if rational:
      return self.NewMixedStrategyRational()
    else:
      return self.NewMixedStrategyDouble()

  def behavior_strategy(self, rational=False):
    if rational:
      return self.NewMixedBehavRational()
    else:
      return self.NewMixedBehavDouble()

  def __str__(self):
    if self.IsTree():
      return "Gambit extensive game '%s'" % self.GetTitle()
    else:
      return "Gambit strategic game '%s'" % self.GetTitle()

  def __repr__(self):  return str(self)
%}
}

//========================================================================
//                             Reading games
//========================================================================

//
// We strive here to be a bit Pythonic, and to deal with some SWIG
// complications.
// 
// First, there's no convenient way to get an std::istream from a Python
// file object.  Thus, we disable the wrapping of the ReadGame() function
// from the C++ API, and replace it with a ReadGame() function that
// acts Pythonic.  It accepts either a string object, in which case it
// simply parses that as if it were the contents of the file, or any
// object that has a .read() member, in which case it assumes that
// .read() returns the contents of the object as a game file.
//

%ignore Gambit::ReadGame(std::istream &) throw(InvalidFileException);

%{
Gambit::Game ReadGameString(const std::string &p_string)
{
  std::istringstream s(p_string);
  return Gambit::ReadGame(s);
}
%}

Gambit::Game ReadGameString(const std::string &p_string);

%pythoncode %{
def ReadGame(x):
  if isinstance(x, str):
    try:
      return ReadGameString(x)
    except RuntimeError:
      pass

    try:
      return ReadGameString(file(x).read())
    except:
      raise RuntimeError

  return ReadGameString(x.read())
%}

//========================================================================
//                            Strategy profiles
//========================================================================

%include <libgambit/mixed.h>

%ignore *::operator[];
%ignore MixedStrategyProfile<T>::MixedStrategyProfile(const MixedStrategyProfile<T> &);
%ignore MixedStrategyProfile<T>::MixedStrategyProfile(const MixedBehavProfile<T> &);

%extend Gambit::MixedStrategyProfile {
  MixedBehavProfile<T> behavior(void) const { return MixedBehavProfile<T>(*self); }

  int __len__(void) const { return self->MixedProfileLength(); }
  T __getitem__(int i) const { return (*self)[i+1]; }
  T __getitem__(const GameStrategy &s) const { return (*self)[s]; }
  void __setitem__(int i, const T &value) { (*self)[i+1] = value; }
  void __setitem__(const GameStrategy &s, const T &value) { (*self)[s] = value; }

%pythoncode %{
  def __repr__(self):
    return "Mixed strategy profile on '%s': [%s]" % \
           (self.GetGame().GetTitle(),
	    ", ".join([ str(self[i]) for i in xrange(len(self)) ]))	

  def __str__(self):   return repr(self)
%}
};

%template(MixedStrategyDouble) Gambit::MixedStrategyProfile<double>;
%template(MixedStrategyRational) Gambit::MixedStrategyProfile<Gambit::Rational>;

%include <libgambit/stratspt.h>

%extend Gambit::StrategySupport {
public:
  MixedStrategyProfile<double> NewMixedStrategyDouble(void)
  { return MixedStrategyProfile<double>(*self); }
};


%extend Gambit::SupportStrategyIterator {
  %pythoncode %{
    def __iter__(self): return self
    def next(self):
      if self.AtEnd(): raise StopIteration
      value = self.__deref__()
      self.plusplus()       
      return value
  %}
}

%include <libgambit/behav.h>

%extend Gambit::MixedBehavProfile {
  int __len__(void) const { return self->Length(); }
  T __getitem__(int i) const { return (*self)[i+1]; }
  T __getitem__(const GameAction &s) const { return (*self)(s); }
  void __setitem__(int i, const T &value) { (*self)[i+1] = value; }
  void __setitem__(const GameAction &s, const T &value) { (*self)(s) = value; }

%pythoncode %{
  def __repr__(self):
    return "Behavior strategy profile on '%s': [%s]" % \
           (self.GetGame().GetTitle(),
	    ", ".join([ str(self[i]) for i in xrange(len(self)) ]))	

  def __str__(self):   return repr(self)
%}
};

%template(MixedBehavDouble) Gambit::MixedBehavProfile<double>;
%template(MixedBehavRational) Gambit::MixedBehavProfile<Gambit::Rational>;

%include <libgambit/behavspt.h>


//========================================================================
//                       Contingency iterators
//========================================================================

// We disable SWIG's automatic pointer-wrapping behavior, instead
// instrumenting this class to look like a Python-style iterator

%ignore Gambit::BehavIterator::operator->;
%ignore Gambit::StrategyIterator::operator->;

%include <libgambit/behavitr.h>
%include <libgambit/stratitr.h>

%extend Gambit::BehavIterator {
public:
  %pythoncode %{
    def __iter__(self): return self
    def next(self):
      if self.AtEnd(): raise StopIteration
      value = self.__ref__()
      self.plusplus()       
      return value
  %}
}

%extend Gambit::StrategyIterator {
public:
  %pythoncode %{
    def __iter__(self): return self
    def next(self):
      if self.AtEnd(): raise StopIteration
      value = self.__ref__()
      self.plusplus()       
      return value
  %}
}
    
