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
//                 gbt_nfg_strategy_rep: Declaration
//----------------------------------------------------------------------

struct gbt_nfg_player_rep;

gbt_nfg_strategy_rep::gbt_nfg_strategy_rep(gbt_nfg_infoset_rep *p_infoset,
					   int p_id)
  : m_id(p_id), m_infoset(p_infoset), m_behav(0),
    m_deleted(false), m_index(0L), m_refCount(0)
{ }

gbt_nfg_strategy_rep::~gbt_nfg_strategy_rep()
{
  if (m_behav) {
    delete m_behav;
  }
}

gbtNfgAction::gbtNfgAction(void)
  : rep(0)
{ }

gbtNfgAction::gbtNfgAction(gbt_nfg_strategy_rep *p_rep)
  : rep(p_rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtNfgAction::gbtNfgAction(const gbtNfgAction &p_outcome)
  : rep(p_outcome.rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtNfgAction::~gbtNfgAction()
{
  if (rep) {
    if (--rep->m_refCount == 0) {
      // delete rep;
    }
  }
}

gbtNfgAction &gbtNfgAction::operator=(const gbtNfgAction &p_outcome)
{
  if (this == &p_outcome) {
    return *this;
  }

  if (rep && --rep->m_refCount == 0) {
    // delete rep;
  }

  if ((rep = p_outcome.rep) != 0) {
    rep->m_refCount++;
  }
  return *this;
}

bool gbtNfgAction::operator==(const gbtNfgAction &p_outcome) const
{
  return (rep == p_outcome.rep);
} 

bool gbtNfgAction::operator!=(const gbtNfgAction &p_outcome) const
{
  return (rep != p_outcome.rep);
} 

int gbtNfgAction::GetId(void) const
{
  return (rep) ? rep->m_id : 0;
}

bool gbtNfgAction::IsNull(void) const
{
  return (rep == 0);
}

gbtText gbtNfgAction::GetLabel(void) const
{
  if (rep) {
    return rep->m_label;
  }
  else {
    return "";
  }
}

void gbtNfgAction::SetLabel(const gbtText &p_label)
{
  if (rep) {
    rep->m_label = p_label;
  }
}

gbtNfgPlayer gbtNfgAction::GetPlayer(void) const
{
  if (rep) {
    return rep->m_infoset->m_player;
  }
  else {
    return 0;
  }
}

const gbtArray<int> *const gbtNfgAction::GetBehavior(void) const
{ 
  if (rep) {
    return rep->m_behav;
  }
  else {
    return 0;
  }
}

long gbtNfgAction::GetIndex(void) const
{
  return (rep) ? rep->m_index : 0L;
}

gbtOutput &operator<<(gbtOutput &p_stream, const gbtNfgAction &)
{ 
  return p_stream;
}

gbt_nfg_infoset_rep::gbt_nfg_infoset_rep(gbt_nfg_player_rep *p_player,
					 int p_id, int p_br)
  : m_id(p_id), m_player(p_player), m_deleted(false),
    m_refCount(0), m_actions(p_br)
{
  for (int act = 1; act <= p_br; act++) {
    m_actions[act] = new gbt_nfg_strategy_rep(this, act);
  }
}    


gbt_nfg_game_rep::gbt_nfg_game_rep(gbt_efg_game_rep *p_efg)
  : m_refCount(1), m_revision(0), m_outcomeRevision(-1),
    m_efg(p_efg)
{ }

static int Product(const gbtArray<int> &p_dim)
{
  int accum = 1;
  for (int i = 1; i <= p_dim.Length(); accum *= p_dim[i++]);
  return accum;
}

gbt_nfg_game_rep::gbt_nfg_game_rep(const gbtArray<int> &p_dim)
  : m_refCount(1),
    m_revision(0), m_outcomeRevision(-1), 
    m_label("UNTITLED"), m_dimensions(p_dim), m_players(p_dim.Length()),
    m_results(Product(p_dim)), m_efg(0)
{ }

gbt_nfg_game_rep::~gbt_nfg_game_rep()
{
  for (int pl = 1; pl <= m_players.Length(); delete m_players[pl++]);
  for (int outc = 1; outc <= m_outcomes.Length(); delete m_outcomes[outc++]);
}

//
// Deletes the outcome from the normal form.
// Assumes outcome is not null.
//
void gbt_nfg_game_rep::DeleteOutcome(gbt_nfg_outcome_rep *p_outcome)
{
  // Remove references to the outcome from the table
  for (int i = 1; i <= m_results.Length(); i++) {
    if (m_results[i] == p_outcome) {
      m_results[i] = 0;
    }
  }

  // Remove the outcome from the list of defined outcomes
  m_outcomes.Remove(m_outcomes.Find(p_outcome));

  // If no external references, deallocate the memory;
  // otherwise, mark as "deleted"
  if (p_outcome->m_refCount == 0) {
    delete p_outcome;
  }
  else {
    p_outcome->m_deleted = true;
  }

  // Renumber the remaining outcomes
  for (int outc = 1; outc <= m_outcomes.Length(); outc++) {
    m_outcomes[outc]->m_id = outc;
  }

  m_revision++;
}

//----------------------------------------------------
// Nfg: Constructors, Destructors, Operators
//----------------------------------------------------

gbtNfgGame::gbtNfgGame(const gbtArray<int> &dim)
  : rep(new gbt_nfg_game_rep(dim))
{
  for (int pl = 1; pl <= rep->m_players.Length(); pl++)  {
    rep->m_players[pl] = new gbt_nfg_player_rep(rep, pl, dim[pl]);
    rep->m_players[pl]->m_label = ToText(pl);
    for (int st = 1; st <= dim[pl]; st++) {
      rep->m_players[pl]->m_infosets[1]->m_actions[st]->m_label = ToText(st);
    }
  }
  IndexStrategies();

  for (int cont = 1; cont <= rep->m_results.Length();
       rep->m_results[cont++] = (gbt_nfg_outcome_rep *) 0);
}

gbtNfgGame::gbtNfgGame(const gbtNfgGame &p_nfg)
  : rep(p_nfg.rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtNfgGame::gbtNfgGame(gbt_nfg_game_rep *p_rep)
  : rep(p_rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtNfgGame::~gbtNfgGame()
{
  if (rep && --rep->m_refCount == 0) {
    if (rep->m_efg)  {
      // FIXME: Work on efg<->nfg integration
    }
    //    delete rep;
  }
}

gbtNfgGame &gbtNfgGame::operator=(const gbtNfgGame &p_nfg)
{
  if (this != &p_nfg) {
    if (rep && --rep->m_refCount == 0) {
      //  delete rep;
    }

    if ((rep = p_nfg.rep) != 0) {
      rep->m_refCount++;
    }
  }
  return *this;
}

bool gbtNfgGame::operator==(const gbtNfgGame &p_nfg) const
{
  return rep == p_nfg.rep;
}

bool gbtNfgGame::operator!=(const gbtNfgGame &p_nfg) const
{
  return rep != p_nfg.rep;
}

void gbtNfgGame::BreakLink(void)
{
  if (rep->m_efg)  {
    //  FIXME: work on efg<->nfg integration 
  }
  rep->m_efg = 0;
}

//-------------------------------
// gbtNfgGame: Member Functions
//-------------------------------

bool gbtNfgGame::IsConstSum(void) const
{
  int pl, index;
  gbtNumber cvalue = (gbtNumber) 0;

  if (NumOutcomes() == 0)  return true;

  for (pl = 1; pl <= NumPlayers(); pl++) {
    cvalue += GetOutcome(1).GetPayoff(GetPlayer(pl));
  }

  for (index = 2; index <= NumOutcomes(); index++)  {
    gbtNumber thisvalue = (gbtNumber) 0;

    for (pl = 1; pl <= NumPlayers(); pl++) {
      thisvalue += GetOutcome(index).GetPayoff(GetPlayer(pl));
    }       

    if (thisvalue > cvalue || thisvalue < cvalue) {
      return false;
    }
  }
  
  return true;
}

long gbtNfgGame::RevisionNumber(void) const
{ return rep->m_revision; }

static void WriteNfg(const gbtNfgGame &p_game,
		     gbtNfgIterator &p_iter, int pl, gbtOutput &p_file)
{
  p_iter.Set(pl, 1);
  do {
    if (pl == 1) {
      for (int p = 1; p <= p_game.NumPlayers(); p++) {
	p_file << p_iter.GetPayoff(p_game.GetPlayer(p)) << ' ';
      }
      p_file << '\n';
    }
    else {
      WriteNfg(p_game, p_iter, pl - 1, p_file);
    }
  } while (p_iter.Next(pl));
} 

void gbtNfgGame::WriteNfg(gbtOutput &p_file) const
{ 
  p_file << "NFG 1 R";
  p_file << " \"" << EscapeQuotes(GetLabel()) << "\" { ";

  for (int i = 1; i <= NumPlayers(); i++) {
    p_file << '"' << EscapeQuotes(GetPlayer(i).GetLabel()) << "\" ";
  }

  p_file << "}\n\n{ ";
  
  for (int i = 1; i <= NumPlayers(); i++)   {
    gbtNfgPlayer player = GetPlayer(i);
    p_file << "{ ";
    for (int j = 1; j <= player.NumStrategies(); j++)
      p_file << '"' << EscapeQuotes(player.GetStrategy(j).GetLabel()) << "\" ";
    p_file << "}\n";
  }
  
  p_file << "}\n";

  p_file << "\"" << EscapeQuotes(rep->m_comment) << "\"\n\n";

  int ncont = 1;
  for (int i = 1; i <= NumPlayers(); i++)
    ncont *= NumStrats(i);

  if (rep->m_outcomes.Length() > 0) {
    p_file << "{\n";
    for (int outc = 1; outc <= rep->m_outcomes.Length(); outc++)   {
      p_file << "{ \"" << EscapeQuotes(rep->m_outcomes[outc]->m_label) << "\" ";
      for (int pl = 1; pl <= rep->m_players.Length(); pl++)  {
	p_file << rep->m_outcomes[outc]->m_payoffs[pl];
	if (pl < rep->m_players.Length()) {
	  p_file << ", ";
	}
	else {
	  p_file << " }\n";
	}
      }
    }
    p_file << "}\n";
  
    for (int cont = 1; cont <= ncont; cont++)  {
      if (rep->m_results[cont] != 0)
	p_file << rep->m_results[cont]->m_id << ' ';
      else
	p_file << "0 ";
    }
    p_file << "\n";
  }
  else {
    gbtNfgSupport support(*this);
    gbtNfgIterator iter(support);
    ::WriteNfg(*this, iter, NumPlayers(), p_file);
  }
}

gbtNfgOutcome gbtNfgGame::NewOutcome(void)
{
  rep->m_revision++;
  gbt_nfg_outcome_rep *outcome = new gbt_nfg_outcome_rep(rep, 
							 rep->m_outcomes.Length()+1);
  rep->m_outcomes.Append(outcome);
  return outcome;
}

void gbtNfgGame::SetLabel(const gbtText &p_label) 
{
  rep->m_label = p_label;
  rep->m_revision++;
}

gbtText gbtNfgGame::GetLabel(void) const 
{ return rep->m_label; }

void gbtNfgGame::SetComment(const gbtText &s)
{
  rep->m_comment = s; 
  rep->m_revision++;
}

gbtText gbtNfgGame::GetComment(void) const
{ return rep->m_comment; }


int gbtNfgGame::NumPlayers(void) const 
{ 
  return (rep->m_players.Length()); 
}

gbtNfgPlayer gbtNfgGame::GetPlayer(int pl) const
{
  return rep->m_players[pl];
}

int gbtNfgGame::NumStrats(int pl) const
{
  return ((pl > 0 && pl <= NumPlayers()) ? 
	  rep->m_players[pl]->m_infosets[1]->m_actions.Length() : 0);
}

const gbtArray<int> &gbtNfgGame::NumStrats(void) const
{
  return rep->m_dimensions;
}

int gbtNfgGame::ProfileLength(void) const
{
  int nprof = 0;
  for (int i = 1; i <= rep->m_players.Length(); i++)
    nprof += rep->m_players[i]->m_infosets[1]->m_actions.Length();
  return nprof;
}

int gbtNfgGame::NumOutcomes(void) const
{
  return rep->m_outcomes.Length(); 
}

gbtNfgOutcome gbtNfgGame::GetOutcome(int p_id) const
{
  return rep->m_outcomes[p_id];
}

gbtNfgOutcome gbtNfgGame::GetOutcomeIndex(int p_index) const
{
  return rep->m_results[p_index];
}

void gbtNfgGame::SetOutcomeIndex(int p_index, const gbtNfgOutcome &p_outcome)
{
  rep->m_results[p_index] = p_outcome.rep;
}

gbtNfgSupport gbtNfgGame::NewSupport(void) const
{
  return gbtNfgSupport(*this);
}

// ---------------------------------------
// gbtNfgGame: Private member functions
// ---------------------------------------

void gbtNfgGame::IndexStrategies(void)
{
  long offset = 1L;

  for (int i = 1; i <= NumPlayers(); i++)  {
    int j;
    for (j = 1; j <= NumStrats(i); j++)  {
      gbt_nfg_strategy_rep *s = rep->m_players[i]->m_infosets[1]->m_actions[j];
      s->m_id = j;
      s->m_index = (j - 1) * offset;
    }
    offset *= (j - 1);
  }
}

void gbtNfgGame::InitPayoffs(void) const 
{
  if (rep->m_outcomeRevision == RevisionNumber()) {
    return;
  }

  for (int outc = 1; outc <= NumOutcomes(); outc++) {
    for (int pl = 1; pl <= NumPlayers(); pl++) {
      rep->m_outcomes[outc]->m_doublePayoffs[pl] = rep->m_outcomes[outc]->m_payoffs[pl];
    }
  }

  rep->m_outcomeRevision = RevisionNumber();
}

gbtEfgGame gbtNfgGame::AssociatedEfg(void) const
{
  return rep->m_efg;
}

bool gbtNfgGame::HasAssociatedEfg(void) const
{
  return rep->m_efg;
}
