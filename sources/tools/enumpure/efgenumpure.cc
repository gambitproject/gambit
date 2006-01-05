//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute pure-strategy equilibria in extensive form games
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

#include <unistd.h>
#include <iostream>
#include "libgambit/libgambit.h"

class EfgIter    {
  private:
    Gambit::Game _efg;
    gbtEfgSupport _support;
    Gambit::PureBehavProfile _profile;
    gbtPVector<int> _current;
    mutable gbtVector<gbtRational> _payoff;

  public:
    EfgIter(Gambit::Game);
    EfgIter(const gbtEfgSupport &);
    EfgIter(const EfgIter &);
    EfgIter(const EfgContIter &);
    ~EfgIter();
  
    EfgIter &operator=(const EfgIter &);
  
    void First(void);
    int Next(int p, int iset);
    int Set(int p, int iset, int act);
  
    gbtRational Payoff(int p) const;
    void Payoff(gbtVector<gbtRational> &) const;

    const gbtEfgSupport &Support(void) const;
};



EfgIter::EfgIter(Gambit::Game efg)
  : _efg(efg), _support(efg),
    _profile(efg), _current(_efg->NumInfosets()),
    _payoff(_efg->NumPlayers())
{
  First();
}

EfgIter::EfgIter(const gbtEfgSupport &s)
  : _efg(s.GetGame()), _support(s),
    _profile(s.GetGame()), _current(_efg->NumInfosets()),
    _payoff(_efg->NumPlayers())
{
  First();
}

EfgIter::EfgIter(const EfgIter &it)
  : _efg(it._efg), _support(it._support),
    _profile(it._profile), _current(it._current),
    _payoff(_efg->NumPlayers())
{ }

EfgIter::EfgIter(const EfgContIter &it)
  : _efg(it._efg), _support(it._support),
    _profile(it._profile), _current(it._current),
    _payoff(_efg->NumPlayers())
{ }
  

EfgIter::~EfgIter()
{ }


EfgIter &EfgIter::operator=(const EfgIter &it)
{
  if (this != &it && _efg == it._efg)  {
    _profile = it._profile;
    _current = it._current;
  }
  return *this;
}


void EfgIter::First(void)
{
  _current = 1;

  for (int pl = 1; pl <= _efg->NumPlayers(); pl++)  {
    for (int iset = 1; iset <= _efg->GetPlayer(pl)->NumInfosets(); iset++)
      _profile.SetAction(_support.Actions(pl, iset)[1]);
  }
}

int EfgIter::Next(int pl, int iset)
{
  const gbtArray<Gambit::GameAction> &actions = _support.Actions(pl, iset);
  
  if (_current(pl, iset) == actions.Length())   {
    _current(pl, iset) = 1;
    _profile.SetAction(actions[1]);
    return 0;
  }

  _current(pl, iset)++;
  _profile.SetAction(actions[_current(pl, iset)]);
  return 1;
}

int EfgIter::Set(int pl, int iset, int act)
{
  if (pl <= 0 || pl > _efg->NumPlayers() ||
      iset <= 0 || iset > _efg->GetPlayer(pl)->NumInfosets() ||
      act <= 0 || act > _support.NumActions(pl, iset))
    return 0;

  _current(pl, iset) = act;
  _profile.SetAction(_support.Actions(pl, iset)[act]);
  return 1;
}

gbtRational EfgIter::Payoff(int pl) const
{
  return _profile.GetPayoff(pl);
}

void EfgIter::Payoff(gbtVector<gbtRational> &payoff) const
{
  for (int pl = 1; pl <= _efg->NumPlayers(); pl++) {
    payoff[pl] = _profile.GetPayoff(pl);
  }
}




template <class T>
void PrintProfile(std::ostream &p_stream,
		  const Gambit::MixedBehavProfile<T> &p_profile)
{
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream << p_profile[i];
    if (i < p_profile.Length()) {
      p_stream << ',';
    }
  }

  p_stream << std::endl;
}

void Solve(const gbtEfgSupport &p_support)
{
  EfgContIter citer(p_support);
  gbtPVector<gbtRational> probs(p_support.GetGame()->NumInfosets());

  int ncont = 1;
  for (int pl = 1; pl <= p_support.GetGame()->NumPlayers(); pl++) {
    Gambit::GamePlayer player = p_support.GetGame()->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++)
      ncont *= p_support.NumActions(pl, iset);
  }

  int contNumber = 1;
  try {
    do {
      bool flag = true;
      // Commenting this out means we don't take advantage of reachability
      // information; fix this in future.
      //citer.GetProfile().InfosetProbs(probs);
      
      EfgIter eiter(citer);
      
      for (int pl = 1; flag && pl <= p_support.GetGame()->NumPlayers(); pl++)  {
	gbtRational current = citer.Payoff(pl);
	for (int iset = 1;
	     flag && iset <= p_support.GetGame()->GetPlayer(pl)->NumInfosets();
	     iset++)  {
	  //if (probs(pl, iset) == gbtRational(0))   continue;
	  for (int act = 1; act <= p_support.NumActions(pl, iset); act++)  {
	    eiter.Next(pl, iset);
	    if (eiter.Payoff(pl) > current)  {
	      flag = false;
	      break;
	    }
	  }
	}
      }
      
      if (flag)  {
	Gambit::MixedBehavProfile<gbtRational> temp(gbtEfgSupport(p_support.GetGame()));
	// zero out all the entries, since any equilibria are pure
	((gbtVector<gbtRational> &) temp).operator=(gbtRational(0));
	const Gambit::PureBehavProfile &profile = citer.GetProfile();
	for (int pl = 1; pl <= p_support.GetGame()->NumPlayers(); pl++)  {
	  for (int iset = 1;
	       iset <= p_support.GetGame()->GetPlayer(pl)->NumInfosets();
	       iset++)
	    temp(pl, iset,
		 profile.GetAction(p_support.GetGame()->GetPlayer(pl)->
				   GetInfoset(iset))->GetNumber()) = 1;
	}

	PrintProfile(std::cout, temp);
      }
      contNumber++;
    }  while (citer.NextContingency());
  }
  catch (...) {
    // catch exception; return solutions computed (if any)
  }
}

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Search for Nash equilibria in pure strategies\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 2005, The Gambit Project\n\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS]\n";
  std::cerr << "Accepts extensive game on standard input.\n";
  std::cerr << "With no options, locates all Nash equilibria in pure strategies.\n\n";

  std::cerr << "Options:\n";
  std::cerr << "  -h               print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  exit(1);
}


int main(int argc, char *argv[])
{
  opterr = 0;
  bool quiet = false;

  int c;
  while ((c = getopt(argc, argv, "hq")) != -1) {
    switch (c) {
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'q':
      quiet = true;
      break;
    case '?':
      if (isprint(optopt)) {
	std::cerr << argv[0] << ": Unknown option `-" << ((char) optopt) << "'.\n";
      }
      else {
	std::cerr << argv[0] << ": Unknown option character `\\x" << optopt << "`.\n";
      }
      return 1;
    default:
      abort();
    }
  }

  if (!quiet) {
    PrintBanner(std::cerr);
  }

  Gambit::Game efg;

  try {
    efg = Gambit::ReadEfg(std::cin);
  }
  catch (...) {
    return 1;
  }

  Solve(efg);

  return 0;
}





