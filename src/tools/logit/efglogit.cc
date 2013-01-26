//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/logit/efglogit.cc
// Computation of agent quantal response equilibrium correspondence for
// extensive games.
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

#include <cmath>
#include <iostream>
#include <libgambit/libgambit.h>
#include "logbehav.imp"

using namespace Gambit;

#include "efglogit.h"

//------------------------------------------------------------------------------
//                        Classes representing equations
//------------------------------------------------------------------------------

//
// This abstract base class represents one of the defining equations of the system.
//
class Equation {
public:
  virtual ~Equation() { }

  virtual double Value(const LogBehavProfile<double> &p_point,
		       double p_lambda) = 0;
  virtual void Gradient(const LogBehavProfile<double> &p_point, 
			double p_lambda,
			Vector<double> &p_gradient) = 0;
};

//
// This class represents the equation that the probabilities of actions
// in information set (pl,iset) sums to one
//
class SumToOneEquation : public Equation {
private:
  Game m_game;
  int m_pl, m_iset;
  GameInfoset m_infoset;

public:
  SumToOneEquation(Game p_game, int p_player, int p_infoset)
    : m_game(p_game), m_pl(p_player), m_iset(p_infoset),
      m_infoset(p_game->GetPlayer(p_player)->GetInfoset(p_infoset))
  { }

  double Value(const LogBehavProfile<double> &p_profile,
	       double p_lambda);
  void Gradient(const LogBehavProfile<double> &p_profile, double p_lambda,
		Vector<double> &p_gradient);
};



double SumToOneEquation::Value(const LogBehavProfile<double> &p_profile,
			       double p_lambda)
{
  double value = -1.0;
  for (int act = 1; act <= m_infoset->NumActions(); act++) {
    value += p_profile.GetProb(m_pl, m_iset, act);
  }
  return value;
}

void SumToOneEquation::Gradient(const LogBehavProfile<double> &p_profile,
				double p_lambda,
				Vector<double> &p_gradient)
{
  int i = 1;
  for (int pl = 1; pl <= m_game->NumPlayers(); pl++) {
    GamePlayer player = m_game->GetPlayer(pl);

    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfoset infoset = player->GetInfoset(iset);

      for (int act = 1; act <= infoset->NumActions(); act++, i++) {
	if (pl == m_pl && iset == m_iset) {
	  p_gradient[i] = p_profile.GetProb(pl, iset, act);
	}
	else {
	  p_gradient[i] = 0.0;
	}
      }
    }
  }

  // Derivative wrt lambda is zero
  p_gradient[i] = 0.0;
}
			       

//
// This class represents the equation relating the probability of 
// playing action (pl,iset,act) to the probability of playing action
// (pl,iset,1)
//
class RatioEquation : public Equation {
private:
  Game m_game;
  int m_pl, m_iset, m_act;
  GameInfoset m_infoset;

public:
  RatioEquation(Game p_game, int p_player, int p_infoset, int p_action)
    : m_game(p_game), m_pl(p_player), m_iset(p_infoset), m_act(p_action),
      m_infoset(p_game->GetPlayer(p_player)->GetInfoset(p_infoset))
  { }

  double Value(const LogBehavProfile<double> &p_profile, 
	       double p_lambda);
  void Gradient(const LogBehavProfile<double> &p_profile, double p_lambda,
		Vector<double> &p_gradient);
};


double RatioEquation::Value(const LogBehavProfile<double> &p_profile,
			    double p_lambda)
{
  return (p_profile.GetLogProb(m_pl, m_iset, m_act) - 
	  p_profile.GetLogProb(m_pl, m_iset, 1) -
	  p_lambda *
	  (p_profile.GetPayoff(m_infoset->GetAction(m_act)) -
	   p_profile.GetPayoff(m_infoset->GetAction(1))));
}

void RatioEquation::Gradient(const LogBehavProfile<double> &p_profile,
			     double p_lambda,
			     Vector<double> &p_gradient)
{
  int i = 1;
  for (int pl = 1; pl <= m_game->NumPlayers(); pl++) {
    GamePlayer player = m_game->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      GameInfoset infoset = player->GetInfoset(iset);
      for (int act = 1; act <= infoset->NumActions(); act++, i++) {
	if (infoset == m_infoset) {
	  if (act == 1) {
	    p_gradient[i] = -1.0;
	  }
	  else if (act == m_act) {
	    p_gradient[i] = 1.0; 
	  }
	  else {
	    p_gradient[i] = 0.0;
	  }
	}
	else {   // infoset1 != infoset2
	  p_gradient[i] = 
	    -p_lambda * 
	    (p_profile.DiffActionValue(m_infoset->GetAction(m_act),
				       infoset->GetAction(act)) -
	     p_profile.DiffActionValue(m_infoset->GetAction(1),
				       infoset->GetAction(act)));
	}
      }
    }
  }

  p_gradient[i] = (p_profile.GetPayoff(m_infoset->GetAction(1)) -
		   p_profile.GetPayoff(m_infoset->GetAction(m_act)));
}


//------------------------------------------------------------------------------
//                        AgentQREPathTracer: Lifecycle
//------------------------------------------------------------------------------

AgentQREPathTracer::AgentQREPathTracer(const MixedBehavProfile<double> &p_start) 
  : m_start(p_start), m_fullGraph(true), m_decimals(6)
{ 
  SetTargetParam(-1.0);
  for (int pl = 1; pl <= p_start.GetGame()->NumPlayers(); pl++) {
    GamePlayer player = p_start.GetGame()->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      m_equations.Append(new SumToOneEquation(p_start.GetGame(), pl, iset));
      for (int act = 2; act <= player->GetInfoset(iset)->NumActions(); act++) {
	m_equations.Append(new RatioEquation(p_start.GetGame(), pl, iset, act));
      }
    }
  }
}

AgentQREPathTracer::~AgentQREPathTracer()
{
  for (int i = 1; i <= m_equations.Length(); i++) {
    delete m_equations[i];
  }
}

//------------------------------------------------------------------------------
//              AgentQREPathTracer: Wrapper to the tracing engine
//------------------------------------------------------------------------------

void 
AgentQREPathTracer::TraceAgentPath(const MixedBehavProfile<double> &p_start,
				   double p_startLambda, double p_maxLambda, 
				   double p_omega)
{
  Vector<double> x(p_start.Length() + 1);
  for (int i = 1; i <= p_start.Length(); i++) {
    x[i] = log(p_start[i]);
  }
  x[x.Length()] = p_startLambda;
  
  TracePath(x, p_maxLambda, p_omega);
}

//------------------------------------------------------------------------------
//               AgentQREPathTracer: Providing virtual functions
//------------------------------------------------------------------------------

double
AgentQREPathTracer::Criterion(const Vector<double> &p_point,
			      const Vector<double> &p_tangent)
{
  if (GetTargetParam() > 0.0) {
    return p_point[p_point.Length()] - GetTargetParam();
  }
  else {
    return PathTracer::Criterion(p_point, p_tangent);
  }
}

void
AgentQREPathTracer::GetLHS(const Vector<double> &p_point, Vector<double> &p_lhs)
{
  Game game = m_start.GetGame();
  LogBehavProfile<double> profile(game);
  for (int i = 1; i <= profile.Length(); i++) {
    profile.SetLogProb(i, p_point[i]);
  }

  double lambda = p_point[p_point.Length()];

  for (int i = 1; i <= p_lhs.Length(); i++) {
    p_lhs[i] = m_equations[i]->Value(profile, lambda);
  }
}

void
AgentQREPathTracer::GetJacobian(const Vector<double> &p_point, 
				Matrix<double> &p_matrix)
{
  Game game = m_start.GetGame();
  LogBehavProfile<double> profile(game);
  for (int i = 1; i <= profile.Length(); i++) {
    profile.SetLogProb(i, p_point[i]);
  }
  double lambda = p_point[p_point.Length()];

  for (int i = 1; i <= m_equations.Length(); i++) {
    Vector<double> column(p_point.Length());
    m_equations[i]->Gradient(profile, lambda, column);
    p_matrix.SetColumn(i, column);
  }
}


//----------------------------------------------------------------------------
//                 AgentQREPathTracer: Outputting profiles
//----------------------------------------------------------------------------

void 
AgentQREPathTracer::PrintProfile(std::ostream &p_stream,
				 const Vector<double> &x,
				 bool p_isTerminal)
{
  p_stream.setf(std::ios::fixed);
  // By convention, we output lambda first
  if (!p_isTerminal) {
    p_stream << std::setprecision(m_decimals) << x[x.Length()];
  }
  else {
    p_stream << "NE";
  }
  p_stream.unsetf(std::ios::fixed);

  for (int i = 1; i < x.Length(); i++) {
    p_stream << "," << std::setprecision(m_decimals) << exp(x[i]);
  }

  p_stream << std::endl;
}

void 
AgentQREPathTracer::OnStep(const Vector<double> &x,
			   bool p_isTerminal = false)
{
  if ((m_fullGraph && !p_isTerminal) || (!m_fullGraph && p_isTerminal)) {
    PrintProfile(std::cout, x, p_isTerminal);
  }
}



