//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlnash.h
// Dialog for selecting algorithms to compute Nash equilibria
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

#ifndef GAMBIT_GUI_DLNASH_H
#define GAMBIT_GUI_DLNASH_H

#include <optional>
#include <vector>

#include <wx/collpane.h>
#include <wx/listctrl.h>
#include <wx/radiobut.h>

#include "gamedoc.h"
#include "nashvariants.h"

namespace Gambit::GUI {
class NashChoiceDialog final : public wxDialog {
  std::shared_ptr<GameDocument> m_doc;
  wxRadioButton *m_goalButtons[3];
  wxStaticText *m_goalNote;
  wxTextCtrl *m_detail;
  wxCollapsiblePane *m_parameters;
  wxStaticText *m_parametersText;
  wxString m_parametersLabel;
  int m_bestHeight{0};
  wxListCtrl *m_list;
  std::vector<const NashVariant *> m_variants;
  std::vector<const NashVariant *> m_rows;
  const NashVariant *m_userChoice{nullptr};
  std::optional<std::size_t> m_contingencies;
  int m_sortColumn{-1};
  bool m_sortAscending{true};
  bool m_refreshing{false};
  wxString m_tip;

  void OnGoal(wxCommandEvent &);
  void OnItemSelected(wxListEvent &);
  void OnColumnClick(wxListEvent &);
  void OnListMotion(wxWindow *, wxMouseEvent &);
  void RefreshList();
  void UpdateRecommendation();
  void SelectRow(const NashVariant &);
  void UpdateDetail();
  void WrapParameters();
  void OnParametersToggled(wxCollapsiblePaneEvent &);
  wxString DescribeVariant(const NashVariant &);

  NashGoal GetGoal() const;
  const NashVariant &GetSelectedVariant() const;

  NashComputationSpec GetComputation() const;

public:
  NashChoiceDialog(wxWindow *, const std::shared_ptr<GameDocument> &);

  std::shared_ptr<AnalysisOutput> GetCommand() const;

  bool UseStrategic() const;
};
} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLNASH_H
