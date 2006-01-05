//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria via Mangasarian's algorithm
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
#include <iomanip>

#include "vertenum.imp"
#include "tableau.imp"

#include "libgambit/libgambit.h"

int g_numDecimals = 6;

template <class T> bool EqZero(const T &x)
{
  T eps;
  gEpsilon(eps, 12);
  return (x <= eps && x >= -eps);
}     

int m_stopAfter = 0;

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const gbtMixedProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << ',' << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const gbtMixedProfile<gbtRational> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream << ',' << p_profile[i];
  }

  p_stream << std::endl;
}

template <class T> void Solve(Gambit::Game p_nfg, const T &)
{
  gbtList<gbtVector<T> > key1, key2;  
  gbtList<int> node1, node2;   // IDs of each component of the extreme equilibria

  gbtStrategyProfile profile(p_nfg);

  gbtRational min = p_nfg->GetMinPayoff();
  if (min > gbtRational(0)) {
    min = gbtRational(0);
  }
  min -= gbtRational(1);

  gbtRational max = p_nfg->GetMaxPayoff();
  if (max < gbtRational(0)) {
    max = gbtRational(0);
  }

  gbtRational fac(1, max - min);

  // Construct matrices A1, A2
  Gambit::Matrix<T> A1(1, p_nfg->GetPlayer(1)->NumStrategies(), 
		  1, p_nfg->GetPlayer(2)->NumStrategies());
  Gambit::Matrix<T> A2(1, p_nfg->GetPlayer(2)->NumStrategies(),
		  1, p_nfg->GetPlayer(1)->NumStrategies());

  for (int i = 1; i <= p_nfg->GetPlayer(1)->NumStrategies(); i++) {
    profile.SetStrategy(p_nfg->GetPlayer(1)->GetStrategy(i));
    for (int j = 1; j <= p_nfg->GetPlayer(2)->NumStrategies(); j++) {
      profile.SetStrategy(p_nfg->GetPlayer(2)->GetStrategy(j));
      A1(i, j) = fac * (profile.GetPayoff(1) - min);
      A2(j, i) = fac * (profile.GetPayoff(2) - min);
    }
  }

  // Construct vectors b1, b2
  gbtVector<T> b1(1, p_nfg->GetPlayer(1)->NumStrategies());
  gbtVector<T> b2(1, p_nfg->GetPlayer(2)->NumStrategies());
  b1 = (T) -1;
  b2 = (T) -1;

  // enumerate vertices of A1 x + b1 <= 0 and A2 x + b2 <= 0
  VertEnum<T> poly1(A1, b1);
  VertEnum<T> poly2(A2, b2);

  const gbtList<BFS<T> > &verts1(poly1.VertexList());
  const gbtList<BFS<T> > &verts2(poly2.VertexList());
  int v1 = verts1.Length();
  int v2 = verts2.Length();

  gbtArray<int> vert1id(v1);
  gbtArray<int> vert2id(v2);
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
	for (int k = 1; nash && k <= p_nfg->GetPlayer(1)->NumStrategies(); k++) {
	  if (bfs1.IsDefined(k) && bfs2.IsDefined(-k)) {
	    nash = nash && EqZero(bfs1(k) * bfs2(-k));
	  }
	}

	for (int k = 1; nash && k <= p_nfg->GetPlayer(2)->NumStrategies(); k++) {
	  if (bfs2.IsDefined(k) && bfs1.IsDefined(-k)) {
	    nash = nash && EqZero(bfs2(k) * bfs1(-k));
	  }
	}

	if (nash) {
	  gbtMixedProfile<T> profile(p_nfg);
	  T sum = (T) 0;
	  for (int k = 1; k <= p_nfg->GetPlayer(1)->NumStrategies(); k++) {
	    profile(1, k) = (T) 0;
	    if (bfs1.IsDefined(k)) {
	      profile(1,k) = -bfs1(k);
	      sum += profile(1,k);
	    }
	  } 
	  
	  for (int k = 1; k <= p_nfg->GetPlayer(1)->NumStrategies(); k++) {
	    if (bfs1.IsDefined(k)) { 
	      profile(1,k) /= sum;
	    }
	  }
	  
	  sum = (T) 0;
	  for (int k = 1; k <= p_nfg->GetPlayer(2)->NumStrategies(); k++) {
	    profile(2,k) = (T) 0;
	    if (bfs2.IsDefined(k)) {
	      profile(2,k) =-bfs2(k);
	      sum += profile(2,k);
	    }
	  } 
	  
	  for (int k = 1; k <= p_nfg->GetPlayer(2)->NumStrategies(); k++) {
	    if (bfs2.IsDefined(k)) { 
	      profile(2,k) /= sum;
	    }
	  }

	  PrintProfile(std::cout, "NE", profile);
	  
	  // note: The keys give the mixed strategy associated with each node. 
	  //       The keys should also keep track of the basis
	  //       As things stand now, two different bases could lead to
	  //       the same key... BAD!
	  if (vert1id[i1] == 0) {
	    id1++;
	    vert1id[i1] = id1;
	    key2.Append(profile.GetRow(2));
	  }
	  if (vert2id[i2] == 0) {
	    id2++;
	    vert2id[i2] = id2;
	    key1.Append(profile.GetRow(1));
	  }
	  node1.Append(vert2id[i2]);
	  node2.Append(vert1id[i1]);
	}
      }
    }
  }
  catch (...) {
    // stop processing, just return equilibria found (if any)
  }
}

#ifdef UNUSED
template <class T> void EnumModule<T>::GetCliques(gOutput &p_stream) const
{
  int n = node1.Length();
  assert(node2.Length() == n);

  gbtArray<edge> edgelist(n);

  p_stream << "\nKey:\nPlayer 1:";
  for (int i = 1; i <= key1.Length(); i++) {
    p_stream << "\n" << i << ": " << key1[i];
  }
  p_stream << "\nPlayer 2:";
  for (int i = 1; i <= key2.Length(); i++) {
    p_stream << "\n" << i << ": " << key2[i];
  }
  p_stream << "\nExtreme equilibria:";
  for (int i = 1; i <= n; i++) {
    edgelist[i].node1 = node1[i];
    edgelist[i].node2 = node2[i];
    p_stream << "\n" << node1[i] << " " << node2[i];
  }
  EnumCliques clique(edgelist, v2+1, v1+1);
}

#endif  // UNUSED

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by enumerating extreme points\n";
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

  Gambit::Game nfg;

  try {
    nfg = Gambit::ReadNfg(std::cin);
  }
  catch (...) {
    return 1;
  }

  if (nfg->NumPlayers() != 2) {
    return 1;
  }

  try {
    if (useFloat) {
      Solve(nfg, 0.0);
    }
    else {
      Solve(nfg, gbtRational(0));
    }
    return 0;
  }
  catch (...) {
    return 1;
  }
}










