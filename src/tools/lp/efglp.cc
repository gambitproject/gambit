//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/lp/efglp.cc
// Implementation of algorithm to solve efgs via linear programming
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

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include "libgambit/libgambit.h"

#include "tableau.h"
#include "lpsolve.h"

using namespace Gambit;

extern int g_numDecimals;

//
// Structure for caching data which might take a little time to compute
//
struct GameData {
  int ns1, ns2, ni1, ni2;
  Rational minpay;
  PVector<int> infosetIndex, infosetOffset;
  
  GameData(const Game &p_game)
    : infosetIndex(p_game->NumInfosets()), infosetOffset(p_game->NumInfosets())
  {
    infosetIndex = 0;
    infosetOffset = 0;
  }
};


//
// Recursively fills the constraint matrix A for the subtree rooted at 'n'.
//
template <class T>
void BuildConstraintMatrix(GameData &p_data,
			   const BehavSupport &p_support,
			   Matrix<T> &A, const GameNode &n, const T &prob,
			   int s1, int s2, int i1, int i2)
{
  GameOutcome outcome = n->GetOutcome();
  if (outcome) {
    A(s1,s2) += 
      (T) (Rational(prob) * outcome->GetPayoff<Rational>(1) - p_data.minpay);
  }

  if (n->NumChildren() == 0) {
    // Terminal node, recursion ends
    return;
  }

  if (n->GetPlayer()->IsChance()) {
    GameInfoset infoset = n->GetInfoset();
    for (int i = 1; i <= n->NumChildren(); i++) {
      BuildConstraintMatrix(p_data, p_support, A, n->GetChild(i),
			    prob * infoset->GetActionProb<T>(i),
			    s1, s2, i1, i2);
    }
  }
  else if (n->GetPlayer()->GetNumber() == 1) {
    i1 = p_data.infosetIndex(1, n->GetInfoset()->GetNumber());
    int snew = p_data.infosetOffset(1, n->GetInfoset()->GetNumber());
    A(s1, p_data.ns2+i1+1) = (T) 1;
    for (int i = 1; i <= p_support.NumActions(n->GetInfoset()); i++) {
      A(snew+i, p_data.ns2+i1+1) = (T) -1;
      BuildConstraintMatrix(p_data, p_support, A, 
			    n->GetChild(p_support.GetAction(n->GetInfoset(), i)->GetNumber()),
			    prob, snew+i, s2, i1, i2);
    }
  }
  else {  // Must be player 2
    i2 = p_data.infosetIndex(2, n->GetInfoset()->GetNumber());
    int snew = p_data.infosetOffset(2, n->GetInfoset()->GetNumber());
    A(p_data.ns1+i2+1, s2) = (T) -1;
    for (int i = 1; i <= p_support.NumActions(n->GetInfoset()); i++) {
      A(p_data.ns1+i2+1, snew+i) = (T) 1;
      BuildConstraintMatrix(p_data, p_support, A, 
			    n->GetChild(p_support.GetAction(n->GetInfoset(), i)->GetNumber()),
			    prob, s1, snew+i, i1, i2);
    }
  }
}

#ifdef UNUSED

//
// This function outputs the LP problem in CPLEX format to the specified
// stream.
//
template <class T>
void PrintCPLEX(std::ostream &p_stream,
		const Matrix<T> &A,
		const Vector<T> &b,
		const Vector<T> &c,
		int nequals)
{
  p_stream << "Minimize" << std::endl;
  for (int i = 1; i <= c.Length(); i++) {
    if (i > 1 && c[i] >= (T) 0) {
      p_stream << "+";
    }
    p_stream << c[i] << " x" << i << " ";
  }
  p_stream << std::endl;

  p_stream << std::endl << "Subject To" << std::endl;

  for (int j = 1; j <= b.Length(); j++) {
    for (int i = 1; i <= c.Length(); i++) {
      if (i > 1 && A(j, i) >= (T) 0) {
	p_stream << "+";
      }
      p_stream << A(j, i) << " x" << i << " ";
    }
    if (j + nequals > b.Length()) {
      p_stream << " = " << b[j] << std::endl;
    }
    else {
      p_stream << " >= " << b[j] << std::endl;
    }
  }

  p_stream << std::endl << "Bounds" << std::endl;
  for (int i = 1; i <= c.Length(); i++) {
    p_stream << "x" << i << " >= 0" << std::endl;
  }

  p_stream << "End" << std::endl;
}
#endif  // UNUSED


//
// The routine to actually solve the LP
// This routine takes an LP of the form
//    maximize c x subject to Ax>=b and x>=0,
// except the last 'nequals' constraints in A hold with equality.
// It expects the array p_primal to be the same length as the
// number of columns in A, and the routine returns the primal solution;
// similarly, the array p_dual should have the same length as the
// number of rows in A, and the routine returns the dual solution.
//
// To implement your own custom solver for this problem, simply
// replace this function.
//
template <class T> bool
SolveLP(const Matrix<T> &A, const Vector<T> &b, const Vector<T> &c,
	int nequals,
	Array<T> &p_primal, Array<T> &p_dual)
{
  LPSolve<T> LP(A, b, c, nequals);
  if (!LP.IsAborted()) {
    BFS<T> cbfs((T) 0);
    LP.OptBFS(cbfs);

    for (int i = 1; i <= A.NumColumns(); i++) {
      if (cbfs.IsDefined(i)) {
	p_primal[i] = cbfs(i);
      }
      else {
	p_primal[i] = (T) 0;
      }
    }

    for (int i = 1; i <= A.NumRows(); i++) {
      if (cbfs.IsDefined(-i)) {
	p_dual[i] = cbfs(-i);
      }
      else {
	p_dual[i] = (T) 0;
      }
    }
    return true;
  }
  else {
    return false;
  }
}

template <class T>
void PrintProfileDetail(std::ostream &p_stream,
			const MixedBehavProfile<T> &p_profile)
{
  char buffer[256];

  for (int pl = 1; pl <= p_profile.GetGame()->NumPlayers(); pl++) {
    GamePlayer player = p_profile.GetGame()->GetPlayer(pl);
    p_stream << "Behavior profile for player " << pl << ":\n";
    
    p_stream << "Infoset    Action     Prob          Value\n";
    p_stream << "-------    -------    -----------   -----------\n";

    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfoset infoset = player->GetInfoset(iset);

      for (int act = 1; act <= infoset->NumActions(); act++) {
	GameAction action = infoset->GetAction(act);

	if (infoset->GetLabel() != "") {
	  sprintf(buffer, "%7s    ", infoset->GetLabel().c_str());
	}
	else {
	  sprintf(buffer, "%7d    ", iset);
	}
	p_stream << buffer;
	
	if (action->GetLabel() != "") {
	  sprintf(buffer, "%7s    ", action->GetLabel().c_str());
	}
	else {
	  sprintf(buffer, "%7d   ", act);
	}
	p_stream << buffer;
	
	sprintf(buffer, "%11s   ", ToText(p_profile(pl, iset, act), g_numDecimals).c_str());
	p_stream << buffer;

	sprintf(buffer, "%11s   ", ToText(p_profile.GetActionValue(infoset->GetAction(act)), g_numDecimals).c_str());
	p_stream << buffer;

	p_stream << "\n";
      }
    }

    p_stream << "\n";
 
    p_stream << "Infoset    Node       Belief        Prob\n";
    p_stream << "-------    -------    -----------   -----------\n";

    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfoset infoset = player->GetInfoset(iset);
      
      for (int n = 1; n <= infoset->NumMembers(); n++) {
	sprintf(buffer, "%7d    ", iset);
	p_stream << buffer;

	sprintf(buffer, "%7d    ", n);
	p_stream << buffer;

	sprintf(buffer, "%11s   ", ToText(p_profile.GetBeliefProb(infoset->GetMember(n)), g_numDecimals).c_str());
	p_stream << buffer;

	sprintf(buffer, "%11s    ", ToText(p_profile.GetRealizProb(infoset->GetMember(n)), g_numDecimals).c_str());
	p_stream << buffer;

	p_stream << "\n";
      }
    }

    p_stream << "\n";
  }
}

void PrintProfile(std::ostream &p_stream,
		  const std::string &p_label,
		  const MixedBehavProfile<double> &p_profile)
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
		  const MixedBehavProfile<Rational> &p_profile)
{
  p_stream << p_label;
  for (int i = 1; i <= p_profile.Length(); i++) {
    p_stream << ',' << p_profile[i];
  }

  p_stream << std::endl;
}

//
// Sets the action probabilities at unreached information sets
// which are left undefined by the sequence form method to
// the centroid.  This is useful for the LiapValue, since that
// implements a penalty for having information sets where the
// action probabilities do not sum to one.
//
// This is really a hack; in the future, behavior profiles need to be
// "smarter" about the possibility they are not defined off the
// equilibrium path.
//
template <class T>
void UndefinedToCentroid(MixedBehavProfile<T> &p_profile)
{
  Game efg = p_profile.GetGame();

  for (int pl = 1; pl <= efg->NumPlayers(); pl++) {
    GamePlayer player = efg->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfoset infoset = player->GetInfoset(iset);
      
      if (p_profile.GetInfosetProb(infoset) > (T) 0) {
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
// Recursively construct the behavior profile from the sequence form
// solution represented by 'p_primal' (containing player 2's
// sequences) and 'p_dual' (containing player 1's sequences).
//
// Any information sets not reached with positive probability have
// their action probabilities set to zero.
//
template <class T> 
void GetBehavior(const GameData &p_data,
		 const BehavSupport &p_support,
		 MixedBehavProfile<T> &v,
		 const Array<T> &p_primal, const Array<T> &p_dual,
		 const GameNode &n,
		 int s1, int s2)
{
  if (n->NumChildren() == 0) {
    return;
  }

  if (n->GetPlayer()->IsChance()) {
    for(int i = 1; i <= n->NumChildren(); i++) {
      GetBehavior(p_data, p_support, v, p_primal, p_dual,
		  n->GetChild(i), s1, s2);
    }
  }
  else if (n->GetPlayer()->GetNumber() == 2) {
    int inf = p_data.infosetIndex(2, n->GetInfoset()->GetNumber());
    int snew = p_data.infosetOffset(2, n->GetInfoset()->GetNumber());
    for (int i = 1; i <= p_support.NumActions(n->GetInfoset()); i++) {
      if (p_primal[s1] > (T) 0) {
	v(2,inf,i) = p_primal[snew+i] / p_primal[s1];
      } 
      else {
	v(2,inf,i) = (T) 0;
      }
      GetBehavior(p_data, p_support, v, p_primal, p_dual,
		  n->GetChild(p_support.GetAction(n->GetInfoset(), i)->GetNumber()),
		  snew+i, s2);
    }
  }
  else {  // Must be player 1
    int inf = p_data.infosetIndex(1, n->GetInfoset()->GetNumber());
    int snew = p_data.infosetOffset(1, n->GetInfoset()->GetNumber());
    for (int i = 1; i <= p_support.NumActions(n->GetInfoset()); i++) {
      if (p_dual[s2] > (T) 0) {
	v(1,inf,i) = p_dual[snew+i] / p_dual[s2];
      }
      else {
	v(1,inf,i) = (T) 0;
      }
      GetBehavior(p_data, p_support, v, p_primal, p_dual,
		  n->GetChild(p_support.GetAction(n->GetInfoset(), i)->GetNumber()),
		  s1, snew+i);
    }
  }
}

//
// Convert the sequence form solution represented by the vectors
// (p_primal, p_dual) back to a behavior profile.
// Information sets not reached with positive probability have their
// probabilities set to the centroid
//
template <class T>
void SequenceToBehavior(const GameData &p_data,
			const BehavSupport &p_support,
			const Array<T> &p_primal, const Array<T> &p_dual)
{
  MixedBehavProfile<T> profile(p_support);
  GetBehavior(p_data, p_support,
	      profile, p_primal, p_dual,
	      p_support.GetGame()->GetRoot(), 1, 1);
  UndefinedToCentroid(profile);
  PrintProfile(std::cout, "NE", profile);
  //PrintProfileDetail(std::cout, profile);
}


//
// Compute and print one equilibrium by solving a linear program based
// on the sequence form representation of the game.
//
template <class T>
void SolveExtensive(const Game &p_game)
{
  BFS<T> cbfs((T) 0);
  
  BehavSupport support(p_game);

  // Cache some data for convenience
  GameData data(p_game);
  data.ns1 = support.NumSequences(1);
  data.ns2 = support.NumSequences(2);
  data.ni1 = support.GetGame()->GetPlayer(1)->NumInfosets()+1;  
  data.ni2 = support.GetGame()->GetPlayer(2)->NumInfosets()+1; 
  support.ReachableInfosets(p_game->GetRoot(), data.infosetIndex);
  for (int iset = 1, offset = 1, index = 1; iset < data.ni1; iset++) {
    if (data.infosetIndex(1, iset) > 0) {
      data.infosetOffset(1, iset) = offset;
      data.infosetIndex(1, iset) = index++;
      offset += support.NumActions(1, iset);
    }
  }
  for (int iset = 1, offset = 1, index = 1; iset < data.ni2; iset++) {
    if (data.infosetIndex(2, iset) > 0) {
      data.infosetOffset(2, iset) = offset;
      data.infosetIndex(2, iset) = index++;
      offset += support.NumActions(2, iset);
    }
  }
  data.minpay = p_game->GetMinPayoff();

  Matrix<T> A(1, data.ns1 + data.ni2, 1, data.ns2 + data.ni1);
  Vector<T> b(1, data.ns1 + data.ni2);
  Vector<T> c(1, data.ns2 + data.ni1);

  A = (T) 0;
  b = (T) 0;
  c = (T) 0;

  BuildConstraintMatrix(data, support, A, p_game->GetRoot(), 
			(T) 1, 1, 1, 0, 0);
  A(1, data.ns2 + 1) = (T) -1;
  A(data.ns1 + 1, 1) = (T) 1;

  b[data.ns1 + 1] = (T) 1;
  c[data.ns2 + 1] = (T) -1;

  Array<T> primal(A.NumColumns()), dual(A.NumRows());
  if (SolveLP(A, b, c, data.ni2, primal, dual)) {
    SequenceToBehavior(data, support, primal, dual);
  }
}

template void SolveExtensive<double>(const Game &);
template void SolveExtensive<Rational>(const Game &);
