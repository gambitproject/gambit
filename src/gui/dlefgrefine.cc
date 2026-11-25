//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlefgrefine.cc
// Dialog for refining information sets by the action taken at a move
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
#include <optional>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP

#include "games.h"
#include "gamedoc.h"
#include "dialogstyle.h"

namespace {

using namespace Gambit;
using namespace Gambit::GUI;

class RefineInfosetsDialog final : public wxDialog {
  struct PlayerEntry {
    GamePlayer player;
    wxCheckBox *checkbox;
  };
  std::vector<PlayerEntry> m_entries;

  void OnCheckbox(wxCommandEvent &) { UpdateButtonState(); }
  void UpdateButtonState();

public:
  RefineInfosetsDialog(wxWindow *p_parent, const GameDocument &p_doc);
  std::vector<GamePlayer> GetPlayers() const;
};

RefineInfosetsDialog::RefineInfosetsDialog(wxWindow *p_parent, const GameDocument &p_doc)
  : wxDialog(p_parent, wxID_ANY, _("Refine information sets by this move"), wxDefaultPosition,
             wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
  auto *contentSizer = new wxBoxSizer(wxVERTICAL);

  auto *groupLabel =
      new wxStaticText(this, wxID_ANY, _("Players who observe the action taken at this move"));
  groupLabel->SetFont(groupLabel->GetFont().Bold());
  contentSizer->Add(groupLabel);

  const auto &players = p_doc.GetGame()->GetPlayers();
  m_entries.reserve(players.size());

  for (const auto &player : players) {
    wxString label;
    if (!player->GetLabel().empty()) {
      label << wxString::FromUTF8(player->GetLabel());
    }
    else {
      label << _("Player ") << player->GetNumber();
    }
    auto *cb = new wxCheckBox(this, wxID_ANY, label);
    cb->SetValue(true);
    cb->Bind(wxEVT_CHECKBOX, &RefineInfosetsDialog::OnCheckbox, this);
    m_entries.push_back({.player = player, .checkbox = cb});

    auto *row = new wxBoxSizer(wxHORIZONTAL);
    row->Add(new wxStaticBitmap(this, wxID_ANY, MakeColorSwatch(p_doc.GetPlayerColor(player))), 0,
             wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(c_spaceTight));
    row->Add(cb, 1, wxALIGN_CENTER_VERTICAL);
    contentSizer->Add(row, 0, wxEXPAND | wxTOP, FromDIP(c_spaceRelated));
  }

  auto *note = new wxStaticText(
      this, wxID_ANY,
      _("Information sets below this node are refined now; does not affect subsequent edits."));
  contentSizer->Add(note, 0, wxTOP, FromDIP(c_spaceMargin));

  SetDialogLayout(this, contentSizer, wxOK | wxCANCEL);
  if (auto *ok = dynamic_cast<wxButton *>(FindWindow(wxID_OK))) {
    ok->SetLabel(_("Refine"));
  }
  RefreshDialogMinSize(this);
  UpdateButtonState();
}

void RefineInfosetsDialog::UpdateButtonState()
{
  const bool anyChecked = std::ranges::any_of(
      m_entries, [](const PlayerEntry &entry) { return entry.checkbox->IsChecked(); });
  FindWindow(wxID_OK)->Enable(anyChecked);
}

std::vector<GamePlayer> RefineInfosetsDialog::GetPlayers() const
{
  std::vector<GamePlayer> result;
  result.reserve(m_entries.size());
  for (const auto &[player, checkbox] : m_entries) {
    if (checkbox->GetValue()) {
      result.push_back(player);
    }
  }
  return result;
}
} // anonymous namespace

namespace Gambit::GUI {

std::optional<std::vector<GamePlayer>> ChooseRefiningPlayers(wxWindow *p_parent,
                                                             const GameDocument &p_doc)
{
  if (RefineInfosetsDialog dialog(p_parent, p_doc); dialog.ShowModal() == wxID_OK) {
    return dialog.GetPlayers();
  }
  return std::nullopt;
}

} // namespace Gambit::GUI
