//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Solution class for mixed strategies
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

#ifndef MIXEDSOL_H
#define MIXEDSOL_H

#include "base/base.h"
#include "math/gnumber.h"
#include "game/nfg.h"
#include "algutils.h"  // needed for gFact

class MixedSolution   {
protected:
  MixedProfile<gNumber> m_profile;
  gPrecision m_precision;
  mutable gbtNfgSupport m_support;
  mutable gbtText m_creator;
  mutable gFact<gbtTriState> m_Nash, m_Perfect, m_Proper;
  mutable gFact<gNumber> m_liapValue;
  mutable gNumber m_epsilon, m_qreLambda, m_qreValue;
  gbtArray<gNumber> m_payoff;
  gbtText m_name;
  mutable long m_revision;
  
  gbtTriState GetNash(void) const;
  gbtTriState GetPerfect(void) const;
  gbtTriState GetProper(void) const;

  void LevelPrecision(void);

public:
  // CONSTRUCTORS, DESTRUCTOR, AND CONSTRUCTIVE OPERATORS
  MixedSolution(const MixedProfile<double> &, const gbtText & = "User");
  MixedSolution(const MixedProfile<gRational> &, const gbtText & = "User");
  MixedSolution(const MixedProfile<gNumber> &, const gbtText & = "User");
  MixedSolution(const MixedSolution &);
  virtual ~MixedSolution();

  MixedSolution &operator=(const MixedSolution &);

  // OPERATOR OVERLOADING
  bool Equals(const MixedProfile<double> &s) const;
  bool operator==(const MixedSolution &) const;
  bool operator!=(const MixedSolution &S) const { return !(*this == S); } 

  void SetStrategyProb(gbtNfgStrategy, const gNumber &);
  const gNumber &operator()(gbtNfgStrategy) const;
  const gNumber &GetStrategyProb(const gbtNfgStrategy &p_strategy) const
    { return (*this)(p_strategy); }

  MixedSolution &operator+=(const MixedSolution &);
  MixedSolution &operator-=(const MixedSolution &);
  MixedSolution &operator*=(const gNumber &);

  // GENERAL DATA ACCESS
  gbtNfgGame GetGame(void) const { return m_profile.GetGame(); }
  gPrecision Precision(void) const { return m_precision; }
  const MixedProfile<gNumber> *Profile(void) const { return &m_profile; }

  // Do probabilities sum to one (within m_epsilon) for each player?)
  bool IsComplete(void) const;

  const gbtText &GetLabel(void) const { return m_name; }
  void SetLabel(const gbtText &p_name) { m_name = p_name; }

  const gbtText &GetCreator(void) const { CheckIsValid(); return m_creator; }
  const gbtNfgSupport &Support(void) const { CheckIsValid(); return m_support; }
  const gbtTriState &IsNash(void) const;
  const gbtTriState &IsPerfect(void) const;
  const gbtTriState &IsProper(void) const;
  const gNumber &Epsilon(void) const { CheckIsValid(); return m_epsilon; }
  const gNumber &QreLambda(void) const { CheckIsValid(); return m_qreLambda; }
  const gNumber &QreValue(void) const { CheckIsValid(); return m_qreValue; }
  const gNumber &GetLiapValue(void) const;

  void SetEpsilon(const gNumber &p_epsilon) { m_epsilon = p_epsilon; }
  void SetQre(const gNumber &p_qreLambda, const gNumber &p_qreValue)
    { m_qreLambda = p_qreLambda; m_qreValue = p_qreValue; }
  
  // Force the invalidation of cached quantities
  void Invalidate(void) const;
  void CheckIsValid(void) const {if(!IsValid()) Invalidate();}
  bool IsValid(void) const {return (m_revision == GetGame().RevisionNumber());}
  MixedSolution PolishEq() const;

  // FUNCTIONS FOR COMPATIBILITY WITH GUI
  // these are all obsolescent :)
  gNumber Payoff(int p_player) const { return m_profile.Payoff(p_player); }
  const gbtArray<int> &Lengths(void) const { return m_profile.Lengths(); }
  
  // PAYOFF COMPUTATION
  gNumber GetPayoff(gbtNfgPlayer) const;
  gNumber GetStrategyValue(gbtNfgStrategy) const;

  // OUTPUT
  void Dump(gbtOutput &) const;
  void DumpInfo(gbtOutput &) const;
};


gbtOutput &operator<<(gbtOutput &f, const MixedSolution &);


#endif    // MIXEDSOL_H
