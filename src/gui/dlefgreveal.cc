//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlefgreveal.cc
// Dialog for revealing actions to players
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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP

#include "gambit.h"
#include "dlefgreveal.h"

namespace Gambit::GUI {

class RevealMoveDialog final : public wxDialog {
  GameDocument *m_doc{nullptr};

  struct PlayerEntry {
    GamePlayer player;
    wxCheckBox *checkbox;
  };

  std::vector<PlayerEntry> m_entries;

public:
  RevealMoveDialog(wxWindow *p_parent, GameDocument *p_doc);
  std::vector<GamePlayer> GetPlayers() const;
};

RevealMoveDialog::RevealMoveDialog(wxWindow *p_parent, GameDocument *p_doc)
  : wxDialog(p_parent, wxID_ANY, _("Reveal this move to players"), wxDefaultPosition,
             wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_doc(p_doc)
{
  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *playerBox = new wxStaticBoxSizer(wxVERTICAL, this, _("Reveal the move to players"));

  const auto &players = m_doc->GetGame()->GetPlayers();
  m_entries.reserve(players.size());

  for (const auto &player : players) {
    wxString label;
    if (!player->GetLabel().empty()) {
      label = wxString::FromUTF8(player->GetLabel());
    }
    else {
      label = wxString::Format("Player %u", player->GetNumber());
    }
    auto *cb = new wxCheckBox(this, wxID_ANY, label);
    cb->SetValue(true);
    cb->SetForegroundColour(m_doc->GetStyle().GetPlayerColor(player));

    m_entries.push_back({player, cb});
    playerBox->Add(cb, wxSizerFlags().Expand().Border(wxALL, 2));
  }

  topSizer->Add(playerBox, wxSizerFlags(1).Expand().Border());

  auto *buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL);
  topSizer->Add(buttonSizer, wxSizerFlags().Right().Border());

  SetSizerAndFit(topSizer);
  CenterOnParent();
}

std::vector<GamePlayer> RevealMoveDialog::GetPlayers() const
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

std::optional<std::vector<GamePlayer>> RevealMove(wxWindow *p_parent, GameDocument *p_doc)
{
  RevealMoveDialog dialog(p_parent, p_doc);
  if (dialog.ShowModal() == wxID_OK) {
    return dialog.GetPlayers();
  }
  return std::nullopt;
}

} // namespace Gambit::GUI
