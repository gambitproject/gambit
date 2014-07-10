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


//
// Global options
//

bool g_verbose = false;
bool g_useFloat = false;
int g_numDecimals = 6;
int g_gridResize = 2;

class nfgSimpdiv {
private:
  int m_nRestarts, m_leashLength;

  int t, ibar;
  Rational d,pay,maxz,bestz;

  Rational Simplex(MixedStrategyProfile<Rational> &);
  Rational getlabel(MixedStrategyProfile<Rational> &yy, Array<int> &, PVector<Rational> &);
  void update(RectArray<int> &, RectArray<int> &, PVector<Rational> &,
	      const PVector<int> &, int j, int i);
  void getY(MixedStrategyProfile<Rational> &x, PVector<Rational> &, 
	    const PVector<int> &, const PVector<int> &, 
	    const PVector<Rational> &, const RectArray<int> &, int k);
  void getnexty(MixedStrategyProfile<Rational> &x, const RectArray<int> &,
		const PVector<int> &, int i) const;
  int get_c(int j, int h, int nstrats, const PVector<int> &) const;
  int get_b(int j, int h, int nstrats, const PVector<int> &) const;
  
public:
  nfgSimpdiv(void);
  virtual ~nfgSimpdiv();

  int NumRestarts(void) const { return m_nRestarts; }
  void SetNumRestarts(int p_nRestarts) { m_nRestarts = p_nRestarts; }

  int LeashLength(void) const { return m_leashLength; }
  void SetLeashLength(int p_leashLength) { m_leashLength = p_leashLength; }

  void Solve(const Game &, const MixedStrategyProfile<Rational> &);
};


//-------------------------------------------------------------------------
//               nfgSimpdiv: Constructor and destructor
//-------------------------------------------------------------------------

nfgSimpdiv::nfgSimpdiv(void)
  : m_nRestarts(20), m_leashLength(0)
{
  t = 0;
  ibar = 1;
}

nfgSimpdiv::~nfgSimpdiv()
{ }


//-------------------------------------------------------------------------
//               nfgSimpdiv: Private member functions
//-------------------------------------------------------------------------

inline GameStrategy GetStrategy(const Game &game, int pl, int st)
{
  return game->Players()[pl]->Strategies()[st];
}

Rational nfgSimpdiv::Simplex(MixedStrategyProfile<Rational> &y)
{
  Game game = y.GetGame();
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
  ibar = 1;
  t = 0;
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
  maxz = getlabel(y, ylabel, besty);
  j = ylabel[1];
  h = ylabel[2];
  labels(ibar,1) = j;
  labels(ibar,2) = h;
  
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
      i = t+1;
      goto step2;
    }
  }
  /* case1b */
  else if (TT(j,h))  {
    i = 1;
    while (labels(i,1) != j || labels(i,2) != h || i == ibar)  {
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
      i = t+1;
    }
    else {
      i = 1;
      while ((pi(i,1)!=j || pi(i,2)!=k) && i<=t) {
	i++;
      }
    }
    goto step2;
  }
  
 step2:
  getY(y, v, U, TT, ab, pi, i);
  pi.RotateDown(i, t+1);
  pi(i,1) = j;
  pi(i,2) = h;
  labels.RotateDown(i+1, t+2);
  ibar = i+1;
  t++;
  getnexty(y, pi, U, i);
  TT(j,h) = 1;
  U(j,h) = 0;
  goto step1;
  
 step3:
  ii = (i == t+1) ? t : 1;
  j = pi(ii,1);
  h = pi(ii,2);
  k = h;
  if (i < t+1) {
    k = get_b(j, h, nstrats[j], U);
  }
  kk = get_c(j, h, nstrats[j], U);
  if (i == 1) {
    ii = t+1;
  }
  else if (i == t+1) {
    ii = 1;
  }
  else {
    ii = i-1;
  }
  getY(y, v, U, TT, ab, pi, ii);
  
  /* case3a */
  if (i==1 && 
      (y[GetStrategy(game, j, k)]<=Rational(0) || 
       (v(j,k)-y[GetStrategy(game, j, k)]) >= Rational(m_leashLength)*d)) {
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
      update(pi, labels, ab, U, j, i);
      U(j,k) = 1;
      getnexty(y, pi, U, t);
      goto step1;
    }
  }
  /* case3b */
  else if (i>=2 && i<=t &&
	   (y[GetStrategy(game, j, k)] <= Rational(0) || 
	    (v(j,k)-y[GetStrategy(game, j, k)]) >= Rational(m_leashLength)*d)) {
    goto step4;
  }
  /* case3c */
  else if (i==t+1 && ab(j,kk) == Rational(0)) {
    if (y[GetStrategy(game, j, h)] <= Rational(0) || 
	(v(j,h)-y[GetStrategy(game, j, h)]) >= Rational(m_leashLength)*d) {
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
      getnexty(y, pi, U, 1);
    }
    else if (i<=t) {
      getnexty(y, pi, U, i);
    }
    else if (i==t+1) {
      j = pi(t,1);
      h = pi(t,2);
      hh = get_b(j,h,nstrats[j],U);
      y[GetStrategy(game, j, h)] -= d;
      y[GetStrategy(game, j, hh)] += d;
    }
    update(pi, labels, ab, U, j, i);
  }
  goto step1;

 step4:
  getY(y, v, U, TT, ab, pi, 1);
  j = pi(i-1,1);
  h = pi(i-1,2);
  TT(j,h) = 0;
  if (y[GetStrategy(game, j, h)] <= Rational(0) || 
      (v(j,h)-y[GetStrategy(game, j, h)]) >= Rational(m_leashLength)*d) {
    U(j,h) = 1;
  }
  labels.RotateUp(i,t+1);
  pi.RotateUp(i-1,t);
  t--;
  ii=1;
  while (labels(ii,1)!=j || labels(ii,2)!=h) {
    ii++;
  }
  i=ii;
  goto step3;

 step5:
  k=kk;
  labels.RotateDown(1,t+1);
  ibar=1;
  pi.RotateDown(1,t);
  U(j,k)=0;
  jj=pi(1,1);
  hh=pi(1,2);
  kk=get_b(jj,hh,nstrats[jj],U);
  y[GetStrategy(game, jj, hh)] -= d;
  y[GetStrategy(game, jj, kk)] += d;
  
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
  maxz=bestz;
  for (i = 1; i <= game->NumPlayers(); i++) {
    for (j = 1; j <= nstrats[i]; j++) {
      y[GetStrategy(game, i, j)] = besty(i,j);
    }
  }
  return maxz;
}

void nfgSimpdiv::update(RectArray<int> &pi,
			RectArray<int> &labels,
			PVector<Rational> &ab,
			const PVector<int> &U,
			int j, int i)
{
  int jj, hh, k,f;
  
  f=1;
  if(i>=2 && i<=t) {
    pi.SwitchRows(i,i-1);
    ibar=i;
  }
  else if(i==1) {
    labels.RotateUp(1,t+1);
    ibar=t+1;
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
      pi.RotateUp(1,t);
    }
  }
  else if(i==t+1) {
    labels.RotateDown(1,t+1);
    ibar=1;
    jj=pi(t,1);
    hh=pi(t,2);
    if(jj==j) {
      k=get_c(jj,hh,ab.Lengths()[jj],U);
      while(f) {
	if(k==hh)f=0;
	ab(j,k) -= Rational(1);
	k++;
	if(k>ab.Lengths()[jj])k=1;
      }
      pi.RotateDown(1,t);
    }
  }
}

void nfgSimpdiv::getY(MixedStrategyProfile<Rational> &x,
		      PVector<Rational> &v, 
		      const PVector<int> &U,
		      const PVector<int> &TT,
		      const PVector<Rational> &ab,
		      const RectArray<int> &pi,
		      int k)
{
  static_cast<Vector<Rational> & >(x) = v;
  for (int j = 1; j <= x.GetGame()->Players().size(); j++) {
    GamePlayer player = x.GetGame()->Players()[j];
    for (int h = 1; h <= player->Strategies().size(); h++) {
      if (TT(j,h) == 1 || U(j,h) == 1) {
	x[player->Strategies()[h]] += d*ab(j,h);
	int hh = (h > 1) ? h-1 : player->Strategies().size();
	x[player->Strategies()[hh]] -= d*ab(j,h);
      }
    }
  }
  for (int i = 2; i <= k; i++) {
    getnexty(x, pi, U, i-1);
  }
}

void nfgSimpdiv::getnexty(MixedStrategyProfile<Rational> &x,
			  const RectArray<int> &pi, 
			  const PVector<int> &U,
			  int i) const
{
  int j = pi(i,1);
  GamePlayer player = x.GetGame()->Players()[j];
  int h = pi(i,2);
  x[player->Strategies()[h]] += d;
  int hh = get_b(j, h, player->Strategies().size(), U);
  x[player->Strategies()[hh]] -= d;
}

int nfgSimpdiv::get_b(int j, int h, int nstrats, const PVector<int> &U) const
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

int nfgSimpdiv::get_c(int j, int h, int nstrats, const PVector<int> &U) const
{
  int hh = get_b(j, h, nstrats, U) + 1;
  return (hh > nstrats) ? 1 : hh;
}

Rational nfgSimpdiv::getlabel(MixedStrategyProfile<Rational> &yy,
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
//               nfgSimpdiv: Main solution algorithm
//-------------------------------------------------------------------------

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const MixedStrategyProfile<Rational> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    if (g_useFloat) {
      p_stream.setf(std::ios::fixed);
      p_stream << "," << std::setprecision(g_numDecimals) << ((double) p_profile[i]);
    }
    else {
      p_stream << "," << p_profile[i];
    }
  }

  p_stream << std::endl;
}

bool ReadProfile(std::istream &p_stream,
		 MixedStrategyProfile<Rational> &p_profile)
{
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    if (p_stream.eof() || p_stream.bad()) {
      return false;
    }

    p_stream >> p_profile[i];
    if (i < p_profile.MixedProfileLength()) {
      char comma;
      p_stream >> comma;
    }
  }

  // Read in the rest of the line and discard
  std::string foo;
  std::getline(p_stream, foo);
  return true;
}

Integer find_lcd(const Vector<Rational> &vec)
{
  Integer lcd(1);
  for (int i = vec.First(); i <= vec.Last(); i++) {
    lcd = lcm(vec[i].denominator(), lcd);
  }
  return lcd;
}

void nfgSimpdiv::Solve(const Game &p_nfg, 
		       const MixedStrategyProfile<Rational> &p_start)
{
  // A raft of initializations moved here from the former constructor.
  // This algorithm is in need of some serious reorganization!
  t = 0;
  ibar = 1;

  if (m_leashLength == 0) {
    m_leashLength = 32000;  // not the best way to do this.  Change this!
  }
  bestz = 1.0e30;          // ditto

  Integer k = find_lcd((const Vector<Rational> &) p_start);
  d = Rational(1, k);
    
  MixedStrategyProfile<Rational> y(p_start);
  if (g_verbose) {
    PrintProfile(std::cout, "start", y);
  }

  while (true) {
    const double TOL = 1.0e-10;
    d /= g_gridResize;
    maxz = Simplex(y);
    
    if (g_verbose) {
      PrintProfile(std::cout, lexical_cast<std::string>(d), y);
    }
    if (maxz < Rational(TOL)) break;
  }
    
  PrintProfile(std::cout, "NE", y);
}

void Randomize(MixedStrategyProfile<Rational> &p_profile, int p_denom)
{
  Game nfg = p_profile.GetGame();
  static_cast<Vector<Rational> &>(p_profile) = Rational(0);

  for (int pl = 1; pl <= nfg->NumPlayers(); pl++) {
    GamePlayer player = nfg->Players()[pl];
    int sum = 0;
    for (int st = 1; sum < p_denom && st < player->Strategies().size(); st++) {
      double r = ((double) rand() / ((double) (RAND_MAX) + 1.0));
      double x = r * (p_denom - sum + 1);
      int y = (int) x;
      // y is now uniform on [0, (p_denom-sum)].
      p_profile[player->Strategies()[st]] = Rational(y, p_denom);
      sum += y;
    }

    p_profile[player->Strategies().back()] = Rational(p_denom - sum, p_denom);
  }
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
  std::cerr << "  -d DECIMALS      show equilibria as floating point, with DECIMALS digits\n";
  std::cerr << "                   (default is to show as rational numbers)\n";
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
  int randDenom = 1, stopAfter = 1;
  bool quiet = false;

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
    case 'd':
      g_numDecimals = atoi(optarg);
      g_useFloat = true;
      break;
    case 'g':
      g_gridResize = atoi(optarg);
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
      g_verbose = true;
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

    if (startFile != "") {
      std::ifstream startPoints(startFile.c_str());
      
      while (!startPoints.eof() && !startPoints.bad()) {
	MixedStrategyProfile<Rational> start(game->NewMixedStrategyProfile(Rational(0)));
	if (ReadProfile(startPoints, start)) {
	  nfgSimpdiv algorithm;
	  algorithm.Solve(game, start);
	}
      }
    }
    else if (useRandom) {
      for (int i = 1; i <= stopAfter; i++) {
	MixedStrategyProfile<Rational> start(game->NewMixedStrategyProfile(Rational(0)));
	Randomize(start, randDenom);
	
	nfgSimpdiv algorithm;
	algorithm.Solve(game, start);
      }
    }
    else {
      MixedStrategyProfile<Rational> start(game->NewMixedStrategyProfile(Rational(0)));
  
      static_cast<Vector<Rational> &>(start) = Rational(0);
      for (int pl = 1; pl <= game->NumPlayers(); pl++) {
	start[game->Players()[pl]->Strategies()[1]] = Rational(1);
      }
      
      nfgSimpdiv algorithm;
      algorithm.Solve(game, start);
    }
   
    return 0;
  }
  catch (InvalidFileException e) {
    std::cerr << "Error: Game not in a recognized format.\n";
    if (g_verbose) std::cerr<<e.what()<<endl;
    return 1;
  }
  catch (...) {
    std::cerr << "Error: An internal error occurred.\n";
    return 1;
  }
}
