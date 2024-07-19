//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/sfg.h
// Interface to sequence form classes
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

#ifndef SFG_H
#define SFG_H

#include "gambit.h"
#include "odometer.h"
#include "gnarray.h"

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
    : number(n), player(pl), action(a), parent(p)
  {
  }
  ~Sequence() = default;

public:
  const std::string &GetName() const { return name; }
  void SetName(const std::string &s) { name = s; }

  Gambit::List<Gambit::GameAction> History() const;
  int GetNumber() const { return number; }
  Gambit::GameAction GetAction() const { return action; }
  Gambit::GameInfoset GetInfoset() const
  {
    if (action) {
      return action->GetInfoset();
    }
    return nullptr;
  }
  Gambit::GamePlayer Player() const { return player; }
  const Sequence *Parent() const { return parent; }
};

class SFSequenceSet {
protected:
  Gambit::GamePlayer efp;
  Gambit::Array<Sequence *> sequences;

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
  bool RemoveSequence(Sequence *s);
  Sequence *Find(int j);

  // Number of sequences in the SFSequenceSet
  int NumSequences() const;

  //  return the entire sequence set in a const Gambit::Array
  const Gambit::Array<Sequence *> &GetSFSequenceSet() const;
};

class Sfg {
private:
  Gambit::Game EF;
  const Gambit::BehaviorSupportProfile &efsupp;
  Gambit::Array<SFSequenceSet *> *sequences;
  gNArray<Gambit::Array<Gambit::Rational> *> *SF; // sequence form
  Gambit::Array<Gambit::RectArray<Gambit::Rational> *>
      *E; // constraint matrices for sequence form.
  Gambit::Array<int> seq;
  Gambit::PVector<int> isetFlag, isetRow;
  Gambit::Array<Gambit::List<Gambit::GameInfoset>> infosets;

  void MakeSequenceForm(const Gambit::GameNode &, const Gambit::Rational &, Gambit::Array<int>,
                        Gambit::Array<Gambit::GameInfoset>, Gambit::Array<Sequence *>);
  void GetSequenceDims(const Gambit::GameNode &);

public:
  explicit Sfg(const Gambit::BehaviorSupportProfile &);
  virtual ~Sfg();

  inline int NumSequences(int pl) const { return seq[pl]; }
  inline int NumInfosets(int pl) const { return infosets[pl].Length(); }
  inline Gambit::Array<int> NumSequences() const { return seq; }
  int TotalNumSequences() const;
  int NumPlayerInfosets() const;
  inline int NumPlayers() const { return EF->NumPlayers(); }

  inline Gambit::Array<Gambit::Rational> Payoffs(const Gambit::Array<int> &index) const
  {
    return *((*SF)[index]);
  }
  Gambit::Rational Payoff(const Gambit::Array<int> &index, int pl) const;

  Gambit::RectArray<Gambit::Rational> Constraints(int player) const { return *((*E)[player]); };
  int InfosetRowNumber(int pl, int sequence) const;
  int ActionNumber(int pl, int sequence) const;
  Gambit::GameInfoset GetInfoset(int pl, int sequence) const;
  Gambit::GameAction GetAction(int pl, int sequence) const;
  const Gambit::Game &GetEfg() const { return EF; }
  Gambit::MixedBehaviorProfile<double> ToBehav(const Gambit::PVector<double> &x) const;
  const Sequence *GetSequence(int pl, int seq) const { return ((*sequences)[pl])->Find(seq); }
};

#endif // SFG_H
