//
// This file is part of Gambit
// Copyright (c) 1994-2013, The Gambit Project (http://www.gambit-project.org)
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

#include <libgambit/libgambit.h>
using namespace Gambit;

#include "nfglogit.h"

//----------------------------------------------------------------------------
//           StrategicQREPathTracer: Wrapper to the tracing engine
//----------------------------------------------------------------------------

void 
StrategicQREPathTracer::TraceStrategicPath(const MixedStrategyProfile<double> &p_start,
					   double p_startLambda, double p_maxLambda, 
					   double p_omega)
{
  Vector<double> x(p_start.MixedProfileLength() + 1);
  for (int i = 1; i <= p_start.MixedProfileLength(); i++) {
    x[i] = log(p_start[i]);
  }
  x[x.Length()] = p_startLambda;

  if (IsMLEMode()) {
    while (x[x.Length()] < p_maxLambda) {
      TracePath(x, p_maxLambda, p_omega);
      if (x[x.Length()] < p_maxLambda) {
	std::cout << std::endl;
      }
    }
  }
  else {
    TracePath(x, p_maxLambda, p_omega);
  }
}

//----------------------------------------------------------------------------
//             StrategicQREPathTracer: Providing virtual functions
//----------------------------------------------------------------------------

double
StrategicQREPathTracer::Criterion(const Vector<double> &p_point,
				  const Vector<double> &p_tangent)
{
  if (IsMLEMode()) {
    double logL = 0.0;
    for (int i = 1; i <= m_frequencies.Length(); i++) {
      logL += m_frequencies[i] * p_tangent[i];
    }
    return logL;
  }
  else if (GetTargetParam() > 0.0) {
    return p_point[p_point.Length()] - GetTargetParam();
  }
  else {
    return PathTracer::Criterion(p_point, p_tangent);
  }
}

void 
StrategicQREPathTracer::GetLHS(const Vector<double> &p_point, Vector<double> &p_lhs)
{
  const StrategySupport &support = m_start.GetSupport();
  MixedStrategyProfile<double> profile(support.NewMixedStrategyProfile<double>()), logprofile(support.NewMixedStrategyProfile<double>());
  for (int i = 1; i <= profile.MixedProfileLength(); i++) {
    profile[i] = exp(p_point[i]);
    logprofile[i] = p_point[i];
  }
  double lambda = p_point[p_point.Length()];
  p_lhs = 0.0;
  for (int rowno = 0, pl = 1; pl <= support.NumPlayers(); pl++) {
    StrategySupportPlayer player = support.GetPlayer(pl);
    for (int st = 1; st <= player->NumStrategies(); st++) {
      rowno++;
      if (st == 1) {
	// This is a sum-to-one equation
	p_lhs[rowno] = -1.0;
	for (int j = 1; j <= player->NumStrategies(); j++) {
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
StrategicQREPathTracer::GetJacobian(const Vector<double> &p_point,
				    Matrix<double> &p_matrix)
{
  const StrategySupport &support = m_start.GetSupport();
  MixedStrategyProfile<double> profile(support.NewMixedStrategyProfile<double>()), logprofile(support.NewMixedStrategyProfile<double>());
  for (int i = 1; i <= profile.MixedProfileLength(); i++) {
    profile[i] = exp(p_point[i]);
    logprofile[i] = p_point[i];
  }
  double lambda = p_point[p_point.Length()];

  p_matrix = 0.0;

  for (int rowno = 0, i = 1; i <= support.NumPlayers(); i++) {
    StrategySupportPlayer player = support.GetPlayer(i);
    for (int j = 1; j <= player->NumStrategies(); j++) {
      rowno++;
      if (j == 1) {
	// This is a sum-to-one equation
	for (int colno = 0, ell = 1; ell <= support.NumPlayers(); ell++) {
	  StrategySupportPlayer player2 = support.GetPlayer(ell);
	  for (int m = 1; m <= player2->NumStrategies(); m++) {
	    colno++;
	    if (i == ell) {
	      p_matrix(colno, rowno) = profile[player2->GetStrategy(m)];
	    }
	    // Otherwise, entry is zero
	  }
	}
	// The last column is derivate wrt lamba, which is zero
      }
      else {
	// This is a ratio equation
	for (int colno = 0, ell = 1; ell <= support.NumPlayers(); ell++) {
	  StrategySupportPlayer player2 = support.GetPlayer(ell);
	  for (int m = 1; m <= player2->NumStrategies(); m++) {
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
//            StrategicQREPathTracer: Maximum likelihood estimation
//----------------------------------------------------------------------------

double 
StrategicQREPathTracer::LogLike(const Array<double> &p_point)
{
  double ret = 0.0;
  
  for (int i = 1; i <= m_frequencies.Length(); i++) {
    ret += m_frequencies[i] * log(p_point[i]);
  }

  return ret;
}


//----------------------------------------------------------------------------
//                StrategicQREPathTracer: Outputting profiles
//----------------------------------------------------------------------------

void 
StrategicQREPathTracer::PrintProfile(std::ostream &p_stream,
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

  for (int i = 1; i <  x.Length(); i++) {
    p_stream << "," << std::setprecision(m_decimals) << exp(x[i]);
  }

  if (IsMLEMode()) {
    MixedStrategyProfile<double> profile(m_start);
    for (int i = 1; i <= profile.MixedProfileLength(); i++) {
      profile[i] = exp(x[i]);
    }

    p_stream.setf(std::ios::fixed);
    p_stream << "," << std::setprecision(m_decimals) << LogLike((const Vector<double> &) profile);
    p_stream.unsetf(std::ios::fixed);
  }

  p_stream << std::endl;
}

void 
StrategicQREPathTracer::OnStep(const Vector<double> &x, bool p_isTerminal = false)
{
  if ((m_fullGraph && !p_isTerminal) || (!m_fullGraph && p_isTerminal)) {
    PrintProfile(std::cout, x, p_isTerminal);
  }
}



