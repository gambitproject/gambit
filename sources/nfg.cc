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
// NFGameForm: Constructors, Destructors, Operators
//----------------------------------------------------


int NFGameForm::Product(const gArray<int> &dim)
{
  int accum = 1;
  for (int i = 1; i <= dim.Length(); accum *= dim[i++]);
  return accum;
}
  
NFGameForm::NFGameForm(const NFPayoffs &pay, const gArray<int> &dim)
  : dimensions(dim), players(dim.Length()), results(Product(dim)),
    paytable((NFPayoffs *) &pay)
{
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

NFGameForm::NFGameForm(const NFGameForm &b)
  : title(b.title), dimensions(b.dimensions),
    players(b.players.Length()), outcomes(b.outcomes.Length()),
    results(b.results.Length()), paytable(b.paytable)
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
  }

  for (int cont = 1; cont <= results.Length(); cont++)    
    results[cont] = (b.results[cont]) ?
                     outcomes[b.results[cont]->GetNumber()] : (NFOutcome *) 0;
}


NFGameForm::~NFGameForm()
{
  for (int pl = 1; pl <= players.Length(); pl++)
    delete players[pl];
  for (int outc = 1; outc <= outcomes.Length(); outc++)
    delete outcomes[outc];
}


//-------------------------------
// NFGameForm: Member Functions
//-------------------------------


NFOutcome *NFGameForm::NewOutcome(void)
{
  NFOutcome *outcome = new NFOutcome(outcomes.Length() + 1, this);
  outcomes.Append(outcome);

  paytable->NewOutcome();

  return outcome;
}

void NFGameForm::DeleteOutcome(NFOutcome *outcome)
{
  paytable->DeleteOutcome(outcome->GetNumber());
  delete outcomes.Remove(outcome->GetNumber());
  for (int outc = 1; outc <= outcomes.Length(); outc++)
    outcomes[outc]->number = outc;
}

const gArray<Strategy *> &NFGameForm::Strategies(int p) const
{
  return (players[p]->Strategies());
}

void NFGameForm::SetTitle(const gString &s) 
{ 
  title = s; 
}

const gString &NFGameForm::GetTitle(void) const 
{ 
  return title; 
}

int NFGameForm::NumPlayers(void) const 
{ 
  return (players.Length()); 
}

const gArray<NFPlayer *> &NFGameForm::Players(void) const
{
  return players;
}

int NFGameForm::NumStrats(int pl) const
{
  return ((pl > 0 && pl <= NumPlayers()) ? 
	  (players[pl])->strategies.Length() : 0);
}

int NFGameForm::ProfileLength(void) const
{
  int nprof = 0;
  for (int i = 1; i <= players.Length(); i++)
    nprof += players[i]->strategies.Length();
  return nprof;
}

void NFGameForm::SetOutcome(const gArray<int> &profile, NFOutcome *outcome)
{
  int index = 1;
  for (int i = 1; i <= profile.Length(); i++)
    index += players[i]->strategies[profile[i]]->index;
  results[index] = outcome;
  paytable->BreakLink();
}


void NFGameForm::SetOutcome(const StrategyProfile &p, NFOutcome *outcome)
{
  results[p.index + 1] = outcome;
  paytable->BreakLink();
}

NFOutcome *NFGameForm::GetOutcome(const gArray<int> &profile) const 
{
  int index = 1;
  for (int i = 1; i <= profile.Length(); i++)
	 index += players[i]->strategies[profile[i]]->index;
  return results[index];
}

NFOutcome *NFGameForm::GetOutcome(const StrategyProfile &p) const
{
  return results[p.index + 1];
}


// ---------------------------------------
// NFGameForm: Private member functions
// ---------------------------------------

void NFGameForm::IndexStrategies(void)
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

NFPlayer::NFPlayer(int n, NFGameForm *N, int num)
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

NFGameForm &NFPlayer::BelongsTo(void) const
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
