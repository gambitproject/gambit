//
// FILE: nfgprofile.cc -- Implementation of normal form profile list
//
// $Id$
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
  EVT_MENU(NFG_PROFILES_FILTER, NfgProfileList::OnSortFilter)
  EVT_RIGHT_DOWN(NfgProfileList::OnRightClick)
END_EVENT_TABLE()

NfgProfileList::NfgProfileList(NfgShow *p_nfgShow, wxWindow *p_parent)
  : wxListCtrl(p_parent, idNFG_SOLUTION_LIST, wxDefaultPosition,
	       wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL),
    m_parent(p_nfgShow)
{
  m_menu = new wxMenu("Profiles");
  m_menu->Append(NFG_PROFILES_FILTER, "Sort/Filter...",
		 "Sort and filter profiles");
  m_menu->Append(NFG_PROFILES_NEW, "New", "Create a new profile");
  m_menu->Append(NFG_PROFILES_CLONE, "Clone", "Clone this profile");
  m_menu->Append(NFG_PROFILES_RENAME, "Rename", "Rename this profile");
  m_menu->Append(NFG_PROFILES_EDIT, "Edit", "Edit this profile");
  m_menu->Append(NFG_PROFILES_DELETE, "Delete", "Delete this profile");

  InsertColumn(0, "Name");
  InsertColumn(1, "Creator");
  InsertColumn(2, "Nash");
  InsertColumn(3, "Perfect");
  InsertColumn(4, "Liap Value");
  InsertColumn(5, "Qre Lambda");

  UpdateValues();
}

NfgProfileList::~NfgProfileList()
{ }

void NfgProfileList::UpdateValues(void)
{
  DeleteAllItems();
  for (int i = 1; i <= Length(); i++) {
    const MixedSolution &profile = (*this)[i];
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
  }

  if (Length() > 0) {
    wxListItem item;
    item.m_mask = wxLIST_MASK_STATE;
    item.m_itemId = m_parent->CurrentProfile() - 1;
    item.m_state = wxLIST_STATE_SELECTED;
    item.m_stateMask = wxLIST_STATE_SELECTED;
    SetItem(item);
  }
}

int NfgProfileList::Append(const MixedSolution &p_solution)
{
  int number = Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= Length(); i++) {
      if ((*this)[i].GetName() == "Profile" + ToText(number)) {
	break;
      }
    }

    if (i > Length()) {
      break;
    }
    
    number++;
  }

  (*this)[gList<MixedSolution>::Append(p_solution)].SetName("Profile" + ToText(number));
  return Length();
}

void NfgProfileList::OnRightClick(wxMouseEvent &p_event)
{
  m_menu->Enable(NFG_PROFILES_CLONE, m_parent->CurrentProfile() > 0);
  m_menu->Enable(NFG_PROFILES_RENAME, m_parent->CurrentProfile() > 0);
  m_menu->Enable(NFG_PROFILES_EDIT, m_parent->CurrentProfile() > 0);
  m_menu->Enable(NFG_PROFILES_DELETE, m_parent->CurrentProfile() > 0);
  PopupMenu(m_menu, p_event.m_x, p_event.m_y);
}

void NfgProfileList::OnSortFilter(wxCommandEvent &)
{
#ifdef UNUSED
  dialogMixedFilter dialog(this, m_options);

  if (dialog.ShowModal() == wxID_OK) {
    dialog.Update(m_options);

    if (VisibleLength() > 0) {
      MixedSolution &currentSolution = (*this)[m_parent->CurrentSolution()];
      m_options.Filter(*this);
      m_options.Sort(*this);
      UpdateValues();
      if (this->Find(currentSolution) <= VisibleLength()) {
	m_parent->ChangeSolution(this->Find(currentSolution));
      }
      else {
	m_parent->ChangeSolution(1);
      }
    }
    else {
      m_options.Filter(*this);
      m_options.Sort(*this);
      UpdateValues();
      m_parent->ChangeSolution((VisibleLength() > 0) ? 1 : 0);
    }
  }
#endif  // UNUSED
}
