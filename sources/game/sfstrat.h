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
#include "game/efg.h"

struct gbtSfgSequence {
friend class gbtSfgGame;
friend class gbtSfgSequenceSet;
private:
  int number;
  gbtText name;
  gbtEfgPlayer player;
  gbtEfgAction action;
  const gbtSfgSequence *parent;
  
  gbtSfgSequence(const gbtEfgPlayer &pl, const gbtEfgAction &a,
	   const gbtSfgSequence *p, int n) 
    : number(n), player(pl), action(a), parent(p) { }
  ~gbtSfgSequence() { }
public:
  const gbtText &GetName(void) const   { return name; }
  void SetName(const gbtText &s)       { name = s; }
  
  gbtList<gbtEfgAction> History(void) const;
  int GetNumber(void) const        { return number; }
  gbtEfgAction GetAction(void) const  {return action; }
  gbtEfgInfoset GetInfoset(void) const 
  { if (!action.IsNull()) return action->GetInfoset(); else return 0; }
  gbtEfgPlayer Player(void) const  { return player; }
  const gbtSfgSequence *Parent(void) const   { return parent; }
  void Dump(gbtOutput &) const;
};

class gbtSfgSequenceSet {
protected:
  gbtEfgPlayer efp;
  gbtBlock <gbtSfgSequence *> sequences;
  
public:
  gbtSfgSequenceSet(const gbtSfgSequenceSet &s); 
  gbtSfgSequenceSet(const gbtEfgPlayer &);
  
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


class gbtSfgSupport {
protected:
  const gbtSfgGame *bsfg;
  gbtArray <gbtSfgSequenceSet *> sups;
  
public:
  gbtSfgSupport(const gbtSfgGame &);
  gbtSfgSupport(const gbtSfgSupport &s); 
  virtual ~gbtSfgSupport();
  gbtSfgSupport &operator=(const gbtSfgSupport &s);

  bool operator==(const gbtSfgSupport &s) const;
  bool operator!=(const gbtSfgSupport &s) const;

  const gbtSfgGame &Game(void) const   { return *bsfg; }
  
  const gbtBlock<gbtSfgSequence *> &Sequences(int pl) const;

  int NumSequences(int pl) const;
  const gbtArray<int> NumSequences(void) const;
  int TotalNumSequences(void) const;

  void AddSequence(gbtSfgSequence *);
  bool RemoveSequence(gbtSfgSequence *);
  
  bool IsSubset(const gbtSfgSupport &s) const;

  // returns the index of the sequence in the support if it exists,
  // otherwise returns zero
  int Find(gbtSfgSequence *) const; 

  void Dump(gbtOutput &) const;
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


gbtOutput &operator<<(gbtOutput &f, const gbtSfgSequence &);
gbtOutput &operator<<(gbtOutput &f, const gbtSfgSupport &);

#endif    // SFSTRAT_H
