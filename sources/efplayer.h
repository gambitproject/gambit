//
// FILE: player.h -- Declaration of Player data type
//
// $Id$
//

#ifndef PLAYER_H
#define PLAYER_H

#include "gstring.h"
#include "gset.h"

#ifdef __GNUG__
extern class Infoset;
#elif defined __BORLANDC__
class Infoset;
#else
#error Unsupported compiler type.
#endif   // __GNUG__, __BORLANDC__


class Player : public Handled   {
  private:
    gString *_name;
    gSet<Infoset> *_infosets;

  public:
    Player(void)   { _name = new gString;  _infosets = new gSet<Infoset>; }
    ~Player()   { delete _name;   delete _infosets; }
};

#endif    // PLAYER_H

