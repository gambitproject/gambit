//
// FILE: behav.h -- Behavioral strategy profile classes
//
// $Id$
//

#ifndef BEHAV_H
#define BEHAV_H

#include "gmisc.h"
#include "gstring.h"
#include "efstrat.h"

#include "gdpvect.h"

template <class T> class Efg;
class Infoset;

template <class T> class BehavProfile : public gDPVector<T>  {
  protected:
    const Efg<T> *E;
    EFSupport support;

    void Payoff(Node *n, T prob, int pl, T &value) const;
    void NodeValues(Node *n, int pl, gArray<T> &valarray,
		    int &index) const;
    void CondPayoff(Node *n, T prob, gPVector<T> &, gDPVector<T> &) const;
    void NodeRealizProbs(Node *n, T prob, int &index, gArray<T> &NRProbs) const;
    void Beliefs(Node *n, T prob, gDPVector<T> &BProbs, 
		 gPVector<T> &gpv) const;

  public:
    BehavProfile(const Efg<T> &);
    BehavProfile(const Efg<T> &, const gDPVector<T> &);
    BehavProfile(const Efg<T> &, const EFSupport &);
    BehavProfile(const BehavProfile<T> &);
    virtual ~BehavProfile();

    BehavProfile<T> &operator=(const BehavProfile<T> &);

    Efg<T> &BelongsTo(void) const   { return const_cast< Efg<T>& >( *E ); }

    const T &GetValue(Infoset *s, int act) const;

    T Payoff(int pl) const;
    gArray<T> NodeValues(int pl) const;
    void CondPayoff(gDPVector<T> &value, gPVector<T> &probs) const;
    gArray<T> NodeRealizProbs(void) const;
    gDPVector<T> Beliefs(void) const
;

    T LiapValue(void) const;
    void Gripe(gDPVector<T> &value) const;
    T MaxGripe(void) const;

    void Centroid(void) const;

    bool operator==(const BehavProfile<T> &) const;
    
    const EFSupport &Support(void) const   { return support; }
};
#ifndef __BORLANDC__
template <class T> gOutput &operator<<(gOutput &f, const BehavProfile<T> &p);
#endif
#endif   // BEHAV_H
