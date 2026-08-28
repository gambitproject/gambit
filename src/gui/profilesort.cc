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

void ProfileSortOrder::Rebuild(int p_numProfiles, int p_numCols, const CompareFunc &p_compare)
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

  // Order on the sort column first, then on the profile as a whole, taking
  // the columns from left to right.  std::stable_sort leaves profiles which
  // compare equal throughout in the order in which they were computed, in
  // both sort directions.
  std::stable_sort(m_profiles.begin(), m_profiles.end(), [&](int p_left, int p_right) {
    const int direction = m_ascending ? 1 : -1;

    const int onSortColumn = p_compare(m_column, p_left, p_right);
    if (onSortColumn != 0) {
      return onSortColumn * direction < 0;
    }
    for (int col = 1; col <= p_numCols; ++col) {
      const int onColumn = p_compare(col, p_left, p_right);
      if (onColumn != 0) {
        return onColumn * direction < 0;
      }
    }
    return false;
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
  if (p_col != m_column) {
    return "";
  }
  return m_ascending ? " ▲" : " ▼";
}

} // namespace Gambit::GUI
