//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of normal form game classes
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

#include "math/rational.h"
#include "nfg.h"
#include "nfstrat.h"
#include "player.h"
#include "nfgint.h"
#include "efg.h"
#include "efgint.h"
#include "nfgiter.h"
#include "nfgciter.h"

//----------------------------------------------------------------------
//                 gbtNfgActionBase: Member functions
//----------------------------------------------------------------------

gbtNfgActionBase::gbtNfgActionBase(gbt_nfg_infoset_rep *p_infoset,
				   int p_id)
  : m_id(p_id), m_infoset(p_infoset), m_behav(0),
    m_deleted(false), m_index(0L), m_refCount(0)
{ }

gbtNfgActionBase::~gbtNfgActionBase()
{
  if (m_behav) {
    delete m_behav;
  }
}

gbtNfgPlayer gbtNfgActionBase::GetPlayer(void) const 
{ return m_infoset->m_player; }

gbtEfgStrategy gbtNfgActionBase::GetBehavior(void) const
{ return m_behav; }

gbtOutput &operator<<(gbtOutput &p_stream, const gbtNfgAction &)
{ 
  return p_stream;
}

gbt_nfg_infoset_rep::gbt_nfg_infoset_rep(gbtNfgPlayerBase *p_player,
					 int p_id, int p_br)
  : m_id(p_id), m_player(p_player), m_deleted(false),
    m_refCount(0), m_actions(p_br)
{
  for (int act = 1; act <= p_br; act++) {
    m_actions[act] = new gbtNfgActionBase(this, act);
  }
}    


gbtNfgGameBase::gbtNfgGameBase(gbtEfgGameBase *p_efg)
  : m_revision(0), m_outcomeRevision(-1),
    m_efg(p_efg)
{ }

static int Product(const gbtArray<int> &p_dim)
{
  int accum = 1;
  for (int i = 1; i <= p_dim.Length(); accum *= p_dim[i++]);
  return accum;
}

gbtNfgGameBase::~gbtNfgGameBase()
{
  for (int pl = 1; pl <= m_players.Length(); delete m_players[pl++]);
  for (int outc = 1; outc <= m_outcomes.Length(); delete m_outcomes[outc++]);
}

//
// Deletes the outcome from the normal form.
// Assumes outcome is not null.
//
void gbtNfgGameBase::DeleteOutcome(gbtNfgOutcomeBase *p_outcome)
{
  // Remove references to the outcome from the table
  for (int i = 1; i <= m_results.Length(); i++) {
    if (m_results[i] == p_outcome) {
      m_results[i] = 0;
    }
  }

  // Remove the outcome from the list of defined outcomes
  m_outcomes.Remove(m_outcomes.Find(p_outcome));

  // Renumber the remaining outcomes
  for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
    m_outcomes[outc]->m_id = outc;
  }

  m_revision++;
}

//----------------------------------------------------
// Nfg: Constructors, Destructors, Operators
//----------------------------------------------------

gbtNfgGameBase::gbtNfgGameBase(const gbtArray<int> &p_dim)
  : m_revision(0), m_outcomeRevision(-1), 
    m_label("UNTITLED"), m_dimensions(p_dim), m_players(p_dim.Length()),
    m_results(Product(p_dim)), m_efg(0)
{
  for (int pl = 1; pl <= m_players.Length(); pl++)  {
    m_players[pl] = new gbtNfgPlayerBase(this, pl, p_dim[pl]);
    m_players[pl]->m_label = ToText(pl);
    for (int st = 1; st <= p_dim[pl]; st++) {
      m_players[pl]->m_infosets[1]->m_actions[st]->m_label = ToText(st);
    }
  }
  IndexStrategies();

  for (int cont = 1; cont <= m_results.Length();
       m_results[cont++] = (gbtNfgOutcomeBase *) 0);
}

void gbtNfgGameBase::BreakLink(void)
{
  //  FIXME: work on efg<->nfg integration 
  m_efg = 0;
}

//-------------------------------
// gbtNfgGame: Member Functions
//-------------------------------

bool gbtNfgGameBase::IsConstSum(void) const
{
  int pl, index;
  gbtNumber cvalue = (gbtNumber) 0;

  if (NumOutcomes() == 0)  return true;

  for (pl = 1; pl <= NumPlayers(); pl++) {
    cvalue += GetOutcome(1)->GetPayoff(GetPlayer(pl));
  }

  for (index = 2; index <= NumOutcomes(); index++)  {
    gbtNumber thisvalue = (gbtNumber) 0;

    for (pl = 1; pl <= NumPlayers(); pl++) {
      thisvalue += GetOutcome(index)->GetPayoff(GetPlayer(pl));
    }       

    if (thisvalue > cvalue || thisvalue < cvalue) {
      return false;
    }
  }
  
  return true;
}

long gbtNfgGameBase::RevisionNumber(void) const
{ return m_revision; }

static void WriteNfg(const gbtNfgGame &p_game,
		     gbtNfgIterator &p_iter, int pl, gbtOutput &p_file)
{
  p_iter.Set(pl, 1);
  do {
    if (pl == 1) {
      for (int p = 1; p <= p_game->NumPlayers(); p++) {
	p_file << p_iter.GetPayoff(p_game->GetPlayer(p)) << ' ';
      }
      p_file << '\n';
    }
    else {
      WriteNfg(p_game, p_iter, pl - 1, p_file);
    }
  } while (p_iter.Next(pl));
} 

void gbtNfgGameBase::WriteNfg(gbtOutput &p_file) const
{ 
  p_file << "NFG 1 R";
  p_file << " \"" << EscapeQuotes(GetLabel()) << "\" { ";

  for (int i = 1; i <= NumPlayers(); i++) {
    p_file << '"' << EscapeQuotes(GetPlayer(i)->GetLabel()) << "\" ";
  }

  p_file << "}\n\n{ ";
  
  for (int i = 1; i <= NumPlayers(); i++)   {
    gbtNfgPlayer player = GetPlayer(i);
    p_file << "{ ";
    for (int j = 1; j <= player->NumStrategies(); j++)
      p_file << '"' << EscapeQuotes(player->GetStrategy(j)->GetLabel()) << "\" ";
    p_file << "}\n";
  }
  
  p_file << "}\n";

  p_file << "\"" << EscapeQuotes(m_comment) << "\"\n\n";

  int ncont = 1;
  for (int i = 1; i <= NumPlayers(); i++)
    ncont *= NumStrats(i);

  if (m_outcomes.Length() > 0) {
    p_file << "{\n";
    for (int outc = 1; outc <= m_outcomes.Length(); outc++)   {
      p_file << "{ \"" << EscapeQuotes(m_outcomes[outc]->m_label) << "\" ";
      for (int pl = 1; pl <= m_players.Length(); pl++)  {
	p_file << m_outcomes[outc]->m_payoffs[pl];
	if (pl < m_players.Length()) {
	  p_file << ", ";
	}
	else {
	  p_file << " }\n";
	}
      }
    }
    p_file << "}\n";
  
    for (int cont = 1; cont <= ncont; cont++)  {
      if (m_results[cont] != 0)
	p_file << m_results[cont]->m_id << ' ';
      else
	p_file << "0 ";
    }
    p_file << "\n";
  }
  else {
    gbtNfgSupport support(const_cast<gbtNfgGameBase *>(this));
    gbtNfgIterator iter(support);
    ::WriteNfg(const_cast<gbtNfgGameBase *>(this), iter, NumPlayers(), p_file);
  }
}

gbtNfgOutcome gbtNfgGameBase::NewOutcome(void)
{
  m_revision++;
  gbtNfgOutcomeBase *outcome = new gbtNfgOutcomeBase(this, 
						     m_outcomes.Length()+1);
  m_outcomes.Append(outcome);
  return outcome;
}

void gbtNfgGameBase::SetLabel(const gbtText &p_label) 
{
  m_label = p_label;
  m_revision++;
}

gbtText gbtNfgGameBase::GetLabel(void) const 
{ return m_label; }

void gbtNfgGameBase::SetComment(const gbtText &s)
{
  m_comment = s; 
  m_revision++;
}

gbtText gbtNfgGameBase::GetComment(void) const
{ return m_comment; }


int gbtNfgGameBase::NumPlayers(void) const 
{ 
  return (m_players.Length()); 
}

gbtNfgPlayer gbtNfgGameBase::GetPlayer(int pl) const
{
  return m_players[pl];
}

int gbtNfgGameBase::NumStrats(int pl) const
{
  return ((pl > 0 && pl <= NumPlayers()) ? 
	  m_players[pl]->m_infosets[1]->m_actions.Length() : 0);
}

const gbtArray<int> &gbtNfgGameBase::NumStrats(void) const
{
  return m_dimensions;
}

int gbtNfgGameBase::ProfileLength(void) const
{
  int nprof = 0;
  for (int i = 1; i <= m_players.Length(); i++)
    nprof += m_players[i]->m_infosets[1]->m_actions.Length();
  return nprof;
}

int gbtNfgGameBase::NumOutcomes(void) const
{
  return m_outcomes.Length(); 
}

gbtNfgOutcome gbtNfgGameBase::GetOutcome(int p_id) const
{
  return m_outcomes[p_id];
}

gbtNfgOutcome gbtNfgGameBase::GetOutcomeIndex(int p_index) const
{
  return m_results[p_index];
}

void gbtNfgGameBase::SetOutcomeIndex(int p_index, const gbtNfgOutcome &p_outcome)
{
  m_results[p_index] = dynamic_cast<gbtNfgOutcomeBase *>(p_outcome.Get());
}

gbtNfgSupport gbtNfgGameBase::NewSupport(void) const
{
  return gbtNfgSupport(const_cast<gbtNfgGameBase *>(this));
}

// ---------------------------------------
// gbtNfgGame: Private member functions
// ---------------------------------------

void gbtNfgGameBase::IndexStrategies(void)
{
  long offset = 1L;

  for (int i = 1; i <= NumPlayers(); i++)  {
    int j;
    for (j = 1; j <= NumStrats(i); j++)  {
      gbtNfgActionBase *s = m_players[i]->m_infosets[1]->m_actions[j];
      s->m_id = j;
      s->m_index = (j - 1) * offset;
    }
    offset *= (j - 1);
  }
}

void gbtNfgGameBase::InitPayoffs(void) const 
{
  if (m_outcomeRevision == RevisionNumber()) {
    return;
  }

  for (int outc = 1; outc <= NumOutcomes(); outc++) {
    for (int pl = 1; pl <= NumPlayers(); pl++) {
      m_outcomes[outc]->m_doublePayoffs[pl] = m_outcomes[outc]->m_payoffs[pl];
    }
  }

  m_outcomeRevision = RevisionNumber();
}

gbtEfgGame gbtNfgGameBase::AssociatedEfg(void) const
{
  return m_efg;
}

bool gbtNfgGameBase::HasAssociatedEfg(void) const
{
  return m_efg;
}

//-------------------------------------------------------------------------
//                           Global functions 
//-------------------------------------------------------------------------

gbtNfgGame NewNfg(const gbtArray<int> &p_dim)
{
  return gbtNfgGame(new gbtNfgGameBase(p_dim));
}
