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
#include "dlefgnash.h"

#include "nash/subsolve.h"

#include "nash/efgpure.h"
#include "nash/nfgpure.h"
#include "nash/enum.h"
#include "nash/seqform.h"
#include "nash/lemke.h"
#include "nash/efgqre.h"
#include "nash/simpdiv.h"

const int idCHECKBOX_FINDALL = 2000;
const int idSPINCTRL_STOPAFTER = 2001;

class panelEfgNashAlgorithm : public wxPanel {
public:
  panelEfgNashAlgorithm(wxWindow *p_parent) : wxPanel(p_parent, -1) { }

  virtual efgNashAlgorithm *GetAlgorithm(void) const = 0;
};

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

  efgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelEfgEnumPure, panelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelEfgEnumPure::OnFindAll)
END_EVENT_TABLE()

panelEfgEnumPure::panelEfgEnumPure(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxString solveChoices[] = { "Extensive form", "Normal form" };
  m_solveUsing = new wxRadioBox(this, -1, "Find equilibria using",
				wxDefaultPosition, wxDefaultSize,
				2, solveChoices, 1, wxRA_SPECIFY_ROWS);
  topSizer->Add(m_solveUsing, 0, wxALL | wxCENTER, 5);

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

void panelEfgEnumPure::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

efgNashAlgorithm *panelEfgEnumPure::GetAlgorithm(void) const
{
  SubgameSolver *algorithm = new SubgameSolver;
  
  if (m_solveUsing->GetSelection() == 0) {
    efgEnumPure *subAlgorithm = new efgEnumPure;
    subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
			       0 : m_stopAfter->GetValue());
    algorithm->SetAlgorithm(subAlgorithm);
  }
  else {
    nfgEnumPure *subAlgorithm = new nfgEnumPure;
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

  efgNashAlgorithm *GetAlgorithm(void) const;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(panelEfgEnumMixed, panelEfgNashAlgorithm)
  EVT_CHECKBOX(idCHECKBOX_FINDALL, panelEfgEnumMixed::OnFindAll)
END_EVENT_TABLE()

panelEfgEnumMixed::panelEfgEnumMixed(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxString solveChoices[] = { "Extensive form", "Normal form" };
  m_solveUsing = new wxRadioBox(this, -1, "Find equilibria using",
				wxDefaultPosition, wxDefaultSize,
				2, solveChoices, 1, wxRA_SPECIFY_ROWS);
  m_solveUsing->SetSelection(1);
  m_solveUsing->Enable(false);
  topSizer->Add(m_solveUsing, 0, wxALL | wxCENTER, 5);

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

void panelEfgEnumMixed::OnFindAll(wxCommandEvent &)
{
  m_stopAfter->Enable(!m_findAll->GetValue());
}

efgNashAlgorithm *panelEfgEnumMixed::GetAlgorithm(void) const
{
  SubgameSolver *algorithm = new SubgameSolver;

  if (m_precision->GetSelection() == 0) {
    nfgEnumMixed<double> *subAlgorithm = new nfgEnumMixed<double>;
    subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
			       0 : m_stopAfter->GetValue());
    algorithm->SetAlgorithm(subAlgorithm);
  }
  else {
    nfgEnumMixed<gRational> *subAlgorithm = new nfgEnumMixed<gRational>;
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

  efgNashAlgorithm *GetAlgorithm(void) const;

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

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxString solveChoices[] = { "Extensive form", "Normal form" };
  m_solveUsing = new wxRadioBox(this, -1, "Find equilibria using",
				wxDefaultPosition, wxDefaultSize,
				2, solveChoices, 1, wxRA_SPECIFY_ROWS);
  topSizer->Add(m_solveUsing, 0, wxALL | wxCENTER, 5);

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

efgNashAlgorithm *panelEfgLcp::GetAlgorithm(void) const
{
  SubgameSolver *algorithm = new SubgameSolver;

  if (m_solveUsing->GetSelection() == 0) {
    if (m_precision->GetSelection() == 0) {
      efgLcp<double> *subAlgorithm = new efgLcp<double>;
      subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
				 0 : m_stopAfter->GetValue());
      subAlgorithm->SetMaxDepth((m_limitDepth->GetValue()) ?
				m_maxDepth->GetValue() : 0);
      algorithm->SetAlgorithm(subAlgorithm);
    }
    else {
      efgLcp<gRational> *subAlgorithm = new efgLcp<gRational>;
      subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
				 0 : m_stopAfter->GetValue());
      subAlgorithm->SetMaxDepth((m_limitDepth->GetValue()) ?
				m_maxDepth->GetValue() : 0);
      algorithm->SetAlgorithm(subAlgorithm);
    }
  }
  else {
    if (m_precision->GetSelection() == 0) {
      nfgLcp<double> *subAlgorithm = new nfgLcp<double>;
      subAlgorithm->SetStopAfter((m_findAll->GetValue()) ?
				 0 : m_stopAfter->GetValue());
      subAlgorithm->SetMaxDepth((m_limitDepth->GetValue()) ?
				m_maxDepth->GetValue() : 0);
      algorithm->SetAlgorithm(subAlgorithm);
    }
    else {
      nfgLcp<gRational> *subAlgorithm = new nfgLcp<gRational>;
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
//                        class panelEfgQre
//========================================================================

class panelEfgQre : public panelEfgNashAlgorithm {
public:
  panelEfgQre(wxWindow *);

  efgNashAlgorithm *GetAlgorithm(void) const;
};

panelEfgQre::panelEfgQre(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  (void) new wxStaticText(this, wxID_STATIC, "QreSolve");
  Show(false);
}

efgNashAlgorithm *panelEfgQre::GetAlgorithm(void) const
{
  return new efgQre;
}

//========================================================================
//                       class panelEfgSimpdiv
//========================================================================

class panelEfgSimpdiv : public panelEfgNashAlgorithm {
public:
  panelEfgSimpdiv(wxWindow *);

  efgNashAlgorithm *GetAlgorithm(void) const;
};

panelEfgSimpdiv::panelEfgSimpdiv(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  (void) new wxStaticText(this, wxID_STATIC, "SimpdivSolve");
  Show(false);
}

efgNashAlgorithm *panelEfgSimpdiv::GetAlgorithm(void) const
{
  SubgameSolver *algorithm = new SubgameSolver;
  algorithm->SetAlgorithm(new nfgSimpdiv<double>);
  return algorithm;
}

//========================================================================
//                        class dialogEfgNash
//========================================================================

const int idTREECTRL_ALGORITHMS = 2000;

BEGIN_EVENT_TABLE(dialogEfgNash, wxDialog)
  EVT_TREE_SEL_CHANGED(idTREECTRL_ALGORITHMS,
		       dialogEfgNash::OnSelectionChanged)
END_EVENT_TABLE()

dialogEfgNash::dialogEfgNash(wxWindow *p_parent, const EFSupport &p_support)
  : wxDialog(p_parent, -1, "Compute Nash equilibria"),
    m_algorithms(0)
{
  SetAutoLayout(true);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *algPanelSizer = new wxBoxSizer(wxHORIZONTAL);
  m_algorithmTree = new wxTreeCtrl(this, idTREECTRL_ALGORITHMS,
				   wxDefaultPosition, wxSize(200, 400));
  LoadAlgorithms();
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

void dialogEfgNash::LoadAlgorithms(void)
{
  wxTreeItemId id;

  // Eventually, these should be loaded from wxConfig; for now,
  // I am going to hard-code them
  wxTreeItemId root = m_algorithmTree->AddRoot("Algorithms");
  wxTreeItemId standard = m_algorithmTree->AppendItem(root,
						      "Standard algorithms");
  m_algorithmTree->AppendItem(standard, "One Nash equilibrium");
  m_algorithmTree->AppendItem(standard, "Two Nash equilibria");
  m_algorithmTree->AppendItem(standard, "All Nash equilibria");
  m_algorithmTree->AppendItem(standard, "One perfect equilibrium");
  m_algorithmTree->AppendItem(standard, "Two perfect equilibria");
  m_algorithmTree->AppendItem(standard, "All perfect equilibria");

  wxTreeItemId custom = m_algorithmTree->AppendItem(root, "Custom algorithms");

  id = m_algorithmTree->AppendItem(custom, "EnumPure");
  m_algorithms.Define(id, new panelEfgEnumPure(this));

  id = m_algorithmTree->AppendItem(custom, "EnumMixed");
  m_algorithms.Define(id, new panelEfgEnumMixed(this));

  id = m_algorithmTree->AppendItem(custom, "LcpSolve");
  m_algorithms.Define(id, new panelEfgLcp(this));

  id = m_algorithmTree->AppendItem(custom, "LpSolve");

  id = m_algorithmTree->AppendItem(custom, "PolEnumSolve");

  id = m_algorithmTree->AppendItem(custom, "QreSolve");
  m_algorithms.Define(id, new panelEfgQre(this));

  id = m_algorithmTree->AppendItem(custom, "SimpdivSolve");
  m_algorithms.Define(id, new panelEfgSimpdiv(this));

  m_algorithmTree->Expand(standard);
  m_algorithmTree->Expand(custom);
  m_algorithmTree->Expand(root);
}

void dialogEfgNash::OnSelectionChanged(wxTreeEvent &)
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

efgNashAlgorithm *dialogEfgNash::GetAlgorithm(void) const
{
  if (m_algorithms(m_algorithmTree->GetSelection())) {
    return m_algorithms(m_algorithmTree->GetSelection())->GetAlgorithm();
  }
  else {
    return 0;
  }
}


#ifdef __BCC55__
// Need to provide an explicit output operator to silence a BC 5.5
// compilation ambiguity for operator<<
static gOutput &operator<<(gOutput &p_stream, wxTreeItemId)
{ return p_stream; }
#endif  // __BCC55__

#include "base/gmap.imp"
template class gBaseMap<wxTreeItemId, panelEfgNashAlgorithm *>;
template class gOrdMap<wxTreeItemId, panelEfgNashAlgorithm *>;


