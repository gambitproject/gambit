//
// FILE: sfstrat.cc -- Implementation of strategy representation for sfg
//
// $Id$
//

#include "sfstrat.h"
#include "sfg.h"

//--------------------------------------
// Sequence:  Member functions
//--------------------------------------

#include "base/glist.imp"
#include "base/garray.imp"
#include "base/gblock.imp"

gList<const Action *> Sequence::History(void) const 
{ 
  gList<const Action *> h;
  const Action * a = action;
  const Sequence * s = (this);
  while(a) {
    h.Append(a);
    s = s->parent;
    a = s->GetAction();
  }
  return h;    
}

void Sequence::Dump(gOutput &out) const
{
  int a = 0, p = 0;
  if(action) a = action->GetNumber();
  if(parent) p = parent->GetNumber();
  out << "\nPl#: " << player->GetNumber() << " Seq# " << number << " act# " << a << " parent: " << p;
}

gOutput& operator<<(gOutput& s, const Sequence& seq)
{
  seq.Dump(s);
  return s;
}


//--------------------------------------
// SFSequenceSet:  Member functions
//--------------------------------------

SFSequenceSet::SFSequenceSet(const EFPlayer *p)
  : efp(p), sequences()
{
  Sequence *empty;
  empty = new Sequence(p,0,0,1);
  AddSequence(empty);
}

SFSequenceSet::SFSequenceSet(const SFSequenceSet &s)
  : efp(s.efp), sequences(s.sequences)
{ }

SFSequenceSet::~SFSequenceSet()
{ 

  // potential problem here?  It is not clear this is where this belongs.  
  // What if there are multiple SFSequenceSets pointing to 
  // the same sequences?

  for(int i=1;i<=sequences.Length();i++)
    delete sequences[i];   
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
  if (sequences.Length() != s.sequences.Length()) return (false);
  int i;
  for (i = 1; i <= sequences. Length() 
       && sequences[i] == s.sequences[i]; i++);
  if (i > sequences.Length()) return (true);
  else return (false);
}

//------------------------------------------
// SFSequenceSet: Member functions 
//------------------------------------------

// Append a sequences to the SFSequenceSet
void SFSequenceSet::AddSequence(Sequence *s) 
{ 
  assert (efp == s->Player());
  sequences.Append(s); 
}

// Removes a sequence pointer. Returns true if the sequence was successfully
// removed, false otherwise.
bool SFSequenceSet::RemoveSequence( Sequence *s ) 
{ 
  assert (efp == s->Player());
  int t; 
  t = sequences.Find(s); 
  if (t>0) sequences.Remove(t); 
  return (t>0); 
} 

// Finds the sequence pointer of sequence number j. Returns 0 if there 
// is no sequence with that number.  
Sequence *SFSequenceSet::Find( int j ) 
{ 
  int t=1;
  while(t <= sequences.Length()) {
    if(sequences[t]->GetNumber() == j) return sequences[t];
    t++;
  }
  return 0;
}

// Number of Sequences in a SFSequenceSet
int SFSequenceSet::NumSequences(void) const
{
  return (sequences.Length());
}

// Return the entire sequence set
const gBlock<Sequence *> &SFSequenceSet::GetSFSequenceSet(void) const
{
  return sequences;
}

//-----------------------------------------------
// SFSupport: Ctors, Dtor, Operators
//-----------------------------------------------

SFSupport::SFSupport(const Sfg &SF) : bsfg(&SF), sups(SF.GetEfg().NumPlayers())
{ 
  for (int i = 1; i <= sups.Length(); i++)
    sups[i] = new SFSequenceSet(SF.GetEfg().Players()[i]);
}

SFSupport::SFSupport(const SFSupport &s)
  : bsfg(s.bsfg), sups(s.sups.Length())
{
  for (int i = 1; i <= sups.Length(); i++)
    sups[i] = new SFSequenceSet(*s.sups[i]);
}

SFSupport::~SFSupport()
{ 
  for (int i = 1; i <= sups.Length(); i++)
    delete sups[i];
}

SFSupport &SFSupport::operator=(const SFSupport &s)
{
  if (this != &s && bsfg == s.bsfg) {
    for (int i = 1; i <= sups.Length(); i++)  {
      delete sups[i];
      sups[i] = new SFSequenceSet(*s.sups[i]);
    }
  }
  return *this;
}

bool SFSupport::operator==(const SFSupport &s) const
{
  assert(sups.Length() == s.sups.Length());
  int i;
  for (i = 1; i <= sups.Length() && *sups[i] == *s.sups[i]; i++);
  if (i > sups.Length()) return (true);
  else return (false);
}
  
bool SFSupport::operator!=(const SFSupport &s) const
{
  return !(*this == s);
}

//------------------------
// SFSupport: Members
//------------------------

const gBlock<Sequence *> &SFSupport::Sequences(int pl) const
{
  return (sups[pl]->GetSFSequenceSet());
}

int SFSupport::NumSequences(int pl) const
{
  return sups[pl]->NumSequences();
}

const gArray<int> SFSupport::NumSequences(void) const
{
  gArray<int> a(sups.Length());

  for (int i = 1 ; i <= a.Length(); i++)
    a[i] = sups[i]->NumSequences();
  return a;
}

int SFSupport::TotalNumSequences(void) const
{
  int total = 0;
  for (int i = 1 ; i <= sups.Length(); i++)
    total += sups[i]->NumSequences();
  return total;
}

int SFSupport::Find(Sequence *s) const
{
  return sups[s->Player()->GetNumber()]->GetSFSequenceSet().Find(s);
}

void SFSupport::AddSequence(Sequence *s)
{
  sups[s->Player()->GetNumber()]->AddSequence(s);
}

bool SFSupport::RemoveSequence(Sequence *s)
{
  return sups[s->Player()->GetNumber()]->RemoveSequence(s);
}


// Returns true if all sequences in _THIS_ belong to _S_
bool SFSupport::IsSubset(const SFSupport &s) const
{
  assert(sups.Length() == s.sups.Length());
  for (int i = 1; i <= sups.Length(); i++)
    if (NumSequences(i) > s.NumSequences(i))
      return false;
    else  {
      const gBlock<Sequence *> &strats =
        sups[i]->GetSFSequenceSet();

      for (int j = 1; j <= NumSequences(i); j++)
	if (!s.sups[i]->GetSFSequenceSet().Find(strats[j]))
	  return false;
    }
  return true;
}


void SFSupport::Dump(gOutput&s) const
{
  int numplayers;
  int i;
  int j;
  gArray<Sequence *> strat;

  s << "{ ";
  numplayers = (*bsfg).GetEfg().NumPlayers();
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

gOutput& operator<<(gOutput& s, const SFSupport& n)
{
  n.Dump(s);
  return s;
}

//template class gList<Action *>;
template class gBlock<Sequence *>;
template class gArray<Sequence *>;
template class gArray<SFSequenceSet *>;
