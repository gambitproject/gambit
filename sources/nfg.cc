//
// FILE: nfg.cc -- Implementation of normal form member functions
//              -- and Implementation of NFPlayer member functions
//
// $Id$
//

#include "rational.h"
#include <assert.h>
#include "nfg.h"
#include "nfstrat.h"
#include "nfplayer.h"


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
  : dimensions(dim), players(dim.Length()), results(Product(dim)),
    parameters(new gSpace)
{
  ORD_PTR ord = &lex;
  paramorder = new term_order(parameters, ord);
  for (int pl = 1; pl <= players.Length(); pl++)  {
    players[pl] = new NFPlayer(pl, this, dim[pl]);
	  players[pl]->name = ToString(pl);
    for (int st = 1; st <= players[pl]->NumStrats(); st++)
      players[pl]->strategies[st]->name = ToString(st);
  }
  IndexStrategies();

  for (int cont = 1; cont <= results.Length();
       results[cont++] = (NFOutcome *) 0);
}

Nfg::Nfg(const Nfg &b)
  : title(b.title), dimensions(b.dimensions),
    players(b.players.Length()), outcomes(b.outcomes.Length()),
    results(b.results.Length()),
    parameters(b.parameters), paramorder(b.paramorder)
{
  for (int pl = 1; pl <= players.Length(); pl++)  {
    players[pl] = new NFPlayer(pl, this, dimensions[pl]);
    players[pl]->name = b.players[pl]->name;
    for (int st = 1; st <= players[pl]->NumStrats(); st++)  {
      *(players[pl]->strategies[st]) = *(b.players[pl]->strategies[st]);
      players[pl]->strategies[st]->nfp = players[pl];
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

#ifndef NFG_ONLY
#include "efg.h"
#include "lexicon.h"
#endif   // NFG_ONLY

Nfg::~Nfg()
{
  for (int pl = 1; pl <= players.Length(); pl++)
    delete players[pl];
  for (int outc = 1; outc <= outcomes.Length(); outc++)
    delete outcomes[outc];

#ifndef NFG_ONLY
  if (efg)  {
    const Efg *tmp = efg;
    // note that Lexicon dtor unsets the efg member...

    delete efg->lexicon;
    tmp->lexicon = 0;
  }
  efg = 0;
#endif   // NFG_ONLY

}

void Nfg::BreakLink(void)
{
#ifndef NFG_ONLY
  if (efg)  {
    const Efg *tmp = efg;
    // note that Lexicon dtor unsets the efg member...

    delete efg->lexicon;
    tmp->lexicon = 0;
  }
  efg = 0;
#endif   // NFG_ONLY
}

//-------------------------------
// Nfg: Member Functions
//-------------------------------

#include "nfgiter.h"

void Nfg::WriteNfgFile(gOutput &f) const
{
  int i;


  f << "NFG 1 R";
  f << " \"" << EscapeQuotes(GetTitle()) << "\" { ";

  for (i = 1; i <= NumPlayers(); i++)
    f << '"' << EscapeQuotes(Players()[i]->GetName()) << "\" ";

  f << "}\n\n{ ";
  
  for (i = 1; i <= NumPlayers(); i++)   {
    NFPlayer *player = Players()[i];
    f << "{ ";
    for (int j = 1; j <= player->NumStrats(); j++)
      f << '"' << EscapeQuotes(player->Strategies()[j]->name) << "\" ";
    f << "}\n";
  }
  
  f << "}\n\n";


  int ncont = 1;
  for (i = 1; i <= NumPlayers(); i++)
    ncont *= NumStrats(i);

  for (i = 1; i <= ncont; i++)
    for (int j = 1; j <= NumPlayers(); j++)
      if (GetOutcome(i))
	f << Payoff(GetOutcome(i), j) << ' ';
      else
	f << 0 << ' ';

  f << '\n';
}

NFOutcome *Nfg::NewOutcome(void)
{
  NFOutcome *outcome = new NFOutcome(outcomes.Length() + 1, this);
  outcomes.Append(outcome);
  return outcome;
}

void Nfg::DeleteOutcome(NFOutcome *outcome)
{
  delete outcomes.Remove(outcome->GetNumber());

  for (int outc = 1; outc <= outcomes.Length(); outc++)
    outcomes[outc]->number = outc;
}

const gArray<Strategy *> &Nfg::Strategies(int p) const
{
  return (players[p]->Strategies());
}

void Nfg::SetTitle(const gString &s) 
{ 
  title = s; 
}

const gString &Nfg::GetTitle(void) const 
{ 
  return title; 
}

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
    index += players[i]->strategies[profile[i]]->index;
  results[index] = outcome;
  BreakLink();
}


void Nfg::SetOutcome(const StrategyProfile &p, NFOutcome *outcome)
{
  results[p.index + 1] = outcome;
  BreakLink();
}

NFOutcome *Nfg::GetOutcome(const gArray<int> &profile) const 
{
  int index = 1;
  for (int i = 1; i <= profile.Length(); i++)
	 index += players[i]->strategies[profile[i]]->index;
  return results[index];
}

NFOutcome *Nfg::GetOutcome(const StrategyProfile &p) const
{
  return results[p.index + 1];
}

void Nfg::SetPayoff(NFOutcome *outcome,
                    int pl, const gPoly<gNumber> &value)
{
  if (outcome)   outcome->payoffs[pl] = value;
}

gPoly<gNumber> Nfg::Payoff(NFOutcome *outcome, int pl) const
{
  if (outcome)
	  return outcome->payoffs[pl];
  else
	  return gPoly<gNumber>(Parameters(), gNumber(0), ParamOrder());
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
      s->number = j;
      s->index = (j - 1) * offset;
    }
	 assert(j - 1 == NumStrats(i));
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

const gString &NFPlayer::GetName(void) const
{
  return name;
}

void NFPlayer::SetName(const gString &s)
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



// Not really how these should be handled; but it works for now, and
// we can deal with housekeeping later

#include "nfgiter.imp"
#include "nfgciter.imp"

#include "garray.imp"
#include "gblock.imp"
#include "glist.imp"

template class gArray<NFStrategySet *>;
template class gArray<Strategy *>;
template class gArray<NFOutcome *>;

template class gArray<NFPlayer *>;
template class gBlock<Strategy *>;

template class gArray<NFOutcome *>;
template class gBlock<NFOutcome *>;
