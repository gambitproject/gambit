//
// FILE: algutils.h -- class definition for Algorithm specific classes
//

#ifndef ALGUTILS_H
#define ALGUTILS_H

#include "gstream.h"
#include "gstatus.h"
#include "gnumber.h"

class AlgParams   {
protected:
  gNumber m_accuracy;
  
public:
  int trace, stopAfter;
  gPrecision precision;
  
  gOutput *tracefile;
  gStatus &status;
  
  AlgParams(gStatus & = gstatus);
  virtual ~AlgParams();
  
  const gNumber &Accuracy(void) const { return m_accuracy; }
  virtual void SetAccuracy(const gNumber &);
};

class FuncMinParams : public AlgParams {
public:
  int maxits1, maxitsN;
  double tol1, tolN;
  
  FuncMinParams(gStatus & = gstatus);
  virtual ~FuncMinParams();
  
  void SetAccuracy(const gNumber &);
};



// 
// gFact is a class for storing information that may or may not 
// be needed, and which might be expensive to compute.  Hence: 
// 
// 1. Answer is not computed unless specifically requested, and  
// 2. Answer is cached in case it is requested again.  
// 

template <class T> class gFact {
private:
  mutable T *answer;
public:
  class Contradiction : public gException  {
  public:
    virtual ~Contradiction();
    gText Description(void) const;   
  };
  class Unknown : public gException  {
  public:
    virtual ~Unknown();
    gText Description(void) const;   
  };
  
  gFact();
  gFact(const gFact<T> &);
  virtual ~gFact(void);
  
  gFact &operator=(const gFact<T> &);
  
  const T &Answer(void) const;
  const bool Checked(void) const {return answer;}
  void Invalidate(void) {delete answer;}
  void Set(T state);
};

// Following is an intitial, more complicated implementation of the above
// I am checking it in (commented out), so I don't forget how to 
// pass pointers to member functions of a class, since this may be useful 
// elsewhere.  
// 
// Here is a sample of how to use it:
//
// class MyClass {
// private:
//   gNumber GetInfo(void);
//   mutable gFact<MyClass, gTriState> m_Info;
// public:
//  MyClass() :  m_Info(*this,&MyClass::GetInfo);
// }  

/*
template <class C, class T> class gFact {
  typedef T (C::*FactFinder)();
private:
  const C &ms;
  mutable T *answer;
  FactFinder gofer;
public:
  class Contradiction : public gException  {
  public:
    virtual ~Contradiction();
    gText Description(void) const;   
  };
  
  gFact(const C &,FactFinder);
  gFact(const gFact<C, T> &);
  virtual ~gFact(void);
  
  gFact &operator=(const gFact<C, T> &);
  
  const T &Answer(void) const;
  const bool Checked(void) const {return answer;}
  void Invalidate(void) {delete answer;}
  void Set(T state);
};
*/

#endif // ALGUTILS_H

