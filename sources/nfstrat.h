//#
//# FILE: nfstrat.h -- Declaration of Normal Form Strategy data type
//#
//# $Id$
//#

#ifndef NFSTRAT_H
#define NFSTRAT_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

#include "gstring.h"
#include "gblock.h"

struct Strategy   {
  int number;
  Strategy *dominator;
  long index;
  gString name;

  Strategy(void);
  Strategy(const Strategy &s);
  ~Strategy();
};


class StrategyProfile   {
private:
  long index;
  gArray<Strategy *> profile;
  
public:
  StrategyProfile(int pl);
  StrategyProfile(const StrategyProfile &p);

  ~StrategyProfile();
  
  StrategyProfile &operator=(const StrategyProfile &);
  
  int IsValid(void) const; 
  
  long GetIndex(void) const;
  
  Strategy *const operator[](int p) const;
  Strategy *const Get(int p) const;
  void Set(int p, Strategy  *const s);
};

class NFPlayer;
class Strategy;
template <class T> class NfgIter;
template <class T> class CIter;

class StrategySet {
  friend class NfgIter<double>;
  friend class NfgIter<gRational>;
  friend class CIter<double>;
  friend class CIter<gRational>;
protected:
  const NFPlayer * nfp;
  gBlock <Strategy *> strategies;
  
public:

  //
  // Constructors, Destructor, operators
  //

  StrategySet();

  StrategySet(const StrategySet &s); 
  
  StrategySet(const NFPlayer &p);
  
  StrategySet &operator=(const StrategySet &s); 
  
  virtual ~StrategySet();

  //
  // Member Functions
  //

  // Append a strategies to the StrategySet
  void AddStrategy(Strategy *s);

  // Insert a strategy to a particular place in the gBlock;
  void AddStrategy(Strategy *s, int i);

  // Remove a strategy at int i, returns the removed strategy pointer
  Strategy* RemoveStrategy(int i);
  
  // Removes a strategy pointer. Returns true if the strategy was successfully
  // removed, false otherwise.
  bool RemoveStrategy( Strategy *s ); 

  // Get a Strategy
  Strategy* GetStrategy(int num) const;

  // Number of Strategies in the StrategySet
  int NumStrats(void);


};

class BaseNfg;

class Support {

  friend class NfgIter<double>;
  friend class NfgIter<gRational>;
  friend class CIter<double>;
  friend class CIter<gRational>;

protected:
  gString name;
  gArray <StrategySet *> sups;
  
public:

  //
  // Constructors, Destructors, operators
  //

  Support(const BaseNfg &);
  Support(const Support &s); 
  virtual ~Support();
  Support &operator=(const Support &s);
  
  void SetStrategySet(int pl, StrategySet *s)   { sups[pl] = s; }
  StrategySet *GetStrategySet(int pl)     { return sups[pl]; }

  //---------
  // Members
  //---------
  Strategy *GetStrategy(int pl, int num) const;

};



#endif //#NFSTRAT_H


