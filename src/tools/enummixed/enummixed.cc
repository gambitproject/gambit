//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enummixed/enummixed.cc
// Compute Nash equilibria via Mangasarian's algorithm
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

#include <cstdlib>
#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <iomanip>

#include "libgambit/libgambit.h"
#include "clique.h"
#include "vertenum.imp"


using namespace Gambit;

bool g_showConnect = false;
int g_numDecimals = 6;

bool EqZero(const double &x)
{
  double eps = ::pow(10.0, -15.0);
  return (x <= eps && x >= -eps);
}     

bool EqZero(const Rational &x)
{
  return (x == Gambit::Rational(0));
}

int m_stopAfter = 0;

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const MixedStrategyProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << ',' << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const MixedStrategyProfile<Rational> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    p_stream << ',' << p_profile[i];
  }

  p_stream << std::endl;
}

template <class T> void GetCliques(std::ostream &p_stream,
				   const StrategySupport &p_support,
				   const List<int> &p_node1,
				   const List<Vector<T> > &p_key1,
				   int p_v1,
				   const List<int> &p_node2,
				   const List<Vector<T> > &p_key2,
				   int p_v2)
{
  int n = p_node1.Length();
  if (p_node2.Length() != n)  throw DimensionException();

  Array<edge> edgelist(n);

  //p_stream << "\nKey:\nPlayer 1:";
  //for (int i = 1; i <= p_key1.Length(); i++) {
    //p_stream << "\n" << i << ":";
    //for (int j = 1; j <= p_key1[i].Length(); j++) {
    //  p_stream << " " << p_key1[i][j];
    //}
  //p_stream << "\n";
  //}
  //p_stream << "\nPlayer 2:";
  //for (int i = 1; i <= p_key2.Length(); i++) {
  // p_stream << "\n" << i << ": ";
  // for (int j = 1; j <= p_key2[i].Length(); j++) {
  //    p_stream << " " << p_key2[i][j];
  //  }
  //}
  //p_stream << "\nExtreme equilibria:";
  for (int i = 1; i <= n; i++) {
    edgelist[i].node1 = p_node1[i];
    edgelist[i].node2 = p_node2[i];
    // p_stream << "\n" << p_node1[i] << " " << p_node2[i];
  }

  EnumCliques clique(edgelist, p_v2+1, p_v1+1);
  const List<Array<int> > &cliques1 = clique.GetCliques1();
  const List<Array<int> > &cliques2 = clique.GetCliques2();

  for (int cl = 1; cl <= cliques1.Length(); cl++) {
    for (int i = 1; i <= cliques1[cl].Length(); i++) {
      for (int j = 1; j <= cliques2[cl].Length(); j++) {
	MixedStrategyProfile<T> profile(p_support.NewMixedStrategyProfile<T>());

	for (int k = 1; k <= p_key1[cliques1[cl][i]].Length(); k++) {
	  profile[k] = p_key1[cliques1[cl][i]][k];
	}
	for (int k = 1; k <= p_key2[cliques2[cl][j]].Length(); k++) {
	  profile[k + p_key1[cliques1[cl][i]].Length()] =
	    p_key2[cliques2[cl][j]][k];
	}

	PrintProfile(p_stream, "convex-" + lexical_cast<std::string>(cl), 
		     profile.ToFullSupport());
      }
    }
  }
}

template <class T> void Solve(const StrategySupport &p_support)
{
  List<Vector<T> > key1, key2;  
  List<int> node1, node2;   // IDs of each component of the extreme equilibria

  PureStrategyProfile profile = p_support.GetGame()->NewPureStrategyProfile();

  Rational min = p_support.GetGame()->GetMinPayoff();
  if (min > Rational(0)) {
    min = Rational(0);
  }
  min -= Rational(1);

  Rational max = p_support.GetGame()->GetMaxPayoff();
  if (max < Rational(0)) {
    max = Rational(0);
  }

  Rational fac(1, max - min);

  // Construct matrices A1, A2
  Matrix<T> A1(1, p_support.NumStrategies(1),
	       1, p_support.NumStrategies(2));
  Matrix<T> A2(1, p_support.NumStrategies(2),
	       1, p_support.NumStrategies(1));

  for (int i = 1; i <= p_support.NumStrategies(1); i++) {
    profile->SetStrategy(p_support.GetStrategy(1, i));
    for (int j = 1; j <= p_support.NumStrategies(2); j++) {
      profile->SetStrategy(p_support.GetStrategy(2, j));
      A1(i, j) = fac * (profile->GetPayoff(1) - min);
      A2(j, i) = fac * (profile->GetPayoff(2) - min);
    }
  }

  // Construct vectors b1, b2
  Vector<T> b1(1, p_support.NumStrategies(1));
  Vector<T> b2(1, p_support.NumStrategies(2));
  b1 = (T) -1;
  b2 = (T) -1;

  // enumerate vertices of A1 x + b1 <= 0 and A2 x + b2 <= 0
  VertEnum<T> poly1(A1, b1);
  VertEnum<T> poly2(A2, b2);

  const List<BFS<T> > &verts1(poly1.VertexList());
  const List<BFS<T> > &verts2(poly2.VertexList());
  int v1 = verts1.Length();
  int v2 = verts2.Length();

  Array<int> vert1id(v1);
  Array<int> vert2id(v2);
  for (int i = 1; i <= vert1id.Length(); vert1id[i++] = 0);
  for (int i = 1; i <= vert2id.Length(); vert2id[i++] = 0);

  int i = 0;
  int id1 = 0, id2 = 0;

  try {
    for (int i2 = 2; i2 <= v2; i2++) {
      BFS<T> bfs1 = verts2[i2];
      i++;
      for (int i1 = 2; i1 <= v1; i1++) {
	BFS<T> bfs2 = verts1[i1];
	
	// check if solution is nash 
	// need only check complementarity, since it is feasible
	bool nash = true;
	for (int k = 1; nash && k <= p_support.NumStrategies(1); k++) {
	  if (bfs1.count(k) && bfs2.count(-k)) {
	    nash = nash && EqZero(bfs1[k] * bfs2[-k]);
	  }
	}

	for (int k = 1; nash && k <= p_support.NumStrategies(2); k++) {
	  if (bfs2.count(k) && bfs1.count(-k)) {
	    nash = nash && EqZero(bfs2[k] * bfs1[-k]);
	  }
	}

	if (nash) {
	  MixedStrategyProfile<T> profile(p_support.NewMixedStrategyProfile<T>());
	  T sum = (T) 0;
	  for (int k = 1; k <= p_support.NumStrategies(1); k++) {
	    profile[p_support.GetStrategy(1, k)] = (T) 0;
	    if (bfs1.count(k)) {
	      profile[p_support.GetStrategy(1, k)] = -bfs1[k];
	      sum += profile[p_support.GetStrategy(1, k)];
	    }
	  } 
	  
	  for (int k = 1; k <= p_support.NumStrategies(1); k++) {
	    if (bfs1.count(k)) { 
	      profile[p_support.GetStrategy(1, k)] /= sum;
	    }
	  }
	  
	  sum = (T) 0;
	  for (int k = 1; k <= p_support.NumStrategies(2); k++) {
	    profile[p_support.GetStrategy(2, k)] = (T) 0;
	    if (bfs2.count(k)) {
	      profile[p_support.GetStrategy(2, k)] = -bfs2[k];
	      sum += profile[p_support.GetStrategy(2, k)];
	    }
	  } 
	  
	  for (int k = 1; k <= p_support.NumStrategies(2); k++) {
	    if (bfs2.count(k)) { 
	      profile[p_support.GetStrategy(2, k)] /= sum;
	    }
	  }

	  PrintProfile(std::cout, "NE", profile.ToFullSupport());
	  
	  // note: The keys give the mixed strategy associated with each node. 
	  //       The keys should also keep track of the basis
	  //       As things stand now, two different bases could lead to
	  //       the same key... BAD!
	  if (vert1id[i1] == 0) {
	    id1++;
	    vert1id[i1] = id1;

	    Vector<T> probs(profile.GetSupport().NumStrategies(2));
	    for (SupportStrategyIterator strategy = profile.GetSupport().Strategies(profile.GetGame()->GetPlayer(2));
		 !strategy.AtEnd(); strategy++) {
	      probs[strategy.GetIndex()] = profile[strategy];
	    }
	    key2.Append(probs);
	  }
	  if (vert2id[i2] == 0) {
	    id2++;
	    vert2id[i2] = id2;

	    Vector<T> probs(profile.GetSupport().NumStrategies(1));
	    for (SupportStrategyIterator strategy = profile.GetSupport().Strategies(profile.GetGame()->GetPlayer(1));
		 !strategy.AtEnd(); strategy++) {
	      probs[strategy.GetIndex()] = profile[strategy];
	    }
	    key1.Append(probs);
	  }
	  node1.Append(vert2id[i2]);
	  node2.Append(vert1id[i1]);
	}
      }
    }
    if (g_showConnect) {
      GetCliques(std::cout, p_support, node1, key1, v1, node2, key2, v2);
    }
  }
  catch (...) {
    // stop processing, just return equilibria found (if any)
  }
}

extern void LrsSolve(const StrategySupport &);

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by enumerating extreme points\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2010, The Gambit Project\n";
  p_stream << "Enumeration code based on lrslib 4.2b,\n";
  p_stream << "Copyright (C) 1995-2005 by David Avis (avis@cs.mcgill.ca)\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS] [file]\n";
  std::cerr << "If file is not specified, attempts to read game from standard input.\n";
  std::cerr << "With no options, reports all Nash equilibria found.\n\n";

  std::cerr << "Options:\n";
  std::cerr << "  -d DECIMALS      compute using floating-point arithmetic;\n";
  std::cerr << "                   display results with DECIMALS digits\n";
  std::cerr << "  -D               don't eliminate dominated strategies first\n";
  std::cerr << "  -L               use lrslib for enumeration (experimental!)\n";
  std::cerr << "  -c               output connectedness information\n";
  std::cerr << "  -h, --help       print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -v, --version    print version information\n";
  exit(1);
}

int main(int argc, char *argv[])
{
  int c;
  bool useFloat = false, uselrs = false, quiet = false, eliminate = true;

  int long_opt_index = 0;
  struct option long_options[] = {
    { "help", 0, NULL, 'h'   },
    { "version", 0, NULL, 'v'  },
    { 0,    0,    0,    0   }
  };
  while ((c = getopt_long(argc, argv, "d:DvhqcS", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr); exit(1);
    case 'd':
      useFloat = true;
      g_numDecimals = atoi(optarg);
      break;
    case 'D':
      eliminate = false;
      break;
    case 'L':
      uselrs = true;
      break;
    case 'h':
      PrintHelp(argv[0]);
      break;
    case 'c':
      g_showConnect = true;
      break;
    case 'S':
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

    if (game->NumPlayers() != 2) {
      std::cerr << "Error: Game does not have two players.\n";
      return 1;
    }

    StrategySupport support(game);
    if (eliminate) {
      while (true) {
	StrategySupport newSupport = support.Undominated(true);
	if (newSupport == support) break;
	support = newSupport;
      }
    }

    if (uselrs) {
      LrsSolve(support);
    }
    else if (useFloat) {
      Solve<double>(support);
    }
    else {
      Solve<Rational>(support);
    }
    return 0;
  }
  catch (InvalidFileException) {
    std::cerr << "Error: Game not in a recognized format.\n";
    return 1;
  }
  catch (...) {
    std::cerr << "Error: An internal error occurred.\n";
    return 1;
  }
}










