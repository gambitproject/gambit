//
// FILE: nfg.h  -- Declaration of normal form representation
//
// $Id$
//

#ifndef NFG_H
#define NFG_H

#include "garray.h"
#include "gstring.h"

class BaseNfg;

class NFOutcome   {
  friend class BaseNfg;
  private:
    int number;
    BaseNfg *nfg;
    gString name;

    NFOutcome(int n, BaseNfg *N) : number(n), nfg(N)  { }
    NFOutcome(int n, const NFOutcome &c) 
      : number(n), nfg(c.nfg),name(c.name) { }
    ~NFOutcome() { }

  public:
    int GetNumber(void) const    { return number; }

    const gString &GetName(void) const   { return name; }
    void SetName(const gString &s)   { name = s; }

    BaseNfg *BelongsTo(void) const   { return nfg; }
};

#include "glist.h"

class NFPlayer;
class Strategy;
class StrategyProfile;

template <class T> class Lexicon;
template <class T> class Efg;

class BaseNfg {
protected:
  gString title;
  gArray<int> dimensions;

  gArray<NFPlayer *> players;
  gList<NFOutcome *> outcomes;

  gArray<NFOutcome *> results;

  void IndexStrategies(void);
  virtual void BreakLink(void) = 0;

  int Product(const gArray<int> &);

  NFOutcome *NewOutcome(void);

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
  const gArray<NFPlayer *> &Players(void) const;

  int NumStrats(int pl) const;
  const gArray<int> &NumStrats(void) const  { return dimensions; }

  int ProfileLength(void) const;

  const gList<NFOutcome *> &Outcomes(void) const  { return outcomes; }
  int NumOutcomes(void) const   { return outcomes.Length(); }

  void SetOutcome(const gArray<int> &profile, NFOutcome *outcome);
  NFOutcome *GetOutcome(const gArray<int> &profile) const;
  void SetOutcome(const StrategyProfile &p, NFOutcome *outcome);
  NFOutcome *GetOutcome(const StrategyProfile &p) const;

  void SetOutcome(int index, NFOutcome *outcome)  { results[index] = outcome; }
  NFOutcome *GetOutcome(int index) const   { return results[index]; }

  const gArray<Strategy *> &Strategies(int p) const;

  virtual void WriteNfgFile(gOutput &) const = 0;
};


template <class T> class MixedProfile;
template <class T> class NfgFile;

#include "gpvector.h"

#include "grblock.h"

template <class T> class Nfg : private BaseNfg {

friend class MixedProfile<T>;
friend class NfgFile<T>;
friend class Lexicon<T>;
friend void SetEfg(Nfg<T> *, Efg<T> *);
private:
  Efg<T> *efg;
  gRectBlock<T> payoffs;

  void BreakLink(void);

public:
  Nfg(const gArray<int> &dim);
  Nfg(const Nfg<T> &n);
  virtual ~Nfg();

  // returns the type of the nfg, DOUBLE or RATIONAL
  DataType Type(void) const;
  bool IsConstSum(void) const;
  T MinPayoff(int pl = 0) const;
  T MaxPayoff(int pl = 0) const;

  BaseNfg &GameForm(void) const   { return (BaseNfg &) *this; }

  int NumPlayers(void) const   { return BaseNfg::NumPlayers(); }
  const gArray<NFPlayer *> &Players(void) const
    { return BaseNfg::Players(); }

  int NumStrats(int pl) const  { return BaseNfg::NumStrats(pl); }
  const gArray<int> &NumStrats(void) const  { return BaseNfg::NumStrats(); }

  const gList<NFOutcome *> &Outcomes(void) const 
    { return BaseNfg::Outcomes(); }
  int NumOutcomes(void) const   { return BaseNfg::NumOutcomes(); }

  NFOutcome *NewOutcome(void);

  void SetPayoff(NFOutcome *, int pl, const T &value);
  const T &Payoff(NFOutcome *, int pl) const;

  void WriteNfgFile(gOutput &) const;

  Efg<T> *AssociatedEfg(void) const   { return efg; }

  // defined in nfgutils.cc
  friend void RandomNfg(Nfg<T> &);
};

template <class T> int ReadNfgFile(gInput &, Nfg<T> *&);
void NfgFileType(gInput &f, bool &valid, DataType &type);

#endif    // NFG_H
