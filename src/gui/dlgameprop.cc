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

#include <algorithm>
#include <functional>
#include <set>
#include <vector>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/notebook.h>
#include <wx/scrolwin.h>
#include <wx/colourdata.h>
#ifdef __WXMAC__
#include <wx/generic/colrdlgg.h>
#else
#include <wx/colordlg.h>
#endif

#include "gambit.h"
#include "gamedoc.h"
#include "dlgameprop.h"
#include "editlabel.h"

namespace Gambit::GUI {

namespace {
const wxColour kInvalidLabelBg(255, 220, 220);
const wxColour kNewLabelColour(0, 102, 0);      // new player: bold, dark green
const wxColour kRenamedLabelColour(0, 51, 204); // renamed player: italic, strong blue
// A neutral placeholder swatch for a player that doesn't exist yet; freely overridable via
// the row's own colour button before the dialog is committed.
const wxColour kNewPlayerColor(200, 200, 200);

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
//                        class PlayerPanel
//========================================================================

// An editable, variable-length list of the game's players (not including chance), colour-coded
// like a source-control diff (added/renamed/deleted), mirroring StrategyPanel
// (dleditstrategies.cc) with an added per-row colour swatch.
//
// Each row's *stable label* (its label before this dialog touched it, or a placeholder for a
// newly added row) never changes; it's what `Game::SetPlayers()` uses to match the row to its
// underlying player regardless of what the user has typed into its *current* label text.
// Deleting an existing row marks it deleted and disables it, with its delete button becoming
// a restore button, rather than removing it outright -- a newly added row has nothing to
// preserve, so deleting one of those does remove it.
class PlayerPanel final : public wxScrolledWindow {
public:
  struct Row {
    GamePlayer player; // null for a row added in this dialog, with no prior player at all
    std::string stableLabel;
    bool isNew = false;     // true for a row added in this dialog
    bool isDeleted = false; // true for an existing row marked for deletion (kept, disabled)
    wxString currentLabelText;
    wxColour color; // staged: not written to the document until the dialog's OK is committed
    LabelTextCtrl *labelCtrl = nullptr;
    wxBitmapButton *colorButton = nullptr;
    wxButton *upButton = nullptr;
    wxButton *downButton = nullptr;
    wxButton *deleteButton = nullptr; // doubles as the restore button when isDeleted
  };

private:
  std::shared_ptr<GameDocument> m_doc;
  std::vector<Row> m_rows;
  wxBoxSizer *m_topSizer;
  wxColour m_defaultBg;
  std::function<void()> m_onChanged;
  // False until the constructor completes, so Rebuild()'s initial call can't notify the
  // owning dialog before it's finished being constructed.
  bool m_ready = false;
  // True from the moment a row mutation starts until its (deferred) Rebuild() finishes; see
  // StrategyPanel (dleditstrategies.cc) for why this guard is needed.
  bool m_rebuilding = false;

  std::string NextPlaceholderLabel() const;
  int ActiveCount() const
  {
    return static_cast<int>(
        std::count_if(m_rows.begin(), m_rows.end(), [](const Row &r) { return !r.isDeleted; }));
  }
  // The reason `row` can't be deleted because of the game's own rules -- it has decisions (in
  // an extensive game) or more than one strategy (in a strategic game) -- or an empty string
  // if there's no such structural obstruction. Independent of the separate "at least one
  // player must remain" rule enforced where this is used.
  wxString BlockedDeleteReason(const Row &row) const;
  void Rebuild();
  // Colours a (non-deleted) row's label to reflect whether it's new, renamed from its stable
  // label, or unchanged, and sets/clears the "renamed from" tooltip to match.
  static void UpdateRowStyle(Row &row);
  void NotifyChanged() const
  {
    if (m_ready) {
      m_onChanged();
    }
  }

  void OnSetColor(int p_index);
  // Builds a fresh color-swatch button for a row and swaps it in for the old one, rather than
  // mutating the existing button's bitmap in place -- see PlayerPanel::OnSetColor() for why.
  void RecreateColorButton(int p_index);
  wxBitmapButton *MakeColorButton(int p_index);

public:
  PlayerPanel(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc,
              const std::function<void()> &p_onChanged);

  int NumPlayers() const { return static_cast<int>(m_rows.size()); }
  bool IsDeleted(int p_index) const { return m_rows.at(p_index).isDeleted; }
  std::string GetStableLabel(int p_index) const { return m_rows.at(p_index).stableLabel; }
  wxString GetPlayerLabel(int p_index) const
  {
    return m_rows.at(p_index).labelCtrl->GetNormalizedValue();
  }
  wxColour GetPlayerColor(int p_index) const { return m_rows.at(p_index).color; }

  void AddPlayer();
  // Marks an existing row deleted (or restores an already-deleted one); removes a row added
  // in this dialog outright. A no-op if this would leave no active (non-deleted) rows, or if
  // the row is structurally blocked from deletion (see BlockedDeleteReason()).
  void ToggleDeleted(int p_index);
  void MovePlayerUp(int p_index);
  void MovePlayerDown(int p_index);

  // Highlights any empty or duplicate player labels among active (non-deleted) rows, and
  // returns a description of the first problem found, or an empty string if all are valid.
  wxString ValidateLabels();
};

PlayerPanel::PlayerPanel(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc,
                         const std::function<void()> &p_onChanged)
  : wxScrolledWindow(p_parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                     wxVSCROLL | wxTAB_TRAVERSAL),
    m_doc(p_doc), m_defaultBg(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)),
    m_onChanged(p_onChanged)
{
  for (const auto &player : m_doc->GetGame()->GetPlayers()) {
    Row row;
    row.player = player;
    row.stableLabel = player->GetLabel();
    row.currentLabelText = wxString::FromUTF8(row.stableLabel);
    row.color = m_doc->GetStyle().GetPlayerColor(player);
    m_rows.push_back(std::move(row));
  }

  m_topSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(m_topSizer);
  SetScrollRate(0, FromDIP(10));

  Rebuild();

  const wxSize bestSize = m_topSizer->CalcMin();
  SetMinSize(wxSize(FromDIP(400), std::min(bestSize.GetHeight(), FromDIP(250))));

  if (!m_rows.empty()) {
    m_defaultBg = m_rows.front().labelCtrl->GetBackgroundColour();
  }

  m_ready = true;
}

std::string PlayerPanel::NextPlaceholderLabel() const
{
  std::set<std::string> used;
  for (const auto &row : m_rows) {
    used.insert(row.stableLabel);
    used.insert(row.currentLabelText.ToStdString(wxConvUTF8));
  }
  int number = static_cast<int>(m_rows.size()) + 1;
  while (contains(used, "Player " + std::to_string(number))) {
    number++;
  }
  return "Player " + std::to_string(number);
}

void PlayerPanel::UpdateRowStyle(Row &row)
{
  // Colour alone can be hard to tell apart (or perceive at all); weight/slant give the same
  // information a second way, so the state still reads even if the colours don't.
  wxFont font = row.labelCtrl->GetFont();
  font.SetWeight(wxFONTWEIGHT_NORMAL);
  font.SetStyle(wxFONTSTYLE_NORMAL);

  if (row.isNew) {
    font.SetWeight(wxFONTWEIGHT_BOLD);
    row.labelCtrl->SetForegroundColour(kNewLabelColour);
    row.labelCtrl->UnsetToolTip();
  }
  else if (row.currentLabelText.ToStdString(wxConvUTF8) != row.stableLabel) {
    font.SetStyle(wxFONTSTYLE_ITALIC);
    row.labelCtrl->SetForegroundColour(kRenamedLabelColour);
    row.labelCtrl->SetToolTip(
        wxString::Format(_("Renamed from \"%s\""), wxString::FromUTF8(row.stableLabel)));
  }
  else {
    row.labelCtrl->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    row.labelCtrl->UnsetToolTip();
  }
  row.labelCtrl->SetFont(font);
  row.labelCtrl->Refresh();
}

wxString PlayerPanel::BlockedDeleteReason(const Row &row) const
{
  if (row.isNew) {
    return {}; // nothing has been committed yet -- freely removable
  }
  if (m_doc->GetGame()->IsTree()) {
    if (row.player->GetInfosets().size() > 0) {
      return _("This player has decisions in the game and cannot be deleted.");
    }
  }
  else if (row.player->GetStrategies().size() != 1) {
    return _("This player has more than one strategy and cannot be deleted.");
  }
  return {};
}

void PlayerPanel::Rebuild()
{
  m_rebuilding = true;
  m_topSizer->Clear(true); // destroys the previous row controls; each Row's own state isn't
                           // owned by them

  auto *gridSizer = new wxFlexGridSizer(4, FromDIP(5), FromDIP(10));
  gridSizer->AddGrowableCol(2, 1);

  gridSizer->AddSpacer(1);
  gridSizer->AddSpacer(1);
  gridSizer->Add(new wxStaticText(this, wxID_STATIC, _("Label")), 0, wxALIGN_CENTER_VERTICAL);
  gridSizer->AddSpacer(1);

  const int activeCount = ActiveCount();
  const wxSize buttonSize(FromDIP(28), -1);

  for (size_t i = 0; i < m_rows.size(); i++) {
    Row &row = m_rows[i];

    row.colorButton = MakeColorButton(static_cast<int>(i));
    gridSizer->Add(row.colorButton, 0, wxALIGN_CENTER_VERTICAL | wxLEFT, FromDIP(5));

    wxString number;
    number << (i + 1);
    gridSizer->Add(new wxStaticText(this, wxID_STATIC, number), 0,
                   wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);

    row.labelCtrl = new LabelTextCtrl(this, wxID_ANY, row.currentLabelText);
    if (row.isDeleted) {
      wxFont strikeFont = row.labelCtrl->GetFont();
      strikeFont.SetStrikethrough(true);
      row.labelCtrl->SetFont(strikeFont);
      row.labelCtrl->Disable();
    }
    else {
      UpdateRowStyle(row);
    }
    row.labelCtrl->Bind(wxEVT_TEXT, [this, i](wxCommandEvent &p_event) {
      if (!m_rebuilding && i < m_rows.size()) {
        Row &changedRow = m_rows.at(i);
        changedRow.currentLabelText = changedRow.labelCtrl->GetValue();
        UpdateRowStyle(changedRow);
        NotifyChanged();
      }
      p_event.Skip();
    });
    row.labelCtrl->Bind(wxEVT_KILL_FOCUS, [this, i](wxFocusEvent &p_event) {
      if (!m_rebuilding && i < m_rows.size()) {
        Row &changedRow = m_rows.at(i);
        changedRow.currentLabelText = changedRow.labelCtrl->GetNormalizedValue();
        UpdateRowStyle(changedRow);
        NotifyChanged();
      }
      p_event.Skip();
    });
    gridSizer->Add(row.labelCtrl, 1, wxEXPAND);

    auto *buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    if (!row.isDeleted) {
      row.upButton =
          new wxButton(this, wxID_ANY, wxUniChar(0x2191), wxDefaultPosition, buttonSize);
      row.upButton->Enable(i > 0);
      row.upButton->Bind(wxEVT_BUTTON,
                         [this, i](wxCommandEvent &) { MovePlayerUp(static_cast<int>(i)); });
      buttonSizer->Add(row.upButton, 0, wxLEFT, 2);

      row.downButton =
          new wxButton(this, wxID_ANY, wxUniChar(0x2193), wxDefaultPosition, buttonSize);
      row.downButton->Enable(i + 1 < m_rows.size());
      row.downButton->Bind(wxEVT_BUTTON,
                           [this, i](wxCommandEvent &) { MovePlayerDown(static_cast<int>(i)); });
      buttonSizer->Add(row.downButton, 0, wxLEFT, 2);
    }
    else {
      // Keep the delete/restore button aligned under its counterpart in other rows, in place
      // of the Up/Down buttons a deleted row no longer has. Clear() already destroyed the
      // previous Up/Down controls these pointers referred to; null them out rather than leave
      // them dangling.
      row.upButton = nullptr;
      row.downButton = nullptr;
      buttonSizer->AddSpacer(buttonSize.GetWidth() + 2);
      buttonSizer->AddSpacer(buttonSize.GetWidth() + 2);
    }

    const wxString blockedReason = row.isDeleted ? wxString() : BlockedDeleteReason(row);
    const bool canDelete = row.isDeleted || (blockedReason.empty() && activeCount > 1);

    row.deleteButton =
        new wxButton(this, wxID_ANY, row.isDeleted ? wxUniChar(0x21BA) : wxUniChar(0x2715),
                     wxDefaultPosition, buttonSize);
    if (row.isDeleted) {
      row.deleteButton->SetToolTip(_("Restore player"));
    }
    else if (!blockedReason.empty()) {
      row.deleteButton->SetToolTip(blockedReason);
    }
    else if (activeCount <= 1) {
      row.deleteButton->SetToolTip(_("At least one player must remain."));
    }
    else {
      row.deleteButton->SetToolTip(_("Delete player"));
    }
    row.deleteButton->Enable(canDelete);
    row.deleteButton->Bind(wxEVT_BUTTON,
                           [this, i](wxCommandEvent &) { ToggleDeleted(static_cast<int>(i)); });
    buttonSizer->Add(row.deleteButton, 0, wxLEFT, 2);

    gridSizer->Add(buttonSizer, 0, wxALIGN_CENTER_VERTICAL);
  }

  // A trailing row with only its button-column cell populated, so the "+" control lands
  // directly below the last row's Up/Down/Delete buttons rather than as a separate control
  // elsewhere in the dialog.
  gridSizer->AddSpacer(1);
  gridSizer->AddSpacer(1);
  gridSizer->AddSpacer(1);
  auto *addButton = new wxButton(this, wxID_ANY, "+", wxDefaultPosition, buttonSize);
  addButton->SetToolTip(_("Add player"));
  addButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) { AddPlayer(); });
  auto *addSizer = new wxBoxSizer(wxHORIZONTAL);
  addSizer->Add(addButton, 0, wxLEFT, 2);
  gridSizer->Add(addSizer, 0, wxALIGN_CENTER_VERTICAL);

  m_topSizer->Add(gridSizer, 1, wxALL | wxEXPAND, 5);
  FitInside();
  Layout();
  m_rebuilding = false;
  // No NotifyChanged() call here -- see m_ready's comment. Callers that mutate m_rows are
  // responsible for notifying once Rebuild() returns.
}

void PlayerPanel::AddPlayer()
{
  if (m_rebuilding) {
    return; // a rebuild from an earlier click is still pending; ignore this stale one
  }
  Row row;
  row.stableLabel = NextPlaceholderLabel();
  row.isNew = true;
  row.currentLabelText = wxString::FromUTF8(row.stableLabel);
  row.color = kNewPlayerColor;
  m_rows.push_back(std::move(row));
  // See StrategyPanel::AddStrategy() (dleditstrategies.cc) for why this defers Rebuild() and
  // raises m_rebuilding immediately, rather than just once Rebuild() starts.
  m_rebuilding = true;
  CallAfter([this]() {
    Rebuild();
    NotifyChanged();
  });
}

void PlayerPanel::ToggleDeleted(int p_index)
{
  if (m_rebuilding) {
    return; // a rebuild from an earlier click is still pending; ignore this stale one
  }
  Row &row = m_rows.at(p_index);
  if (!row.isDeleted) {
    if (!BlockedDeleteReason(row).empty()) {
      return; // structurally blocked; the disabled button shouldn't have fired regardless
    }
    if (ActiveCount() <= 1) {
      return; // refuse to drop below one active player
    }
  }
  if (row.isNew) {
    m_rows.erase(m_rows.begin() + p_index);
  }
  else {
    row.isDeleted = !row.isDeleted;
  }
  m_rebuilding = true;
  CallAfter([this]() {
    Rebuild();
    NotifyChanged();
  });
}

void PlayerPanel::MovePlayerUp(int p_index)
{
  if (m_rebuilding) {
    return; // a rebuild from an earlier click is still pending; ignore this stale one
  }
  if (p_index <= 0) {
    return;
  }
  std::swap(m_rows[p_index - 1], m_rows[p_index]);
  m_rebuilding = true;
  CallAfter([this]() {
    Rebuild();
    NotifyChanged();
  });
}

void PlayerPanel::MovePlayerDown(int p_index)
{
  if (m_rebuilding) {
    return; // a rebuild from an earlier click is still pending; ignore this stale one
  }
  if (p_index + 1 >= static_cast<int>(m_rows.size())) {
    return;
  }
  std::swap(m_rows[p_index], m_rows[p_index + 1]);
  m_rebuilding = true;
  CallAfter([this]() {
    Rebuild();
    NotifyChanged();
  });
}

wxString PlayerPanel::ValidateLabels()
{
  const int numPlayers = NumPlayers();
  wxString message;

  for (int i = 0; i < numPlayers; i++) {
    if (m_rows[i].isDeleted) {
      continue;
    }
    const wxString value = m_rows[i].labelCtrl->GetValue();
    bool invalid = value.empty();
    for (int other = 0; !invalid && other < numPlayers; other++) {
      if (other != i && !m_rows[other].isDeleted && m_rows[other].labelCtrl->GetValue() == value) {
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

wxBitmapButton *PlayerPanel::MakeColorButton(int p_index)
{
  Row &row = m_rows.at(p_index);
  auto *button = new wxBitmapButton(this, wxID_ANY, MakeColorSwatch(row.color), wxDefaultPosition,
                                    wxDefaultSize, wxNO_BORDER);
  button->SetToolTip(_("Change the color for this player"));
  button->Bind(wxEVT_BUTTON, [this, p_index](wxCommandEvent &) { OnSetColor(p_index); });
  return button;
}

void PlayerPanel::OnSetColor(int p_index)
{
  Row &row = m_rows.at(p_index);

  wxColourData data;
  data.SetColour(row.color);
  PlatformColourDialog dialog(this, &data);
  dialog.SetTitle(wxString::Format(_("Choose color for %s"), row.currentLabelText));

  if (dialog.ShowModal() != wxID_OK) {
    return;
  }

  // Staged only -- not written to the document until the dialog's OK is committed (by
  // GameFrame::OnEditGame), so canceling the dialog leaves colors untouched.
  row.color = dialog.GetColourData().GetColour();

  // Recreated (rather than mutated in place via SetBitmap()) because SetBitmap() on an
  // existing wxBitmapButton visibly shifted its rendered content on this platform. Deferred
  // via CallAfter(): this destroys the very button whose click invoked this handler, which is
  // still on the call stack -- see ActionPanel::AddAction() in dleditmove.cc for the same
  // hazard.
  CallAfter([this, p_index]() { RecreateColorButton(p_index); });
}

void PlayerPanel::RecreateColorButton(int p_index)
{
  Row &row = m_rows.at(p_index);

  wxBitmapButton *oldButton = row.colorButton;
  wxBitmapButton *newButton = MakeColorButton(p_index);

  // The button lives in a gridSizer nested inside m_topSizer, not directly in m_topSizer
  // itself, so the search needs to recurse into child sizers to find it.
  m_topSizer->Replace(oldButton, newButton, true);
  oldButton->Destroy();
  row.colorButton = newButton;

  Layout();
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

  auto *playersPanel = new wxPanel(notebook);
  auto *playersSizer = new wxBoxSizer(wxVERTICAL);

  if (game->IsTree()) {
    m_chanceColor = m_doc->GetStyle().ChanceColor();
    auto *chanceSizer = new wxBoxSizer(wxHORIZONTAL);
    m_chanceColorButton = MakeChanceColorButton(playersPanel);
    chanceSizer->Add(m_chanceColorButton, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, FromDIP(5));
    chanceSizer->Add(new wxStaticText(playersPanel, wxID_STATIC,
                                      wxString::FromUTF8(game->GetChance()->GetLabel())),
                     0, wxALIGN_CENTER_VERTICAL);
    playersSizer->Add(chanceSizer, 0, wxALL, FromDIP(5));
  }

  m_playerPanel = new PlayerPanel(playersPanel, m_doc, [this]() { UpdateValidation(); });
  playersSizer->Add(m_playerPanel, 1, wxALL | wxEXPAND, FromDIP(5));

  playersPanel->SetSizer(playersSizer);
  notebook->AddPage(playersPanel, _("Players"));

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

int GamePropertiesDialog::NumPlayerRows() const { return m_playerPanel->NumPlayers(); }

bool GamePropertiesDialog::IsPlayerDeleted(int p_index) const
{
  return m_playerPanel->IsDeleted(p_index);
}

std::string GamePropertiesDialog::GetPlayerStableLabel(int p_index) const
{
  return m_playerPanel->GetStableLabel(p_index);
}

wxString GamePropertiesDialog::GetPlayerLabel(int p_index) const
{
  return m_playerPanel->GetPlayerLabel(p_index);
}

wxColour GamePropertiesDialog::GetPlayerColor(int p_index) const
{
  return m_playerPanel->GetPlayerColor(p_index);
}

wxBitmapButton *GamePropertiesDialog::MakeChanceColorButton(wxWindow *p_parent)
{
  auto *button = new wxBitmapButton(p_parent, wxID_ANY, MakeColorSwatch(m_chanceColor),
                                    wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
  button->SetToolTip(_("Change the color for chance"));
  button->Bind(wxEVT_BUTTON, [this](wxCommandEvent &) { OnSetChanceColor(); });
  return button;
}

void GamePropertiesDialog::OnSetChanceColor()
{
  wxColourData data;
  data.SetColour(m_chanceColor);
  PlatformColourDialog dialog(this, &data);
  dialog.SetTitle(_("Choose color for chance"));

  if (dialog.ShowModal() != wxID_OK) {
    return;
  }

  m_chanceColor = dialog.GetColourData().GetColour();
  CallAfter([this]() { RecreateChanceColorButton(); });
}

void GamePropertiesDialog::RecreateChanceColorButton()
{
  wxWindow *parent = m_chanceColorButton->GetParent();
  wxBitmapButton *oldButton = m_chanceColorButton;
  wxBitmapButton *newButton = MakeChanceColorButton(parent);

  parent->GetSizer()->Replace(oldButton, newButton, true);
  oldButton->Destroy();
  m_chanceColorButton = newButton;

  parent->Layout();
}

} // namespace Gambit::GUI
