//
// FILE: infoset.h -- Declaration of Infoset class
//
// $Id$
//

#ifndef INFOSET_H
#define INFOSET_H

#include "basic.h"
#include "ghandle.h"
#include "gstring.h"
#include "gset.h"

#include "branch.h"

#ifdef __GNUG__
extern class Node;
extern class Player;
#elif defined __BORLANDC__
class Node;
class Player;
#else
#error Unsupported compiler type.
#endif   // __GNUG__, __BORLANDC__


class Infoset : public Handled   {
  private:
    gString *_name;
    gSet<Node> *_members;
    gSet<Branch> *_branches;
    gHandle<Player> _player;

  public:
    Infoset(gHandle<Node> &n, gHandle<Player> &p);
    ~Infoset();

    gSet<Node> *InsertBranch(uint branch_number);
    gSet<Node> *DeleteBranch(uint branch_number);

    void AddMember(gHandle<Node> &n);
    void RemoveMember(gHandle<Node> &n);

    gHandle<Player> Player(void) const  { return _player; }
};


#endif   // INFOSET_H

