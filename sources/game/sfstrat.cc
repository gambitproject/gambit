//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of sequence form strategy classes
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

#include "sfstrat.h"
#include "sfg.h"

//--------------------------------------
// gbtSfgSequence:  Member functions
//--------------------------------------

#include "base/glist.imp"
#include "base/garray.imp"
#include "base/gblock.imp"

gbtList<gbtGameAction> gbtSfgSequence::History(void) const 
{ 
  gbtList<gbtGameAction> h;
  gbtGameAction a = action;
  const gbtSfgSequence *s = this;
  while (!a.IsNull()) {
    h.Append(a);
    s = s->parent;
    a = s->GetAction();
  }
  return h;    
}

void gbtSfgSequence::Dump(gbtOutput &out) const
{
  int a = 0, p = 0;
  if (!action.IsNull()) a = action->GetId();
  if(parent) p = parent->GetNumber();
  out << "\nPl#: " << player->GetId() << " Seq# " << number << " act# " << a << " parent: " << p;
}

gbtOutput& operator<<(gbtOutput& s, const gbtSfgSequence& seq)
{
  seq.Dump(s);
  return s;
}


//--------------------------------------
// gbtSfgSequenceSet:  Member functions
//--------------------------------------

gbtSfgSequenceSet::gbtSfgSequenceSet(const gbtGamePlayer &p)
  : efp(p), sequences()
{
  gbtSfgSequence *empty;
  empty = new gbtSfgSequence(p,0,0,1);
  AddSequence(empty);
}

gbtSfgSequenceSet::gbtSfgSequenceSet(const gbtSfgSequenceSet &s)
  : efp(s.efp), sequences(s.sequences)
{ }

gbtSfgSequenceSet::~gbtSfgSequenceSet()
{ 

  // potential problem here?  It is not clear this is where this belongs.  
  // What if there are multiple SFSequenceSets pointing to 
  // the same sequences?

  for(int i=1;i<=sequences.Length();i++)
    delete sequences[i];   
}

gbtSfgSequenceSet &gbtSfgSequenceSet::operator=(const gbtSfgSequenceSet &s)
{
  if (this != &s) {
    efp = s.efp;
    sequences = s.sequences;
  }
  return *this;
}


bool gbtSfgSequenceSet::operator==(const gbtSfgSequenceSet &s)
{
  if (sequences.Length() != s.sequences.Length()) return (false);
  int i;
  for (i = 1; i <= sequences. Length() 
       && sequences[i] == s.sequences[i]; i++);
  if (i > sequences.Length()) return (true);
  else return (false);
}

//------------------------------------------
// gbtSfgSequenceSet: Member functions 
//------------------------------------------

// Append a sequences to the gbtSfgSequenceSet
void gbtSfgSequenceSet::AddSequence(gbtSfgSequence *s) 
{ 
  assert (efp == s->Player());
  sequences.Append(s); 
}

// Removes a sequence pointer. Returns true if the sequence was successfully
// removed, false otherwise.
bool gbtSfgSequenceSet::RemoveSequence( gbtSfgSequence *s ) 
{ 
  assert (efp == s->Player());
  int t; 
  t = sequences.Find(s); 
  if (t>0) sequences.Remove(t); 
  return (t>0); 
} 

// Finds the sequence pointer of sequence number j. Returns 0 if there 
// is no sequence with that number.  
gbtSfgSequence *gbtSfgSequenceSet::Find( int j ) 
{ 
  int t=1;
  while(t <= sequences.Length()) {
    if(sequences[t]->GetNumber() == j) return sequences[t];
    t++;
  }
  return 0;
}

// Number of Sequences in a gbtSfgSequenceSet
int gbtSfgSequenceSet::NumSequences(void) const
{
  return (sequences.Length());
}

// Return the entire sequence set
const gbtBlock<gbtSfgSequence *> &gbtSfgSequenceSet::GetSFSequenceSet(void) const
{
  return sequences;
}

//-----------------------------------------------
// gbtSfgSupport: Ctors, Dtor, Operators
//-----------------------------------------------

gbtSfgSupport::gbtSfgSupport(const gbtSfgGame &SF) 
  : bsfg(&SF), sups(SF.GetEfg()->NumPlayers())
{ 
  for (int i = 1; i <= sups.Length(); i++) {
    sups[i] = new gbtSfgSequenceSet(SF.GetEfg()->GetPlayer(i));
  }
}

gbtSfgSupport::gbtSfgSupport(const gbtSfgSupport &s)
  : bsfg(s.bsfg), sups(s.sups.Length())
{
  for (int i = 1; i <= sups.Length(); i++)
    sups[i] = new gbtSfgSequenceSet(*s.sups[i]);
}

gbtSfgSupport::~gbtSfgSupport()
{ 
  for (int i = 1; i <= sups.Length(); i++)
    delete sups[i];
}

gbtSfgSupport &gbtSfgSupport::operator=(const gbtSfgSupport &s)
{
  if (this != &s && bsfg == s.bsfg) {
    for (int i = 1; i <= sups.Length(); i++)  {
      delete sups[i];
      sups[i] = new gbtSfgSequenceSet(*s.sups[i]);
    }
  }
  return *this;
}

bool gbtSfgSupport::operator==(const gbtSfgSupport &s) const
{
  assert(sups.Length() == s.sups.Length());
  int i;
  for (i = 1; i <= sups.Length() && *sups[i] == *s.sups[i]; i++);
  if (i > sups.Length()) return (true);
  else return (false);
}
  
bool gbtSfgSupport::operator!=(const gbtSfgSupport &s) const
{
  return !(*this == s);
}

//------------------------
// gbtSfgSupport: Members
//------------------------

const gbtBlock<gbtSfgSequence *> &gbtSfgSupport::Sequences(int pl) const
{
  return (sups[pl]->GetSFSequenceSet());
}

int gbtSfgSupport::NumSequences(int pl) const
{
  return sups[pl]->NumSequences();
}

const gbtArray<int> gbtSfgSupport::NumSequences(void) const
{
  gbtArray<int> a(sups.Length());

  for (int i = 1 ; i <= a.Length(); i++)
    a[i] = sups[i]->NumSequences();
  return a;
}

int gbtSfgSupport::TotalNumSequences(void) const
{
  int total = 0;
  for (int i = 1 ; i <= sups.Length(); i++)
    total += sups[i]->NumSequences();
  return total;
}

int gbtSfgSupport::Find(gbtSfgSequence *s) const
{
  return sups[s->Player()->GetId()]->GetSFSequenceSet().Find(s);
}

void gbtSfgSupport::AddSequence(gbtSfgSequence *s)
{
  sups[s->Player()->GetId()]->AddSequence(s);
}

bool gbtSfgSupport::RemoveSequence(gbtSfgSequence *s)
{
  return sups[s->Player()->GetId()]->RemoveSequence(s);
}


// Returns true if all sequences in _THIS_ belong to _S_
bool gbtSfgSupport::IsSubset(const gbtSfgSupport &s) const
{
  assert(sups.Length() == s.sups.Length());
  for (int i = 1; i <= sups.Length(); i++)
    if (NumSequences(i) > s.NumSequences(i))
      return false;
    else  {
      const gbtBlock<gbtSfgSequence *> &strats =
        sups[i]->GetSFSequenceSet();

      for (int j = 1; j <= NumSequences(i); j++)
	if (!s.sups[i]->GetSFSequenceSet().Find(strats[j]))
	  return false;
    }
  return true;
}


void gbtSfgSupport::Dump(gbtOutput&s) const
{
  int numplayers;
  int i;
  int j;
  gbtArray<gbtSfgSequence *> strat;

  s << "{ ";
  numplayers = (*bsfg).GetEfg()->NumPlayers();
  for( i = 1; i <= numplayers; i++ )
  {
    s << "{ ";
    strat = Sequences( i );
    for (j = 1; j <= strat.Length(); j++ )
    {
      s << "\"" << strat[ j ]->GetName() << "\" ";
    }
    s << "} ";
  }
  s << "} ";
}

gbtOutput& operator<<(gbtOutput& s, const gbtSfgSupport& n)
{
  n.Dump(s);
  return s;
}

//template class gbtList<Action *>;
template class gbtBlock<gbtSfgSequence *>;
template class gbtArray<gbtSfgSequence *>;
template class gbtArray<gbtSfgSequenceSet *>;
