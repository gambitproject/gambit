//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to sequence form strategy classes
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef SFSTRAT_H
#define SFSTRAT_H

#include "base/base.h"
#include "game.h"

struct gbtSfgSequence {
friend class gbtSfgGame;
friend class gbtSfgSequenceSet;
private:
  int number;
  std::string name;
  gbtGamePlayer player;
  gbtGameAction action;
  const gbtSfgSequence *parent;
  
  gbtSfgSequence(const gbtGamePlayer &pl, const gbtGameAction &a,
	   const gbtSfgSequence *p, int n) 
    : number(n), player(pl), action(a), parent(p) { }
  ~gbtSfgSequence() { }
public:
  const std::string &GetName(void) const   { return name; }
  void SetName(const std::string &s)       { name = s; }
  
  gbtList<gbtGameAction> History(void) const;
  int GetNumber(void) const        { return number; }
  gbtGameAction GetAction(void) const  {return action; }
  gbtGameInfoset GetInfoset(void) const 
  { if (!action.IsNull()) return action->GetInfoset(); else return 0; }
  gbtGamePlayer Player(void) const  { return player; }
  const gbtSfgSequence *Parent(void) const   { return parent; }
  void Dump(std::ostream &) const;
};

class gbtSfgSequenceSet {
protected:
  gbtGamePlayer efp;
  gbtBlock <gbtSfgSequence *> sequences;
  
public:
  gbtSfgSequenceSet(const gbtSfgSequenceSet &s); 
  gbtSfgSequenceSet(const gbtGamePlayer &);
  
  gbtSfgSequenceSet &operator=(const gbtSfgSequenceSet &s); 
  bool operator==(const gbtSfgSequenceSet &s);

  virtual ~gbtSfgSequenceSet();

  // Append a sequence to the gbtSfgSequenceSet
  void AddSequence(gbtSfgSequence *s);

  // Removes a sequence pointer. Returns true if the sequence was successfully
  // removed, false otherwise.
  bool RemoveSequence( gbtSfgSequence *s ); 
  gbtSfgSequence * Find(int j);

  // Number of sequences in the gbtSfgSequenceSet
  int NumSequences(void) const;

  //  return the entire sequence set in a const gbtArray
  const gbtBlock<gbtSfgSequence *> &GetSFSequenceSet(void) const;
};

class gbtSfgContingency   {
  friend class gbtSfgGame;
private:
  long index;
  gbtArray<gbtSfgSequence *> profile;
  
public:
  gbtSfgContingency(const gbtSfgGame &);
  gbtSfgContingency(const gbtSfgContingency &p);

  ~gbtSfgContingency();
  
  gbtSfgContingency &operator=(const gbtSfgContingency &);
  
  bool IsValid(void) const; 
  
  long GetIndex(void) const;
  
  gbtSfgSequence *const operator[](int p) const;
  gbtSfgSequence *const Get(int p) const;
  void Set(int p, gbtSfgSequence  *const s);
};


std::ostream &operator<<(std::ostream &f, const gbtSfgSequence &);

#endif    // SFSTRAT_H
