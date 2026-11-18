//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/labellist.h
// Editable list of labelled items, as staged by the game-editing dialogs
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

#ifndef GAMBIT_GUI_LABELLIST_H
#define GAMBIT_GUI_LABELLIST_H

#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace Gambit::GUI {

/// The outcome of checking the labels of a `LabelListEdit`
struct LabelValidation {
  enum class Problem { None, Empty, Duplicate };
  /// The problem with the first invalid row, or `None` if every label is valid
  Problem first{Problem::None};
  /// For each row, whether its label is invalid; always false for a deleted row
  std::vector<bool> invalid;
};

/// An edited list of labelled items (players, strategies, actions), staged until committed.
///
/// Each row has a *stable label*, which identifies it to the game: the item's label before the
/// edit, or a placeholder for an item added during the edit. The stable label never changes;
/// the row's current label is what the user has typed. An existing row is deleted by marking it,
/// so that it can be restored; a new row is removed outright. At least one row stays active.
///
/// `Payload` holds per-row data that moves with the row (a colour, a probability).
template <class Payload = std::monostate> class LabelListEdit {
public:
  struct Row {
    std::string stableLabel;
    std::string label;
    bool isNew{false};
    bool isDeleted{false};
    Payload payload{};
  };

  /// Appends a row for an item that exists before the edit
  void AddExisting(const std::string &p_label, Payload p_payload = Payload{})
  {
    m_rows.push_back(Row{p_label, p_label, false, false, std::move(p_payload)});
  }
  /// Appends a row for a new item, labelled `p_prefix` followed by the first number, counting
  /// from one past the number of rows, that gives a label no row has, stable or current
  void AddNew(const std::string &p_prefix, Payload p_payload = Payload{})
  {
    std::set<std::string> used;
    for (const auto &row : m_rows) {
      used.insert(row.stableLabel);
      used.insert(row.label);
    }
    size_t number = m_rows.size() + 1;
    while (used.contains(p_prefix + std::to_string(number))) {
      number++;
    }
    const std::string label = p_prefix + std::to_string(number);
    m_rows.push_back(Row{label, label, true, false, std::move(p_payload)});
  }

  size_t Size() const { return m_rows.size(); }
  const Row &GetRow(size_t p_index) const { return m_rows.at(p_index); }
  Row &GetRow(size_t p_index) { return m_rows.at(p_index); }
  size_t ActiveCount() const
  {
    return std::ranges::count_if(m_rows, [](const Row &r) { return !r.isDeleted; });
  }

  void SetLabel(size_t p_index, const std::string &p_label) { m_rows.at(p_index).label = p_label; }

  /// Marks an existing row deleted, or restores a deleted one; removes a new row outright.
  /// Returns false, changing nothing, if this would leave no active row.
  bool ToggleDeleted(size_t p_index)
  {
    Row &row = m_rows.at(p_index);
    if (!row.isDeleted && ActiveCount() <= 1) {
      return false;
    }
    if (row.isNew) {
      m_rows.erase(m_rows.begin() + static_cast<std::ptrdiff_t>(p_index));
    }
    else {
      row.isDeleted = !row.isDeleted;
    }
    return true;
  }
  /// Swaps a row with the one above it; returns false if it is the first row
  bool MoveUp(size_t p_index)
  {
    if (p_index == 0 || p_index >= m_rows.size()) {
      return false;
    }
    std::swap(m_rows[p_index - 1], m_rows[p_index]);
    return true;
  }
  /// Swaps a row with the one below it; returns false if it is the last row
  bool MoveDown(size_t p_index)
  {
    if (p_index + 1 >= m_rows.size()) {
      return false;
    }
    std::swap(m_rows[p_index], m_rows[p_index + 1]);
    return true;
  }

  /// Checks that the active rows' labels are nonempty and distinct
  LabelValidation Validate() const
  {
    LabelValidation result;
    result.invalid.resize(m_rows.size(), false);
    for (size_t i = 0; i < m_rows.size(); i++) {
      if (m_rows[i].isDeleted) {
        continue;
      }
      const std::string &label = m_rows[i].label;
      bool duplicate = false;
      for (size_t other = 0; !duplicate && other < m_rows.size(); other++) {
        duplicate = other != i && !m_rows[other].isDeleted && m_rows[other].label == label;
      }
      if (label.empty() || duplicate) {
        result.invalid[i] = true;
        if (result.first == LabelValidation::Problem::None) {
          result.first = label.empty() ? LabelValidation::Problem::Empty
                                       : LabelValidation::Problem::Duplicate;
        }
      }
    }
    return result;
  }

  /// The stable labels of the active rows, in order
  std::vector<std::string> GetStableLabels() const
  {
    std::vector<std::string> labels;
    for (const auto &row : m_rows) {
      if (!row.isDeleted) {
        labels.push_back(row.stableLabel);
      }
    }
    return labels;
  }
  /// The current labels of the active rows, in order
  std::vector<std::string> GetLabels() const
  {
    std::vector<std::string> labels;
    for (const auto &row : m_rows) {
      if (!row.isDeleted) {
        labels.push_back(row.label);
      }
    }
    return labels;
  }

private:
  std::vector<Row> m_rows;
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_LABELLIST_H
