//
// FILE: efgview.cc -- Extensive form viewing class
//
// $Id$
//

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/mdi.h"
#endif

#include "wx/splitter.h"
#include "wx/proplist.h"

#include "efg.h"
#include "behavsol.h"

#include "guiapp.h"
#include "efgview.h"
#include "nfgview.h"
#include "efgtree.h"
#include "efginfopanel.h"
#include "efgsupports.h"
#include "efgsolutions.h"


guiEfgPropertiesView::guiEfgPropertiesView(guiEfgView *p_parent,
					   const Efg &p_efg)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_efg(p_efg), m_outcome(0)
{
  SetLabelSize(wxHORIZONTAL, 0);
  SetLabelSize(wxVERTICAL, 100);
  SetCellBackgroundColour(*wxLIGHT_GREY);
}

void guiEfgPropertiesView::OnChangeLabels(void)
{
  if (m_outcome) {
    SetLabelValue(wxVERTICAL, "Label", 0);
    for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
      SetLabelValue(wxVERTICAL,
		    (char *) (ToText(pl)+": "+m_efg.Players()[pl]->GetName()),
		    pl);
    }
  }
  else if (m_node) {
    SetLabelValue(wxVERTICAL, "Label", 0);
    SetLabelValue(wxVERTICAL, "Player", 1);
    SetLabelValue(wxVERTICAL, "Infoset", 2);
  }

  SetColumnWidth(0, 95);
}

void guiEfgPropertiesView::SetPropertyCount(int p_properties)
{
  if (GetRows() < p_properties) {
    AppendRows(p_properties - GetRows());
  }
  else if (GetRows() > p_properties) {
    DeleteRows(0, GetRows() - p_properties);
  }
  else {
    OnChangeLabels();
  }
}

void guiEfgPropertiesView::SetOutcome(EFOutcome *p_outcome)
{
  m_outcome = p_outcome;
  m_node = 0;

  SetPropertyCount(m_efg.NumPlayers() + 1);

  SetCellValue((char *) m_outcome->GetName(), 0, 0);

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    SetCellValue((char *) ToText(m_efg.Payoff(m_outcome, m_efg.Players()[pl])),
		 pl, 0);
  }
}

void guiEfgPropertiesView::SetNode(Node *p_node)
{
  m_node = p_node;
  m_outcome = 0;

  SetPropertyCount(3);

  SetCellValue((char *) m_node->GetName(), 0, 0);
  if (m_node->GetInfoset()) {
    SetCellValue((char *) m_node->GetPlayer()->GetName(), 1, 0);
    SetCellValue((char *) m_node->GetInfoset()->GetName(), 2, 0);
  }
  else {
    SetCellValue("(terminal)", 1, 0);
    SetCellValue("(terminal)", 2, 0);
  }
  
}

BEGIN_EVENT_TABLE(guiEfgView, wxWindow)
  EVT_SIZE(OnSize)
END_EVENT_TABLE()

guiEfgView::guiEfgView(guiEfgFrame *p_parent, FullEfg *p_efg,
		       wxWindow *p_solutionSplitter)
  : wxPanel(p_solutionSplitter, -1),
    m_parent(p_parent), m_efg(p_efg), m_copyNode(0), m_copyOutcome(0)
{
  m_supportsView = new guiEfgSupports(this, *p_efg);
  m_supportsView->Show(true);

  m_tree = new guiEfgTree(this, *p_efg);
  m_tree->Show(true);

  m_infoPanel = new guiEfgInfoPanel(this, *p_efg);
  m_infoPanel->Show(true);

  m_propertyView = new guiEfgPropertiesView(this, *p_efg);
  m_propertyView->SetEditable(false);
  m_propertyView->Show(false);

  Show(true);
  m_supportsView->SetCurrentSupport(1);
}

void guiEfgView::OnSize(wxSizeEvent &p_event)
{
  Arrange();
}

void guiEfgView::OnCopy(void)
{
  if (m_tree->SelectedNode()) {
    m_copyNode = m_tree->SelectedNode();
    m_copyOutcome = 0;
  }
  else if (m_tree->SelectedOutcome()) {
    m_copyNode = 0;
    m_copyOutcome = m_tree->SelectedOutcome();
  }
}

void guiEfgView::OnPaste(void)
{
  if (m_copyNode) {
    Node *currentNode = m_tree->SelectedNode();

    if (!currentNode) {
      return;
    }
    
    if (m_efg->NumChildren(currentNode) > 0) {
      return;
    }
    
    m_efg->CopyTree(m_copyNode, currentNode);
    m_tree->OnTreeChanged();
  }
  else if (m_copyOutcome) {
    Node *currentNode = m_tree->SelectedNode();

    if (!currentNode) {
      return;
    }

    currentNode->SetOutcome(m_copyOutcome);
    m_tree->OnTreeChanged();
  }
}

void guiEfgView::OnDelete(void)
{
}

void guiEfgView::OnProperties(void)
{
}

void guiEfgView::OnOutcomeChanged(EFOutcome *p_outcome)
{
  m_tree->OnOutcomeChanged(p_outcome);
  m_propertyView->SetOutcome(p_outcome);
}

void guiEfgView::OnNodeChanged(Node *p_node)
{
  m_tree->OnNodeChanged(p_node);
}

void guiEfgView::Arrange(void)
{
  int width, height;
  GetClientSize(&width, &height);
  m_supportsView->SetSize(0, 0, 200, height / 2);
  if (m_propertyView) {
    m_propertyView->SetSize(0, height / 2, 200, height / 2);
  }
  m_tree->SetSize(200, 0, width - 200, height);
}

void guiEfgView::OnSelectedOutcome(EFOutcome *p_outcome, bool p_selected)
{
  if (p_selected) {
    m_propertyView->SetOutcome(p_outcome);
    m_propertyView->Show(true);
  }
  else {
    m_propertyView->Show(false);
  }
  m_infoPanel->Show(!p_selected);
  Arrange();
}

EFOutcome *guiEfgView::SelectedOutcome(void) const
{ return m_tree->SelectedOutcome(); }

void guiEfgView::OnSelectedNode(Node *p_node, bool p_selected)
{
  if (p_selected) {
    m_propertyView->SetNode(p_node);
    m_propertyView->Show(true);
  }
  else {
    m_propertyView->Show(false);
  }
  m_infoPanel->Show(!p_selected);
  Arrange();
}

void guiEfgView::ShowWindows(bool p_show)
{
  m_tree->Show(p_show);
  m_infoPanel->Show(p_show);
}

double guiEfgView::GetZoom(void) const
{ return m_tree->GetZoom(); }

void guiEfgView::SetZoom(double p_zoom)
{ m_tree->SetZoom(p_zoom); }

void guiEfgView::AddSupport(EFSupport *p_support)
{ m_supportsView->AddSupport(p_support); }

const gList<EFSupport *> &guiEfgView::Supports(void) const 
{ return m_supportsView->Supports(); }

int guiEfgView::NumSupports(void) const 
{ return m_supportsView->NumSupports(); }

EFSupport *guiEfgView::CurrentSupport(void) const 
{ return m_supportsView->CurrentSupport(); }

int guiEfgView::CurrentSupportIndex(void) const
{ return m_supportsView->CurrentSupportIndex(); }

void guiEfgView::SetCurrentSupport(int p_index, bool p_updateWindow)
{
  if (p_updateWindow) {
    m_supportsView->SetCurrentSupport(p_index);
  }
  m_tree->OnSupportChanged(m_supportsView->CurrentSupport());
}

void guiEfgView::EditCurrentSupport(const EFSupport &p_support)
{
  m_supportsView->EditCurrentSupport(p_support);
  m_tree->OnSupportChanged(m_supportsView->CurrentSupport());
}
  
void guiEfgView::DeleteSupport(int p_index)
{
  m_supportsView->DeleteSupport(p_index);
  m_tree->OnSupportChanged(m_supportsView->CurrentSupport());
}

gText guiEfgView::UniqueSupportName(void) const
{ return m_supportsView->UniqueSupportName(); }

