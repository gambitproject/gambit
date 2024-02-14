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
#include "sfstrat.h"
#include "gnarray.imp"
#include "gambit.h"

//----------------------------------------------------
// Sfg: Constructors, Destructors, Operators
//----------------------------------------------------

Sfg::Sfg(const Gambit::BehaviorSupportProfile &S)
  : EF(S.GetGame()), efsupp(S), seq(EF->NumPlayers()), isetFlag(S.GetGame()->NumInfosets()),
    isetRow(S.GetGame()->NumInfosets()), infosets(EF->NumPlayers())
{
  int i;
  Gambit::Array<Gambit::GameInfoset> zero(EF->NumPlayers());
  Gambit::Array<int> one(EF->NumPlayers());

  Gambit::BehaviorSupportProfile support(EF);

  for (i = 1; i <= EF->NumPlayers(); i++) {
    seq[i] = 1;
    zero[i] = nullptr;
    one[i] = 1;
  }

  isetFlag = 0;
  isetRow = 0;

  GetSequenceDims(EF->GetRoot());

  isetFlag = 0;

  gIndexOdometer index(seq);

  SF = new gNArray<Gambit::Array<Gambit::Rational> *>(seq);
  while (index.Turn()) {
    (*SF)[index.CurrentIndices()] = new Gambit::Array<Gambit::Rational>(EF->NumPlayers());
    for (i = 1; i <= EF->NumPlayers(); i++) {
      (*(*SF)[index.CurrentIndices()])[i] = (Gambit::Rational)0;
    }
  }

  E = new Gambit::Array<Gambit::RectArray<Gambit::Rational> *>(EF->NumPlayers());
  for (i = 1; i <= EF->NumPlayers(); i++) {
    (*E)[i] = new Gambit::RectArray<Gambit::Rational>(infosets[i].Length() + 1, seq[i]);
    for (int j = (*(*E)[i]).MinRow(); j <= (*(*E)[i]).MaxRow(); j++) {
      for (int k = (*(*E)[i]).MinCol(); k <= (*(*E)[i]).MaxCol(); k++) {
        (*(*E)[i])(j, k) = (Gambit::Rational)0;
      }
    }
    (*(*E)[i])(1, 1) = (Gambit::Rational)1;
  }

  sequences = new Gambit::Array<SFSequenceSet *>(EF->NumPlayers());
  for (i = 1; i <= EF->NumPlayers(); i++) {
    (*sequences)[i] = new SFSequenceSet(EF->GetPlayer(i));
  }

  Gambit::Array<Sequence *> parent(EF->NumPlayers());
  for (i = 1; i <= EF->NumPlayers(); i++) {
    parent[i] = (((*sequences)[i])->GetSFSequenceSet())[1];
  }

  MakeSequenceForm(EF->GetRoot(), (Gambit::Rational)1, one, zero, parent);
}

Sfg::~Sfg()
{
  gIndexOdometer index(seq);

  while (index.Turn()) {
    delete (*SF)[index.CurrentIndices()];
  }
  delete SF;

  int i;

  for (i = 1; i <= EF->NumPlayers(); i++) {
    delete (*E)[i];
  }
  delete E;

  for (i = 1; i <= EF->NumPlayers(); i++) {
    delete (*sequences)[i];
  }
  delete sequences;
}

void Sfg::MakeSequenceForm(const Gambit::GameNode &n, const Gambit::Rational &prob,
                           Gambit::Array<int> seq, Gambit::Array<Gambit::GameInfoset> iset,
                           Gambit::Array<Sequence *> parent)
{
  int i, pl;

  if (n->GetOutcome()) {
    for (pl = 1; pl <= seq.Length(); pl++) {
      (*(*SF)[seq])[pl] += prob * static_cast<Gambit::Rational>(n->GetOutcome()->GetPayoff(pl));
    }
  }
  if (n->GetInfoset()) {
    if (n->GetPlayer()->IsChance()) {
      for (i = 1; i <= n->NumChildren(); i++) {
        MakeSequenceForm(n->GetChild(i),
                         prob * static_cast<Gambit::Rational>(n->GetInfoset()->GetActionProb(i)),
                         seq, iset, parent);
      }
    }
    else {
      int pl = n->GetPlayer()->GetNumber();
      iset[pl] = n->GetInfoset();
      int isetnum = iset[pl]->GetNumber();
      Gambit::Array<int> snew(seq);
      snew[pl] = 1;
      for (i = 1; i < isetnum; i++) {
        if (isetRow(pl, i)) {
          snew[pl] += efsupp.NumActions(pl, i);
        }
      }

      (*(*E)[pl])(isetRow(pl, isetnum), seq[pl]) = (Gambit::Rational)1;
      Sequence *myparent(parent[pl]);

      bool flag = false;
      if (!isetFlag(pl, isetnum)) { // on first visit to iset, create new sequences
        isetFlag(pl, isetnum) = 1;
        flag = true;
      }
      for (i = 1; i <= n->NumChildren(); i++) {
        if (efsupp.Contains(n->GetInfoset()->GetAction(i))) {
          snew[pl] += 1;
          if (flag) {
            Sequence *child;
            child =
                new Sequence(n->GetPlayer(), n->GetInfoset()->GetAction(i), myparent, snew[pl]);
            parent[pl] = child;
            ((*sequences)[pl])->AddSequence(child);
          }

          (*(*E)[pl])(isetRow(pl, isetnum), snew[pl]) = -(Gambit::Rational)1;
          MakeSequenceForm(n->GetChild(i), prob, snew, iset, parent);
        }
      }
    }
  }
}

void Sfg::GetSequenceDims(const Gambit::GameNode &n)
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
        if (efsupp.Contains(n->GetInfoset()->GetAction(i))) {
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
  int isetnum = (*sequences)[pl]->Find(j)->GetInfoset()->GetNumber();
  return isetRow(pl, isetnum);
}

int Sfg::ActionNumber(int pl, int j) const
{
  if (j == 1) {
    return 0;
  }
  return efsupp.GetIndex(GetAction(pl, j));
}

Gambit::GameInfoset Sfg::GetInfoset(int pl, int j) const
{
  if (j == 1) {
    return nullptr;
  }
  return (*sequences)[pl]->Find(j)->GetInfoset();
}

Gambit::GameAction Sfg::GetAction(int pl, int j) const
{
  if (j == 1) {
    return nullptr;
  }
  return (*sequences)[pl]->Find(j)->GetAction();
}

Gambit::MixedBehaviorProfile<double> Sfg::ToBehav(const Gambit::PVector<double> &x) const
{
  Gambit::MixedBehaviorProfile<double> b(efsupp);

  b = (Gambit::Rational)0;

  Sequence *sij;
  const Sequence *parent;
  Gambit::Rational value;

  int i, j;
  for (i = 1; i <= EF->NumPlayers(); i++) {
    for (j = 2; j <= seq[i]; j++) {
      sij = ((*sequences)[i]->GetSFSequenceSet())[j];
      int sn = sij->GetNumber();
      parent = sij->Parent();

      // gout << "\ni,j,sn,iset,act: " << i << " " << j << " " << sn << " ";
      // gout << sij->GetInfoset()->GetNumber() << " " << sij->GetAction()->GetNumber();

      if (x(i, parent->GetNumber()) > (double)0) {
        value = Gambit::Rational(x(i, sn) / x(i, parent->GetNumber()));
      }
      else {
        value = Gambit::Rational(0);
      }

      b[sij->GetAction()] = value;
    }
  }
  return b;
}

Gambit::Rational Sfg::Payoff(const Gambit::Array<int> &index, int pl) const
{
  return Payoffs(index)[pl];
}

template class gNArray<Gambit::Array<Gambit::Rational> *>;
