//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/sfstrat.h
// Interface to sequence form strategy classes
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

#include "gambit.h"

struct Sequence {
friend class Sfg;
friend class SFSequenceSet;
private:
  int number;
  std::string name;
  Gambit::GamePlayer player;
  Gambit::GameAction action;
  const Sequence *parent;
  
  Sequence(const Gambit::GamePlayer &pl, const Gambit::GameAction &a, const Sequence *p, int n)
    : number(n), player(pl), action(a), parent(p) { }
  ~Sequence() = default;
public:
  const std::string &GetName() const   { return name; }
  void SetName(const std::string &s)       { name = s; }
  
  Gambit::List<Gambit::GameAction> History() const;
  int GetNumber() const        { return number; }
  Gambit::GameAction GetAction() const  {return action; }
  Gambit::GameInfoset GetInfoset() const   { if(action) return action->GetInfoset();return nullptr; }
  Gambit::GamePlayer Player() const  { return player; }
  const Sequence *Parent() const   { return parent; }
};

class SFSequenceSet {
protected:
  Gambit::GamePlayer efp;
  Gambit::Array <Sequence *> sequences;
  
public:
  SFSequenceSet(const SFSequenceSet &s); 
  explicit SFSequenceSet(const Gambit::GamePlayer &p);
  
  SFSequenceSet &operator=(const SFSequenceSet &s); 
  bool operator==(const SFSequenceSet &s);

  virtual ~SFSequenceSet();

  // Append a sequence to the SFSequenceSet
  void AddSequence(Sequence *s);

  // Removes a sequence pointer. Returns true if the sequence was successfully
  // removed, false otherwise.
  bool RemoveSequence( Sequence *s ); 
  Sequence * Find(int j);

  // Number of sequences in the SFSequenceSet
  int NumSequences() const;

  //  return the entire sequence set in a const Gambit::Array
  const Gambit::Array<Sequence *> &GetSFSequenceSet() const;
};


class Sfg;

class SFSupport {
protected:
  const Sfg *bsfg;
  Gambit::Array <SFSequenceSet *> sups;
  
public:
  explicit SFSupport(const Sfg &);
  SFSupport(const SFSupport &s); 
  virtual ~SFSupport();
  SFSupport &operator=(const SFSupport &s);

  bool operator==(const SFSupport &s) const;
  bool operator!=(const SFSupport &s) const;

  const Sfg &Game() const   { return *bsfg; }
  
  const Gambit::Array<Sequence *> &Sequences(int pl) const;

  int NumSequences(int pl) const;
  Gambit::Array<int> NumSequences() const;
  int TotalNumSequences() const;

  void AddSequence(Sequence *);
  bool RemoveSequence(Sequence *);
  
  bool IsSubset(const SFSupport &s) const;

  // returns the index of the sequence in the support if it exists,
  // otherwise returns zero
  int Find(Sequence *) const; 
};

class SequenceProfile   {
  friend class Sfg;
private:
  long index;
  Gambit::Array<Sequence *> profile;
  
public:
  explicit SequenceProfile(const Sfg &);
  SequenceProfile(const SequenceProfile &p);

  ~SequenceProfile();
  
  SequenceProfile &operator=(const SequenceProfile &);
  
  bool IsValid() const; 
  
  long GetIndex() const;
  
  Sequence *const operator[](int p) const;
  Sequence *const Get(int p) const;
  void Set(int p, Sequence  *const s);
};

#endif    // SFSTRAT_H


