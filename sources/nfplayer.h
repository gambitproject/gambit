//
// FILE: nfplayer.h -- Declaration of normal form player data type
//
// $Id$
//

#ifndef NFPLAYER_H
#define NFPLAYER_H

class Strategy;

class NFPlayer {
  friend class NFGameForm;

private:
  int number;
  gString name;
  NFGameForm *N;
  
  gArray<Strategy *> strategies;

public:
  NFPlayer(int n, NFGameForm *no, int num);
  ~NFPlayer();

  NFGameForm &BelongsTo(void) const;
  
  const gString &GetName(void) const;
  void SetName(const gString &s);

  int NumStrats(void) const;

  const gArray<Strategy *> &Strategies(void) const;

  int GetNumber(void) const  { return number; }
};

#endif    // NFPLAYER_H


