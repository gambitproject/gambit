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

#include "nash/nfgpure.h"
#include "nash/enum.h"
#include "nash/nliap.h"
#include "nash/nfgalleq.h"
#include "nash/nfgqre.h"
#include "nash/simpdiv.h"

const int idCHECKBOX_FINDALL = 2000;

class panelNfgNashAlgorithm : public wxPanel {
public:
  panelNfgNashAlgorithm(wxWindow *p_parent) : wxPanel(p_parent, -1) { }

  virtual nfgNashAlgorithm *GetAlgorithm(void) const = 0;
};

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
  stopAfterSizer->Add(m_findAll, 0, wxALL, 5);
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
  stopAfterSizer->Add(m_findAll, 0, wxALL, 5);
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
  stopAfterSizer->Add(m_findAll, 0, wxALL, 5);
  stopAfterSizer->Add(new wxStaticText(this, wxID_STATIC, "Stop after"),
		      0, wxALL | wxCENTER, 5);
  m_stopAfter = new wxSpinCtrl(this, -1, "1",
			       wxDefaultPosition, wxDefaultSize,
			       wxSP_ARROW_KEYS, 1, 10000);
  stopAfterSizer->Add(m_stopAfter, 0, wxALL, 5);
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
  paramSizer->Add(m_maxits1, 0, wxALL, 5);
  
  paramSizer->Add(new wxStaticText(this, wxID_STATIC,
				   "Tolerance for line search (in digits)"),
		  0, wxALL | wxCENTER, 5);
  m_tol1 = new wxSpinCtrl(this, -1, "10",
			  wxDefaultPosition, wxDefaultSize,
			  wxSP_ARROW_KEYS, 2, 20);
  paramSizer->Add(m_tol1, 0, wxALL, 5);

  paramSizer->Add(new wxStaticText(this, wxID_STATIC,
				   "Maximum iterations in minimization"),
		  0, wxALL | wxCENTER, 5);
  m_maxitsN = new wxSpinCtrl(this, -1, "20",
			     wxDefaultPosition, wxDefaultSize,
			     wxSP_ARROW_KEYS, 10, 1000);
  paramSizer->Add(m_maxitsN, 0, wxALL, 5);
  
  paramSizer->Add(new wxStaticText(this, wxID_STATIC,
				   "Tolerance in minimization (in digits)"),
		  0, wxALL | wxCENTER, 5);
  m_tolN = new wxSpinCtrl(this, -1, "10",
			  wxDefaultPosition, wxDefaultSize,
			  wxSP_ARROW_KEYS, 2, 20);
  paramSizer->Add(m_tolN, 0, wxALL, 5);

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
  stopAfterSizer->Add(m_findAll, 0, wxALL, 5);
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
  (void) new wxStaticText(this, wxID_STATIC, "QreSolve");
  Show(false);
}

nfgNashAlgorithm *panelNfgQre::GetAlgorithm(void) const
{
  return new nfgQre;
}

//========================================================================
//                       class panelNfgSimpdiv
//========================================================================

const int idCHECKBOX_USELEASH = 2001;

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
  leashSizer->Add(m_useLeash, 0, wxALL, 5);
  leashSizer->Add(new wxStaticText(this, wxID_STATIC, "Leash length"),
		  0, wxALL | wxCENTER, 5);
  m_leashLength = new wxSpinCtrl(this, -1, "100",
				 wxDefaultPosition, wxDefaultSize,
				 wxSP_ARROW_KEYS, 1, 10000);
  m_leashLength->Enable(false);
  leashSizer->Add(m_leashLength, 0, wxALL, 5);
  algorithmSizer->Add(leashSizer, 0, wxALL | wxCENTER, 5);

  wxBoxSizer *restartSizer = new wxBoxSizer(wxHORIZONTAL);
  restartSizer->Add(new wxStaticText(this, wxID_STATIC, "Number of restarts"),
		    0, wxALL, 5);
  m_numRestarts = new wxSpinCtrl(this, -1, "20",
				 wxDefaultPosition, wxDefaultSize,
				 wxSP_ARROW_KEYS, 1, 10000);
  restartSizer->Add(m_numRestarts, 0, wxALL, 5);
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
  // Eventually, these should be loaded from wxConfig; for now,
  // I am going to hard-code them
  wxTreeItemId root = m_algorithmTree->AddRoot("Algorithms");
  wxTreeItemId standard = m_algorithmTree->AppendItem(root,
						      "Standard algorithms");
  m_algorithmTree->AppendItem(standard, "One Nash equilibrium");
  m_algorithmTree->AppendItem(standard, "Two Nash equilibria");
  m_algorithmTree->AppendItem(standard, "All Nash equilibria");
  if (p_nfg.NumPlayers() == 2) {
    m_algorithmTree->AppendItem(standard, "One perfect equilibrium");
    m_algorithmTree->AppendItem(standard, "Two perfect equilibria");
    m_algorithmTree->AppendItem(standard, "All perfect equilibria");
  }

  wxTreeItemId custom = m_algorithmTree->AppendItem(root, "Custom algorithms");
  wxTreeItemId id;
  id = m_algorithmTree->AppendItem(custom, "EnumPure");
  m_algorithms.Define(id, new panelNfgEnumPure(this));

  if (p_nfg.NumPlayers() == 2) {
    id = m_algorithmTree->AppendItem(custom, "EnumMixed");
    m_algorithms.Define(id, new panelNfgEnumMixed(this));
    m_algorithmTree->AppendItem(custom, "LcpSolve");
    if (IsConstSum(p_nfg)) {
      m_algorithmTree->AppendItem(custom, "LpSolve");
    }
  }

  id = m_algorithmTree->AppendItem(custom, "LiapSolve");
  m_algorithms.Define(id, new panelNfgLiap(this));

  id = m_algorithmTree->AppendItem(custom, "PolEnumSolve");
  m_algorithms.Define(id, new panelNfgPolEnum(this));
  
  id = m_algorithmTree->AppendItem(custom, "QreSolve");
  m_algorithms.Define(id, new panelNfgQre(this));
  m_algorithmTree->AppendItem(custom, "QreGridSolve");
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
}


#include "base/gmap.imp"
template class gBaseMap<wxTreeItemId, panelNfgNashAlgorithm *>;
template class gOrdMap<wxTreeItemId, panelNfgNashAlgorithm *>;
