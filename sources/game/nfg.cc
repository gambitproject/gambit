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

//----------------------------------------------------------------------
//                gbt_nfg_outcome_rep: Declaration
//----------------------------------------------------------------------

gbt_nfg_outcome_rep::gbt_nfg_outcome_rep(gbt_nfg_game_rep *p_nfg, int p_id)
  : m_id(p_id), m_nfg(p_nfg), m_deleted(false), 
    m_payoffs(p_nfg->m_players.Length()),
    m_doublePayoffs(p_nfg->m_players.Length()),
    m_refCount(0)
{
  for (int i = 1; i <= m_payoffs.Length(); i++) {
    m_payoffs[i] = 0;
    m_doublePayoffs[i] = 0.0;
  }
}

gbtNfgOutcome::gbtNfgOutcome(void)
  : rep(0)
{ }

gbtNfgOutcome::gbtNfgOutcome(gbt_nfg_outcome_rep *p_rep)
  : rep(p_rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtNfgOutcome::gbtNfgOutcome(const gbtNfgOutcome &p_outcome)
  : rep(p_outcome.rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtNfgOutcome::~gbtNfgOutcome()
{
  if (rep) {
    if (--rep->m_refCount == 0) {
      // delete rep;
    }
  }
}

gbtNfgOutcome &gbtNfgOutcome::operator=(const gbtNfgOutcome &p_outcome)
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

bool gbtNfgOutcome::operator==(const gbtNfgOutcome &p_outcome) const
{
  return (rep == p_outcome.rep);
} 

bool gbtNfgOutcome::operator!=(const gbtNfgOutcome &p_outcome) const
{
  return (rep != p_outcome.rep);
} 

int gbtNfgOutcome::GetId(void) const
{
  return (rep) ? rep->m_id : 0;
}

bool gbtNfgOutcome::IsNull(void) const
{
  return (rep == 0);
}

gbtNfgGame gbtNfgOutcome::GetGame(void) const
{
  return (rep) ? rep->m_nfg : 0;
}

gText gbtNfgOutcome::GetLabel(void) const
{
  if (rep) {
    return rep->m_label;
  }
  else {
    return "";
  }
}

void gbtNfgOutcome::SetLabel(const gText &p_label)
{
  if (rep) {
    rep->m_label = p_label;
  }
}

gNumber gbtNfgOutcome::GetPayoff(const gbtNfgPlayer &p_player) const
{
  if (rep && p_player.rep) {
    return rep->m_payoffs[p_player.rep->m_id];
  }
  else {
    return 0;
  }
}

gArray<gNumber> gbtNfgOutcome::GetPayoff(void) const
{
  if (rep) {
    return rep->m_payoffs;
  }
  else {
    return gArray<gNumber>();
  }
}

double gbtNfgOutcome::GetPayoffDouble(int p_player) const
{
  if (rep) {
    return rep->m_payoffs[p_player];
  }
  else {
    return 0;
  }
}

void gbtNfgOutcome::SetPayoff(const gbtNfgPlayer &p_player,
			      const gNumber &p_value)
{
  if (rep && p_player.rep) {
    rep->m_payoffs[p_player.rep->m_id] = p_value;
    rep->m_doublePayoffs[p_player.rep->m_id] = (double) p_value;
    // FIXME: tell game to update cached values
  }
}

void gbtNfgOutcome::SetPayoff(const gArray<gNumber> &p_value)
{
  if (rep && rep->m_payoffs.Length() == p_value.Length()) {
    (gArray<gNumber> &) rep->m_payoffs = p_value;
  }
}


gOutput &operator<<(gOutput &p_stream, const gbtNfgOutcome &)
{ 
  return p_stream;
}

//----------------------------------------------------------------------
//                 gbt_nfg_strategy_rep: Declaration
//----------------------------------------------------------------------

struct gbt_nfg_player_rep;

gbt_nfg_strategy_rep::gbt_nfg_strategy_rep(gbt_nfg_player_rep *p_player)
  : m_id(0), m_player(p_player), m_behav(0),
    m_deleted(false), m_index(0L), m_refCount(0)
{ }

gbt_nfg_strategy_rep::~gbt_nfg_strategy_rep()
{
  if (m_behav) {
    delete m_behav;
  }
}

gbtNfgStrategy::gbtNfgStrategy(void)
  : rep(0)
{ }

gbtNfgStrategy::gbtNfgStrategy(gbt_nfg_strategy_rep *p_rep)
  : rep(p_rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtNfgStrategy::gbtNfgStrategy(const gbtNfgStrategy &p_outcome)
  : rep(p_outcome.rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtNfgStrategy::~gbtNfgStrategy()
{
  if (rep) {
    if (--rep->m_refCount == 0) {
      // delete rep;
    }
  }
}

gbtNfgStrategy &gbtNfgStrategy::operator=(const gbtNfgStrategy &p_outcome)
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

bool gbtNfgStrategy::operator==(const gbtNfgStrategy &p_outcome) const
{
  return (rep == p_outcome.rep);
} 

bool gbtNfgStrategy::operator!=(const gbtNfgStrategy &p_outcome) const
{
  return (rep != p_outcome.rep);
} 

int gbtNfgStrategy::GetId(void) const
{
  return (rep) ? rep->m_id : 0;
}

bool gbtNfgStrategy::IsNull(void) const
{
  return (rep == 0);
}

gText gbtNfgStrategy::GetLabel(void) const
{
  if (rep) {
    return rep->m_label;
  }
  else {
    return "";
  }
}

void gbtNfgStrategy::SetLabel(const gText &p_label)
{
  if (rep) {
    rep->m_label = p_label;
  }
}

gbtNfgPlayer gbtNfgStrategy::GetPlayer(void) const
{
  if (rep) {
    return rep->m_player;
  }
  else {
    return 0;
  }
}

const gArray<int> *const gbtNfgStrategy::GetBehavior(void) const
{ 
  if (rep) {
    return rep->m_behav;
  }
  else {
    return 0;
  }
}

long gbtNfgStrategy::GetIndex(void) const
{
  return (rep) ? rep->m_index : 0L;
}

gOutput &operator<<(gOutput &p_stream, const gbtNfgStrategy &)
{ 
  return p_stream;
}

//----------------------------------------------------------------------
//                 gbt_nfg_player_rep: Declaration
//----------------------------------------------------------------------

gbt_nfg_player_rep::gbt_nfg_player_rep(gbt_nfg_game_rep *p_nfg,
				       int p_id, int p_strats)
  : m_id(p_id), m_nfg(p_nfg), m_deleted(false), m_strategies(p_strats),
    m_refCount(0)
{
  for (int i = 1; i <= p_strats; i++) {
    m_strategies[i] = new gbt_nfg_strategy_rep(this);
  }
}

gbtNfgPlayer::gbtNfgPlayer(void)
  : rep(0)
{ }

gbtNfgPlayer::gbtNfgPlayer(gbt_nfg_player_rep *p_rep)
  : rep(p_rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtNfgPlayer::gbtNfgPlayer(const gbtNfgPlayer &p_outcome)
  : rep(p_outcome.rep)
{
  if (rep) {
    rep->m_refCount++;
  }
}

gbtNfgPlayer::~gbtNfgPlayer()
{
  if (rep) {
    if (--rep->m_refCount == 0) {
      // delete rep;
    }
  }
}

gbtNfgPlayer &gbtNfgPlayer::operator=(const gbtNfgPlayer &p_outcome)
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

bool gbtNfgPlayer::operator==(const gbtNfgPlayer &p_outcome) const
{
  return (rep == p_outcome.rep);
} 

bool gbtNfgPlayer::operator!=(const gbtNfgPlayer &p_outcome) const
{
  return (rep != p_outcome.rep);
} 

int gbtNfgPlayer::GetId(void) const
{
  return (rep) ? rep->m_id : 0;
}

bool gbtNfgPlayer::IsNull(void) const
{
  return (rep == 0);
}

gbtNfgGame gbtNfgPlayer::GetGame(void) const
{
  return (rep) ? rep->m_nfg : 0;
}

gText gbtNfgPlayer::GetLabel(void) const
{
  if (rep) {
    return rep->m_label;
  }
  else {
    return "";
  }
}

void gbtNfgPlayer::SetLabel(const gText &p_label)
{
  if (rep) {
    rep->m_label = p_label;
  }
}

int gbtNfgPlayer::NumStrategies(void) const
{
  return (rep) ? rep->m_strategies.Length() : 0;
}

gbtNfgStrategy gbtNfgPlayer::GetStrategy(int st) const
{
  return (rep) ? rep->m_strategies[st] : 0;
}

gbt_nfg_game_rep::gbt_nfg_game_rep(gbt_efg_game_rep *p_efg)
  : m_refCount(1), m_revision(0), m_outcomeRevision(-1),
    m_efg(p_efg)
{ }

static int Product(const gArray<int> &p_dim)
{
  int accum = 1;
  for (int i = 1; i <= p_dim.Length(); accum *= p_dim[i++]);
  return accum;
}

gbt_nfg_game_rep::gbt_nfg_game_rep(const gArray<int> &p_dim)
  : m_refCount(1),
    m_revision(0), m_outcomeRevision(-1), 
    m_title("UNTITLED"), m_dimensions(p_dim), m_players(p_dim.Length()),
    m_results(Product(p_dim)), m_efg(0)
{ }

gbt_nfg_game_rep::~gbt_nfg_game_rep()
{
  for (int pl = 1; pl <= m_players.Length(); delete m_players[pl++]);
  for (int outc = 1; outc <= m_outcomes.Length(); delete m_outcomes[outc++]);
}

//----------------------------------------------------
// Nfg: Constructors, Destructors, Operators
//----------------------------------------------------

gbtNfgGame::gbtNfgGame(const gArray<int> &dim)
  : rep(new gbt_nfg_game_rep(dim))
{
  for (int pl = 1; pl <= rep->m_players.Length(); pl++)  {
    rep->m_players[pl] = new gbt_nfg_player_rep(rep, pl, dim[pl]);
    rep->m_players[pl]->m_label = ToText(pl);
    for (int st = 1; st <= dim[pl]; st++) {
      rep->m_players[pl]->m_strategies[st]->m_label = ToText(st);
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
    delete rep;
  }
}

gbtNfgGame &gbtNfgGame::operator=(const gbtNfgGame &p_nfg)
{
  if (this != &p_nfg) {
    if (rep && --rep->m_refCount == 0) {
      delete rep;
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

long gbtNfgGame::RevisionNumber(void) const
{ return rep->m_revision; }

void gbtNfgGame::WriteNfgFile(gOutput &p_file, int p_nDecimals) const
{ 
  int oldDecimals = p_file.GetPrec();
  p_file.SetPrec(p_nDecimals);

  try {
    p_file << "NFG 1 R";
    p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";

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

    p_file << '\n';
    p_file.SetPrec(oldDecimals);
  }
  catch (...) {
    p_file.SetPrec(oldDecimals);
    throw;
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

void gbtNfgGame::DeleteOutcome(gbtNfgOutcome p_outcome)
{
  rep->m_revision++;

  if (p_outcome.rep) {
    for (int i = 1; i <= rep->m_results.Length(); i++) {
      if (rep->m_results[i] == p_outcome.rep)
	rep->m_results[i] = 0;
    }

    delete rep->m_outcomes.Remove(p_outcome.rep->m_id);

    for (int outc = 1; outc <= rep->m_outcomes.Length(); outc++) {
      rep->m_outcomes[outc]->m_id = outc;
    }
  }
}

void gbtNfgGame::SetTitle(const gText &s) 
{
  rep->m_title = s; 
  rep->m_revision++;
}

const gText &gbtNfgGame::GetTitle(void) const 
{ return rep->m_title; }

void gbtNfgGame::SetComment(const gText &s)
{
  rep->m_comment = s; 
  rep->m_revision++;
}

const gText &gbtNfgGame::GetComment(void) const
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
	  rep->m_players[pl]->m_strategies.Length() : 0);
}

const gArray<int> &gbtNfgGame::NumStrats(void) const
{
  return rep->m_dimensions;
}

int gbtNfgGame::ProfileLength(void) const
{
  int nprof = 0;
  for (int i = 1; i <= rep->m_players.Length(); i++)
    nprof += rep->m_players[i]->m_strategies.Length();
  return nprof;
}

int gbtNfgGame::NumOutcomes(void) const
{
  return rep->m_outcomes.Length(); 
}

gbtNfgOutcome gbtNfgGame::GetOutcomeId(int p_id) const
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

// ---------------------------------------
// gbtNfgGame: Private member functions
// ---------------------------------------

void gbtNfgGame::IndexStrategies(void)
{
  long offset = 1L;

  for (int i = 1; i <= NumPlayers(); i++)  {
    int j;
    for (j = 1; j <= NumStrats(i); j++)  {
      gbt_nfg_strategy_rep *s = rep->m_players[i]->m_strategies[j];
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
