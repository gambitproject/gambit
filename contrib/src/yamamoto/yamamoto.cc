//
// $Source: /cvsroot/gambit/libgambit/libgambit/nash/nfgyamamoto.cc,v $
// $Date: 2005/04/26 01:19:10 $
// $Revision: 1.2 $
//
// DESCRIPTION:
// Yamamoto's algorithm for computing one proper equilibrium
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

#include <iostream>
#include <sstream>
#include <libgambit/libgambit.h>
#include <libgambit/sqmatrix.h>

using namespace Gambit;

int g_numDecimals = 6;

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const MixedStrategyProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(g_numDecimals) << ((double) p_profile[i]);
  }

  p_stream << std::endl;
}

int NumMembers(const Matrix<int> &p_partition, int p_index)
{
  int count = 0;
  
  for (int col = 1; col <= p_partition.NumColumns(); col++) {
    if (p_partition(p_index, col) > 0) {
      count++;
    }
  }

  return count;
}

static int FirstMember(const Matrix<int> &p_partition, int p_index)
{
  for (int col = 1; col <= p_partition.NumColumns(); col++) {
    if (p_partition(p_index, col) > 0) {
      return col;
    }
  }

  // shouldn't happen!
  return 0;
}

static double Payoff(const MixedStrategyProfile<double> &p_profile, 
		     int p_player,
		     const Matrix<int> &p_partition, int p_index)
{
  for (int st = 1; st <= p_profile.GetGame()->GetPlayer(p_player)->NumStrategies(); st++) {
    if (p_partition(p_index, st) > 0) {
      return p_profile.GetStrategyValue(p_profile.GetGame()->GetPlayer(p_player)->GetStrategy(st));
    }
  }
  
  // shouldn't happen!
  return 0.0;
}


Matrix<int> RankStrategies(const MixedStrategyProfile<double> &p_profile,
			   int p_player)
{
  Vector<double> payoffs(p_profile.GetGame()->GetPlayer(p_player)->NumStrategies());
  Array<int> strategies(p_profile.GetGame()->GetPlayer(p_player)->NumStrategies());
  for (int st = 1; st <= payoffs.Length(); st++) {
    p_profile.GetStrategyValue(p_profile.GetGame()->GetPlayer(p_player)->GetStrategy(st));
  }

  for (int i = 1; i <= strategies.Length(); i++) {
    strategies[i] = i;
  }

  // bubble sort; crude but effective
  bool changed;

  do { 
    changed = false;
    
    for (int i = 1; i <= strategies.Length() - 1; i++) {
      if (payoffs[i] < payoffs[i+1]) {
	double tmp1 = payoffs[i];
	payoffs[i] = payoffs[i+1];
	payoffs[i+1] = tmp1;

	int tmp2 = strategies[i];
	strategies[i] = strategies[i+1];
	strategies[i+1] = tmp2;

	changed = true;
      }
    }
  } while (changed);

  Matrix<int> partition(strategies.Length(), strategies.Length());
  for (int row = 1; row <= partition.NumRows(); row++) {
    for (int col = 1; col <= partition.NumColumns(); col++) {
      partition(row, col) = 0;
    }
  }

  // FIXME: This assumes that no strategies have the same payoff
  for (int row = 1; row <= strategies.Length(); row++) {
    partition(row, strategies[row]) = 1;
  } 

  return partition;
}

void YamamotoJacobian(const MixedStrategyProfile<double> &p_profile,
		      double p_lambda,
		      const List<Matrix<int> > &p_partition,
		      Matrix<double> &p_matrix)
{
  int rowno = 0;
  p_matrix = 0.0;

  for (int pl = 1; pl <= p_profile.GetGame()->NumPlayers(); pl++) {
    int strats = 0;
    for (int part = 1; part <= p_partition[pl].NumRows(); part++) {
      if (NumMembers(p_partition[pl], part) > 0) {
	// First, the probability equation
	rowno++;

	int colno = 0;
	for (int pl2 = 1; pl2 <= p_profile.GetGame()->NumPlayers(); pl2++) {
	  for (int st2 = 1; st2 <= p_profile.GetGame()->GetPlayer(pl2)->NumStrategies(); st2++) {
	    colno++;
	    if (pl != pl2) {
	      continue;
	    }

	    if (p_partition[pl](part, st2) > 0) {
	    // strategy number st2 is in this partition
	      for (int i = 1; i <= p_profile.GetGame()->GetPlayer(pl)->NumStrategies(); i++) {
		p_matrix(rowno, colno) += pow(p_lambda, (double) (i-1));
	      }
	    }
	  }
	}

	// The final column is the derivative wrt lambda
	colno++;
	double totalprob = 0.0;
	for (int st = 1; st <= p_profile.GetGame()->GetPlayer(pl)->NumStrategies(); st++) {
	  if (p_partition[pl](part, st) > 0) {
	    totalprob += p_profile[p_profile.GetGame()->GetPlayer(pl)->GetStrategy(st)];
	  }
	}

	for (int i = 1; i <= p_profile.GetGame()->GetPlayer(pl)->NumStrategies(); i++) {
	  p_matrix(rowno, colno) += ((double) (i - 1)) * pow(p_lambda, (double) (i-2)) * totalprob;
	}

	for (int st = strats + 1; st <= strats + NumMembers(p_partition[pl], part); st++) {
	  p_matrix(rowno, colno) -= ((double) st - 1) * pow(p_lambda, (double) (st - 2));
	}
      }

      if (NumMembers(p_partition[pl], part) > 1) {
	// We need to have #members - 1 equations
	int st1 = FirstMember(p_partition[pl], part);

	for (int st = st1 + 1; st <= p_profile.GetGame()->GetPlayer(pl)->NumStrategies(); st++) {
	  if (p_partition[pl](part, st) > 0) {
	    rowno++;

	    int colno = 0;
	    for (int pl2 = 1; pl2 <= p_profile.GetGame()->NumPlayers(); pl2++) {
	      for (int st2 = 1; st2 <= p_profile.GetGame()->GetPlayer(pl2)->NumStrategies(); st2++) {
		colno++;
		if (pl == pl2) {
		  continue;
		}

		p_matrix(rowno, colno) = 
		  p_profile.GetPayoffDeriv(pl, p_profile.GetGame()->GetPlayer(pl)->GetStrategy(st1), p_profile.GetGame()->GetPlayer(pl2)->GetStrategy(st2)) - p_profile.GetPayoffDeriv(pl, p_profile.GetGame()->GetPlayer(pl)->GetStrategy(st), p_profile.GetGame()->GetPlayer(pl2)->GetStrategy(st2));
	      }
	    }
	  }
	}
      }
      strats += NumMembers(p_partition[pl], part);
    }
  }
}
		      
void YamamotoComputeStep(const MixedStrategyProfile<double> &p_profile,
			 const Matrix<double> &p_matrix,
			 PVector<double> &p_delta, double &p_lambdainc,
			 double p_initialsign, double p_stepsize)
{
  double sign = p_initialsign;
  int rowno = 0; 

  SquareMatrix<double> M(p_matrix.NumRows());

  for (int row = 1; row <= M.NumRows(); row++) {
    for (int col = 1; col <= M.NumColumns(); col++) {
      M(row, col) = p_matrix(row, col + 1);
    }
  }

  for (int pl = 1; pl <= p_profile.GetGame()->NumPlayers(); pl++) {
    for (int st = 1; st <= p_profile.GetGame()->GetPlayer(pl)->NumStrategies(); st++) {
      rowno++;
      p_delta(pl, st) = sign * M.Determinant();   
      sign *= -1.0;

      for (int row = 1; row <= M.NumRows(); row++) {
	M(row, rowno) = p_matrix(row, rowno);
	if (rowno < M.NumColumns()) {
	  M(row, rowno + 1) = p_matrix(row, rowno + 2);
	}
      }
    }
  }   

  p_lambdainc = sign * M.Determinant();

  double norm = 0.0;
  for (int pl = 1; pl <= p_profile.GetGame()->NumPlayers(); pl++) {
    for (int st = 1; st <= p_profile.GetGame()->GetPlayer(pl)->NumStrategies(); st++) {
      norm += p_delta(pl, st) * p_delta(pl, st);
    }
  }
  norm += p_lambdainc * p_lambdainc; 
  
  for (int pl = 1; pl <= p_profile.GetGame()->NumPlayers(); pl++) {
    for (int st = 1; st <= p_profile.GetGame()->GetPlayer(pl)->NumStrategies(); st++) {
      p_delta(pl, st) /= sqrt(norm / p_stepsize);
    }
  }

  p_lambdainc /= sqrt(norm / p_stepsize);
}

List<int> SortProbs(const MixedStrategyProfile<double> &p_profile,
		    int p_player,
		    const Matrix<int> &p_partition,
		    int p_index)
{
  List<int> strategies;
  List<double> probs;

  for (int st = 1; st <= p_partition.NumColumns(); st++) {
    if (p_partition(p_index, st) > 0) {
      strategies.Append(st);
      probs.Append(p_profile[p_profile.GetGame()->GetPlayer(p_player)->GetStrategy(st)]);
    }
  }

  bool changed;

  do { 
    changed = false;
    
    for (int i = 1; i <= strategies.Length() - 1; i++) {
      if (probs[i] < probs[i+1]) {
	double tmp1 = probs[i];
	probs[i] = probs[i+1];
	probs[i+1] = tmp1;

	int tmp2 = strategies[i];
	strategies[i] = strategies[i+1];
	strategies[i+1] = tmp2;

	changed = true;
      }
    }
  } while (changed);

  return strategies;
}

double PDenom(double p_lambda, int p_m)
{
  double total = 0.0;

  for (int i = 1; i <= p_m; i++) {
    total += pow(p_lambda, (double) (i - 1));
  }

  return total;
}

void Solve(const Game &p_game)
{
  // In the notation of Yamamoto's paper, profile(i,j)=x_{ij}
  // and lambda=t
  MixedStrategyProfile<double> profile(p_game);
  double lambda = 1.0;
  double initialsign = -1.0; 
  List<Matrix<int> > partitions;
  Matrix<double> H(p_game->MixedProfileLength(),
		   p_game->MixedProfileLength() + 1);

  for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
    partitions.Append(RankStrategies(profile, pl));
  }

  for (int step = 1; step <= 50000 && lambda > 0.01; step++) { 
    YamamotoJacobian(profile, lambda, partitions, H);
      
    PVector<double> delta(profile.GetGame()->NumStrategies());
    for (int i = 1; i <= delta.Length(); i++) {
      delta[i] = profile[i];
    }
    double lambdainc;

    YamamotoComputeStep(profile, H, delta, lambdainc, initialsign, .000001);

    for (int i = 1; i <= delta.Length(); i++) {
      profile[i] += delta[i];
    }
    lambda += lambdainc;

    PrintProfile(std::cout, ToText(lambda), profile);
    // Check for inequalities
    for (int pl = 1; pl <= p_game->NumPlayers(); pl++) {
      int strats = 0;
      for (int part = 1; part < p_game->GetPlayer(pl)->NumStrategies(); part++) {
	if (NumMembers(partitions[pl], part) > 0 &&
	    NumMembers(partitions[pl], part + 1) > 0) {
	  if (Payoff(profile, pl, partitions[pl], part) <=
	      Payoff(profile, pl, partitions[pl], part + 1)) {
	    // Combine partitions part and part+1
	    partitions[pl].SetRow(part, partitions[pl].Row(part) + partitions[pl].Row(part+1));
	    for (int p = part + 1; p < p_game->GetPlayer(pl)->NumStrategies(); p++) {
	      partitions[pl].SetRow(p, partitions[pl].Row(p+1));
	    }  
	    for (int col = 1; col <= partitions[pl].NumColumns(); col++) {
	      partitions[pl](partitions[pl].NumRows(), col) = 0;
	    }
	    // Redo this partition, in case multiple partitions 
	    // coalesce (non-generic, but conceivable)
	    // part--;
	    //    gout << "Relax!\n";
	    // Working hypothesis: need to flip the sign at every relaxation
	    initialsign *= -1.0;
	  }
	  else if (NumMembers(partitions[pl], part) > 1) {
	    // check for possible splitting of partition
	    List<int> sortstrats = SortProbs(profile, pl, partitions[pl], 
					      part);
	    double totX = 0.0, totP = 0.0;
	    for (int i = 1; i < sortstrats.Length(); i++) {
	      totP += pow(lambda, (double) (strats + i - 1)) / PDenom(lambda, p_game->GetPlayer(pl)->NumStrategies());
	      totX += profile[profile.GetGame()->GetPlayer(pl)->GetStrategy(sortstrats[i])];
	      if (totX >= totP) {
		//		gout << pl << " " << part << " " << totP << " " << totX << "Xaler!\n";
		for (int p = part + 1; p < p_game->GetPlayer(pl)->NumStrategies(); p++) {
		  partitions[pl].SetRow(p+1, partitions[pl].Row(p));
		}  
		for (int col = 1; col <= partitions[pl].NumColumns(); col++) {
		  partitions[pl](part, col) = 0;
		  partitions[pl](part + 1, col) = 0;
		}
		for (int st = 1; st <= i; st++) {
		  partitions[pl](part, st) = 1;
		}
		for (int st = i + 1; st <= sortstrats.Length(); st++) {
		  partitions[pl](part + 1, st) = 1;
		}
		//		initialsign *= -1.0;
		break;
	      }
	    }
	  }
	}
	strats += NumMembers(partitions[pl], part);
      }
    } 
  }

  PrintProfile(std::cout, "NE", profile);
}

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute one proper Nash equilibrium using a tracing procedure\n";
  //  p_stream << "Gambit version " VERSION ", Copyright (C) 2006, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}


void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS]\n";
  std::cerr << "Accepts game on standard input.\n";
  std::cerr << "With no options, computes one proper Nash equilibrium.\n\n";
/*
  std::cerr << "Options:\n";
  std::cerr << "  -d DECIMALS      show equilibria as floating point, with DECIMALS digits\n";
  std::cerr << "                   (default is to show as rational numbers)\n";
  std::cerr << "  -g MULT          granularity of grid refinement at each step (default is 2)\n";
  std::cerr << "  -h               print this help message\n";
  std::cerr << "  -r DENOM         generate random starting points with denominator DENOM\n";
  std::cerr << "  -n COUNT         number of starting points to generate (requires -r)\n";
  std::cerr << "  -s FILE          file containing starting points\n";
  std::cerr << "  -q               quiet mode (suppresses banner)\n";
  std::cerr << "  -v               verbose mode (shows intermediate output)\n";
  std::cerr << "                   (default is to only show equilibria)\n";
*/
  exit(1);
}

int main(int argc, char *argv[])
{
  opterr = 0;
  bool quiet = false;

  int c;
  while ((c = getopt(argc, argv, "h")) != -1) {
    switch (c) {
    case 'h':
      PrintHelp(argv[0]);
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

  try {
    Gambit::Game game = Gambit::ReadGame(std::cin);

    game->BuildComputedValues();

    Solve(game);
  }
  catch (Gambit::InvalidFileException) {
    std::cerr << "Error: Game not in a recognized format.\n";
    return 1;
  }
  catch (...) {
    std::cerr << "Error: An internal error occurred.\n";
    return 1;
  }
}
