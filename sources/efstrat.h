//
// FILE: efstrat.h -- Supports and stuff for the extensive form games
//
// $Id$
//

#ifndef EFSTRAT_H
#define EFSTRAT_H

#include "gstring.h"
#include "gblock.h"
#include "efg.h"
#include "gpvector.h"

class Action;
class EFPlayer;

class EFActionSet;
class BaseEfg;  


class EFSupport {
  protected:
    gString name;
    const BaseEfg *befg;
    gArray<EFActionSet *> sets;

public:
  EFSupport ( const BaseEfg &);
  EFSupport ( const EFSupport &);
  virtual ~EFSupport();
  EFSupport &operator=(const EFSupport &);

  bool operator==(const EFSupport &) const;
  bool operator!=(const EFSupport &) const;

  // Return the number of players, infosets, actions in a support 
  int NumActions(int pl, int iset) const;
  gPVector<int> NumActions(void) const;

  // Returns the position of the action in the support.  Returns zero
  // if it is not there.
  int Find(Action *) const;

  const gArray<Action *> &Actions(int pl, int iset) const;

  // Action editing functions
  bool RemoveAction(Action *);
  void AddAction(Action *);

  // Returns the BaseEfg associated with this Support.
  const BaseEfg &BelongsTo(void) const;

  // Checks to see that every infoset in the support has at least one
  // action in it.
  bool IsValid(void) const;

  // Number of Sequences for the player
  int NumSequences(int pl) const;

  void Dump(gOutput& s) const;
};

gOutput &operator<<(gOutput &f, const EFSupport &);


#endif  //# EFSTRAT_H














