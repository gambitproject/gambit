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
#include "dlefgnash.h"

#include "nash/subsolve.h"
#include "nash/efgpure.h"
#include "nash/enum.h"
#include "nash/efgqre.h"
#include "nash/simpdiv.h"

class panelEfgNashAlgorithm : public wxPanel {
public:
  panelEfgNashAlgorithm(wxWindow *p_parent) : wxPanel(p_parent, -1) { }

  virtual efgNashAlgorithm *GetAlgorithm(void) const = 0;
};

//========================================================================
//                      class panelEfgEnumPure
//========================================================================

class panelEfgEnumPure : public panelEfgNashAlgorithm {
public:
  panelEfgEnumPure(wxWindow *);

  efgNashAlgorithm *GetAlgorithm(void) const;
};

panelEfgEnumPure::panelEfgEnumPure(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  (void) new wxStaticText(this, wxID_STATIC, "EnumPureSolve");
  Show(false);
}

efgNashAlgorithm *panelEfgEnumPure::GetAlgorithm(void) const
{
  SubgameSolver *algorithm = new SubgameSolver;
  algorithm->SetAlgorithm(new efgEnumPure);
  return algorithm;
}

//========================================================================
//                      class panelEfgEnumMixed
//========================================================================

class panelEfgEnumMixed : public panelEfgNashAlgorithm {
public:
  panelEfgEnumMixed(wxWindow *);

  efgNashAlgorithm *GetAlgorithm(void) const;
};

panelEfgEnumMixed::panelEfgEnumMixed(wxWindow *p_parent)
  : panelEfgNashAlgorithm(p_parent)
{
  (void) new wxStaticText(this, wxID_STATIC, "EnumMixedSolve");
  Show(false);
}

efgNashAlgorithm *panelEfgEnumMixed::GetAlgorithm(void) const
{
  SubgameSolver *algorithm = new SubgameSolver;
  algorithm->SetAlgorithm(new nfgEnumMixed<double>);
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
  wxTreeItemId id;
  id = m_algorithmTree->AppendItem(custom, "EnumPure");
  m_algorithms.Define(id, new panelEfgEnumPure(this));
  id = m_algorithmTree->AppendItem(custom, "EnumMixed");
  m_algorithms.Define(id, new panelEfgEnumMixed(this));
  m_algorithmTree->AppendItem(custom, "LcpSolve");
  m_algorithmTree->AppendItem(custom, "LpSolve");
  m_algorithmTree->AppendItem(custom, "PolEnumSolve");
  id = m_algorithmTree->AppendItem(custom, "QreSolve");
  m_algorithms.Define(id, new panelEfgQre(this));
  m_algorithmTree->AppendItem(custom, "QreGridSolve");
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


