//
// FILE: nfg.h  -- Declaration of normal form representation
//
// $Id$
//

#ifndef NFG_H
#define NFG_H

#include "garray.h"
#include "gstring.h"

class NFGameForm;

class NFOutcome   {
  friend class NFGameForm;
  private:
    int number;
    NFGameForm *nfg;
    gString name;

    NFOutcome(int n, NFGameForm *N) : number(n), nfg(N)  { }
    NFOutcome(int n, const NFOutcome &c) 
      : number(n), nfg(c.nfg),name(c.name) { }
    ~NFOutcome() { }

  public:
    int GetNumber(void) const    { return number; }

    const gString &GetName(void) const   { return name; }
    void SetName(const gString &s)   { name = s; }

    NFGameForm *BelongsTo(void) const   { return nfg; }
};

#include "gblock.h"

class NFPlayer;
class Strategy;
class StrategyProfile;
class NFPayoffs;

#ifndef NFG_ONLY
template <class T> class Lexicon;
template <class T> class Efg;
#endif   // NFG_ONLY

class NFGameForm  {
protected:
  gString title;
  gArray<int> dimensions;

  gArray<NFPlayer *> players;
  gBlock<NFOutcome *> outcomes;

  gArray<NFOutcome *> results;

  NFPayoffs *paytable;

  // PRIVATE AUXILIARY MEMBER FUNCTIONS
  void IndexStrategies(void);
  int Product(const gArray<int> &);

public:
  // CONSTRUCTORS, DESTRUCTORS, CONSTRUCTIVE OPERATORS
  NFGameForm(const NFPayoffs &p, const gArray<int> &dim);
  NFGameForm(const NFGameForm &b);
  ~NFGameForm();
    
  // GENERAL DATA ACCESS AND MANIPULATION  
  void SetTitle(const gString &s);
  const gString &GetTitle(void) const;

  NFPayoffs *PayoffTable(void) const   { return paytable; }

  // PLAYERS AND STRATEGIES
  int NumPlayers(void) const;
  const gArray<NFPlayer *> &Players(void) const;

  int NumStrats(int pl) const;
  const gArray<int> &NumStrats(void) const  { return dimensions; }
  const gArray<Strategy *> &Strategies(int p) const;

  int ProfileLength(void) const;

  // OUTCOMES
  NFOutcome *NewOutcome(void);
  void DeleteOutcome(NFOutcome *);

  const gArray<NFOutcome *> &Outcomes(void) const  { return outcomes; }
  int NumOutcomes(void) const   { return outcomes.Length(); }

  void SetOutcome(const gArray<int> &profile, NFOutcome *outcome);
  NFOutcome *GetOutcome(const gArray<int> &profile) const;
  void SetOutcome(const StrategyProfile &p, NFOutcome *outcome);
  NFOutcome *GetOutcome(const StrategyProfile &p) const;

  void SetOutcome(int index, NFOutcome *outcome)  { results[index] = outcome; }
  NFOutcome *GetOutcome(int index) const   { return results[index]; }
};


class NFPayoffs   {
  friend class NFGameForm;
  private:
    virtual void BreakLink(void) = 0;
    virtual void NewOutcome(void) = 0;
    virtual void DeleteOutcome(int) = 0;

  public:
    virtual ~NFPayoffs()  { }
    virtual DataType Type(void) const = 0;
};

template <class T> class MixedProfile;
template <class T> class NfgFile;

#include "gpvector.h"
#include "grblock.h"

template <class T> class Nfg : public NFPayoffs  {
  friend class MixedProfile<T>;
  friend class NfgFile<T>;
#ifndef NFG_ONLY
  friend class Lexicon<T>;
  friend void SetEfg(Nfg<T> *, Efg<T> *);
#endif  // NFG_ONLY

private:
  NFGameForm *gameform;
#ifndef NFG_ONLY
  Efg<T> *efg;
#endif  // NFG_ONLY
  gRectBlock<T> payoffs;

  void BreakLink(void);
  void NewOutcome(void);
  void DeleteOutcome(int);

public:
  Nfg(const gArray<int> &dim);
  Nfg(const Nfg<T> &n);
  virtual ~Nfg();

  // returns the type of the nfg, DOUBLE or RATIONAL
  DataType Type(void) const;
  bool IsConstSum(void) const;
  T MinPayoff(int pl = 0) const;
  T MaxPayoff(int pl = 0) const;

  NFGameForm &GameForm(void) const   { return *gameform; }

  int NumPlayers(void) const   { return gameform->NumPlayers(); }
  const gArray<NFPlayer *> &Players(void) const
    { return gameform->Players(); }

  int NumStrats(int pl) const  { return gameform->NumStrats(pl); }
  const gArray<int> &NumStrats(void) const  { return gameform->NumStrats(); }

  const gArray<NFOutcome *> &Outcomes(void) const 
    { return gameform->Outcomes(); }
  int NumOutcomes(void) const   { return gameform->NumOutcomes(); }

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

#endif    // NFG_H
