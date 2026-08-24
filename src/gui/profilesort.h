//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/profilesort.h
// Ordering of profiles in the profile list windows
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

#ifndef GAMBIT_GUI_PROFILESORT_H
#define GAMBIT_GUI_PROFILESORT_H

#include <functional>
#include <optional>
#include <vector>

namespace Gambit::GUI {

//!
//! Maintains the order in which the profiles of an analysis output are
//! displayed in a profile list window.
//!
//! Profiles and columns are both numbered from one, following the
//! convention of the analysis classes; display rows are numbered from
//! zero, following the convention of wxGrid.  With no sort column set,
//! profiles are shown in the order in which they were computed.
//!
//! Sorting is on the probability in the sort column, with ties broken by
//! comparing the profiles entry-by-entry from the first column onwards,
//! and any remaining ties broken by the order of computation.  Sorting on
//! the first column therefore orders the list lexicographically by
//! profile.  Entries which are not defined for a profile (for instance,
//! the behavior at an information set which the profile does not reach)
//! sort after all defined entries.
//!
class ProfileSortOrder {
public:
  /// Returns the entry in column p_col of profile p_profile, if defined
  using EntryFunc = std::function<std::optional<double>(int p_col, int p_profile)>;

  /// @name Selecting the sort
  //@{
  /// Sort on p_col; selecting the current sort column reverses the direction
  void ToggleColumn(int p_col);

  /// Restore the order in which the profiles were computed
  void Reset();

  /// The column being sorted on, or zero if the list is unsorted
  int GetColumn() const { return m_column; }

  /// Is the sort in ascending order?
  bool IsAscending() const { return m_ascending; }
  //@}

  /// @name Applying the sort
  //@{
  /// Recompute the ordering of p_numProfiles profiles of p_numCols entries
  void Rebuild(int p_numProfiles, int p_numCols, const EntryFunc &p_entry);

  /// The profile displayed in row p_row, or zero if there is no such row
  int GetProfile(int p_row) const;

  /// The row in which p_profile is displayed, or -1 if there is no such profile
  int GetRow(int p_profile) const;

  /// The marker to append to the label of column p_col, as UTF-8: an arrow
  /// showing the direction of the sort on the column being sorted on, and a
  /// hint that sorting is possible on the others.  Empty when there are
  /// fewer than two profiles, as there is then nothing to sort.
  const char *GetColumnMarker(int p_col) const;
  //@}

private:
  int m_column{0};
  bool m_ascending{true};
  /// The profile shown in each row, in display order
  std::vector<int> m_profiles;
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_PROFILESORT_H
