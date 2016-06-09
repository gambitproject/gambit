//
// This file is part of Gambit
// Copyright (c) 1994-2016, The Gambit Project (http://www.gambit-project.org)
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

#include "gambit/gambit.h"
#include "gambit/linalg/vertenum.imp"
#include "gambit/nash/enummixed.h"
#include "clique.h"


using namespace Gambit;
using namespace Gambit::linalg;
using namespace Gambit::Nash;


template <class T> class EnumMixedStrategySolver;

///
/// This encapsulates the data output by a run of enumeration of
/// mixed strategies.
///
template <class T> class EnumMixedStrategySolution {
  friend class EnumMixedStrategySolver<T>;
public:
  EnumMixedStrategySolution(const Game &p_game) : m_game(p_game) { }
  ~EnumMixedStrategySolution()  { }

  const Game &GetGame(void) const { return m_game; }
  const List<MixedStrategyProfile<T> > &GetExtremeEquilibria(void) const
  { return m_extremeEquilibria; }
  
  List<List<MixedStrategyProfile<T> > > GetCliques(void) const;

private:
  Game m_game;
  List<MixedStrategyProfile<T> > m_extremeEquilibria;

  /// Representation of the graph connecting the extreme equilibria
  ///@{
  List<Vector<T> > m_key1, m_key2;  
  List<int> m_node1, m_node2; // IDs of each component of the extreme equilibria
  int m_v1, m_v2;
  ///@}

  /// Representation of the connectedness of the extreme equilibria
  /// These are generated only on demand
  mutable List<Array<int> > m_cliques1, m_cliques2;
};

template <class T> List<List<MixedStrategyProfile<T> > > 
EnumMixedStrategySolution<T>::GetCliques(void) const
{
  if (m_cliques1.size() == 0) {
    // Cliques are generated on demand
    int n = m_node1.Length();
    if (m_node2.Length() != n)  throw DimensionException();

    Array<edge> edgelist(n);
    for (int i = 1; i <= n; i++) {
      edgelist[i].node1 = m_node1[i];
      edgelist[i].node2 = m_node2[i];
    }

    EnumCliques clique(edgelist, m_v2+1, m_v1+1);
    m_cliques1 = clique.GetCliques1();
    m_cliques2 = clique.GetCliques2();
  }

  List<List<MixedStrategyProfile<T> > > solution;
  for (int cl = 1; cl <= m_cliques1.Length(); cl++) {
    solution.push_back(List<MixedStrategyProfile<T> >());
    for (int i = 1; i <= m_cliques1[cl].Length(); i++) {
      for (int j = 1; j <= m_cliques2[cl].Length(); j++) {
	MixedStrategyProfile<T> profile(m_game->NewMixedStrategyProfile(static_cast<T>(0)));

	for (int k = 1; k <= m_key1[m_cliques1[cl][i]].Length(); k++) {
	  profile[k] = m_key1[m_cliques1[cl][i]][k];
	}
	for (int k = 1; k <= m_key2[m_cliques2[cl][j]].Length(); k++) {
	  profile[k + m_key1[m_cliques1[cl][i]].Length()] =
	    m_key2[m_cliques2[cl][j]][k];
	}
	solution[cl].push_back(profile);
      }
    }
  }
  return solution;
}

template <class T> class EnumMixedStrategySolver : public StrategySolver<T> {
public:
  EnumMixedStrategySolver(shared_ptr<StrategyProfileRenderer<T> > p_onEquilibrium = 0)
    : StrategySolver<T>(p_onEquilibrium) {}
  virtual ~EnumMixedStrategySolver() { }

  shared_ptr<EnumMixedStrategySolution<T> > SolveDetailed(const Game &p_game) const;
  List<MixedStrategyProfile<T> > Solve(const Game &p_game) const
  { return SolveDetailed(p_game)->GetExtremeEquilibria(); }
  
  
private:
  /// Implement fuzzy equality for floating-point version when testing Nashness
  static bool EqZero(const T &x);
};

template <class T> shared_ptr<EnumMixedStrategySolution<T> >
EnumMixedStrategySolver<T>::SolveDetailed(const Game &p_game) const
{
  shared_ptr<EnumMixedStrategySolution<T> > solution = new EnumMixedStrategySolution<T>(p_game);

  PureStrategyProfile profile = p_game->NewPureStrategyProfile();

  Rational min = p_game->GetMinPayoff();
  if (min > Rational(0)) {
    min = Rational(0);
  }
  min -= Rational(1);

  Rational max = p_game->GetMaxPayoff();
  if (max < Rational(0)) {
    max = Rational(0);
  }

  Rational fac(1, max - min);

  // Construct matrices A1, A2
  Matrix<T> A1(1, p_game->Players()[1]->Strategies().size(),
	       1, p_game->Players()[2]->Strategies().size());
  Matrix<T> A2(1, p_game->Players()[2]->Strategies().size(),
	       1, p_game->Players()[1]->Strategies().size());

  for (int i = 1; i <= p_game->Players()[1]->Strategies().size(); i++) {
    profile->SetStrategy(p_game->Players()[1]->Strategies()[i]);
    for (int j = 1; j <= p_game->Players()[2]->Strategies().size(); j++) {
      profile->SetStrategy(p_game->Players()[2]->Strategies()[j]);
      A1(i, j) = fac * (profile->GetPayoff(1) - min);
      A2(j, i) = fac * (profile->GetPayoff(2) - min);
    }
  }

  // Construct vectors b1, b2
  Vector<T> b1(1, p_game->Players()[1]->Strategies().size());
  Vector<T> b2(1, p_game->Players()[2]->Strategies().size());
  b1 = (T) -1;
  b2 = (T) -1;

  // enumerate vertices of A1 x + b1 <= 0 and A2 x + b2 <= 0
  VertexEnumerator<T> poly1(A1, b1);
  VertexEnumerator<T> poly2(A2, b2);

  const List<BFS<T> > &verts1(poly1.VertexList());
  const List<BFS<T> > &verts2(poly2.VertexList());
  solution->m_v1 = verts1.Length();
  solution->m_v2 = verts2.Length();

  Array<int> vert1id(solution->m_v1);
  Array<int> vert2id(solution->m_v2);
  for (int i = 1; i <= vert1id.Length(); vert1id[i++] = 0);
  for (int i = 1; i <= vert2id.Length(); vert2id[i++] = 0);

  int i = 0;
  int id1 = 0, id2 = 0;

  for (int i2 = 2; i2 <= solution->m_v2; i2++) {
    BFS<T> bfs1 = verts2[i2];
    i++;
    for (int i1 = 2; i1 <= solution->m_v1; i1++) {
      BFS<T> bfs2 = verts1[i1];
	
      // check if solution is nash 
      // need only check complementarity, since it is feasible
      bool nash = true;
      for (int k = 1; nash && k <= p_game->Players()[1]->Strategies().size(); k++) {
	if (bfs1.count(k) && bfs2.count(-k)) {
	  nash = nash && EqZero(bfs1[k] * bfs2[-k]);
	}
      }

      for (int k = 1; nash && k <= p_game->Players()[2]->Strategies().size(); k++) {
	if (bfs2.count(k) && bfs1.count(-k)) {
	  nash = nash && EqZero(bfs2[k] * bfs1[-k]);
	}
      }

      if (nash) {
	MixedStrategyProfile<T> profile(p_game->NewMixedStrategyProfile(static_cast<T>(0)));
	static_cast<Vector<T> &>(profile) = static_cast<T>(0);
	for (int k = 1; k <= p_game->Players()[1]->Strategies().size(); k++) {
	  if (bfs1.count(k)) {
	    profile[p_game->Players()[1]->Strategies()[k]] = -bfs1[k];
	  }
	} 
	for (int k = 1; k <= p_game->Players()[2]->Strategies().size(); k++) {
	  if (bfs2.count(k)) {
	    profile[p_game->Players()[2]->Strategies()[k]] = -bfs2[k];
	  }
	} 
	profile.Normalize();
	solution->m_extremeEquilibria.push_back(profile);
	this->m_onEquilibrium->Render(profile);
	  
	// note: The keys give the mixed strategy associated with each node. 
	//       The keys should also keep track of the basis
	//       As things stand now, two different bases could lead to
	//       the same key... BAD!
	if (vert1id[i1] == 0) {
	  id1++;
	  vert1id[i1] = id1;
	  solution->m_key2.push_back(profile[p_game->GetPlayer(2)]);
	}
	if (vert2id[i2] == 0) {
	  id2++;
	  vert2id[i2] = id2;
	  solution->m_key1.push_back(profile[p_game->GetPlayer(1)]);
	}
	solution->m_node1.Append(vert2id[i2]);
	solution->m_node2.Append(vert1id[i1]);
      }
    }
  }
  return solution;
}

template<> bool EnumMixedStrategySolver<double>::EqZero(const double &x) 
{
  double eps = ::pow(10.0, -15.0);
  return (x <= eps && x >= -eps);
}     

template<> bool EnumMixedStrategySolver<Rational>::EqZero(const Rational &x)
{
  return (x == Rational(0));
}

template <class T> void
PrintCliques(const List<List<MixedStrategyProfile<T> > > &p_cliques,
	     shared_ptr<StrategyProfileRenderer<T> > p_renderer)
{
  for (int cl = 1; cl <= p_cliques.size(); cl++) {
    for (int i = 1; i <= p_cliques[cl].size(); i++) {
      p_renderer->Render(p_cliques[cl][i],
			 "convex-" + lexical_cast<std::string>(cl)); 
    }
  }
}
      
void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by enumerating extreme points\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2016, The Gambit Project\n";
  p_stream << "Enumeration code based on lrslib 6.2,\n";
  p_stream << "Copyright (C) 1995-2016 by David Avis (avis@cs.mcgill.ca)\n";
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
  bool showConnect = false;
  int numDecimals = 6;

  int long_opt_index = 0;
  struct option long_options[] = {
    { "help", 0, NULL, 'h'   },
    { "version", 0, NULL, 'v'  },
    { 0,    0,    0,    0   }
  };
  while ((c = getopt_long(argc, argv, "d:DvhqcSL", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr); exit(1);
    case 'd':
      useFloat = true;
      numDecimals = atoi(optarg);
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
      showConnect = true;
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
    if (!game->IsPerfectRecall()) {
      throw UndefinedException("Computing equilibria of games with imperfect recall is not supported.");
    }

    if (uselrs) {
      shared_ptr<StrategyProfileRenderer<Rational> > renderer;
      renderer = new MixedStrategyCSVRenderer<Rational>(std::cout);
      EnumMixedLrsStrategySolver solver(renderer);
      solver.Solve(game);
    }
    else if (useFloat) {
      shared_ptr<StrategyProfileRenderer<double> > renderer;
      renderer = new MixedStrategyCSVRenderer<double>(std::cout,
						      numDecimals);
      EnumMixedStrategySolver<double> solver(renderer);
      shared_ptr<EnumMixedStrategySolution<double> > solution =
	solver.SolveDetailed(game);
      if (showConnect) {
	List<List<MixedStrategyProfile<double> > > cliques =
	  solution->GetCliques();
	PrintCliques(cliques, renderer);
      }
    }
    else {
      shared_ptr<StrategyProfileRenderer<Rational> > renderer;
      renderer = new MixedStrategyCSVRenderer<Rational>(std::cout);
      EnumMixedStrategySolver<Rational> solver(renderer);
      shared_ptr<EnumMixedStrategySolution<Rational> > solution =
	solver.SolveDetailed(game);
      if (showConnect) {
	List<List<MixedStrategyProfile<Rational> > > cliques =
	  solution->GetCliques();
	PrintCliques(cliques, renderer);
      }
    }
    return 0;
  }
  catch (std::runtime_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}










