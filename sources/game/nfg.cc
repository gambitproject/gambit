//
// FILE: nfg.cc -- Implementation of normal form member functions
//              -- and Implementation of NFPlayer member functions
//
// $Id$
//

#include "math/rational.h"
#include "nfg.h"
#include "nfstrat.h"
#include "nfplayer.h"

//--------------------------------------
// Strategy:  Constructors, Destructors
//--------------------------------------

Strategy::Strategy(NFPlayer *p) : m_number(0), m_player(p), m_index(0L)
{ }

Strategy::Strategy(const Strategy &s) : m_player(s.m_player), m_name(s.m_name)
{ }

Strategy &Strategy::operator=(const Strategy &s)
{
  m_player = s.m_player;
  m_name = s.m_name;
  return *this;
}

Strategy::~Strategy()
{ }


//----------------------------------------------------
// Nfg: Constructors, Destructors, Operators
//----------------------------------------------------


int Nfg::Product(const gArray<int> &dim)
{
  int accum = 1;
  for (int i = 1; i <= dim.Length(); accum *= dim[i++]);
  return accum;
}
  
Nfg::Nfg(const gArray<int> &dim)
  : m_dirty(false), m_revision(0),  m_outcome_revision(-1), 
    title("UNTITLED"), dimensions(dim), players(dim.Length()),
    results(Product(dim)), efg(0)
{
  for (int pl = 1; pl <= players.Length(); pl++)  {
    players[pl] = new NFPlayer(pl, this, dim[pl]);
	  players[pl]->name = ToText(pl);
    for (int st = 1; st <= players[pl]->NumStrats(); st++)
      players[pl]->strategies[st]->m_name = ToText(st);
  }
  IndexStrategies();

  for (int cont = 1; cont <= results.Length();
       results[cont++] = (NFOutcome *) 0);
}

Nfg::Nfg(const Nfg &b)
  : m_dirty(false), m_revision(0),  m_outcome_revision(-1), 
    title(b.title), comment(b.comment), 
    dimensions(b.dimensions),
    players(b.players.Length()), outcomes(b.outcomes.Length()),
    results(b.results.Length()), efg(0)
{
  for (int pl = 1; pl <= players.Length(); pl++)  {
    players[pl] = new NFPlayer(pl, this, dimensions[pl]);
    players[pl]->name = b.players[pl]->name;
    for (int st = 1; st <= players[pl]->NumStrats(); st++)  {
      *(players[pl]->strategies[st]) = *(b.players[pl]->strategies[st]);
      players[pl]->strategies[st]->m_player = players[pl];
    }
  }
  IndexStrategies();
  
  for (int outc = 1; outc <= outcomes.Length(); outc++)  {
    outcomes[outc] = new NFOutcome(outc, this);
    outcomes[outc]->SetName(b.outcomes[outc]->GetName());
    outcomes[outc]->payoffs = b.outcomes[outc]->payoffs;
  }

  for (int cont = 1; cont <= results.Length(); cont++)    
    results[cont] = (b.results[cont]) ?
                     outcomes[b.results[cont]->GetNumber()] : (NFOutcome *) 0;
}

#include "efg.h"
#include "lexicon.h"

Nfg::~Nfg()
{
  for (int pl = 1; pl <= players.Length(); pl++)
    delete players[pl];
  for (int outc = 1; outc <= outcomes.Length(); outc++)
    delete outcomes[outc];

  if (efg)  {
    const efgGame *tmp = (efgGame *) efg;
    // note that Lexicon dtor unsets the efg member...

    delete ((efgGame *) efg)->lexicon;
    tmp->lexicon = 0;
  }
  efg = 0;
}

void Nfg::BreakLink(void)
{
  if (efg)  {
    const efgGame *tmp = (efgGame *) efg;
    // note that Lexicon dtor unsets the efg member...

    delete ((efgGame *) efg)->lexicon;
    tmp->lexicon = 0;
  }
  efg = 0;
}

//-------------------------------
// Nfg: Member Functions
//-------------------------------

#include "nfgiter.h"

void Nfg::WriteNfgFile(gOutput &p_file, int p_nDecimals) const
{ 
  int oldDecimals = p_file.GetPrec();
  p_file.SetPrec(p_nDecimals);

  try {
    p_file << "NFG 1 R";
    p_file << " \"" << EscapeQuotes(GetTitle()) << "\" { ";

    for (int i = 1; i <= NumPlayers(); i++)
      p_file << '"' << EscapeQuotes(Players()[i]->GetName()) << "\" ";

    p_file << "}\n\n{ ";
  
    for (int i = 1; i <= NumPlayers(); i++)   {
      NFPlayer *player = Players()[i];
      p_file << "{ ";
      for (int j = 1; j <= player->NumStrats(); j++)
	p_file << '"' << EscapeQuotes(player->Strategies()[j]->Name()) << "\" ";
      p_file << "}\n";
    }
  
    p_file << "}\n";

    p_file << "\"" << EscapeQuotes(comment) << "\"\n\n";

    int ncont = 1;
    for (int i = 1; i <= NumPlayers(); i++)
      ncont *= NumStrats(i);

    p_file << "{\n";
    for (int outc = 1; outc <= outcomes.Length(); outc++)   {
      p_file << "{ \"" << EscapeQuotes(outcomes[outc]->name) << "\" ";
      for (int pl = 1; pl <= players.Length(); pl++)  {
	p_file << outcomes[outc]->payoffs[pl];
	if (pl < players.Length())
	  p_file << ", ";
	else
	  p_file << " }\n";
      }
    }
    p_file << "}\n";
  
    for (int cont = 1; cont <= ncont; cont++)  {
      if (results[cont] != 0)
	p_file << results[cont]->number << ' ';
      else
	p_file << "0 ";
    }

    p_file << '\n';
    p_file.SetPrec(oldDecimals);
    m_dirty = false;
  }
  catch (...) {
    p_file.SetPrec(oldDecimals);
    throw;
  }
}

NFOutcome *Nfg::NewOutcome(void)
{
  m_dirty = true;
  m_revision++;
  NFOutcome *outcome = new NFOutcome(outcomes.Length() + 1, this);
  outcomes.Append(outcome);
  return outcome;
}

void Nfg::DeleteOutcome(NFOutcome *outcome)
{
  m_dirty = true;
  m_revision++;

  for (int i = 1; i <= results.Length(); i++) {
    if (results[i] == outcome)
      results[i] = 0;
  }

  delete outcomes.Remove(outcome->GetNumber());

  for (int outc = 1; outc <= outcomes.Length(); outc++)
    outcomes[outc]->number = outc;
}

const gArray<Strategy *> &Nfg::Strategies(int p) const
{
  return (players[p]->Strategies());
}

void Nfg::SetTitle(const gText &s) 
{
  title = s; 
  m_dirty = true;
  m_revision++;
}

const gText &Nfg::GetTitle(void) const 
{ return title; }

void Nfg::SetComment(const gText &s)
{
  comment = s; 
  m_dirty = true;
  m_revision++;
}

const gText &Nfg::GetComment(void) const
{ return comment; }


int Nfg::NumPlayers(void) const 
{ 
  return (players.Length()); 
}

const gArray<NFPlayer *> &Nfg::Players(void) const
{
  return players;
}

int Nfg::NumStrats(int pl) const
{
  return ((pl > 0 && pl <= NumPlayers()) ? 
	  (players[pl])->strategies.Length() : 0);
}

int Nfg::ProfileLength(void) const
{
  int nprof = 0;
  for (int i = 1; i <= players.Length(); i++)
    nprof += players[i]->strategies.Length();
  return nprof;
}

void Nfg::SetOutcome(const gArray<int> &profile, NFOutcome *outcome)
{
  int index = 1;
  for (int i = 1; i <= profile.Length(); i++)
    index += players[i]->strategies[profile[i]]->m_index;
  results[index] = outcome;
  m_dirty = true;
  m_revision++;
  BreakLink();
}


void Nfg::SetOutcome(const StrategyProfile &p, NFOutcome *outcome)
{
  results[p.index + 1] = outcome;
  m_dirty = true;
  m_revision++;
  BreakLink();
}

NFOutcome *Nfg::GetOutcome(const gArray<int> &profile) const 
{
  int index = 1;
  for (int i = 1; i <= profile.Length(); i++)
	 index += players[i]->strategies[profile[i]]->m_index;
  return results[index];
}

NFOutcome *Nfg::GetOutcome(const StrategyProfile &p) const
{
  return results[p.index + 1];
}

void Nfg::SetPayoff(NFOutcome *outcome, int pl, const gNumber &value)
{
  if (outcome) {
    outcome->payoffs[pl] = value;
    m_dirty = true;
    m_revision++;
  }
}

gNumber Nfg::Payoff(NFOutcome *outcome, int pl) const
{
  if (outcome)
    return outcome->payoffs[pl];
  else
    return 0;
}

// ---------------------------------------
// Nfg: Private member functions
// ---------------------------------------

void Nfg::IndexStrategies(void)
{
  long offset = 1L;

  for (int i = 1; i <= NumPlayers(); i++)  {
    int j;
    for (j = 1; j <= NumStrats(i); j++)  {
      Strategy *s = (players[i])->strategies[j];
      s->m_number = j;
      s->m_index = (j - 1) * offset;
    }
    offset *= (j - 1);
  }
}

// --------------------------
// NFPlayer: Member functions 
// --------------------------

NFPlayer::NFPlayer(int n, Nfg *N, int num)
: number(n), N(N), strategies(num)
{ 
  for (int j = 1; j <= num; j++)
    strategies[j] = new Strategy(this);
}

NFPlayer::~NFPlayer()
{ 
  for (int j = 1; j <= strategies.Length(); j++)
    delete strategies[j];
}

Nfg &NFPlayer::Game(void) const
{
  return *N;
}

const gText &NFPlayer::GetName(void) const
{
  return name;
}

void NFPlayer::SetName(const gText &s)
{
  name = s;
}

int NFPlayer::NumStrats(void) const 
{
  return strategies.Length();
}


const gArray<Strategy *> &NFPlayer::Strategies(void) const
{
  return strategies;
}

void Nfg::InitPayoffs(void) const 
{
  if(m_outcome_revision == RevisionNumber()) return;

  for (int outc = 1; outc <= NumOutcomes(); outc++)
    for (int pl = 1; pl <= NumPlayers(); pl++)
      outcomes[outc]->double_payoffs[pl] = outcomes[outc]->payoffs[pl];

  m_outcome_revision = RevisionNumber();
}

