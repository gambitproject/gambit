//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for selecting algorithms to compute Nash equilibria
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/spinctrl.h"
#include "dlnfgnash.h"

#include "base/gnullstatus.h"
#include "nash/nfgpure.h"
#include "nash/enum.h"
#include "nash/lemke.h"
#include "nash/nfgcsum.h"
#include "nash/nliap.h"
#include "nash/nfgalleq.h"
#include "nash/nfgqre.h"
#include "nash/simpdiv.h"

const int idCHECKBOX_FINDALL = 2000;
const int idSPINCTRL_STOPAFTER = 2001;

class panelNfgNashAlgorithm : public wxPanel {
public:
  panelNfgNashAlgorithm(wxWindow *p_parent) : wxPanel(p_parent, -1) { }

  virtual nfgNashAlgorithm *GetAlgorithm(void) const = 0;
};

//========================================================================
//                         class nfgOneNash
//========================================================================

class nfgOneNash : public nfgNashAlgorithm {
public:
  gText GetAlgorithm(void) const { return "OneNash"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};

gList<MixedSolution> nfgOneNash::Solve(const NFSupport &p_support,
				       gStatus &p_status)
{
  gArray<int> players(p_support.Game().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gNullStatus status;
    gNullOutput gnull;
    /* one round of elimination of weakly dominated strategies */
    NFSupport support = p_support.Undominated(false, players, gnull, status);

    if (p_support.Game().NumPlayers() == 2) {
      if (IsConstSum(p_support.Game())) {
	nfgLp<double> algorithm;
	return algorithm.Solve(support, p_status);
      }
      else {
	nfgLcp<double> algorithm;
	algorithm.SetStopAfter(1);
	return algorithm.Solve(support, p_status);
      }
    }
    else {
      nfgSimpdiv<double> algorithm;
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

  nfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgOneNash::panelNfgOneNash(wxWindow *p_parent)
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

nfgNashAlgorithm *panelNfgOneNash::GetAlgorithm(void) const
{
  return new nfgOneNash;
}

//========================================================================
//                         class nfgTwoNash
//========================================================================

class nfgTwoNash : public nfgNashAlgorithm {
public:
  gText GetAlgorithm(void) const { return "TwoNash"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};

gList<MixedSolution> nfgTwoNash::Solve(const NFSupport &p_support,
				       gStatus &p_status)
{
  gArray<int> players(p_support.Game().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    NFSupport support(p_support);

    while (true) {
      gNullStatus status;
      gNullOutput gnull;
      NFSupport newSupport = support.Undominated(true, players,
						 gnull, status);
      
      if (newSupport == support) {
	break;
      }
      else {
	support = newSupport;
      }
    }

    if (p_support.Game().NumPlayers() == 2) {
      nfgEnumMixed<double> algorithm;
      algorithm.SetStopAfter(2);
      return algorithm.Solve(support, p_status);
    }
    else {
      nfgPolEnum algorithm;
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

  nfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgTwoNash::panelNfgTwoNash(wxWindow *p_parent)
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

nfgNashAlgorithm *panelNfgTwoNash::GetAlgorithm(void) const
{
  return new nfgTwoNash;
}

//========================================================================
//                         class nfgAllNash
//========================================================================

class nfgAllNash : public nfgNashAlgorithm {
public:
  gText GetAlgorithm(void) const { return "AllNash"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};

gList<MixedSolution> nfgAllNash::Solve(const NFSupport &p_support,
				       gStatus &p_status)
{
  gArray<int> players(p_support.Game().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    NFSupport support(p_support);

    while (true) {
      gNullStatus status;
      gNullOutput gnull;
      NFSupport newSupport = support.Undominated(true, players,
						 gnull, status);
      
      if (newSupport == support) {
	break;
      }
      else {
	support = newSupport;
      }
    }

    if (p_support.Game().NumPlayers() == 2) {
      nfgEnumMixed<double> algorithm;
      algorithm.SetStopAfter(0);
      return algorithm.Solve(support, p_status);
    }
    else {
      nfgPolEnum algorithm;
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

  nfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgAllNash::panelNfgAllNash(wxWindow *p_parent)
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

nfgNashAlgorithm *panelNfgAllNash::GetAlgorithm(void) const
{
  return new nfgAllNash;
}

//========================================================================
//                         class nfgOnePerfect
//========================================================================

class nfgOnePerfect : public nfgNashAlgorithm {
public:
  gText GetAlgorithm(void) const { return "OnePerfect"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};

gList<MixedSolution> nfgOnePerfect::Solve(const NFSupport &p_support,
					  gStatus &p_status)
{
  gArray<int> players(p_support.Game().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gNullStatus status;
    gNullOutput gnull;
    /* one round of elimination of weakly dominated strategies */
    NFSupport support = p_support.Undominated(false, players, gnull, status);

    nfgLcp<double> algorithm;
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

  nfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgOnePerfect::panelNfgOnePerfect(wxWindow *p_parent)
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

nfgNashAlgorithm *panelNfgOnePerfect::GetAlgorithm(void) const
{
  return new nfgOnePerfect;
}

//========================================================================
//                         class nfgTwoPerfect
//========================================================================

class nfgTwoPerfect : public nfgNashAlgorithm {
public:
  gText GetAlgorithm(void) const { return "TwoPerfect"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};

gList<MixedSolution> nfgTwoPerfect::Solve(const NFSupport &p_support,
					  gStatus &p_status)
{
  gArray<int> players(p_support.Game().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gNullStatus status;
    gNullOutput gnull;
    /* one round of elimination of weakly dominated strategies */
    NFSupport support = p_support.Undominated(false, players, gnull, status);

    nfgEnumMixed<double> algorithm;
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

  nfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgTwoPerfect::panelNfgTwoPerfect(wxWindow *p_parent)
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

nfgNashAlgorithm *panelNfgTwoPerfect::GetAlgorithm(void) const
{
  return new nfgTwoPerfect;
}

//========================================================================
//                         class nfgAllPerfect
//========================================================================

class nfgAllPerfect : public nfgNashAlgorithm {
public:
  gText GetAlgorithm(void) const { return "AllPerfect"; }
  gList<MixedSolution> Solve(const NFSupport &, gStatus &);
};

gList<MixedSolution> nfgAllPerfect::Solve(const NFSupport &p_support,
					  gStatus &p_status)
{
  gArray<int> players(p_support.Game().NumPlayers());
  for (int pl = 1; pl <= players.Length(); pl++) {
    players[pl] = pl;
  }

  try {
    gNullStatus status;
    gNullOutput gnull;
    /* one round of elimination of weakly dominated strategies */
    NFSupport support = p_support.Undominated(false, players, gnull, status);

    nfgEnumMixed<double> algorithm;
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

  nfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgAllPerfect::panelNfgAllPerfect(wxWindow *p_parent)
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

nfgNashAlgorithm *panelNfgAllPerfect::GetAlgorithm(void) const
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

  nfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelNfgEnumPure, panelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelNfgEnumPure::OnFindAll)
END_EVENT_TABLE()

panelNfgEnumPure::panelNfgEnumPure(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

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
  topSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 5);

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

nfgNashAlgorithm *panelNfgEnumPure::GetAlgorithm(void) const
{
  nfgEnumPure *algorithm = new nfgEnumPure;
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

  nfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelNfgEnumMixed, panelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelNfgEnumMixed::OnFindAll)
END_EVENT_TABLE()

panelNfgEnumMixed::panelNfgEnumMixed(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxString precisionChoices[] = { "Floating point", "Rational" };
  m_precision = new wxRadioBox(this, -1, "Precision",
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 1, wxRA_SPECIFY_ROWS);
  topSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);

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
  topSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 5);

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

nfgNashAlgorithm *panelNfgEnumMixed::GetAlgorithm(void) const
{
  if (m_precision->GetSelection() == 0) {
    nfgEnumMixed<double> *algorithm = new nfgEnumMixed<double>;
    algorithm->SetStopAfter((m_findAll->GetValue()) ?
			    0 : m_stopAfter->GetValue());
    return algorithm;
  }
  else {
    nfgEnumMixed<gRational> *algorithm = new nfgEnumMixed<gRational>;
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

  nfgNashAlgorithm *GetAlgorithm(void) const;

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

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxString precisionChoices[] = { "Floating point", "Rational" };
  m_precision = new wxRadioBox(this, -1, "Precision",
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 1, wxRA_SPECIFY_ROWS);
  topSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);

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
  topSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 5);

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
  topSizer->Add(depthSizer, 0, wxALL | wxCENTER, 5);

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

nfgNashAlgorithm *panelNfgLcp::GetAlgorithm(void) const
{
  if (m_precision->GetSelection() == 0) {
    nfgLcp<double> *algorithm = new nfgLcp<double>;
    algorithm->SetStopAfter((m_findAll->GetValue()) ?
			    0 : m_stopAfter->GetValue());
    algorithm->SetMaxDepth((m_limitDepth->GetValue()) ?
			   m_maxDepth->GetValue() : 0);
    return algorithm;
  }
  else {
    nfgLcp<gRational> *algorithm = new nfgLcp<gRational>;
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

  nfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelNfgLp, panelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelNfgLp::OnFindAll)
END_EVENT_TABLE()

panelNfgLp::panelNfgLp(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxString precisionChoices[] = { "Floating point", "Rational" };
  m_precision = new wxRadioBox(this, -1, "Precision",
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 1, wxRA_SPECIFY_ROWS);
  topSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);

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
  topSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 5);

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

nfgNashAlgorithm *panelNfgLp::GetAlgorithm(void) const
{
  if (m_precision->GetSelection() == 0) {
    nfgLp<double> *algorithm = new nfgLp<double>;
    return algorithm;
  }
  else {
    nfgLp<gRational> *algorithm = new nfgLp<gRational>;
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
  wxSpinCtrl *m_maxits1, *m_tol1, *m_maxitsN, *m_tolN;

  // Private event handlers
  void OnFindAll(wxCommandEvent &);

public:
  panelNfgLiap(wxWindow *);

  nfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelNfgLiap, panelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelNfgLiap::OnFindAll)
END_EVENT_TABLE()

panelNfgLiap::panelNfgLiap(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

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
  topSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 5);

  wxStaticBox *algorithmBox = new wxStaticBox(this, wxID_STATIC,
					      "Algorithm behavior");
  wxStaticBoxSizer *algorithmSizer = new wxStaticBoxSizer(algorithmBox,
							  wxHORIZONTAL);
  algorithmSizer->Add(new wxStaticText(this, wxID_STATIC,
				       "Number of restarts"),
		      0, wxALL | wxCENTER, 5);
  m_numTries = new wxSpinCtrl(this, -1, "20",
			      wxDefaultPosition, wxDefaultSize,
			      wxSP_ARROW_KEYS, 1, 10000);
  algorithmSizer->Add(m_numTries, 0, wxALL, 5);
  topSizer->Add(algorithmSizer, 0, wxALL | wxCENTER, 5);

  wxStaticBox *funcminBox = new wxStaticBox(this, wxID_STATIC,
					    "Function minimization behavior");
  wxStaticBoxSizer *funcminSizer = new wxStaticBoxSizer(funcminBox,
							wxVERTICAL);
  wxFlexGridSizer *paramSizer = new wxFlexGridSizer(2);
  paramSizer->Add(new wxStaticText(this, wxID_STATIC,
				   "Maximum iterations for line search"),
		  0, wxALL | wxCENTER, 5);
  m_maxits1 = new wxSpinCtrl(this, -1, "100",
			     wxDefaultPosition, wxDefaultSize,
			     wxSP_ARROW_KEYS, 10, 10000);
  paramSizer->Add(m_maxits1, 0, wxALL | wxCENTER, 5);
  
  paramSizer->Add(new wxStaticText(this, wxID_STATIC,
				   "Tolerance for line search (in digits)"),
		  0, wxALL | wxCENTER, 5);
  m_tol1 = new wxSpinCtrl(this, -1, "10",
			  wxDefaultPosition, wxDefaultSize,
			  wxSP_ARROW_KEYS, 2, 20);
  paramSizer->Add(m_tol1, 0, wxALL | wxCENTER, 5);

  paramSizer->Add(new wxStaticText(this, wxID_STATIC,
				   "Maximum iterations in minimization"),
		  0, wxALL | wxCENTER, 5);
  m_maxitsN = new wxSpinCtrl(this, -1, "20",
			     wxDefaultPosition, wxDefaultSize,
			     wxSP_ARROW_KEYS, 10, 1000);
  paramSizer->Add(m_maxitsN, 0, wxALL | wxCENTER, 5);
  
  paramSizer->Add(new wxStaticText(this, wxID_STATIC,
				   "Tolerance in minimization (in digits)"),
		  0, wxALL | wxCENTER, 5);
  m_tolN = new wxSpinCtrl(this, -1, "10",
			  wxDefaultPosition, wxDefaultSize,
			  wxSP_ARROW_KEYS, 2, 20);
  paramSizer->Add(m_tolN, 0, wxALL | wxCENTER, 5);

  funcminSizer->Add(paramSizer, 0, wxALL, 5);
  
  topSizer->Add(funcminSizer, 0, wxALL | wxCENTER, 5);

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

nfgNashAlgorithm *panelNfgLiap::GetAlgorithm(void) const
{
  nfgLiap *algorithm = new nfgLiap;
  algorithm->SetStopAfter((m_findAll->GetValue()) ?
			  0 : m_stopAfter->GetValue());
  algorithm->SetNumTries(m_numTries->GetValue());
  algorithm->SetMaxits1(m_maxits1->GetValue());
  algorithm->SetTol1(pow(10.0, (double) -m_tol1->GetValue()));
  algorithm->SetMaxitsN(m_maxitsN->GetValue());
  algorithm->SetTolN(pow(10.0, (double) -m_tolN->GetValue()));
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

  nfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelNfgPolEnum, panelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelNfgPolEnum::OnFindAll)
END_EVENT_TABLE()

panelNfgPolEnum::panelNfgPolEnum(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
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
  topSizer->Add(stopAfterSizer, 0, wxALL | wxCENTER, 5);

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

nfgNashAlgorithm *panelNfgPolEnum::GetAlgorithm(void) const
{
  nfgPolEnum *algorithm = new nfgPolEnum;
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

  nfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgQre::panelNfgQre(wxWindow *p_parent)
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

nfgNashAlgorithm *panelNfgQre::GetAlgorithm(void) const
{
  nfgQre *algorithm = new nfgQre;
  algorithm->SetFullGraph(false);
  algorithm->SetMaxLambda(1000);
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

  nfgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelNfgSimpdiv, panelNfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_USELEASH, panelNfgSimpdiv::OnUseLeash)
END_EVENT_TABLE()

panelNfgSimpdiv::panelNfgSimpdiv(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxString precisionChoices[] = { "Floating point", "Rational" };
  m_precision = new wxRadioBox(this, -1, "Precision",
			       wxDefaultPosition, wxDefaultSize,
			       2, precisionChoices, 1, wxRA_SPECIFY_ROWS);
  topSizer->Add(m_precision, 0, wxALL | wxCENTER, 5);

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

  topSizer->Add(algorithmSizer, 0, wxALL | wxCENTER, 5);

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

nfgNashAlgorithm *panelNfgSimpdiv::GetAlgorithm(void) const
{
  if (m_precision->GetSelection() == 0) {
    nfgSimpdiv<double> *algorithm = new nfgSimpdiv<double>;
    algorithm->SetLeashLength((m_useLeash->GetValue()) ?
			      m_leashLength->GetValue() : 0);
    algorithm->SetNumRestarts(m_numRestarts->GetValue());
    return algorithm;
  }
  else {
    nfgSimpdiv<gRational> *algorithm = new nfgSimpdiv<gRational>;
    algorithm->SetLeashLength((m_useLeash->GetValue()) ?
			      m_leashLength->GetValue() : 0);
    algorithm->SetNumRestarts(m_numRestarts->GetValue());
    return algorithm;
  }
}

//========================================================================
//                        class dialogNfgNash
//========================================================================

const int idTREECTRL_ALGORITHMS = 2000;

BEGIN_EVENT_TABLE(dialogNfgNash, wxDialog)
  EVT_TREE_SEL_CHANGED(idTREECTRL_ALGORITHMS,
		       dialogNfgNash::OnSelectionChanged)
END_EVENT_TABLE()

dialogNfgNash::dialogNfgNash(wxWindow *p_parent, const NFSupport &p_support)
  : wxDialog(p_parent, -1, "Compute Nash equilibria"),
    m_algorithms(0)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *algPanelSizer = new wxBoxSizer(wxHORIZONTAL);
  m_algorithmTree = new wxTreeCtrl(this, idTREECTRL_ALGORITHMS,
				   wxDefaultPosition, wxSize(200, 400));
  LoadAlgorithms(p_support.Game());
  algPanelSizer->Add(m_algorithmTree, 1, wxALL | wxEXPAND, 5);

  m_fooPanel = new wxPanel(this, -1, wxDefaultPosition, wxSize(400, 400));
  m_currentPanel = m_fooPanel;
  algPanelSizer->Add(m_fooPanel, 0, wxALL, 5);
  topSizer->Add(algPanelSizer, 1, wxALL | wxEXPAND, 5);

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), 0, wxALL, 5);
  buttonSizer->Add(new wxButton(this, wxID_HELP, "Help"), 0, wxALL, 5);

  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

void dialogNfgNash::LoadAlgorithms(const Nfg &p_nfg)
{
  wxTreeItemId id;

  // Eventually, these should be loaded from wxConfig; for now,
  // I am going to hard-code them
  wxTreeItemId root = m_algorithmTree->AddRoot("Algorithms");
  wxTreeItemId standard = m_algorithmTree->AppendItem(root,
						      "Standard algorithms");

  id = m_algorithmTree->AppendItem(standard, "One Nash equilibrium");
  m_algorithms.Define(id, new panelNfgOneNash(this));
  
  id = m_algorithmTree->AppendItem(standard, "Two Nash equilibria");
  m_algorithms.Define(id, new panelNfgTwoNash(this));

  id = m_algorithmTree->AppendItem(standard, "All Nash equilibria");
  m_algorithms.Define(id, new panelNfgAllNash(this));

  if (p_nfg.NumPlayers() == 2) {
    id = m_algorithmTree->AppendItem(standard, "One perfect equilibrium");
    m_algorithms.Define(id, new panelNfgOnePerfect(this));

    id = m_algorithmTree->AppendItem(standard, "Two perfect equilibria");
    m_algorithms.Define(id, new panelNfgTwoPerfect(this));

    id = m_algorithmTree->AppendItem(standard, "All perfect equilibria");
    m_algorithms.Define(id, new panelNfgAllPerfect(this));
  }

  wxTreeItemId custom = m_algorithmTree->AppendItem(root, "Custom algorithms");

  id = m_algorithmTree->AppendItem(custom, "EnumPureSolve");
  m_algorithms.Define(id, new panelNfgEnumPure(this));

  if (p_nfg.NumPlayers() == 2) {
    id = m_algorithmTree->AppendItem(custom, "EnumMixedSolve");
    m_algorithms.Define(id, new panelNfgEnumMixed(this));

    id = m_algorithmTree->AppendItem(custom, "LcpSolve");
    m_algorithms.Define(id, new panelNfgLcp(this));

    if (IsConstSum(p_nfg)) {
      id = m_algorithmTree->AppendItem(custom, "LpSolve");
      m_algorithms.Define(id, new panelNfgLp(this));
    }
  }

  id = m_algorithmTree->AppendItem(custom, "LiapSolve");
  m_algorithms.Define(id, new panelNfgLiap(this));

  id = m_algorithmTree->AppendItem(custom, "PolEnumSolve");
  m_algorithms.Define(id, new panelNfgPolEnum(this));
  
  id = m_algorithmTree->AppendItem(custom, "QreSolve");
  m_algorithms.Define(id, new panelNfgQre(this));

  id = m_algorithmTree->AppendItem(custom, "SimpdivSolve");
  m_algorithms.Define(id, new panelNfgSimpdiv(this));

  m_algorithmTree->Expand(standard);
  m_algorithmTree->Expand(custom);
  m_algorithmTree->Expand(root);
}

void dialogNfgNash::OnSelectionChanged(wxTreeEvent &)
{
  wxPanel *panel = m_algorithms(m_algorithmTree->GetSelection());
  m_currentPanel->Show(false);
  if (panel) {
    m_currentPanel = panel;
    panel->Move(m_fooPanel->GetPosition());
    panel->SetSize(m_fooPanel->GetSize());
    panel->Show(true);
  }
  else {
    m_currentPanel = m_fooPanel;
    m_fooPanel->Show(true);
  }
}

nfgNashAlgorithm *dialogNfgNash::GetAlgorithm(void) const
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

