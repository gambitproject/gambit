//
// FILE: algutils.cc -- Implementation of Algorithm specific classes
//

#include "algutils.h"
#include "assert.h"

AlgParams::AlgParams(gStatus &s)
  : m_accuracy(0), trace(0), stopAfter(0), precision(precDOUBLE), 
    tracefile(&gnull), status(s)
{ }

AlgParams::~AlgParams()
{ }

void AlgParams::SetAccuracy(const gNumber &p_accuracy)
{ m_accuracy = p_accuracy; }


FuncMinParams::FuncMinParams(gStatus &s)
  : AlgParams(s), maxits1(100), maxitsN(20), tol1(2.0e-10), tolN(1.0e-10) 
{ }

FuncMinParams::~FuncMinParams()
{ }

void FuncMinParams::SetAccuracy(const gNumber &p_accuracy)
{ 
  m_accuracy = p_accuracy;
  maxits1 = maxitsN = 200;
  tol1 = tolN = p_accuracy * p_accuracy;
}

//#include "algutils.imp"

//
// gFact Member functions
//

template <class T> gFact<T>::gFact() 
  : answer(0)
{ }

template <class T> gFact<T>::gFact(const gFact<T> &f) 
  : answer(0)
{ 
  if(f.Checked()) Set(f.Answer());
}

template <class T> gFact<T>::~gFact(void) 
{ 
  if(answer) delete answer;
}


template <class T> gFact<T> &gFact<T>::operator=(const gFact<T> &b)
{ 
  if (this != &b) {
    if(b.Checked()) 
      Set(b.Answer());
  } 
  return *this;
}

template <class T> const T &gFact<T>::Answer(void) const
{
  if(!answer) 
    throw Unknown();
  return *answer;
}

template <class T> void gFact<T>::Set(T state)
{
  if(answer && *answer!=state ) 
    throw Contradiction();
  if(!answer) 
    answer = new T(state);
}

template <class T> gFact<T>::Contradiction::~Contradiction()
{ }

template <class T> gText gFact<T>::Contradiction::Description(void) const 
{
  return "Contradictory information in gFact";
}

template <class T> gFact<T>::Unknown::~Unknown()
{ }

template <class T> gText gFact<T>::Unknown::Description(void) const 
{
  return "Unknown information in gFact";
}

template class gFact<gNumber>;
template class gFact<gTriState>;

#include "gpvector.h"
template class gFact<gPVector<gNumber> >;

// Following is an intitial, more complicated implementation of the above
// I am checking it in (commented out), so I don't forget how to 
// pass pointers to member functions of a class, since this may be useful 
// elsewhere.  

/*
template <class C, class T> gFact<C, T>::gFact(const C &m,FactFinder f) 
  : ms(m), answer(0), gofer(f)
{ }

template <class C, class T> gFact<C, T>::gFact(const gFact<C, T> &f) 
  : ms(f.ms), answer(f.answer), gofer(f.gofer)
{
  // Don't use this constructor.  
  assert(0);  
}

template <class C, class T> gFact<C, T>::~gFact(void) 
{ 
  if(answer) delete answer;
}


template <class C, class T> gFact<C, T> &gFact<C, T>::operator=(const gFact<C, T> &b)
{ 
  if (this != &b) {
    if(b.Checked()) 
      Set(b.Answer());
  } 
  return *this;
}

template <class C, class T> const T &gFact<C, T>::Answer(void) const
{
  if(!answer) 
    answer = new T((ms.*gofer)());
  return *answer;
}

template <class C, class T> void gFact<C, T>::Set(T state)
{
  if(!answer) 
    answer = new T(state);
  if(*answer!=state ) 
    throw Contradiction();
}

template <class C, class T> gFact<C, T>::Contradiction::~Contradiction()
{ }

template <class C, class T> gText gFact<C, T>::Contradiction::Description(void) const 
{
  return "Contradictory information in gFact";
}

class MixedSolution;
class BehavSolution;
template class gFact<MixedSolution, gNumber>;
template class gFact<MixedSolution, gTriState>;

template class gFact<BehavSolution, gNumber>;
template class gFact<BehavSolution, gTriState>;
#include "gpvector.h"
template class gFact<BehavSolution, gPVector<gNumber> >;

*/

