//#
//# FILE: nfg.h  -- Declaration to  BaseNfg class
//#
//# $Date$ $Id$
//#

#ifndef NFG_H
#define NFG_H

#include "garray.h"
#include "gstring.h"

class NFPlayer;
class Strategy;

class BaseNfg {
 
protected:
  gString title;
  gArray<NFPlayer *> players;
  gArray<int> dimensions;
  void IndexStrategies(void);

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

  int NumPlayers(void) const;
  int NumStrats(int pl) const;
  int ProfileLength(void) const;
  const gArray<int> &Dimensionality(void) const  { return dimensions; }
  const gArray<NFPlayer *> &PlayerList(void) const;

// operator to get strategy s of player p

  Strategy * GetStrategy(int p, int s) const;

  const gArray <Strategy *> &GetStrategy(int p) const;
  const gArray <Strategy *> &operator()(int p) const;

  virtual void WriteNfgFile(gOutput &) const = 0;
};


template <class T> class MixedProfile;
template <class T> class NfgFile;

#include "gpvector.h"

template <class T> class Nfg : public BaseNfg {

friend class MixedProfile<T>;
friend class NfgFile<T>;
private:
  int NumPayPerPlayer;
  gArray<T *> payoffs;

public:
  Nfg(const gArray<int> &dim);
  Nfg(const Nfg &n);
  virtual ~Nfg();

  // returns the type of the nfg, DOUBLE or RATIONAL
  DataType Type(void) const;

  void WriteNfgFile(gOutput &) const;

  void SetPayoff(int pl, const gArray<int> &profile, const T &value);
  const T &Payoff(int pl, const gArray<int> &profile) const;
  void SetPayoff(int pl, const StrategyProfile *p, const T &value);
  const T &Payoff(int pl, const StrategyProfile *p) const;

};

template <class T> int ReadNfgFile(gInput &, Nfg<T> *&);
void NfgFileType(gInput &f, bool &valid, DataType &type);

#endif //# NFG_H
