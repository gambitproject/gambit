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

void gbtSfgSequence::Dump(std::ostream &out) const
{
  int a = 0, p = 0;
  if (!action.IsNull()) a = action->GetId();
  if(parent) p = parent->GetNumber();
  out << "\nPl#: " << player->GetId() << " Seq# " << number << " act# " << a << " parent: " << p;
}

std::ostream& operator<<(std::ostream& s, const gbtSfgSequence& seq)
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
