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

class NFPlayer;

struct Strategy   {
  int number;
  NFPlayer *nfp;
  long index;
  gString name;

  Strategy(void);
  Strategy(const Strategy &s);
  Strategy &operator=(const Strategy &s);
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

template <class T> class NfgIter;
template <class T> class ContIter;

class NFStrategySet {
  friend class NfgIter<double>;
  friend class NfgIter<gRational>;
  friend class ContIter<double>;
  friend class ContIter<gRational>;
protected:
  NFPlayer * nfp;
  gBlock <Strategy *> strategies;
  
public:

  //
  // Constructors, Destructor, operators
  //

  NFStrategySet();

  NFStrategySet(const NFStrategySet &s); 
  
  NFStrategySet( NFPlayer &p);
  
  NFStrategySet &operator=(const NFStrategySet &s); 
  bool operator==(const NFStrategySet &s);

  virtual ~NFStrategySet();

  //
  // Member Functions
  //

  // Append a strategies to the NFStrategySet
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

  // Number of Strategies in the NFStrategySet
  int NumStrats(void);

  //  return the entire strategy set in a const gArray
  const gArray<Strategy *> &GetNFStrategySet(void);

  // return the NFPlayer of this NFStrategySet
  NFPlayer &GetPlayer(void) const;

};

class BaseNfg;

class NFSupport {

  friend class NfgIter<double>;
  friend class NfgIter<gRational>;
  friend class ContIter<double>;
  friend class ContIter<gRational>;

protected:
  gString name;
  const BaseNfg *bnfg;
  gArray <NFStrategySet *> sups;
  
public:

  //
  // Constructors, Destructors, operators
  //

  NFSupport( const BaseNfg &);
  NFSupport(const NFSupport &s); 
  virtual ~NFSupport();
  NFSupport &operator=(const NFSupport &s);
  bool operator==(const NFSupport &s);

  //---------
  // Members
  //---------

  void SetNFStrategySet(int pl, NFStrategySet *s);   
  NFStrategySet *GetNFStrategySet(int pl)     { return sups[pl]; }

  Strategy *GetStrategy(int pl, int num) const;
  const gArray<Strategy *> &GetStrategy(int pl) const;
  const BaseNfg &BelongsTo(void) const;
};



#endif //#NFSTRAT_H


