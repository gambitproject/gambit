//
// FILE: nfg.h  -- Declaration of normal form representation
//
// $Date$ $Id$
//

#ifndef NFG_H
#define NFG_H

#include "garray.h"
#include "gstring.h"

class NFPlayer;
class Strategy;
class StrategyProfile;

class BaseEfg;

class BaseNfg {
// these friend declarations are awful... but they get the job done for now
friend class Lexicon; 
friend void SetEfg(BaseNfg *, BaseEfg *);
protected:
  gString title;
  BaseEfg *efg;
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
  virtual bool IsConstSum(void) const = 0;


  int NumPlayers(void) const;
  int NumStrats(int pl) const;
  int ProfileLength(void) const;
  const gArray<int> &Dimensionality(void) const  { return dimensions; }
  const gArray<NFPlayer *> &PlayerList(void) const;
  BaseEfg *AssociatedEfg(void) const   { return efg; }

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
  bool IsConstSum(void) const;
  T MinPayoff(int pl = 0) const;
  T MaxPayoff(int pl = 0) const;

  void WriteNfgFile(gOutput &) const;

  void SetPayoff(int pl, const gArray<int> &profile, const T &value);
  const T &Payoff(int pl, const gArray<int> &profile) const;
  void SetPayoff(int pl, const StrategyProfile *p, const T &value);
  const T &Payoff(int pl, const StrategyProfile *p) const;

  // defined in nfgutils.cc
  friend void RandomNfg(Nfg<T> &);
};

template <class T> int ReadNfgFile(gInput &, Nfg<T> *&);
void NfgFileType(gInput &f, bool &valid, DataType &type);

#endif //# NFG_H
