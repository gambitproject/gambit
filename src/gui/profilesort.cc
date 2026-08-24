//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/profilesort.cc
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

#include <algorithm>
#include <numeric>

#include "profilesort.h"

namespace Gambit::GUI {

namespace {

/// The sort key of a profile: the entry in the sort column, followed by
/// the entries of the profile in column order.  Undefined entries are
/// represented by an empty optional, and sort after all defined entries.
using SortKey = std::vector<std::optional<double>>;

bool EntryLess(const std::optional<double> &p_left, const std::optional<double> &p_right)
{
  if (!p_left.has_value()) {
    return false;
  }
  if (!p_right.has_value()) {
    return true;
  }
  return p_left.value() < p_right.value();
}

bool KeyLess(const SortKey &p_left, const SortKey &p_right)
{
  return std::lexicographical_compare(p_left.begin(), p_left.end(), p_right.begin(), p_right.end(),
                                      EntryLess);
}

} // end anonymous namespace

void ProfileSortOrder::ToggleColumn(int p_col)
{
  if (p_col == m_column) {
    m_ascending = !m_ascending;
  }
  else {
    m_column = p_col;
    m_ascending = true;
  }
}

void ProfileSortOrder::Reset()
{
  m_column = 0;
  m_ascending = true;
}

void ProfileSortOrder::Rebuild(int p_numProfiles, int p_numCols, const EntryFunc &p_entry)
{
  m_profiles.resize(p_numProfiles);
  std::iota(m_profiles.begin(), m_profiles.end(), 1);

  if (m_column < 1 || m_column > p_numCols) {
    // Either the list is unsorted, or the column sorted on is no longer
    // part of the game; either way, show the profiles as computed.
    m_column = 0;
    m_ascending = true;
    return;
  }

  std::vector<SortKey> keys(p_numProfiles);
  for (int profile = 1; profile <= p_numProfiles; ++profile) {
    SortKey &key = keys[profile - 1];
    key.reserve(p_numCols + 1);
    key.push_back(p_entry(m_column, profile));
    for (int col = 1; col <= p_numCols; ++col) {
      key.push_back(p_entry(col, profile));
    }
  }

  // std::stable_sort keeps profiles whose keys are equal in the order in
  // which they were computed, in both sort directions.
  std::stable_sort(m_profiles.begin(), m_profiles.end(), [&](int p_left, int p_right) {
    const SortKey &left = keys[p_left - 1];
    const SortKey &right = keys[p_right - 1];
    return m_ascending ? KeyLess(left, right) : KeyLess(right, left);
  });
}

int ProfileSortOrder::GetProfile(int p_row) const
{
  if (p_row < 0 || p_row >= static_cast<int>(m_profiles.size())) {
    return 0;
  }
  return m_profiles[p_row];
}

int ProfileSortOrder::GetRow(int p_profile) const
{
  const auto row = std::find(m_profiles.begin(), m_profiles.end(), p_profile);
  return (row == m_profiles.end()) ? -1 : static_cast<int>(row - m_profiles.begin());
}

const char *ProfileSortOrder::GetColumnMarker(int p_col) const
{
  if (m_profiles.size() < 2) {
    // With at most one profile there is nothing to sort, so offering to
    // sort would only be a distraction.
    return "";
  }
  if (p_col != m_column) {
    // A light marker on the other columns, so that it is apparent that
    // the list can be sorted on any of them.
    return " ↕";
  }
  return m_ascending ? " ▲" : " ▼";
}

} // namespace Gambit::GUI
