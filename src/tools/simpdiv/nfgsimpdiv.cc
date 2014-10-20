//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/simpdiv/nfgsimpdiv.cc
// Compute Nash equilibria via simplicial subdivision on the normal form
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
#include <getopt.h>
#include <cstdlib>
#include <iostream>
#include <cerrno>
#include <iomanip>
#include <fstream>
#include "libgambit/libgambit.h"
#include "libgambit/nash.h"

using namespace Gambit;

//
// simpdiv is a simplicial subdivision algorithm with restart, for finding
// mixed strategy solutions to general finite n-person games.  It is based on
// van Der Laan, Talman and van Der Heyden, Math in Oper Res, 1987.
//
// The program accepts a normal form game file on standard input.
//
// The program accepts the following command-line options:
// -v:  Verbose output.  Lists the profile computed at each stage of the
//      mesh refinement.  Default is off, in which case the program only
//      prints the (approximate) Nash equilibrium of the last refinement.
//
// -s <filename>: A list of strategy profiles to use as starting points
//
// -f:  Print profiles in floating-point (even though the algorithm inherently
//      computes using rational numbers)
//
// -d #:  Number of decimals to show in floating-point output
//        (Only has an effect in conjunction with -f)
//
// -r #:  Generate random starting points with denominator #.
//
// -n #:  Stop after # equilibria (only effective with -r)
//
// -g #:  Multiplier for grid restart (default is 2)
//
// 
// Some history:
// 
// Original code was written by R. McKelvey.  The implementation was
// kludged to conform to the new standard algorithm interface (namely,
// by removing passing a reference to the game in the ctor); truly,
// the algorithm needs recoded from the ground up to remove gotos and
// simplify things. (TLT, 6/2002)
//

class NashSimpdivStrategySolver : public NashStrategySolver<Rational> {
public:
  NashSimpdivStrategySolver(int p_gridResize = 2, int p_leashLength = 0,
			    bool p_verbose = false,
			    shared_ptr<StrategyProfileRenderer<Rational> > p_onEquilibrium = 0)
    : NashStrategySolver<Rational>(p_onEquilibrium),
      m_gridResize(p_gridResize),
      m_leashLength((p_leashLength > 0) ? p_leashLength : 32000),
      m_verbose(p_verbose)
  { }
  virtual ~NashSimpdivStrategySolver() { }

  List<MixedStrategyProfile<Rational> > Solve(const MixedStrategyProfile<Rational> &p_start) const;
  List<MixedStrategyProfile<Rational> > Solve(const Game &p_game) const
  { return Solve(p_game->NewMixedStrategyProfile(Rational(0))); }


private:
  int m_gridResize, m_leashLength;
  bool m_verbose;

  class State {
  public:
    int t, ibar;
    Rational d, pay, maxz, bestz;
    
    State(void) : t(0), ibar(1), bestz(1.0e30) { }
    Rational getlabel(MixedStrategyProfile<Rational> &yy, Array<int> &, 
		      PVector<Rational> &);
  };

  Rational Simplex(MixedStrategyProfile<Rational> &, const Rational &d) const;
  void update(State &, RectArray<int> &, RectArray<int> &, PVector<Rational> &,
	      const PVector<int> &, int j, int i) const;
  void getY(State &, MixedStrategyProfile<Rational> &x, PVector<Rational> &, 
	    const PVector<int> &, const PVector<int> &, 
	    const PVector<Rational> &, const RectArray<int> &, int k) const;
  void getnexty(State &, 
		MixedStrategyProfile<Rational> &x, const RectArray<int> &,
		const PVector<int> &, int i) const;
  int get_c(int j, int h, int nstrats, const PVector<int> &) const;
  int get_b(int j, int h, int nstrats, const PVector<int> &) const;
  
};

//-------------------------------------------------------------------------
//          NashSimpdivStrategySolver: Private member functions
//-------------------------------------------------------------------------

inline GameStrategy GetStrategy(const Game &game, int pl, int st)
{
  return game->Players()[pl]->Strategies()[st];
}

Rational 
NashSimpdivStrategySolver::Simplex(MixedStrategyProfile<Rational> &y,
				   const Rational &d) const
{
  Game game = y.GetGame();
  State state;
  state.d = d;
  Array<int> nstrats(game->NumStrategies());
  Array<int> ylabel(2);
  RectArray<int> labels(y.MixedProfileLength(), 2), pi(y.MixedProfileLength(), 2);
  PVector<int> U(nstrats), TT(nstrats);
  PVector<Rational> ab(nstrats), besty(nstrats), v(nstrats);
  for (int i = 1; i <= v.Length(); i++) {
    v[i] = y[i];
  }
  besty = static_cast<Vector<Rational> &>(y);
  int i = 0;
  int j, k, h, jj, hh,ii, kk,tot;
  Rational maxz;

// Label step0 not currently used, hence commented
// step0:
  TT = 0;
  U = 0;
  ab = Rational(0);
  for (j = 1; j <= game->NumPlayers(); j++)  {
    GamePlayer player = game->Players()[j];
    for (h = 1; h <= nstrats[j]; h++)  {
      if (v(j,h) == Rational(0)) {
	U(j,h) = 1;
      }
      y[player->Strategies()[h]] = v(j,h);
    }
  }

 step1:
  maxz = state.getlabel(y, ylabel, besty);
  j = ylabel[1];
  h = ylabel[2];
  labels(state.ibar,1) = j;
  labels(state.ibar,2) = h;
  
// Label case1a not currently used, hence commented
// case1a:
  if (TT(j,h)==0 && U(j,h)==0)  {
    for (hh=1, tot=0; hh <= nstrats[j]; hh++) {
      if (TT(j,hh)==1 || U(j,hh)==1) {
	tot++;
      }
    }
    if (tot == nstrats[j] - 1) {
      goto end;
    }      
    else {
      i = state.t+1;
      goto step2;
    }
  }
  /* case1b */
  else if (TT(j,h))  {
    i = 1;
    while (labels(i,1) != j || labels(i,2) != h || i == state.ibar)  {
      i++;
    }
    goto step3;
  }
  /* case1c */
  else if (U(j,h)) {
    k = h;
    while (U(j,k)) {
      k++;
      if (k > nstrats[j]) {
	k = 1;
      }
    }
    if (TT(j,k) == 0) {
      i = state.t+1;
    }
    else {
      i = 1;
      while ((pi(i,1)!=j || pi(i,2)!=k) && i<=state.t) {
	i++;
      }
    }
    goto step2;
  }
  
 step2:
  getY(state, y, v, U, TT, ab, pi, i);
  pi.RotateDown(i, state.t+1);
  pi(i,1) = j;
  pi(i,2) = h;
  labels.RotateDown(i+1, state.t+2);
  state.ibar = i+1;
  state.t++;
  getnexty(state, y, pi, U, i);
  TT(j,h) = 1;
  U(j,h) = 0;
  goto step1;
  
 step3:
  ii = (i == state.t+1) ? state.t : 1;
  j = pi(ii,1);
  h = pi(ii,2);
  k = h;
  if (i < state.t+1) {
    k = get_b(j, h, nstrats[j], U);
  }
  kk = get_c(j, h, nstrats[j], U);
  if (i == 1) {
    ii = state.t+1;
  }
  else if (i == state.t+1) {
    ii = 1;
  }
  else {
    ii = i-1;
  }
  getY(state, y, v, U, TT, ab, pi, ii);
  
  /* case3a */
  if (i==1 && 
      (y[GetStrategy(game, j, k)]<=Rational(0) || 
       (v(j,k)-y[GetStrategy(game, j, k)]) >= Rational(m_leashLength)*state.d)) {
    for (hh = 1, tot = 0; hh <= nstrats[j]; hh++) {
      if (TT(j,hh)==1 || U(j,hh)==1)  {
	tot++;
      }
    }
    if (tot == nstrats[j] - 1) {
      U(j,k)=1;
      goto end;
    }
    else {
      update(state, pi, labels, ab, U, j, i);
      U(j,k) = 1;
      getnexty(state, y, pi, U, state.t);
      goto step1;
    }
  }
  /* case3b */
  else if (i>=2 && i<=state.t &&
	   (y[GetStrategy(game, j, k)] <= Rational(0) || 
	    (v(j,k)-y[GetStrategy(game, j, k)]) >= Rational(m_leashLength)*state.d)) {
    goto step4;
  }
  /* case3c */
  else if (i==state.t+1 && ab(j,kk) == Rational(0)) {
    if (y[GetStrategy(game, j, h)] <= Rational(0) || 
	(v(j,h)-y[GetStrategy(game, j, h)]) >= Rational(m_leashLength)*state.d) {
      goto step4;
    }
    else {
      k=0;
      while (ab(j,kk) == Rational(0) && k==0) {
	if(kk==h)k=1;
	kk++;
	if (kk > nstrats[j]) {
	  kk=1; 
	}
      }
      kk--;
      if (kk == 0) {
	kk = nstrats[j];
      }
      if (kk == h) {
	goto step4;
      }
      else {
	goto step5;
      }
    }
  }
  else {
    if (i==1) {
      getnexty(state, y, pi, U, 1);
    }
    else if (i<=state.t) {
      getnexty(state, y, pi, U, i);
    }
    else if (i==state.t+1) {
      j = pi(state.t,1);
      h = pi(state.t,2);
      hh = get_b(j,h,nstrats[j],U);
      y[GetStrategy(game, j, h)] -= state.d;
      y[GetStrategy(game, j, hh)] += state.d;
    }
    update(state, pi, labels, ab, U, j, i);
  }
  goto step1;

 step4:
  getY(state, y, v, U, TT, ab, pi, 1);
  j = pi(i-1,1);
  h = pi(i-1,2);
  TT(j,h) = 0;
  if (y[GetStrategy(game, j, h)] <= Rational(0) || 
      (v(j,h)-y[GetStrategy(game, j, h)]) >= Rational(m_leashLength)*state.d) {
    U(j,h) = 1;
  }
  labels.RotateUp(i,state.t+1);
  pi.RotateUp(i-1,state.t);
  state.t--;
  ii=1;
  while (labels(ii,1)!=j || labels(ii,2)!=h) {
    ii++;
  }
  i=ii;
  goto step3;

 step5:
  k=kk;
  labels.RotateDown(1,state.t+1);
  state.ibar=1;
  pi.RotateDown(1,state.t);
  U(j,k)=0;
  jj=pi(1,1);
  hh=pi(1,2);
  kk=get_b(jj,hh,nstrats[jj],U);
  y[GetStrategy(game, jj, hh)] -= state.d;
  y[GetStrategy(game, jj, kk)] += state.d;
  
  k = get_c(j,h,nstrats[j],U);
  kk=1;
  while(kk){
    if (k == h) {
      kk = 0;
    }
    ab(j,k) -= Rational(1);
    k++;
    if (k > nstrats[j]) {
      k = 1;
    }
  }
  goto step1;

 end:
  maxz=state.bestz;
  for (i = 1; i <= game->NumPlayers(); i++) {
    for (j = 1; j <= nstrats[i]; j++) {
      y[GetStrategy(game, i, j)] = besty(i,j);
    }
  }
  return maxz;
}

void NashSimpdivStrategySolver::update(State &state,
				       RectArray<int> &pi,
				       RectArray<int> &labels,
				       PVector<Rational> &ab,
				       const PVector<int> &U,
				       int j, int i) const
{
  int jj, hh, k,f;
  
  f=1;
  if(i>=2 && i<=state.t) {
    pi.SwitchRows(i,i-1);
    state.ibar=i;
  }
  else if(i==1) {
    labels.RotateUp(1,state.t+1);
    state.ibar=state.t+1;
    jj=pi(1,1);
    hh=pi(1,2);
    if(jj==j) {
      k=get_c(jj,hh,ab.Lengths()[jj],U);
      while(f) {
	if(k==hh)f=0;
	ab(j,k) += Rational(1);
	k++;
	if(k>ab.Lengths()[jj])k=1;
      }
      pi.RotateUp(1,state.t);
    }
  }
  else if(i==state.t+1) {
    labels.RotateDown(1,state.t+1);
    state.ibar=1;
    jj=pi(state.t,1);
    hh=pi(state.t,2);
    if(jj==j) {
      k=get_c(jj,hh,ab.Lengths()[jj],U);
      while(f) {
	if(k==hh)f=0;
	ab(j,k) -= Rational(1);
	k++;
	if(k>ab.Lengths()[jj])k=1;
      }
      pi.RotateDown(1,state.t);
    }
  }
}

void NashSimpdivStrategySolver::getY(State &state,
				     MixedStrategyProfile<Rational> &x,
				     PVector<Rational> &v, 
				     const PVector<int> &U,
				     const PVector<int> &TT,
				     const PVector<Rational> &ab,
				     const RectArray<int> &pi,
				     int k) const
{
  static_cast<Vector<Rational> & >(x) = v;
  for (int j = 1; j <= x.GetGame()->Players().size(); j++) {
    GamePlayer player = x.GetGame()->Players()[j];
    for (int h = 1; h <= player->Strategies().size(); h++) {
      if (TT(j,h) == 1 || U(j,h) == 1) {
	x[player->Strategies()[h]] += state.d*ab(j,h);
	int hh = (h > 1) ? h-1 : player->Strategies().size();
	x[player->Strategies()[hh]] -= state.d*ab(j,h);
      }
    }
  }
  for (int i = 2; i <= k; i++) {
    getnexty(state, x, pi, U, i-1);
  }
}

void NashSimpdivStrategySolver::getnexty(State &state,
					 MixedStrategyProfile<Rational> &x,
					 const RectArray<int> &pi, 
					 const PVector<int> &U,
					 int i) const
{
  int j = pi(i,1);
  GamePlayer player = x.GetGame()->Players()[j];
  int h = pi(i,2);
  x[player->Strategies()[h]] += state.d;
  int hh = get_b(j, h, player->Strategies().size(), U);
  x[player->Strategies()[hh]] -= state.d;
}

int NashSimpdivStrategySolver::get_b(int j, int h, int nstrats, const PVector<int> &U) const
{
  int hh = (h > 1) ? h-1 : nstrats;
  while (U(j,hh)) {
    hh--;
    if (hh == 0) { 
      hh = nstrats;
    }
  }
  return hh;
}

int NashSimpdivStrategySolver::get_c(int j, int h, int nstrats,
				     const PVector<int> &U) const
{
  int hh = get_b(j, h, nstrats, U) + 1;
  return (hh > nstrats) ? 1 : hh;
}

Rational 
NashSimpdivStrategySolver::State::getlabel(MixedStrategyProfile<Rational> &yy,
					   Array<int> &ylabel,
					   PVector<Rational> &besty)
{
  Rational maxz = -1000000;
  ylabel[1] = 1;
  ylabel[2] = 1;
  
  for (int i = 1; i <= yy.GetGame()->NumPlayers(); i++) {
    GamePlayer player = yy.GetGame()->Players()[i];
    Rational payoff = 0;
    Rational maxval = -1000000;
    int jj = 0;
    for (int j = 1; j <= player->Strategies().size(); j++) {
      pay = yy.GetPayoff(player->Strategies()[j]);
      payoff += yy[player->Strategies()[j]] * pay;
      if (pay > maxval) {
	maxval = pay;
	jj = j;
      }
    }
    if (maxval - payoff > maxz) {
      maxz = maxval - payoff;
      ylabel[1] = i;
      ylabel[2] = jj;
    }
  }
  if (maxz < bestz) {
    bestz = maxz;
    for (int i = 1; i <= yy.GetGame()->NumPlayers(); i++) {
      GamePlayer player = yy.GetGame()->Players()[i];
      for (int j = 1; j <= player->Strategies().size(); j++) {
	besty(i,j) = yy[player->Strategies()[j]];
      }
    }
  }
  return maxz;
}


//-------------------------------------------------------------------------
//           NashSimpdivStrategySolver: Main solution algorithm
//-------------------------------------------------------------------------

List<MixedStrategyProfile<Rational> > 
ReadProfiles(const Game &p_game, std::istream &p_stream)
{
  List<MixedStrategyProfile<Rational> > profiles;
  while (!p_stream.eof() && !p_stream.bad()) {
    MixedStrategyProfile<Rational> p(p_game->NewMixedStrategyProfile(Rational(0)));
    for (int i = 1; i <= p.MixedProfileLength(); i++) {
      if (p_stream.eof() || p_stream.bad()) {
	break;
      }
      p_stream >> p[i];
      if (i < p.MixedProfileLength()) {
	char comma;
	p_stream >> comma;
      }
    }
    // Read in the rest of the line and discard
    std::string foo;
    std::getline(p_stream, foo);
    profiles.push_back(p);
  }
  return profiles;
}

List<MixedStrategyProfile<Rational> > 
RandomProfiles(const Game &p_game, int p_count, const Rational &denom)
{
  List<MixedStrategyProfile<Rational> > profiles;
  for (int i = 1; i <= p_count; i++) {
    MixedStrategyProfile<Rational> p(p_game->NewMixedStrategyProfile(Rational(0)));
    p.Randomize(denom);
    profiles.push_back(p);
  }
  return profiles;
}

Integer find_lcd(const Vector<Rational> &vec)
{
  Integer lcd(1);
  for (int i = vec.First(); i <= vec.Last(); i++) {
    lcd = lcm(vec[i].denominator(), lcd);
  }
  return lcd;
}

List<MixedStrategyProfile<Rational> >
NashSimpdivStrategySolver::Solve(const MixedStrategyProfile<Rational> &p_start) const
{
  if (!p_start.GetGame()->IsPerfectRecall()) {
    throw UndefinedException("Computing equilibria of games with imperfect recall is not supported.");
  }
  Integer k = find_lcd((const Vector<Rational> &) p_start);
  Rational d = Rational(1, k);
    
  MixedStrategyProfile<Rational> y(p_start);
  if (m_verbose) {
    this->m_onEquilibrium->Render(y, "start");
  }

  while (true) {
    const double TOL = 1.0e-10;
    d /= m_gridResize;
    Rational maxz = Simplex(y, d);
    
    if (m_verbose) {
      this->m_onEquilibrium->Render(y, lexical_cast<std::string>(d));
    }
    if (maxz < Rational(TOL)) break;
  }
    
  this->m_onEquilibrium->Render(y);
  List<MixedStrategyProfile<Rational> > sol;
  sol.push_back(y);
  return sol;
}

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria using simplicial subdivision\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2014, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}


void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS] [file]\n";
  std::cerr << "If file is not specified, attempts to read game from standard input.\n";
  std::cerr << "With no options, computes one approximate Nash equilibrium.\n\n";

  std::cerr << "Options:\n";
  std::cerr << "  -g MULT          granularity of grid refinement at each step (default is 2)\n";
  std::cerr << "  -h, --help       print this help message\n";
  std::cerr << "  -r DENOM         generate random starting points with denominator DENOM\n";
  std::cerr << "  -n COUNT         number of starting points to generate (requires -r)\n";
  std::cerr << "  -s FILE          file containing starting points\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -V, --verbose    verbose mode (shows intermediate output)\n";
  std::cerr << "  -v, --version    print version information\n";
  std::cerr << "                   (default is to only show equilibria)\n";
  exit(1);
}

int main(int argc, char *argv[])
{
  opterr = 0;
  std::string startFile;
  bool useRandom = false;
  int randDenom = 1, gridResize = 2, stopAfter = 1;
  bool verbose = false, quiet = false;

  int long_opt_index = 0;
  struct option long_options[] = {
    { "help", 0, NULL, 'h'   },
    { "version", 0, NULL, 'v'  },
    { "verbose", 0, NULL, 'V'  },
    { 0,    0,    0,    0   }
  };
  int c;
  while ((c = getopt_long(argc, argv, "g:hVvn:r:s:d:qS", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr); exit(1);
    case 'g':
      gridResize = atoi(optarg);
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'r':
      useRandom = true;
      randDenom = atoi(optarg);
      break;
    case 'n':
      stopAfter = atoi(optarg);
      break;
    case 's':
      startFile = optarg;
      break;
    case 'q':
      quiet = true;
      break;
    case 'V':
      verbose = true;
      break;
    case 'S':
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

  std::istream* input_stream = &std::cin;
  std::ifstream file_stream;
  if (optind < argc) {
    file_stream.open(argv[optind]);
    if (!file_stream.is_open()) {
      std::ostringstream error_message;
      error_message << argv[0] << ": " << argv[optind];
      perror(error_message.str().c_str());
      exit(1);
    }
    input_stream = &file_stream;
  }

  try {
    Game game = ReadGame(*input_stream);
    List<MixedStrategyProfile<Rational> > starts;
    if (startFile != "") {
      std::ifstream startPoints(startFile.c_str());
      starts = ReadProfiles(game, startPoints);
    }
    else if (useRandom) {
      starts = RandomProfiles(game, stopAfter, randDenom);
    }
    else {
      starts.push_back(game->NewMixedStrategyProfile(Rational(0)));
      static_cast<Vector<Rational> &>(starts[1]) = Rational(0);
      for (int pl = 1; pl <= game->NumPlayers(); pl++) {
	starts[1][game->Players()[pl]->Strategies()[1]] = Rational(1);
      }
    }
    for (int i = 1; i <= starts.size(); i++) {
      shared_ptr<StrategyProfileRenderer<Rational> > renderer;
      renderer = new MixedStrategyCSVRenderer<Rational>(std::cout);
      NashSimpdivStrategySolver algorithm(gridResize, 0, verbose,
					  renderer);
      algorithm.Solve(starts[i]);
    }
    return 0;
  }
  catch (std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
