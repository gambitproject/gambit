//#
//# FILE: mixed.h -- Mixed strategy profile classes
//#
//# @(#)newmixed.h	1.4 23 Aug 1995
//#

#ifndef MIXED_H
#define MIXED_H

#include "gmisc.h"
#include "nfstrat.h"

class BaseNfg;

class BaseMixedProfile     {
  protected:
    const BaseNfg *N;
    bool truncated;
    NFSupport stratset;
    BaseMixedProfile(const BaseNfg &, bool trunc);
    BaseMixedProfile(const BaseNfg &, bool trunc, const NFSupport &s);
    BaseMixedProfile(const BaseMixedProfile &);
    BaseMixedProfile &operator=(const BaseMixedProfile &);

  public:
    virtual ~BaseMixedProfile();

    DataType Type(void) const;
    virtual bool IsPure(void) const = 0;
    virtual bool IsPure(int pl) const = 0;
    bool IsTruncated(void) const    { return truncated; }
    const NFSupport &GetNFSupport(void) const;
};


#include "gpvector.h"

template <class T> class Nfg;

template <class T> class MixedProfile 
  : public BaseMixedProfile, public gPVector<T>  {

  private:

    const Nfg<T> *Nf;
    
    // Private Payoff functions

    T PPayoff(int pl, int index, int i) const;
    void PPayoff(int pl, int const_pl, int const_st, int cur_pl, long index,
		 T prob, T&value) const;
    void PPayoff(int pl, int const_pl1, int const_st1, int const_pl2, 
		int const_st2, int cur_pl, long index, T prob, T &value) const;
    void PPayoff(int pl, int const_pl, int cur_pl, long index, T prob,
		gVector<T> &value) const;
    
  public:
    

    MixedProfile(const Nfg<T> &, bool truncated = false);
    MixedProfile(const Nfg<T> &, const NFSupport &);
    MixedProfile(const Nfg<T> &, const gPVector<T> &);
    MixedProfile(const MixedProfile<T> &);
    ~MixedProfile();

    MixedProfile<T> &operator=(const MixedProfile<T> &);

    Nfg<T> *BelongsTo(void) const  { return (Nfg<T> *) N; }

    bool IsPure(void) const;
    bool IsPure(int pl) const;
    void Centroid(void);

    T Payoff(int pl) const;
    T Payoff(int pl, int player1, int strat1) const;
    T Payoff(int pl, int player1, int strat1, int player2, int strat2) const;
    void Payoff(int pl, int const_pl, gVector<T> &payoff) const;

    
};
#ifndef __BORLANDC__
template <class T> gOutput &operator<<(gOutput &f, const MixedProfile<T> &);
#endif
#endif    // MIXED_H


