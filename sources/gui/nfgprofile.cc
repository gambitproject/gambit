//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Normal form mixed profile window
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "nfgprofile.h"
#include "nfgconst.h"

//-------------------------------------------------------------------------
//                  class NfgProfileList: Member functions
//-------------------------------------------------------------------------

BEGIN_EVENT_TABLE(NfgProfileList, wxListCtrl)
  EVT_RIGHT_DOWN(NfgProfileList::OnRightClick)
END_EVENT_TABLE()

NfgProfileList::NfgProfileList(NfgShow *p_nfgShow, wxWindow *p_parent)
  : wxListCtrl(p_parent, idNFG_SOLUTION_LIST, wxDefaultPosition,
	       wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL),
    m_parent(p_nfgShow)
{
  m_menu = new wxMenu("Profiles");
  m_menu->Append(NFG_PROFILES_NEW, "New", "Create a new profile");
  m_menu->Append(NFG_PROFILES_DUPLICATE, "Duplicate",
		 "Duplicate this profile");
  m_menu->Append(NFG_PROFILES_DELETE, "Delete", "Delete this profile");
  m_menu->Append(NFG_PROFILES_PROPERTIES, "Properties",
		 "View and edit properties of this profile");

  UpdateValues();
}

NfgProfileList::~NfgProfileList()
{ }

void NfgProfileList::UpdateValues(void)
{
  ClearAll();
  InsertColumn(0, "Name");
  InsertColumn(1, "Creator");
  InsertColumn(2, "Nash");
  InsertColumn(3, "Perfect");
  InsertColumn(4, "Liap Value");
  InsertColumn(5, "Qre Lambda");
  
  const Nfg &nfg = m_parent->Game();
  int maxColumn = 5;

  for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
    NFPlayer *player = nfg.Players()[pl];
    for (int st = 1; st <= player->NumStrats(); st++) {
      InsertColumn(++maxColumn,
		   wxString::Format("%d:%d", pl, st));
    }
  }

  for (int i = 1; i <= m_parent->Profiles().Length(); i++) {
    const MixedSolution &profile = m_parent->Profiles()[i];
    InsertItem(i - 1, (char *) profile.GetName());
    SetItem(i - 1, 1, (char *) ToText(profile.Creator()));
    SetItem(i - 1, 2, (char *) ToText(profile.IsNash()));
    SetItem(i - 1, 3, (char *) ToText(profile.IsPerfect()));
    SetItem(i - 1, 4, (char *) ToText(profile.LiapValue()));
    if (profile.Creator() == algorithmNfg_QRE) {
      SetItem(i - 1, 5, (char *) ToText(profile.QreLambda()));
    }
    else {
      SetItem(i - 1, 5, "--");
    }

    int column = 5;
    for (int pl = 1; pl <= nfg.NumPlayers(); pl++) {
      NFPlayer *player = nfg.Players()[pl];
      for (int st = 1; st <= player->NumStrats(); st++) {
	SetItem(i - 1, ++column,
		(char *) ToText(profile(player->Strategies()[st])));
      }
    }    
  }

  if (m_parent->Profiles().Length() > 0) {
    wxListItem item;
    item.m_mask = wxLIST_MASK_STATE;
    item.m_itemId = m_parent->CurrentProfile() - 1;
    item.m_state = wxLIST_STATE_SELECTED;
    item.m_stateMask = wxLIST_STATE_SELECTED;
    SetItem(item);
  }
}

void NfgProfileList::OnRightClick(wxMouseEvent &p_event)
{
  m_menu->Enable(NFG_PROFILES_DUPLICATE, m_parent->CurrentProfile() > 0);
  m_menu->Enable(NFG_PROFILES_DELETE, m_parent->CurrentProfile() > 0);
  m_menu->Enable(NFG_PROFILES_PROPERTIES, m_parent->CurrentProfile() > 0);
  PopupMenu(m_menu, p_event.m_x, p_event.m_y);
}

