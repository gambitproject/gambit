//
// FILE: nfgview.cc -- Normal form viewing class
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

#include "guiapp.h"
#include "nfgview.h"
#include "nfgframe.h"
#include "nfggrid.h"
#include "nfginfopanel.h"

BEGIN_EVENT_TABLE(guiNfgView, wxWindow)
  EVT_SIZE(OnSize)
END_EVENT_TABLE()

guiNfgView::guiNfgView(Nfg *p_nfg, wxWindow *p_solutionSplitter)
  : wxWindow(p_solutionSplitter, -1), m_nfg(p_nfg), m_currentSupport(1)
{
  SetAutoLayout(true);
  m_grid = new guiNfgGrid(this, *p_nfg);
  m_infoPanel = new guiNfgInfoPanel(this, *p_nfg);

  m_infoPanel->SetConstraints(new wxLayoutConstraints);
  m_infoPanel->GetConstraints()->left.SameAs(this, wxLeft);
  m_infoPanel->GetConstraints()->top.SameAs(this, wxTop);
  m_infoPanel->GetConstraints()->width.Absolute(200);
  m_infoPanel->GetConstraints()->height.SameAs(this, wxHeight);

  m_grid->SetConstraints(new wxLayoutConstraints);
  m_grid->GetConstraints()->left.SameAs(m_infoPanel, wxRight);
  m_grid->GetConstraints()->top.SameAs(this, wxTop);
  m_grid->GetConstraints()->right.SameAs(this, wxRight);
  m_grid->GetConstraints()->height.SameAs(this, wxHeight);

  m_infoPanel->SetProfile(m_grid->GetProfile());

  m_grid->SetEditable(FALSE);
  m_grid->Refresh();

  NFSupport *support = new NFSupport(*p_nfg);
  support->SetName("Full Support");
  AddSupport(support);
  SetCurrentSupport(1);
}

wxWindow *guiNfgView::GridWindow(void) const
{ return m_grid; }

wxWindow *guiNfgView::InfoPanel(void) const
{ return m_infoPanel; }

void guiNfgView::ShowWindows(bool p_show)
{
  m_infoPanel->Show(p_show);
  m_grid->Show(p_show);
}
		 

void guiNfgView::AddSupport(NFSupport *p_support)
{
  m_supports.Append(p_support);
  m_infoPanel->UpdateSupports(m_supports);
  SetCurrentSupport(m_supports.Length());
}

void guiNfgView::SetCurrentSupport(int p_index)
{
  m_currentSupport = p_index;
  m_infoPanel->SetSupport(m_supports[p_index]);
  m_grid->SetSupport(m_supports[p_index]);
}

void guiNfgView::SetShowProbs(bool p_show)
{
  m_grid->SetShowProbs(p_show);
}

void guiNfgView::OnSize(wxSizeEvent &p_event)
{
  int width, height;
  GetClientSize(&width, &height);
  m_infoPanel->SetSize(0, 0, 300, height);
  m_grid->SetSize(300, 0, width - 300, height);
}

void guiNfgView::SetPlayers(int p_rowPlayer, int p_colPlayer)
{
  m_grid->SetPlayers(p_rowPlayer, p_colPlayer);
}

void guiNfgView::SetProfile(const gArray<int> &p_profile)
{
  m_infoPanel->SetProfile(p_profile);
  m_grid->SetProfile(p_profile);
}

void guiNfgView::SetOutcome(NFOutcome *p_outcome)
{
  m_nfg->SetOutcome(m_grid->GetProfile(), p_outcome);
  m_infoPanel->SetOutcome(p_outcome);
  m_grid->SetOutcome(p_outcome);
}

void guiNfgView::SetPayoffs(void)
{
  NFOutcome *outcome = m_nfg->GetOutcome(m_grid->GetProfile());

  if (outcome) {
    for (int pl = 1; pl <= m_nfg->NumPlayers(); pl++) {
      m_nfg->SetPayoff(outcome, pl, m_infoPanel->GetPayoff(pl));
    }
  }
}

void guiNfgView::EditCurrentSupport(const NFSupport &p_support)
{
  *m_supports[m_currentSupport] = p_support;
  m_grid->SetSupport(m_supports[m_currentSupport]);
  m_infoPanel->UpdateSupports(m_supports);
  m_infoPanel->SetSupport(m_supports[m_currentSupport]);
}
  
void guiNfgView::DeleteSupport(int p_index)
{
  if (m_currentSupport == p_index) {
    m_currentSupport = 1;
  }
  else if (m_currentSupport > p_index) {
    m_currentSupport -= 1;
  }

  delete m_supports.Remove(p_index);
  m_grid->SetSupport(m_supports[m_currentSupport]);
  m_infoPanel->UpdateSupports(m_supports);
  m_infoPanel->SetSupport(m_supports[m_currentSupport]);
}

void guiNfgView::SetSolution(const MixedSolution &p_solution)
{
  m_grid->SetSolution(p_solution);
}

int guiNfgView::RowPlayer(void) const { return m_grid->RowPlayer(); }
int guiNfgView::ColPlayer(void) const { return m_grid->ColPlayer(); }

gText guiNfgView::UniqueSupportName(void) const
{
  int number = m_supports.Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= m_supports.Length(); i++) {
      if (m_supports[i]->GetName() == "Support" + ToText(number)) {
	break;
      }
    }

    if (i > m_supports.Length())
      return "Support" + ToText(number);
    
    number++;
  }
}
