//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/sfg.cc
// Implementation of sequence form classes
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

#include "sfg.h"
#include "gnarray.imp"
#include "gambit.h"

using namespace Gambit;

namespace Gambit {

//--------------------------------------
// Sequence:  Member functions
//--------------------------------------

List<GameAction> Sequence::History() const
{
  List<GameAction> h;
  GameAction a = action;
  const Sequence *s = (this);
  while (a) {
    h.push_back(a);
    s = s->parent;
    a = s->GetAction();
  }
  return h;
}

//--------------------------------------
// SequenceSet:  Member functions
//--------------------------------------

SequenceSet::SequenceSet(const GamePlayer &p) : efp(p), sequences()
{
  AddSequence(new Sequence(p, nullptr, nullptr, 1));
}

SequenceSet::~SequenceSet()
{

  // potential problem here?  It is not clear this is where this belongs.
  // What if there are multiple SequenceSets pointing to
  // the same sequences?

  for (int i = 1; i <= sequences.Length(); i++) {
    delete sequences[i];
  }
}

//------------------------------------------
// SequenceSet: Member functions
//------------------------------------------

// Append a sequences to the SequenceSet
void SequenceSet::AddSequence(Sequence *s)
{
  if (efp != s->GetPlayer()) {
    throw MismatchException();
  }
  sequences.push_back(s);
}

// Finds the sequence pointer of sequence number j. Returns 0 if there
// is no sequence with that number.
Sequence *SequenceSet::Find(int j)
{
  int t = 1;
  while (t <= sequences.Length()) {
    if (sequences[t]->GetNumber() == j) {
      return sequences[t];
    }
    t++;
  }
  return nullptr;
}

//----------------------------------------------------
// Sfg: Constructors, Destructors, Operators
//----------------------------------------------------

Sfg::Sfg(const BehaviorSupportProfile &S)
  : support(S), seq(support.GetGame()->NumPlayers()), isetFlag(S.GetGame()->NumInfosets()),
    isetRow(S.GetGame()->NumInfosets()), infosets(support.GetGame()->NumPlayers())
{
  Array<GameInfoset> zero(support.GetGame()->NumPlayers());
  Array<int> one(support.GetGame()->NumPlayers());

  for (int i = 1; i <= support.GetGame()->NumPlayers(); i++) {
    seq[i] = 1;
    zero[i] = nullptr;
    one[i] = 1;
  }

  isetFlag = 0;
  isetRow = 0;

  GetSequenceDims(support.GetGame()->GetRoot());

  isetFlag = 0;

  gIndexOdometer index(seq);

  SF = std::make_unique<gNArray<Array<Rational> *>>(seq);
  while (index.Turn()) {
    (*SF)[index.CurrentIndices()] = new Array<Rational>(support.GetGame()->NumPlayers());
    for (int i = 1; i <= support.GetGame()->NumPlayers(); i++) {
      (*(*SF)[index.CurrentIndices()])[i] = (Rational)0;
    }
  }

  for (int i = 1; i <= support.GetGame()->NumPlayers(); i++) {
    E.push_back(new RectArray<Rational>(infosets[i].Length() + 1, seq[i]));
    for (int j = (*E[i]).MinRow(); j <= (*E[i]).MaxRow(); j++) {
      for (int k = (*E[i]).MinCol(); k <= (*E[i]).MaxCol(); k++) {
        (*E[i])(j, k) = (Rational)0;
      }
    }
    (*E[i])(1, 1) = (Rational)1;
  }

  for (int i = 1; i <= support.GetGame()->NumPlayers(); i++) {
    sequences.push_back(new SequenceSet(support.GetGame()->GetPlayer(i)));
  }

  Array<Sequence *> parent(support.GetGame()->NumPlayers());
  for (int i = 1; i <= support.GetGame()->NumPlayers(); i++) {
    parent[i] = sequences[i]->GetSequenceSet()[1];
  }

  MakeSequenceForm(support.GetGame()->GetRoot(), Rational(1), one, zero, parent);
}

Sfg::~Sfg()
{
  gIndexOdometer index(seq);

  while (index.Turn()) {
    delete (*SF)[index.CurrentIndices()];
  }

  for (int i = 1; i <= support.GetGame()->NumPlayers(); i++) {
    delete E[i];
  }

  for (int i = 1; i <= support.GetGame()->NumPlayers(); i++) {
    delete sequences[i];
  }
}

void Sfg::MakeSequenceForm(const GameNode &n, const Rational &prob, Array<int> seq,
                           Array<GameInfoset> iset, Array<Sequence *> parent)
{
  if (n->GetOutcome()) {
    for (int pl = 1; pl <= seq.Length(); pl++) {
      (*(*SF)[seq])[pl] += prob * static_cast<Rational>(n->GetOutcome()->GetPayoff(pl));
    }
  }
  if (n->GetInfoset()) {
    if (n->GetPlayer()->IsChance()) {
      for (int i = 1; i <= n->NumChildren(); i++) {
        MakeSequenceForm(n->GetChild(i),
                         prob * static_cast<Rational>(n->GetInfoset()->GetActionProb(i)), seq,
                         iset, parent);
      }
    }
    else {
      int pl = n->GetPlayer()->GetNumber();
      iset[pl] = n->GetInfoset();
      int isetnum = iset[pl]->GetNumber();
      Array<int> snew(seq);
      snew[pl] = 1;
      for (int i = 1; i < isetnum; i++) {
        if (isetRow(pl, i)) {
          snew[pl] += support.NumActions(pl, i);
        }
      }

      (*E[pl])(isetRow(pl, isetnum), seq[pl]) = (Rational)1;
      Sequence *myparent(parent[pl]);

      bool flag = false;
      if (!isetFlag(pl, isetnum)) { // on first visit to iset, create new sequences
        isetFlag(pl, isetnum) = 1;
        flag = true;
      }
      for (int i = 1; i <= n->NumChildren(); i++) {
        if (support.Contains(n->GetInfoset()->GetAction(i))) {
          snew[pl] += 1;
          if (flag) {
            Sequence *child;
            child =
                new Sequence(n->GetPlayer(), n->GetInfoset()->GetAction(i), myparent, snew[pl]);
            parent[pl] = child;
            sequences[pl]->AddSequence(child);
          }

          (*E[pl])(isetRow(pl, isetnum), snew[pl]) = -(Rational)1;
          MakeSequenceForm(n->GetChild(i), prob, snew, iset, parent);
        }
      }
    }
  }
}

void Sfg::GetSequenceDims(const GameNode &n)
{
  int i;

  if (n->GetInfoset()) {
    if (n->GetPlayer()->IsChance()) {
      for (i = 1; i <= n->NumChildren(); i++) {
        GetSequenceDims(n->GetChild(i));
      }
    }
    else {
      int pl = n->GetPlayer()->GetNumber();
      int isetnum = n->GetInfoset()->GetNumber();

      bool flag = false;
      if (!isetFlag(pl, isetnum)) { // on first visit to iset, create new sequences
        infosets[pl].push_back(n->GetInfoset());
        isetFlag(pl, isetnum) = 1;
        isetRow(pl, isetnum) = infosets[pl].Length() + 1;
        flag = true;
      }
      for (i = 1; i <= n->NumChildren(); i++) {
        if (support.Contains(n->GetInfoset()->GetAction(i))) {
          if (flag) {
            seq[pl]++;
          }
          GetSequenceDims(n->GetChild(i));
        }
      }
    }
  }
}

int Sfg::TotalNumSequences() const
{
  int tot = 0;
  for (int i = 1; i <= seq.Length(); i++) {
    tot += seq[i];
  }
  return tot;
}

int Sfg::NumPlayerInfosets() const
{
  int tot = 0;
  for (int i = 1; i <= infosets.Length(); i++) {
    tot += infosets[i].Length();
  }
  return tot;
}

int Sfg::InfosetRowNumber(int pl, int j) const
{
  if (j == 1) {
    return 0;
  }
  int isetnum = sequences[pl]->Find(j)->GetInfoset()->GetNumber();
  return isetRow(pl, isetnum);
}

int Sfg::ActionNumber(int pl, int j) const
{
  if (j == 1) {
    return 0;
  }
  return support.GetIndex(GetAction(pl, j));
}

GameInfoset Sfg::GetInfoset(int pl, int j) const
{
  return (j == 1) ? nullptr : sequences[pl]->Find(j)->GetInfoset();
}

GameAction Sfg::GetAction(int pl, int j) const
{
  return (j == 1) ? nullptr : sequences[pl]->Find(j)->GetAction();
}

MixedBehaviorProfile<double> Sfg::ToBehav(const PVector<double> &x) const
{
  MixedBehaviorProfile<double> b(support);

  b = (Rational)0;

  Sequence *sij;
  const Sequence *parent;
  Rational value;

  int i, j;
  for (i = 1; i <= support.GetGame()->NumPlayers(); i++) {
    for (j = 2; j <= seq[i]; j++) {
      sij = sequences[i]->GetSequenceSet()[j];
      int sn = sij->GetNumber();
      parent = sij->Parent();

      // gout << "\ni,j,sn,iset,act: " << i << " " << j << " " << sn << " ";
      // gout << sij->GetInfoset()->GetNumber() << " " << sij->GetAction()->GetNumber();

      if (x(i, parent->GetNumber()) > (double)0) {
        value = Rational(x(i, sn) / x(i, parent->GetNumber()));
      }
      else {
        value = Rational(0);
      }

      b[sij->GetAction()] = value;
    }
  }
  return b;
}

Rational Sfg::Payoff(const Array<int> &index, int pl) const { return Payoffs(index)[pl]; }

} // end namespace Gambit

template class gNArray<Array<Rational> *>;
