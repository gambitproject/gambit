//
// This file is part of Gambit
// Copyright (c) 1994-2024, The Gambit Project (http://www.gambit-project.org)
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

#include "gambit.h"

namespace Gambit {

namespace Nash {

//========================================================================
//                       Profile renderer classes
//========================================================================

template <class T> class StrategyProfileRenderer {
public:
  virtual ~StrategyProfileRenderer() = default;
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
  ~MixedStrategyRenderer() override = default;
  void Render(const MixedStrategyProfile<T> &p_profile,
		      const std::string &p_label = "NE") const override = 0;
  void Render(const MixedBehaviorProfile<T> &p_profile,
		      const std::string &p_label = "NE") const override
  { Render(p_profile.ToMixedProfile(), p_label); }
};

template <class T> 
class MixedStrategyNullRenderer : public MixedStrategyRenderer<T> {
public:
  ~MixedStrategyNullRenderer() override = default;
  void Render(const MixedStrategyProfile<T> &p_profile,
		      const std::string &p_label = "NE") const override { }
};

template <class T>
class MixedStrategyCSVRenderer : public MixedStrategyRenderer<T> {
public:
  explicit MixedStrategyCSVRenderer(std::ostream &p_stream, int p_numDecimals = 6)
    : m_stream(p_stream), m_numDecimals(p_numDecimals) { }
  ~MixedStrategyCSVRenderer() override = default;
  void Render(const MixedStrategyProfile<T> &p_profile,
		      const std::string &p_label = "NE") const override;

private:
  std::ostream &m_stream;
  int m_numDecimals;
};

template <class T>
class MixedStrategyDetailRenderer : public MixedStrategyRenderer<T> {
public:
  explicit MixedStrategyDetailRenderer(std::ostream &p_stream, int p_numDecimals = 6)
    : m_stream(p_stream), m_numDecimals(p_numDecimals) { }
  ~MixedStrategyDetailRenderer() override = default;
  void Render(const MixedStrategyProfile<T> &p_profile,
		      const std::string &p_label = "NE") const override;

private:
  std::ostream &m_stream;
  int m_numDecimals;
};

//
// Encapsulates the rendering of a behavior profile to various text formats.
//
template <class T> class BehavStrategyRenderer : public StrategyProfileRenderer<T> {
public:
  ~BehavStrategyRenderer() override = default;
  void Render(const MixedStrategyProfile<T> &p_profile,
		      const std::string &p_label = "NE") const override
  { Render(MixedBehaviorProfile<T>(p_profile), p_label); }
  void Render(const MixedBehaviorProfile<T> &p_profile,
		      const std::string &p_label = "NE") const override = 0;
};

template <class T> 
class BehavStrategyNullRenderer : public BehavStrategyRenderer<T> {
public:
  ~BehavStrategyNullRenderer() override = default;
  void Render(const MixedBehaviorProfile<T> &p_profile,
		      const std::string &p_label = "NE") const override { }
};

template <class T>
class BehavStrategyCSVRenderer : public BehavStrategyRenderer<T> {
public:
  explicit BehavStrategyCSVRenderer(std::ostream &p_stream, int p_numDecimals = 6)
    : m_stream(p_stream), m_numDecimals(p_numDecimals) { }
  ~BehavStrategyCSVRenderer() override = default;
  void Render(const MixedBehaviorProfile<T> &p_profile,
		      const std::string &p_label = "NE") const override;

private:
  std::ostream &m_stream;
  int m_numDecimals;
};

template <class T>
class BehavStrategyDetailRenderer : public BehavStrategyRenderer<T> {
public:
  explicit BehavStrategyDetailRenderer(std::ostream &p_stream, int p_numDecimals = 6)
    : m_stream(p_stream), m_numDecimals(p_numDecimals) { }
  ~BehavStrategyDetailRenderer() override = default;
  void Render(const MixedBehaviorProfile<T> &p_profile,
		      const std::string &p_label = "NE") const override;

private:
  std::ostream &m_stream;
  int m_numDecimals;
};

//------------------------------------------------------------------------
//                      Algorithm base classes
//------------------------------------------------------------------------

// Encapsulation of algorithms via the strategy pattern.

template <class T> class StrategySolver {
public:
  explicit StrategySolver(std::shared_ptr<StrategyProfileRenderer<T> > p_onEquilibrium = 0);
  virtual ~StrategySolver()  = default;

  virtual List<MixedStrategyProfile<T> > Solve(const Game &) const = 0;

protected:
  std::shared_ptr<StrategyProfileRenderer<T> > m_onEquilibrium;
};

template <class T> class BehavSolver {
public:
  explicit BehavSolver(std::shared_ptr<StrategyProfileRenderer<T> > p_onEquilibrium = 0);
  virtual ~BehavSolver()  = default;

  virtual List<MixedBehaviorProfile<T> > Solve(const Game &) const = 0;

protected:
  std::shared_ptr<StrategyProfileRenderer<T> > m_onEquilibrium;
};

//
// This is an adaptor class which allows a client expecting behavior profiles
// to call a solver which works in terms of strategy profiles
//
template <class T> class BehavViaStrategySolver : public BehavSolver<T> {
public:
  explicit BehavViaStrategySolver(std::shared_ptr<StrategySolver<T> > p_solver,
			 std::shared_ptr<StrategyProfileRenderer<T> > p_onEquilibrium = 0);
  ~BehavViaStrategySolver() override = default;

  List<MixedBehaviorProfile<T> > Solve(const Game &) const override;

protected:
  std::shared_ptr<StrategySolver<T> > m_solver;
};

template <class T> class SubgameBehavSolver : public BehavSolver<T> {
public:
  explicit SubgameBehavSolver(std::shared_ptr<BehavSolver<T> > p_solver,
		     std::shared_ptr<StrategyProfileRenderer<T> > p_onEquilibrium = 0);
  ~SubgameBehavSolver()  override = default;

  List<MixedBehaviorProfile<T> > Solve(const Game &) const override;

protected:
  std::shared_ptr<BehavSolver<T> > m_solver;

private:
  void SolveSubgames(const Game &p_game,
                     const DVector<T> &p_templateSolution,
                     const GameNode &n,
                     List<DVector<T> > &solns,
                     List<GameOutcome> &values) const;
};

//
// Exception raised when maximum number of equilibria to compute
// has been reached.  A convenience for unraveling a potentially
// deep recursion.
//
class EquilibriumLimitReached : public Exception {
public:
  ~EquilibriumLimitReached() noexcept override = default;
  const char *what() const noexcept override { return "Reached target number of equilibria"; }
};

}  // namespace Gambit::Nash
 
}  // namespace Gambit


#endif  // LIBGAMBIT_NASH_H
