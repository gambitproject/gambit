//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/enumpure/enumpure.cc
// Compute pure-strategy equilibria in extensive form games
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
#include <getopt.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cerrno>
#include "libgambit/libgambit.h"
#include "libgambit/subgame.h"

using namespace Gambit;

template <class T>
void PrintProfile(std::ostream &p_stream,
		  const MixedBehavProfile<T> &p_profile)
{
  p_stream << "NE,";
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream << p_profile[i];
    if (i < p_profile.Length()) {
      p_stream << ',';
    }
  }

  p_stream << std::endl;
}

template <class T> class MixedStrategyRenderer {
public:
  virtual ~MixedStrategyRenderer() { }
  virtual void Render(const MixedStrategyProfile<T> &p_profile) = 0;
};

template <class T> 
class MixedStrategyNullRenderer : public MixedStrategyRenderer<T> {
public:
  virtual ~MixedStrategyNullRenderer() { }
  virtual void Render(const MixedStrategyProfile<T> &p_profile) { }
};

template <class T>
class MixedStrategyCSVRenderer : public MixedStrategyRenderer<T> {
private:
  std::ostream &m_stream;

public:
  MixedStrategyCSVRenderer(std::ostream &p_stream)
    : m_stream(p_stream) { }
  virtual ~MixedStrategyCSVRenderer() { }
  virtual void Render(const MixedStrategyProfile<T> &p_profile);
};

template <class T>
void MixedStrategyCSVRenderer<T>::Render(const MixedStrategyProfile<T> &p_profile)
{
  m_stream << "NE,";
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    m_stream << p_profile[i];
    if (i < p_profile.MixedProfileLength()) {
      m_stream << ',';
    }
  }
  m_stream << std::endl;
}

//
// Render mixed strategies as their behavior strategy equivalents
//
template <class T>
class MixedStrategyAsBehavCSVRenderer : public MixedStrategyRenderer<T> {
private:
  std::ostream &m_stream;

public:
  MixedStrategyAsBehavCSVRenderer(std::ostream &p_stream)
    : m_stream(p_stream) { }
  virtual ~MixedStrategyAsBehavCSVRenderer() { }
  virtual void Render(const MixedStrategyProfile<T> &p_profile);
};

template <class T>
void MixedStrategyAsBehavCSVRenderer<T>::Render(const MixedStrategyProfile<T> &p_profile)
{
  MixedBehavProfile<T> behav(p_profile);
  m_stream << "NE,";
  for (int i = 1; i <= behav.Length(); i++) {
    m_stream << behav[i];
    if (i < behav.Length()) {
      m_stream << ',';
    }
  }
  m_stream << std::endl;
}


class NashEnumPureStrategySolver {
private:
  MixedStrategyRenderer<Rational> *m_onEquilibrium;

public:
  NashEnumPureStrategySolver(MixedStrategyRenderer<Rational> *p_onEquilibrium = 0);
  ~NashEnumPureStrategySolver()  { delete m_onEquilibrium; }
  List<MixedStrategyProfile<Rational> > Solve(const Game &p_game) const;
};

NashEnumPureStrategySolver::NashEnumPureStrategySolver(MixedStrategyRenderer<Rational> *p_onEquilibrium /* = 0 */)
  : m_onEquilibrium(p_onEquilibrium)
{
  if (!m_onEquilibrium) {
    m_onEquilibrium = new MixedStrategyNullRenderer<Rational>();
  }
}

List<MixedStrategyProfile<Rational> >
NashEnumPureStrategySolver::Solve(const Game &p_game) const
{
  List<MixedStrategyProfile<Rational> > solutions;
  for (StrategyIterator citer(p_game); !citer.AtEnd(); citer++) {
    if ((*citer)->IsNash()) {
      MixedStrategyProfile<Rational> profile = (*citer)->ToMixedStrategyProfile();
      m_onEquilibrium->Render(profile);
      solutions.Append(profile);
    }
  }
  return solutions;
}



template <class T> class BehavStrategyRenderer {
public:
  virtual ~BehavStrategyRenderer() { }
  virtual void Render(const MixedBehavProfile<T> &p_profile) = 0;
};

template <class T> 
class BehavStrategyNullRenderer : public BehavStrategyRenderer<T> {
public:
  virtual ~BehavStrategyNullRenderer() { }
  virtual void Render(const MixedBehavProfile<T> &p_profile) { }
};

template <class T>
class BehavStrategyCSVRenderer : public BehavStrategyRenderer<T> {
private:
  std::ostream &m_stream;

public:
  BehavStrategyCSVRenderer(std::ostream &p_stream)
    : m_stream(p_stream) { }
  virtual ~BehavStrategyCSVRenderer() { }
  virtual void Render(const MixedBehavProfile<T> &p_profile);
};

template <class T>
void BehavStrategyCSVRenderer<T>::Render(const MixedBehavProfile<T> &p_profile)
{
  m_stream << "NE,";
  for (int i = 1; i <= p_profile.Length(); i++) {
    m_stream << p_profile[i];
    if (i < p_profile.Length()) {
      m_stream << ',';
    }
  }
  m_stream << std::endl;
}

class NashEnumPureBehavSolver {
private:
  BehavStrategyRenderer<Rational> *m_onEquilibrium;

public:
  NashEnumPureBehavSolver(BehavStrategyRenderer<Rational> *p_onEquilibrium = 0);
  ~NashEnumPureBehavSolver()  { delete m_onEquilibrium; }
  List<MixedBehavProfile<Rational> > Solve(const BehavSupport &) const;
};

NashEnumPureBehavSolver::NashEnumPureBehavSolver(BehavStrategyRenderer<Rational> *p_onEquilibrium /* = 0 */)
  : m_onEquilibrium(p_onEquilibrium)
{
  if (!m_onEquilibrium) {
    m_onEquilibrium = new BehavStrategyNullRenderer<Rational>();
  }
}

List<MixedBehavProfile<Rational> > 
NashEnumPureBehavSolver::Solve(const BehavSupport &p_support) const
{
  List<MixedBehavProfile<Rational> > solutions;
  for (BehavIterator citer(p_support); !citer.AtEnd(); citer++) {
    if (citer->IsAgentNash()) {
      MixedBehavProfile<Rational> profile = citer->ToMixedBehavProfile();
      m_onEquilibrium->Render(profile);
      solutions.Append(profile);
    }
  }
  return solutions;
}

List<MixedBehavProfile<Rational> > 
SubsolveBehav(const BehavSupport &p_support)
{
  NashEnumPureBehavSolver algorithm = NashEnumPureBehavSolver();
  return algorithm.Solve(p_support);
}


void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Search for Nash equilibria in pure strategies\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 1994-2013, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS] [file]\n";
  std::cerr << "If file is not specified, attempts to read game from standard input.\n";
  std::cerr << "With no options, locates all Nash equilibria in pure strategies.\n\n";

  std::cerr << "Options:\n";
  std::cerr << "  -S               use strategic game (default)\n";
  std::cerr << "  -A               return agent form equilibria\n";
  std::cerr << "  -P               find only subgame-perfect equilibria\n";
  std::cerr << "  -h, --help       print this help message\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -v, --version    print version information\n";
  exit(1);
}


int main(int argc, char *argv[])
{
  opterr = 0;
  bool quiet = false, useStrategic = false, useAgent = false, bySubgames = false;

  int long_opt_index = 0;
  struct option long_options[] = {
    { "help", 0, NULL, 'h'   },
    { "version", 0, NULL, 'v'  },
    { 0,    0,    0,    0   }
  };
  int c;
  while ((c = getopt_long(argc, argv, "vhqASP", long_options, &long_opt_index)) != -1) {
    switch (c) {
    case 'v':
      PrintBanner(std::cerr); exit(1);
    case 'S':
      useStrategic = true;
      break;
    case 'A':
      useAgent = true;
      break;
    case 'P':
      bySubgames = true;
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

    if (game->IsTree())  {
      if (bySubgames) {
	List<MixedBehavProfile<Rational> > solutions;
	solutions = SolveBySubgames<Rational>(BehavSupport(game), 
					      &SubsolveBehav);
	for (int i = 1; i <= solutions.Length(); i++) {
	  PrintProfile(std::cout, solutions[i]);
	}
      }
      else if (useStrategic)  {
	NashEnumPureStrategySolver algorithm = NashEnumPureStrategySolver(new MixedStrategyCSVRenderer<Rational>(std::cout));
	algorithm.Solve(game);
      }
      else if (useAgent) {
	NashEnumPureBehavSolver algorithm = NashEnumPureBehavSolver(new BehavStrategyCSVRenderer<Rational>(std::cout));
	algorithm.Solve(game);
      }
      else {
	NashEnumPureStrategySolver algorithm = NashEnumPureStrategySolver(new MixedStrategyAsBehavCSVRenderer<Rational>(std::cout));
	algorithm.Solve(game);
      }
    }
    else {
      NashEnumPureStrategySolver algorithm = NashEnumPureStrategySolver(new MixedStrategyCSVRenderer<Rational>(std::cout));
      algorithm.Solve(game);
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





