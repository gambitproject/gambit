//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/nfgpanel.cc
// Implementation of normal form panel
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

#include "gamedoc.h"
#include "nfgpanel.h"

#include "dlexcept.h"
#include "nfgtable.h"
#include "menuconst.h"

namespace Gambit::GUI {
//=====================================================================
//               class StrategyDominanceToolbar
//=====================================================================

class StrategyDominanceToolbar final : public wxPanel, public GameView {
private:
  wxButton *m_topButton, *m_prevButton, *m_nextButton, *m_allButton;
  wxStaticText *m_level;

  // Overriding GameView members
  void OnUpdate() override;

  // Event handlers
  void OnStrength(wxCommandEvent &);
  void OnTopLevel(wxCommandEvent &);
  void OnPreviousLevel(wxCommandEvent &);
  void OnNextLevel(wxCommandEvent &);
  void OnLastLevel(wxCommandEvent &);

public:
  StrategyDominanceToolbar(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc);
  ~StrategyDominanceToolbar() override = default;
};

#include "bitmaps/next.xpm"
#include "bitmaps/prev.xpm"
#include "bitmaps/tobegin.xpm"
#include "bitmaps/toend.xpm"

StrategyDominanceToolbar::StrategyDominanceToolbar(wxWindow *p_parent,
                                                   const std::shared_ptr<GameDocument> &p_doc)
  : wxPanel(p_parent, wxID_ANY), GameView(p_doc)
{
  auto *topSizer = new wxBoxSizer(wxHORIZONTAL);

  topSizer->Add(new wxStaticText(this, wxID_STATIC, wxT("Hide strategies which are ")), 0,
                wxALL | wxALIGN_CENTER, 5);

  wxString domChoices[] = {wxT("strictly"), wxT("strictly or weakly")};
  auto *choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, domChoices);
  choice->SetSelection(0);
  Connect(choice->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
          wxCommandEventHandler(StrategyDominanceToolbar::OnStrength));
  topSizer->Add(choice, 0, wxALL | wxALIGN_CENTER, 5);

  topSizer->Add(new wxStaticText(this, wxID_STATIC, wxT("dominated:")), 0, wxALL | wxALIGN_CENTER,
                5);

  m_topButton = new wxBitmapButton(this, wxID_ANY, wxBitmap(tobegin_xpm));
  m_topButton->SetToolTip(_("Show all strategies"));
  Connect(m_topButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(StrategyDominanceToolbar::OnTopLevel));
  topSizer->Add(m_topButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_prevButton = new wxBitmapButton(this, wxID_ANY, wxBitmap(prev_xpm));
  m_prevButton->SetToolTip(_("Previous round of elimination"));
  Connect(m_prevButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(StrategyDominanceToolbar::OnPreviousLevel));
  topSizer->Add(m_prevButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_level = new wxStaticText(this, wxID_STATIC, wxT("All strategies shown"), wxDefaultPosition,
                             wxDefaultSize, wxALIGN_CENTER | wxST_NO_AUTORESIZE);
  topSizer->Add(m_level, 0, wxALL | wxALIGN_CENTER, 5);

  m_nextButton = new wxBitmapButton(this, wxID_ANY, wxBitmap(next_xpm));
  m_nextButton->SetToolTip(_("Next round of elimination"));
  Connect(m_nextButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(StrategyDominanceToolbar::OnNextLevel));
  topSizer->Add(m_nextButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_allButton = new wxBitmapButton(this, wxID_ANY, wxBitmap(toend_xpm));
  m_allButton->SetToolTip(_("Eliminate iteratively"));
  Connect(m_allButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
          wxCommandEventHandler(StrategyDominanceToolbar::OnLastLevel));
  topSizer->Add(m_allButton, 0, wxALL | wxALIGN_CENTER, 5);

  SetSizer(topSizer);
  wxWindowBase::Layout();
}

void StrategyDominanceToolbar::OnStrength(wxCommandEvent &p_event)
{
  m_doc->DoSetDominanceStrictness(p_event.GetSelection() == 0);
}

void StrategyDominanceToolbar::OnTopLevel(wxCommandEvent &) { m_doc->DoTopDominanceLevel(); }

void StrategyDominanceToolbar::OnPreviousLevel(wxCommandEvent &)
{
  m_doc->DoPreviousDominanceLevel();
}

void StrategyDominanceToolbar::OnNextLevel(wxCommandEvent &) { m_doc->DoNextDominanceLevel(); }

void StrategyDominanceToolbar::OnLastLevel(wxCommandEvent &)
{
  while (m_doc->DoNextDominanceLevel())
    ;
}

void StrategyDominanceToolbar::OnUpdate()
{
  m_topButton->Enable(m_doc->GetWorkspace().GetStrategyElimLevel() > 1);
  m_prevButton->Enable(m_doc->GetWorkspace().GetStrategyElimLevel() > 1);
  m_nextButton->Enable(m_doc->GetWorkspace().CanStrategyElim());
  m_allButton->Enable(m_doc->GetWorkspace().CanStrategyElim());
  if (m_doc->GetWorkspace().GetStrategyElimLevel() == 1) {
    m_level->SetLabel(wxT("All strategies shown"));
  }
  else if (m_doc->GetWorkspace().GetStrategyElimLevel() == 2) {
    m_level->SetLabel(wxT("Eliminated 1 level"));
  }
  else {
    wxString label;
    label << "Eliminated " << (m_doc->GetWorkspace().GetStrategyElimLevel() - 1) << " levels";
    m_level->SetLabel(label);
  }
  GetSizer()->Layout();
}

//======================================================================
//                   class NfgPanel: Member functions
//======================================================================

BEGIN_EVENT_TABLE(NfgPanel, wxPanel)
EVT_MENU(GBT_MENU_TOOLS_DOMINANCE, NfgPanel::OnToolsDominance)
END_EVENT_TABLE()

NfgPanel::NfgPanel(wxWindow *p_parent, const std::shared_ptr<GameDocument> &p_doc,
                   bool p_showDominance)
  : wxPanel(p_parent, wxID_ANY), GameView(p_doc),
    m_dominanceToolbar(new StrategyDominanceToolbar(this, m_doc)),
    m_tableWidget(new TableWidget(this, wxID_ANY, m_doc))
{
  auto *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_dominanceToolbar, 0, wxEXPAND, 0);
  topSizer->Show(m_dominanceToolbar, p_showDominance);
  topSizer->Add(m_tableWidget, 1, wxEXPAND, 0);
  SetSizer(topSizer);
  wxWindowBase::Layout();
  OnUpdate();
}

void NfgPanel::OnToolsDominance(wxCommandEvent &p_event)
{
  GetSizer()->Show(m_dominanceToolbar, p_event.IsChecked(), true);
  GetSizer()->Layout();

  // Redraw the table with/without dominance markings
  m_tableWidget->OnUpdate();

  // This call is necessary on MSW to clear out a ghost of the
  // dominance toolbar's strength control, under certain circumstances.
  // (I think it is because of the empty upper-left panel in the
  // table widget, but I'm not sure.)
  Refresh();
}

void NfgPanel::OnUpdate()
{
  m_tableWidget->OnUpdate();
  GetSizer()->Layout();
}

void NfgPanel::PostPendingChanges() { m_tableWidget->PostPendingChanges(); }

wxPrintout *NfgPanel::GetPrintout() { return m_tableWidget->GetPrintout(); }

bool NfgPanel::GetBitmap(wxBitmap &p_bitmap, int p_marginX, int p_marginY)
{
  return m_tableWidget->GetBitmap(p_bitmap, p_marginX, p_marginY);
}

void NfgPanel::GetSVG(const wxString &p_filename, int p_marginX, int p_marginY)
{
  m_tableWidget->GetSVG(p_filename, p_marginX, p_marginY);
}

void NfgPanel::RenderGame(wxDC &p_dc, int p_marginX, int p_marginY)
{
  m_tableWidget->RenderGame(p_dc, p_marginX, p_marginY);
}
} // namespace Gambit::GUI
