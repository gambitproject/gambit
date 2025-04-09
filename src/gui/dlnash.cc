//
// This file is part of Gambit
// Copyright (c) 1994-2025, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlnash.cc
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

#include <sstream>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/stdpaths.h>

#include "dlnash.h"

static wxString s_recommended(wxT("with Gambit's recommended method"));
static wxString s_enumpure(wxT("by looking for pure strategy equilibria"));
static wxString s_enummixed(wxT("by enumerating extreme points"));
static wxString s_enumpoly(wxT("by solving systems of polynomial equations"));
static wxString s_gnm(wxT("by global Newton tracing"));
static wxString s_ipa(wxT("by iterated polymatrix approximation"));
static wxString s_lp(wxT("by solving a linear program"));
static wxString s_lcp(wxT("by solving a linear complementarity program"));
static wxString s_liap(wxT("by minimizing the Lyapunov function"));
static wxString s_logit(wxT("by tracing logit equilibria"));
static wxString s_simpdiv(wxT("by simplicial subdivision"));

gbtNashChoiceDialog::gbtNashChoiceDialog(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxDialog(p_parent, wxID_ANY, wxT("Compute Nash equilibria"), wxDefaultPosition), m_doc(p_doc)
{
  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  if (m_doc->GetGame()->NumPlayers() == 2) {
    wxString countChoices[] = {wxT("Compute one Nash equilibrium"),
                               wxT("Compute some Nash equilibria"),
                               wxT("Compute all Nash equilibria")};
    m_countChoice =
        new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, countChoices);
  }
  else {
    wxString countChoices[] = {wxT("Compute one Nash equilibrium"),
                               wxT("Compute some Nash equilibria")};
    m_countChoice =
        new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, countChoices);
  }
  m_countChoice->SetSelection(0);

  Connect(m_countChoice->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
          wxCommandEventHandler(gbtNashChoiceDialog::OnCount));
  topSizer->Add(m_countChoice, 0, wxALL | wxEXPAND, 5);

  if (p_doc->NumPlayers() == 2 && m_doc->IsConstSum()) {
    wxString methodChoices[] = {s_recommended, s_lp, s_simpdiv, s_logit, s_enumpoly};
    m_methodChoice =
        new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 5, methodChoices);
  }
  else {
    wxString methodChoices[] = {s_recommended, s_simpdiv, s_logit, s_enumpoly};
    m_methodChoice =
        new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 4, methodChoices);
  }
  m_methodChoice->SetSelection(0);
  topSizer->Add(m_methodChoice, 0, wxALL | wxEXPAND, 5);

  if (m_doc->IsTree()) {
    wxString repChoices[] = {wxT("using the extensive game"), wxT("using the strategic game")};
    m_repChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 2, repChoices);
    m_repChoice->SetSelection(0);
    topSizer->Add(m_repChoice, 0, wxALL | wxEXPAND, 5);

    // We only need to respond to changes in method when we have an
    // extensive game
    Connect(m_methodChoice->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
            wxCommandEventHandler(gbtNashChoiceDialog::OnMethod));
  }
  else {
    m_repChoice = nullptr;
  }

  auto *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  auto *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxALIGN_RIGHT, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
  CenterOnParent();
}

void gbtNashChoiceDialog::OnCount(wxCommandEvent &p_event)
{
  m_methodChoice->Clear();
  m_methodChoice->Append(s_recommended);

  if (p_event.GetSelection() == 0) {
    if (m_doc->NumPlayers() == 2 && m_doc->IsConstSum()) {
      m_methodChoice->Append(s_lp);
    }
    m_methodChoice->Append(s_simpdiv);
    m_methodChoice->Append(s_logit);
  }
  else if (p_event.GetSelection() == 1) {
    if (m_doc->NumPlayers() == 2) {
      m_methodChoice->Append(s_lcp);
    }
    m_methodChoice->Append(s_enumpure);
    m_methodChoice->Append(s_liap);
    m_methodChoice->Append(s_gnm);
    m_methodChoice->Append(s_ipa);
    m_methodChoice->Append(s_enumpoly);
  }
  else {
    if (m_doc->NumPlayers() == 2) {
      m_methodChoice->Append(s_enummixed);
    }
  }
  m_methodChoice->SetSelection(0);
}

void gbtNashChoiceDialog::OnMethod(wxCommandEvent &p_event)
{
  const wxString method = m_methodChoice->GetString(p_event.GetSelection());

  if (method == s_simpdiv || method == s_enummixed || method == s_gnm || method == s_ipa) {
    m_repChoice->SetSelection(1);
    m_repChoice->Enable(false);
  }
  else {
    m_repChoice->Enable(true);
  }
}

bool gbtNashChoiceDialog::UseStrategic() const
{
  return (m_repChoice == nullptr || m_repChoice->GetSelection() == 1);
}

std::shared_ptr<gbtAnalysisOutput> gbtNashChoiceDialog::GetCommand() const
{
  const bool useEfg = m_repChoice && m_repChoice->GetSelection() == 0;
  std::shared_ptr<gbtAnalysisOutput> cmd = nullptr;

  const wxString method = m_methodChoice->GetStringSelection();

  wxString prefix, options, game, count;
#ifdef __WXMAC__
  // Look in the app bundle.  The command-line tools should be placed
  // in the same folder inside the app bundle as the GUI executable.
  // GetExecutablePath() returns the full path to the GUI executable,
  // including the 'gambit', so all we need is the dash to form the prefix.
  prefix = wxStandardPaths::Get().GetExecutablePath() + wxT("-");
#else
  prefix = wxT("gambit-");
#endif // __WXMAC__

  if (useEfg) {
    game = wxT("in extensive game");
  }
  else {
    options = wxT(" -S ");
    game = wxT("in strategic game");
  }

  if (m_countChoice->GetSelection() == 0) {
    count = wxT("One equilibrium");
  }
  else if (m_countChoice->GetSelection() == 1) {
    count = wxT("Some equilibria");
  }
  else {
    count = wxT("All equilibria");
  }

  if (method == s_recommended) {
    if (m_countChoice->GetSelection() == 0) {
      if (m_doc->NumPlayers() == 2 && m_doc->IsConstSum()) {
        cmd = std::make_shared<gbtAnalysisProfileList<Rational>>(m_doc, useEfg);
        cmd->SetCommand(prefix + wxT("lp") + options);
        cmd->SetDescription(wxT("One equilibrium by solving a linear program ") + game);
      }
      else {
        cmd = std::make_shared<gbtAnalysisProfileList<double>>(m_doc, useEfg);
        cmd->SetCommand(prefix + wxT("logit -e -d 10"));
        cmd->SetDescription(wxT("One equilibrium by logit tracing ") + game);
      }
    }
    else if (m_countChoice->GetSelection() == 1) {
      if (m_doc->NumPlayers() == 2) {
        cmd = std::make_shared<gbtAnalysisProfileList<Rational>>(m_doc, useEfg);
        cmd->SetCommand(prefix + wxT("lcp") + options);
        cmd->SetDescription(wxT("Some equilibria by solving a linear complementarity program ") +
                            game);
      }
      else {
        cmd = std::make_shared<gbtAnalysisProfileList<double>>(m_doc, false);
        cmd->SetCommand(prefix + wxT("simpdiv -d 10 -n 20 -r 100") + options);
        cmd->SetDescription(wxT("Some equilibria by simplicial subdivision ") + game);
      }
    }
    else {
      if (m_doc->NumPlayers() == 2) {
        cmd = std::make_shared<gbtAnalysisProfileList<Rational>>(m_doc, false);
        cmd->SetCommand(prefix + wxT("enummixed"));
        cmd->SetDescription(
            wxT("All equilibria by enumeration of mixed strategies in strategic game"));
      }
      else {
        cmd = std::make_shared<gbtAnalysisProfileList<double>>(m_doc, useEfg);
        cmd->SetCommand(prefix + wxT("enumpoly -d 10") + options);
        cmd->SetDescription(wxT("All equilibria by solving polynomial systems ") + game);
      }
    }
  }
  else if (method == s_enumpure) {
    cmd = std::make_shared<gbtAnalysisProfileList<Rational>>(m_doc, useEfg);
    cmd->SetCommand(prefix + wxT("enumpure") + options);
    cmd->SetDescription(count + wxT(" in pure strategies ") + game);
  }
  else if (method == s_enummixed) {
    cmd = std::make_shared<gbtAnalysisProfileList<Rational>>(m_doc, false);
    cmd->SetCommand(prefix + wxT("enummixed") + options);
    cmd->SetDescription(count + wxT(" by enumeration of mixed strategies in strategic game"));
  }
  else if (method == s_enumpoly) {
    cmd = std::make_shared<gbtAnalysisProfileList<double>>(m_doc, useEfg);
    cmd->SetCommand(prefix + wxT("enumpoly -d 10") + options);
    cmd->SetDescription(count + wxT(" by solving polynomial systems ") + game);
  }
  else if (method == s_gnm) {
    cmd = std::make_shared<gbtAnalysisProfileList<double>>(m_doc, false);
    cmd->SetCommand(prefix + wxT("gnm -d 10") + options);
    cmd->SetDescription(count + wxT(" by global Newton tracing in strategic game"));
  }
  else if (method == s_ipa) {
    cmd = std::make_shared<gbtAnalysisProfileList<double>>(m_doc, false);
    cmd->SetCommand(prefix + wxT("ipa -d 10") + options);
    cmd->SetDescription(count + wxT(" by iterated polymatrix approximation in strategic game"));
  }
  else if (method == s_lp) {
    cmd = std::make_shared<gbtAnalysisProfileList<Rational>>(m_doc, useEfg);
    cmd->SetCommand(prefix + wxT("lp") + options);
    cmd->SetDescription(count + wxT(" by solving a linear program ") + game);
  }
  else if (method == s_lcp) {
    cmd = std::make_shared<gbtAnalysisProfileList<Rational>>(m_doc, useEfg);
    cmd->SetCommand(prefix + wxT("lcp") + options);
    cmd->SetDescription(count + wxT(" by solving a linear complementarity program ") + game);
  }
  else if (method == s_liap) {
    cmd = std::make_shared<gbtAnalysisProfileList<double>>(m_doc, useEfg);
    cmd->SetCommand(prefix + wxT("liap -d 10") + options);
    cmd->SetDescription(count + wxT(" by function minimization ") + game);
  }
  else if (method == s_logit) {
    cmd = std::make_shared<gbtAnalysisProfileList<double>>(m_doc, useEfg);
    cmd->SetCommand(prefix + wxT("logit -e -d 10") + options);
    cmd->SetDescription(count + wxT(" by logit tracing ") + game);
  }
  else if (method == s_simpdiv) {
    cmd = std::make_shared<gbtAnalysisProfileList<double>>(m_doc, false);
    cmd->SetCommand(prefix + wxT("simpdiv -d 10 -n 20 -r 100") + options);
    cmd->SetDescription(count + wxT(" by simplicial subdivision in strategic game"));
  }
  else {
    // Shouldn't happen!
  }

  return cmd;
}
