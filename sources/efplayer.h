//
// FILE: player.h -- Declaration of Player data type
//
// $Id$
//

#ifndef PLAYER_H
#define PLAYER_H

#include "gstring.h"
#include "gset.h"

class Player   {
  private:
    struct prep  {
      gString _name;

      int count;

      prep(void) : count(1)   { }
    };

    prep *p;

  public:
	// CONSTRUCTORS AND DESTRUCTOR
	// construct a new player
    Player(void)   { p = new prep; }
	// destruct a player
    ~Player()   {
      if (--p->count == 0)
	delete p;
      }

	// OPERATOR OVERLOADING
	// determine if two players are identical
    int operator==(const Player &player) const { return (p == player.p); }
    int operator!=(const Player &player) const { return (p != player.p); }

	// assign another player to this one
    Player &operator=(const Player &player)  {
      player.p->count++;
      if (--p->count == 0)
	delete p;
      p = player.p;
      return *this;
    }

	// NAMING THE PLAYER
    void SetName(const gString &name)   { p->_name = name; }
    gString Name(void) const    { return p->_name; }
};

#endif    // PLAYER_H

