//
// FILE: efgprofile.cc -- Implementation of extensive form profile list
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "efgprofile.h"

#include "legendc.h"
#include "treewin.h"

//-------------------------------------------------------------------------
//                  class EfgProfileList: Member functions
//-------------------------------------------------------------------------

BEGIN_EVENT_TABLE(EfgProfileList, wxGrid)
  EVT_MENU(efgmenuPROFILES_FILTER, EfgProfileList::OnSortFilter)
  EVT_GRID_CELL_LEFT_CLICK(EfgProfileList::OnLeftClick)
  EVT_GRID_CELL_RIGHT_CLICK(EfgProfileList::OnRightClick)
  EVT_GRID_LABEL_LEFT_CLICK(EfgProfileList::OnColumnClick)
  EVT_GRID_LABEL_LEFT_DCLICK(EfgProfileList::OnColumnDoubleClick)
END_EVENT_TABLE()

EfgProfileList::EfgProfileList(EfgShow *p_efgShow, wxWindow *p_parent)
  : wxGrid(p_parent, idEFG_SOLUTION_LIST, wxDefaultPosition,
	   wxDefaultSize),
    m_parent(p_efgShow)
{
  m_menu = new wxMenu("Profiles");
  m_menu->Append(efgmenuPROFILES_FILTER, "Sort/Filter...",
		 "Sort and filter profiles");
  m_menu->Append(efgmenuPROFILES_NEW, "New", "Create a new profile");
  m_menu->Append(efgmenuPROFILES_CLONE, "Clone", "Clone this profile");
  m_menu->Append(efgmenuPROFILES_RENAME, "Rename", "Rename this profile");
  m_menu->Append(efgmenuPROFILES_EDIT, "Edit", "Edit this profile");
  m_menu->Append(efgmenuPROFILES_DELETE, "Delete", "Delete this profile");

  CreateGrid(0, 8);
  SetLabelValue(wxHORIZONTAL, "Name", 0);
  SetLabelValue(wxHORIZONTAL, "Creator", 1);
  SetLabelValue(wxHORIZONTAL, "Nash", 2);
  SetLabelValue(wxHORIZONTAL, "Perfect", 3);
  SetLabelValue(wxHORIZONTAL, "Sequential", 4);
  SetLabelValue(wxHORIZONTAL, "Liap Value", 5);
  SetLabelValue(wxHORIZONTAL, "Qre Lambda", 6);
  SetLabelValue(wxHORIZONTAL, "Qre Value", 7);

  EnableGridLines(false);
  SetLabelSize(wxVERTICAL, 0);
  SetDefaultCellAlignment(wxCENTER, wxCENTER);
  DisableDragRowSize();
  SetColLabelAlignment(wxCENTER, wxCENTER);
  SetEditable(false);

  wxScreenDC dc;
  dc.SetFont(GetLabelFont());
  wxCoord w, h, descent;
  dc.GetTextExtent("Sequential", &w, &h, &descent);
  SetLabelSize(wxHORIZONTAL, (int) ((h + descent) * 1.25));

  UpdateValues();
}

EfgProfileList::~EfgProfileList()
{ }

void EfgProfileList::UpdateValues(void)
{
  if (GetRows() > 0) {
    DeleteRows(0, GetRows());
  }
  if (!m_options.FilterNash()[1] || !m_options.FilterNash()[2] ||
      !m_options.FilterNash()[3]) {
    SetCellTextColour(*wxGREEN, -1, 2);
  }
  else {
    SetCellTextColour(*wxBLACK, -1, 2);
  }

  for (int i = 1; i <= m_displayOrder.Length(); i++) {
    const BehavSolution &solution = (*this)[m_displayOrder[i]];
    AppendRows();
    SetCellValue(i - 1, 0, (char *) solution.GetName());
    SetCellValue(i - 1, 1, (char *) ToText(solution.Creator()));
    SetCellValue(i - 1, 2, (char *) ToText(solution.IsNash()));
    SetCellValue(i - 1, 3, (char *) ToText(solution.IsSubgamePerfect()));
    SetCellValue(i - 1, 4, (char *) ToText(solution.IsSequential()));
    SetCellValue(i - 1, 5, (char *) ToText(solution.LiapValue()));
    if (solution.Creator() == algorithmEfg_QRE_EFG ||
	solution.Creator() == algorithmEfg_QRE_NFG) {
      SetCellValue(i - 1, 6, (char *) ToText(solution.QreLambda()));
      SetCellValue(i - 1, 7, (char *) ToText(solution.QreValue()));
    }
    else {
      SetCellValue(i - 1, 6, "N/A");
      SetCellValue(i - 1, 7, "N/A");
    }
    if (m_options.SortBy() != BSORT_NONE) {
      SetCellBackgroundColour(i - 1, m_options.SortBy() - 1, *wxCYAN);
    }
  }

  for (int i = 1; i <= m_displayOrder.Length(); i++) {
    if (m_displayOrder[i] == m_parent->CurrentProfile()) {
      for (int j = 0; j < GetCols(); j++) {
	SetCellTextColour(i - 1, j, *wxRED);
      }
    }
  }
}

void EfgProfileList::Resort(void)
{
  m_displayOrder = gBlock<int>(0);
  for (int i = 1; i <= Length(); i++) {
    bool passes = true;
    const BehavSolution &sol = (*this)[i];
    passes = (passes &&
	      ((sol.IsNash() == triTRUE && m_options.FilterNash()[1]) ||
	       (sol.IsNash() == triFALSE && m_options.FilterNash()[2]) ||
	       (sol.IsNash() == triUNKNOWN && m_options.FilterNash()[3])));
    if (passes) {
      m_displayOrder.Append(i);
    }
  }

  bool changed;

  do { 
    changed = false;
    
    for (int i = 1; i <= m_displayOrder.Length() - 1; i++) {
      bool outoforder = false;
      const BehavSolution &sol1 = (*this)[m_displayOrder[i]];
      const BehavSolution &sol2 = (*this)[m_displayOrder[i+1]];
      outoforder |= (m_options.SortBy() == BSORT_BY_NAME &&
		     (sol1.GetName() > sol2.GetName()));
      outoforder |= (m_options.SortBy() == BSORT_BY_CREATOR &&
		     (ToText(sol1.Creator()) > ToText(sol2.Creator())));
      outoforder |= (m_options.SortBy() == BSORT_BY_NASH &&
		     (sol1.IsNash() < sol2.IsNash()));
      outoforder |= (m_options.SortBy() == BSORT_BY_PERFECT &&
		     (sol1.IsSubgamePerfect() < sol2.IsSubgamePerfect()));
      outoforder |= (m_options.SortBy() == BSORT_BY_SEQ &&
		     (sol1.IsSequential() < sol2.IsSequential()));
      outoforder |= (m_options.SortBy() == BSORT_BY_LVALUE &&
		     (sol1.LiapValue() > sol2.LiapValue()));
      outoforder |= (m_options.SortBy() == BSORT_BY_GLAMBDA &&
		     (sol1.QreLambda() > sol2.QreLambda()));
      outoforder |= (m_options.SortBy() == BSORT_BY_GVALUE &&
		     (sol1.QreValue() > sol2.QreValue()));
      if (outoforder) {
	int foo = m_displayOrder[i];
	m_displayOrder[i] = m_displayOrder[i+1];
	m_displayOrder[i+1] = foo;
	changed = true;
      }
    }
  } while (changed);

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

  (*this)[gList<BehavSolution>::Append(p_solution)].SetName("Profile" + ToText(number));
  Resort();
  return Length();
}

void EfgProfileList::OnLeftClick(wxGridEvent &p_event)
{
  m_parent->ChangeProfile(m_displayOrder[p_event.GetRow() + 1]);
  p_event.Veto();
}

void EfgProfileList::OnRightClick(wxGridEvent &p_event)
{
  m_menu->Enable(efgmenuPROFILES_CLONE, m_parent->CurrentProfile() > 0);
  m_menu->Enable(efgmenuPROFILES_RENAME, m_parent->CurrentProfile() > 0);
  m_menu->Enable(efgmenuPROFILES_EDIT, m_parent->CurrentProfile() > 0);
  m_menu->Enable(efgmenuPROFILES_DELETE, m_parent->CurrentProfile() > 0);
  PopupMenu(m_menu, p_event.GetPosition().x, p_event.GetPosition().y);
}

void EfgProfileList::OnSortFilter(wxCommandEvent &)
{
  dialogBehavFilter dialog(this, m_options);

  if (dialog.ShowModal() == wxID_OK) {
    dialog.Update(m_options);
    Resort();
    UpdateValues();
  }
}

void EfgProfileList::OnColumnClick(wxGridEvent &p_event)
{
  p_event.Veto();
}

void EfgProfileList::OnColumnDoubleClick(wxGridEvent &p_event)
{
  if (p_event.GetCol() + 1 == m_options.SortBy()) {
    m_options.SortBy() = BSORT_NONE;
  }
  else {
    m_options.SortBy() = p_event.GetCol() + 1;
  }

  Resort();
  UpdateValues();

#ifdef UNUSED
  if (VisibleLength() > 0) {
    BehavSolution &currentSolution = (*this)[m_parent->CurrentSolution()];
    m_options.Sort(*this);
    UpdateValues();
    m_parent->ChangeSolution(this->Find(currentSolution));
  }
#endif  // UNUSED
}


