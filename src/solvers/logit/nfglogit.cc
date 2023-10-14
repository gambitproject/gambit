//
// This file is part of Gambit
// Copyright (c) 1994-2023, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/tools/logit/nfglogit.cc
// Computation of quantal response equilibrium correspondence for
// normal form games.
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
#include <fstream>

#include "gambit.h"
#include "nfglogit.h"

namespace Gambit {

//------------------------------------------------------------------------------
//            StrategicQREPathTracer: Classes representing equations
//------------------------------------------------------------------------------

class StrategicQREPathTracer::EquationSystem : public PathTracer::EquationSystem {
public:
  explicit EquationSystem(const Game &p_game) : m_game(p_game) { }
  ~EquationSystem() override = default;
  // Compute the value of the system of equations at the specified point.
  void GetValue(const Vector<double> &p_point,
  	                Vector<double> &p_lhs) const override;
  // Compute the Jacobian matrix at the specified point.
  void GetJacobian(const Vector<double> &p_point,
			   Matrix<double> &p_matrix) const override;

private:
  Game m_game;
};

void 
StrategicQREPathTracer::EquationSystem::GetValue(const Vector<double> &p_point,
						 Vector<double> &p_lhs) const
{
  MixedStrategyProfile<double> profile(m_game->NewMixedStrategyProfile(0.0)), logprofile(m_game->NewMixedStrategyProfile(0.0));
  for (int i = 1; i <= profile.MixedProfileLength(); i++) {
    profile[i] = exp(p_point[i]);
    logprofile[i] = p_point[i];
  }
  double lambda = p_point[p_point.Length()];
  p_lhs = 0.0;
  for (int rowno = 0, pl = 1; pl <= m_game->NumPlayers(); pl++) {
    GamePlayer player = m_game->GetPlayer(pl);
    for (size_t st = 1; st <= player->Strategies().size(); st++) {
      rowno++;
      if (st == 1) {
	// This is a sum-to-one equation
	p_lhs[rowno] = -1.0;
	for (size_t j = 1; j <= player->Strategies().size(); j++) {
	  p_lhs[rowno] += profile[player->GetStrategy(j)];
	}
      }
      else {
	// This is a ratio equation
	p_lhs[rowno] = (logprofile[player->GetStrategy(st)] - 
			logprofile[player->GetStrategy(1)] -
			lambda * (profile.GetPayoff(player->GetStrategy(st)) -
				  profile.GetPayoff(player->GetStrategy(1))));

      }
    }
  }
}

void
StrategicQREPathTracer::EquationSystem::GetJacobian(const Vector<double> &p_point,
						    Matrix<double> &p_matrix) const
{
  MixedStrategyProfile<double> profile(m_game->NewMixedStrategyProfile(0.0)), logprofile(m_game->NewMixedStrategyProfile(0.0));
  for (int i = 1; i <= profile.MixedProfileLength(); i++) {
    profile[i] = exp(p_point[i]);
    logprofile[i] = p_point[i];
  }
  double lambda = p_point[p_point.Length()];

  p_matrix = 0.0;

  for (int rowno = 0, i = 1; i <= m_game->NumPlayers(); i++) {
    GamePlayer player = m_game->GetPlayer(i);
    for (size_t j = 1; j <= player->Strategies().size(); j++) {
      rowno++;
      if (j == 1) {
	// This is a sum-to-one equation
	for (int colno = 0, ell = 1; ell <= m_game->NumPlayers(); ell++) {
	  GamePlayer player2 = m_game->GetPlayer(ell);
	  for (size_t m = 1; m <= player2->Strategies().size(); m++) {
	    colno++;
	    if (i == ell) {
	      p_matrix(colno, rowno) = profile[player2->GetStrategy(m)];
	    }
	    // Otherwise, entry is zero
	  }
	}
	// The last column is derivative wrt lamba, which is zero
      }
      else {
	// This is a ratio equation
	for (int colno = 0, ell = 1; ell <= m_game->NumPlayers(); ell++) {
	  GamePlayer player2 = m_game->GetPlayer(ell);
  	  for (size_t m = 1; m <= player2->Strategies().size(); m++) {
	    colno++;
	    if (i == ell) {
	      if (m == 1) {
		p_matrix(colno, rowno) = -1.0;
	      }
	      else if (m == j) {
		p_matrix(colno, rowno) = 1.0;
	      }
	      // Entry is zero for all other strategy pairs
	    }
	    else {
	      p_matrix(colno, rowno) =
		-lambda * profile[player2->GetStrategy(m)] *
		(profile.GetPayoffDeriv(i, 
					player->GetStrategy(j),
					player2->GetStrategy(m)) -
		 profile.GetPayoffDeriv(i, 
					player->GetStrategy(1),
					player2->GetStrategy(m)));
	    }
	  }
	}
	// Fill the last column, the derivative wrt lambda
	p_matrix(p_matrix.NumRows(), rowno) =
	  (profile.GetPayoff(player->GetStrategy(1)) - 
	   profile.GetPayoff(player->GetStrategy(j)));
      }
    }
  }
}

//----------------------------------------------------------------------------
//               StrategicQREPathTracer: Criterion function
//----------------------------------------------------------------------------

class StrategicQREPathTracer::LambdaCriterion : public PathTracer::CriterionFunction {
public:
  explicit LambdaCriterion(double p_lambda) : m_lambda(p_lambda) { }

  double operator()(const Vector<double> &p_point,
			    const Vector<double> &p_tangent) const override
  { return p_point[p_point.Length()] - m_lambda; }

private:
  double m_lambda;
};

//----------------------------------------------------------------------------
//               StrategicQREPathTracer: Callback function
//----------------------------------------------------------------------------

class StrategicQREPathTracer::CallbackFunction : public PathTracer::CallbackFunction {
public:
  CallbackFunction(std::ostream &p_stream,
		   const Game &p_game,
		   bool p_fullGraph, int p_decimals)
    : m_stream(p_stream), m_game(p_game),
      m_fullGraph(p_fullGraph), m_decimals(p_decimals) { }
  ~CallbackFunction() override = default;
  
  void operator()(const Vector<double> &p_point, bool p_isTerminal) const override;
  const List<LogitQREMixedStrategyProfile> &GetProfiles() const
  { return m_profiles; }
  
private:
  std::ostream &m_stream;
  Game m_game;
  bool m_fullGraph;
  int m_decimals;
  mutable List<LogitQREMixedStrategyProfile> m_profiles;
};

void 
StrategicQREPathTracer::CallbackFunction::operator()(const Vector<double> &p_point,
                                                     bool p_isTerminal) const
{
  if ((!m_fullGraph || p_isTerminal) && (m_fullGraph || !p_isTerminal)) {
    return;
  }
  m_stream.setf(std::ios::fixed);
  // By convention, we output lambda first
  if (!p_isTerminal) {
    m_stream << std::setprecision(m_decimals) << p_point[p_point.Length()];
  }
  else {
    m_stream << "NE";
  }
  m_stream.unsetf(std::ios::fixed);
  MixedStrategyProfile<double> profile(m_game->NewMixedStrategyProfile(0.0));
  for (int i = 1; i < p_point.Length(); i++) {
    profile[i] = exp(p_point[i]);
    m_stream << "," << std::setprecision(m_decimals) << profile[i];
  }
  m_stream << std::endl;
  m_profiles.push_back(LogitQREMixedStrategyProfile(profile, p_point.back(), 0.0));
}

//----------------------------------------------------------------------------
//               StrategicQREPathTracer: Main driver routines
//----------------------------------------------------------------------------

List<LogitQREMixedStrategyProfile>
StrategicQREPathTracer::TraceStrategicPath(const LogitQREMixedStrategyProfile &p_start,
					   std::ostream &p_stream,
					   double p_maxLambda, 
					   double p_omega) const
{
  Vector<double> x(p_start.MixedProfileLength() + 1);
  for (int i = 1; i <= p_start.MixedProfileLength(); i++) {
    x[i] = log(p_start[i]);
  }
  x[x.Length()] = p_start.GetLambda();
  CallbackFunction func(p_stream, p_start.GetGame(), m_fullGraph, m_decimals);
  TracePath(EquationSystem(p_start.GetGame()),
	    x, p_maxLambda, p_omega, func);
  return func.GetProfiles();
}

LogitQREMixedStrategyProfile
StrategicQREPathTracer::SolveAtLambda(const LogitQREMixedStrategyProfile &p_start,
				      std::ostream &p_stream,
				      double p_targetLambda, double p_omega) const
{
  Vector<double> x(p_start.MixedProfileLength() + 1);
  for (int i = 1; i <= p_start.MixedProfileLength(); i++) {
    x[i] = log(p_start[i]);
  }
  x[x.Length()] = p_start.GetLambda();
  CallbackFunction func(p_stream, p_start.GetGame(), m_fullGraph, m_decimals);
  TracePath(EquationSystem(p_start.GetGame()),
	    x, std::max(1.0, 3.0*p_targetLambda), p_omega,
	    func,
	    LambdaCriterion(p_targetLambda));
  return func.GetProfiles().back();
}

//----------------------------------------------------------------------------
//                 StrategicQREEstimator: Criterion function
//----------------------------------------------------------------------------

namespace { 
double LogLike(const Vector<double> &p_frequencies, const Vector<double> &p_point)
{
  double logL = 0.0;
  for (int i = 1; i <= p_frequencies.Length(); i++) {
    logL += p_frequencies[i] * log(p_point[i]);
  }
  return logL;
}

}   // end anonymous namespace

class StrategicQREEstimator::CriterionFunction : public PathTracer::CriterionFunction {
public:
  explicit CriterionFunction(const Vector<double> &p_frequencies)
    : m_frequencies(p_frequencies) { }
  ~CriterionFunction() override = default;

  double operator()(const Vector<double> &p_point,
			    const Vector<double> &p_tangent) const override
  {
    double diff_logL = 0.0;
    for (int i = 1; i <= m_frequencies.Length(); i++) {
      diff_logL += m_frequencies[i] * p_tangent[i];
    }
    return diff_logL;
  }

private:
  Vector<double> m_frequencies;
};

//----------------------------------------------------------------------------
//                StrategicQREEstimator: Callback function
//----------------------------------------------------------------------------

class StrategicQREEstimator::CallbackFunction : public PathTracer::CallbackFunction {
public:
  CallbackFunction(std::ostream &p_stream,
		   const Game &p_game,
		   const Vector<double> &p_frequencies,
		   bool p_fullGraph, int p_decimals);
  ~CallbackFunction() override = default;
  
  void operator()(const Vector<double> &p_point,
			  bool p_isTerminal) const override;

  LogitQREMixedStrategyProfile GetMaximizer() const {
    return { m_bestProfile, m_bestLambda, m_maxlogL };
  }
  void PrintMaximizer() const;
		    
private:
  void PrintProfile(const MixedStrategyProfile<double> &, double) const;

  std::ostream &m_stream;
  Game m_game;
  const Vector<double> &m_frequencies;
  bool m_fullGraph;
  int m_decimals;
  mutable MixedStrategyProfile<double> m_bestProfile;
  mutable double m_bestLambda;
  mutable double m_maxlogL;
};

StrategicQREEstimator::CallbackFunction::CallbackFunction(std::ostream &p_stream,
							  const Game &p_game,
							  const Vector<double> &p_frequencies,
							  bool p_fullGraph, int p_decimals)
  : m_stream(p_stream), m_game(p_game), m_frequencies(p_frequencies),
    m_fullGraph(p_fullGraph), m_decimals(p_decimals),
    m_bestProfile(p_game->NewMixedStrategyProfile(0.0)),
    m_bestLambda(0.0),
    m_maxlogL(LogLike(p_frequencies, static_cast<const Vector<double> &>(m_bestProfile)))
{ }

void
StrategicQREEstimator::CallbackFunction::PrintProfile(const MixedStrategyProfile<double> &p_profile,
						      double p_logL) const
{
  for (int i = 1; i <= p_profile.MixedProfileLength(); i++) {
    m_stream << "," << std::setprecision(m_decimals) << p_profile[i];
  }
  m_stream.setf(std::ios::fixed);
  m_stream << "," << std::setprecision(m_decimals);
  m_stream << p_logL;
  m_stream.unsetf(std::ios::fixed);
}

void
StrategicQREEstimator::CallbackFunction::PrintMaximizer() const
{
  m_stream.setf(std::ios::fixed);
  // By convention, we output lambda first
  m_stream << std::setprecision(m_decimals) << m_bestLambda;
  m_stream.unsetf(std::ios::fixed);
  PrintProfile(m_bestProfile, m_maxlogL);
  m_stream << std::endl;
}

void 
StrategicQREEstimator::CallbackFunction::operator()(const Vector<double> &x,
						    bool p_isTerminal) const
{
  m_stream.setf(std::ios::fixed);
  // By convention, we output lambda first
  if (!p_isTerminal) {
    m_stream << std::setprecision(m_decimals) << x[x.Length()];
  }
  else {
    m_stream << "NE";
  }
  m_stream.unsetf(std::ios::fixed);
  MixedStrategyProfile<double> profile(m_game->NewMixedStrategyProfile(0.0));
  for (int i = 1; i < x.Length(); i++) {
    profile[i] = exp(x[i]);
  }
  double logL = LogLike(m_frequencies, static_cast<const Vector<double> &>(profile));
  PrintProfile(profile, logL);
  m_stream << std::endl;
  if (logL > m_maxlogL) {
    m_maxlogL = logL;
    m_bestLambda = x[x.Length()];
    m_bestProfile = profile;
  }
}

//----------------------------------------------------------------------------
//               StrategicQREEstimator: Main driver routine
//----------------------------------------------------------------------------

LogitQREMixedStrategyProfile
StrategicQREEstimator::Estimate(const LogitQREMixedStrategyProfile &p_start,
				const MixedStrategyProfile<double> &p_frequencies,
				std::ostream &p_stream,
				double p_maxLambda, double p_omega)
{
  if (p_start.GetGame() != p_frequencies.GetGame()) {
    throw MismatchException();
  }
  
  Vector<double> x(p_start.MixedProfileLength() + 1);
  for (int i = 1; i <= p_start.MixedProfileLength(); i++) {
    x[i] = log(p_start[i]);
  }
  x[x.Length()] = p_start.GetLambda();

  CallbackFunction callback(p_stream, p_start.GetGame(),
			    static_cast<const Vector<double> &>(p_frequencies), m_fullGraph, m_decimals);
  while (x[x.Length()] < p_maxLambda) {
    TracePath(EquationSystem(p_start.GetGame()),
	      x, p_maxLambda, p_omega,
	      callback,
	      CriterionFunction(static_cast<const Vector<double> &>(p_frequencies)));
    if (x[x.Length()] < p_maxLambda) {
      // Found an extremum of the likelihood function
      // start iterating again from the same point in case of
      // local optima.
    }
  }
  callback.PrintMaximizer();
  return callback.GetMaximizer();
}

}   // end namespace Gambit
