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
class NfgPayoffs;

#ifndef NFG_ONLY
template <class T> class Lexicon;
template <class T> class Efg;
#endif   // NFG_ONLY

class BaseNfg {
protected:
  gString title;
  gArray<int> dimensions;

  gArray<NFPlayer *> players;
  gList<NFOutcome *> outcomes;

  gArray<NFOutcome *> results;

  NfgPayoffs *paytable;

  void IndexStrategies(void);

  int Product(const gArray<int> &);

public:
  
  // ------------------------------------
  // Constructors, Destructors, Operators
  // ------------------------------------
  
  BaseNfg(const NfgPayoffs &p, const gArray<int> &dim);
  BaseNfg(const BaseNfg &b);
  virtual ~BaseNfg();
  
  
  // ----------------
  // Member Functions
  // ----------------
  
  void SetTitle(const gString &s);
  const gString &GetTitle(void) const;

  int NumPlayers(void) const;
  const gArray<NFPlayer *> &Players(void) const;

  int NumStrats(int pl) const;
  const gArray<int> &NumStrats(void) const  { return dimensions; }

  int ProfileLength(void) const;

  const gList<NFOutcome *> &Outcomes(void) const  { return outcomes; }
  int NumOutcomes(void) const   { return outcomes.Length(); }

  NfgPayoffs *PayoffTable(void) const   { return paytable; }

  NFOutcome *NewOutcome(void);

  void SetOutcome(const gArray<int> &profile, NFOutcome *outcome);
  NFOutcome *GetOutcome(const gArray<int> &profile) const;
  void SetOutcome(const StrategyProfile &p, NFOutcome *outcome);
  NFOutcome *GetOutcome(const StrategyProfile &p) const;

  void SetOutcome(int index, NFOutcome *outcome)  { results[index] = outcome; }
  NFOutcome *GetOutcome(int index) const   { return results[index]; }

  const gArray<Strategy *> &Strategies(int p) const;
};


class NfgPayoffs   {
  public:
    virtual ~NfgPayoffs()  { }
    virtual DataType Type(void) const = 0;
};

template <class T> class MixedProfile;
template <class T> class NfgFile;

#include "gpvector.h"

#include "grblock.h"

template <class T> class Nfg : public NfgPayoffs  {

friend class MixedProfile<T>;
friend class NfgFile<T>;
#ifndef NFG_ONLY
friend class Lexicon<T>;
friend void SetEfg(Nfg<T> *, Efg<T> *);
#endif  // NFG_ONLY

private:
  BaseNfg *gameform;
#ifndef NFG_ONLY
  Efg<T> *efg;
#endif  // NFG_ONLY
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

  BaseNfg &GameForm(void) const   { return *gameform; }

  int NumPlayers(void) const   { return gameform->NumPlayers(); }
  const gArray<NFPlayer *> &Players(void) const
    { return gameform->Players(); }

  int NumStrats(int pl) const  { return gameform->NumStrats(pl); }
  const gArray<int> &NumStrats(void) const  { return gameform->NumStrats(); }

  const gList<NFOutcome *> &Outcomes(void) const 
    { return gameform->Outcomes(); }
  int NumOutcomes(void) const   { return gameform->NumOutcomes(); }

  NFOutcome *NewOutcome(void);

  void SetPayoff(NFOutcome *, int pl, const T &value);
  const T &Payoff(NFOutcome *, int pl) const;

  void WriteNfgFile(gOutput &) const;

#ifndef NFG_ONLY
  Efg<T> *AssociatedEfg(void) const   { return efg; }
#endif   // NFG_ONLY

  // defined in nfgutils.cc
  friend void RandomNfg(Nfg<T> &);
};

int ReadNfgFile(gInput &, Nfg<double> *&, Nfg<gRational> *&);
//void NfgFileType(gInput &f, bool &valid, DataType &type);

#endif    // NFG_H
