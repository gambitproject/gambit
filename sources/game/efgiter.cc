//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of extensive form action iterators
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

#include "efgciter.h"
#include "efgiter.h"

gbtEfgIterator::gbtEfgIterator(const gbtEfgSupport &p_support)
  : m_support(p_support),
    m_current(m_support->NumInfosets()),
    m_profile(m_support)
{
  First();
}

gbtEfgIterator::gbtEfgIterator(const gbtEfgIterator &p_iter)
  : m_support(p_iter.m_support),
    m_current(p_iter.m_current),
    m_profile(p_iter.m_profile) 
{ }

gbtEfgIterator::gbtEfgIterator(const gbtEfgContIterator &p_iter)
  : m_support(p_iter.m_support),
    m_current(p_iter.m_current),
    m_profile(p_iter.m_profile) 
{ }
  

gbtEfgIterator::~gbtEfgIterator()
{ }


gbtEfgIterator &gbtEfgIterator::operator=(const gbtEfgIterator &p_iter)
{
  if (this != &p_iter && m_support == p_iter.m_support)  {
    m_profile = p_iter.m_profile;
    m_current = p_iter.m_current;
  }
  return *this;
}


void gbtEfgIterator::First(void)
{
  m_current = 1;

  for (int pl = 1; pl <= m_support->NumPlayers(); pl++)  {
    for (int iset = 1; iset <= m_support->GetPlayer(pl)->NumInfosets(); iset++) {
      m_profile.Set(m_support->GetAction(pl, iset, 1));
    }
  }
}

int gbtEfgIterator::Next(int pl, int iset)
{  
  if (m_current(pl, iset) == m_support->NumActions(pl, iset)) {
    m_current(pl, iset) = 1;
    m_profile.Set(m_support->GetAction(pl, iset, 1));
    return 0;
  }

  m_current(pl, iset)++;
  m_profile.Set(m_support->GetAction(pl, iset, m_current(pl, iset)));
  return 1;
}

int gbtEfgIterator::Set(int pl, int iset, int act)
{
  if (pl <= 0 || pl > m_support->NumPlayers() ||
      iset <= 0 || iset > m_support->GetPlayer(pl)->NumInfosets() ||
      act <= 0 || act > m_support->NumActions(pl, iset))
    return 0;

  m_current(pl, iset) = act;
  m_profile.Set(m_support->GetAction(pl, iset, act));
  return 1;
}

gbtNumber gbtEfgIterator::GetPayoff(int pl) const
{
  gbtArray<gbtNumber> payoff(m_support->NumPlayers());
  m_profile.Payoff(payoff);
  return payoff[pl];
}

void gbtEfgIterator::GetPayoff(gbtVector<gbtNumber> &p_payoff) const
{
  m_profile.Payoff(p_payoff);
}


gbtEfgContIterator::gbtEfgContIterator(const gbtEfgSupport &p_support)
  : m_frozenPlayer(0), m_frozenInfoset(0),
    m_support(p_support), m_profile(m_support), 
    m_current(m_support->NumInfosets()),
    m_numActiveInfosets(m_support->NumPlayers())
{
  for (int pl = 1; pl <= m_support->NumPlayers(); pl++) {
    m_numActiveInfosets[pl] = 0;
    gbtGamePlayer player = m_support->GetPlayer(pl);
    gbtBlock<bool> activeForPlayer(player->NumInfosets());
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      activeForPlayer[iset] = m_support->MayReach(player->GetInfoset(iset));
      m_numActiveInfosets[pl]++;
    }
    m_isActive += activeForPlayer;
  }
  First();
}

gbtEfgContIterator::gbtEfgContIterator(const gbtEfgSupport &p_support, 
				       const gbtList<gbtGameInfoset> &p_active)
  : m_frozenPlayer(0), m_frozenInfoset(0),
    m_support(p_support), m_profile(m_support), 
    m_current(m_support->NumInfosets()),
    m_numActiveInfosets(m_support->NumPlayers())
{
  for (int pl = 1; pl <= m_support->NumPlayers(); pl++) {
    m_numActiveInfosets[pl] = 0;
    gbtGamePlayer player = m_support->GetPlayer(pl);
    gbtBlock<bool> activeForPlayer(player->NumInfosets());
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      if (p_active.Contains(player->GetInfoset(iset))) {
	activeForPlayer[iset] = true;
	m_numActiveInfosets[pl]++;
      }
      else {
	activeForPlayer[iset] = false;
      }
    }
    m_isActive += activeForPlayer;
  }
  First();
}

gbtEfgContIterator::~gbtEfgContIterator()
{ }


void gbtEfgContIterator::First(void)
{
  for (int pl = 1; pl <= m_support->NumPlayers(); pl++)  {
    for (int iset = 1; iset <= m_support->GetPlayer(pl)->NumInfosets();
	 iset++) {
      if (pl != m_frozenPlayer && iset != m_frozenInfoset)  {
	m_current(pl, iset) = 1;
	if (m_isActive[pl][iset]) {      
	  m_profile.Set(m_support->GetAction(pl, iset, 1));
	}
      }
    }
  }
}

void gbtEfgContIterator::Set(int pl, int iset, int act)
{
  if (pl != m_frozenPlayer || iset != m_frozenInfoset)   return;

  m_current(pl, iset) = act;
  m_profile.Set(m_support->GetAction(pl, iset, act));
}


void gbtEfgContIterator::Set(const gbtGameAction &a)
{
  if (a->GetInfoset()->GetPlayer()->GetId() != m_frozenPlayer ||
      a->GetInfoset()->GetId() != m_frozenInfoset) return;
  m_profile.Set(a);
}

int gbtEfgContIterator::Next(int pl, int iset)
{
  if (pl != m_frozenPlayer || iset != m_frozenInfoset)   return 1;
  
  if (m_current(pl, iset) == m_support->NumActions(pl, iset)) {
    m_current(pl, iset) = 1;
    m_profile.Set(m_support->GetAction(pl, iset, 1));
    return 0;
  }

  m_current(pl, iset)++;
  m_profile.Set(m_support->GetAction(pl, iset, m_current(pl, iset)));
  return 1;
}
  

void gbtEfgContIterator::Freeze(int pl, int iset)
{
  m_frozenPlayer = pl;
  m_frozenInfoset = iset;
  First();
}

int gbtEfgContIterator::NextContingency(void)
{
  int pl = m_support->NumPlayers();
  while (pl > 0 && m_numActiveInfosets[pl] == 0) {
    --pl;
  }
  if (pl == 0)   return 0;
  int iset = m_support->GetPlayer(pl)->NumInfosets();
    
  while (true) {
    if (m_isActive[pl][iset] && (pl != m_frozenPlayer || 
				 iset != m_frozenInfoset)) {
      if (m_current(pl, iset) < m_support->NumActions(pl, iset))  {
	m_current(pl, iset) += 1;
	m_profile.Set(m_support->GetAction(pl, iset, m_current(pl, iset)));
	return 1;
      }
      else {
	m_current(pl, iset) = 1;
	m_profile.Set(m_support->GetAction(pl, iset, 1));
      }
    }
    
    iset--;
    if (iset == 0)  {
      do  {
	--pl;
      }  while (pl > 0 && m_numActiveInfosets[pl] == 0);
      
      if (pl == 0)   return 0;
      iset = m_support->GetPlayer(pl)->NumInfosets();
    }
  }
}

gbtNumber gbtEfgContIterator::GetPayoff(int pl) const
{
  gbtArray<gbtNumber> payoff(m_support->NumPlayers());
  m_profile.Payoff(payoff);
  return payoff[pl];
}



gbtEfgConditionalContIterator::gbtEfgConditionalContIterator(const gbtEfgSupport &s)
  : _support(s),
    _profile(s), _current(s->NumInfosets()),
    _is_active(),
    _num_active_infosets(_support->NumPlayers()),
    _payoff(_support->NumPlayers())
{
  for (int pl = 1; pl <= _support->NumPlayers(); pl++) {
    _num_active_infosets[pl] = 0;
    gbtBlock<bool> active_for_pl(_support->GetPlayer(pl)->NumInfosets());
    for (int iset = 1; iset <= _support->GetPlayer(pl)->NumInfosets(); iset++) {
      active_for_pl[iset] = true;
      _num_active_infosets[pl]++;
    }
    _is_active += active_for_pl;
  }
  First();
}

gbtEfgConditionalContIterator::gbtEfgConditionalContIterator(const gbtEfgSupport &s, 
					       const gbtList<gbtGameInfoset> &active)
  : _support(s),
    _profile(s), _current(s->NumInfosets()),
    _is_active(),
    _num_active_infosets(_support->NumPlayers()),
    _payoff(_support->NumPlayers())
{
  for (int pl = 1; pl <= _support->NumPlayers(); pl++) {
    _num_active_infosets[pl] = 0;
    gbtBlock<bool> active_for_pl(_support->GetPlayer(pl)->NumInfosets());
    for (int iset = 1; iset <= _support->GetPlayer(pl)->NumInfosets(); iset++) {
      if ( active.Contains(_support->GetPlayer(pl)->GetInfoset(iset)) ) {
	active_for_pl[iset] = true;
	_num_active_infosets[pl]++;
      }
      else
	active_for_pl[iset] = false;
    }
    _is_active += active_for_pl;
  }
  First();
}

gbtEfgConditionalContIterator::~gbtEfgConditionalContIterator()
{ }


void gbtEfgConditionalContIterator::First(void)
{
  for (int pl = 1; pl <= _support->NumPlayers(); pl++)  {
    for (int iset = 1; iset <= _support->GetPlayer(pl)->NumInfosets(); iset++) {
      _current(pl, iset) = 1;
      if (_is_active[pl][iset])
	_profile.Set(_support->GetAction(pl, iset, 1));
    }
  }
}

void gbtEfgConditionalContIterator::Set(int pl, int iset, int act)
{
  _current(pl, iset) = act;
  _profile.Set(_support->GetAction(pl, iset, act));
}

void gbtEfgConditionalContIterator::Set(const gbtGameAction &a)
{
  _profile.Set(a);
}

int gbtEfgConditionalContIterator::Next(int pl, int iset)
{
  if (_current(pl, iset) == _support->NumActions(pl, iset)) {
    _current(pl, iset) = 1;
    _profile.Set(_support->GetAction(pl, iset, 1));
    return 0;
  }

  _current(pl, iset)++;
  _profile.Set(_support->GetAction(pl, iset, _current(pl, iset)));
  return 1;
}

int gbtEfgConditionalContIterator::NextContingency(void)
{
  int pl = _support->NumPlayers();
  while (pl > 0 && _num_active_infosets[pl] == 0)
    --pl;
  if (pl == 0)   return 0;
  int iset = _support->GetPlayer(pl)->NumInfosets();
    
  while (true) {

    if (_is_active[pl][iset]) 
      if (_current(pl, iset) < _support->NumActions(pl, iset))  {
	_current(pl, iset) += 1;
	_profile.Set(_support->GetAction(pl, iset, _current(pl, iset)));
	return 1;
      }
      else {
	_current(pl, iset) = 1;
	_profile.Set(_support->GetAction(pl, iset, 1));
      }
    
    iset--;
    if (iset == 0)  {
      do  {
	--pl;
      }  while (pl > 0 && _num_active_infosets[pl] == 0);
      
      if (pl == 0)   return 0;
      iset = _support->GetPlayer(pl)->NumInfosets();
    }
  }
}

gbtNumber gbtEfgConditionalContIterator::Payoff(int pl) const
{
  _profile.Payoff(_payoff);
  return _payoff[pl];
}

gbtNumber gbtEfgConditionalContIterator::Payoff(const gbtGameNode &n, int pl) const
{
  return _profile.Payoff(n,pl);
}

void gbtEfgConditionalContIterator::Dump(gbtOutput &f) const
{
  _current.Dump(f);
}
