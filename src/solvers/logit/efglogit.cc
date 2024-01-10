//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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
#include "gambit.h"
#include "logbehav.imp"
#include "efglogit.h"

namespace Gambit {

//------------------------------------------------------------------------------
//                        Classes representing equations
//------------------------------------------------------------------------------

class AgentQREPathTracer::EquationSystem : public PathTracer::EquationSystem {
public:
  explicit EquationSystem(const Game &p_game);
  ~EquationSystem() override;
  // Compute the value of the system of equations at the specified point.
  void GetValue(const Vector<double> &p_point,
  	                Vector<double> &p_lhs) const override;
  // Compute the Jacobian matrix at the specified point.
  void GetJacobian(const Vector<double> &p_point,
			   Matrix<double> &p_matrix) const override;

private:
  //
  // This abstract base class represents one of the defining equations of the system.
  //
  class Equation {
  public:
    virtual ~Equation() = default;
    virtual double Value(const LogBehavProfile<double> &p_point,
			 double p_lambda) const = 0;
    virtual void Gradient(const LogBehavProfile<double> &p_point, 
			  double p_lambda,
			  Vector<double> &p_gradient) const = 0;
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
    SumToOneEquation(const Game &p_game, int p_player, int p_infoset)
      : m_game(p_game), m_pl(p_player), m_iset(p_infoset),
	m_infoset(p_game->GetPlayer(p_player)->GetInfoset(p_infoset))
    { }

    double Value(const LogBehavProfile<double> &p_profile,
		 double p_lambda) const override;
    void Gradient(const LogBehavProfile<double> &p_profile, double p_lambda,
		  Vector<double> &p_gradient) const override;
  };

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
    RatioEquation(const Game &p_game, int p_player, int p_infoset, int p_action)
      : m_game(p_game), m_pl(p_player), m_iset(p_infoset), m_act(p_action),
	m_infoset(p_game->GetPlayer(p_player)->GetInfoset(p_infoset))
    { }

    double Value(const LogBehavProfile<double> &p_profile, 
		 double p_lambda) const override;
    void Gradient(const LogBehavProfile<double> &p_profile, double p_lambda,
		  Vector<double> &p_gradient) const override;
  };

  Array<Equation *> m_equations;
  const Game &m_game;
};

AgentQREPathTracer::EquationSystem::EquationSystem(const Game &p_game)
  : m_game(p_game)
{
  for (int pl = 1; pl <= m_game->NumPlayers(); pl++) {
    GamePlayer player = m_game->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      m_equations.push_back(new SumToOneEquation(m_game, pl, iset));
      for (int act = 2; act <= player->GetInfoset(iset)->NumActions(); act++) {
        m_equations.push_back(new RatioEquation(m_game, pl, iset, act));
      }
    }
  }
}

AgentQREPathTracer::EquationSystem::~EquationSystem()
{
  for (int i = 1; i <= m_equations.Length(); i++) {
    delete m_equations[i];
  }
}

double
AgentQREPathTracer::EquationSystem::SumToOneEquation::Value(const LogBehavProfile<double> &p_profile,
							    double p_lambda) const
{
  double value = -1.0;
  for (int act = 1; act <= m_infoset->NumActions(); act++) {
    value += p_profile.GetProb(m_pl, m_iset, act);
  }
  return value;
}

void
AgentQREPathTracer::EquationSystem::SumToOneEquation::Gradient(const LogBehavProfile<double> &p_profile,
							       double p_lambda,
							       Vector<double> &p_gradient) const
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
			       


double
AgentQREPathTracer::EquationSystem::RatioEquation::Value(const LogBehavProfile<double> &p_profile,
							 double p_lambda) const
{
  return (p_profile.GetLogProb(m_pl, m_iset, m_act) - 
	  p_profile.GetLogProb(m_pl, m_iset, 1) -
	  p_lambda *
	  (p_profile.GetPayoff(m_infoset->GetAction(m_act)) -
	   p_profile.GetPayoff(m_infoset->GetAction(1))));
}

void
AgentQREPathTracer::EquationSystem::RatioEquation::Gradient(const LogBehavProfile<double> &p_profile,
							    double p_lambda,
							    Vector<double> &p_gradient) const
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


void
AgentQREPathTracer::EquationSystem::GetValue(const Vector<double> &p_point,
					     Vector<double> &p_lhs) const
{
  LogBehavProfile<double> profile((BehaviorSupportProfile(m_game)));
  for (int i = 1; i <= profile.Length(); i++) {
    profile.SetLogProb(i, p_point[i]);
  }

  double lambda = p_point[p_point.Length()];

  for (int i = 1; i <= p_lhs.Length(); i++) {
    p_lhs[i] = m_equations[i]->Value(profile, lambda);
  }
}

void
AgentQREPathTracer::EquationSystem::GetJacobian(const Vector<double> &p_point, 
						Matrix<double> &p_matrix) const
{
  LogBehavProfile<double> profile((BehaviorSupportProfile(m_game)));
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

class AgentQREPathTracer::CallbackFunction : public PathTracer::CallbackFunction {
public:
  CallbackFunction(std::ostream &p_stream,
                   const Game &p_game,
                   bool p_fullGraph, int p_decimals)
    : m_stream(p_stream), m_game(p_game),
      m_fullGraph(p_fullGraph), m_decimals(p_decimals)
  { }
  ~CallbackFunction() override = default;

  void operator()(const Vector<double> &p_point, bool p_isTerminal) const override;
  const List<LogitQREMixedBehaviorProfile> &GetProfiles() const { return m_profiles; }

private:
  std::ostream &m_stream;
  Game m_game;
  bool m_fullGraph;
  int m_decimals;
  mutable List<LogitQREMixedBehaviorProfile> m_profiles;
};

void AgentQREPathTracer::CallbackFunction::operator()(const Vector<double> &p_point, bool p_isTerminal) const
{
  if ((!m_fullGraph || p_isTerminal) && (m_fullGraph || !p_isTerminal)) {
    return;
  }

  m_stream.setf(std::ios::fixed);
  // By convention, we output lambda first
  if (!p_isTerminal) {
    m_stream << std::setprecision(m_decimals) << p_point.back();
  }
  else {
    m_stream << "NE";
  }
  m_stream.unsetf(std::ios::fixed);

  for (int i = 1; i < p_point.Length(); i++) {
    m_stream << "," << std::setprecision(m_decimals) << exp(p_point[i]);
  }

  m_stream << std::endl;

  MixedBehaviorProfile<double> profile(m_game);
  for (int i = 1; i < p_point.Length(); i++) {
    profile[i] = exp(p_point[i]);
  }
  m_profiles.push_back(LogitQREMixedBehaviorProfile(profile, p_point.back()));
}

//------------------------------------------------------------------------------
//                   AgentQREPathTracer: Criterion function
//------------------------------------------------------------------------------

class AgentQREPathTracer::LambdaCriterion : public PathTracer::CriterionFunction {
public:
  explicit LambdaCriterion(double p_lambda) : m_lambda(p_lambda) { }

  double operator()(const Vector<double> &p_point,
			    const Vector<double> &p_tangent) const override
  { return p_point[p_point.Length()] - m_lambda; }

private:
  double m_lambda;
};

//------------------------------------------------------------------------------
//              AgentQREPathTracer: Wrapper to the tracing engine
//------------------------------------------------------------------------------


List<LogitQREMixedBehaviorProfile>
AgentQREPathTracer::TraceAgentPath(const LogitQREMixedBehaviorProfile &p_start,
				   std::ostream &p_stream, 
				   double p_maxLambda, 
				   double p_omega, double p_targetLambda)
{
  List<LogitQREMixedBehaviorProfile> ret;
  Vector<double> x(p_start.BehaviorProfileLength() + 1);
  for (size_t i = 1; i <= p_start.BehaviorProfileLength(); i++) {
    x[i] = log(p_start[i]);
  }
  x[x.Length()] = p_start.GetLambda();

  CallbackFunction func(p_stream, p_start.GetGame(), m_fullGraph, m_decimals);
  if (p_targetLambda > 0.0) {
    TracePath(EquationSystem(p_start.GetGame()),
	      x, p_maxLambda, p_omega, func,
	      LambdaCriterion(p_targetLambda));
  }
  else {
    TracePath(EquationSystem(p_start.GetGame()),
	      x, p_maxLambda, p_omega, func);
  }
  return func.GetProfiles();
}

}  // end namespace Gambit




