//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of support class for normal forms
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

#include "game.h"
#include "nfgsupport.h"
#include "gamebase.h"
#include "mixed.h"

//==========================================================================
//                      class gbtNfgSupportPlayerRep
//==========================================================================

//
// A gbtNfgSupportPlayerRep represents a player in the "view" of a game
// afforded by a gbtNfgSupport.  In particular, the number and indexing
// of strategies (potentially) differs from the full game.
//
class gbtNfgSupportPlayerRep : public gbtGamePlayerRep {
private:
  gbtGamePlayer m_player;
  const gbtNfgSupportBase *m_support;
  
public:
  gbtNfgSupportPlayerRep(const gbtNfgSupportBase *p_support, int pl)
    : m_player(p_support->m_nfg->GetPlayer(pl)),
      m_support(p_support) { }
  virtual ~gbtNfgSupportPlayerRep() { }

  gbtText GetLabel(void) const { return m_player->GetLabel(); }
  void SetLabel(const gbtText &p_label) { m_player->SetLabel(p_label); }
  int GetId(void) const { return m_player->GetId(); }

  bool IsChance(void) const { return (GetId() == 0); }

  int NumInfosets(void) const { return m_player->NumInfosets(); }
  gbtGameInfoset NewInfoset(int p_actions)
  { throw gbtGameException(); }
  gbtGameInfoset GetInfoset(int p_index) const
  { return m_player->GetInfoset(p_index); }

  int NumStrategies(void) const
  { return m_support->NumStrats(m_player->GetId()); }
  gbtGameStrategy GetStrategy(int p_index) const; 

  gbtNumber GetMinPayoff(void) const { return m_player->GetMinPayoff(); }
  gbtNumber GetMaxPayoff(void) const { return m_player->GetMaxPayoff(); }
};


gbtGameStrategy gbtNfgSupportPlayerRep::GetStrategy(int p_index) const
{
  int count = 0;
  for (int i = 1; i <= m_support->m_strategies.Lengths()[m_player->GetId()];
       i++) {
    if (m_support->m_strategies(m_player->GetId(), i)) {
      if (++count == p_index) {
	return m_player->GetStrategy(i);
      }
    }    
  }
  return 0;
}

//==========================================================================
//                         class gbtNfgSupportBase
//==========================================================================

//--------------------------------------------------------------------------
//                   class gbtNfgSupportBase: Lifecycle
//--------------------------------------------------------------------------

gbtNfgSupportBase::gbtNfgSupportBase(gbtGameBase *p_nfg)
  : m_nfg(p_nfg), m_strategies(p_nfg->NumStrategies())
{ 
  // Initially, all strategies are contained in the support
  m_strategies = 1;
}

gbtNfgSupportRep *gbtNfgSupportBase::Copy(void) const
{
  return new gbtNfgSupportBase(*this);
}

//--------------------------------------------------------------------------
//                   class gbtNfgSupportBase: Operators
//--------------------------------------------------------------------------

bool gbtNfgSupportBase::operator==(const gbtNfgSupportRep &p_support) const
{
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    for (int st = 1; st <= NumStrats(pl); st++) {
      bool term1 = m_strategies(pl, st);
      bool term2 = p_support.Contains(m_nfg->GetPlayer(pl)->GetStrategy(st));

      if (term1 != term2) {
	return false;
      }
    }
  }
  return true;
}
  
//--------------------------------------------------------------------------
//            class gbtNfgSupportBase: Data access -- players
//--------------------------------------------------------------------------

gbtGamePlayer gbtNfgSupportBase::GetPlayer(int pl) const
{
  return new gbtNfgSupportPlayerRep(this, pl);
}

//--------------------------------------------------------------------------
//            class gbtNfgSupportBase: Data access -- strategies
//--------------------------------------------------------------------------

int gbtNfgSupportBase::NumStrats(int pl) const
{
  int total = 0;
  for (int st = 1; st <= m_strategies.Lengths()[pl]; st++) {
    total += m_strategies(pl, st);
  }
  return total;
}

gbtArray<int> gbtNfgSupportBase::NumStrategies(void) const
{
  gbtArray<int> ret(m_nfg->NumPlayers());

  for (int pl = 1; pl <= ret.Length(); pl++) {
    ret[pl] = NumStrats(pl);
  }
  return ret;
}

int gbtNfgSupportBase::MixedProfileLength(void) const
{
  int total = 0;
  for (int i = 1; i <= m_strategies.Length(); i++) {
    total += m_strategies[i];
  }
  return total;
}

int gbtNfgSupportBase::GetIndex(gbtGameStrategy p_strategy) const
{
  int pl = p_strategy->GetPlayer()->GetId();
  for (int st = 1; st <= NumStrats(pl); st++) {
    if (GetPlayer(pl)->GetStrategy(st) == p_strategy) {
      return st;
    }
  }
  return 0;
}

bool gbtNfgSupportBase::Contains(gbtGameStrategy p_strategy) const
{
  return m_strategies(p_strategy->GetPlayer()->GetId(), p_strategy->GetId());
}

//--------------------------------------------------------------------------
//                  class gbtNfgSupportBase: Manipulation
//--------------------------------------------------------------------------

void gbtNfgSupportBase::AddStrategy(gbtGameStrategy s)
{
  m_strategies(s->GetPlayer()->GetId(), s->GetId()) = 1;
}

void gbtNfgSupportBase::RemoveStrategy(gbtGameStrategy s)
{
  m_strategies(s->GetPlayer()->GetId(), s->GetId()) = 0;
}

//--------------------------------------------------------------------------
//           class gbtNfgSupportBase: Data access -- properties
//--------------------------------------------------------------------------

gbtNfgContingency gbtNfgSupportBase::NewContingency(void) const
{
  return m_nfg->NewContingency();
}

gbtMixedProfile<double> gbtNfgSupportBase::NewMixedProfile(double) const
{
  if (IsMatrix()) {
    return new gbtMixedProfileTable<double>(*const_cast<gbtNfgSupportBase *>(this));
  }
  else {
    return new gbtMixedProfileTree<double>(gbtEfgSupportBase(m_nfg));
  }
}

gbtMixedProfile<gbtRational> 
gbtNfgSupportBase::NewMixedProfile(const gbtRational &) const
{
  if (IsMatrix()) {
    return new gbtMixedProfileTable<gbtRational>(*const_cast<gbtNfgSupportBase *>(this));
  }
  else {
    return new gbtMixedProfileTree<gbtRational>(gbtEfgSupportBase(m_nfg));
  }
}

gbtMixedProfile<gbtNumber>
gbtNfgSupportBase::NewMixedProfile(const gbtNumber &) const
{
  if (IsMatrix()) {
    return new gbtMixedProfileTable<gbtNumber>(*const_cast<gbtNfgSupportBase *>(this));
  }
  else {
    return new gbtMixedProfileTree<gbtNumber>(gbtEfgSupportBase(m_nfg));
  }
}

gbtOutput& operator<<(gbtOutput &p_stream, const gbtNfgSupport &p_support)
{
  return p_stream;
}
