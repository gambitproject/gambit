//
// $Source$
// $Revision$
// $Date$
//
// DESCRIPTION:
// Implementation of normal form support palette window
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "nfgsupport.h"

const int idSUPPORTLISTCHOICE = 8000;
const int idSUPPORTPREVBUTTON = 8001;
const int idSUPPORTNEXTBUTTON = 8002;
const int idSTRATEGYTREE = 8003;

BEGIN_EVENT_TABLE(NfgSupportWindow, wxPanel)
  EVT_CHOICE(idSUPPORTLISTCHOICE, NfgSupportWindow::OnSupportList)
  EVT_BUTTON(idSUPPORTPREVBUTTON, NfgSupportWindow::OnSupportPrev)
  EVT_BUTTON(idSUPPORTNEXTBUTTON, NfgSupportWindow::OnSupportNext)
  EVT_TREE_ITEM_COLLAPSING(idSTRATEGYTREE, NfgSupportWindow::OnTreeItemCollapse)
END_EVENT_TABLE()

NfgSupportWindow::NfgSupportWindow(NfgShow *p_nfgShow, wxWindow *p_parent)
  : wxPanel(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_parent(p_nfgShow), m_map((Strategy *) 0)
{
  SetAutoLayout(true);

  m_supportList = new wxChoice(this, idSUPPORTLISTCHOICE,
			       wxDefaultPosition, wxDefaultSize,
			       0, 0);
  m_prevButton = new wxButton(this, idSUPPORTPREVBUTTON, "<-",
			      wxDefaultPosition, wxSize(30, 30));
  m_nextButton = new wxButton(this, idSUPPORTNEXTBUTTON, "->",
			      wxDefaultPosition, wxSize(30, 30));
  m_strategyTree = new wxTreeCtrl(this, idSTRATEGYTREE,
				  wxDefaultPosition, wxDefaultSize);
  UpdateValues();

  wxBoxSizer *selectSizer = new wxBoxSizer(wxHORIZONTAL);
  selectSizer->Add(m_prevButton, 0, wxALL, 5);
  selectSizer->Add(m_supportList, 1, wxALL | wxEXPAND, 5);
  selectSizer->Add(m_nextButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(selectSizer, 0, wxEXPAND, 5);
  topSizer->Add(m_strategyTree, 1, wxEXPAND, 5);
  
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);

  Layout();

  Show(true);
}

void NfgSupportWindow::UpdateValues(void)
{
  m_supportList->Clear();

  const gList<NFSupport *> &supports = m_parent->Supports();

  for (int i = 1; i <= supports.Length(); i++) {
    m_supportList->Append((char *)
			  (ToText(i) + ": " + supports[i]->GetName()));
  }

  int supportIndex = supports.Find(m_parent->GetSupport());
  m_supportList->SetSelection(supportIndex - 1);
  m_prevButton->Enable((supportIndex > 1) ? true : false);
  m_nextButton->Enable((supportIndex < supports.Length()) ? true : false);

  m_strategyTree->DeleteAllItems();

  m_strategyTree->AddRoot((char *) m_parent->GetSupport()->GetName());
  for (int pl = 1; pl <= m_parent->Game().NumPlayers(); pl++) {
    NFPlayer *player = m_parent->Game().Players()[pl];

    wxTreeItemId id = m_strategyTree->AppendItem(m_strategyTree->GetRootItem(),
					       (char *) player->GetName());
    
    for (int st = 1; st <= m_parent->GetSupport()->NumStrats(pl); st++) {
      Strategy *strategy = m_parent->GetSupport()->GetStrategy(pl, st);

      wxTreeItemId stratID = m_strategyTree->AppendItem(id, 
						       (char *) strategy->Name());
      m_map.Define(stratID, strategy);
      m_strategyTree->Expand(stratID);
    }
    m_strategyTree->Expand(id);
  }
  m_strategyTree->Expand(m_strategyTree->GetRootItem());

}

void NfgSupportWindow::OnSupportList(wxCommandEvent &p_event)
{
  m_parent->SetSupportNumber(p_event.GetSelection() + 1);
}

void NfgSupportWindow::OnSupportPrev(wxCommandEvent &)
{
  m_parent->SetSupportNumber(m_supportList->GetSelection());
}

void NfgSupportWindow::OnSupportNext(wxCommandEvent &)
{
  m_parent->SetSupportNumber(m_supportList->GetSelection() + 2);
}

void NfgSupportWindow::OnTreeItemCollapse(wxTreeEvent &p_event)
{
  if (p_event.GetItem() == m_strategyTree->GetRootItem()) {
    p_event.Veto();
  }
}

#include "base/gmap.imp"

static gOutput &operator<<(gOutput &p_output, wxTreeItemId)
{ return p_output; }

template class gBaseMap<wxTreeItemId, Strategy *>;
template class gOrdMap<wxTreeItemId, Strategy *>;
