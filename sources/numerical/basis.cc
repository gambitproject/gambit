//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of gbtBasis class
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

#include "basis.h"
#include "base/base.h"

// ---------------------------------------------------------------------------
// Class gbtBasis
// ---------------------------------------------------------------------------

// -----------------------
// C-tor, D-tor, Operators
// -----------------------

gbtBasis::gbtBasis(int first, int last, int firstlabel, int lastlabel)
  : basis(first, last), cols(firstlabel, lastlabel), 
    slacks(first, last), colBlocked(firstlabel,lastlabel),
    rowBlocked(first,last)
{
  int i;
  for(i = cols.First(); i <= cols.Last(); i++) {
    cols[i] = 0;
    colBlocked[i] = false;
  }

  for(i = basis.First(); i <= basis.Last(); i++) {
    basis[i]= - i;
    slacks[i] = i;
    rowBlocked[i] = false;
  }
  IsBasisIdent = true;
}

gbtBasis::gbtBasis(const gbtBasis &bas)
: basis(bas.basis), cols( bas.cols ), slacks( bas.slacks ),
  colBlocked(bas.colBlocked), rowBlocked(bas.rowBlocked), 
  IsBasisIdent(bas.IsBasisIdent)
{ }

gbtBasis::~gbtBasis()
{ }

gbtBasis& gbtBasis::operator=(const gbtBasis &orig)
{
  if(this != &orig) {
    basis = orig.basis; 
    cols = orig.cols;
    slacks = orig.slacks;
    rowBlocked = orig.rowBlocked;
    colBlocked = orig.colBlocked;
    IsBasisIdent = orig.IsBasisIdent;

  }
  return *this;
}

// -------------------------
// Public Members
// -------------------------

int gbtBasis::First() const
{ return basis.First();}

int gbtBasis::Last() const
{ return basis.Last();}

int gbtBasis::MinCol() const
{ return cols.First();}

int gbtBasis::MaxCol() const
{ return cols.Last();}

bool gbtBasis::IsRegColumn( int col ) const
{return col >= cols.First() && col <= cols.Last();} 
  
bool gbtBasis::IsSlackColumn( int col ) const 
{return  -col >= basis.First() && -col <= basis.Last();} 
  
int gbtBasis::Pivot(int outindex, int col)
{
  int outlabel = basis[outindex];
 
  if (IsSlackColumn(col)) slacks[-col] = outindex;
  else if (IsRegColumn(col)) cols[col] = outindex;
  else throw BadIndex(); // not a valid column to pivot in.
  
  if (IsSlackColumn(outlabel)) slacks[-outlabel] = 0;
  else if (IsRegColumn(outlabel)) cols[outlabel] = 0;
  else {
    // Note: here, should back out outindex.    
    throw BadIndex(); // not a valid column to pivot out. 
  }
  
  basis[outindex] = col;
  CheckBasis();
  
  return outlabel;
}

bool gbtBasis::Member( int col ) const
{
  int ret;

  if (IsSlackColumn(col)) ret = slacks[-col];
  else if (IsRegColumn(col)) ret = cols[col];
  else ret = 0;

  return (ret != 0);
}


int gbtBasis::Find( int col ) const
{
  int ret;

  if ( IsSlackColumn(col)) ret = slacks[-col];
  else if (IsRegColumn(col)) ret = cols[col];
  else throw BadIndex();
  
  return ret;
}

int gbtBasis::Label(int index) const
{
  return  basis[index];
}

void gbtBasis::Mark(int col )
{
  if (IsSlackColumn(col)) rowBlocked[-col] = true;
  else if (IsRegColumn(col)) colBlocked[col] = true;
}

void gbtBasis::UnMark(int col )
{
  if (IsSlackColumn(col)) rowBlocked[-col] = false;
  else if (IsRegColumn(col)) colBlocked[col] = false;
}

bool gbtBasis::IsBlocked(int col) const
{
  if (IsSlackColumn(col)) return rowBlocked[-col];
  else if (IsRegColumn(col)) return colBlocked[col];
  return false;
}

void gbtBasis::CheckBasis() 
{
  bool check = true;

  for (int i =basis.First(); i <= basis.Last() && check; i++)
    if(basis[i] != -i) check = false;
  
  IsBasisIdent = check;
}

bool gbtBasis::IsIdent()
{
  return IsBasisIdent;
}

void gbtBasis::Dump(gbtOutput &to) const
{ 
  to << "{";
  for(int i=basis.First();i<=basis.Last();i++) 
    to << "  " << basis[i];  
  to << " }";
}

gbtBasis::BadIndex::~BadIndex()
{ }

gbtText gbtBasis::BadIndex::Description(void) const
{
  return "Bad index in gbtBasis";
}

gbtOutput &operator<<(gbtOutput &to, const gbtBasis &v)
{
  v.Dump(to); return to;
}
