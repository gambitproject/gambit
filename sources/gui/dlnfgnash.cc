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

class panelNfgNashAlgorithm : public wxPanel {
public:
  panelNfgNashAlgorithm(wxWindow *p_parent) : wxPanel(p_parent, -1) { }

  virtual gbtNfgNashAlgorithm *GetAlgorithm(void) const = 0;
};

//========================================================================
//                         class nfgOneNash
//========================================================================

class nfgOneNash : public gbtNfgNashAlgorithm {
public:
  gText GetAlgorithm(void) const { return "OneNash"; }
  gList<MixedSolution> Solve(const gbtNfgSupport &, gStatus &);
};

gList<MixedSolution> nfgOneNash::Solve(const gbtNfgSupport &p_support,
				       gStatus &p_status)
{
  gArray<int> players(p_support.GetGame().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gNullStatus status;
    gNullOutput gnull;
    /* one round of elimination of weakly dominated strategies */
    gbtNfgSupport support = p_support.Undominated(false, players, gnull, status);

    if (p_support.GetGame().NumPlayers() == 2) {
      if (IsConstSum(p_support.GetGame())) {
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
    return gList<MixedSolution>();
  }
}

//========================================================================
//                       class panelNfgOneNash
//========================================================================

class panelNfgOneNash : public panelNfgNashAlgorithm {
public:
  panelNfgOneNash(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgOneNash::panelNfgOneNash(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, "OneNashSolve");
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Find one Nash equilibrium"),
		   0, wxALL | wxCENTER, 5);

  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "This algorithm requires no parameters"),
		   0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *panelNfgOneNash::GetAlgorithm(void) const
{
  return new nfgOneNash;
}

//========================================================================
//                         class nfgTwoNash
//========================================================================

class nfgTwoNash : public gbtNfgNashAlgorithm {
public:
  gText GetAlgorithm(void) const { return "TwoNash"; }
  gList<MixedSolution> Solve(const gbtNfgSupport &, gStatus &);
};

gList<MixedSolution> nfgTwoNash::Solve(const gbtNfgSupport &p_support,
				       gStatus &p_status)
{
  gArray<int> players(p_support.GetGame().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gbtNfgSupport support(p_support);

    while (true) {
      gNullStatus status;
      gNullOutput gnull;
      gbtNfgSupport newSupport = support.Undominated(true, players,
						 gnull, status);
      
      if (newSupport == support) {
	break;
      }
      else {
	support = newSupport;
      }
    }

    if (p_support.GetGame().NumPlayers() == 2) {
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
    return gList<MixedSolution>();
  }
}

//========================================================================
//                       class panelNfgTwoNash
//========================================================================

class panelNfgTwoNash : public panelNfgNashAlgorithm {
public:
  panelNfgTwoNash(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgTwoNash::panelNfgTwoNash(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, "TwoNashSolve");
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Find two Nash equilibria"),
		   0, wxALL | wxCENTER, 5);

  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "This algorithm requires no parameters"),
		   0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *panelNfgTwoNash::GetAlgorithm(void) const
{
  return new nfgTwoNash;
}

//========================================================================
//                         class nfgAllNash
//========================================================================

class nfgAllNash : public gbtNfgNashAlgorithm {
public:
  gText GetAlgorithm(void) const { return "AllNash"; }
  gList<MixedSolution> Solve(const gbtNfgSupport &, gStatus &);
};

gList<MixedSolution> nfgAllNash::Solve(const gbtNfgSupport &p_support,
				       gStatus &p_status)
{
  gArray<int> players(p_support.GetGame().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gbtNfgSupport support(p_support);

    while (true) {
      gNullStatus status;
      gNullOutput gnull;
      gbtNfgSupport newSupport = support.Undominated(true, players,
						 gnull, status);
      
      if (newSupport == support) {
	break;
      }
      else {
	support = newSupport;
      }
    }

    if (p_support.GetGame().NumPlayers() == 2) {
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
    return gList<MixedSolution>();
  }
}

//========================================================================
//                       class panelNfgAllNash
//========================================================================

class panelNfgAllNash : public panelNfgNashAlgorithm {
public:
  panelNfgAllNash(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgAllNash::panelNfgAllNash(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, "AllNashSolve");
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Find all Nash equilibria"),
		   0, wxALL | wxCENTER, 5);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "This algorithm requires no parameters"),
		   0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *panelNfgAllNash::GetAlgorithm(void) const
{
  return new nfgAllNash;
}

//========================================================================
//                         class nfgOnePerfect
//========================================================================

class nfgOnePerfect : public gbtNfgNashAlgorithm {
public:
  gText GetAlgorithm(void) const { return "OnePerfect"; }
  gList<MixedSolution> Solve(const gbtNfgSupport &, gStatus &);
};

gList<MixedSolution> nfgOnePerfect::Solve(const gbtNfgSupport &p_support,
					  gStatus &p_status)
{
  gArray<int> players(p_support.GetGame().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gNullStatus status;
    gNullOutput gnull;
    /* one round of elimination of weakly dominated strategies */
    gbtNfgSupport support = p_support.Undominated(false, players, gnull, status);

    gbtNfgNashLcp<double> algorithm;
    algorithm.SetStopAfter(1);
    return algorithm.Solve(support, p_status);
  }
  catch (...) {
    return gList<MixedSolution>();
  }
}

//========================================================================
//                     class panelNfgOnePerfect
//========================================================================

class panelNfgOnePerfect : public panelNfgNashAlgorithm {
public:
  panelNfgOnePerfect(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgOnePerfect::panelNfgOnePerfect(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   "OnePerfectSolve");
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Find one perfect Nash equilibrium"),
		   0, wxALL | wxCENTER, 5);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "This algorithm requires no parameters"),
		   0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *panelNfgOnePerfect::GetAlgorithm(void) const
{
  return new nfgOnePerfect;
}

//========================================================================
//                         class nfgTwoPerfect
//========================================================================

class nfgTwoPerfect : public gbtNfgNashAlgorithm {
public:
  gText GetAlgorithm(void) const { return "TwoPerfect"; }
  gList<MixedSolution> Solve(const gbtNfgSupport &, gStatus &);
};

gList<MixedSolution> nfgTwoPerfect::Solve(const gbtNfgSupport &p_support,
					  gStatus &p_status)
{
  gArray<int> players(p_support.GetGame().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gNullStatus status;
    gNullOutput gnull;
    /* one round of elimination of weakly dominated strategies */
    gbtNfgSupport support = p_support.Undominated(false, players, gnull, status);

    gbtNfgNashEnumMixed<double> algorithm;
    algorithm.SetStopAfter(2);
    return algorithm.Solve(support, p_status);
  }
  catch (...) {
    return gList<MixedSolution>();
  }
}

//========================================================================
//                     class panelNfgTwoPerfect
//========================================================================

class panelNfgTwoPerfect : public panelNfgNashAlgorithm {
public:
  panelNfgTwoPerfect(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgTwoPerfect::panelNfgTwoPerfect(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   "TwoPerfectSolve");
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Find two perfect Nash equilibria"),
		   0, wxALL | wxCENTER, 5);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "This algorithm requires no parameters"),
		   0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *panelNfgTwoPerfect::GetAlgorithm(void) const
{
  return new nfgTwoPerfect;
}

//========================================================================
//                         class nfgAllPerfect
//========================================================================

class nfgAllPerfect : public gbtNfgNashAlgorithm {
public:
  gText GetAlgorithm(void) const { return "AllPerfect"; }
  gList<MixedSolution> Solve(const gbtNfgSupport &, gStatus &);
};

gList<MixedSolution> nfgAllPerfect::Solve(const gbtNfgSupport &p_support,
					  gStatus &p_status)
{
  gArray<int> players(p_support.GetGame().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gNullStatus status;
    gNullOutput gnull;
    /* one round of elimination of weakly dominated strategies */
    gbtNfgSupport support = p_support.Undominated(false, players, gnull, status);

    gbtNfgNashEnumMixed<double> algorithm;
    algorithm.SetStopAfter(0);
    return algorithm.Solve(support, p_status);
  }
  catch (...) {
    return gList<MixedSolution>();
  }
}

//========================================================================
//                     class panelNfgAllPerfect
//========================================================================

class panelNfgAllPerfect : public panelNfgNashAlgorithm {
public:
  panelNfgAllPerfect(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgAllPerfect::panelNfgAllPerfect(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   "AllPerfectSolve");
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Find all perfect Nash equilibria"),
		   0, wxALL | wxCENTER, 5);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "This algorithm requires no parameters"),
		   0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *panelNfgAllPerfect::GetAlgorithm(void) const
{
  return new nfgAllPerfect;
}

//========================================================================
//                      class panelNfgEnumPure
//========================================================================

class panelNfgEnumPure : public panelNfgNashAlgorithm {
private:
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  panelNfgEnumPure(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelNfgEnumPure, panelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelNfgEnumPure::OnFindAll)
END_EVENT_TABLE()

panelNfgEnumPure::panelNfgEnumPure(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, "EnumPureSolve");
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Find Nash equilibria by enumerating "
				    "pure strategies"),
		   0, wxALL | wxCENTER, 5);

  wxStaticBox *stopAfterBox = new wxStaticBox(this, wxID_STATIC,
					      "Number to find");
  wxStaticBoxSizer *stopAfterSizer = new wxStaticBoxSizer(stopAfterBox,
							  wxHORIZONTAL);
  m_findAll = new wxCheckBox(this, idCHECKBOX_FINDALL, "Find all");
  m_findAll->SetValue(false);
  stopAfterSizer->Add(m_findAll, 0, wxALL | wxCENTER, 5);
  stopAfterSizer->Add(new wxStaticText(this, wxID_STATIC, "Stop after"),
		      0, wxALL | wxCENTER, 5);
  m_stopAfter = new wxSpinCtrl(this, -1, "1",
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

void panelNfgEnumPure::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtNfgNashAlgorithm *panelNfgEnumPure::GetAlgorithm(void) const
{
  gbtNfgNashEnumPure *algorithm = new gbtNfgNashEnumPure;
  algorithm->SetStopAfter((m_findAll->GetValue()) ?
			  0 : m_stopAfter->GetValue());
  return algorithm;
}

//========================================================================
//                      class panelNfgEnumMixed
//========================================================================

class panelNfgEnumMixed : public panelNfgNashAlgorithm {
private:
  wxRadioBox *m_precision;
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  panelNfgEnumMixed(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelNfgEnumMixed, panelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelNfgEnumMixed::OnFindAll)
END_EVENT_TABLE()

panelNfgEnumMixed::panelNfgEnumMixed(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   "EnumMixedSolve");
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Find Nash equilibria by enumerating "
				    "mixed strategies"),
		   0, wxALL | wxCENTER, 5);

  wxString precisionChoices[] = { "Floating point", "Rational" };
  m_precision = new wxRadioBox(this, -1, "Precision",
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 1, wxRA_SPECIFY_ROWS);
  centerSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);

  wxStaticBox *stopAfterBox = new wxStaticBox(this, wxID_STATIC,
					      "Number to find");
  wxStaticBoxSizer *stopAfterSizer = new wxStaticBoxSizer(stopAfterBox,
							  wxHORIZONTAL);
  m_findAll = new wxCheckBox(this, idCHECKBOX_FINDALL, "Find all");
  m_findAll->SetValue(false);
  stopAfterSizer->Add(m_findAll, 0, wxALL | wxCENTER, 5);
  stopAfterSizer->Add(new wxStaticText(this, wxID_STATIC, "Stop after"),
		      0, wxALL | wxCENTER, 5);
  m_stopAfter = new wxSpinCtrl(this, -1, "1",
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

void panelNfgEnumMixed::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtNfgNashAlgorithm *panelNfgEnumMixed::GetAlgorithm(void) const
{
  if (m_precision->GetSelection() == 0) {
    gbtNfgNashEnumMixed<double> *algorithm = new gbtNfgNashEnumMixed<double>;
    algorithm->SetStopAfter((m_findAll->GetValue()) ?
			    0 : m_stopAfter->GetValue());
    return algorithm;
  }
  else {
    gbtNfgNashEnumMixed<gRational> *algorithm = new gbtNfgNashEnumMixed<gRational>;
    algorithm->SetStopAfter((m_findAll->GetValue()) ?
			    0 : m_stopAfter->GetValue());
    return algorithm;
  }
}

//========================================================================
//                         class panelNfgLcp
//========================================================================

const int idCHECKBOX_LIMITDEPTH = 2002;

class panelNfgLcp : public panelNfgNashAlgorithm {
private:
  wxRadioBox *m_precision;
  wxCheckBox *m_findAll, *m_limitDepth;
  wxSpinCtrl *m_stopAfter, *m_maxDepth;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);
  void OnStopAfter(wxSpinEvent &);
  void OnLimitDepth(wxCommandEvent &);

public:
  panelNfgLcp(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelNfgLcp, panelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelNfgLcp::OnFindAll)
  EVT_SPINCTRL(idSPINCTRL_STOPAFTER, panelNfgLcp::OnStopAfter)
  EVT_CHECKBOX(idCHECKBOX_LIMITDEPTH, panelNfgLcp::OnLimitDepth)
END_EVENT_TABLE()

panelNfgLcp::panelNfgLcp(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, "LcpSolve");
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Find Nash equilibria via linear "
				    "complementarity program"),
		   0, wxALL | wxCENTER, 5);

  wxString precisionChoices[] = { "Floating point", "Rational" };
  m_precision = new wxRadioBox(this, -1, "Precision",
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 1, wxRA_SPECIFY_ROWS);
  centerSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);

  wxStaticBox *stopAfterBox = new wxStaticBox(this, wxID_STATIC,
					      "Number to find");
  wxStaticBoxSizer *stopAfterSizer = new wxStaticBoxSizer(stopAfterBox,
							  wxHORIZONTAL);
  m_findAll = new wxCheckBox(this, idCHECKBOX_FINDALL, "Find all");
  m_findAll->SetValue(false);
  stopAfterSizer->Add(m_findAll, 0, wxALL | wxCENTER, 5);
  stopAfterSizer->Add(new wxStaticText(this, wxID_STATIC, "Stop after"),
		      0, wxALL | wxCENTER, 5);
  m_stopAfter = new wxSpinCtrl(this, idSPINCTRL_STOPAFTER, "1",
			       wxDefaultPosition, wxDefaultSize,
			       wxSP_ARROW_KEYS, 1, 10000);
  stopAfterSizer->Add(m_stopAfter, 0, wxALL | wxCENTER, 5);
  centerSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 5);

  wxStaticBox *depthBox = new wxStaticBox(this, wxID_STATIC, 
					  "Algorithm behavior");
  wxStaticBoxSizer *depthSizer = new wxStaticBoxSizer(depthBox,
						      wxHORIZONTAL);
  m_limitDepth = new wxCheckBox(this, idCHECKBOX_LIMITDEPTH, "Limit depth");
  m_limitDepth->SetValue(false);
  m_limitDepth->Enable(false);
  depthSizer->Add(m_limitDepth, 0, wxALL | wxCENTER, 5);
  depthSizer->Add(new wxStaticText(this, wxID_STATIC, "Maximum depth"),
		  0, wxALL | wxCENTER, 5);
  m_maxDepth = new wxSpinCtrl(this, -1, "10",
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

void panelNfgLcp::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
  m_limitDepth->Enable(m_findAll->GetValue() || 
		       m_stopAfter->GetValue() > 1);
  m_maxDepth->Enable((m_findAll->GetValue() || m_stopAfter->GetValue() > 1) &&
		     m_limitDepth->GetValue());
}

void panelNfgLcp::OnStopAfter(wxSpinEvent &)
{
  m_limitDepth->Enable(m_stopAfter->GetValue() > 1);
  m_maxDepth->Enable(m_stopAfter->GetValue() > 1 && m_limitDepth->GetValue());
}

void panelNfgLcp::OnLimitDepth(wxCommandEvent &)
{
  m_maxDepth->Enable(m_limitDepth->GetValue());
}

gbtNfgNashAlgorithm *panelNfgLcp::GetAlgorithm(void) const
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
    gbtNfgNashLcp<gRational> *algorithm = new gbtNfgNashLcp<gRational>;
    algorithm->SetStopAfter((m_findAll->GetValue()) ?
			    0 : m_stopAfter->GetValue());
    algorithm->SetMaxDepth((m_limitDepth->GetValue()) ?
			   m_maxDepth->GetValue() : 0);
    return algorithm;
  }
}

//========================================================================
//                         class panelNfgLp
//========================================================================

class panelNfgLp : public panelNfgNashAlgorithm {
private:
  wxRadioBox *m_precision;
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  panelNfgLp(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelNfgLp, panelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelNfgLp::OnFindAll)
END_EVENT_TABLE()

panelNfgLp::panelNfgLp(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, "LpSolve");
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Find Nash equilibria via linear "
				    "program"),
		   0, wxALL | wxCENTER, 5);

  wxString precisionChoices[] = { "Floating point", "Rational" };
  m_precision = new wxRadioBox(this, -1, "Precision",
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 1, wxRA_SPECIFY_ROWS);
  centerSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);

  // The "find all" feature of LpSolve currently does not work;
  // therefore, the controls are disabled in this version
  wxStaticBox *stopAfterBox = new wxStaticBox(this, wxID_STATIC,
					      "Number to find");
  wxStaticBoxSizer *stopAfterSizer = new wxStaticBoxSizer(stopAfterBox,
							  wxHORIZONTAL);
  m_findAll = new wxCheckBox(this, idCHECKBOX_FINDALL, "Find all");
  m_findAll->SetValue(false);
  m_findAll->Enable(false);
  stopAfterSizer->Add(m_findAll, 0, wxALL | wxCENTER, 5);
  stopAfterSizer->Add(new wxStaticText(this, wxID_STATIC, "Stop after"),
		      0, wxALL | wxCENTER, 5);
  m_stopAfter = new wxSpinCtrl(this, -1, "1",
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

void panelNfgLp::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtNfgNashAlgorithm *panelNfgLp::GetAlgorithm(void) const
{
  if (m_precision->GetSelection() == 0) {
    gbtNfgNashLp<double> *algorithm = new gbtNfgNashLp<double>;
    return algorithm;
  }
  else {
    gbtNfgNashLp<gRational> *algorithm = new gbtNfgNashLp<gRational>;
    return algorithm;
  }
}

//========================================================================
//                        class panelNfgLiap
//========================================================================

class panelNfgLiap : public panelNfgNashAlgorithm {
private:
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter, *m_numTries;
  wxSpinCtrl *m_maxits;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  panelNfgLiap(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelNfgLiap, panelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelNfgLiap::OnFindAll)
END_EVENT_TABLE()

panelNfgLiap::panelNfgLiap(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, "LiapSolve");
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Find Nash equilibria using "
				    "Lyapunov function minimization"),
		   0, wxALL | wxCENTER, 5);

  wxStaticBox *stopAfterBox = new wxStaticBox(this, wxID_STATIC,
					      "Number to find");
  wxStaticBoxSizer *stopAfterSizer = new wxStaticBoxSizer(stopAfterBox,
							  wxHORIZONTAL);
  m_findAll = new wxCheckBox(this, idCHECKBOX_FINDALL, "No limit");
  m_findAll->SetValue(false);
  stopAfterSizer->Add(m_findAll, 0, wxALL | wxCENTER, 5);
  stopAfterSizer->Add(new wxStaticText(this, wxID_STATIC, "Stop after"),
		      0, wxALL | wxCENTER, 5);
  m_stopAfter = new wxSpinCtrl(this, -1, "1",
			       wxDefaultPosition, wxDefaultSize,
			       wxSP_ARROW_KEYS, 1, 10000);
  stopAfterSizer->Add(m_stopAfter, 0, wxALL | wxCENTER, 5);
  centerSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 5);

  wxStaticBox *algorithmBox = new wxStaticBox(this, wxID_STATIC,
					      "Algorithm behavior");
  wxStaticBoxSizer *algorithmSizer = new wxStaticBoxSizer(algorithmBox,
							  wxHORIZONTAL);

  wxFlexGridSizer *paramSizer = new wxFlexGridSizer(2);
  paramSizer->Add(new wxStaticText(this, wxID_STATIC, "Number of restarts"),
		  0, wxALL | wxCENTER, 5);
  m_numTries = new wxSpinCtrl(this, -1, "100",
			      wxDefaultPosition, wxDefaultSize,
			      wxSP_ARROW_KEYS, 1, 10000);
  paramSizer->Add(m_numTries, 0, wxALL, 5);

  paramSizer->Add(new wxStaticText(this, wxID_STATIC,
				   "Maximum iterations in minimization"),
		  0, wxALL | wxCENTER, 5);
  m_maxits = new wxSpinCtrl(this, -1, "500",
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

void panelNfgLiap::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtNfgNashAlgorithm *panelNfgLiap::GetAlgorithm(void) const
{
  gbtNfgNashLiap *algorithm = new gbtNfgNashLiap;
  algorithm->SetStopAfter((m_findAll->GetValue()) ?
			  0 : m_stopAfter->GetValue());
  algorithm->SetNumTries(m_numTries->GetValue());
  algorithm->SetMaxitsN(m_maxits->GetValue());
  return algorithm;
}

//========================================================================
//                      class panelNfgPolEnum
//========================================================================

class panelNfgPolEnum : public panelNfgNashAlgorithm {
private:
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  panelNfgPolEnum(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelNfgPolEnum, panelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelNfgPolEnum::OnFindAll)
END_EVENT_TABLE()

panelNfgPolEnum::panelNfgPolEnum(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, "PolEnumSolve");
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Find Nash equilibria via solving "
				    "polynomial equations"),
		   0, wxALL | wxCENTER, 5);

  wxStaticBox *stopAfterBox = new wxStaticBox(this, wxID_STATIC,
					      "Number to find");
  wxStaticBoxSizer *stopAfterSizer = new wxStaticBoxSizer(stopAfterBox,
							  wxHORIZONTAL);
  m_findAll = new wxCheckBox(this, idCHECKBOX_FINDALL, "Find all");
  m_findAll->SetValue(false);
  stopAfterSizer->Add(m_findAll, 0, wxALL | wxCENTER, 5);
  stopAfterSizer->Add(new wxStaticText(this, wxID_STATIC, "Stop after"),
		      0, wxALL | wxCENTER, 5);
  m_stopAfter = new wxSpinCtrl(this, -1, "1",
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

void panelNfgPolEnum::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtNfgNashAlgorithm *panelNfgPolEnum::GetAlgorithm(void) const
{
  gbtNfgNashEnumPoly *algorithm = new gbtNfgNashEnumPoly;
  algorithm->SetStopAfter((m_findAll->GetValue()) ?
			  0 : m_stopAfter->GetValue());
  return algorithm;
}

//========================================================================
//                         class panelNfgQre
//========================================================================

class panelNfgQre : public panelNfgNashAlgorithm {
public:
  panelNfgQre(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgQre::panelNfgQre(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, "QreSolve");
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Find Nash equilibria via tracing "
				    "logit equilibria"),
		   0, wxALL | wxCENTER, 5);

  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "This algorithm requires no parameters"),
		   0, wxALL | wxCENTER, 5);
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 0);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *panelNfgQre::GetAlgorithm(void) const
{
  gbtNfgNashLogit *algorithm = new gbtNfgNashLogit;
  algorithm->SetFullGraph(false);
  algorithm->SetMaxLambda(1000000000);
  return algorithm;
}

//========================================================================
//                       class panelNfgSimpdiv
//========================================================================

const int idCHECKBOX_USELEASH = 2002;

class panelNfgSimpdiv : public panelNfgNashAlgorithm {
private:
  wxRadioBox *m_precision;
  wxCheckBox *m_useLeash;
  wxSpinCtrl *m_leashLength, *m_numRestarts;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);
  void OnUseLeash(wxCommandEvent &);

public:
  panelNfgSimpdiv(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelNfgSimpdiv, panelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_USELEASH, panelNfgSimpdiv::OnUseLeash)
END_EVENT_TABLE()

panelNfgSimpdiv::panelNfgSimpdiv(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, "SimpdivSolve");
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    "Find Nash equilibria via simplicial "
				    "subdivision"),
		   0, wxALL | wxCENTER, 5);

  wxString precisionChoices[] = { "Floating point", "Rational" };
  m_precision = new wxRadioBox(this, -1, "Precision",
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 1, wxRA_SPECIFY_ROWS);
  centerSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);

  wxStaticBox *algorithmBox = new wxStaticBox(this, wxID_STATIC,
					      "Algorithm behavior");
  wxStaticBoxSizer *algorithmSizer = new wxStaticBoxSizer(algorithmBox,
							  wxVERTICAL);
  
  wxBoxSizer *leashSizer = new wxBoxSizer(wxHORIZONTAL);
  m_useLeash = new wxCheckBox(this, idCHECKBOX_USELEASH, "Use leash");
  m_useLeash->SetValue(false);
  leashSizer->Add(m_useLeash, 0, wxALL | wxCENTER, 5);
  leashSizer->Add(new wxStaticText(this, wxID_STATIC, "Leash length"),
		  0, wxALL | wxCENTER, 5);
  m_leashLength = new wxSpinCtrl(this, -1, "100",
				 wxDefaultPosition, wxDefaultSize,
				 wxSP_ARROW_KEYS, 1, 10000);
  m_leashLength->Enable(false);
  leashSizer->Add(m_leashLength, 0, wxALL | wxCENTER, 5);
  algorithmSizer->Add(leashSizer, 0, wxALL | wxCENTER, 5);

  wxBoxSizer *restartSizer = new wxBoxSizer(wxHORIZONTAL);
  restartSizer->Add(new wxStaticText(this, wxID_STATIC, "Number of restarts"),
		    0, wxALL | wxCENTER, 5);
  m_numRestarts = new wxSpinCtrl(this, -1, "20",
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

void panelNfgSimpdiv::OnUseLeash(wxCommandEvent &)
{
  m_leashLength->Enable(m_useLeash->GetValue());
}

gbtNfgNashAlgorithm *panelNfgSimpdiv::GetAlgorithm(void) const
{
  if (m_precision->GetSelection() == 0) {
    gbtNfgNashSimpdiv<double> *algorithm = new gbtNfgNashSimpdiv<double>;
    algorithm->SetLeashLength((m_useLeash->GetValue()) ?
			      m_leashLength->GetValue() : 0);
    algorithm->SetNumRestarts(m_numRestarts->GetValue());
    return algorithm;
  }
  else {
    gbtNfgNashSimpdiv<gRational> *algorithm = new gbtNfgNashSimpdiv<gRational>;
    algorithm->SetLeashLength((m_useLeash->GetValue()) ?
			      m_leashLength->GetValue() : 0);
    algorithm->SetNumRestarts(m_numRestarts->GetValue());
    return algorithm;
  }
}

//========================================================================
//                      class panelNfgYamamoto
//========================================================================

class panelNfgYamamoto : public panelNfgNashAlgorithm {
public:
  panelNfgYamamoto(wxWindow *);

  gbtNfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgYamamoto::panelNfgYamamoto(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 "This algorithm requires no parameters"),
		0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

gbtNfgNashAlgorithm *panelNfgYamamoto::GetAlgorithm(void) const
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

dialogNfgNash::dialogNfgNash(wxWindow *p_parent, const gbtNfgSupport &p_support)
  : wxDialog(p_parent, -1, "Compute Nash equilibria"),
    m_algorithms(0)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  m_algPanelSizer = new wxBoxSizer(wxHORIZONTAL);
  m_algorithmTree = new wxTreeCtrl(this, idTREECTRL_ALGORITHMS,
				   wxDefaultPosition, wxSize(200, 400),
				   wxTR_NO_BUTTONS | wxTR_HIDE_ROOT |
				   wxTR_NO_LINES | wxTR_ROW_LINES);
  wxTreeItemId init = LoadAlgorithms(p_support.GetGame());
  m_algPanelSizer->Add(m_algorithmTree, 1, wxALL, 5);
  m_currentPanel = m_algorithms(init);
  m_algPanelSizer->Add(m_currentPanel, 0, wxALL | wxCENTER, 5);
  topSizer->Add(m_algPanelSizer, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  //  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);

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
  wxTreeItemId root = m_algorithmTree->AddRoot("Algorithms");
  wxTreeItemId standard = m_algorithmTree->AppendItem(root,
						      "Standard algorithms");
  m_algorithmTree->SetItemBold(standard);
  // This is added to silence some BC warnings
  panelNfgNashAlgorithm *panel;
  id = m_algorithmTree->AppendItem(standard, "One Nash equilibrium");
  m_algorithms.Define(id, panel = new panelNfgOneNash(this));
  wxTreeItemId init = id;
  
  id = m_algorithmTree->AppendItem(standard, "Two Nash equilibria");
  m_algorithms.Define(id, panel = new panelNfgTwoNash(this));

  id = m_algorithmTree->AppendItem(standard, "All Nash equilibria");
  m_algorithms.Define(id, panel = new panelNfgAllNash(this));

  if (p_nfg.NumPlayers() == 2) {
    id = m_algorithmTree->AppendItem(standard, "One perfect equilibrium");
    m_algorithms.Define(id, panel = new panelNfgOnePerfect(this));

    id = m_algorithmTree->AppendItem(standard, "Two perfect equilibria");
    m_algorithms.Define(id, panel = new panelNfgTwoPerfect(this));

    id = m_algorithmTree->AppendItem(standard, "All perfect equilibria");
    m_algorithms.Define(id, panel = new panelNfgAllPerfect(this));
  }

  wxTreeItemId custom = m_algorithmTree->AppendItem(root, "Custom algorithms");
  m_algorithmTree->SetItemBold(custom);

  id = m_algorithmTree->AppendItem(custom, "EnumPureSolve");
  m_algorithms.Define(id, panel = new panelNfgEnumPure(this));

  if (p_nfg.NumPlayers() == 2) {
    id = m_algorithmTree->AppendItem(custom, "EnumMixedSolve");
    m_algorithms.Define(id, panel = new panelNfgEnumMixed(this));

    id = m_algorithmTree->AppendItem(custom, "LcpSolve");
    m_algorithms.Define(id, panel = new panelNfgLcp(this));

    if (IsConstSum(p_nfg)) {
      id = m_algorithmTree->AppendItem(custom, "LpSolve");
      m_algorithms.Define(id, panel = new panelNfgLp(this));
    }
  }

  id = m_algorithmTree->AppendItem(custom, "LiapSolve");
  m_algorithms.Define(id, panel = new panelNfgLiap(this));

  id = m_algorithmTree->AppendItem(custom, "PolEnumSolve");
  m_algorithms.Define(id, panel = new panelNfgPolEnum(this));
  
  id = m_algorithmTree->AppendItem(custom, "QreSolve");
  m_algorithms.Define(id, panel = new panelNfgQre(this));

  id = m_algorithmTree->AppendItem(custom, "SimpdivSolve");
  m_algorithms.Define(id, panel = new panelNfgSimpdiv(this));

  id = m_algorithmTree->AppendItem(custom, "YamamotoSolve");
  m_algorithms.Define(id, panel = new panelNfgYamamoto(this));

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

static gOutput &operator<<(gOutput &p_stream, wxTreeItemId)
{ return p_stream; }

#include "base/gmap.imp"
template class gBaseMap<wxTreeItemId, panelNfgNashAlgorithm *>;
template class gOrdMap<wxTreeItemId, panelNfgNashAlgorithm *>;

