//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for selecting algorithms to compute Nash equilibria
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
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

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/spinctrl.h"
#include "dlnfgnash.h"

#include "base/gnullstatus.h"
#include "nash/nfgpure.h"
#include "nash/nfgmixed.h"
#include "nash/nfglcp.h"
#include "nash/nfglp.h"
#include "nash/nfgliap.h"
#include "nash/nfgpoly.h"
#include "nash/nfglogit.h"
#include "nash/nfgsimpdiv.h"
#include "nash/nfgyamamoto.h"

const int idCHECKBOX_FINDALL = 2000;
const int idSPINCTRL_STOPAFTER = 2001;

class gbtPanelNfgNashAlgorithm : public wxPanel {
public:
  gbtPanelNfgNashAlgorithm(wxWindow *p_parent) : wxPanel(p_parent, -1) { }

  virtual gbtNfgNashAlgorithm *GetAlgorithm(void) const = 0;
};

//========================================================================
//                         class gbtNfgNashOneNash
//========================================================================

class gbtNfgNashOneNash : public gbtNfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "OneNash"; }
  gbtMixedNashSet Solve(const gbtNfgGame &, gbtStatus &);
};

gbtMixedNashSet gbtNfgNashOneNash::Solve(const gbtNfgGame &p_nfg,
						gbtStatus &p_status)
{
  gbtArray<int> players(p_nfg->NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gbtNullStatus status;
    gbtNullOutput gnull;
    /* one round of elimination of weakly dominated strategies */
    gbtNfgSupport support = p_nfg->NewNfgSupport()->Undominated(false, players, gnull, status);

    if (support->NumPlayers() == 2) {
      if (support->IsConstSum()) {
	gbtNfgNashLp<double> algorithm;
	return algorithm.Solve(support, p_status);
      }
      else {
	gbtNfgNashLcp<double> algorithm;
	algorithm.SetStopAfter(1);
	return algorithm.Solve(support, p_status);
      }
    }
    else {
      gbtNfgNashSimpdiv<double> algorithm;
      return algorithm.Solve(support, p_status);
    }
  }
  catch (...) {
    return gbtMixedNashSet();
  }
}

//========================================================================
//                       class gbtPanelNfgOneNash
//========================================================================

class gbtPanelNfgOneNash : public gbtPanelNfgNashAlgorithm {
public:
  gbtPanelNfgOneNash(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelNfgOneNash::gbtPanelNfgOneNash(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, 
					   wxT("OneNashSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find one Nash equilibrium")),
		   0, wxALL | wxCENTER, 5);

  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("This algorithm requires no parameters")),
		   0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *gbtPanelNfgOneNash::GetAlgorithm(void) const
{
  return new gbtNfgNashOneNash;
}

//========================================================================
//                         class gbtNfgNashTwoNash
//========================================================================

class gbtNfgNashTwoNash : public gbtNfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "TwoNash"; }
  gbtMixedNashSet Solve(const gbtNfgGame &, gbtStatus &);
};

gbtMixedNashSet gbtNfgNashTwoNash::Solve(const gbtNfgGame &p_game,
						gbtStatus &p_status)
{
  gbtArray<int> players(p_game->NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gbtNfgSupport support = p_game->NewNfgSupport();

    while (true) {
      gbtNullStatus status;
      gbtNullOutput gnull;
      gbtNfgSupport newSupport = support->Undominated(true, players,
						      gnull, status);
      
      if (newSupport == support) {
	break;
      }
      else {
	support = newSupport;
      }
    }

    if (support->NumPlayers() == 2) {
      gbtNfgNashEnumMixed<double> algorithm;
      algorithm.SetStopAfter(2);
      return algorithm.Solve(support, p_status);
    }
    else {
      gbtNfgNashEnumPoly algorithm;
      algorithm.SetStopAfter(2);
      return algorithm.Solve(support, p_status);
    }
  }
  catch (...) {
    return gbtMixedNashSet();
  }
}

//========================================================================
//                       class gbtPanelNfgTwoNash
//========================================================================

class gbtPanelNfgTwoNash : public gbtPanelNfgNashAlgorithm {
public:
  gbtPanelNfgTwoNash(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelNfgTwoNash::gbtPanelNfgTwoNash(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, 
					   wxT("TwoNashSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find two Nash equilibria")),
		   0, wxALL | wxCENTER, 5);

  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("This algorithm requires no parameters")),
		   0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *gbtPanelNfgTwoNash::GetAlgorithm(void) const
{
  return new gbtNfgNashTwoNash;
}

//========================================================================
//                         class gbtNfgNashAllNash
//========================================================================

class gbtNfgNashAllNash : public gbtNfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "AllNash"; }
  gbtMixedNashSet Solve(const gbtNfgGame &, gbtStatus &);
};

gbtMixedNashSet gbtNfgNashAllNash::Solve(const gbtNfgGame &p_game,
						gbtStatus &p_status)
{
  gbtArray<int> players(p_game->NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gbtNfgSupport support = p_game->NewNfgSupport();

    while (true) {
      gbtNullStatus status;
      gbtNullOutput gnull;
      gbtNfgSupport newSupport = support->Undominated(true, players,
						      gnull, status);
      
      if (newSupport == support) {
	break;
      }
      else {
	support = newSupport;
      }
    }

    if (support->NumPlayers() == 2) {
      gbtNfgNashEnumMixed<double> algorithm;
      algorithm.SetStopAfter(0);
      return algorithm.Solve(support, p_status);
    }
    else {
      gbtNfgNashEnumPoly algorithm;
      algorithm.SetStopAfter(0);
      return algorithm.Solve(support, p_status);
    }
  }
  catch (...) {
    return gbtMixedNashSet();
  }
}

//========================================================================
//                       class gbtPanelNfgAllNash
//========================================================================

class gbtPanelNfgAllNash : public gbtPanelNfgNashAlgorithm {
public:
  gbtPanelNfgAllNash(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelNfgAllNash::gbtPanelNfgAllNash(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   wxT("AllNashSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find all Nash equilibria")),
		   0, wxALL | wxCENTER, 5);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("This algorithm requires no parameters")),
		   0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *gbtPanelNfgAllNash::GetAlgorithm(void) const
{
  return new gbtNfgNashAllNash;
}

//========================================================================
//                         class gbtNfgNashOnePerfect
//========================================================================

class gbtNfgNashOnePerfect : public gbtNfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "OnePerfect"; }
  gbtMixedNashSet Solve(const gbtNfgGame &, gbtStatus &);
};

gbtMixedNashSet gbtNfgNashOnePerfect::Solve(const gbtNfgGame &p_game,
						   gbtStatus &p_status)
{
  gbtArray<int> players(p_game->NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gbtNullStatus status;
    gbtNullOutput gnull;
    /* one round of elimination of weakly dominated strategies */
    gbtNfgSupport support = p_game->NewNfgSupport()->Undominated(false, players, gnull, status);

    gbtNfgNashLcp<double> algorithm;
    algorithm.SetStopAfter(1);
    return algorithm.Solve(support, p_status);
  }
  catch (...) {
    return gbtMixedNashSet();
  }
}

//========================================================================
//                     class gbtPanelNfgOnePerfect
//========================================================================

class gbtPanelNfgOnePerfect : public gbtPanelNfgNashAlgorithm {
public:
  gbtPanelNfgOnePerfect(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelNfgOnePerfect::gbtPanelNfgOnePerfect(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   wxT("OnePerfectSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find one perfect Nash equilibrium")),
		   0, wxALL | wxCENTER, 5);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("This algorithm requires no parameters")),
		   0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *gbtPanelNfgOnePerfect::GetAlgorithm(void) const
{
  return new gbtNfgNashOnePerfect;
}

//========================================================================
//                         class gbtNfgNashTwoPerfect
//========================================================================

class gbtNfgNashTwoPerfect : public gbtNfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "TwoPerfect"; }
  gbtMixedNashSet Solve(const gbtNfgGame &, gbtStatus &);
};

gbtMixedNashSet gbtNfgNashTwoPerfect::Solve(const gbtNfgGame &p_game,
						   gbtStatus &p_status)
{
  gbtArray<int> players(p_game->NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gbtNullStatus status;
    gbtNullOutput gnull;
    /* one round of elimination of weakly dominated strategies */
    gbtNfgSupport support = p_game->NewNfgSupport()->Undominated(false, players, gnull, status);

    gbtNfgNashEnumMixed<double> algorithm;
    algorithm.SetStopAfter(2);
    return algorithm.Solve(support, p_status);
  }
  catch (...) {
    return gbtMixedNashSet();
  }
}

//========================================================================
//                     class gbtPanelNfgTwoPerfect
//========================================================================

class gbtPanelNfgTwoPerfect : public gbtPanelNfgNashAlgorithm {
public:
  gbtPanelNfgTwoPerfect(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelNfgTwoPerfect::gbtPanelNfgTwoPerfect(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   wxT("TwoPerfectSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find two perfect Nash equilibria")),
		   0, wxALL | wxCENTER, 5);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("This algorithm requires no parameters")),
		   0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *gbtPanelNfgTwoPerfect::GetAlgorithm(void) const
{
  return new gbtNfgNashTwoPerfect;
}

//========================================================================
//                         class gbtNfgNashAllPerfect
//========================================================================

class gbtNfgNashAllPerfect : public gbtNfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "AllPerfect"; }
  gbtMixedNashSet Solve(const gbtNfgGame &, gbtStatus &);
};

gbtMixedNashSet gbtNfgNashAllPerfect::Solve(const gbtNfgGame &p_game,
						   gbtStatus &p_status)
{
  gbtArray<int> players(p_game->NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gbtNullStatus status;
    gbtNullOutput gnull;
    /* one round of elimination of weakly dominated strategies */
    gbtNfgSupport support = p_game->NewNfgSupport()->Undominated(false, players, gnull, status);

    gbtNfgNashEnumMixed<double> algorithm;
    algorithm.SetStopAfter(0);
    return algorithm.Solve(support, p_status);
  }
  catch (...) {
    return gbtMixedNashSet();
  }
}

//========================================================================
//                     class gbtPanelNfgAllPerfect
//========================================================================

class gbtPanelNfgAllPerfect : public gbtPanelNfgNashAlgorithm {
public:
  gbtPanelNfgAllPerfect(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelNfgAllPerfect::gbtPanelNfgAllPerfect(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   wxT("AllPerfectSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find all perfect Nash equilibria")),
		   0, wxALL | wxCENTER, 5);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("This algorithm requires no parameters")),
		   0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *gbtPanelNfgAllPerfect::GetAlgorithm(void) const
{
  return new gbtNfgNashAllPerfect;
}

//========================================================================
//                      class gbtPanelNfgEnumPure
//========================================================================

class gbtPanelNfgEnumPure : public gbtPanelNfgNashAlgorithm {
private:
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  gbtPanelNfgEnumPure(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtPanelNfgEnumPure, gbtPanelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, gbtPanelNfgEnumPure::OnFindAll)
END_EVENT_TABLE()

gbtPanelNfgEnumPure::gbtPanelNfgEnumPure(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, 
					   wxT("EnumPureSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria by enumerating pure strategies")),
		   0, wxALL | wxCENTER, 5);

  wxStaticBox *stopAfterBox = new wxStaticBox(this, wxID_STATIC,
					      _("Number to find"));
  wxStaticBoxSizer *stopAfterSizer = new wxStaticBoxSizer(stopAfterBox,
							  wxHORIZONTAL);
  m_findAll = new wxCheckBox(this, idCHECKBOX_FINDALL, _("Find all"));
  m_findAll->SetValue(false);
  stopAfterSizer->Add(m_findAll, 0, wxALL | wxCENTER, 5);
  stopAfterSizer->Add(new wxStaticText(this, wxID_STATIC, _("Stop after")),
		      0, wxALL | wxCENTER, 5);
  m_stopAfter = new wxSpinCtrl(this, -1, wxT("1"),
			       wxDefaultPosition, wxDefaultSize,
			       wxSP_ARROW_KEYS, 1, 10000);
  stopAfterSizer->Add(m_stopAfter, 0, wxALL | wxCENTER, 5);
  centerSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 0);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

void gbtPanelNfgEnumPure::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtNfgNashAlgorithm *gbtPanelNfgEnumPure::GetAlgorithm(void) const
{
  gbtNfgNashEnumPure *algorithm = new gbtNfgNashEnumPure;
  algorithm->SetStopAfter((m_findAll->GetValue()) ?
			  0 : m_stopAfter->GetValue());
  return algorithm;
}

//========================================================================
//                      class gbtPanelNfgEnumMixed
//========================================================================

class gbtPanelNfgEnumMixed : public gbtPanelNfgNashAlgorithm {
private:
  wxRadioBox *m_precision;
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  gbtPanelNfgEnumMixed(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtPanelNfgEnumMixed, gbtPanelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, gbtPanelNfgEnumMixed::OnFindAll)
END_EVENT_TABLE()

gbtPanelNfgEnumMixed::gbtPanelNfgEnumMixed(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   wxT("EnumMixedSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria by enumerating mixed strategies")),
		   0, wxALL | wxCENTER, 5);

  wxString precisionChoices[] = { _("Floating point"), _("Rational") };
  m_precision = new wxRadioBox(this, -1, _("Precision"),
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 1, wxRA_SPECIFY_ROWS);
  centerSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);

  wxStaticBox *stopAfterBox = new wxStaticBox(this, wxID_STATIC,
					      _("Number to find"));
  wxStaticBoxSizer *stopAfterSizer = new wxStaticBoxSizer(stopAfterBox,
							  wxHORIZONTAL);
  m_findAll = new wxCheckBox(this, idCHECKBOX_FINDALL, _("Find all"));
  m_findAll->SetValue(false);
  stopAfterSizer->Add(m_findAll, 0, wxALL | wxCENTER, 5);
  stopAfterSizer->Add(new wxStaticText(this, wxID_STATIC, _("Stop after")),
		      0, wxALL | wxCENTER, 5);
  m_stopAfter = new wxSpinCtrl(this, -1, wxT("1"),
			       wxDefaultPosition, wxDefaultSize,
			       wxSP_ARROW_KEYS, 1, 10000);
  stopAfterSizer->Add(m_stopAfter, 0, wxALL | wxCENTER, 5);
  centerSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 0);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

void gbtPanelNfgEnumMixed::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtNfgNashAlgorithm *gbtPanelNfgEnumMixed::GetAlgorithm(void) const
{
  if (m_precision->GetSelection() == 0) {
    gbtNfgNashEnumMixed<double> *algorithm = new gbtNfgNashEnumMixed<double>;
    algorithm->SetStopAfter((m_findAll->GetValue()) ?
			    0 : m_stopAfter->GetValue());
    return algorithm;
  }
  else {
    gbtNfgNashEnumMixed<gbtRational> *algorithm = new gbtNfgNashEnumMixed<gbtRational>;
    algorithm->SetStopAfter((m_findAll->GetValue()) ?
			    0 : m_stopAfter->GetValue());
    return algorithm;
  }
}

//========================================================================
//                         class gbtPanelNfgLcp
//========================================================================

const int idCHECKBOX_LIMITDEPTH = 2002;

class gbtPanelNfgLcp : public gbtPanelNfgNashAlgorithm {
private:
  wxRadioBox *m_precision;
  wxCheckBox *m_findAll, *m_limitDepth;
  wxSpinCtrl *m_stopAfter, *m_maxDepth;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);
  void OnStopAfter(wxSpinEvent &);
  void OnLimitDepth(wxCommandEvent &);

public:
  gbtPanelNfgLcp(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtPanelNfgLcp, gbtPanelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, gbtPanelNfgLcp::OnFindAll)
  EVT_SPINCTRL(idSPINCTRL_STOPAFTER, gbtPanelNfgLcp::OnStopAfter)
  EVT_CHECKBOX(idCHECKBOX_LIMITDEPTH, gbtPanelNfgLcp::OnLimitDepth)
END_EVENT_TABLE()

gbtPanelNfgLcp::gbtPanelNfgLcp(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, wxT("LcpSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria via linear complementarity program")),
		   0, wxALL | wxCENTER, 5);

  wxString precisionChoices[] = { _("Floating point"), _("Rational") };
  m_precision = new wxRadioBox(this, -1, _("Precision"),
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 1, wxRA_SPECIFY_ROWS);
  centerSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);

  wxStaticBox *stopAfterBox = new wxStaticBox(this, wxID_STATIC,
					      _("Number to find"));
  wxStaticBoxSizer *stopAfterSizer = new wxStaticBoxSizer(stopAfterBox,
							  wxHORIZONTAL);
  m_findAll = new wxCheckBox(this, idCHECKBOX_FINDALL, _("Find all"));
  m_findAll->SetValue(false);
  stopAfterSizer->Add(m_findAll, 0, wxALL | wxCENTER, 5);
  stopAfterSizer->Add(new wxStaticText(this, wxID_STATIC, _("Stop after")),
		      0, wxALL | wxCENTER, 5);
  m_stopAfter = new wxSpinCtrl(this, idSPINCTRL_STOPAFTER, wxT("1"),
			       wxDefaultPosition, wxDefaultSize,
			       wxSP_ARROW_KEYS, 1, 10000);
  stopAfterSizer->Add(m_stopAfter, 0, wxALL | wxCENTER, 5);
  centerSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 5);

  wxStaticBox *depthBox = new wxStaticBox(this, wxID_STATIC, 
					  _("Algorithm behavior"));
  wxStaticBoxSizer *depthSizer = new wxStaticBoxSizer(depthBox,
						      wxHORIZONTAL);
  m_limitDepth = new wxCheckBox(this, idCHECKBOX_LIMITDEPTH, _("Limit depth"));
  m_limitDepth->SetValue(false);
  m_limitDepth->Enable(false);
  depthSizer->Add(m_limitDepth, 0, wxALL | wxCENTER, 5);
  depthSizer->Add(new wxStaticText(this, wxID_STATIC, _("Maximum depth")),
		  0, wxALL | wxCENTER, 5);
  m_maxDepth = new wxSpinCtrl(this, -1, wxT("10"),
			      wxDefaultPosition, wxDefaultSize,
			      wxSP_ARROW_KEYS, 1, 1000);
  m_maxDepth->Enable(false);
  depthSizer->Add(m_maxDepth, 0, wxALL | wxCENTER, 5);
  centerSizer->Add(depthSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 0);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

void gbtPanelNfgLcp::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
  m_limitDepth->Enable(m_findAll->GetValue() || 
		       m_stopAfter->GetValue() > 1);
  m_maxDepth->Enable((m_findAll->GetValue() || m_stopAfter->GetValue() > 1) &&
		     m_limitDepth->GetValue());
}

void gbtPanelNfgLcp::OnStopAfter(wxSpinEvent &)
{
  m_limitDepth->Enable(m_stopAfter->GetValue() > 1);
  m_maxDepth->Enable(m_stopAfter->GetValue() > 1 && m_limitDepth->GetValue());
}

void gbtPanelNfgLcp::OnLimitDepth(wxCommandEvent &)
{
  m_maxDepth->Enable(m_limitDepth->GetValue());
}

gbtNfgNashAlgorithm *gbtPanelNfgLcp::GetAlgorithm(void) const
{
  if (m_precision->GetSelection() == 0) {
    gbtNfgNashLcp<double> *algorithm = new gbtNfgNashLcp<double>;
    algorithm->SetStopAfter((m_findAll->GetValue()) ?
			    0 : m_stopAfter->GetValue());
    algorithm->SetMaxDepth((m_limitDepth->GetValue()) ?
			   m_maxDepth->GetValue() : 0);
    return algorithm;
  }
  else {
    gbtNfgNashLcp<gbtRational> *algorithm = new gbtNfgNashLcp<gbtRational>;
    algorithm->SetStopAfter((m_findAll->GetValue()) ?
			    0 : m_stopAfter->GetValue());
    algorithm->SetMaxDepth((m_limitDepth->GetValue()) ?
			   m_maxDepth->GetValue() : 0);
    return algorithm;
  }
}

//========================================================================
//                         class gbtPanelNfgLp
//========================================================================

class gbtPanelNfgLp : public gbtPanelNfgNashAlgorithm {
private:
  wxRadioBox *m_precision;
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  gbtPanelNfgLp(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtPanelNfgLp, gbtPanelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, gbtPanelNfgLp::OnFindAll)
END_EVENT_TABLE()

gbtPanelNfgLp::gbtPanelNfgLp(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, _("LpSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria via linear program")),
		   0, wxALL | wxCENTER, 5);

  wxString precisionChoices[] = { _("Floating point"), _("Rational") };
  m_precision = new wxRadioBox(this, -1, _("Precision"),
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 1, wxRA_SPECIFY_ROWS);
  centerSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);

  // The "find all" feature of LpSolve currently does not work;
  // therefore, the controls are disabled in this version
  wxStaticBox *stopAfterBox = new wxStaticBox(this, wxID_STATIC,
					      _("Number to find"));
  wxStaticBoxSizer *stopAfterSizer = new wxStaticBoxSizer(stopAfterBox,
							  wxHORIZONTAL);
  m_findAll = new wxCheckBox(this, idCHECKBOX_FINDALL, _("Find all"));
  m_findAll->SetValue(false);
  m_findAll->Enable(false);
  stopAfterSizer->Add(m_findAll, 0, wxALL | wxCENTER, 5);
  stopAfterSizer->Add(new wxStaticText(this, wxID_STATIC, _("Stop after")),
		      0, wxALL | wxCENTER, 5);
  m_stopAfter = new wxSpinCtrl(this, -1, wxT("1"),
			       wxDefaultPosition, wxDefaultSize,
			       wxSP_ARROW_KEYS, 1, 10000);
  m_stopAfter->Enable(false);
  stopAfterSizer->Add(m_stopAfter, 0, wxALL | wxCENTER, 5);
  centerSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 0);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

void gbtPanelNfgLp::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtNfgNashAlgorithm *gbtPanelNfgLp::GetAlgorithm(void) const
{
  if (m_precision->GetSelection() == 0) {
    gbtNfgNashLp<double> *algorithm = new gbtNfgNashLp<double>;
    return algorithm;
  }
  else {
    gbtNfgNashLp<gbtRational> *algorithm = new gbtNfgNashLp<gbtRational>;
    return algorithm;
  }
}

//========================================================================
//                        class gbtPanelNfgLiap
//========================================================================

class gbtPanelNfgLiap : public gbtPanelNfgNashAlgorithm {
private:
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter, *m_numTries;
  wxSpinCtrl *m_maxits;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  gbtPanelNfgLiap(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtPanelNfgLiap, gbtPanelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, gbtPanelNfgLiap::OnFindAll)
END_EVENT_TABLE()

gbtPanelNfgLiap::gbtPanelNfgLiap(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, 
					   wxT("LiapSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria using Lyapunov function minimization")),
		   0, wxALL | wxCENTER, 5);

  wxStaticBox *stopAfterBox = new wxStaticBox(this, wxID_STATIC,
					      _("Number to find"));
  wxStaticBoxSizer *stopAfterSizer = new wxStaticBoxSizer(stopAfterBox,
							  wxHORIZONTAL);
  m_findAll = new wxCheckBox(this, idCHECKBOX_FINDALL, _("No limit"));
  m_findAll->SetValue(false);
  stopAfterSizer->Add(m_findAll, 0, wxALL | wxCENTER, 5);
  stopAfterSizer->Add(new wxStaticText(this, wxID_STATIC, _("Stop after")),
		      0, wxALL | wxCENTER, 5);
  m_stopAfter = new wxSpinCtrl(this, -1, wxT("1"),
			       wxDefaultPosition, wxDefaultSize,
			       wxSP_ARROW_KEYS, 1, 10000);
  stopAfterSizer->Add(m_stopAfter, 0, wxALL | wxCENTER, 5);
  centerSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 5);

  wxStaticBox *algorithmBox = new wxStaticBox(this, wxID_STATIC,
					      _("Algorithm behavior"));
  wxStaticBoxSizer *algorithmSizer = new wxStaticBoxSizer(algorithmBox,
							  wxHORIZONTAL);

  wxFlexGridSizer *paramSizer = new wxFlexGridSizer(2);
  paramSizer->Add(new wxStaticText(this, wxID_STATIC, 
				   _("Number of restarts")),
		  0, wxALL | wxCENTER, 5);
  m_numTries = new wxSpinCtrl(this, -1, wxT("100"),
			      wxDefaultPosition, wxDefaultSize,
			      wxSP_ARROW_KEYS, 1, 10000);
  paramSizer->Add(m_numTries, 0, wxALL, 5);

  paramSizer->Add(new wxStaticText(this, wxID_STATIC,
				   _("Maximum iterations in minimization")),
		  0, wxALL | wxCENTER, 5);
  m_maxits = new wxSpinCtrl(this, -1, wxT("500"),
			    wxDefaultPosition, wxDefaultSize,
			    wxSP_ARROW_KEYS, 10, 1000);
  paramSizer->Add(m_maxits, 0, wxALL | wxCENTER, 5);
  algorithmSizer->Add(paramSizer, 0, wxALL, 5);

  centerSizer->Add(algorithmSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 0);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

void gbtPanelNfgLiap::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtNfgNashAlgorithm *gbtPanelNfgLiap::GetAlgorithm(void) const
{
  gbtNfgNashLiap *algorithm = new gbtNfgNashLiap;
  algorithm->SetStopAfter((m_findAll->GetValue()) ?
			  0 : m_stopAfter->GetValue());
  algorithm->SetNumTries(m_numTries->GetValue());
  algorithm->SetMaxitsN(m_maxits->GetValue());
  return algorithm;
}

//========================================================================
//                      class gbtPanelNfgEnumPoly
//========================================================================

class gbtPanelNfgEnumPoly : public gbtPanelNfgNashAlgorithm {
private:
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  gbtPanelNfgEnumPoly(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtPanelNfgEnumPoly, gbtPanelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, gbtPanelNfgEnumPoly::OnFindAll)
END_EVENT_TABLE()

gbtPanelNfgEnumPoly::gbtPanelNfgEnumPoly(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, 
					   wxT("PolEnumSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria via solving gbtPolyUni equations")),
		   0, wxALL | wxCENTER, 5);

  wxStaticBox *stopAfterBox = new wxStaticBox(this, wxID_STATIC,
					      _("Number to find"));
  wxStaticBoxSizer *stopAfterSizer = new wxStaticBoxSizer(stopAfterBox,
							  wxHORIZONTAL);
  m_findAll = new wxCheckBox(this, idCHECKBOX_FINDALL, _("Find all"));
  m_findAll->SetValue(false);
  stopAfterSizer->Add(m_findAll, 0, wxALL | wxCENTER, 5);
  stopAfterSizer->Add(new wxStaticText(this, wxID_STATIC, _("Stop after")),
		      0, wxALL | wxCENTER, 5);
  m_stopAfter = new wxSpinCtrl(this, -1, wxT("1"),
			       wxDefaultPosition, wxDefaultSize,
			       wxSP_ARROW_KEYS, 1, 10000);
  stopAfterSizer->Add(m_stopAfter, 0, wxALL, 5);
  centerSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 0);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

void gbtPanelNfgEnumPoly::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtNfgNashAlgorithm *gbtPanelNfgEnumPoly::GetAlgorithm(void) const
{
  gbtNfgNashEnumPoly *algorithm = new gbtNfgNashEnumPoly;
  algorithm->SetStopAfter((m_findAll->GetValue()) ?
			  0 : m_stopAfter->GetValue());
  return algorithm;
}

//========================================================================
//                         class gbtPanelNfgLogit
//========================================================================

class gbtPanelNfgLogit : public gbtPanelNfgNashAlgorithm {
public:
  gbtPanelNfgLogit(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelNfgLogit::gbtPanelNfgLogit(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, 
					   wxT("QreSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria via tracing logit equilibria")),
		   0, wxALL | wxCENTER, 5);

  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("This algorithm requires no parameters")),
		   0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 0);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *gbtPanelNfgLogit::GetAlgorithm(void) const
{
  gbtNfgNashLogit *algorithm = new gbtNfgNashLogit;
  algorithm->SetFullGraph(false);
  algorithm->SetMaxLambda(1000000000);
  return algorithm;
}

//========================================================================
//                       class gbtPanelNfgSimpdiv
//========================================================================

const int idCHECKBOX_USELEASH = 2002;

class gbtPanelNfgSimpdiv : public gbtPanelNfgNashAlgorithm {
private:
  wxRadioBox *m_precision;
  wxCheckBox *m_useLeash;
  wxSpinCtrl *m_leashLength, *m_numRestarts;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);
  void OnUseLeash(wxCommandEvent &);

public:
  gbtPanelNfgSimpdiv(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtPanelNfgSimpdiv, gbtPanelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_USELEASH, gbtPanelNfgSimpdiv::OnUseLeash)
END_EVENT_TABLE()

gbtPanelNfgSimpdiv::gbtPanelNfgSimpdiv(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, 
					   wxT("SimpdivSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria via simplicial subdivision")),
		   0, wxALL | wxCENTER, 5);

  wxString precisionChoices[] = { _("Floating point"), _("Rational") };
  m_precision = new wxRadioBox(this, -1, _("Precision"),
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 1, wxRA_SPECIFY_ROWS);
  centerSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);

  wxStaticBox *algorithmBox = new wxStaticBox(this, wxID_STATIC,
					      _("Algorithm behavior"));
  wxStaticBoxSizer *algorithmSizer = new wxStaticBoxSizer(algorithmBox,
							  wxVERTICAL);
  
  wxBoxSizer *leashSizer = new wxBoxSizer(wxHORIZONTAL);
  m_useLeash = new wxCheckBox(this, idCHECKBOX_USELEASH, _("Use leash"));
  m_useLeash->SetValue(false);
  leashSizer->Add(m_useLeash, 0, wxALL | wxCENTER, 5);
  leashSizer->Add(new wxStaticText(this, wxID_STATIC, _("Leash length")),
		  0, wxALL | wxCENTER, 5);
  m_leashLength = new wxSpinCtrl(this, -1, wxT("100"),
				 wxDefaultPosition, wxDefaultSize,
				 wxSP_ARROW_KEYS, 1, 10000);
  m_leashLength->Enable(false);
  leashSizer->Add(m_leashLength, 0, wxALL | wxCENTER, 5);
  algorithmSizer->Add(leashSizer, 0, wxALL | wxCENTER, 5);

  wxBoxSizer *restartSizer = new wxBoxSizer(wxHORIZONTAL);
  restartSizer->Add(new wxStaticText(this, wxID_STATIC, 
				     _("Number of restarts")),
		    0, wxALL | wxCENTER, 5);
  m_numRestarts = new wxSpinCtrl(this, -1, wxT("20"),
				 wxDefaultPosition, wxDefaultSize,
				 wxSP_ARROW_KEYS, 1, 10000);
  restartSizer->Add(m_numRestarts, 0, wxALL | wxCENTER, 5);
  algorithmSizer->Add(restartSizer, 0, wxALL | wxCENTER, 5);

  centerSizer->Add(algorithmSizer, 0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 0);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

void gbtPanelNfgSimpdiv::OnUseLeash(wxCommandEvent &)
{
  m_leashLength->Enable(m_useLeash->GetValue());
}

gbtNfgNashAlgorithm *gbtPanelNfgSimpdiv::GetAlgorithm(void) const
{
  if (m_precision->GetSelection() == 0) {
    gbtNfgNashSimpdiv<double> *algorithm = new gbtNfgNashSimpdiv<double>;
    algorithm->SetLeashLength((m_useLeash->GetValue()) ?
			      m_leashLength->GetValue() : 0);
    algorithm->SetNumRestarts(m_numRestarts->GetValue());
    return algorithm;
  }
  else {
    gbtNfgNashSimpdiv<gbtRational> *algorithm = new gbtNfgNashSimpdiv<gbtRational>;
    algorithm->SetLeashLength((m_useLeash->GetValue()) ?
			      m_leashLength->GetValue() : 0);
    algorithm->SetNumRestarts(m_numRestarts->GetValue());
    return algorithm;
  }
}

//========================================================================
//                      class gbtPanelNfgYamamoto
//========================================================================

class gbtPanelNfgYamamoto : public gbtPanelNfgNashAlgorithm {
public:
  gbtPanelNfgYamamoto(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelNfgYamamoto::gbtPanelNfgYamamoto(wxWindow *p_parent)
  : gbtPanelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 _("This algorithm requires no parameters")),
		0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *gbtPanelNfgYamamoto::GetAlgorithm(void) const
{
  return new gbtNfgNashYamamoto;
}

//========================================================================
//                        class dialogNfgNash
//========================================================================

const int idTREECTRL_ALGORITHMS = 2000;

BEGIN_EVENT_TABLE(dialogNfgNash, wxDialog)
  EVT_TREE_SEL_CHANGING(idTREECTRL_ALGORITHMS,
			dialogNfgNash::OnSelectionChanging)
  EVT_TREE_ITEM_COLLAPSING(idTREECTRL_ALGORITHMS,
			   dialogNfgNash::OnItemCollapsing)
END_EVENT_TABLE()

dialogNfgNash::dialogNfgNash(wxWindow *p_parent, const gbtNfgGame &p_game)
  : wxDialog(p_parent, -1, _("Compute Nash equilibria"), wxDefaultPosition),
    m_algorithms(0)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  m_algPanelSizer = new wxBoxSizer(wxHORIZONTAL);
  m_algorithmTree = new wxTreeCtrl(this, idTREECTRL_ALGORITHMS,
				   wxDefaultPosition, wxSize(200, 400),
				   wxTR_NO_BUTTONS | wxTR_HIDE_ROOT |
				   wxTR_NO_LINES | wxTR_ROW_LINES);
  wxTreeItemId init = LoadAlgorithms(p_game);
  m_algPanelSizer->Add(m_algorithmTree, 1, wxALL, 5);
  m_currentPanel = m_algorithms(init);
  m_algPanelSizer->Add(m_currentPanel, 0, wxALL | wxCENTER, 5);
  topSizer->Add(m_algPanelSizer, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, _("OK"));
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, _("Cancel")), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, _("Help")), 0, wxALL, 5);

  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
  CenterOnParent();
  m_algorithmTree->SelectItem(init);
}

int dialogNfgNash::LoadAlgorithms(const gbtNfgGame &p_nfg)
{
  wxTreeItemId id;

  // Eventually, these should be loaded from wxConfig; for now,
  // I am going to hard-code them
  wxTreeItemId root = m_algorithmTree->AddRoot(_("Algorithms"));
  wxTreeItemId standard = m_algorithmTree->AppendItem(root,
						      _("Standard algorithms"));
  m_algorithmTree->SetItemBold(standard);
  // This is added to silence some BC warnings
  gbtPanelNfgNashAlgorithm *panel;
  id = m_algorithmTree->AppendItem(standard, _("One Nash equilibrium"));
  m_algorithms.Define(id, panel = new gbtPanelNfgOneNash(this));
  wxTreeItemId init = id;
  
  id = m_algorithmTree->AppendItem(standard, _("Two Nash equilibria"));
  m_algorithms.Define(id, panel = new gbtPanelNfgTwoNash(this));

  id = m_algorithmTree->AppendItem(standard, _("All Nash equilibria"));
  m_algorithms.Define(id, panel = new gbtPanelNfgAllNash(this));

  if (p_nfg->NumPlayers() == 2) {
    id = m_algorithmTree->AppendItem(standard, _("One perfect equilibrium"));
    m_algorithms.Define(id, panel = new gbtPanelNfgOnePerfect(this));

    id = m_algorithmTree->AppendItem(standard, _("Two perfect equilibria"));
    m_algorithms.Define(id, panel = new gbtPanelNfgTwoPerfect(this));

    id = m_algorithmTree->AppendItem(standard, _("All perfect equilibria"));
    m_algorithms.Define(id, panel = new gbtPanelNfgAllPerfect(this));
  }

  wxTreeItemId custom = m_algorithmTree->AppendItem(root,
						    _("Custom algorithms"));
  m_algorithmTree->SetItemBold(custom);

  id = m_algorithmTree->AppendItem(custom, wxT("EnumPureSolve"));
  m_algorithms.Define(id, panel = new gbtPanelNfgEnumPure(this));

  if (p_nfg->NumPlayers() == 2) {
    id = m_algorithmTree->AppendItem(custom, wxT("EnumMixedSolve"));
    m_algorithms.Define(id, panel = new gbtPanelNfgEnumMixed(this));

    id = m_algorithmTree->AppendItem(custom, wxT("LcpSolve"));
    m_algorithms.Define(id, panel = new gbtPanelNfgLcp(this));

    if (p_nfg->IsConstSum()) {
      id = m_algorithmTree->AppendItem(custom, wxT("LpSolve"));
      m_algorithms.Define(id, panel = new gbtPanelNfgLp(this));
    }
  }

  id = m_algorithmTree->AppendItem(custom, wxT("LiapSolve"));
  m_algorithms.Define(id, panel = new gbtPanelNfgLiap(this));

  id = m_algorithmTree->AppendItem(custom, wxT("PolEnumSolve"));
  m_algorithms.Define(id, panel = new gbtPanelNfgEnumPoly(this));
  
  id = m_algorithmTree->AppendItem(custom, wxT("QreSolve"));
  m_algorithms.Define(id, panel = new gbtPanelNfgLogit(this));

  id = m_algorithmTree->AppendItem(custom, wxT("SimpdivSolve"));
  m_algorithms.Define(id, panel = new gbtPanelNfgSimpdiv(this));

  id = m_algorithmTree->AppendItem(custom, wxT("YamamotoSolve"));
  m_algorithms.Define(id, panel = new gbtPanelNfgYamamoto(this));

  m_algorithmTree->Expand(standard);
  m_algorithmTree->Expand(custom);
  m_algorithmTree->Expand(root);

  return init;
}

void dialogNfgNash::OnSelectionChanging(wxTreeEvent &p_event)
{
  wxPanel *panel = m_algorithms(p_event.GetItem());
  if (!panel) {
    p_event.Veto();
    return;
  }

  if (m_currentPanel) {
    m_currentPanel->Show(false);
  }

  m_currentPanel = panel;
  panel->Show(true);
  m_algPanelSizer->Remove(1);
  m_algPanelSizer->Add(panel, 0, wxALL | wxCENTER, 5);
  m_algPanelSizer->Layout();
  GetSizer()->Layout();
  GetSizer()->Fit(this);
  CenterOnParent();
}

void dialogNfgNash::OnItemCollapsing(wxTreeEvent &p_event)
{
  p_event.Veto();
}

gbtNfgNashAlgorithm *dialogNfgNash::GetAlgorithm(void) const
{
  if (m_algorithms(m_algorithmTree->GetSelection())) {
    return m_algorithms(m_algorithmTree->GetSelection())->GetAlgorithm();
  }
  else {
    return 0;
  }
}

static gbtOutput &operator<<(gbtOutput &p_stream, wxTreeItemId)
{ return p_stream; }

#include "base/gmap.imp"
template class gbtBaseMap<wxTreeItemId, gbtPanelNfgNashAlgorithm *>;
template class gbtOrdMap<wxTreeItemId, gbtPanelNfgNashAlgorithm *>;
