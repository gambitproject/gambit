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

#include "base/base.h"
#include "math/gnumber.h"
#include "game/behav.h"
#include "game/efg.h"
#include "algutils.h"  // needed for gFact

class BehavSolution {
protected:
  BehavProfile<gNumber> *m_profile;
  gPrecision m_precision;
  mutable EFSupport m_support;
  mutable gText m_creator;
  mutable gFact<gTriState> m_ANFNash, m_Nash, m_SubgamePerfect, m_Sequential;
  mutable gNumber m_epsilon, m_qreLambda, m_qreValue;
  mutable gFact<gNumber> m_liapValue;
  mutable gFact<gPVector<gNumber> > m_rnfRegret;
  gText m_name;
  mutable long m_revision;

  // PRIVATE AUXILIARY MEMBER FUNCTIONS
  gTriState GetNash(void) const;
  gTriState GetANFNash(void) const;
  gTriState GetSubgamePerfect(void) const;
  gTriState GetSequential(void) const;
  gPVector<gNumber> GetRNFRegret(void) const;

  void LevelPrecision(void);

public:
  // CONSTRUCTORS, DESTRUCTOR, CONSTRUCTIVE OPERATORS
  BehavSolution(const BehavProfile<double> &, const gText & = "User");
  BehavSolution(const BehavProfile<gRational> &, const gText & = "User");
  BehavSolution(const BehavProfile<gNumber> &, const gText & = "User");
  BehavSolution(const BehavSolution &);
  virtual ~BehavSolution();

  BehavSolution &operator=(const BehavSolution &);

  // OPERATOR OVERLOADING
  bool Equals(const BehavProfile<double> &) const;
  bool operator==(const BehavSolution &) const;
  bool operator!=(const BehavSolution &p_solution) const
    { return !(*this == p_solution); }

  void Set(const gbtEfgAction &, const gNumber &);
  void Set(int, int, int, const gNumber &);
  const gNumber &operator()(const gbtEfgAction &) const;
  gNumber operator[](const gbtEfgAction &) const;
  gNumber &operator[](const gbtEfgAction &);

  BehavSolution &operator+=(const BehavSolution &);
  BehavSolution &operator-=(const BehavSolution &);
  BehavSolution &operator*=(const gNumber &);

  // GENERAL DATA ACCESS
  efgGame &GetGame(void) const { return m_profile->GetGame(); }
  const BehavProfile<gNumber> *Profile(void) const { CheckIsValid(); return m_profile; }
  gPrecision Precision(void) const { return m_precision; }

  // Do probabilities sum to one (within m_epsilon) for each infoset?
  bool IsComplete(void) const;

  const gText &GetName(void) const { return m_name; }
  const gText &Creator(void) const { CheckIsValid(); return m_creator; }
  EFSupport Support(void) const { CheckIsValid(); return m_support; }
  const gTriState &IsNash(void) const;
  BehavSolution PolishEq(void) const;
  const gTriState &IsANFNash(void) const;
  const gTriState &IsSubgamePerfect(void) const;
  const gTriState &IsSequential(void) const;
  const gNumber &Epsilon(void) const { CheckIsValid(); return m_epsilon; }
  const gNumber &QreLambda(void) const { CheckIsValid(); return m_qreLambda; }
  const gNumber &QreValue(void) const { CheckIsValid(); return m_qreValue; }
  const gNumber &LiapValue(void) const;
  const gPVector<gNumber> &ReducedNormalFormRegret(void) const;
  const gNumber MaxRegret(void) const;
  const gNumber MaxRNFRegret(void) const;

  void SetName(const gText &p_name) { m_name = p_name; }
  void SetCreator(const gText &p_creator) { m_creator = p_creator; }
  void SetEpsilon(const gNumber &p_epsilon) { m_epsilon = p_epsilon; }
  void SetQre(const gNumber &p_qreLambda, const gNumber &p_qreValue)
    { m_qreLambda = p_qreLambda; m_qreValue = p_qreValue; }
	 
  // Force the invalidation of cached data
  void Invalidate(void) const;
  void CheckIsValid(void) const {if(!IsValid()) Invalidate();}
  bool IsValid(void) const {return (m_revision == GetGame().RevisionNumber());}

  // COMPUTATION OF INTERESTING QUANTITIES
  gNumber Payoff(int pl) const   { return m_profile->Payoff(pl); }
  
  // DATA ACCESS
  gNumber RealizProb(const Node *node) const
    { return m_profile->GetRealizProb(node); }
  gNumber BeliefProb(const Node *node) const
    { return m_profile->GetBeliefProb(node); }
  gVector<gNumber> NodeValue(const Node *node) const
    { return m_profile->GetNodeValue(node); }
  gNumber IsetProb(const gbtEfgInfoset &iset) const
    { return m_profile->GetIsetProb(iset); }
  gNumber IsetValue(const gbtEfgInfoset &iset) const
    { return m_profile->GetIsetValue(iset); }
  gNumber ActionProb(const gbtEfgAction &act) const
    { return m_profile->GetActionProb(act); }
  gNumber ActionValue(const gbtEfgAction &act) const
    { return m_profile->GetActionValue(act); }
  gNumber Regret(const gbtEfgAction &act) const
    { return m_profile->GetRegret(act); }

  // OUTPUT
  void Dump(gOutput &) const;
  void DumpInfo(gOutput &) const;
};

gOutput &operator<<(gOutput &f, const BehavSolution &);

#endif    // BEHAVSOL_H
