//
// FILE: nfstrat.h -- Declaration of Normal Form Strategy data type
//
// $Id$
//

#ifndef NFSTRAT_H
#define NFSTRAT_H

#include "gstring.h"
#include "gblock.h"
#include "gambitio.h"

class NFPlayer;

struct Strategy   {
  int number;
  NFPlayer *nfp;
  long index;
  gString name;

  Strategy(NFPlayer *);
  Strategy(const Strategy &s);
  Strategy &operator=(const Strategy &s);
  ~Strategy();
};

class NFGameForm;
template <class T> class Nfg;

class StrategyProfile   {
  friend class NFGameForm;
private:
  long index;
  gArray<Strategy *> profile;
  
public:
  StrategyProfile(NFGameForm &);
  StrategyProfile(const StrategyProfile &p);

  ~StrategyProfile();
  
  StrategyProfile &operator=(const StrategyProfile &);
  
  bool IsValid(void) const; 
  
  long GetIndex(void) const;
  
  Strategy *const operator[](int p) const;
  Strategy *const Get(int p) const;
  void Set(int p, Strategy  *const s);
};


class NFGameForm;
class NFStrategySet;

class NFSupport {
protected:
  const NFGameForm *bnfg;
  gArray <NFStrategySet *> sups;
  
public:
  NFSupport(const NFGameForm &);
  NFSupport(const NFSupport &s); 
  virtual ~NFSupport();
  NFSupport &operator=(const NFSupport &s);

  bool operator==(const NFSupport &s) const;
  bool operator!=(const NFSupport &s) const;

  const NFGameForm &BelongsTo(void) const   { return *bnfg; }
  
  const gBlock<Strategy *> &Strategies(int pl) const;

  int NumStrats(int pl) const;
  const gArray<int> NumStrats(void) const;

  void AddStrategy(Strategy *);
  bool RemoveStrategy(Strategy *);
  
  bool IsSubset(const NFSupport &s) const;

  // returns the index of the strategy in the support if it exists,
  // otherwise returns zero
  int Find(Strategy *) const; 

  void Dump(gOutput &) const;
};

gOutput &operator<<(gOutput &f, const NFSupport &);

#endif    // NFSTRAT_H


