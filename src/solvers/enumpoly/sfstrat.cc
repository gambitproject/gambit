//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpoly/sfstrat.cc
// Implementation of sequence form strategy classes
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

#include "sfstrat.h"
#include "sfg.h"

//--------------------------------------
// Sequence:  Member functions
//--------------------------------------

Gambit::List<Gambit::GameAction> Sequence::History() const
{
  Gambit::List<Gambit::GameAction> h;
  Gambit::GameAction a = action;
  const Sequence *s = (this);
  while (a) {
    h.push_back(a);
    s = s->parent;
    a = s->GetAction();
  }
  return h;
}

//--------------------------------------
// SFSequenceSet:  Member functions
//--------------------------------------

SFSequenceSet::SFSequenceSet(const Gambit::GamePlayer &p) : efp(p), sequences()
{
  Sequence *empty;
  empty = new Sequence(p, nullptr, nullptr, 1);
  AddSequence(empty);
}

SFSequenceSet::SFSequenceSet(const SFSequenceSet &s)

    = default;

SFSequenceSet::~SFSequenceSet()
{

  // potential problem here?  It is not clear this is where this belongs.
  // What if there are multiple SFSequenceSets pointing to
  // the same sequences?

  for (int i = 1; i <= sequences.Length(); i++) {
    delete sequences[i];
  }
}

SFSequenceSet &SFSequenceSet::operator=(const SFSequenceSet &s)
{
  if (this != &s) {
    efp = s.efp;
    sequences = s.sequences;
  }
  return *this;
}

bool SFSequenceSet::operator==(const SFSequenceSet &s)
{
  if (sequences.Length() != s.sequences.Length()) {
    return (false);
  }
  int i;
  for (i = 1; i <= sequences.Length() && sequences[i] == s.sequences[i]; i++)
    ;
  if (i > sequences.Length()) {
    return (true);
  }
  else {
    return (false);
  }
}

//------------------------------------------
// SFSequenceSet: Member functions
//------------------------------------------

// Append a sequences to the SFSequenceSet
void SFSequenceSet::AddSequence(Sequence *s)
{
  if (efp != s->Player()) {
    throw Gambit::MismatchException();
  }
  sequences.push_back(s);
}

// Removes a sequence pointer. Returns true if the sequence was successfully
// removed, false otherwise.
bool SFSequenceSet::RemoveSequence(Sequence *s)
{
  if (efp != s->Player()) {
    throw Gambit::MismatchException();
  }
  int t;
  t = sequences.Find(s);
  if (t > 0) {
    sequences.Remove(t);
  }
  return (t > 0);
}

// Finds the sequence pointer of sequence number j. Returns 0 if there
// is no sequence with that number.
Sequence *SFSequenceSet::Find(int j)
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

// Number of Sequences in a SFSequenceSet
int SFSequenceSet::NumSequences() const { return (sequences.Length()); }

// Return the entire sequence set
const Gambit::Array<Sequence *> &SFSequenceSet::GetSFSequenceSet() const { return sequences; }

//-----------------------------------------------
// SFSupport: Ctors, Dtor, Operators
//-----------------------------------------------

SFSupport::SFSupport(const Sfg &SF) : bsfg(&SF), sups(SF.GetEfg()->NumPlayers())
{
  for (int i = 1; i <= sups.Length(); i++) {
    sups[i] = new SFSequenceSet(SF.GetEfg()->GetPlayer(i));
  }
}

SFSupport::SFSupport(const SFSupport &s) : bsfg(s.bsfg), sups(s.sups.Length())
{
  for (int i = 1; i <= sups.Length(); i++) {
    sups[i] = new SFSequenceSet(*s.sups[i]);
  }
}

SFSupport::~SFSupport()
{
  for (int i = 1; i <= sups.Length(); i++) {
    delete sups[i];
  }
}

SFSupport &SFSupport::operator=(const SFSupport &s)
{
  if (this != &s && bsfg == s.bsfg) {
    for (int i = 1; i <= sups.Length(); i++) {
      delete sups[i];
      sups[i] = new SFSequenceSet(*s.sups[i]);
    }
  }
  return *this;
}

bool SFSupport::operator==(const SFSupport &s) const
{
  int i;
  for (i = 1; i <= sups.Length() && *sups[i] == *s.sups[i]; i++)
    ;
  return i > sups.Length();
}

bool SFSupport::operator!=(const SFSupport &s) const { return !(*this == s); }

//------------------------
// SFSupport: Members
//------------------------

const Gambit::Array<Sequence *> &SFSupport::Sequences(int pl) const
{
  return (sups[pl]->GetSFSequenceSet());
}

int SFSupport::NumSequences(int pl) const { return sups[pl]->NumSequences(); }

Gambit::Array<int> SFSupport::NumSequences() const
{
  Gambit::Array<int> a(sups.Length());

  for (int i = 1; i <= a.Length(); i++) {
    a[i] = sups[i]->NumSequences();
  }
  return a;
}

int SFSupport::TotalNumSequences() const
{
  int total = 0;
  for (int i = 1; i <= sups.Length(); i++) {
    total += sups[i]->NumSequences();
  }
  return total;
}

int SFSupport::Find(Sequence *s) const
{
  return sups[s->Player()->GetNumber()]->GetSFSequenceSet().Find(s);
}

void SFSupport::AddSequence(Sequence *s) { sups[s->Player()->GetNumber()]->AddSequence(s); }

bool SFSupport::RemoveSequence(Sequence *s)
{
  return sups[s->Player()->GetNumber()]->RemoveSequence(s);
}

// Returns true if all sequences in _THIS_ belong to _S_
bool SFSupport::IsSubset(const SFSupport &s) const
{
  for (int i = 1; i <= sups.Length(); i++) {
    if (NumSequences(i) > s.NumSequences(i)) {
      return false;
    }
    else {
      const Gambit::Array<Sequence *> &strats = sups[i]->GetSFSequenceSet();

      for (int j = 1; j <= NumSequences(i); j++) {
        if (!s.sups[i]->GetSFSequenceSet().Find(strats[j])) {
          return false;
        }
      }
    }
  }
  return true;
}
