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

guiEfgView::guiEfgView(guiEfgFrame *p_parent, Efg *p_efg,
		       wxSplitterWindow *p_solutionSplitter,
		       wxSplitterWindow *p_infoSplitter)
  : m_parent(p_parent), m_efg(p_efg), m_copyNode(0), m_copyOutcome(0)
{
  m_tree = new guiEfgTree(this, p_solutionSplitter, *p_efg);
  m_infoPanel = new guiEfgInfoPanel(this, p_infoSplitter, *p_efg);

  p_infoSplitter->SplitVertically(m_infoPanel,
				  p_solutionSplitter, 300);
  p_solutionSplitter->Initialize(m_tree);
  m_tree->Show(TRUE);
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

