//
// FILE: nfg.h  -- Declaration of normal form representation
//
// $Id$
//

#ifndef NFG_H
#define NFG_H

#include "garray.h"
#include "gstring.h"

class NFPlayer;
class Strategy;
class StrategyProfile;

class BaseEfg;

class BaseNfg {
// these friend declarations are awful... but they get the job done for now
friend class Lexicon; 
friend void SetEfg(BaseNfg *, BaseEfg *);
protected:
  gString title;
  BaseEfg *efg;
  gArray<NFPlayer *> players;
  gArray<int> dimensions;
  void IndexStrategies(void);

  void BreakLink(void);

public:
  
  // ------------------------------------
  // Constructors, Destructors, Operators
  // ------------------------------------
  
  BaseNfg(const gArray<int> &dim);
  BaseNfg(const BaseNfg &b);
  virtual ~BaseNfg();
  
  
  // ----------------
  // Member Functions
  // ----------------
  
  void SetTitle(const gString &s);
  const gString &GetTitle(void) const;

  virtual DataType Type(void) const = 0;
  virtual bool IsConstSum(void) const = 0;


  int NumPlayers(void) const;
  int NumStrats(int pl) const;
  const gArray<int> &NumStrats(void) const  { return dimensions; }

  int ProfileLength(void) const;

  const gArray<NFPlayer *> &Players(void) const;
  BaseEfg *AssociatedEfg(void) const   { return efg; }

  const gArray<Strategy *> &Strategies(int p) const;

  virtual void WriteNfgFile(gOutput &) const = 0;
};


template <class T> class MixedProfile;
template <class T> class NfgFile;

#include "gpvector.h"

template <class T> class NFOutcome   {
  private:
    gVector<T> payoffs;

  public:
    NFOutcome(const gVector<T> &p) : payoffs(p)  { }
    ~NFOutcome() { }

    const gVector<T> &Payoffs(const)   { return payoffs; }
    const T &operator[](int pl) const    { return payoffs[pl]; }
    T &operator[](int pl)       { return payoffs[pl]; }
};

#include "glist.h"

template <class T> class Nfg : public BaseNfg {

friend class MixedProfile<T>;
friend class NfgFile<T>;
private:
  gArray<NFOutcome<T> *> payoffs;
  gList<NFOutcome<T> *> outcomes;

  int Product(const gArray<int> &);

public:
  Nfg(const gArray<int> &dim);
  Nfg(const Nfg &n);
  virtual ~Nfg();

  // returns the type of the nfg, DOUBLE or RATIONAL
  DataType Type(void) const;
  bool IsConstSum(void) const;
  T MinPayoff(int pl = 0) const;
  T MaxPayoff(int pl = 0) const;

  void WriteNfgFile(gOutput &) const;

  void SetOutcome(const gArray<int> &profile, NFOutcome<T> *outcome);
  NFOutcome<T> *Outcome(const gArray<int> &profile) const;
  void SetOutcome(const StrategyProfile *p, NFOutcome<T> *outcome);
  NFOutcome<T> *Outcome(const StrategyProfile *p) const;

  // defined in nfgutils.cc
  friend void RandomNfg(Nfg<T> &);
};

template <class T> int ReadNfgFile(gInput &, Nfg<T> *&);
void NfgFileType(gInput &f, bool &valid, DataType &type);

#endif //# NFG_H
