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

class gbtPanelEfgNashAlgorithm : public wxPanel {
public:
  gbtPanelEfgNashAlgorithm(wxWindow *p_parent) : wxPanel(p_parent, -1) { }

  virtual gbtEfgNashAlgorithm *GetAlgorithm(void) const = 0;
};

//========================================================================
//                         class gbtEfgNashOneNash
//========================================================================

class gbtEfgNashOneNash : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "OneNash"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> gbtEfgNashOneNash::Solve(const gbtEfgSupport &p_support,
				       gbtStatus &p_status)
{
  gbtArray<int> players(p_support.NumPlayers());
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
    p_support.GetTree()->MarkSubgames();

    if (p_support.NumPlayers() == 2) {
      if (p_support.IsConstSum()) {
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
//                       class gbtPanelEfgOneNash
//========================================================================

class gbtPanelEfgOneNash : public gbtPanelEfgNashAlgorithm {
public:
  gbtPanelEfgOneNash(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelEfgOneNash::gbtPanelEfgOneNash(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

gbtEfgNashAlgorithm *gbtPanelEfgOneNash::GetAlgorithm(void) const
{
  return new gbtEfgNashOneNash;
}

//========================================================================
//                         class gbtEfgNashTwoNash
//========================================================================

class gbtEfgNashTwoNash : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "TwoNash"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> gbtEfgNashTwoNash::Solve(const gbtEfgSupport &p_support,
				       gbtStatus &p_status)
{
  gbtArray<int> players(p_support.NumPlayers());
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
    p_support.GetTree()->UnmarkSubgames(p_support.GetRoot());

    if (p_support.NumPlayers() == 2) {
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
//                       class gbtPanelEfgTwoNash
//========================================================================

class gbtPanelEfgTwoNash : public gbtPanelEfgNashAlgorithm {
public:
  gbtPanelEfgTwoNash(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelEfgTwoNash::gbtPanelEfgTwoNash(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

gbtEfgNashAlgorithm *gbtPanelEfgTwoNash::GetAlgorithm(void) const
{
  return new gbtEfgNashTwoNash;
}

//========================================================================
//                         class gbtEfgNashAllNash
//========================================================================

class gbtEfgNashAllNash : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "AllNash"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> gbtEfgNashAllNash::Solve(const gbtEfgSupport &p_support,
				       gbtStatus &p_status)
{
  gbtArray<int> players(p_support.NumPlayers());
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
    p_support.GetTree()->UnmarkSubgames(p_support.GetRoot());

    if (p_support.NumPlayers() == 2) {
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
//                       class gbtPanelEfgAllNash
//========================================================================

class gbtPanelEfgAllNash : public gbtPanelEfgNashAlgorithm {
public:
  gbtPanelEfgAllNash(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelEfgAllNash::gbtPanelEfgAllNash(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

gbtEfgNashAlgorithm *gbtPanelEfgAllNash::GetAlgorithm(void) const
{
  return new gbtEfgNashAllNash;
}

//========================================================================
//                         class gbtEfgNashOnePerfect
//========================================================================

class gbtEfgNashOnePerfect : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "OnePerfect"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> gbtEfgNashOnePerfect::Solve(const gbtEfgSupport &p_support,
					  gbtStatus &p_status)
{
  gbtArray<int> players(p_support.NumPlayers());
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
    p_support.GetTree()->MarkSubgames();

    if (p_support.NumPlayers() == 2) {
      if (p_support.IsConstSum()) {
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
//                       class gbtPanelEfgOnePerfect
//========================================================================

class gbtPanelEfgOnePerfect : public gbtPanelEfgNashAlgorithm {
public:
  gbtPanelEfgOnePerfect(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelEfgOnePerfect::gbtPanelEfgOnePerfect(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

gbtEfgNashAlgorithm *gbtPanelEfgOnePerfect::GetAlgorithm(void) const
{
  return new gbtEfgNashOnePerfect;
}

//========================================================================
//                         class gbtEfgNashTwoPerfect
//========================================================================

class gbtEfgNashTwoPerfect : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "TwoPerfect"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> gbtEfgNashTwoPerfect::Solve(const gbtEfgSupport &p_support,
					  gbtStatus &p_status)
{
  gbtArray<int> players(p_support.NumPlayers());
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
    p_support.GetTree()->MarkSubgames();

    if (p_support.NumPlayers() == 2) {
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
//                       class gbtPanelEfgTwoPerfect
//========================================================================

class gbtPanelEfgTwoPerfect : public gbtPanelEfgNashAlgorithm {
public:
  gbtPanelEfgTwoPerfect(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelEfgTwoPerfect::gbtPanelEfgTwoPerfect(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

gbtEfgNashAlgorithm *gbtPanelEfgTwoPerfect::GetAlgorithm(void) const
{
  return new gbtEfgNashTwoPerfect;
}

//========================================================================
//                         class gbtEfgNashAllPerfect
//========================================================================

class gbtEfgNashAllPerfect : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "AllPerfect"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> gbtEfgNashAllPerfect::Solve(const gbtEfgSupport &p_support,
					  gbtStatus &p_status)
{
  gbtArray<int> players(p_support.NumPlayers());
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
    p_support.GetTree()->MarkSubgames();

    if (p_support.NumPlayers() == 2) {
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
//                       class gbtPanelEfgAllPerfect
//========================================================================

class gbtPanelEfgAllPerfect : public gbtPanelEfgNashAlgorithm {
public:
  gbtPanelEfgAllPerfect(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelEfgAllPerfect::gbtPanelEfgAllPerfect(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

gbtEfgNashAlgorithm *gbtPanelEfgAllPerfect::GetAlgorithm(void) const
{
  return new gbtEfgNashAllPerfect;
}

//========================================================================
//                       class gbtEfgNashOneSequential
//========================================================================

class gbtEfgNashOneSequential : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "OneSequential"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> gbtEfgNashOneSequential::Solve(const gbtEfgSupport &p_support,
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
//                     class gbtPanelEfgOneSequential
//========================================================================

class gbtPanelEfgOneSequential : public gbtPanelEfgNashAlgorithm {
public:
  gbtPanelEfgOneSequential(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelEfgOneSequential::gbtPanelEfgOneSequential(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

gbtEfgNashAlgorithm *gbtPanelEfgOneSequential::GetAlgorithm(void) const
{
  return new gbtEfgNashOneSequential;
}

//========================================================================
//                       class gbtEfgNashTwoSequential
//========================================================================

class gbtEfgNashTwoSequential : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "TwoSequential"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> gbtEfgNashTwoSequential::Solve(const gbtEfgSupport &p_support,
					     gbtStatus &p_status)
{
  try {
    p_support.GetTree()->MarkSubgames();
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
//                     class gbtPanelEfgTwoSequential
//========================================================================

class gbtPanelEfgTwoSequential : public gbtPanelEfgNashAlgorithm {
public:
  gbtPanelEfgTwoSequential(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelEfgTwoSequential::gbtPanelEfgTwoSequential(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

gbtEfgNashAlgorithm *gbtPanelEfgTwoSequential::GetAlgorithm(void) const
{
  return new gbtEfgNashTwoSequential;
}

//========================================================================
//                       class gbtEfgNashAllSequential
//========================================================================

class gbtEfgNashAllSequential : public gbtEfgNashAlgorithm {
public:
  gbtText GetAlgorithm(void) const { return "AllSequential"; }
  gbtList<BehavSolution> Solve(const gbtEfgSupport &, gbtStatus &);
};

gbtList<BehavSolution> gbtEfgNashAllSequential::Solve(const gbtEfgSupport &p_support,
					     gbtStatus &p_status)
{
  try {
    p_support.GetTree()->MarkSubgames();
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
//                     class gbtPanelEfgAllSequential
//========================================================================

class gbtPanelEfgAllSequential : public gbtPanelEfgNashAlgorithm {
public:
  gbtPanelEfgAllSequential(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelEfgAllSequential::gbtPanelEfgAllSequential(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

gbtEfgNashAlgorithm *gbtPanelEfgAllSequential::GetAlgorithm(void) const
{
  return new gbtEfgNashAllSequential;
}

//========================================================================
//                      class gbtPanelEfgEnumPure
//========================================================================

class gbtPanelEfgEnumPure : public gbtPanelEfgNashAlgorithm {
private:
  wxRadioBox *m_solveUsing;
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  gbtPanelEfgEnumPure(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtPanelEfgEnumPure, gbtPanelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, gbtPanelEfgEnumPure::OnFindAll)
END_EVENT_TABLE()

gbtPanelEfgEnumPure::gbtPanelEfgEnumPure(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

void gbtPanelEfgEnumPure::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtEfgNashAlgorithm *gbtPanelEfgEnumPure::GetAlgorithm(void) const
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
//                      class gbtPanelEfgEnumMixed
//========================================================================

class gbtPanelEfgEnumMixed : public gbtPanelEfgNashAlgorithm {
private:
  wxRadioBox *m_solveUsing, *m_precision;
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  gbtPanelEfgEnumMixed(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtPanelEfgEnumMixed, gbtPanelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, gbtPanelEfgEnumMixed::OnFindAll)
END_EVENT_TABLE()

gbtPanelEfgEnumMixed::gbtPanelEfgEnumMixed(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

void gbtPanelEfgEnumMixed::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtEfgNashAlgorithm *gbtPanelEfgEnumMixed::GetAlgorithm(void) const
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
//                         class gbtPanelEfgLcp
//========================================================================

const int idCHECKBOX_LIMITDEPTH = 2002;

class gbtPanelEfgLcp : public gbtPanelEfgNashAlgorithm {
private:
  wxRadioBox *m_solveUsing, *m_precision;
  wxCheckBox *m_findAll, *m_limitDepth;
  wxSpinCtrl *m_stopAfter, *m_maxDepth;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);
  void OnStopAfter(wxSpinEvent &);
  void OnLimitDepth(wxCommandEvent &);

public:
  gbtPanelEfgLcp(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtPanelEfgLcp, gbtPanelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, gbtPanelEfgLcp::OnFindAll)
  EVT_SPINCTRL(idSPINCTRL_STOPAFTER, gbtPanelEfgLcp::OnStopAfter)
  EVT_CHECKBOX(idCHECKBOX_LIMITDEPTH, gbtPanelEfgLcp::OnLimitDepth)
END_EVENT_TABLE()

gbtPanelEfgLcp::gbtPanelEfgLcp(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

void gbtPanelEfgLcp::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
  m_limitDepth->Enable(m_findAll->GetValue() || 
		       m_stopAfter->GetValue() > 1);
  m_maxDepth->Enable((m_findAll->GetValue() || m_stopAfter->GetValue() > 1) &&
		     m_limitDepth->GetValue());
}

void gbtPanelEfgLcp::OnStopAfter(wxSpinEvent &)
{
  m_limitDepth->Enable(m_stopAfter->GetValue() > 1);
  m_maxDepth->Enable(m_stopAfter->GetValue() > 1 && m_limitDepth->GetValue());
}

void gbtPanelEfgLcp::OnLimitDepth(wxCommandEvent &)
{
  m_maxDepth->Enable(m_limitDepth->GetValue());
}

gbtEfgNashAlgorithm *gbtPanelEfgLcp::GetAlgorithm(void) const
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
//                         class gbtPanelEfgLp
//========================================================================

class gbtPanelEfgLp : public gbtPanelEfgNashAlgorithm {
private:
  wxRadioBox *m_solveUsing, *m_precision;
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  gbtPanelEfgLp(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtPanelEfgLp, gbtPanelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, gbtPanelEfgLp::OnFindAll)
END_EVENT_TABLE()

gbtPanelEfgLp::gbtPanelEfgLp(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

void gbtPanelEfgLp::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtEfgNashAlgorithm *gbtPanelEfgLp::GetAlgorithm(void) const
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
//                        class gbtPanelEfgLiap
//========================================================================

class gbtPanelEfgLiap : public gbtPanelEfgNashAlgorithm {
private:
  wxRadioBox *m_solveUsing;
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter, *m_numTries;
  wxSpinCtrl *m_maxits;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  gbtPanelEfgLiap(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtPanelEfgLiap, gbtPanelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, gbtPanelEfgLiap::OnFindAll)
END_EVENT_TABLE()

gbtPanelEfgLiap::gbtPanelEfgLiap(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

void gbtPanelEfgLiap::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtEfgNashAlgorithm *gbtPanelEfgLiap::GetAlgorithm(void) const
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
//                      class gbtPanelEfgEnumPoly
//========================================================================

class gbtPanelEfgEnumPoly : public gbtPanelEfgNashAlgorithm {
private:
  wxRadioBox *m_solveUsing;
  wxCheckBox *m_findAll;
  wxSpinCtrl *m_stopAfter;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  gbtPanelEfgEnumPoly(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtPanelEfgEnumPoly, gbtPanelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, gbtPanelEfgEnumPoly::OnFindAll)
END_EVENT_TABLE()

gbtPanelEfgEnumPoly::gbtPanelEfgEnumPoly(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  wxStaticBox *centerBox = new wxStaticBox(this, wxID_STATIC, 
					   wxT("PolEnumSolve"));
  wxStaticBoxSizer *centerSizer = new wxStaticBoxSizer(centerBox, wxVERTICAL);
  centerSizer->Add(new wxStaticText(this, wxID_STATIC,
				    _("Find Nash equilibria via solving gbtPolyUni equations")),
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

void gbtPanelEfgEnumPoly::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

gbtEfgNashAlgorithm *gbtPanelEfgEnumPoly::GetAlgorithm(void) const
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
//                         class gbtPanelEfgLogit
//========================================================================

class gbtPanelEfgLogit : public gbtPanelEfgNashAlgorithm {
public:
  gbtPanelEfgLogit(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;
};

gbtPanelEfgLogit::gbtPanelEfgLogit(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

gbtEfgNashAlgorithm *gbtPanelEfgLogit::GetAlgorithm(void) const
{
  gbtEfgNashLogit *algorithm = new gbtEfgNashLogit;
  algorithm->SetFullGraph(false);
  algorithm->SetMaxLambda(1000000000);
  return algorithm;
}

//========================================================================
//                       class gbtPanelEfgSimpdiv
//========================================================================

const int idCHECKBOX_USELEASH = 2002;

class gbtPanelEfgSimpdiv : public gbtPanelEfgNashAlgorithm {
private:
  wxRadioBox *m_solveUsing, *m_precision;
  wxCheckBox *m_useLeash;
  wxSpinCtrl *m_leashLength, *m_numRestarts;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);
  void OnUseLeash(wxCommandEvent &);

public:
  gbtPanelEfgSimpdiv(wxWindow *);

  gbtEfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(gbtPanelEfgSimpdiv, gbtPanelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_USELEASH, gbtPanelEfgSimpdiv::OnUseLeash)
END_EVENT_TABLE()

gbtPanelEfgSimpdiv::gbtPanelEfgSimpdiv(wxWindow *p_parent)
  : gbtPanelEfgNashAlgorithm(p_parent)
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

void gbtPanelEfgSimpdiv::OnUseLeash(wxCommandEvent &)
{
  m_leashLength->Enable(m_useLeash->GetValue());
}

gbtEfgNashAlgorithm *gbtPanelEfgSimpdiv::GetAlgorithm(void) const
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
  wxTreeItemId init = LoadAlgorithms(p_support.GetTree());
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
  gbtPanelEfgNashAlgorithm *panel;
  id = m_algorithmTree->AppendItem(standard, _("One Nash equilibrium"));
  m_algorithms.Define(id, panel = new gbtPanelEfgOneNash(this));
  wxTreeItemId init = id;

  id = m_algorithmTree->AppendItem(standard, _("Two Nash equilibria"));
  m_algorithms.Define(id, panel = new gbtPanelEfgTwoNash(this));

  id = m_algorithmTree->AppendItem(standard, _("All Nash equilibria"));
  m_algorithms.Define(id, panel = new gbtPanelEfgAllNash(this));

  id = m_algorithmTree->AppendItem(standard, _("One perfect equilibrium"));
  m_algorithms.Define(id, panel = new gbtPanelEfgOnePerfect(this));

  id = m_algorithmTree->AppendItem(standard, _("Two perfect equilibria"));
  m_algorithms.Define(id, panel = new gbtPanelEfgTwoPerfect(this));

  id = m_algorithmTree->AppendItem(standard, _("All perfect equilibria"));
  m_algorithms.Define(id, panel = new gbtPanelEfgAllPerfect(this));

  id = m_algorithmTree->AppendItem(standard, _("One sequential equilibrium"));
  m_algorithms.Define(id, panel = new gbtPanelEfgOneSequential(this));

  id = m_algorithmTree->AppendItem(standard, _("Two sequential equilibria"));
  m_algorithms.Define(id, panel = new gbtPanelEfgTwoSequential(this));

  id = m_algorithmTree->AppendItem(standard, _("All sequential equilibria"));
  m_algorithms.Define(id, panel = new gbtPanelEfgAllSequential(this));

  wxTreeItemId custom = m_algorithmTree->AppendItem(root, 
						    _("Custom algorithms"));
  m_algorithmTree->SetItemBold(custom);

  id = m_algorithmTree->AppendItem(custom, wxT("EnumPureSolve"));
  m_algorithms.Define(id, panel = new gbtPanelEfgEnumPure(this));

  if (p_efg->NumPlayers() == 2) {
    id = m_algorithmTree->AppendItem(custom, wxT("EnumMixedSolve"));
    m_algorithms.Define(id, panel = new gbtPanelEfgEnumMixed(this));

    id = m_algorithmTree->AppendItem(custom, wxT("LcpSolve"));
    m_algorithms.Define(id, panel = new gbtPanelEfgLcp(this));

    if (p_efg->IsConstSum()) {
      id = m_algorithmTree->AppendItem(custom, wxT("LpSolve"));
      m_algorithms.Define(id, panel = new gbtPanelEfgLp(this));
    }
  }

  id = m_algorithmTree->AppendItem(custom, wxT("LiapSolve"));
  m_algorithms.Define(id, panel = new gbtPanelEfgLiap(this));

  id = m_algorithmTree->AppendItem(custom, wxT("PolEnumSolve"));
  m_algorithms.Define(id, panel = new gbtPanelEfgEnumPoly(this));

  id = m_algorithmTree->AppendItem(custom, wxT("QreSolve"));
  m_algorithms.Define(id, panel = new gbtPanelEfgLogit(this));

  id = m_algorithmTree->AppendItem(custom, wxT("SimpdivSolve"));
  m_algorithms.Define(id, panel = new gbtPanelEfgSimpdiv(this));

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
template class gbtBaseMap<wxTreeItemId, gbtPanelEfgNashAlgorithm *>;
template class gbtOrdMap<wxTreeItemId, gbtPanelEfgNashAlgorithm *>;
