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

#include "efg.h"
#include "behavsol.h"

#include "guiapp.h"
#include "efgview.h"
#include "nfgview.h"
#include "efgtree.h"
#include "efginfopanel.h"
#include "efgsolutions.h"

BEGIN_EVENT_TABLE(guiEfgView, wxWindow)
  EVT_SIZE(OnSize)
END_EVENT_TABLE()

guiEfgView::guiEfgView(guiEfgFrame *p_parent, FullEfg *p_efg,
		       wxWindow *p_solutionSplitter)
  : wxPanel(p_solutionSplitter, -1),
    m_parent(p_parent), m_efg(p_efg), m_copyNode(0), m_copyOutcome(0)
{
  m_tree = new guiEfgTree(this, *p_efg);
  m_tree->Show(TRUE);
  m_infoPanel = new guiEfgInfoPanel(this, *p_efg);
  m_infoPanel->Show(TRUE);
  /*
  m_infoPanel->SetConstraints(new wxLayoutConstraints);
  m_infoPanel->GetConstraints()->left.SameAs(this, wxLeft);
  m_infoPanel->GetConstraints()->top.SameAs(this, wxTop);
  m_infoPanel->GetConstraints()->width.Absolute(200);
  m_infoPanel->GetConstraints()->height.SameAs(this, wxHeight);

  m_tree->SetConstraints(new wxLayoutConstraints);
  m_tree->GetConstraints()->left.SameAs(m_infoPanel, wxRight);
  m_tree->GetConstraints()->top.SameAs(this, wxTop);
  m_tree->GetConstraints()->right.SameAs(this, wxRight);
  m_tree->GetConstraints()->height.SameAs(this, wxHeight);
  */
  Show(TRUE);
  //  Layout();
}

void guiEfgView::OnSize(wxSizeEvent &p_event)
{
  int width, height;
  GetClientSize(&width, &height);
  m_infoPanel->SetSize(0, 0, 200, height);
  m_tree->SetSize(200, 0, width - 200, height);
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
    
    if (currentNode->NumChildren() > 0) {
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

void guiEfgView::ShowWindows(bool p_show)
{
  m_tree->Show(p_show);
  m_infoPanel->Show(p_show);
}

double guiEfgView::GetZoom(void) const
{
  return m_tree->GetZoom();
}

void guiEfgView::SetZoom(double p_zoom)
{
  m_tree->SetZoom(p_zoom);
}

