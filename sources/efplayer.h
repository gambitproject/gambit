//#
//# FILE: player.h -- Declaration of PlayerSet data type
//#
//# $Id$
//#

#ifndef PLAYER_H
#define PLAYER_H

#include "gstring.h"
#include "gblock.h"

//
// The PlayerSet is a gBlock with the special feature that there are
// by default two players defined:  -1 (dummy) and 0 (chance)
// Players are thus (for external purposes) sequentially numbered from
// -1 through the highest "real" player number.
//
class PlayerSet   {
  private:
    gBlock<gString> players;

  public:
	//# CONSTRUCTORS AND DESTRUCTOR
	//
	// initialize the default PlayerSet
	//
    PlayerSet()
      { players.Append("DUMMY");
	players.Append("CHANCE"); }
	//
	// copy constructor
	//
    PlayerSet(const PlayerSet &s)
      { for (int i = 1; i <= s.players.Length(); i++)
	  players.Append(s.players[i]); }
	//
	// clean up after a PlayerSet
	//
    ~PlayerSet()
      { while (players.Length())  players.Remove(1); }
    
	//# OPERATOR OVERLOADING
	//
	// assignment operator
	//
    PlayerSet &operator=(const PlayerSet &s)
      { if (this != &s) { 
	  while (players.Length()) players.Remove(1);
	  for (int i = 1; i <= s.players.Length(); i++)
	    players.Append(s.players[i]);
	}
	return *this; }

	//# OPERATIONS ON PLAYERS
	//
	// add player number p to the set
	//
    void AddPlayer(int p)
      { while (players.Length() < p+2)  players.Append(gString()); }

	//
	// append a player to the player set, with name name
	//
    void AddPlayer(const gString &name)
      { players.Append(name); }

	//
	// returns the number of players in the set (not including
	//   dummy or chance)
	//
    int NumPlayers(void) const
      { return (players.Length() - 2); }

	//
	// set the name of a player
	//
    void SetPlayerName(int p, const gString &name)
      { players[p + 2] = name; }

	//
	// returns the name of a player
	//
    gString GetPlayerName(int p) const
      { return players[p + 2]; }
};
#endif    //# PLAYER_H


