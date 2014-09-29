//
// This file is part of Gambit
// Copyright (c) 1994-2014, The Gambit Project (http://www.gambit-project.org)
//
// FILE: src/libgambit/nash.h
// Framework for computing (sub)sets of Nash equilibria.
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

#ifndef LIBGAMBIT_NASH_H
#define LIBGAMBIT_NASH_H

#include "libgambit.h"

namespace Gambit {

//========================================================================
//                       Profile renderer classes
//========================================================================

template <class T> class StrategyProfileRenderer {
public:
  virtual ~StrategyProfileRenderer() { }
  virtual void Render(const MixedStrategyProfile<T> &p_profile,
		      const std::string &p_label = "NE") const = 0;
  virtual void Render(const MixedBehaviorProfile<T> &p_profile,
		      const std::string &p_label = "NE") const = 0;
};

//
// Encapsulates the rendering of a mixed strategy profile to various text formats.
// Implements automatic conversion of behavior strategy profiles to mixed
// strategy profiles.
//
template <class T> class MixedStrategyRenderer : public StrategyProfileRenderer<T> {
public:
  virtual ~MixedStrategyRenderer() { }
  virtual void Render(const MixedStrategyProfile<T> &p_profile,
		      const std::string &p_label = "NE") const = 0;
  virtual void Render(const MixedBehaviorProfile<T> &p_profile,
		      const std::string &p_label = "NE") const
  { Render(p_profile.ToMixedProfile(), p_label); }
};

template <class T> 
class MixedStrategyNullRenderer : public MixedStrategyRenderer<T> {
public:
  virtual ~MixedStrategyNullRenderer() { }
  virtual void Render(const MixedStrategyProfile<T> &p_profile,
		      const std::string &p_label = "NE") const { }
};

template <class T>
class MixedStrategyCSVRenderer : public MixedStrategyRenderer<T> {
public:
  MixedStrategyCSVRenderer(std::ostream &p_stream, int p_numDecimals = 6)
    : m_stream(p_stream), m_numDecimals(p_numDecimals) { }
  virtual ~MixedStrategyCSVRenderer() { }
  virtual void Render(const MixedStrategyProfile<T> &p_profile,
		      const std::string &p_label = "NE") const;

private:
  std::ostream &m_stream;
  int m_numDecimals;
};

template <class T>
class MixedStrategyDetailRenderer : public MixedStrategyRenderer<T> {
public:
  MixedStrategyDetailRenderer(std::ostream &p_stream, int p_numDecimals = 6)
    : m_stream(p_stream), m_numDecimals(p_numDecimals) { }
  virtual ~MixedStrategyDetailRenderer() { }
  virtual void Render(const MixedStrategyProfile<T> &p_profile,
		      const std::string &p_label = "NE") const;

private:
  std::ostream &m_stream;
  int m_numDecimals;
};

//
// Encapsulates the rendering of a behavior profile to various text formats.
//
template <class T> class BehavStrategyRenderer : public StrategyProfileRenderer<T> {
public:
  virtual ~BehavStrategyRenderer() { }
  virtual void Render(const MixedStrategyProfile<T> &p_profile,
		      const std::string &p_label = "NE") const
  { Render(MixedBehaviorProfile<T>(p_profile), p_label); }
  virtual void Render(const MixedBehaviorProfile<T> &p_profile,
		      const std::string &p_label = "NE") const = 0;
};

template <class T> 
class BehavStrategyNullRenderer : public BehavStrategyRenderer<T> {
public:
  virtual ~BehavStrategyNullRenderer() { }
  virtual void Render(const MixedBehaviorProfile<T> &p_profile,
		      const std::string &p_label = "NE") const { }
};

template <class T>
class BehavStrategyCSVRenderer : public BehavStrategyRenderer<T> {
public:
  BehavStrategyCSVRenderer(std::ostream &p_stream, int p_numDecimals = 6) 
    : m_stream(p_stream), m_numDecimals(p_numDecimals) { }
  virtual ~BehavStrategyCSVRenderer() { }
  virtual void Render(const MixedBehaviorProfile<T> &p_profile,
		      const std::string &p_label = "NE") const;

private:
  std::ostream &m_stream;
  int m_numDecimals;
};

template <class T>
class BehavStrategyDetailRenderer : public BehavStrategyRenderer<T> {
public:
  BehavStrategyDetailRenderer(std::ostream &p_stream, int p_numDecimals = 6)
    : m_stream(p_stream), m_numDecimals(p_numDecimals) { }
  virtual ~BehavStrategyDetailRenderer() { }
  virtual void Render(const MixedBehaviorProfile<T> &p_profile,
		      const std::string &p_label = "NE") const;

private:
  std::ostream &m_stream;
  int m_numDecimals;
};

//------------------------------------------------------------------------
//                      Algorithm base classes
//------------------------------------------------------------------------

// Encapsulation of algorithms via the strategy pattern.

template <class T> class NashStrategySolver {
public:
  NashStrategySolver(shared_ptr<StrategyProfileRenderer<T> > p_onEquilibrium = 0);
  virtual ~NashStrategySolver()  { }

  virtual List<MixedStrategyProfile<T> > Solve(const Game &) const = 0;

protected:
  shared_ptr<StrategyProfileRenderer<T> > m_onEquilibrium;
};

template <class T> class NashBehavSolver {
public:
  NashBehavSolver(shared_ptr<StrategyProfileRenderer<T> > p_onEquilibrium = 0);
  virtual ~NashBehavSolver()  { }

  virtual List<MixedBehaviorProfile<T> > Solve(const BehaviorSupportProfile &) const = 0;

protected:
  shared_ptr<StrategyProfileRenderer<T> > m_onEquilibrium;
};

//
// This is an adaptor class which allows a client expecting behavior profiles
// to call a solver which works in terms of strategy profiles
//
template <class T> class NashBehavViaStrategySolver : public NashBehavSolver<T> {
public:
  NashBehavViaStrategySolver(shared_ptr<NashStrategySolver<T> > p_solver,
			     shared_ptr<StrategyProfileRenderer<T> > p_onEquilibrium = 0);
  virtual ~NashBehavViaStrategySolver() { }

  virtual List<MixedBehaviorProfile<T> > Solve(const BehaviorSupportProfile &) const;

protected:
  shared_ptr<NashStrategySolver<T> > m_solver;
};

template <class T> class SubgameNashBehavSolver : public NashBehavSolver<T> {
public:
  SubgameNashBehavSolver(shared_ptr<NashBehavSolver<T> > p_solver,
			 shared_ptr<StrategyProfileRenderer<T> > p_onEquilibrium = 0);
  virtual ~SubgameNashBehavSolver()  { }

  virtual List<MixedBehaviorProfile<T> > Solve(const BehaviorSupportProfile &) const;

protected:
  shared_ptr<NashBehavSolver<T> > m_solver;

private:
  void SolveSubgames(const BehaviorSupportProfile &p_support,
		     const DVector<T> &p_templateSolution,
		     GameNode n,
		     List<DVector<T> > &solns,
		     List<GameOutcome> &values) const;
};

//
// Exception raised when maximum number of equilibria to compute
// has been reached.  A convenience for unraveling a potentially
// deep recursion.
//
class NashEquilibriumLimitReached : public Exception {
public:
  virtual ~NashEquilibriumLimitReached() throw() { }
  const char *what(void) const throw() { return "Reached target number of equilibria"; }
};

}  // namespace Gambit

#endif  // LIBGAMBIT_NASH_H
