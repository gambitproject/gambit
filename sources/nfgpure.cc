//#
//# FILE: purenash.cc -- Find all pure strategy Nash equilibria
//#
//# $Id$
//#

#include "rational.h"
#include "nfrep.h"
#include "normal.h"

template <class T> class PureNashModule   {
  private:
    const NFRep<T> &rep;

    // potentially other private data
 
  public:
    PureNashModule(const NFRep<T> &r  /* possibly other parameters */);
    virtual ~PureNashModule();

//
// Finds all pure strategy Nash equilibria in the game
//
    int PureNash(void);
};

template <class T> PureNashModule<T>::PureNashModule(const NFRep<T> &r)
  : rep(r)
{
  
}

template <class T> PureNashModule<T>::~PureNashModule()
{

}

template <class T> int PureNashModule<T>::PureNash(void)
{
  return 0;
}

int NormalForm::PureNash(void)
{
  switch (type)   {
    case nfDOUBLE:   {
      PureNashModule<double> M((NFRep<double> &) *data);
      M.PureNash();
      return 1;
    }
    
    case nfRATIONAL:  {
      PureNashModule<Rational> M((NFRep<Rational> &) *data);
      M.PureNash();
      return 1;
    }

    default:
      return 0;
  }
}






