//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Declaration of behavior strategy solution class
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

#ifndef BEHAVSOL_H
#define BEHAVSOL_H

#include "game/game.h"
#include "game/behav.h"
#include "algutils.h"  // needed for gFact

class BehavSolution {
protected:
  gbtBehavProfile<gbtNumber> m_profile;
  gbtPrecision m_precision;
  mutable gbtEfgSupport m_support;
  mutable gbtText m_creator;
  mutable gFact<gbtTriState> m_ANFNash, m_Nash, m_SubgamePerfect, m_Sequential;
  mutable gbtNumber m_epsilon, m_qreLambda, m_qreValue;
  mutable gFact<gbtNumber> m_liapValue;
  mutable gFact<gbtPVector<gbtNumber> > m_rnfRegret;
  gbtText m_label;
  mutable long m_revision;

  // PRIVATE AUXILIARY MEMBER FUNCTIONS
  gbtTriState GetNash(void) const;
  gbtTriState GetANFNash(void) const;
  gbtTriState GetSubgamePerfect(void) const;
  gbtTriState GetSequential(void) const;
  gbtPVector<gbtNumber> GetRNFRegret(void) const;

  void LevelPrecision(void);

public:
  // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
  BehavSolution(const gbtBehavProfile<double> &, const gbtText & = "User");
  BehavSolution(const gbtBehavProfile<gbtRational> &, const gbtText & = "User");
  BehavSolution(const gbtBehavProfile<gbtNumber> &, const gbtText & = "User");
  BehavSolution(const BehavSolution &);
  virtual ~BehavSolution();

  BehavSolution &operator=(const BehavSolution &);

  // OPERATOR OVERLOADING
  bool Equals(const gbtBehavProfile<double> &) const;
  bool operator==(const BehavSolution &) const;
  bool operator!=(const BehavSolution &p_solution) const
    { return !(*this == p_solution); }

  void SetActionProb(const gbtGameAction &, const gbtNumber &);
  void Set(int, int, int, const gbtNumber &);
  const gbtNumber &operator()(const gbtGameAction &) const;
  const gbtNumber &GetActionProb(const gbtGameAction &p_action) const
    { return (*this)(p_action); }

  gbtNumber operator[](const gbtGameAction &) const;
  gbtNumber &operator[](const gbtGameAction &);

  // GENERAL DATA ACCESS
  gbtGame GetGame(void) const { return m_profile->GetGame(); }
  const gbtBehavProfile<gbtNumber> &Profile(void) const
  { CheckIsValid(); return m_profile; }
  gbtPrecision Precision(void) const { return m_precision; }

  // Do probabilities sum to one (within m_epsilon) for each infoset?
  bool IsComplete(void) const;

  const gbtText &GetLabel(void) const { return m_label; }
  void SetLabel(const gbtText &p_label) { m_label = p_label; }

  const gbtText &GetCreator(void) const { CheckIsValid(); return m_creator; }
  gbtEfgSupport Support(void) const { CheckIsValid(); return m_support; }
  const gbtTriState &IsNash(void) const;
  const gbtTriState &IsANFNash(void) const;
  const gbtTriState &IsSubgamePerfect(void) const;
  const gbtTriState &IsSequential(void) const;
  const gbtNumber &Epsilon(void) const { CheckIsValid(); return m_epsilon; }
  const gbtNumber &QreLambda(void) const { CheckIsValid(); return m_qreLambda; }
  const gbtNumber &QreValue(void) const { CheckIsValid(); return m_qreValue; }
  const gbtNumber &GetLiapValue(void) const;
  const gbtPVector<gbtNumber> &ReducedNormalFormRegret(void) const;
  const gbtNumber MaxRegret(void) const;
  const gbtNumber MaxRNFRegret(void) const;

  void SetCreator(const gbtText &p_creator) { m_creator = p_creator; }
  void SetEpsilon(const gbtNumber &p_epsilon) { m_epsilon = p_epsilon; }
  void SetQre(const gbtNumber &p_qreLambda, const gbtNumber &p_qreValue)
    { m_qreLambda = p_qreLambda; m_qreValue = p_qreValue; }
	 
  // Force the invalidation of cached data
  void Invalidate(void) const;
  void CheckIsValid(void) const {if(!IsValid()) Invalidate();}
  bool IsValid(void) const {return (m_revision == GetGame()->RevisionNumber());}

  // COMPUTATION OF INTERESTING QUANTITIES
  // Obsolescent version
  gbtNumber Payoff(int pl) const   { return m_profile->Payoff(pl); }

  gbtNumber GetPayoff(const gbtGamePlayer &p_player) const
    { return m_profile->Payoff(p_player->GetId()); }
  
  // DATA ACCESS
  gbtNumber GetRealizProb(const gbtGameNode &node) const
    { return m_profile->GetRealizProb(node); }
  gbtNumber GetBelief(const gbtGameNode &node) const
    { return m_profile->GetBeliefProb(node); }
  gbtNumber GetNodeValue(const gbtGameNode &p_node,
		       const gbtGamePlayer &p_player) const;
  gbtVector<gbtNumber> NodeValue(const gbtGameNode &node) const
    { return m_profile->GetNodeValue(node); }
  gbtNumber GetInfosetProb(const gbtGameInfoset &iset) const
    { return m_profile->GetIsetProb(iset); }
  gbtNumber GetInfosetValue(const gbtGameInfoset &iset) const
    { return m_profile->GetIsetValue(iset); }
  gbtNumber GetActionValue(const gbtGameAction &act) const
    { return m_profile->GetActionValue(act); }
  gbtNumber GetRegret(const gbtGameAction &act) const
    { return m_profile->GetRegret(act); }

  // OUTPUT
  void Dump(gbtOutput &) const;
  void DumpInfo(gbtOutput &) const;
};

gbtOutput &operator<<(gbtOutput &f, const BehavSolution &);

#endif    // BEHAVSOL_H
