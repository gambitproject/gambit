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

const int idSTRATEGYTREE = 8003;

class widgetStrategyTree : public wxTreeCtrl {
private:
  NfgSupportWindow *m_parent;
  wxMenu *m_menu;

  void OnRightClick(wxTreeEvent &);
  void OnMiddleClick(wxTreeEvent &);
  void OnKeypress(wxTreeEvent &);

public:
  widgetStrategyTree(NfgSupportWindow *p_parent);

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(widgetStrategyTree, wxTreeCtrl)
  EVT_TREE_KEY_DOWN(idSTRATEGYTREE, widgetStrategyTree::OnKeypress)
  EVT_TREE_ITEM_MIDDLE_CLICK(idSTRATEGYTREE, widgetStrategyTree::OnMiddleClick)
  EVT_TREE_ITEM_RIGHT_CLICK(idSTRATEGYTREE, widgetStrategyTree::OnRightClick)
END_EVENT_TABLE()

widgetStrategyTree::widgetStrategyTree(NfgSupportWindow *p_parent)
  : wxTreeCtrl(p_parent, idSTRATEGYTREE), m_parent(p_parent)
{ 
  m_menu = new wxMenu;
  m_menu->Append(NFG_SUPPORT_DUPLICATE, "Duplicate support",
		 "Duplicate this support");
  m_menu->Append(NFG_SUPPORT_DELETE, "Delete support",
		 "Delete this support");
}

void widgetStrategyTree::OnRightClick(wxTreeEvent &p_event)
{
  // Cannot delete the "full support"
  m_menu->Enable(NFG_SUPPORT_DELETE, (m_parent->GetSupport() > 0));
  PopupMenu(m_menu, p_event.GetPoint());
}

void widgetStrategyTree::OnKeypress(wxTreeEvent &p_event)
{
  if (m_parent->GetSupport() == 0) {
    return;
  }
  if (p_event.GetCode() == WXK_SPACE) {
    m_parent->ToggleItem(GetSelection());
  }
}

void widgetStrategyTree::OnMiddleClick(wxTreeEvent &p_event)
{
  if (m_parent->GetSupport() == 0) {
    return;
  }
  m_parent->ToggleItem(p_event.GetItem());
}



const int idSUPPORTLISTCHOICE = 8000;
const int idSUPPORTPREVBUTTON = 8001;
const int idSUPPORTNEXTBUTTON = 8002;

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
  m_strategyTree = new widgetStrategyTree(this);
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
    
    for (int st = 1; st <= m_parent->Game().NumStrats(pl); st++) {
      Strategy *strategy = m_parent->Game().Strategies(pl)[st];

      wxTreeItemId stratID = m_strategyTree->AppendItem(id, 
						       (char *) strategy->Name());
      if (m_parent->GetSupport()->Find(strategy)) {
	m_strategyTree->SetItemTextColour(stratID, *wxBLACK);
      }
      else {
	m_strategyTree->SetItemTextColour(stratID, *wxLIGHT_GREY);
      }
      m_map.Define(stratID, strategy);
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

void NfgSupportWindow::ToggleItem(wxTreeItemId p_id)
{
  Strategy *strategy = m_map(p_id);
  if (!strategy) {
    return;
  }

  if (m_parent->GetSupport()->Find(strategy) &&
      m_parent->GetSupport()->NumStrats(strategy->Player()) > 1) {
    m_parent->GetSupport()->RemoveStrategy(strategy);
    m_strategyTree->SetItemTextColour(p_id, *wxLIGHT_GREY);
  }
  else {
    m_parent->GetSupport()->AddStrategy(strategy);
    m_strategyTree->SetItemTextColour(p_id, *wxBLACK);
  }

  m_parent->SetSupportNumber(m_supportList->GetSelection() + 1);
}

#include "base/gmap.imp"

static gOutput &operator<<(gOutput &p_output, wxTreeItemId)
{ return p_output; }

template class gBaseMap<wxTreeItemId, Strategy *>;
template class gOrdMap<wxTreeItemId, Strategy *>;
