//
// FILE: basis.cc -- Instantiation of Basis class
//
//

#include "basis.h"
#include "gtext.h"

// ---------------------------------------------------------------------------
// Class Basis
// ---------------------------------------------------------------------------

// -----------------------
// C-tor, D-tor, Operators
// -----------------------

Basis::Basis(int first, int last, int firstlabel, int lastlabel)
  : label(first, last), cols(firstlabel, lastlabel), 
    slacks(first, last), artUnitEntry(lastlabel+1, lastlabel),
    colBlocked(firstlabel,lastlabel),rowBlocked(first,last)
{
  int i;
  for(i = cols.First(); i <= cols.Last(); i++) {
    cols[i] = 0;
    colBlocked[i] = false;
  }

  for(i = label.First(); i <= label.Last(); i++) {
    label[i]= - i;
    slacks[i] = i;
    rowBlocked[i] = false;
  }
  IsBasisIdent = true;
}

Basis::Basis(const Basis &bas)
: label(bas.label), cols( bas.cols ), slacks( bas.slacks ),
  artUnitEntry(bas.artUnitEntry), colBlocked(bas.colBlocked), 
  rowBlocked(bas.rowBlocked), IsBasisIdent(bas.IsBasisIdent)
{ }

Basis::~Basis()
{ }

Basis& Basis::operator=(const Basis &orig)
{
  if(this != &orig) {
    label = orig.label; 
    cols = orig.cols;
    slacks = orig.slacks;
    artUnitEntry = orig.artUnitEntry;
    rowBlocked = orig.rowBlocked;
    colBlocked = orig.colBlocked;
    IsBasisIdent = orig.IsBasisIdent;

  }
  return *this;
}

// -------------------------
// Public Members
// -------------------------

int Basis::First() const
{
  return label.First();
}

int Basis::Last() const
{
  return label.Last();
}

int Basis::FirstLabel() const
{
  return cols.First();
}

int Basis::LastLabel() const
{
  return cols.Last();
}

int Basis::Pivot(int outindex, int col)
{
  int outlabel = label[outindex];
 
  if (IsSlackColumn(col)) slacks[-col] = outindex;
  else if (IsRegColumn(col)) cols[col] = outindex;
  else throw BadIndex(); // not a valid column to pivot in.
  
  if (IsSlackColumn(outlabel)) slacks[-outlabel] = 0;
  else if (IsRegColumn(outlabel)) cols[outlabel] = 0;
  else {
    // Note: here, should back out outindex.    
    throw BadIndex(); // not a valid column to pivot out. 
  }
  
  label[outindex] = col;
  CheckBasis();
  
  return outlabel;
}

bool Basis::Member( int col ) const
{
  int ret;

  if (IsSlackColumn(col)) ret = slacks[-col];
  else if (IsRegColumn(col)) ret = cols[col];
  else ret = 0;

  return (ret != 0);
}


int Basis::Find( int col ) const
{
  int ret;

  if ( IsSlackColumn(col)) ret = slacks[-col];
  else if (IsRegColumn(col)) ret = cols[col];
  else ret = 0;
  
  assert (ret != 0);
  return ret;
}

int Basis::Label(int index) const
{
  return  label[index];
}

void Basis::Mark(int col )
{
  if (IsSlackColumn(col)) rowBlocked[-col] = true;
  else if (IsRegColumn(col)) colBlocked[col] = true;
}

void Basis::UnMark(int col )
{
  if (IsSlackColumn(col)) rowBlocked[-col] = false;
  else if (IsRegColumn(col)) colBlocked[col] = false;
}

bool Basis::IsBlocked(int col) const
{
  if (IsSlackColumn(col)) return rowBlocked[-col];
  else if (IsRegColumn(col)) return colBlocked[col];
  return false;
}

int Basis::AppendArtificial( int art )
{
  cols.Append(0);
  return artUnitEntry.Append(art);
}

void Basis::RemoveArtificial( int col )
{
  assert(IsArtifColumn(col));
  assert(cols[col]==0); // can only remove non basic columns
  cols.Remove(col);
  artUnitEntry.Remove( col);
}

gOutput &operator<<(gOutput &to, const Basis &v)
{
  v.Dump(to); return to;
}

void Basis::Dump(gOutput &to) const
{ 
  to << "{";
  for(int i=label.First();i<=label.Last();i++) 
    to << "  " << label[i];  
  to << " }";
}

void Basis::CheckBasis() 
{
  bool check = true;

  for (int i =label.First(); i <= label.Last() && check; i++)
    if(label[i] != -i) check = false;
  
  IsBasisIdent = check;
}

bool Basis::IsIdent()
{
  return IsBasisIdent;
}

Basis::BadIndex::~BadIndex()
{ }

gText Basis::BadIndex::Description(void) const
{
  return "Bad index in gArray";
}

