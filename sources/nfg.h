//
// FILE: nfg.h  -- Declaration of normal form representation
//
// $Id$
//

#ifndef NFG_H
#define NFG_H

#include "base/base.h"
#include "math/gnumber.h"
#include "nfplayer.h"

class NFOutcome;
class NFPlayer;

class Strategy   {
friend class Nfg;
friend class NFPlayer;
private:
  int m_number;
  NFPlayer *m_player;
  long m_index;
  gText m_name;

  Strategy(NFPlayer *);
  Strategy(const Strategy &);
  Strategy &operator=(const Strategy &);
  ~Strategy();

public:
  const gText &Name(void) const { return m_name; }
  void SetName(const gText &s)  { m_name = s; }

  NFPlayer *Player(void) const  { return m_player; }
  int Number(void) const        { return m_number; }
  long Index(void) const        { return m_index; }
};

class StrategyProfile;
class Lexicon;
class Efg;

class Nfg  {
friend class Lexicon;
friend class NfgFileReader;
friend void SetEfg(Nfg *, const Efg *);
protected:
  mutable bool m_dirty;
  mutable long m_revision;
  mutable long m_outcome_revision;
  gText title, comment;
  gArray<int> dimensions;

  gArray<NFPlayer *> players;
  gBlock<NFOutcome *> outcomes;

  gArray<NFOutcome *> results;

  const Efg *efg;

  // PRIVATE AUXILIARY MEMBER FUNCTIONS
  void IndexStrategies(void);
  int Product(const gArray<int> &);

  void BreakLink(void);


public:
  // CONSTRUCTORS, DESTRUCTORS, CONSTRUCTIVE OPERATORS
  Nfg(const gArray<int> &dim);
  Nfg(const Nfg &b);
  ~Nfg();
    
  // GENERAL DATA ACCESS AND MANIPULATION  
  void SetTitle(const gText &s);
  const gText &GetTitle(void) const;

  void SetComment(const gText &);
  const gText &GetComment(void) const;

  long RevisionNumber(void) const { return m_revision; }
  bool IsDirty(void) const { return m_dirty; }
  void SetIsDirty(bool p_dirty) { m_dirty = p_dirty; }

  void WriteNfgFile(gOutput &, int) const;

  // PLAYERS AND STRATEGIES
  int NumPlayers(void) const;
  const gArray<NFPlayer *> &Players(void) const;
  inline NFPlayer *GetPlayer(const int i) const { return Players()[i]; }

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

  void SetPayoff(NFOutcome *, int pl, const gNumber &value);
  gNumber Payoff(NFOutcome *, int pl) const;
  gNumber Payoff(NFOutcome *o, NFPlayer *p) const 
    { return Payoff(o,p->GetNumber()); }

  void InitPayoffs(void) const;

    // defined in nfgutils.cc
  friend void RandomNfg(Nfg &);
  friend bool IsConstSum(const Nfg &);
  friend gNumber MinPayoff(const Nfg &, int pl = 0);
  friend gNumber MaxPayoff(const Nfg &, int pl = 0);

  const Efg *AssociatedEfg(void) const   { return efg; }
};

int ReadNfgFile(gInput &, Nfg *&);

#include "mixed.h"

class NFOutcome   {
  friend class Nfg;
  friend class MixedProfile<double>;
  friend class MixedProfile<gRational>;
  friend class MixedProfile<gNumber>;
  private:
    int number;
    Nfg *nfg;
    gText name;
    gArray<gNumber> payoffs;
    gArray<double> double_payoffs;

    NFOutcome(int n, Nfg *N)
      : number(n), nfg(N), payoffs(N->NumPlayers()), 
	double_payoffs(N->NumPlayers()) { }
    NFOutcome(int n, const NFOutcome &c)
      : number(n), nfg(c.nfg),name(c.name), payoffs(c.payoffs), 
	double_payoffs(c.double_payoffs)  { }
    ~NFOutcome() { }

  public:
    int GetNumber(void) const    { return number; }

    const gText &GetName(void) const   { return name; }
    void SetName(const gText &s)   { name = s; }

    Nfg *Game(void) const   { return nfg; }
};

extern Nfg *CompressNfg(const Nfg &nfg, const NFSupport &S);

#endif    // NFG_H
