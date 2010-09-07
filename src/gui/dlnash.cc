//
// This file is part of Gambit
// Copyright (c) 1994-2010, The Gambit Project (http://www.gambit-project.org)
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
#endif  // WX_PRECOMP

#include "dlnash.h"

static wxString s_recommended(wxT("with Gambit's recommended method"));
static wxString s_enumpure(wxT("by looking for pure strategy equilibria"));
static wxString s_enummixed(wxT("by enumerating extreme points"));
static wxString s_enumpoly(wxT("by solving systems of polynomial equations"));
static wxString s_lp(wxT("by solving a linear program"));
static wxString s_lcp(wxT("by solving a linear complementarity program"));
static wxString s_liap(wxT("by minimizing the Lyapunov function"));
static wxString s_logit(wxT("by tracing logit equilibria"));
static wxString s_simpdiv(wxT("by simplicial subdivision"));

gbtNashChoiceDialog::gbtNashChoiceDialog(wxWindow *p_parent, 
					 gbtGameDocument *p_doc)
  : wxDialog(p_parent, -1, wxT("Compute Nash equilibria"), wxDefaultPosition),
    m_doc(p_doc)
{
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxString countChoices[] = { wxT("Compute one Nash equilibrium"),
			      wxT("Compute as many Nash equilibria as possible"),
			      wxT("Compute all Nash equilibria") };
  m_countChoice = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize,
			       3, countChoices);
  m_countChoice->SetSelection(2);
  Connect(m_countChoice->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
	  wxCommandEventHandler(gbtNashChoiceDialog::OnCount));
  topSizer->Add(m_countChoice, 0, wxALL | wxEXPAND, 5);

  if (p_doc->NumPlayers() == 2) {
    wxString methodChoices[] = { s_recommended, s_enummixed,
				 s_enumpoly };
    m_methodChoice = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize,
				  3, methodChoices);
  }
  else {
    wxString methodChoices[] = { s_recommended, s_enumpoly };
    m_methodChoice = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize,
				  2, methodChoices);
  }
  m_methodChoice->SetSelection(0);
  topSizer->Add(m_methodChoice, 0, wxALL | wxEXPAND, 5);

  if (m_doc->IsTree()) {
    wxString repChoices[] = { wxT("using the extensive game"),
			      wxT("using the strategic game") };
    m_repChoice = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize,
			       2, repChoices);
    m_repChoice->SetSelection(0);
    topSizer->Add(m_repChoice, 0, wxALL | wxEXPAND, 5);

    // We only need to respond to changes in method when we have an
    // extensive game
    Connect(m_methodChoice->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
	    wxCommandEventHandler(gbtNashChoiceDialog::OnMethod));
  }
  else {
    m_repChoice = 0;
  }

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
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
    if (m_doc->IsConstSum()) {
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
  }
  else {
    if (m_doc->NumPlayers() == 2) {
      m_methodChoice->Append(s_enummixed);
    }
    m_methodChoice->Append(s_enumpoly);
  }

  m_methodChoice->SetSelection(0);
}

void gbtNashChoiceDialog::OnMethod(wxCommandEvent &p_event)
{
  wxString method = m_methodChoice->GetString(p_event.GetSelection());

  if (method == s_simpdiv || method == s_enummixed) {
    m_repChoice->SetSelection(1);
    m_repChoice->Enable(false);
  }
  else {
    m_repChoice->Enable(true);
  }
}

gbtAnalysisOutput *gbtNashChoiceDialog::GetCommand(void) const
{
  bool useEfg = m_repChoice && m_repChoice->GetSelection() == 0;
  gbtAnalysisOutput *cmd = 0; 

  wxString method = m_methodChoice->GetStringSelection();

  wxString prefix, options, game, count;
#ifdef __WXMAC__
  // For the moment, Mac assumes the command-line tools are in /usr/local
  prefix = wxT("/usr/local/bin/gambit-");
#else
  prefix = wxT("gambit-");
#endif  // __WXMAC__

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
	cmd = new gbtAnalysisProfileList<Rational>(m_doc, useEfg);
	cmd->SetCommand(prefix + wxT("lp") + options);
	cmd->SetDescription(wxT("One equilibrium by solving a linear program ")
			    + game);
      }
      else {
	cmd = new gbtAnalysisProfileList<double>(m_doc, useEfg);
	cmd->SetCommand(prefix + wxT("logit -e -d 10"));
	cmd->SetDescription(wxT("One equilibrium by logit tracing ") + game);
      }
    }
    else if (m_countChoice->GetSelection() == 1) {
      if (m_doc->NumPlayers() == 2) {
	cmd = new gbtAnalysisProfileList<Rational>(m_doc, useEfg);
	cmd->SetCommand(prefix + wxT("lcp") + options);
	cmd->SetDescription(wxT("Some equilibria by solving a linear "
			       "complementarity program ") + game);
      }
      else {
	cmd = new gbtAnalysisProfileList<double>(m_doc, useEfg);
	cmd->SetCommand(prefix + wxT("enumpoly -d 10") + options);
	cmd->SetDescription(wxT("Some equilibria by solving polynomial "
				"systems ") + game);
      }
    }
    else {
      if (m_doc->NumPlayers() == 2) {
	cmd = new gbtAnalysisProfileList<Rational>(m_doc, false);
	cmd->SetCommand(prefix + wxT("enummixed"));
	cmd->SetDescription(wxT("All equilibria by enumeration of mixed "
			       "strategies in strategic game"));
      }
      else {
	cmd = new gbtAnalysisProfileList<double>(m_doc, useEfg);
	cmd->SetCommand(prefix + wxT("enumpoly -d 10") + options);
	cmd->SetDescription(wxT("All equilibria by solving polynomial "
			       "systems ") + game);
      }
    }
  }
  else if (method == s_enumpure) {
    cmd = new gbtAnalysisProfileList<Rational>(m_doc, useEfg);
    cmd->SetCommand(prefix + wxT("enumpure") + options);
    cmd->SetDescription(count + wxT(" in pure strategies ") + game);
  }
  else if (method == s_enummixed) {
    cmd = new gbtAnalysisProfileList<Rational>(m_doc, false);
    cmd->SetCommand(prefix + wxT("enummixed") + options);
    cmd->SetDescription(count + 
			wxT(" by enumeration of mixed strategies "
			    "in strategic game"));
  }
  else if (method == s_enumpoly) {
    cmd = new gbtAnalysisProfileList<double>(m_doc, useEfg);
    cmd->SetCommand(prefix + wxT("enumpoly -d 10") + options);
    cmd->SetDescription(count + wxT(" by solving polynomial systems ") +
		       game);
  }
  else if (method == s_lp) {
    cmd = new gbtAnalysisProfileList<Rational>(m_doc, useEfg);
    cmd->SetCommand(prefix + wxT("lp") + options);
    cmd->SetDescription(count + wxT(" by solving a linear program ") + game);
  }
  else if (method == s_lcp) {
    cmd = new gbtAnalysisProfileList<Rational>(m_doc, useEfg);
    cmd->SetCommand(prefix + wxT("lcp") + options);
    cmd->SetDescription(count + wxT(" by solving a linear complementarity "
				   "program ") + game);
  }
  else if (method == s_liap) {
    cmd = new gbtAnalysisProfileList<double>(m_doc, useEfg);
    cmd->SetCommand(prefix + wxT("liap -d 10") + options);
    cmd->SetDescription(count + wxT(" by function minimization ") + game);
  }
  else if (method == s_logit) {
    cmd = new gbtAnalysisProfileList<double>(m_doc, useEfg);
    cmd->SetCommand(prefix + wxT("logit -e -d 10") + options);
    cmd->SetDescription(count + wxT(" by logit tracing ") + game); 
  }
  else if (method == s_simpdiv) {
    cmd = new gbtAnalysisProfileList<double>(m_doc, false);
    cmd->SetCommand(prefix + wxT("simpdiv") + options);
    cmd->SetDescription(count + wxT(" by simplicial subdivision "
				    "in strategic game"));
  }
  else {
    // Shouldn't happen!
  }

  return cmd;
}

