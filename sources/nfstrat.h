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

class Support {
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

  Support();

  Support(const Support &s); 
  
  Support(const NFPlayer &p);
  
  Support &operator=(const Support &s); 
  
  virtual ~Support();

  //
  // Member Functions
  //

  // Append a strategies to the support
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

  // Number of Strategies in the support
  int NumStrats(void);


};

class BaseNfg;

class StrategySet {

  friend class NfgIter<double>;
  friend class NfgIter<gRational>;
  friend class CIter<double>;
  friend class CIter<gRational>;

protected:
  gString name;
  gArray <Support *> sups;
  
public:

  //
  // Constructors, Destructors, operators
  //

  StrategySet(const BaseNfg &);
  StrategySet(const StrategySet &s); 
  virtual ~StrategySet();
  StrategySet &operator=(const StrategySet &s);
  
  void SetSupport(int pl, Support *s)   { sups[pl] = s; }
  Support *GetSupport(int pl)     { return sups[pl]; }

  //---------
  // Members
  //---------
  Strategy *GetStrategy(int pl, int num) const;

};



#endif //#NFSTRAT_H


