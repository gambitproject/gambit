//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of extensive form support palette window
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "efgsupport.h"

const int idSUPPORTLISTCHOICE = 8000;
const int idSUPPORTPREVBUTTON = 8001;
const int idSUPPORTNEXTBUTTON = 8002;
const int idACTIONTREE = 8003;

BEGIN_EVENT_TABLE(EfgSupportWindow, wxPanel)
  EVT_CHOICE(idSUPPORTLISTCHOICE, EfgSupportWindow::OnSupportList)
  EVT_BUTTON(idSUPPORTPREVBUTTON, EfgSupportWindow::OnSupportPrev)
  EVT_BUTTON(idSUPPORTNEXTBUTTON, EfgSupportWindow::OnSupportNext)
  EVT_TREE_ITEM_COLLAPSING(idACTIONTREE, EfgSupportWindow::OnTreeItemCollapse)
END_EVENT_TABLE()

EfgSupportWindow::EfgSupportWindow(EfgShow *p_efgShow, wxWindow *p_parent)
  : wxPanel(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_parent(p_efgShow), m_map((Action *) 0)
{
  SetAutoLayout(true);

  m_supportList = new wxChoice(this, idSUPPORTLISTCHOICE,
			       wxDefaultPosition, wxDefaultSize,
			       0, 0);
  m_prevButton = new wxButton(this, idSUPPORTPREVBUTTON, "<-",
			      wxDefaultPosition, wxDefaultSize);
  m_nextButton = new wxButton(this, idSUPPORTNEXTBUTTON, "->",
			      wxDefaultPosition, wxDefaultSize);
  m_actionTree = new wxTreeCtrl(this, idACTIONTREE,
				wxDefaultPosition, wxDefaultSize);
  UpdateValues();

  wxBoxSizer *selectSizer = new wxBoxSizer(wxHORIZONTAL);
  selectSizer->Add(m_prevButton, 0, wxALL, 5);
  selectSizer->Add(m_supportList, 0, wxALL, 5);
  selectSizer->Add(m_nextButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(selectSizer, 0, wxCENTER, 5);
  topSizer->Add(m_actionTree, 1, wxEXPAND, 5);
  
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();

  Show(true);
}

void EfgSupportWindow::UpdateValues(void)
{
  m_supportList->Clear();

  const gList<EFSupport *> &supports = m_parent->Supports();

  for (int i = 1; i <= supports.Length(); i++) {
    m_supportList->Append((char *)
			  (ToText(i) + ": " + supports[i]->GetName()));
  }

  int supportIndex = supports.Find(m_parent->GetSupport());
  m_supportList->SetSelection(supportIndex - 1);
  m_prevButton->Enable((supportIndex > 1) ? true : false);
  m_nextButton->Enable((supportIndex < supports.Length()) ? true : false);

  m_actionTree->DeleteAllItems();

  m_actionTree->AddRoot((char *) m_parent->GetSupport()->GetName());
  for (int pl = 1; pl <= m_parent->Game()->NumPlayers(); pl++) {
    EFPlayer *player = m_parent->Game()->Players()[pl];

    wxTreeItemId id = m_actionTree->AppendItem(m_actionTree->GetRootItem(),
					       (char *) player->GetName());
    
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      Infoset *infoset = player->Infosets()[iset];
      wxTreeItemId isetID = m_actionTree->AppendItem(id, 
						     (char *) infoset->GetName());
      for (int act = 1; act <= infoset->NumActions(); act++) {
	Action *action = infoset->Actions()[act];
	wxTreeItemId actID = m_actionTree->AppendItem(isetID,
						      (char *) action->GetName());
	m_actionTree->SetItemBold(actID,
				  (m_parent->GetSupport()->Find(action)));
	m_map.Define(actID, infoset->Actions()[act]);
      }

      m_actionTree->Expand(isetID);
    }
    m_actionTree->Expand(id);
  }
  m_actionTree->Expand(m_actionTree->GetRootItem());

}

void EfgSupportWindow::OnSupportList(wxCommandEvent &p_event)
{
  m_parent->SetSupportNumber(p_event.GetSelection() + 1);
}

void EfgSupportWindow::OnSupportPrev(wxCommandEvent &)
{
  m_parent->SetSupportNumber(m_supportList->GetSelection());
}

void EfgSupportWindow::OnSupportNext(wxCommandEvent &)
{
  m_parent->SetSupportNumber(m_supportList->GetSelection() + 2);

}

void EfgSupportWindow::OnTreeItemCollapse(wxTreeEvent &p_event)
{
  if (p_event.GetItem() == m_actionTree->GetRootItem()) {
    p_event.Veto();
  }
}

#include "base/gmap.imp"

static gOutput &operator<<(gOutput &p_output, wxTreeItemId)
{ return p_output; }

template class gBaseMap<wxTreeItemId, Action *>;
template class gOrdMap<wxTreeItemId, Action *>;
