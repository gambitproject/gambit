//
// FILE: gameview.cc -- Implemetation of game view child frame
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
#include "gameview.h"

#include "efgtree.h"
#include "efginfopanel.h"
#include "nfggrid.h"
#include "nfginfopanel.h"

#include "dialogauto.h"
#include "dialogstrategies.h"
#include "dialogefgoutcome.h"
#include "dialogelim.h"
#include "dialogsupportselect.h"
#include "dialognfgeditsupport.h"
#include "dialognfgstandard.h"
#include "dialogefgstandard.h"

#include "guistatus.h"

BEGIN_EVENT_TABLE(gambitGameView, wxMDIChildFrame)
  EVT_MENU(GAMBIT_CLOSE, gambitGameView::OnClose)
  EVT_MENU(GAME_EDIT_LABEL, gambitGameView::OnEditLabel)
END_EVENT_TABLE()

gambitGameView::gambitGameView(wxMDIParentFrame *p_parent, Nfg *p_nfg,
			       const wxPoint &p_position, const wxSize &p_size)
  : wxMDIChildFrame(p_parent, -1, "",
		    p_position, p_size, wxDEFAULT_FRAME_STYLE)
{
  SetTitle((char *) p_nfg->GetTitle());
  
  m_infoSplitter = new wxSplitterWindow(this, -1,
					wxPoint(0, 0), p_size);
  m_solutionSplitter = new wxSplitterWindow(m_infoSplitter, -1);
					    
  //  m_solutionGrid = new guiNfgSolutions(this, m_solutionSplitter,
  //				       *p_nfg, MixedProfiles());
  // m_solutionGrid->Show(FALSE);
}

gambitGameView::gambitGameView(wxMDIParentFrame *p_parent, Efg *p_efg,
			       const wxPoint &p_position, const wxSize &p_size)
  : wxMDIChildFrame(p_parent, -1, "", 
		    p_position, p_size, wxDEFAULT_FRAME_STYLE)
{
  SetTitle((char *) p_efg->GetTitle());

  m_infoSplitter = new wxSplitterWindow(this, -1,
					wxPoint(0, 0), p_size);
  m_solutionSplitter = new wxSplitterWindow(m_infoSplitter, -1);
					    
}

gambitGameView::~gambitGameView(void)
{
}

void gambitGameView::OnEditLabel(wxCommandEvent &)
{
  /*
  wxTextEntryDialog dialog(this, "New game label", "Game label", GetTitle());
  if (dialog.ShowModal() == wxID_OK) {
    if (GetNfg()) {
      GetNfg()->SetTitle(dialog.GetValue().c_str());
    }
    if (GetEfg()) {
      GetEfg()->SetTitle(dialog.GetValue().c_str());
    }
    SetTitle(dialog.GetValue());
  }
  */
}

void gambitGameView::OnEditPlayers(wxCommandEvent &)
{
  /*
  for (int pl = 1; pl <= GetNfg()->NumPlayers(); pl++) {
    GetNfg()->Players()[pl]->SetName(m_infoPanel->GetPlayerName(pl));
  }

  dialogStrategies dialog(this, *GetNfg());
  dialog.ShowModal();
  m_grid->OnChangeLabels();
  m_grid->Refresh();
  m_infoPanel->SetLabels();
  */
}

void gambitGameView::OnClose(wxCommandEvent &)
{
  Close(TRUE);
}

void gambitGameView::OnActivate(wxActivateEvent& event)
{
  //  m_grid->SetFocus();
}

//========================================================================
//                     guiAutoDialog: Member functions
//========================================================================

guiAutoDialog::guiAutoDialog(wxWindow *p_parent, char *p_title)
  : wxDialog(p_parent, -1, p_title)
{
  SetAutoLayout(TRUE);

  m_okButton = new wxButton(this, wxID_OK, "OK");
  m_okButton->SetConstraints(new wxLayoutConstraints);

  m_cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");
  m_cancelButton->SetConstraints(new wxLayoutConstraints);

  m_helpButton = new wxButton(this, 100, "Help");
  m_helpButton->SetConstraints(new wxLayoutConstraints);
}

void guiAutoDialog::AutoSize(void)
{
  Layout();
  Fit();

  int minX = 1000, minY = 1000, totalWidth = 0, totalHeight = 0;

  for (wxNode *child = GetChildren().First(); child; child = child->Next()) {
    wxWindow *data = (wxWindow *) child->Data();
    int x, y, width, height;
    data->GetPosition(&x, &y);
    data->GetSize(&width, &height);

    minX = gmin(minX, x);
    minY = gmin(minY, y);
    totalWidth = gmax(totalWidth, x + width);
    totalHeight = gmax(totalHeight, y + height);
  }

  SetClientSize(totalWidth + 10, totalHeight + 10);
}

void gambitGameView::SetSolution(int p_solution)
{
  /*
  m_grid->UpdateTable();
  m_grid->Refresh();
  */
}



#include "garray.imp"

template class gArray<wxChoice *>;
template class gArray<wxTextCtrl *>;

#include "glist.imp"

template class gList<NFSupport *>;
