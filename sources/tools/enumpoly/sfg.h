//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to sequence form classes
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

#ifndef SFG_H
#define SFG_H

#include "libgambit/libgambit.h"
#include "odometer.h"
#include "gnarray.h"
#include "sfstrat.h"

class Sfg  {
private:
  Gambit::Game EF;
  const gbtEfgSupport &efsupp;
  gbtArray<SFSequenceSet *> *sequences;
  gNArray<gbtArray<gbtRational> *> *SF;  // sequence form
  gbtArray<gbtRectArray<gbtRational> *> *E;   // constraint matrices for sequence form.  
  gbtArray<int> seq;
  gbtPVector<int> isetFlag,isetRow;
  gbtArray<gbtList<Gambit::GameInfoset> > infosets;

  void MakeSequenceForm(const Gambit::GameNode &, gbtRational,gbtArray<int>, gbtArray<Gambit::GameInfoset>,
		      gbtArray<Sequence *>);
  void GetSequenceDims(const Gambit::GameNode &);

public:
  Sfg(const gbtEfgSupport &);
  virtual ~Sfg();  

  inline int NumSequences(int pl) const {return seq[pl];}
  inline int NumInfosets(int pl) const {return infosets[pl].Length();}
  inline gbtArray<int> NumSequences() const {return seq;}
  int TotalNumSequences() const;
  int NumPlayerInfosets() const;
  inline int NumPlayers() const {return EF->NumPlayers();}
  
  inline gbtArray<gbtRational> Payoffs(const gbtArray<int> & index) const {return *((*SF)[index]);}
  gbtRational Payoff(const gbtArray<int> & index,int pl) const;

  gbtRectArray<gbtRational> Constraints(int player) const {return *((*E)[player]);};
  int InfosetRowNumber(int pl, int sequence) const;
  int ActionNumber(int pl, int sequence) const;
  Gambit::GameInfoset GetInfoset(int pl, int sequence) const;
  Gambit::GameAction GetAction(int pl, int sequence) const;
  const Gambit::Game &GetEfg(void) const {return EF;}
  gbtBehavProfile<double> ToBehav(const gbtPVector<double> &x) const;
  const Sequence* GetSequence(int pl, int seq) const {return ((*sequences)[pl])->Find(seq);}

};

#endif    // SFG_H

