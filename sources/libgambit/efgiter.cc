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

#include "libgambit.h"

EfgContIter::EfgContIter(const gbtEfgSupport &s)
  : _frozen_pl(0), _frozen_iset(0),
    _efg(s.GetGame()), _support(s),
    _profile(s.GetGame()), _current(s.GetGame()->NumInfosets()),
    _is_active(),
    _num_active_infosets(_efg->NumPlayers()),
    _payoff(_efg->NumPlayers())
{
  for (int pl = 1; pl <= _efg->NumPlayers(); pl++) {
    _num_active_infosets[pl] = 0;
    gbtArray<bool> active_for_pl(_efg->GetPlayer(pl)->NumInfosets());
    for (int iset = 1; iset <= _efg->GetPlayer(pl)->NumInfosets(); iset++) {
      active_for_pl[iset] = s.MayReach(_efg->GetPlayer(pl)->GetInfoset(iset));
      _num_active_infosets[pl]++;
    }
    _is_active.Append(active_for_pl);
  }
  First();
}

EfgContIter::EfgContIter(const gbtEfgSupport &s, const gbtList<gbtEfgInfoset>& active)
  : _frozen_pl(0), _frozen_iset(0),
    _efg(s.GetGame()), _support(s),
    _profile(s.GetGame()), _current(s.GetGame()->NumInfosets()),
    _is_active(),
    _num_active_infosets(_efg->NumPlayers()),
    _payoff(_efg->NumPlayers())
{
  for (int pl = 1; pl <= _efg->NumPlayers(); pl++) {
    _num_active_infosets[pl] = 0;
    gbtArray<bool> active_for_pl(_efg->GetPlayer(pl)->NumInfosets());
    for (int iset = 1; iset <= _efg->GetPlayer(pl)->NumInfosets(); iset++) {
      if ( active.Contains(_efg->GetPlayer(pl)->GetInfoset(iset)) ) {
	active_for_pl[iset] = true;
	_num_active_infosets[pl]++;
      }
      else
	active_for_pl[iset] = false;
    }
    _is_active.Append(active_for_pl);
  }
  First();
}

EfgContIter::~EfgContIter()
{ }


void EfgContIter::First(void)
{
  for (int pl = 1; pl <= _efg->NumPlayers(); pl++)  {
    for (int iset = 1; iset <= _efg->GetPlayer(pl)->NumInfosets(); iset++)
      if (pl != _frozen_pl && iset != _frozen_iset)   {
	_current(pl, iset) = 1;
	if (_is_active[pl][iset])      
	  _profile.Set(_support.Actions(pl, iset)[1]);
      }
  }
}

void EfgContIter::Set(int pl, int iset, int act)
{
  if (pl != _frozen_pl || iset != _frozen_iset)   return;

  _current(pl, iset) = act;
  _profile.Set(_support.Actions(pl, iset)[act]);
}


void EfgContIter::Set(const gbtEfgAction &a) 
{
  if (a->GetInfoset()->GetPlayer()->GetNumber() != _frozen_pl ||
      a->GetInfoset()->GetNumber() != _frozen_iset) return;
  _profile.Set(a);
}

int EfgContIter::Next(int pl, int iset)
{
  if (pl != _frozen_pl || iset != _frozen_iset)   return 1;

  const gbtArray<gbtEfgAction> &actions = _support.Actions(pl, iset);
  
  if (_current(pl, iset) == actions.Length())   {
    _current(pl, iset) = 1;
    _profile.Set(actions[1]);
    return 0;
  }

  _current(pl, iset)++;
  _profile.Set(actions[_current(pl, iset)]);
  return 1;
}
  

void EfgContIter::Freeze(int pl, int iset)
{
  _frozen_pl = pl;
  _frozen_iset = iset;
  First();
}

int EfgContIter::NextContingency(void)
{
  int pl = _efg->NumPlayers();
  while (pl > 0 && _num_active_infosets[pl] == 0)
    --pl;
  if (pl == 0)   return 0;
  int iset = _efg->GetPlayer(pl)->NumInfosets();
    
  while (true) {
    if (_is_active[pl][iset] && (pl != _frozen_pl || iset != _frozen_iset))
      if (_current(pl, iset) < _support.NumActions(pl, iset))  {
	_current(pl, iset) += 1;
	_profile.Set(_support.Actions(pl, iset)[_current(pl, iset)]);
	return 1;
      }
      else {
	_current(pl, iset) = 1;
	_profile.Set(_support.Actions(pl, iset)[1]);
      }
    
    iset--;
    if (iset == 0)  {
      do  {
	--pl;
      }  while (pl > 0 && _num_active_infosets[pl] == 0);
      
      if (pl == 0)   return 0;
      iset = _efg->GetPlayer(pl)->NumInfosets();
    }
  }
}

gbtRational EfgContIter::Payoff(int pl) const
{
  _profile.Payoff(_payoff);
  return _payoff[pl];
}




EfgConditionalContIter::EfgConditionalContIter(const gbtEfgSupport &s)
  : _efg(s.GetGame()), _support(s),
    _profile(s.GetGame()), _current(s.GetGame()->NumInfosets()),
    _is_active(),
    _num_active_infosets(_efg->NumPlayers()),
    _payoff(_efg->NumPlayers())
{
  for (int pl = 1; pl <= _efg->NumPlayers(); pl++) {
    _num_active_infosets[pl] = 0;
    gbtArray<bool> active_for_pl(_efg->GetPlayer(pl)->NumInfosets());
    for (int iset = 1; iset <= _efg->GetPlayer(pl)->NumInfosets(); iset++) {
      active_for_pl[iset] = true;
      _num_active_infosets[pl]++;
    }
    _is_active.Append(active_for_pl);
  }
  First();
}

EfgConditionalContIter::EfgConditionalContIter(const gbtEfgSupport &s, 
					       const gbtList<gbtEfgInfoset>& active)
  : _efg(s.GetGame()), _support(s),
    _profile(s.GetGame()), _current(s.GetGame()->NumInfosets()),
    _is_active(),
    _num_active_infosets(_efg->NumPlayers()),
    _payoff(_efg->NumPlayers())
{
  for (int pl = 1; pl <= _efg->NumPlayers(); pl++) {
    _num_active_infosets[pl] = 0;
    gbtArray<bool> active_for_pl(_efg->GetPlayer(pl)->NumInfosets());
    for (int iset = 1; iset <= _efg->GetPlayer(pl)->NumInfosets(); iset++) {
      if ( active.Contains(_efg->GetPlayer(pl)->GetInfoset(iset)) ) {
	active_for_pl[iset] = true;
	_num_active_infosets[pl]++;
      }
      else
	active_for_pl[iset] = false;
    }
    _is_active.Append(active_for_pl);
  }
  First();
}

EfgConditionalContIter::~EfgConditionalContIter()
{ }


void EfgConditionalContIter::First(void)
{
  for (int pl = 1; pl <= _efg->NumPlayers(); pl++)  {
    for (int iset = 1; iset <= _efg->GetPlayer(pl)->NumInfosets(); iset++) {
      _current(pl, iset) = 1;
      if (_is_active[pl][iset])
	_profile.Set(_support.Actions(pl, iset)[1]);
    }
  }
}

void EfgConditionalContIter::Set(int pl, int iset, int act)
{
  _current(pl, iset) = act;
  _profile.Set(_support.Actions(pl, iset)[act]);
}

void EfgConditionalContIter::Set(const gbtEfgAction &a) 
{
  _profile.Set(a);
}

int EfgConditionalContIter::Next(int pl, int iset)
{
  const gbtArray<gbtEfgAction> &actions = _support.Actions(pl, iset);
  
  if (_current(pl, iset) == actions.Length())   {
    _current(pl, iset) = 1;
    _profile.Set(actions[1]);
    return 0;
  }

  _current(pl, iset)++;
  _profile.Set(actions[_current(pl, iset)]);
  return 1;
}

int EfgConditionalContIter::NextContingency(void)
{
  int pl = _efg->NumPlayers();
  while (pl > 0 && _num_active_infosets[pl] == 0)
    --pl;
  if (pl == 0)   return 0;
  int iset = _efg->GetPlayer(pl)->NumInfosets();
    
  while (true) {

    if (_is_active[pl][iset]) 
      if (_current(pl, iset) < _support.NumActions(pl, iset))  {
	_current(pl, iset) += 1;
	_profile.Set(_support.Actions(pl, iset)[_current(pl, iset)]);
	return 1;
      }
      else {
	_current(pl, iset) = 1;
	_profile.Set(_support.Actions(pl, iset)[1]);
      }
    
    iset--;
    if (iset == 0)  {
      do  {
	--pl;
      }  while (pl > 0 && _num_active_infosets[pl] == 0);
      
      if (pl == 0)   return 0;
      iset = _efg->GetPlayer(pl)->NumInfosets();
    }
  }
}

gbtRational EfgConditionalContIter::Payoff(int pl) const
{
  _profile.Payoff(_payoff);
  return _payoff[pl];
}

gbtRational EfgConditionalContIter::Payoff(const gbtEfgNode &n, int pl) const
{
  return _profile.Payoff(n,pl);
}



