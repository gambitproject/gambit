//
// FILE: infoset.h -- Declaration of Infoset class
//
// $Id$
//

#ifndef INFOSET_H
#define INFOSET_H

#include "basic.h"
#include "gstring.h"
#include "gset.h"

#include "branch.h"
#include "player.h"

class Infoset   {
  private:
    struct irep   {
      gString _name;
      gSet<Branch> _branches;
      Player _player;

      int count;

      irep(void) : count(1)   { } 
    };

    irep *ip;

  public:
	// CONSTRUCTORS AND DESTRUCTOR
	// construct an information set with no branches and
	// setting the player to the dummy player
    Infoset(void)   { ip = new irep; }
	// construct an information set belonging to player Player
	// and having branches branches
    Infoset(Player &p, int branches);
	// destruct an information set
    ~Infoset()  {
      if (--ip->count == 0)
	delete ip;
      }

	// OPERATOR OVERLOADING
	// determine if two information sets are identical
    int operator==(const Infoset &infoset) const { return (ip == infoset.ip); }
    int operator!=(const Infoset &infoset) const { return (ip != infoset.ip); }

	// assign another information set to this one
    Infoset &operator=(const Infoset &infoset)  {
      infoset.ip->count++;
      if (--ip->count == 0)
	delete ip;
      ip = infoset.ip;
      return *this;
    }

	// NAMING THE INFORMATION SET
    void SetName(const gString &name)   { ip->_name = name; }
    gString Name(void) const    { return ip->_name; }

	// BRANCH MANIPULATION
	// insert a new branch as branch number branch_number
    void InsertBranch(uint branch_number);
	// delete branch number branch_number
    void DeleteBranch(uint branch_number);

	// PLAYER MANIPULATION
	// get the player who has the choice at this information set
    Player GetPlayer(void) const  { return ip->_player; }
	// set the player who has the choice at this information set
    void SetPlayer(const Player& p) const   { ip->_player = p; }
};


#endif   // INFOSET_H

