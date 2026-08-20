//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlgameprop.cc
// Dialog for viewing and editing properties of a game
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

#include <functional>
#include <vector>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/notebook.h>
#include <wx/colourdata.h>
#ifdef __WXMAC__
#include <wx/generic/colrdlgg.h>
#else
#include <wx/colordlg.h>
#endif

#include "gamedoc.h"
#include "dlgameprop.h"
#include "editlabel.h"

namespace Gambit::GUI {

namespace {
const wxColour kInvalidLabelBg(255, 220, 220);

// wxGenericColourDialog, not the platform's native wxColourDialog, on macOS/Cocoa only:
// its native colour panel has no real Cancel -- any interaction with it commits live,
// with no way back short of canceling this entire properties dialog. The generic dialog
// draws its own honest OK/Cancel buttons. Elsewhere, the native dialog's Cancel already
// works properly, and the generic dialog's header isn't guaranteed to be available (it
// isn't shipped with the GTK/MSW dev packages used by CI, only bundled on macOS/Homebrew).
#ifdef __WXMAC__
using PlatformColourDialog = wxGenericColourDialog;
#else
using PlatformColourDialog = wxColourDialog;
#endif
} // namespace

//========================================================================
//                      class PlayerLabelPanel
//========================================================================

// A fixed list of the game's players (plus chance), each with a color swatch
// button that opens a color picker; every player but chance also gets an
// editable label (chance's label is reserved and immutable). Adding, deleting,
// and reordering players is not supported here yet.
class PlayerLabelPanel final : public wxPanel {
public:
  struct Row {
    GamePlayer player;
    wxColour color; // staged: not written to the document until the dialog's OK is committed
    LabelTextCtrl *labelCtrl = nullptr; // null for the chance row -- its label isn't editable
    wxBitmapButton *colorButton = nullptr;
  };

private:
  std::shared_ptr<GameDocument> m_doc;
  std::vector<Row> m_rows;
  wxColour m_defaultBg;
  std::function<void()> m_onChanged;

  void OnSetColor(int p_index);
  // Builds a fresh color-swatch button for a row and swaps it in for the old one, rather
  // than mutating the existing button's bitmap in place -- see OnSetColor() for why.
  void RecreateColorButton(int p_index);
  wxBitmapButton *MakeColorButton(int p_index);

public:
  PlayerLabelPanel(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc,
                   const std::function<void()> &p_onChanged);

  int NumRows() const { return static_cast<int>(m_rows.size()); }
  GamePlayer GetPlayer(int p_index) const { return m_rows.at(p_index).player; }
  wxString GetPlayerLabel(int p_index) const
  {
    const Row &row = m_rows.at(p_index);
    return row.labelCtrl ? row.labelCtrl->GetNormalizedValue()
                         : wxString::FromUTF8(row.player->GetLabel());
  }
  wxColour GetPlayerColor(int p_index) const { return m_rows.at(p_index).color; }

  // Highlights any empty or duplicate player labels, and returns a description
  // of the first problem found, or an empty string if all are valid. Chance's
  // (uneditable) row is never a source of invalidity.
  wxString ValidateLabels();
};

PlayerLabelPanel::PlayerLabelPanel(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc,
                                   const std::function<void()> &p_onChanged)
  : wxPanel(p_parent, wxID_ANY), m_doc(p_doc), m_onChanged(p_onChanged)
{
  auto *gridSizer = new wxFlexGridSizer(3, FromDIP(5), FromDIP(10));
  gridSizer->AddGrowableCol(2, 1);

  for (const auto &player : m_doc->GetGame()->GetPlayersWithChance()) {
    Row row;
    row.player = player;
    row.color = m_doc->GetStyle().GetPlayerColor(player);
    m_rows.push_back(row);

    const int index = static_cast<int>(m_rows.size()) - 1;
    Row &stored = m_rows[index];

    stored.colorButton = MakeColorButton(index);
    // The border keeps the swatch's spacing from the panel edge sizer-owned rather than
    // relying on the button's own native chrome, which can shift after the button is
    // recreated (see RecreateColorButton()) if left to the platform to decide.
    gridSizer->Add(stored.colorButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(5));

    wxString number;
    if (!player->IsChance()) {
      number << player->GetNumber();
    }
    gridSizer->Add(new wxStaticText(this, wxID_STATIC, number), 0,
                   wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);

    if (player->IsChance()) {
      // Chance's label is reserved and can't be changed (GameRep::RelabelPlayers rejects it
      // as a key) -- just display it.
      gridSizer->Add(new wxStaticText(this, wxID_STATIC, wxString::FromUTF8(player->GetLabel())),
                     1, wxALIGN_CENTER_VERTICAL);
    }
    else {
      stored.labelCtrl = new LabelTextCtrl(this, wxID_ANY, wxString::FromUTF8(player->GetLabel()));
      m_defaultBg = stored.labelCtrl->GetBackgroundColour();
      stored.labelCtrl->Bind(wxEVT_TEXT, [this](wxCommandEvent &p_event) {
        m_onChanged();
        p_event.Skip();
      });
      stored.labelCtrl->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent &p_event) {
        m_onChanged();
        p_event.Skip();
      });
      gridSizer->Add(stored.labelCtrl, 1, wxEXPAND);
    }
  }

  SetSizer(gridSizer);
}

wxBitmapButton *PlayerLabelPanel::MakeColorButton(int p_index)
{
  Row &row = m_rows.at(p_index);
  auto *button = new wxBitmapButton(this, wxID_ANY, MakeColorSwatch(row.color), wxDefaultPosition,
                                    wxDefaultSize, wxNO_BORDER);
  button->SetToolTip(_("Change the color for this player"));
  button->Bind(wxEVT_BUTTON, [this, p_index](wxCommandEvent &) { OnSetColor(p_index); });
  return button;
}

void PlayerLabelPanel::OnSetColor(int p_index)
{
  Row &row = m_rows.at(p_index);

  wxColourData data;
  data.SetColour(row.color);
  PlatformColourDialog dialog(this, &data);
  wxString title;
  if (row.player->IsChance()) {
    title = _("Choose color for chance");
  }
  else {
    title << _("Choose color for player ") << row.player->GetNumber();
  }
  dialog.SetTitle(title);

  if (dialog.ShowModal() != wxID_OK) {
    return;
  }

  // Staged only -- not written to the document until the dialog's OK is committed
  // (by GameFrame::OnEditGame), so canceling the dialog leaves colors untouched.
  row.color = dialog.GetColourData().GetColour();

  // Recreated (rather than mutated in place via SetBitmap()) because SetBitmap() on an
  // existing wxBitmapButton visibly shifted its rendered content on this platform.
  // Deferred via CallAfter(): this destroys the very button whose click invoked this
  // handler, which is still on the call stack -- see ActionPanel::AddAction() in
  // dleditmove.cc for the same hazard.
  CallAfter([this, p_index]() { RecreateColorButton(p_index); });
}

void PlayerLabelPanel::RecreateColorButton(int p_index)
{
  Row &row = m_rows.at(p_index);

  wxBitmapButton *oldButton = row.colorButton;
  wxBitmapButton *newButton = MakeColorButton(p_index);

  GetSizer()->Replace(oldButton, newButton);
  oldButton->Destroy();
  row.colorButton = newButton;

  Layout();
}

wxString PlayerLabelPanel::ValidateLabels()
{
  const int numRows = NumRows();
  wxString message;

  for (int i = 0; i < numRows; i++) {
    if (!m_rows[i].labelCtrl) {
      continue; // chance's row has no editable label to validate
    }
    const wxString value = m_rows[i].labelCtrl->GetValue();
    bool invalid = value.empty();
    for (int other = 0; !invalid && other < numRows; other++) {
      if (other != i && m_rows[other].labelCtrl && m_rows[other].labelCtrl->GetValue() == value) {
        invalid = true;
      }
    }

    m_rows[i].labelCtrl->SetBackgroundColour(invalid ? kInvalidLabelBg : m_defaultBg);
    m_rows[i].labelCtrl->Refresh();

    if (invalid && message.empty()) {
      message =
          value.empty() ? _("Player labels cannot be empty.") : _("Player labels must be unique.");
    }
  }
  return message;
}

//========================================================================
//                   class GamePropertiesDialog
//========================================================================

GamePropertiesDialog::GamePropertiesDialog(wxWindow *p_parent,
                                           const std::shared_ptr<GameDocument> &p_doc)
  : wxDialog(p_parent, wxID_ANY, _("Game properties"), wxDefaultPosition, wxDefaultSize,
             wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    m_doc(p_doc)
{
  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *notebook = new wxNotebook(this, wxID_ANY);

  auto *generalPanel = new wxPanel(notebook);
  auto *generalSizer = new wxBoxSizer(wxVERTICAL);

  auto *titleSizer = new wxBoxSizer(wxHORIZONTAL);
  titleSizer->Add(new wxStaticText(generalPanel, wxID_STATIC, _("Title")), 0,
                  wxALL | wxALIGN_CENTER, 5);
  m_title =
      new wxTextCtrl(generalPanel, wxID_ANY, wxString::FromUTF8(m_doc->GetGame()->GetTitle()),
                     wxDefaultPosition, wxSize(400, -1));
  titleSizer->Add(m_title, 1, wxALL | wxALIGN_CENTER, 5);
  generalSizer->Add(titleSizer, 0, wxALL | wxEXPAND, 0);

  auto *commentSizer = new wxBoxSizer(wxHORIZONTAL);
  commentSizer->Add(new wxStaticText(generalPanel, wxID_STATIC, _("Comment")), 0,
                    wxALL | wxALIGN_CENTER, 5);
  m_comment = new wxTextCtrl(generalPanel, wxID_ANY,
                             wxString::FromUTF8(m_doc->GetGame()->GetDescription()),
                             wxDefaultPosition, wxSize(400, -1), wxTE_MULTILINE);
  commentSizer->Add(m_comment, 1, wxALL | wxALIGN_CENTER, 5);
  generalSizer->Add(commentSizer, 1, wxALL | wxEXPAND, 0);

  auto *boxSizer =
      new wxStaticBoxSizer(wxVERTICAL, generalPanel, _("Information about this game"));

  boxSizer->Add(new wxStaticText(generalPanel, wxID_STATIC,
                                 wxString(_("Filename: ")) + m_doc->GetFilename()),
                0, wxALL, 5);

  const Game game = m_doc->GetGame();
  if (game->IsConstSum()) {
    boxSizer->Add(new wxStaticText(generalPanel, wxID_STATIC, _("This is a constant-sum game")), 0,
                  wxALL, 5);
  }
  else {
    boxSizer->Add(
        new wxStaticText(generalPanel, wxID_STATIC, _("This is not a constant-sum game")), 0,
        wxALL, 5);
  }

  if (game->IsTree()) {
    if (game->IsPerfectRecall()) {
      boxSizer->Add(
          new wxStaticText(generalPanel, wxID_STATIC, _("This is a game of perfect recall")), 0,
          wxALL, 5);
    }
    else {
      boxSizer->Add(
          new wxStaticText(generalPanel, wxID_STATIC, _("This is not a game of perfect recall")),
          0, wxALL, 5);
    }
  }

  generalSizer->Add(boxSizer, 0, wxALL | wxEXPAND, 5);
  generalPanel->SetSizer(generalSizer);
  notebook->AddPage(generalPanel, _("General"));

  m_playerPanel = new PlayerLabelPanel(notebook, m_doc, [this]() { UpdateValidation(); });
  notebook->AddPage(m_playerPanel, _("Players"));

  topSizer->Add(notebook, 1, wxALL | wxEXPAND, 5);

  m_errorText = new wxStaticText(this, wxID_STATIC, wxEmptyString);
  m_errorText->SetForegroundColour(*wxRED);
  topSizer->Add(m_errorText, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);

  auto *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  auto *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);

  topSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  wxTopLevelWindowBase::Layout();
  CenterOnParent();

  UpdateValidation();
}

void GamePropertiesDialog::UpdateValidation()
{
  const wxString message = m_playerPanel->ValidateLabels();
  m_errorText->SetLabel(message);
  if (auto *ok = FindWindow(wxID_OK)) {
    ok->Enable(message.empty());
  }
}

int GamePropertiesDialog::NumRows() const { return m_playerPanel->NumRows(); }

GamePlayer GamePropertiesDialog::GetPlayer(int p_index) const
{
  return m_playerPanel->GetPlayer(p_index);
}

wxString GamePropertiesDialog::GetPlayerLabel(int p_index) const
{
  return m_playerPanel->GetPlayerLabel(p_index);
}

wxColour GamePropertiesDialog::GetPlayerColor(int p_index) const
{
  return m_playerPanel->GetPlayerColor(p_index);
}

} // namespace Gambit::GUI
