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

#include "game/game.h"
#include "game/mixed.h"
#include "algutils.h"  // needed for gFact

class MixedSolution   {
protected:
  gbtMixedProfile<gbtNumber> m_profile;
  gbtPrecision m_precision;
  mutable gbtNfgSupport m_support;
  mutable gbtText m_creator;
  mutable gFact<gbtTriState> m_Nash, m_Perfect, m_Proper;
  mutable gFact<gbtNumber> m_liapValue;
  mutable gbtNumber m_epsilon, m_qreLambda, m_qreValue;
  gbtArray<gbtNumber> m_payoff;
  gbtText m_name;
  mutable long m_revision;
  
  gbtTriState GetNash(void) const;
  gbtTriState GetPerfect(void) const;
  gbtTriState GetProper(void) const;

  void LevelPrecision(void);

public:
  // CONSTRUCTORS, DESTRUCTOR, AND CONSTRUCTIVE OPERATORS
  MixedSolution(const gbtMixedProfile<double> &, const gbtText & = "User");
  MixedSolution(const gbtMixedProfile<gbtRational> &, const gbtText & = "User");
  MixedSolution(const gbtMixedProfile<gbtNumber> &, const gbtText & = "User");
#if GBT_WITH_MP_FLOAT
  MixedSolution(const gbtMixedProfile<gbtMPFloat> &, const gbtText & = "User");
#endif // GBT_WITH_MP_FLOAT
  MixedSolution(const MixedSolution &);
  virtual ~MixedSolution();

  MixedSolution &operator=(const MixedSolution &);

  // OPERATOR OVERLOADING
  bool Equals(const gbtMixedProfile<double> &s) const;
  bool operator==(const MixedSolution &) const;
  bool operator!=(const MixedSolution &S) const { return !(*this == S); } 

  void SetStrategyProb(gbtGameStrategy, const gbtNumber &);
  const gbtNumber &operator()(gbtGameStrategy) const;
  const gbtNumber &GetStrategyProb(const gbtGameStrategy &p_strategy) const
    { return (*this)(p_strategy); }

  MixedSolution &operator+=(const MixedSolution &);
  MixedSolution &operator-=(const MixedSolution &);
  MixedSolution &operator*=(const gbtNumber &);

  // GENERAL DATA ACCESS
  gbtGame GetGame(void) const { return m_profile.GetGame(); }
  gbtPrecision Precision(void) const { return m_precision; }
  const gbtMixedProfile<gbtNumber> *Profile(void) const { return &m_profile; }

  // Do probabilities sum to one (within m_epsilon) for each player?)
  bool IsComplete(void) const;

  const gbtText &GetLabel(void) const { return m_name; }
  void SetLabel(const gbtText &p_name) { m_name = p_name; }

  const gbtText &GetCreator(void) const { CheckIsValid(); return m_creator; }
  const gbtNfgSupport &Support(void) const { CheckIsValid(); return m_support; }
  const gbtTriState &IsNash(void) const;
  const gbtTriState &IsPerfect(bool p_compute = false) const;
  const gbtTriState &IsProper(void) const;
  const gbtNumber &Epsilon(void) const { CheckIsValid(); return m_epsilon; }
  const gbtNumber &QreLambda(void) const { CheckIsValid(); return m_qreLambda; }
  const gbtNumber &QreValue(void) const { CheckIsValid(); return m_qreValue; }
  const gbtNumber &GetLiapValue(void) const;

  void SetEpsilon(const gbtNumber &p_epsilon) { m_epsilon = p_epsilon; }
  void SetQre(const gbtNumber &p_qreLambda, const gbtNumber &p_qreValue)
    { m_qreLambda = p_qreLambda; m_qreValue = p_qreValue; }
  
  // Force the invalidation of cached quantities
  void Invalidate(void) const;
  void CheckIsValid(void) const {if(!IsValid()) Invalidate();}
  bool IsValid(void) const {return (m_revision == GetGame()->RevisionNumber());}
  MixedSolution PolishEq() const;

  // FUNCTIONS FOR COMPATIBILITY WITH GUI
  // these are all obsolescent :)
  gbtNumber Payoff(int p_player) const { return m_profile.Payoff(p_player); }
  const gbtArray<int> &Lengths(void) const { return m_profile.Lengths(); }
  
  // PAYOFF COMPUTATION
  gbtNumber GetPayoff(gbtGamePlayer) const;
  gbtNumber GetStrategyValue(gbtGameStrategy) const;

  // OUTPUT
  void Dump(gbtOutput &) const;
  void DumpInfo(gbtOutput &) const;
};


gbtOutput &operator<<(gbtOutput &f, const MixedSolution &);


#endif    // MIXEDSOL_H
