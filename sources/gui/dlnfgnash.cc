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
#include "dlnfgnash.h"

#include "nash/nfgpure.h"

class panelNfgNashAlgorithm : public wxPanel {
public:
  panelNfgNashAlgorithm(wxWindow *p_parent) : wxPanel(p_parent, -1) { }

  virtual nfgNashAlgorithm *GetAlgorithm(void) const = 0;
};

//========================================================================
//                      class panelNfgEnumPure
//========================================================================

class panelNfgEnumPure : public panelNfgNashAlgorithm {

public:
  panelNfgEnumPure(wxWindow *);

  nfgNashAlgorithm *GetAlgorithm(void) const;
};

panelNfgEnumPure::panelNfgEnumPure(wxWindow *p_parent)
  : panelNfgNashAlgorithm(p_parent)
{
  (void) new wxStaticText(this, wxID_STATIC, "EnumPureSolve");
  Show(false);
}

nfgNashAlgorithm *panelNfgEnumPure::GetAlgorithm(void) const
{
  return new nfgEnumPure;
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

void dialogNfgNash::LoadAlgorithms(void)
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
  m_algorithms.Define(id, new panelNfgEnumPure(this));
  m_algorithmTree->AppendItem(custom, "EnumMixed");
  m_algorithmTree->AppendItem(custom, "LcpSolve");
  m_algorithmTree->AppendItem(custom, "LpSolve");
  m_algorithmTree->AppendItem(custom, "PolEnumSolve");
  m_algorithmTree->AppendItem(custom, "QreSolve");
  m_algorithmTree->AppendItem(custom, "QreGridSolve");
  m_algorithmTree->AppendItem(custom, "SimpdivSolve");

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
