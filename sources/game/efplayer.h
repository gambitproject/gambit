//
// FILE: efplayer.h -- Declaration of EFPlayer data type
//
// $Id$
//

#ifndef EFPLAYER_H
#define EFPLAYER_H

#ifdef __GNUG__
#pragma interface
#endif   // __GNUG__

class EFPlayer   {
  friend class efgGame;
  friend class BehavProfile<double>;
  friend class BehavProfile<gRational>;
  friend class BehavProfile<gNumber>;
  private:
    int number;
    gText name;
    efgGame *E;
    
    gBlock<Infoset *> infosets;

    EFPlayer(efgGame *e, int n) : number(n), E(e)  { }
    ~EFPlayer();

  public:
    efgGame *Game(void) const   { return E; }
  
    const gText &GetName(void) const   { return name; }
    void SetName(const gText &s)       { name = s; }

    bool IsChance(void) const      { return (number == 0); }

    int NumInfosets(void) const    { return infosets.Length(); }
    const gArray<Infoset *> &Infosets(void) const  { return infosets; }
    const Infoset *GetInfoset(const int iset) const { return infosets[iset]; }

    int GetNumber(void) const    { return number; }
};


#endif    // EFPLAYER_H


