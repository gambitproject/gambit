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
#include "dlefgnash.h"

#include "base/gnullstatus.h"

#include "nash/efgsubsolve.h"

#include "nash/efgpure.h"
#include "nash/nfgpure.h"
#include "nash/nfgmixed.h"
#include "nash/efglcp.h"
#include "nash/nfglcp.h"
#include "nash/efglp.h"
#include "nash/nfglp.h"
#include "nash/efgliap.h"
#include "nash/nfgliap.h"
#include "nash/efgpoly.h"
#include "nash/nfgpoly.h"
#include "nash/efglogit.h"
#include "nash/nfgsimpdiv.h"

const int idCHECKBOX_FINDALL = 2000;
const int idSPINCTRL_STOPAFTER = 2001;

class panelEfgNashAlgorithm : public wxPanel {
public:
  panelEfgNashAlgorithm(wxWindow *p_parent) : wxPanel(p_parent, -1) { }

  virtual gbtEfgNashAlgorithm *GetAlgorithm(void) const = 0;
};

//========================================================================
//                         class efgOneNash
//========================================================================

class efgOneNash : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "OneNash"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> efgOneNash::Solve(const gbtEfgSupport &p_support,
				       gbtStatus &p_status)
{
  gbtArray<int> players(p_support.GetGame().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gbtNullStatus status;
    gbtNullOutput gnull;
    /* one round of elimination of weakly dominated strategies */
    gbtEfgSupport support = p_support.Undominated(false, true,
					      players, gnull, status);
    
    gbtEfgNashSubgames algorithm;
    p_support.GetGame().MarkSubgames();

    if (p_support.GetGame().NumPlayers() == 2) {
      if (p_support.GetGame().IsConstSum()) {
	algorithm.SetAlgorithm(new gbtEfgNashLp<double>);
      }
      else {
	gbtEfgNashLcp<double> *subAlgorithm = new gbtEfgNashLcp<double>;
	subAlgorithm->SetStopAfter(1);
	algorithm.SetAlgorithm(subAlgorithm);
      }
    }
    else {
      algorithm.SetAlgorithm(new gbtNfgNashSimpdiv<double>);
    }

    return algorithm.Solve(p_support, p_status);
  }
  catch (...) {
    return gbtList<BehavSolution>();
  }
}

//========================================================================
//                       class panelEfgOneNash
//========================================================================

class panelEfgOneNash : public panelEfgNashAlgorithm {
public:
  panelEfgOneNash(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

panelEfgOneNash::panelEfgOneNash(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, 
					   _("OneNashSolve"));
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

gbtEfgNashAlgorithm *panelEfgOneNash::GetAlgorithm(void) const
{
  return new efgOneNash;
}

//========================================================================
//                         class efgTwoNash
//========================================================================

class efgTwoNash : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "TwoNash"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> efgTwoNash::Solve(const gbtEfgSupport &p_support,
				       gbtStatus &p_status)
{
  gbtArray<int> players(p_support.GetGame().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gbtEfgSupport support(p_support);

    while (true) {
      gbtNullStatus status;
      gbtNullOutput gnull;
      gbtEfgSupport newSupport = support.Undominated(true, true, players,
						 gnull, status);
      
      if (newSupport == support) {
	break;
      }
      else {
	support = newSupport;
      }
    }

    gbtEfgNashSubgames algorithm;
    p_support.GetGame().UnmarkSubgames(p_support.GetGame().GetRoot());

    if (p_support.GetGame().NumPlayers() == 2) {
      gbtNfgNashEnumMixed<double> *subAlgorithm = new gbtNfgNashEnumMixed<double>;
      subAlgorithm->SetStopAfter(2);
      algorithm.SetAlgorithm(subAlgorithm);
    }
    else {
      gbtEfgNashEnumPoly *subAlgorithm = new gbtEfgNashEnumPoly;
      subAlgorithm->SetStopAfter(2);
      algorithm.SetAlgorithm(subAlgorithm);
    }

    return algorithm.Solve(p_support, p_status);
  }
  catch (...) {
    return gbtList<BehavSolution>();
  }
}

//========================================================================
//                       class panelEfgTwoNash
//========================================================================

class panelEfgTwoNash : public panelEfgNashAlgorithm {
public:
  panelEfgTwoNash(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

panelEfgTwoNash::panelEfgTwoNash(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, 
					   _("TwoNashSolve"));
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

gbtEfgNashAlgorithm *panelEfgTwoNash::GetAlgorithm(void) const
{
  return new efgTwoNash;
}

//========================================================================
//                         class efgAllNash
//========================================================================

class efgAllNash : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "AllNash"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> efgAllNash::Solve(const gbtEfgSupport &p_support,
				       gbtStatus &p_status)
{
  gbtArray<int> players(p_support.GetGame().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gbtEfgSupport support(p_support);

    while (true) {
      gbtNullStatus status;
      gbtNullOutput gnull;
      gbtEfgSupport newSupport = support.Undominated(true, true, players,
						 gnull, status);
      
      if (newSupport == support) {
	break;
      }
      else {
	support = newSupport;
      }
    }

    gbtEfgNashSubgames algorithm;
    p_support.GetGame().UnmarkSubgames(p_support.GetGame().GetRoot());

    if (p_support.GetGame().NumPlayers() == 2) {
      gbtNfgNashEnumMixed<double> *subAlgorithm = new gbtNfgNashEnumMixed<double>;
      subAlgorithm->SetStopAfter(0);
      algorithm.SetAlgorithm(subAlgorithm);
    }
    else {
      gbtEfgNashEnumPoly *subAlgorithm = new gbtEfgNashEnumPoly;
      subAlgorithm->SetStopAfter(0);
      algorithm.SetAlgorithm(subAlgorithm);
    }

    return algorithm.Solve(p_support, p_status);
  }
  catch (...) {
    return gbtList<BehavSolution>();
  }
}

//========================================================================
//                       class panelEfgAllNash
//========================================================================

class panelEfgAllNash : public panelEfgNashAlgorithm {
public:
  panelEfgAllNash(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

panelEfgAllNash::panelEfgAllNash(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   _("AllNashSolve"));
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

gbtEfgNashAlgorithm *panelEfgAllNash::GetAlgorithm(void) const
{
  return new efgAllNash;
}

//========================================================================
//                         class efgOnePerfect
//========================================================================

class efgOnePerfect : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "OnePerfect"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> efgOnePerfect::Solve(const gbtEfgSupport &p_support,
					  gbtStatus &p_status)
{
  gbtArray<int> players(p_support.GetGame().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gbtNullStatus status;
    gbtNullOutput gnull;
    /* one round of elimination of weakly dominated strategies */
    gbtEfgSupport support = p_support.Undominated(false, true,
					      players, gnull, status);
    
    gbtEfgNashSubgames algorithm;
    p_support.GetGame().MarkSubgames();

    if (p_support.GetGame().NumPlayers() == 2) {
      if (p_support.GetGame().IsConstSum()) {
	algorithm.SetAlgorithm(new gbtEfgNashLp<double>);
      }
      else {
	gbtEfgNashLcp<double> *subAlgorithm = new gbtEfgNashLcp<double>;
	subAlgorithm->SetStopAfter(1);
	algorithm.SetAlgorithm(subAlgorithm);
      }
    }
    else {
      algorithm.SetAlgorithm(new gbtNfgNashSimpdiv<double>);
    }

    return algorithm.Solve(p_support, p_status);
  }
  catch (...) {
    return gbtList<BehavSolution>();
  }
}

//========================================================================
//                       class panelEfgOnePerfect
//========================================================================

class panelEfgOnePerfect : public panelEfgNashAlgorithm {
public:
  panelEfgOnePerfect(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

panelEfgOnePerfect::panelEfgOnePerfect(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   _("OnePerfectSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find one subgame perfect Nash equilibrium")),
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

gbtEfgNashAlgorithm *panelEfgOnePerfect::GetAlgorithm(void) const
{
  return new efgOnePerfect;
}

//========================================================================
//                         class efgTwoPerfect
//========================================================================

class efgTwoPerfect : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "TwoPerfect"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> efgTwoPerfect::Solve(const gbtEfgSupport &p_support,
					  gbtStatus &p_status)
{
  gbtArray<int> players(p_support.GetGame().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gbtEfgSupport support(p_support);

    while (true) {
      gbtNullStatus status;
      gbtNullOutput gnull;
      gbtEfgSupport newSupport = support.Undominated(true, true, players,
						 gnull, status);
      
      if (newSupport == support) {
	break;
      }
      else {
	support = newSupport;
      }
    }

    gbtEfgNashSubgames algorithm;
    p_support.GetGame().MarkSubgames();

    if (p_support.GetGame().NumPlayers() == 2) {
      gbtNfgNashEnumMixed<double> *subAlgorithm = new gbtNfgNashEnumMixed<double>;
      subAlgorithm->SetStopAfter(2);
      algorithm.SetAlgorithm(subAlgorithm);
    }
    else {
      gbtEfgNashEnumPoly *subAlgorithm = new gbtEfgNashEnumPoly;
      subAlgorithm->SetStopAfter(2);
      algorithm.SetAlgorithm(subAlgorithm);
    }

    return algorithm.Solve(p_support, p_status);
  }
  catch (...) {
    return gbtList<BehavSolution>();
  }
}

//========================================================================
//                       class panelEfgTwoPerfect
//========================================================================

class panelEfgTwoPerfect : public panelEfgNashAlgorithm {
public:
  panelEfgTwoPerfect(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

panelEfgTwoPerfect::panelEfgTwoPerfect(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   _("TwoPerfectSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find two subgame perfect Nash equilibria")),
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

gbtEfgNashAlgorithm *panelEfgTwoPerfect::GetAlgorithm(void) const
{
  return new efgTwoPerfect;
}

//========================================================================
//                         class efgAllPerfect
//========================================================================

class efgAllPerfect : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "AllPerfect"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> efgAllPerfect::Solve(const gbtEfgSupport &p_support,
					  gbtStatus &p_status)
{
  gbtArray<int> players(p_support.GetGame().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gbtEfgSupport support(p_support);

    while (true) {
      gbtNullStatus status;
      gbtNullOutput gnull;
      gbtEfgSupport newSupport = support.Undominated(true, true, players,
						 gnull, status);
      
      if (newSupport == support) {
	break;
      }
      else {
	support = newSupport;
      }
    }

    gbtEfgNashSubgames algorithm;
    p_support.GetGame().MarkSubgames();

    if (p_support.GetGame().NumPlayers() == 2) {
      gbtNfgNashEnumMixed<double> *subAlgorithm = new gbtNfgNashEnumMixed<double>;
      subAlgorithm->SetStopAfter(0);
      algorithm.SetAlgorithm(subAlgorithm);
    }
    else {
      gbtEfgNashEnumPoly *subAlgorithm = new gbtEfgNashEnumPoly;
      subAlgorithm->SetStopAfter(0);
      algorithm.SetAlgorithm(subAlgorithm);
    }

    return algorithm.Solve(p_support, p_status);
  }
  catch (...) {
    return gbtList<BehavSolution>();
  }
}

//========================================================================
//                       class panelEfgAllPerfect
//========================================================================

class panelEfgAllPerfect : public panelEfgNashAlgorithm {
public:
  panelEfgAllPerfect(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

panelEfgAllPerfect::panelEfgAllPerfect(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   _("AllPerfectSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find all subgame perfect Nash equilibria")),
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

gbtEfgNashAlgorithm *panelEfgAllPerfect::GetAlgorithm(void) const
{
  return new efgAllPerfect;
}

//========================================================================
//                       class efgOneSequential
//========================================================================

class efgOneSequential : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "OneSequential"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> efgOneSequential::Solve(const gbtEfgSupport &p_support,
					     gbtStatus &p_status)
{
  try {
    gbtEfgNashLogit algorithm;
    return algorithm.Solve(p_support, p_status);
  }
  catch (...) {
    return gbtList<BehavSolution>();
  }
}

//========================================================================
//                     class panelEfgOneSequential
//========================================================================

class panelEfgOneSequential : public panelEfgNashAlgorithm {
public:
  panelEfgOneSequential(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

panelEfgOneSequential::panelEfgOneSequential(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   _("OneSequentialSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find one sequential Nash equilibrium")),
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

gbtEfgNashAlgorithm *panelEfgOneSequential::GetAlgorithm(void) const
{
  return new efgOneSequential;
}

//========================================================================
//                       class efgTwoSequential
//========================================================================

class efgTwoSequential : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "TwoSequential"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> efgTwoSequential::Solve(const gbtEfgSupport &p_support,
					     gbtStatus &p_status)
{
  try {
    p_support.GetGame().MarkSubgames();
    gbtEfgNashSubgames algorithm;
    gbtEfgNashLiap *subAlgorithm = new gbtEfgNashLiap;
    subAlgorithm->SetStopAfter(2);
    algorithm.SetAlgorithm(subAlgorithm);
    return algorithm.Solve(p_support, p_status);
  }
  catch (...) {
    return gbtList<BehavSolution>();
  }
}

//========================================================================
//                     class panelEfgTwoSequential
//========================================================================

class panelEfgTwoSequential : public panelEfgNashAlgorithm {
public:
  panelEfgTwoSequential(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

panelEfgTwoSequential::panelEfgTwoSequential(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   _("TwoSequentialSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find two sequential Nash equilibria")),
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

gbtEfgNashAlgorithm *panelEfgTwoSequential::GetAlgorithm(void) const
{
  return new efgTwoSequential;
}

//========================================================================
//                       class efgAllSequential
//========================================================================

class efgAllSequential : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "AllSequential"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> efgAllSequential::Solve(const gbtEfgSupport &p_support,
					     gbtStatus &p_status)
{
  try {
    p_support.GetGame().MarkSubgames();
    gbtEfgNashSubgames algorithm;
    gbtEfgNashLiap *subAlgorithm = new gbtEfgNashLiap;
    subAlgorithm->SetStopAfter(0);
    algorithm.SetAlgorithm(subAlgorithm);
    return algorithm.Solve(p_support, p_status);
  }
  catch (...) {
    return gbtList<BehavSolution>();
  }
}

//========================================================================
//                     class panelEfgAllSequential
//========================================================================

class panelEfgAllSequential : public panelEfgNashAlgorithm {
public:
  panelEfgAllSequential(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

panelEfgAllSequential::panelEfgAllSequential(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   _("AllSequentialSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find all sequential Nash equilibria")),
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

gbtEfgNashAlgorithm *panelEfgAllSequential::GetAlgorithm(void) const
{
  return new efgAllSequential;
}

//========================================================================
//                      class panelEfgEnumPure
//========================================================================

class panelEfgEnumPure : public panelEfgNashAlgorithm {
private:
  wxRadioBox *m_solveUsing;
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  panelEfgEnumPure(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelEfgEnumPure, panelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelEfgEnumPure::OnFindAll)
END_EVENT_TABLE()

panelEfgEnumPure::panelEfgEnumPure(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, 
					   wxT("EnumPureSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria by enumerating pure strategies")),
		   0, wxALL | wxCENTER, 5);

  wxString solveChoices[] = { _("Extensive form"), _("Normal form") };
  m_solveUsing = new wxRadioBox(this, -1, _("Find equilibria using"),
				wxDefaultPosition, wxDefaultSize,
				2, solveChoices, 1, wxRA_SPECIFY_ROWS);
  centerSizer->Add(m_solveUsing, 0, wxALL | wxCENTER, 5);

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

void panelEfgEnumPure::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtEfgNashAlgorithm *panelEfgEnumPure::GetAlgorithm(void) const
{
  gbtEfgNashSubgames *algorithm = new gbtEfgNashSubgames;
  
  if (m_solveUsing->GetSelection() == 0) {
    gbtEfgNashEnumPure *subAlgorithm = new gbtEfgNashEnumPure;
    subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
			       0 : m_stopAfter->GetValue());
    algorithm->SetAlgorithm(subAlgorithm);
  }
  else {
    gbtNfgNashEnumPure *subAlgorithm = new gbtNfgNashEnumPure;
    subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
			       0 : m_stopAfter->GetValue());
    algorithm->SetAlgorithm(subAlgorithm);
  }
  return algorithm;
}

//========================================================================
//                      class panelEfgEnumMixed
//========================================================================

class panelEfgEnumMixed : public panelEfgNashAlgorithm {
private:
  wxRadioBox *m_solveUsing, *m_precision;
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  panelEfgEnumMixed(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelEfgEnumMixed, panelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelEfgEnumMixed::OnFindAll)
END_EVENT_TABLE()

panelEfgEnumMixed::panelEfgEnumMixed(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC,
					   wxT("EnumMixedSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria by enumerating mixed strategies")),
		   0, wxALL | wxCENTER, 5);

  wxString solveChoices[] = { _("Extensive form"), _("Normal form") };
  m_solveUsing = new wxRadioBox(this, -1, _("Find equilibria using"),
				wxDefaultPosition, wxDefaultSize,
				2, solveChoices, 1, wxRA_SPECIFY_ROWS);
  m_solveUsing->SetSelection(1);
  m_solveUsing->Enable(false);
  centerSizer->Add(m_solveUsing, 0, wxALL | wxCENTER, 5);

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

void panelEfgEnumMixed::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtEfgNashAlgorithm *panelEfgEnumMixed::GetAlgorithm(void) const
{
  gbtEfgNashSubgames *algorithm = new gbtEfgNashSubgames;

  if (m_precision->GetSelection() == 0) {
    gbtNfgNashEnumMixed<double> *subAlgorithm = new gbtNfgNashEnumMixed<double>;
    subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
			       0 : m_stopAfter->GetValue());
    algorithm->SetAlgorithm(subAlgorithm);
  }
  else {
    gbtNfgNashEnumMixed<gbtRational> *subAlgorithm = new gbtNfgNashEnumMixed<gbtRational>;
    subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
			       0 : m_stopAfter->GetValue());
    algorithm->SetAlgorithm(subAlgorithm);
  }
  return algorithm;
}

//========================================================================
//                         class panelEfgLcp
//========================================================================

const int idCHECKBOX_LIMITDEPTH = 2002;

class panelEfgLcp : public panelEfgNashAlgorithm {
private:
  wxRadioBox *m_solveUsing, *m_precision;
  wxCheckBox *m_findAll, *m_limitDepth;
  wxSpinCtrl *m_stopAfter, *m_maxDepth;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);
  void OnStopAfter(wxSpinEvent &);
  void OnLimitDepth(wxCommandEvent &);

public:
  panelEfgLcp(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelEfgLcp, panelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelEfgLcp::OnFindAll)
  EVT_SPINCTRL(idSPINCTRL_STOPAFTER, panelEfgLcp::OnStopAfter)
  EVT_CHECKBOX(idCHECKBOX_LIMITDEPTH, panelEfgLcp::OnLimitDepth)
END_EVENT_TABLE()

panelEfgLcp::panelEfgLcp(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, wxT("LcpSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria via linear complementarity program")),
		   0, wxALL | wxCENTER, 5);

  wxString solveChoices[] = { _("Extensive form"), _("Normal form") };
  m_solveUsing = new wxRadioBox(this, -1, _("Find equilibria using"),
				wxDefaultPosition, wxDefaultSize,
				2, solveChoices, 1, wxRA_SPECIFY_ROWS);
  centerSizer->Add(m_solveUsing, 0, wxALL | wxCENTER, 5);

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

void panelEfgLcp::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
  m_limitDepth->Enable(m_findAll->GetValue() || 
		       m_stopAfter->GetValue() > 1);
  m_maxDepth->Enable((m_findAll->GetValue() || m_stopAfter->GetValue() > 1) &&
		     m_limitDepth->GetValue());
}

void panelEfgLcp::OnStopAfter(wxSpinEvent &)
{
  m_limitDepth->Enable(m_stopAfter->GetValue() > 1);
  m_maxDepth->Enable(m_stopAfter->GetValue() > 1 && m_limitDepth->GetValue());
}

void panelEfgLcp::OnLimitDepth(wxCommandEvent &)
{
  m_maxDepth->Enable(m_limitDepth->GetValue());
}

gbtEfgNashAlgorithm *panelEfgLcp::GetAlgorithm(void) const
{
  gbtEfgNashSubgames *algorithm = new gbtEfgNashSubgames;

  if (m_solveUsing->GetSelection() == 0) {
    if (m_precision->GetSelection() == 0) {
      gbtEfgNashLcp<double> *subAlgorithm = new gbtEfgNashLcp<double>;
      subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
				 0 : m_stopAfter->GetValue());
      subAlgorithm->SetMaxDepth((m_limitDepth->GetValue()) ?
				m_maxDepth->GetValue() : 0);
      algorithm->SetAlgorithm(subAlgorithm);
    }
    else {
      gbtEfgNashLcp<gbtRational> *subAlgorithm = new gbtEfgNashLcp<gbtRational>;
      subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
				 0 : m_stopAfter->GetValue());
      subAlgorithm->SetMaxDepth((m_limitDepth->GetValue()) ?
				m_maxDepth->GetValue() : 0);
      algorithm->SetAlgorithm(subAlgorithm);
    }
  }
  else {
    if (m_precision->GetSelection() == 0) {
      gbtNfgNashLcp<double> *subAlgorithm = new gbtNfgNashLcp<double>;
      subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
				 0 : m_stopAfter->GetValue());
      subAlgorithm->SetMaxDepth((m_limitDepth->GetValue()) ?
				m_maxDepth->GetValue() : 0);
      algorithm->SetAlgorithm(subAlgorithm);
    }
    else {
      gbtNfgNashLcp<gbtRational> *subAlgorithm = new gbtNfgNashLcp<gbtRational>;
      subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
				 0 : m_stopAfter->GetValue());
      subAlgorithm->SetMaxDepth((m_limitDepth->GetValue()) ?
				m_maxDepth->GetValue() : 0);
      algorithm->SetAlgorithm(subAlgorithm);
    }
  }

  return algorithm;
}

//========================================================================
//                         class panelEfgLp
//========================================================================

class panelEfgLp : public panelEfgNashAlgorithm {
private:
  wxRadioBox *m_solveUsing, *m_precision;
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  panelEfgLp(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelEfgLp, panelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelEfgLp::OnFindAll)
END_EVENT_TABLE()

panelEfgLp::panelEfgLp(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, wxT("LpSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria via linear program")),
		   0, wxALL | wxCENTER, 5);

  wxString solveChoices[] = { _("Extensive form"), _("Normal form") };
  m_solveUsing = new wxRadioBox(this, -1, _("Find equilibria using"),
				wxDefaultPosition, wxDefaultSize,
				2, solveChoices, 1, wxRA_SPECIFY_ROWS);
  centerSizer->Add(m_solveUsing, 0, wxALL | wxCENTER, 5);

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
  topSizer->Add(centerSizer, 1, wxALL | wxCENTER, 5);
  
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  Show(false);
}

void panelEfgLp::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtEfgNashAlgorithm *panelEfgLp::GetAlgorithm(void) const
{
  gbtEfgNashSubgames *algorithm = new gbtEfgNashSubgames;

  if (m_solveUsing->GetSelection() == 0) {
    if (m_precision->GetSelection() == 0) {
      algorithm->SetAlgorithm(new gbtEfgNashLp<double>);
    }
    else {
      algorithm->SetAlgorithm(new gbtEfgNashLp<gbtRational>);
    }
  }
  else {
    if (m_precision->GetSelection() == 0) {
      algorithm->SetAlgorithm(new gbtNfgNashLp<double>);
    }
    else {
      algorithm->SetAlgorithm(new gbtNfgNashLp<gbtRational>);
    }
  }
  return algorithm;
}

//========================================================================
//                        class panelEfgLiap
//========================================================================

class panelEfgLiap : public panelEfgNashAlgorithm {
private:
  wxRadioBox *m_solveUsing;
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter, *m_numTries;
  wxSpinCtrl *m_maxits;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  panelEfgLiap(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelEfgLiap, panelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelEfgLiap::OnFindAll)
END_EVENT_TABLE()

panelEfgLiap::panelEfgLiap(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, 
					   wxT("LiapSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria using Lyapunov function minimization")),
		   0, wxALL | wxCENTER, 5);

  wxString solveChoices[] = { _("Extensive form"), _("Normal form") };
  m_solveUsing = new wxRadioBox(this, -1, _("Find equilibria using"),
				wxDefaultPosition, wxDefaultSize,
				2, solveChoices, 1, wxRA_SPECIFY_ROWS);
  centerSizer->Add(m_solveUsing, 0, wxALL | wxCENTER, 5);

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
  paramSizer->Add(new wxStaticText(this, wxID_STATIC, _("Number of restarts")),
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

void panelEfgLiap::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtEfgNashAlgorithm *panelEfgLiap::GetAlgorithm(void) const
{
  gbtEfgNashSubgames *algorithm = new gbtEfgNashSubgames;

  if (m_solveUsing->GetSelection() == 0) {
    gbtEfgNashLiap *subAlgorithm = new gbtEfgNashLiap;
    subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
			       0 : m_stopAfter->GetValue());
    subAlgorithm->SetNumTries(m_numTries->GetValue());
    subAlgorithm->SetMaxitsN(m_maxits->GetValue());
    algorithm->SetAlgorithm(subAlgorithm);

  }
  else {
    gbtNfgNashLiap *subAlgorithm = new gbtNfgNashLiap;
    subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
			       0 : m_stopAfter->GetValue());
    subAlgorithm->SetNumTries(m_numTries->GetValue());
    subAlgorithm->SetMaxitsN(m_maxits->GetValue());
    algorithm->SetAlgorithm(subAlgorithm);
  }
  return algorithm;
}

//========================================================================
//                      class panelEfgPolEnum
//========================================================================

class panelEfgPolEnum : public panelEfgNashAlgorithm {
private:
  wxRadioBox *m_solveUsing;
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  panelEfgPolEnum(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelEfgPolEnum, panelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelEfgPolEnum::OnFindAll)
END_EVENT_TABLE()

panelEfgPolEnum::panelEfgPolEnum(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, 
					   wxT("PolEnumSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria via solving polynomial equations")),
		   0, wxALL | wxCENTER, 5);

  wxString solveChoices[] = { _("Extensive form"), _("Normal form") };
  m_solveUsing = new wxRadioBox(this, -1, _("Find equilibria using"),
				wxDefaultPosition, wxDefaultSize,
				2, solveChoices, 1, wxRA_SPECIFY_ROWS);
  centerSizer->Add(m_solveUsing, 0, wxALL | wxCENTER, 5);

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

void panelEfgPolEnum::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtEfgNashAlgorithm *panelEfgPolEnum::GetAlgorithm(void) const
{
  gbtEfgNashSubgames *algorithm = new gbtEfgNashSubgames;

  if (m_solveUsing->GetSelection() == 0) {
    gbtEfgNashEnumPoly *subAlgorithm = new gbtEfgNashEnumPoly;
    subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
			       0 : m_stopAfter->GetValue());
    algorithm->SetAlgorithm(subAlgorithm);
  }
  else {
    gbtNfgNashEnumPoly *subAlgorithm = new gbtNfgNashEnumPoly;
    subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
			       0 : m_stopAfter->GetValue());
    algorithm->SetAlgorithm(subAlgorithm);
  }
  return algorithm;
}

//========================================================================
//                         class panelEfgQre
//========================================================================

class panelEfgQre : public panelEfgNashAlgorithm {
public:
  panelEfgQre(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

panelEfgQre::panelEfgQre(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);
  
  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, wxT("QreSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria via tracing logit equilibria")),
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

gbtEfgNashAlgorithm *panelEfgQre::GetAlgorithm(void) const
{
  gbtEfgNashLogit *algorithm = new gbtEfgNashLogit;
  algorithm->SetFullGraph(false);
  algorithm->SetMaxLambda(1000000000);
  return algorithm;
}

//========================================================================
//                       class panelEfgSimpdiv
//========================================================================

const int idCHECKBOX_USELEASH = 2002;

class panelEfgSimpdiv : public panelEfgNashAlgorithm {
private:
  wxRadioBox *m_solveUsing, *m_precision;
  wxCheckBox *m_useLeash;
  wxSpinCtrl *m_leashLength, *m_numRestarts;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);
  void OnUseLeash(wxCommandEvent &);

public:
  panelEfgSimpdiv(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelEfgSimpdiv, panelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_USELEASH, panelEfgSimpdiv::OnUseLeash)
END_EVENT_TABLE()

panelEfgSimpdiv::panelEfgSimpdiv(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, 
					   wxT("SimpdivSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria via simplicial subdivision")),
		   0, wxALL | wxCENTER, 5);

  wxString solveChoices[] = { _("Extensive form"), _("Normal form") };
  m_solveUsing = new wxRadioBox(this, -1, _("Find equilibria using"),
				wxDefaultPosition, wxDefaultSize,
				2, solveChoices, 1, wxRA_SPECIFY_ROWS);
  m_solveUsing->SetSelection(1);
  m_solveUsing->Enable(false);
  centerSizer->Add(m_solveUsing, 0, wxALL | wxCENTER, 5);

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

void panelEfgSimpdiv::OnUseLeash(wxCommandEvent &)
{
  m_leashLength->Enable(m_useLeash->GetValue());
}

gbtEfgNashAlgorithm *panelEfgSimpdiv::GetAlgorithm(void) const
{
  gbtEfgNashSubgames *algorithm = new gbtEfgNashSubgames;

  if (m_precision->GetSelection() == 0) {
    gbtNfgNashSimpdiv<double> *subAlgorithm = new gbtNfgNashSimpdiv<double>;
    subAlgorithm->SetLeashLength((m_useLeash->GetValue()) ?
				 m_leashLength->GetValue() : 0);
    subAlgorithm->SetNumRestarts(m_numRestarts->GetValue());
    algorithm->SetAlgorithm(subAlgorithm);
  }
  else {
    gbtNfgNashSimpdiv<gbtRational> *subAlgorithm = new gbtNfgNashSimpdiv<gbtRational>;
    subAlgorithm->SetLeashLength((m_useLeash->GetValue()) ?
				 m_leashLength->GetValue() : 0);
    subAlgorithm->SetNumRestarts(m_numRestarts->GetValue());
    algorithm->SetAlgorithm(subAlgorithm);
  }
  return algorithm;
}

//========================================================================
//                        class dialogEfgNash
//========================================================================

const int idTREECTRL_ALGORITHMS = 2000;

BEGIN_EVENT_TABLE(dialogEfgNash, wxDialog)
  EVT_TREE_SEL_CHANGING(idTREECTRL_ALGORITHMS,
			dialogEfgNash::OnSelectionChanging)
  EVT_TREE_ITEM_COLLAPSING(idTREECTRL_ALGORITHMS,
			   dialogEfgNash::OnItemCollapsing)
END_EVENT_TABLE()

dialogEfgNash::dialogEfgNash(wxWindow *p_parent, const gbtEfgSupport &p_support)
  : wxDialog(p_parent, -1, _("Compute Nash equilibria"), wxDefaultPosition),
    m_currentPanel(0), m_algorithms(0) 
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  m_algPanelSizer = new wxBoxSizer(wxHORIZONTAL);
  m_algorithmTree = new wxTreeCtrl(this, idTREECTRL_ALGORITHMS,
				   wxDefaultPosition, wxSize(200, 400),
				   wxTR_NO_BUTTONS | wxTR_HIDE_ROOT |
				   wxTR_NO_LINES | wxTR_ROW_LINES);
  wxTreeItemId init = LoadAlgorithms(p_support.GetGame());
  m_algPanelSizer->Add(m_algorithmTree, 0, wxALL, 5);
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

int dialogEfgNash::LoadAlgorithms(const gbtEfgGame &p_efg)
{
  wxTreeItemId id;

  // Eventually, these should be loaded from wxConfig; for now,
  // I am going to hard-code them
  wxTreeItemId root = m_algorithmTree->AddRoot(_("Algorithms"));
  wxTreeItemId standard = m_algorithmTree->AppendItem(root,
						      _("Standard algorithms"));
  m_algorithmTree->SetItemBold(standard);
  // This is added to silence some BC warnings
  panelEfgNashAlgorithm *panel;
  id = m_algorithmTree->AppendItem(standard, _("One Nash equilibrium"));
  m_algorithms.Define(id, panel = new panelEfgOneNash(this));
  wxTreeItemId init = id;

  id = m_algorithmTree->AppendItem(standard, _("Two Nash equilibria"));
  m_algorithms.Define(id, panel = new panelEfgTwoNash(this));

  id = m_algorithmTree->AppendItem(standard, _("All Nash equilibria"));
  m_algorithms.Define(id, panel = new panelEfgAllNash(this));

  id = m_algorithmTree->AppendItem(standard, _("One perfect equilibrium"));
  m_algorithms.Define(id, panel = new panelEfgOnePerfect(this));

  id = m_algorithmTree->AppendItem(standard, _("Two perfect equilibria"));
  m_algorithms.Define(id, panel = new panelEfgTwoPerfect(this));

  id = m_algorithmTree->AppendItem(standard, _("All perfect equilibria"));
  m_algorithms.Define(id, panel = new panelEfgAllPerfect(this));

  id = m_algorithmTree->AppendItem(standard, _("One sequential equilibrium"));
  m_algorithms.Define(id, panel = new panelEfgOneSequential(this));

  id = m_algorithmTree->AppendItem(standard, _("Two sequential equilibria"));
  m_algorithms.Define(id, panel = new panelEfgTwoSequential(this));

  id = m_algorithmTree->AppendItem(standard, _("All sequential equilibria"));
  m_algorithms.Define(id, panel = new panelEfgAllSequential(this));

  wxTreeItemId custom = m_algorithmTree->AppendItem(root, 
						    _("Custom algorithms"));
  m_algorithmTree->SetItemBold(custom);

  id = m_algorithmTree->AppendItem(custom, wxT("EnumPureSolve"));
  m_algorithms.Define(id, panel = new panelEfgEnumPure(this));

  if (p_efg.NumPlayers() == 2) {
    id = m_algorithmTree->AppendItem(custom, wxT("EnumMixedSolve"));
    m_algorithms.Define(id, panel = new panelEfgEnumMixed(this));

    id = m_algorithmTree->AppendItem(custom, wxT("LcpSolve"));
    m_algorithms.Define(id, panel = new panelEfgLcp(this));

    if (p_efg.IsConstSum()) {
      id = m_algorithmTree->AppendItem(custom, wxT("LpSolve"));
      m_algorithms.Define(id, panel = new panelEfgLp(this));
    }
  }

  id = m_algorithmTree->AppendItem(custom, wxT("LiapSolve"));
  m_algorithms.Define(id, panel = new panelEfgLiap(this));

  id = m_algorithmTree->AppendItem(custom, wxT("PolEnumSolve"));
  m_algorithms.Define(id, panel = new panelEfgPolEnum(this));

  id = m_algorithmTree->AppendItem(custom, wxT("QreSolve"));
  m_algorithms.Define(id, panel = new panelEfgQre(this));

  id = m_algorithmTree->AppendItem(custom, wxT("SimpdivSolve"));
  m_algorithms.Define(id, panel = new panelEfgSimpdiv(this));

  m_algorithmTree->Expand(standard);
  m_algorithmTree->Expand(custom);
  m_algorithmTree->Expand(root);

  return init;
}

void dialogEfgNash::OnSelectionChanging(wxTreeEvent &p_event)
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

void dialogEfgNash::OnItemCollapsing(wxTreeEvent &p_event)
{
  p_event.Veto();
}

gbtEfgNashAlgorithm *dialogEfgNash::GetAlgorithm(void) const
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
template class gbtBaseMap<wxTreeItemId, panelEfgNashAlgorithm *>;
template class gbtOrdMap<wxTreeItemId, panelEfgNashAlgorithm *>;
