//
// FILE: nfplayer.h -- Declaration of normal form player data type
//
// $Id$
//

#ifndef NFPLAYER_H
#define NFPLAYER_H

class Strategy;
class gRational;
template <class T> class Nfg;
template <class T> class NfgIter;
template <class T> class NfgContIter;

class NFPlayer {

  friend class BaseNfg;
  friend class NFStrategySet;
  friend class Nfg<double>;
  friend class Nfg<gRational>;
  friend class NfgIter<double>;
  friend class NfgIter<gRational>;
  friend class NfgContIter<double>;
  friend class NfgContIter<gRational>;

private:
  int number;
  gString name;
  BaseNfg *N;
  
  gArray<Strategy *> strategies;
 

public:

  NFPlayer( int n, BaseNfg *no, int num );
  virtual ~NFPlayer();

  BaseNfg &BelongsTo(void) const;
  
  const gString &GetName(void) const;
  void SetName(const gString &s);

  int NumStrats(void) const;

  const gArray<Strategy *> &Strategies(void) const;

  int GetNumber(void) const  { return number; }
};

#endif    // NFPLAYER_H


