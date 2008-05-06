//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Trace a branch of the agent QRE correspondence
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
#include <math.h>
#include <iostream>
#include <libgambit/libgambit.h>
#include <libgambit/sqmatrix.h>
#include "logbehav.imp"

using namespace Gambit;



//=========================================================================
//             QRE Correspondence Computation via Homotopy
//=========================================================================

//
// The following code implements a homotopy approach to computing
// the logistic QRE correspondence.  This implementation is a basic
// Euler-Newton approach with adaptive step size, based upon the
// ideas and codes presented in Allgower and Georg's
// _Numerical Continuation Methods_.
//
// Many of these functions are duplicated in efgqre.cc and nfgqre.cc.
// This should be fixed in future!
//

inline double sqr(double x) { return x*x; }

static void Givens(Matrix<double> &b, Matrix<double> &q,
		   double &c1, double &c2, int l1, int l2, int l3)
{
  //std::cout << "c1=" << c1 << " c2=" << c2 << std::endl;
  if (fabs(c1) + fabs(c2) == 0.0) {
    return;
  }

  double sn;
  if (fabs(c2) >= fabs(c1)) {
    sn = sqrt(1.0 + sqr(c1/c2)) * fabs(c2);
  }
  else {
    sn = sqrt(1.0 + sqr(c2/c1)) * fabs(c1);
  }
  double s1 = c1/sn;
  double s2 = c2/sn;
  //std::cout << "s1=" << s1 << " s2=" << s2 << std::endl;

  for (int k = 1; k <= q.NumColumns(); k++) {
    double sv1 = q(l1, k);
    double sv2 = q(l2, k);
    q(l1, k) = s1 * sv1 + s2 * sv2;
    q(l2, k) = -s2 * sv1 + s1 * sv2;
  }

  for (int k = l3; k <= b.NumColumns(); k++) {
    double sv1 = b(l1, k);
    double sv2 = b(l2, k);
    b(l1, k) = s1 * sv1 + s2 * sv2;
    b(l2, k) = -s2 * sv1 + s1 * sv2;
  }

  c1 = sn;
  c2 = 0.0;
}

static void QRDecomp(Matrix<double> &b, Matrix<double> &q)
{
  q.MakeIdent();
  for (int m = 1; m <= b.NumColumns(); m++) {
    for (int k = m + 1; k <= b.NumRows(); k++) {
      Givens(b, q, b(m, m), b(k, m), m, k, m + 1);
    }
  }
}

static void NewtonStep(Matrix<double> &q, Matrix<double> &b,
		       Vector<double> &u, Vector<double> &y,
		       double &d)
{
  for (int k = 1; k <= b.NumColumns(); k++) {
    for (int l = 1; l <= k - 1; l++) {
      y[k] -= b(l, k) * y[l];
    }
    y[k] /= b(k, k);
  }

  d = 0.0;
  for (int k = 1; k <= b.NumRows(); k++) {
    double s = 0.0;
    for (int l = 1; l <= b.NumColumns(); l++) {
      s += q(l, k) * y[l];
    }
    u[k] -= s;
    d += s*s;
  }
  d = sqrt(d);
}

//
// This abstract base class represents an equation
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
	  (p_profile.GetActionValue(m_infoset->GetAction(m_act)) -
	   p_profile.GetActionValue(m_infoset->GetAction(1))));
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

  p_gradient[i] = (p_profile.GetActionValue(m_infoset->GetAction(1)) -
		   p_profile.GetActionValue(m_infoset->GetAction(m_act)));
}


static void QreLHS(const Game &p_game,
		   const Array<Equation *> &p_equations,
		   const Vector<double> &p_point,
		   Vector<double> &p_lhs)
{
  LogBehavProfile<double> profile(p_game);
  for (int i = 1; i <= profile.Length(); i++) {
    profile.SetLogProb(i, p_point[i]);
  }

  double lambda = p_point[p_point.Length()];

  for (int i = 1; i <= p_lhs.Length(); i++) {
    p_lhs[i] = p_equations[i]->Value(profile, lambda);
  }
}

static void QreJacobian(const Game &p_game,
			const Array<Equation *> &p_equations,
			const Vector<double> &p_point,
			Matrix<double> &p_matrix)
{
  LogBehavProfile<double> profile(p_game);
  for (int i = 1; i <= profile.Length(); i++) {
    profile.SetLogProb(i, p_point[i]);
  }
  double lambda = p_point[p_point.Length()];

  for (int i = 1; i <= p_equations.Length(); i++) {
    Vector<double> column(p_point.Length());
    p_equations[i]->Gradient(profile, lambda, column);
    p_matrix.SetColumn(i, column);
  }
}

extern int g_numDecimals;
extern double g_targetLambda;

template <class T>
void PrintProfileDetail(std::ostream &p_stream,
			const LogBehavProfile<T> &p_profile)
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
	
	if (p_profile.GetProb(pl, iset, act) > .01) {
	  sprintf(buffer, "%11s   ", ToText(p_profile.GetProb(pl, iset, act), g_numDecimals).c_str());
	}
	else {
	  sprintf(buffer, "%11.3e   ", p_profile.GetProb(pl, iset, act));
	}
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

	if (p_profile.GetRealizProb(infoset->GetMember(n)) > .01) {
	  sprintf(buffer, "%11s    ", ToText(p_profile.GetRealizProb(infoset->GetMember(n)), g_numDecimals).c_str());
	}
	else {
	  sprintf(buffer, "%11.3e    ", p_profile.GetRealizProb(infoset->GetMember(n)));
	}
	p_stream << buffer;

	p_stream << "\n";
      }
    }

    p_stream << "\n";
  }
}

void PrintProfile(std::ostream &p_stream,
		  const BehavSupport &p_support, const Vector<double> &x,
		  bool p_terminal = false)
{
  p_stream.setf(std::ios::fixed);
  // By convention, we output lambda first
  if (!p_terminal) {
    p_stream << std::setprecision(g_numDecimals) << x[x.Length()];
  }
  else {
    p_stream << "NE";
  }
  p_stream.unsetf(std::ios::fixed);

  for (int i = 1; i < x.Length(); i++) {
    p_stream << "," << std::setprecision(g_numDecimals) << exp(x[i]);
  }

  p_stream << std::endl;

  LogBehavProfile<double> profile(p_support);
  for (int i = 1; i <= profile.Length(); i++) {
    profile.SetLogProb(i, x[i]);
  }
  //p_stream << "LiapValue: " << profile.GetLiapValue() << std::endl;
}

extern double g_maxDecel;
extern double g_hStart;
extern bool g_fullGraph;

void TraceAgentPath(const LogBehavProfile<double> &p_start,
		    double p_startLambda, double p_maxLambda, double p_omega)
{
  const double c_tol = 1.0e-4;     // tolerance for corrector iteration
  const double c_maxDist = 0.4;    // maximal distance to curve
  const double c_maxContr = 0.6;   // maximal contraction rate in corrector
  const double c_eta = 0.1;        // perturbation to avoid cancellation
                                   // in calculating contraction rate
  double h = g_hStart;             // initial stepsize
  const double c_hmin = 1.0e-8;    // minimal stepsize
  const int c_maxIter = 100;       // maximum iterations in corrector

  bool newton = false;             // using Newton steplength

  Array<Equation *> equations;
  for (int pl = 1; pl <= p_start.GetGame()->NumPlayers(); pl++) {
    GamePlayer player = p_start.GetGame()->GetPlayer(pl);
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      equations.Append(new SumToOneEquation(p_start.GetGame(), pl, iset));
      for (int act = 2; act <= player->GetInfoset(iset)->NumActions(); act++) {
	equations.Append(new RatioEquation(p_start.GetGame(), pl, iset, act));
      }
    }
  }

  Vector<double> x(p_start.Length() + 1), u(p_start.Length() + 1);
  for (int i = 1; i <= p_start.Length(); i++) {
    x[i] = p_start.GetLogProb(i);
  }
  x[x.Length()] = p_startLambda;

  if (g_fullGraph) {
    PrintProfile(std::cout, p_start.GetSupport(), x);

    /*
    LogBehavProfile<double> current(p_start);
    for (int i = 1; i <= p_start.Length(); i++) {
      if (isLog[i]) {
	current.SetLogProb(i, x[i]);
      }
      else {
	current.SetProb(i, x[i]);
      }
    }
    PrintProfileDetail(std::cout, current);
    */
  }

  Vector<double> t(p_start.Length() + 1);
  Vector<double> y(p_start.Length());

  Matrix<double> b(p_start.Length() + 1, p_start.Length());
  SquareMatrix<double> q(p_start.Length() + 1);
  QreJacobian(p_start.GetGame(), equations, x, b);
  QRDecomp(b, q);
  q.GetRow(q.NumRows(), t);
  
  while (x[x.Length()] >= 0.0 && x[x.Length()] < p_maxLambda) {
    bool accept = true;

    if (fabs(h) <= c_hmin) {
      return;
    }

    // Predictor step
    for (int k = 1; k <= x.Length(); k++) {
      u[k] = x[k] + h * p_omega * t[k];
    }

    double decel = 1.0 / g_maxDecel;  // initialize deceleration factor
    QreJacobian(p_start.GetGame(), equations, u, b);
    QRDecomp(b, q);

    int iter = 1;
    double disto = 0.0;
    while (true) {
      double dist;

      QreLHS(p_start.GetGame(), equations, u, y);

      /*
      std::cout << "LHS: ";
      for (int i = 1; i <= y.Length(); i++) {
	std::cout << y[i] << " ";
      }
      std::cout << std::endl;
      */

      NewtonStep(q, b, u, y, dist);
      /*
      std::cout << "Newt: ";
      std::cout << u[u.Length()] << " ";
      for (int i = 1; i < u.Length(); i++) {
	if (isLog[i]) {
	  std::cout << exp(u[i]) << " ";
	}
	else {
	  std::cout << u[i] << " ";
	}
      }
      std::cout << std::endl;

      std::cout << "dist: " << dist << std::endl;
      */

      if (dist >= c_maxDist) {
	accept = false;
	break;
      }
      
      decel = max(decel, sqrt(dist / c_maxDist) * g_maxDecel);
      if (iter >= 2) {
	double contr = dist / (disto + c_tol * c_eta);
	if (contr > c_maxContr) {
	  accept = false;
	  break;
	}
	decel = max(decel, sqrt(contr / c_maxContr) * g_maxDecel);
      }

      if (dist <= c_tol) {
	// Success; break out of iteration
	break;
      }
      disto = dist;
      iter++;
      if (iter > c_maxIter) {
	return;
      }
    }

    if (!accept) {
      h /= g_maxDecel;   // PC not accepted; change stepsize and retry
      if (fabs(h) <= c_hmin) {
	return;
      }

      continue;
    }

    // Determine new stepsize
    if (decel > g_maxDecel) {
      decel = g_maxDecel;
    }

    if ((!newton && g_targetLambda > 0.0) &&
	((x[x.Length()] - g_targetLambda) * 
	 (u[u.Length()] - g_targetLambda)) < 0.0) {
      newton = true;
    }

    if (newton) {
      h *= -(u[u.Length()] - g_targetLambda) / (u[u.Length()] - x[x.Length()]);
    }
    else {
      h = fabs(h / decel);
    }

    // PC step was successful; update and iterate
    x = u;

    if (g_fullGraph) {
      PrintProfile(std::cout, p_start.GetSupport(), x);

      /*
      LogBehavProfile<double> current(p_start);
      for (int i = 1; i <= p_start.Length(); i++) {
	if (isLog[i]) {
	  current.SetLogProb(i, x[i]);
	}
	else {
	  current.SetProb(i, x[i]);
	}
      }
      PrintProfileDetail(std::cout, current);
      */
    }

    Vector<double> newT(t);
    q.GetRow(q.NumRows(), newT);  // new tangent
    if (t * newT < 0.0) {
      // Bifurcation detected; for now, just "jump over" and continue,
      // taking into account the change in orientation of the curve.
      // Someday, we need to do more here! :)
      printf("bifurcation detected!\n");
      p_omega = -p_omega;
    }
    t = newT;
  }

  if (!g_fullGraph) {
    PrintProfile(std::cout, p_start.GetSupport(), x, true);
  }
}

