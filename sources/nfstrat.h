//
// FILE: nfstrat.h -- Declaration of Normal Form Strategy data type
//
// $Id$
//

#ifndef NFSTRAT_H
#define NFSTRAT_H

#include "gtext.h"
#include "gblock.h"
#include "gstream.h"
#include "gnumber.h"
#include "nfplayer.h"

class NFPlayer;
class Nfg;
class Strategy;

class StrategyProfile   {
  friend class Nfg;
private:
  long index;
  gArray<Strategy *> profile;
  
public:
  StrategyProfile(const Nfg &);
  StrategyProfile(const StrategyProfile &p);

  ~StrategyProfile();
  
  StrategyProfile &operator=(const StrategyProfile &);
  
  bool IsValid(void) const; 
  
  long GetIndex(void) const;
  
  Strategy *const operator[](int p) const;
  Strategy *const Get(int p) const;
  void Set(int p, const Strategy  *const s);
};


class Nfg;
class nfgSupportPlayer;

class NFSupport {
protected:
  const Nfg *bnfg;
  gArray <nfgSupportPlayer *> sups;
  gText m_name;
  
public:
  NFSupport(const Nfg &);
  NFSupport(const NFSupport &s); 
  virtual ~NFSupport();
  NFSupport &operator=(const NFSupport &s);

  bool operator==(const NFSupport &s) const;
  bool operator!=(const NFSupport &s) const;

  const Nfg &Game(void) const   { return *bnfg; }
  const Nfg *GamePtr(void) const { return bnfg; }

  const gText &GetName(void) const { return m_name; }
  void SetName(const gText &p_name) { m_name = p_name; }
  
  const gBlock<Strategy *> &Strategies(int pl) const;
  inline Strategy *GetStrategy(int pl, int i) const
    { return Strategies(pl)[i]; }
  int GetNumber(const Strategy *s) const;

  int NumStrats(int pl) const;
  inline int NumStrats(const NFPlayer* p) const 
    { return NumStrats(p->GetNumber()); }
  const gArray<int> NumStrats(void) const;
  int TotalNumStrats(void) const;

  void AddStrategy(Strategy *);
  bool RemoveStrategy(Strategy *);
  
  bool IsSubset(const NFSupport &s) const;
  bool IsValid(void) const;

  // returns the index of the strategy in the support if it exists,
  // otherwise returns zero
  int Find(Strategy *) const; 
  bool StrategyIsActive(Strategy *) const;

  void Dump(gOutput &) const;
};

gOutput &operator<<(gOutput &f, const NFSupport &);


//
// Computation of dominated strategies
//
class gStatus;

NFSupport *ComputeDominated(const Nfg &, NFSupport &, bool strong,
			    const gArray<int> &players,
			    gOutput &, gStatus &gstatus);
NFSupport *ComputeMixedDominated(NFSupport &, bool strong,
				 gPrecision precision,
				 const gArray<int> &players,
				 gOutput &, gStatus &status);
bool IsMixedDominated(const NFSupport &S,Strategy *str,
			   bool strong, gPrecision precision,
			   gOutput &tracefile);

bool IsDominated(const NFSupport &S, Strategy *s, bool strong, 
		 const gStatus &status);

#endif    // NFSTRAT_H


