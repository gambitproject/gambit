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

namespace Gambit {

struct Sequence {
  friend class Sfg;
  friend class SequenceSet;

private:
  int number;
  std::string name;
  GamePlayer player;
  GameAction action;
  const Sequence *parent;

  Sequence(const GamePlayer &pl, const GameAction &a, const Sequence *p, int n)
    : number(n), player(pl), action(a), parent(p)
  {
  }

  ~Sequence() = default;

public:
  const std::string &GetName() const { return name; }
  void SetName(const std::string &s) { name = s; }

  List<GameAction> History() const;
  int GetNumber() const { return number; }
  GameAction GetAction() const { return action; }
  GameInfoset GetInfoset() const { return (action) ? action->GetInfoset() : nullptr; }
  GamePlayer GetPlayer() const { return player; }
  const Sequence *Parent() const { return parent; }
};

class SequenceSet {
protected:
  GamePlayer efp;
  Array<Sequence *> sequences;

public:
  explicit SequenceSet(const GamePlayer &p);
  SequenceSet(const SequenceSet &) = default;
  ~SequenceSet();

  SequenceSet &operator=(const SequenceSet &s);

  bool operator==(const SequenceSet &s);

  // Append a sequence to the SequenceSet
  void AddSequence(Sequence *s);

  Sequence *Find(int j);

  // Number of sequences in the SequenceSet
  int NumSequences() const;

  //  return the entire sequence set in a const Array
  const Array<Sequence *> &GetSequenceSet() const;
};

class Sfg {
private:
  Game EF;
  const BehaviorSupportProfile &efsupp;
  Array<SequenceSet *> *sequences;
  gNArray<Array<Rational> *> *SF;  // sequence form
  Array<RectArray<Rational> *> *E; // constraint matrices for sequence form.
  Array<int> seq;
  PVector<int> isetFlag, isetRow;
  Array<List<GameInfoset>> infosets;

  void MakeSequenceForm(const GameNode &, const Rational &, Array<int>, Array<GameInfoset>,
                        Array<Sequence *>);

  void GetSequenceDims(const GameNode &);

public:
  explicit Sfg(const BehaviorSupportProfile &);
  virtual ~Sfg();

  int NumSequences(int pl) const { return seq[pl]; }
  int NumInfosets(int pl) const { return infosets[pl].Length(); }
  Array<int> NumSequences() const { return seq; }
  int TotalNumSequences() const;
  int NumPlayerInfosets() const;
  int NumPlayers() const { return EF->NumPlayers(); }
  Array<Rational> Payoffs(const Array<int> &index) const { return *((*SF)[index]); }

  Rational Payoff(const Array<int> &index, int pl) const;

  RectArray<Rational> Constraints(int player) const { return *((*E)[player]); };

  int InfosetRowNumber(int pl, int sequence) const;
  int ActionNumber(int pl, int sequence) const;
  GameInfoset GetInfoset(int pl, int sequence) const;
  GameAction GetAction(int pl, int sequence) const;
  const Game &GetEfg() const { return EF; }

  MixedBehaviorProfile<double> ToBehav(const PVector<double> &x) const;

  const Sequence *GetSequence(int pl, int seq) const { return ((*sequences)[pl])->Find(seq); }
};

} // end namespace Gambit

#endif // SFG_H
