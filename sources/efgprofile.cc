//
// FILE: efgprofile.cc -- Implementation of extensive form profile list
//
// $Id$
//

#include "wx/wx.h"
#include "efgprofile.h"

#include "legendc.h"
#include "treewin.h"

//-------------------------------------------------------------------------
//                  class EfgProfileList: Member functions
//-------------------------------------------------------------------------

BEGIN_EVENT_TABLE(EfgProfileList, wxListCtrl)
  EVT_MENU(efgmenuPROFILES_FILTER, EfgProfileList::OnSortFilter)
  EVT_RIGHT_DOWN(EfgProfileList::OnRightClick)
  EVT_LIST_COL_CLICK(idEFG_SOLUTION_LIST, EfgProfileList::OnColumnClick)
END_EVENT_TABLE()

EfgProfileList::EfgProfileList(EfgShow *p_efgShow, wxWindow *p_parent)
  : wxListCtrl(p_parent, idEFG_SOLUTION_LIST, wxDefaultPosition,
	       wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL),
    m_parent(p_efgShow)
{
  m_menu = new wxMenu("Solutions");
  m_menu->Append(efgmenuPROFILES_FILTER, "Sort/Filter...",
		 "Sort and filter profiles");
  m_menu->Append(efgmenuPROFILES_NEW, "New", "Create a new profile");
  m_menu->Append(efgmenuPROFILES_EDIT, "Edit...", "Edit this profile");
  m_menu->Append(efgmenuPROFILES_DELETE, "Delete", "Delete this profile");

  InsertColumn(0, "Name");
  InsertColumn(1, "Creator");
  InsertColumn(2, "Nash");
  InsertColumn(3, "Perfect");
  InsertColumn(4, "Sequential");
  InsertColumn(5, "Liap Value");
  InsertColumn(6, "Qre Lambda");
  InsertColumn(7, "Qre Value");

  UpdateValues();
}

EfgProfileList::~EfgProfileList()
{ }

void EfgProfileList::UpdateValues(void)
{
  DeleteAllItems();
  for (int i = 1; i <= VisibleLength(); i++) {
    const BehavSolution &solution = (*this)[i];
    InsertItem(i - 1, (char *) solution.GetName());
    SetItem(i - 1, 1, (char *) ToText(solution.Creator()));
    SetItem(i - 1, 2, (char *) ToText(solution.IsNash()));
    SetItem(i - 1, 3, (char *) ToText(solution.IsSubgamePerfect()));
    SetItem(i - 1, 4, (char *) ToText(solution.IsSequential()));
    SetItem(i - 1, 5, (char *) ToText(solution.LiapValue()));
    if (solution.Creator() == algorithmEfg_QRE_EFG ||
	solution.Creator() == algorithmEfg_QRE_NFG) {
      SetItem(i - 1, 6, (char *) ToText(solution.QreLambda()));
      SetItem(i - 1, 7, (char *) ToText(solution.QreValue()));
    }
    else {
      SetItem(i - 1, 6, "N/A");
      SetItem(i - 1, 7, "N/A");
    }
  }

  if (VisibleLength() > 0) {
    wxListItem item;
    item.m_mask = wxLIST_MASK_STATE;
    item.m_itemId = m_parent->CurrentSolution() - 1;
    item.m_state = wxLIST_STATE_SELECTED;
    item.m_stateMask = wxLIST_STATE_SELECTED;
    SetItem(item);
  }
}

int EfgProfileList::Append(const BehavSolution &p_solution)  
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

  (*this)[gSortList<BehavSolution>::Append(p_solution)].SetName("Profile" + ToText(number));
  return Length();
}

void EfgProfileList::OnRightClick(wxMouseEvent &p_event)
{
  m_menu->Enable(efgmenuPROFILES_EDIT, m_parent->CurrentSolution() > 0);
  m_menu->Enable(efgmenuPROFILES_DELETE, m_parent->CurrentSolution() > 0);
  PopupMenu(m_menu, p_event.m_x, p_event.m_y);
}

void EfgProfileList::OnSortFilter(wxCommandEvent &)
{
  dialogBehavFilter dialog(this, m_options);

  if (dialog.ShowModal() == wxID_OK) {
    dialog.Update(m_options);

    if (VisibleLength() > 0) {
      BehavSolution &currentSolution = (*this)[m_parent->CurrentSolution()];
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
}

void EfgProfileList::OnColumnClick(wxListEvent &p_event)
{
  switch (p_event.m_col) {
  case 0:
    m_options.SortBy() = BSORT_BY_NAME;
    break;
  case 1:
    m_options.SortBy() = BSORT_BY_CREATOR;
    break;
  case 2:
    m_options.SortBy() = BSORT_BY_NASH;
    break;
  case 3:
    m_options.SortBy() = BSORT_BY_PERFECT;
    break;
  case 4:
    m_options.SortBy() = BSORT_BY_SEQ;
    break;
  case 5:
    m_options.SortBy() = BSORT_BY_LVALUE;
    break;
  case 6:
    m_options.SortBy() = BSORT_BY_GLAMBDA;
    break;
  case 7:
    m_options.SortBy() = BSORT_BY_GVALUE;
    break;
  default:
    break;
  }

  if (VisibleLength() > 0) {
    BehavSolution &currentSolution = (*this)[m_parent->CurrentSolution()];
    m_options.Sort(*this);
    UpdateValues();
    m_parent->ChangeSolution(this->Find(currentSolution));
  }
}


