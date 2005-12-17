//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of algorithm to compute mixed strategy equilibria
// of constant sum normal form games via linear programming
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
#include "lpsolve.h"

int g_numDecimals = 6;

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const gbtBehavProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream << "," << p_profile[i];
  }

  p_stream << std::endl;
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const gbtBehavProfile<gbtRational> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

template <class T> class nfgLp {
private:
  int Add_BFS(const gbtNfgSupport &, /*const*/ LPSolve<T> &B,
	      gbtList<BFS<T> > &);
  void GetSolutions(const gbtNfgSupport &, const gbtList<BFS<T> > &,
		    const T &) const;

public:
  nfgLp(void);
  virtual ~nfgLp() { }

  void Solve(const gbtNfgSupport &);
};

//-------------------------------------------------------------------------
//                      nfgLp<T>: Member functions
//-------------------------------------------------------------------------

template <class T> nfgLp<T>::nfgLp(void)
{ }

template <class T> 
void nfgLp<T>::Solve(const gbtNfgSupport &p_support)
{
  BFS<T> cbfs((T) 0);
  
  if (p_support.GetGame()->NumPlayers() != 2 || 
      !p_support.GetGame()->IsConstSum()) {
    return; 
  }
  
  gbtList<BFS<T> > bfsList;
  
  int i,j,m,k;
  
  m = p_support.NumStrats(1);
  k = p_support.NumStrats(2);
  gbtMatrix<T> A(1,k+1,1,m+1);
  gbtVector<T> b(1,k+1);
  gbtVector<T> c(1,m+1);
  gbtStrategyProfile profile(p_support.GetGame());

  T minpay = p_support.GetGame()->GetMinPayoff() - gbtRational(1);

  for (i = 1; i <= k; i++)  {
    profile.SetStrategy(p_support.GetStrategy(2, i));
    for (j = 1; j <= m; j++)  {
      profile.SetStrategy(p_support.GetStrategy(1, j));
      A(i, j) = gbtRational(minpay) - profile.GetPayoff(1);
    }
    A(i,m+1) = (T)1;
  }
  for (j = 1;j<=m;j++)  {
    A(k+1,j)= (T)1;
  }
  A(k+1,m+1) = (T)0;

  b = (T)0;
  b[k+1] = (T)1;
  c = (T)0;
  c[m+1] = (T)1;

  LPSolve<T> LP(A,b,c,1);

  if (!LP.IsAborted()) {
    Add_BFS(p_support, LP, bfsList); 
  }

  GetSolutions(p_support, bfsList, LP.Epsilon());
}

template <class T> int nfgLp<T>::Add_BFS(const gbtNfgSupport &p_support,
					 /*const*/ LPSolve<T> &lp,
					 gbtList<BFS<T> > &p_list)
{
  BFS<T> cbfs((T) 0);

  // LPSolve<T>::GetAll() does not currently work correctly; for now,
  // LpSolve is restricted to returning only one equilibrium
  lp.OptBFS(cbfs);
  cbfs.Remove(p_support.GetGame()->NumStrats(1)+1);
  cbfs.Remove(-p_support.GetGame()->NumStrats(2)-1);
  if (p_list.Contains(cbfs))  return 0;
  p_list.Append(cbfs);
  return 1;
}

template <class T>
void nfgLp<T>::GetSolutions(const gbtNfgSupport &p_support,
			    const gbtList<BFS<T> > &p_list,
			    const T &p_epsilon) const
{
  int index;
  int n1 = p_support.NumStrats(1);
  int n2=p_support.NumStrats(2);

  for (int i = 1; i <= p_list.Length(); i++)    {
    gbtMixedProfile<T> profile(p_support);
    int j;
    for (j = 1; j <= n1; j++) 
      if (p_list[i].IsDefined(j))   
	profile(1, j) = p_list[i](j);
      else  profile(1, j) = (T) 0;

    for (j = 1; j <= n2; j++)
      if (p_list[i].IsDefined(-j))
	profile(2, j) = p_list[i](-j);
      else
	profile(2, j) = (T) 0;

    PrintProfile(std::cout, "NE", profile);
  }
}

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by solving a linear program\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 2005, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS]\n";
  std::cerr << "Accepts strategic game on standard input.\n";
  std::cerr << "With no options, reports all Nash equilibria found.\n\n";

  std::cerr << "Options:\n";
  std::cerr << "  -d DECIMALS      compute using floating-point arithmetic;\n";
  std::cerr << "                   display results with DECIMALS digits\n";
  std::cerr << "  -h               print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  exit(1);
}


int main(int argc, char *argv[])
{
  int c;
  bool useFloat = false, quiet = false;

  while ((c = getopt(argc, argv, "d:hq")) != -1) {
    switch (c) {
    case 'd':
      useFloat = true;
      g_numDecimals = atoi(optarg);
      break;
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

  gbtNfgGame nfg;

  try {
    nfg = ReadNfg(std::cin);
  }
  catch (...) {
    return 1;
  }

  if (nfg->NumPlayers() != 2 || !nfg->IsConstSum()) {
    return 1;
  }

  if (useFloat) {
    nfgLp<double> algorithm;
    algorithm.Solve(nfg);
  }
  else {
    nfgLp<gbtRational> algorithm;
    algorithm.Solve(nfg);
  }

  return 0;

}

