//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of algorithm to solve extensive forms using linear
// complementarity program from sequence form
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

#include "lhtab.h"

#include "lemketab.h"

int g_numDecimals = 6;

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const Gambit::MixedBehavProfile<double> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream << "," << p_profile[i];
  }

  p_stream << std::endl;
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const Gambit::MixedBehavProfile<gbtRational> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(g_numDecimals) << p_profile[i];
  }

  p_stream << std::endl;
}

template <class T> class efgLcp {
private:
  int m_stopAfter, m_maxDepth;

  int ns1,ns2,ni1,ni2;
  T maxpay,eps;
  gbtList<BFS<T> > List;
  gbtList<Gambit::GameInfoset> isets1, isets2;

  void FillTableau(const Gambit::BehavSupport &, Gambit::Matrix<T> &, const Gambit::GameNode &, T,
		   int, int, int, int);
  int Add_BFS(const LTableau<T> &tab);
  int All_Lemke(const Gambit::BehavSupport &, int dup, LTableau<T> &B,
		int depth, Gambit::Matrix<T> &);
  
  void GetProfile(const Gambit::BehavSupport &, const LTableau<T> &tab, 
		  gbtDPVector<T> &, const gbtVector<T> &, 
		  const Gambit::GameNode &n, int,int);

public:
  efgLcp(void);
  virtual ~efgLcp();
  
  int StopAfter(void) const { return m_stopAfter; }
  void SetStopAfter(int p_stopAfter) { m_stopAfter = p_stopAfter; }

  int MaxDepth(void) const { return m_maxDepth; }
  void SetMaxDepth(int p_maxDepth) { m_maxDepth = p_maxDepth; }

  void Solve(const Gambit::BehavSupport &);
};


//---------------------------------------------------------------------------
//                        efgLcp: member functions
//---------------------------------------------------------------------------

template <class T>
efgLcp<T>::efgLcp(void)
  : m_stopAfter(0), m_maxDepth(0)
{ } 

template <class T> efgLcp<T>::~efgLcp()
{ }

//
// Sets the action probabilities at unreached information sets
// which are left undefined by the sequence form method to
// the centroid.  This helps IsNash and LiapValue work correctly.
//
template <class T>
void UndefinedToCentroid(Gambit::MixedBehavProfile<T> &p_profile)
{
  Gambit::Game efg = p_profile.GetGame();

  for (int pl = 1; pl <= efg->NumPlayers(); pl++) {
    Gambit::GamePlayer player = efg->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      Gambit::GameInfoset infoset = player->GetInfoset(iset);
      
      if (p_profile.GetIsetProb(infoset) > (T) 0) {
	continue;
      }
	  
      T total = (T) 0;
      for (int act = 1; act <= infoset->NumActions(); act++) {
	total += p_profile.GetActionProb(infoset->GetAction(act));
      }

      if (total == (T) 0) {
	for (int act = 1; act <= infoset->NumActions(); act++) {
	  p_profile(pl, iset, act) = (T) 1.0 / (T) infoset->NumActions();
	}
      }
    }
  }
}


//
// Lemke implements the Lemke's algorithm (as refined by Eaves 
// for degenerate problems) for  Linear Complementarity
// problems, starting from the primary ray.  
//

template <class T> 
void efgLcp<T>::Solve(const Gambit::BehavSupport &p_support)
{
  BFS<T> cbfs((T) 0);
  int i, j;
  
  if (p_support.GetGame()->NumPlayers() != 2) {
    return;
  }
  
  isets1 = p_support.ReachableInfosets(p_support.GetGame()->GetPlayer(1));
  isets2 = p_support.ReachableInfosets(p_support.GetGame()->GetPlayer(2));

  List = gbtList<BFS<T> >();

  int ntot;
  ns1 = p_support.NumSequences(1);
  ns2 = p_support.NumSequences(2);
  ni1 = p_support.GetGame()->GetPlayer(1)->NumInfosets()+1;
  ni2 = p_support.GetGame()->GetPlayer(2)->NumInfosets()+1;

  ntot = ns1+ns2+ni1+ni2;

  Gambit::Matrix<T> A(1,ntot,0,ntot);
  gbtVector<T> b(1,ntot);

  maxpay = p_support.GetGame()->GetMaxPayoff() + gbtRational(1);

  T prob = (T)1;
  for (i = A.MinRow(); i <= A.MaxRow(); i++) {
    b[i] = (T) 0;
    for (j = A.MinCol(); j <= A.MaxCol(); j++) {
      A(i,j) = (T) 0; 
    }
  }

  FillTableau(p_support, A, p_support.GetGame()->GetRoot(), prob, 1, 1, 0, 0);
  for (i = A.MinRow(); i <= A.MaxRow(); i++) { 
    A(i,0) = -(T) 1;
  }
  A(1,ns1+ns2+1) = (T)1;
  A(ns1+ns2+1,1) = -(T)1;
  A(ns1+1,ns1+ns2+ni1+1) = (T)1;
  A(ns1+ns2+ni1+1,ns1+1) = -(T)1;
  b[ns1+ns2+1] = -(T)1;
  b[ns1+ns2+ni1+1] = -(T)1;

  LTableau<T> tab(A,b);
  eps = tab.Epsilon();
  
  Gambit::MixedBehavProfile<T> profile(p_support);
  gbtVector<T> sol(tab.MinRow(),tab.MaxRow());
    
  try {
    if (m_stopAfter != 1) {
      All_Lemke(p_support, ns1+ns2+1, tab, 0, A);
    }
    else {
      tab.Pivot(ns1+ns2+1,0);
      tab.SF_LCPPath(ns1+ns2+1);
      
      Add_BFS(tab);
      tab.BasisVector(sol);
      GetProfile(p_support, tab, 
		 profile.GetDPVector(),sol,p_support.GetGame()->GetRoot(),1,1);
      UndefinedToCentroid(profile);

      PrintProfile(std::cout, "NE", profile);
    }
  }
  catch (...) {
    // catch exception; return solutions computed (if any)
  }
}

template <class T> int efgLcp<T>::Add_BFS(const LTableau<T> &tableau)
{
  BFS<T> cbfs((T) 0);
  gbtVector<T> v(tableau.MinRow(), tableau.MaxRow());
  tableau.BasisVector(v);

  for (int i = tableau.MinCol(); i <= tableau.MaxCol(); i++)
    if (tableau.Member(i)) {
      cbfs.Define(i, v[tableau.Find(i)]);
    }

  if (List.Contains(cbfs))  return 0;
  List.Append(cbfs);
  return 1;
}

//
// All_Lemke finds all accessible Nash equilibria by recursively 
// calling itself.  List maintains the list of basic variables 
// for the equilibria that have already been found.  
// From each new accessible equilibrium, it follows
// all possible paths, adding any new equilibria to the List.  
//
template <class T> int 
efgLcp<T>::All_Lemke(const Gambit::BehavSupport &p_support,
		     int j, LTableau<T> &B, int depth,
		     Gambit::Matrix<T> &A)
{
  if (m_maxDepth != 0 && depth > m_maxDepth) {
    return 1;
  }

  int i,len,newsol,missing;
  T p1,p2,aa;
  T small_num = (T)1/(T)1000;

  gbtVector<T> sol(B.MinRow(),B.MaxRow());
  Gambit::MixedBehavProfile<T> profile(p_support);

  newsol =0;
  for (i = B.MinRow(); 
       i <= B.MaxRow()  && newsol == 0;
       i++) {
    if (i != j)  {
      len=List.Length();  
      p1=(double)len/(double)(len+1);
      p2=(double)(len+1)/(double)(len+2);
      int num_strats = B.MaxCol()-B.MinCol()-1;
      aa=(double)(i)/(double)num_strats;
      
      LTableau<T> BCopy(B);
      A(i,0)=-small_num;
      BCopy.Refactor();

      if(depth==0) {
	BCopy.Pivot(j,0);
	missing = -j;
      }
      else
	missing = BCopy.SF_PivotIn(0);

      assert(missing);
      newsol=0;

      if(BCopy.SF_LCPPath(-missing)==1) {
	newsol = Add_BFS(BCopy);
	BCopy.BasisVector(sol);
	GetProfile(p_support, BCopy, profile.GetDPVector(),sol,p_support.GetGame()->GetRoot(),1,1);
	UndefinedToCentroid(profile);
	if (newsol) {
	  PrintProfile(std::cout, "NE", profile);
	}
      }
      else {
	// gout << ": Dead End";
      }
      
      A(i,0)=-(T)1;
      if(newsol) {
	BCopy.Refactor();
	All_Lemke(p_support,i,BCopy,depth+1, A);
      }
    }
  }
  
  return 1;
}

template <class T>
void efgLcp<T>::FillTableau(const Gambit::BehavSupport &p_support, Gambit::Matrix<T> &A,
			    const Gambit::GameNode &n, T prob,
			    int s1, int s2, int i1, int i2)
{
  int snew;
  if (n->GetOutcome()) {
    A(s1,ns1+s2) = gbtRational(A(s1,ns1+s2)) +
      gbtRational(prob) * (n->GetOutcome()->GetPayoff(1) - gbtRational(maxpay));
    A(ns1+s2,s1) = gbtRational(A(ns1+s2,s1)) +
      gbtRational(prob) * (n->GetOutcome()->GetPayoff(2) - gbtRational(maxpay));
  }
  if (n->GetInfoset()) {
    if (n->GetPlayer()->IsChance()) {
      for (int i = 1; i <= n->NumChildren(); i++) {
	FillTableau(p_support, A, n->GetChild(i),
		    gbtRational(prob) * n->GetInfoset()->GetActionProb(i),
		    s1,s2,i1,i2);
      }
    }
    int pl = n->GetPlayer()->GetNumber();
    if (pl==1) {
      i1=isets1.Find(n->GetInfoset());
      snew=1;
      for (int i = 1; i < i1; i++) {
	snew+=p_support.NumActions(isets1[i]);
      }
      A(s1,ns1+ns2+i1+1) = -(T)1;
      A(ns1+ns2+i1+1,s1) = (T)1;
      for (int i = 1; i <= p_support.NumActions(n->GetInfoset()); i++) {
	A(snew+i,ns1+ns2+i1+1) = (T)1;
	A(ns1+ns2+i1+1,snew+i) = -(T)1;
	FillTableau(p_support, A, n->GetChild(p_support.Actions(n->GetInfoset())[i]->GetNumber()),prob,snew+i,s2,i1,i2);
      }
    }
    if(pl==2) {
      i2=isets2.Find(n->GetInfoset());
      snew=1;
      for (int i = 1; i < i2; i++) {
	snew+=p_support.NumActions(isets2[i]);
      }
      A(ns1+s2,ns1+ns2+ni1+i2+1) = -(T)1;
      A(ns1+ns2+ni1+i2+1,ns1+s2) = (T)1;
      for (int i = 1; i <= p_support.NumActions(n->GetInfoset()); i++) {
	A(ns1+snew+i,ns1+ns2+ni1+i2+1) = (T)1;
	A(ns1+ns2+ni1+i2+1,ns1+snew+i) = -(T)1;
	FillTableau(p_support, A, n->GetChild(p_support.Actions(n->GetInfoset())[i]->GetNumber()),prob,s1,snew+i,i1,i2);
      }
    }
    
  }
}


template <class T>
void efgLcp<T>::GetProfile(const Gambit::BehavSupport &p_support,
				  const LTableau<T> &tab, 
				  gbtDPVector<T> &v, const gbtVector<T> &sol,
				  const Gambit::GameNode &n, int s1,int s2)
{
  int i,pl,inf,snew,ind,ind2;
  if(n->GetInfoset()) {
    if(n->GetPlayer()->IsChance()) {
      for (i = 1; i <= n->NumChildren();i++)
	GetProfile(p_support, tab, v,sol,n->GetChild(i),s1,s2);
    }
    pl = n->GetPlayer()->GetNumber();
    if(pl==1) {
      inf= isets1.Find(n->GetInfoset());
      snew=1;
      for(i=1;i<inf;i++)
	snew+=p_support.NumActions(isets1[i]); 
      for(i=1;i<=p_support.NumActions(n->GetInfoset());i++) {
	v(pl,inf,i) = (T)0;
	if(tab.Member(s1)) {
	  ind = tab.Find(s1);
	  if(sol[ind]>eps) {
	    if(tab.Member(snew+i)) {
	      ind2 = tab.Find(snew+i);
	      if(sol[ind2]>eps)
		v(pl,inf,i) = sol[ind2]/sol[ind];
	    }
	  } 
	} 
	GetProfile(p_support, tab, v,sol,n->GetChild(p_support.Actions(n->GetInfoset())[i]->GetNumber()),snew+i,s2);
      }
    }
    if(pl==2) {
      inf= isets2.Find(n->GetInfoset());
      snew=1;
      for(i=1;i<inf;i++)
	snew+=p_support.NumActions(isets2[i]); 
      for(i=1;i<=p_support.NumActions(n->GetInfoset());i++) {
	v(pl,inf,i) = (T)0;
	if(tab.Member(ns1+s2)) {
	  ind = tab.Find(ns1+s2);
	  if(sol[ind]>eps) {
	    if(tab.Member(ns1+snew+i)) {
	      ind2 = tab.Find(ns1+snew+i);
	      if(sol[ind2]>eps)
		v(pl,inf,i) = sol[ind2]/sol[ind];
	    }
	  } 
	} 
	GetProfile(p_support, tab, v,sol,n->GetChild(p_support.Actions(n->GetInfoset())[i]->GetNumber()),s1,snew+i);
      }
    }
  }
}

void PrintBanner(std::ostream &p_stream)
{
  p_stream << "Compute Nash equilibria by solving a linear complementarity program\n";
  p_stream << "Gambit version " VERSION ", Copyright (C) 2005, The Gambit Project\n";
  p_stream << "This is free software, distributed under the GNU GPL\n\n";
}

void PrintHelp(char *progname)
{
  PrintBanner(std::cerr);
  std::cerr << "Usage: " << progname << " [OPTIONS]\n";
  std::cerr << "Accepts extensive game on standard input.\n";
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

  Gambit::Game efg;

  try {
    efg = Gambit::ReadEfg(std::cin);
  }
  catch (...) {
    return 1;
  }

  if (efg->NumPlayers() != 2) {
    return 1;
  }

  if (useFloat) {
    efgLcp<double> algorithm;
    algorithm.Solve(efg);
  }
  else {
    efgLcp<gbtRational> algorithm;
    algorithm.Solve(efg);
  }

  return 0;
}
