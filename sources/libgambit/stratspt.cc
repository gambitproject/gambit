//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of strategy classes for normal forms
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

namespace Gambit {

//===========================================================================
//                          class StrategySupport
//===========================================================================

//---------------------------------------------------------------------------
//                               Lifecycle
//---------------------------------------------------------------------------

StrategySupport::StrategySupport(const Game &p_nfg) 
  : m_nfg(p_nfg)
{ 
  for (int pl = 1; pl <= p_nfg->NumPlayers(); pl++) {
    m_support.Append(Array<GameStrategy>());
    for (int st = 1; st <= p_nfg->GetPlayer(pl)->NumStrategies(); st++) {
      m_support[pl].Append(p_nfg->GetPlayer(pl)->GetStrategy(st));
    }
  }
}

//---------------------------------------------------------------------------
//                          General information
//---------------------------------------------------------------------------

Array<int> StrategySupport::NumStrategies(void) const
{
  Array<int> a(m_support.Length());

  for (int pl = 1; pl <= a.Length(); pl++) {
    a[pl] = m_support[pl].Length();
  }
  return a;
}

int StrategySupport::MixedProfileLength(void) const
{
  int total = 0;
  for (int pl = 1; pl <= m_nfg->NumPlayers();
       total += m_support[pl++].Length());
  return total;
}

bool StrategySupport::IsSubsetOf(const StrategySupport &p_support) const
{
  if (m_nfg != p_support.m_nfg)  return false;
  for (int pl = 1; pl <= m_support.Length(); pl++) {
    if (m_support[pl].Length() > p_support.m_support[pl].Length()) {
      return false;
    }
    else {
      for (int st = 1; st <= m_support[pl].Length(); st++) {
	if (!p_support.m_support[pl].Contains(m_support[pl][st])) {
	  return false;
	}
      }
    }
  }
  return true;
}

//---------------------------------------------------------------------------
//                        Modifying the support
//---------------------------------------------------------------------------

void StrategySupport::AddStrategy(GameStrategy s)
{ 
  Array<GameStrategy> &sup = m_support[s->GetPlayer()->GetNumber()];
  if (sup.Contains(s))  return;

  int index;
  for (index = 1; 
       index <= sup.Length() && sup[index]->GetNumber() < s->GetNumber(); 
       index++);
  sup.Insert(s, index);
}

bool StrategySupport::RemoveStrategy(GameStrategy s) 
{ 
  Array<GameStrategy> &sup = m_support[s->GetPlayer()->GetNumber()];
  if (!sup.Contains(s)) return false;
  if (sup.Contains(s) && sup.Length() == 1)  return false;
  sup.Remove(sup.Find(s));
  return true;
} 


//---------------------------------------------------------------------------
//                 Identification of dominated strategies
//---------------------------------------------------------------------------

bool StrategySupport::Dominates(const GameStrategy &s, 
				const GameStrategy &t, 
				bool p_strict) const
{
  int player = s->GetPlayer()->GetNumber();

  StrategyIterator A(*this, s), B(*this, t);

  bool equal = true;
  
  do  {
    Rational ap = A.GetPayoff(player), bp = B.GetPayoff(player);
    if (p_strict && ap <= bp) {
      return false;
    }
    else if (!p_strict) {
      if (ap < bp) return false;
      else if (ap > bp) equal = false;
    }
  } while (A.NextContingency() && B.NextContingency());

  return (!equal);
}


bool StrategySupport::IsDominated(const GameStrategy &s, 
				  bool p_strict) const
{
  for (int i = 1; i <= NumStrategies(s->GetPlayer()->GetNumber()); i++) {
    if (GetStrategy(s->GetPlayer()->GetNumber(), i) != s &&
	Dominates(GetStrategy(s->GetPlayer()->GetNumber(), i), s, p_strict)) {
      return true;
    }
  }
  return false;
}

bool StrategySupport::Undominated(StrategySupport &newS, int p_player, 
				  bool p_strict, bool p_external) const
{
  Array<GameStrategy> set((p_external) ? 
			  m_nfg->GetPlayer(p_player)->NumStrategies() :
			  NumStrategies(p_player));

  if (p_external) {
    for (int st = 1; st <= set.Length(); st++) {
      set[st] = m_nfg->GetPlayer(p_player)->GetStrategy(st);
    }
  }
  else {
    for (int st = 1; st <= set.Length(); st++) {
      set[st] = GetStrategy(p_player, st);
    }
  }

  int min = 0, dis = set.Length() - 1;

  while (min <= dis) {
    int pp;
    for (pp = 0;
	 pp < min && !Dominates(set[pp+1], set[dis+1], p_strict);
	 pp++);
    if (pp < min)
      dis--;
    else  {
      GameStrategy foo = set[dis+1];
      set[dis+1] = set[min+1];
      set[min+1] = foo;

      for (int inc = min + 1; inc <= dis; )  {
	if (Dominates(set[min+1], set[dis+1], p_strict)) {
	  //p_tracefile << GetStrategy(p_player, set[dis+1])->GetNumber() << " dominated by " << GetStrategy(p_player, set[min+1])->GetNumber() << '\n';
	  dis--;
	}
	else if (Dominates(set[dis+1], set[min+1], p_strict)) {
	  //p_tracefile << GetStrategy(p_player, set[min+1])->GetNumber() << " dominated by " << GetStrategy(p_player, set[dis+1])->GetNumber() << '\n';
	  foo = set[dis+1];
	  set[dis+1] = set[min+1];
	  set[min+1] = foo;
	  dis--;
	}
	else  {
	  foo = set[dis+1];
	  set[dis+1] = set[inc+1];
	  set[inc+1] = foo;
	  inc++;
	}
      }
      min++;
    }
  }
    
  if (min + 1 <= set.Length()) {
    for (int i = min + 1; i <= set.Length(); i++) {
      newS.RemoveStrategy(set[i]);
    }
    
    return true;
  }
  else {
    return false;
  }
}

StrategySupport StrategySupport::Undominated(bool p_strict,
					     bool p_external) const
{
  StrategySupport newS(*this);

  for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++)   {
    Undominated(newS, pl, p_strict, p_external);
  }

  return newS;
}

StrategySupport 
StrategySupport::Undominated(bool p_strict, const Array<int> &players) const
{
  StrategySupport newS(*this);
  
  for (int i = 1; i <= players.Length(); i++)   {
    //tracefile << "Dominated strategies for player " << pl << ":\n";
    Undominated(newS, players[i], p_strict);
  }

  return newS;
}

} // end namespace Gambit
