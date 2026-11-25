//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/workspace/analysis.h
// Declaration of analysis storage classes
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

#ifndef GAMBIT_GUI_ANALYSIS_H
#define GAMBIT_GUI_ANALYSIS_H

#include <memory>
#include <optional>
#include <string>
#include <type_traits>

#include "games/stratspt.h"
#include "games/workspace.h"
#include "nashspec.h"

//
// This file contains classes which manage the output of analysis tools.
// Currently, one class is supported, which is simply a list of profiles
// output by, e.g., an algorithm to compute Nash equilibria.
// Future classes will be adapted to other types of output (e.g.,
// output of stable sets or convex components of equilibria, etc.)
//

namespace Gambit::GUI {

class GameWorkspace;

/// The action at position `p_index` (from 1) in the order of players, information sets and
/// actions; the column order of behavior profiles
GameAction ActionAt(const Game &p_game, int p_index);

class AnalysisOutput {
protected:
  const GameWorkspace *m_workspace;
  std::string m_description;
  std::optional<NashComputationSpec> m_computation;

public:
  /// @name Lifecycle
  //@{
  /// Construct a new output group for the game of a workspace
  explicit AnalysisOutput(const GameWorkspace *p_workspace) : m_workspace(p_workspace) {}
  /// Clean up an output group
  virtual ~AnalysisOutput() = default;
  //@}

  /// @name General data access
  //@{
  /// Get the description of the list, in UTF-8
  const std::string &GetDescription() const { return m_description; }

  /// Set the description of the list, in UTF-8
  void SetDescription(const std::string &p_desc) { m_description = p_desc; }

  /// Get the typed specification used to generate the list, if available
  const std::optional<NashComputationSpec> &GetComputationSpec() const { return m_computation; }

  /// Set the typed specification used to generate the list
  void SetComputationSpec(const NashComputationSpec &p_computation)
  {
    m_computation = p_computation;
  }

  /// The number of profiles in the list
  virtual int NumProfiles() const = 0;

  /// Get the index of the currently selected profile
  virtual int GetCurrent() const = 0;

  /// Set the index of the currently selected profile
  virtual void SetCurrent(int p_index) = 0;

  /// Are these behavior or strategy profiles natively?
  virtual bool IsBehavior() const = 0;

  /// Are these profiles computed in floating-point (as opposed to exact rational) precision?
  virtual bool IsFloatingPoint() const = 0;

  //@}

  virtual std::string GetPayoff(int pl, int p_index = -1) const = 0;

  virtual std::string GetRealizProb(const GameNode &, int p_index = -1) const = 0;
  virtual std::string GetBeliefProb(const GameNode &, int p_index = -1) const = 0;
  virtual std::string GetNodeValue(const GameNode &, int pl, int p_index = -1) const = 0;
  virtual std::string GetInfosetProb(const GameNode &, int p_index = -1) const = 0;
  virtual std::string GetInfosetValue(const GameNode &, int p_index = -1) const = 0;
  virtual std::string GetActionValue(const GameNode &, int act, int p_index = -1) const = 0;
  virtual std::string GetActionProb(const GameNode &, int act, int p_index = -1) const = 0;
  virtual std::string GetActionProb(int p_action, int p_index = -1) const = 0;
  virtual std::string GetActionProb(const GameAction &p_action, int p_index = -1) const = 0;
  virtual std::string GetStrategyProb(int p_strategy, int p_index = -1) const = 0;
  virtual std::string GetStrategyValue(int p_strategy, int p_index = -1) const = 0;

  /// Compare the probability of an action in two profiles, for ordering
  /// them; negative, zero, or positive as the probability in p_left is less
  /// than, equal to, or greater than that in p_right.  Probabilities are
  /// compared in the type the profiles are stored in, so that exact
  /// representations are ordered exactly.  A profile which does not define
  /// behavior at the action's information set orders after those which do.
  virtual int CompareActionProb(int p_action, int p_left, int p_right) const = 0;
  /// Compare the probability of a strategy in two profiles, as above
  virtual int CompareStrategyProb(int p_strategy, int p_left, int p_right) const = 0;

  virtual WorkspaceFile::Analysis Save() const = 0;
};

//!
//! This class manages a list of profiles, computed, e.g., by an
//! algorithm for finding Nash equilibria.  It maintains two lists
//! of profiles, one with the behavior representation and the other
//! with the mixed representation.
//!
template <class T> class AnalysisProfileList final : public AnalysisOutput {
  bool m_isBehav;
  int m_current{0};
  Array<std::shared_ptr<MixedBehaviorProfile<T>>> m_behavProfiles;
  // For an extensive game, the mixed profile of each behavior profile, derived only when first
  // needed: deriving it requires the reduced strategic form, which grows exponentially with the
  // tree
  mutable Array<std::shared_ptr<MixedStrategyProfile<T>>> m_mixedProfiles;

public:
  //!
  //! @name Lifecycle
  //!
  //@{
  /// Construct a new profile list for the game of a workspace
  AnalysisProfileList(const GameWorkspace *p_workspace, bool p_isBehav)
    : AnalysisOutput(p_workspace), m_isBehav(p_isBehav)
  {
  }
  /// Cleanup the profile list
  ~AnalysisProfileList() override = default;
  //@}

  //!
  //! @name General data access
  //!
  //@{
  /// Are these behavior or strategy profiles natively?
  bool IsBehavior() const override { return m_isBehav; }

  /// Are these profiles computed in floating-point (as opposed to exact rational) precision?
  bool IsFloatingPoint() const override { return std::is_same_v<T, double>; }

  /// The number of profiles in the list
  int NumProfiles() const override;

  std::string GetPayoff(int pl, int p_index = -1) const override;
  /// Return the realization probability of the node for the given
  /// profile.  (index == -1 for currently selected profile)
  std::string GetRealizProb(const GameNode &, int p_index = -1) const override;
  std::string GetBeliefProb(const GameNode &, int p_index = -1) const override;
  std::string GetNodeValue(const GameNode &, int pl, int p_index = -1) const override;
  std::string GetInfosetProb(const GameNode &, int p_index = -1) const override;
  std::string GetInfosetValue(const GameNode &, int p_index = -1) const override;
  std::string GetActionValue(const GameNode &, int act, int p_index = -1) const override;
  std::string GetActionProb(const GameNode &, int act, int p_index = -1) const override;
  std::string GetActionProb(int p_action, int p_index = -1) const override;
  std::string GetActionProb(const GameAction &p_action, int p_index = -1) const override;
  std::string GetStrategyProb(int p_strategy, int p_index = -1) const override;
  std::string GetStrategyValue(int p_strategy, int p_index = -1) const override;
  int CompareActionProb(int p_action, int p_left, int p_right) const override;
  int CompareStrategyProb(int p_strategy, int p_left, int p_right) const override;

  /// Get the index of the currently selected profile
  int GetCurrent() const override { return m_current; }

  /// Set the index of the currently selected profile
  void SetCurrent(int p_index) override { m_current = p_index; }

  //@}

  //!
  //! @name Adding profiles to the list
  //!
  //@{
  void AddProfile(const MixedStrategyProfile<T> &);
  void AddProfile(const MixedBehaviorProfile<T> &);

  /// Remove all profiles from the list
  void Clear();
  //@}

  //!
  //! @name Saving and loading profile lists
  //!
  //@{
  void Load(const WorkspaceFile::Analysis &p_analysis);
  WorkspaceFile::Analysis Save() const override;
  //@}

private:
  /// The mixed profile at position `p_index`, derived from its behavior profile if need be
  const MixedStrategyProfile<T> &GetMixedProfile(int p_index) const;
  /// The probability of an action in a profile, empty if the profile does
  /// not define behavior at the action's information set
  std::optional<T> GetActionProbEntry(int p_action, int p_index) const;
  /// The probability of a strategy in a profile
  std::optional<T> GetStrategyProbEntry(int p_strategy, int p_index) const;
};

//!
//! This class manages the "stack" of supports obtained by eliminating
//! dominated strategies from consideration.
//!
class StrategyDominanceStack {
  const GameWorkspace *m_workspace;
  bool m_strict;
  // Built only when first needed: the full support requires the reduced strategic form, which
  // grows exponentially with the tree of an extensive game
  mutable Array<std::shared_ptr<StrategySupportProfile>> m_supports;
  size_t m_current{0};
  bool m_noFurther{false};

  /// Builds the full support, at level 1, if it has not been built
  void EnsureFullSupport() const;

public:
  StrategyDominanceStack(const GameWorkspace *p_workspace, bool p_strict);
  ~StrategyDominanceStack() = default;

  //!
  //! Returns the number of supports in the stack
  //!
  int NumSupports() const
  {
    EnsureFullSupport();
    return m_supports.size();
  }

  //!
  //! Get the i'th support in the stack
  //! (where i=1 is always the "full" support)
  //!
  const StrategySupportProfile &GetSupport(int i) const
  {
    EnsureFullSupport();
    return *m_supports[i];
  }

  //!
  //! Get the current support
  //!
  const StrategySupportProfile &GetCurrent() const
  {
    EnsureFullSupport();
    return *m_supports[m_current];
  }

  //!
  //! Get the level of iteration (1 = no iteration)
  //!
  int GetLevel() const { return m_current; }

  //!
  //! Sets whether elimination is strict or weak.  If this changes the
  //! internal setting, a Reset() is done
  //!
  void SetStrict(bool p_strict);

  //!
  //! Gets whether elimination is strict or weak.
  //!
  bool GetStrict() const { return m_strict; }

  //!
  //! Reset the stack by clearing out all supports
  //!
  void Reset();

  //!
  //! Go to the next level of iteration.  Returns 'true' if successful,
  //! 'false' if no effect (i.e., no further actions could be eliminated)
  //!
  bool NextLevel();

  //!
  //! Go to the previous level of iteration.  Returns 'true' if successful,
  //! 'false' if no effect (i.e., already at the full support)
  //!
  bool PreviousLevel();

  //!
  //! Go to the top level (the full support)
  //!
  void TopLevel() { m_current = 1; }

  //!
  //! Returns 'false' if it is known that no further eliminations can be done
  //!
  bool CanEliminate() const { return (m_current < m_supports.size() || !m_noFurther); }
};

/// The analyses of a game: lists of computed profiles, and the iterated elimination of dominated
/// strategies
class AnalysisWorkspace {
  const GameWorkspace *m_workspace;

  StrategyDominanceStack m_stratSupports;

  Array<std::shared_ptr<AnalysisOutput>> m_profiles;
  int m_currentProfileList{0};

public:
  explicit AnalysisWorkspace(const GameWorkspace *p_workspace);

  void ResetForGameChange();
  void BuildNfg();

  const AnalysisOutput &GetProfiles() const { return *m_profiles[m_currentProfileList]; }
  const AnalysisOutput &GetProfiles(int p_index) const { return *m_profiles[p_index]; }
  void AddEquilibriumOutput(const std::shared_ptr<AnalysisOutput> &p_profs);
  void SelectEquilibriumOutput(int p_index);
  int NumProfileLists() const { return m_profiles.size(); }
  int GetCurrentProfileList() const { return m_currentProfileList; }

  int GetCurrentProfile() const
  {
    return (m_profiles.size() == 0) ? 0 : GetProfiles().GetCurrent();
  }
  void SelectProfile(int p_profile);

  const StrategySupportProfile &GetNfgSupport() const { return m_stratSupports.GetCurrent(); }
  void SetDominanceStrictness(bool p_strict);
  bool GetStrategyElimStrength() const;
  bool NextDominanceLevel();
  void PreviousDominanceLevel();
  void TopDominanceLevel();
  bool CanStrategyElim() const;
  int GetStrategyElimLevel() const;

  std::vector<WorkspaceFile::Analysis> Save() const;
  bool Load(const std::vector<WorkspaceFile::Analysis> &p_analyses);
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_ANALYSIS_H
