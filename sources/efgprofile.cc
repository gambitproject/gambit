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
  EVT_RIGHT_DOWN(EfgProfileList::OnRightClick)
END_EVENT_TABLE()

EfgProfileList::EfgProfileList(EfgShow *p_efgShow, wxWindow *p_parent)
  : wxListCtrl(p_parent, idEFG_SOLUTION_LIST, wxDefaultPosition,
	       wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL),
    m_parent(p_efgShow)
{
  m_menu = new wxMenu("Solutions");
  m_menu->Append(efgmenuPROFILES_FILTER, "Filter...", "Filter profiles");
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
  for (int i = 1; i <= Length(); i++) {
    const BehavSolution &solution = (*this)[i];
    InsertItem(i - 1, 
	       (char *) ("Profile" + ToText((int) solution.Id())));
    SetItem(i - 1, 1, (char *) NameEfgAlgType(solution.Creator()));
    SetItem(i - 1, 2, (char *) Name(solution.IsNash()));
    SetItem(i - 1, 3, (char *) Name(solution.IsSubgamePerfect()));
    SetItem(i - 1, 4, (char *) Name(solution.IsSequential()));
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

  if (Length() > 0) {
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
  unsigned int number = Length() + 1;
  while (1) {
    int i;
    for (i = 1; i <= Length(); i++) {
      if ((*this)[i].Id() == number) {
	break;
      }
    }

    if (i > Length()) {
      break;
    }
    
    number++;
  }

  (*this)[gSortList<BehavSolution>::Append(p_solution)].SetId(number);
  return Length();
}

void EfgProfileList::OnRightClick(wxMouseEvent &p_event)
{
  m_menu->Enable(efgmenuPROFILES_EDIT, m_parent->CurrentSolution() > 0);
  m_menu->Enable(efgmenuPROFILES_DELETE, m_parent->CurrentSolution() > 0);
  PopupMenu(m_menu, p_event.m_x, p_event.m_y);
}

#ifdef NOT_PORTED_YET
void EfgProfileList::OnLabelLeftClick(int row, int col, int x, int y,
				   bool control, bool shift)
{
  if (row == -1) {
    int old_sort_by = m_options.SortBy();
    
    if (col == FeaturePos(BSOLN_CREATOR) && features[BSOLN_CREATOR])
      m_options.SortBy() = BSORT_BY_CREATOR;

    if (col == FeaturePos(BSOLN_ISNASH) && features[BSOLN_ISNASH])
      m_options.SortBy() = BSORT_BY_NASH;

    if (col == FeaturePos(BSOLN_ISPERF) && features[BSOLN_ISPERF])
      m_options.SortBy() = BSORT_BY_PERFECT;

    if (col == FeaturePos(BSOLN_ISSEQ) && features[BSOLN_ISSEQ])
      m_options.SortBy() = BSORT_BY_SEQ;

    if (col == FeaturePos(BSOLN_GLAMBDA) && features[BSOLN_GLAMBDA])
      m_options.SortBy() = BSORT_BY_GLAMBDA;

    if (col == FeaturePos(BSOLN_GVALUE) && features[BSOLN_GVALUE])
      m_options.SortBy() = BSORT_BY_GVALUE;

    if (col == FeaturePos(BSOLN_LVALUE) && features[BSOLN_LVALUE])
      m_options.SortBy() = BSORT_BY_LVALUE;

    if (old_sort_by != m_options.SortBy()) 
      SortFilter(false);
  }
}

void EfgProfileList::SortFilter(bool)
{
#ifdef NOT_PORTED_YET
  int old_num_sol = num_solutions;  // current state
  const BehavSolution *cur_solnp = 0;

  if (cur_soln) 
    cur_solnp = &solns[cur_soln];
    
  BSolnSorterFilter SF(solns, m_options); 
  int new_num_sol = solns.VisibleLength();    // new state
  int i, j;
  int new_soln = 0;                           // try to find the new pos of cur_soln

  for (i = 1; i <= solns.VisibleLength(); i++) {
    if (cur_solnp == &solns[i]) 
      new_soln = i;
  }

  int num_isets = m_parent->Game()->NumPlayerInfosets();

  if (old_num_sol > new_num_sol) {
    for (i = old_num_sol; i > new_num_sol; i--)
      for (j = 1; j <= num_isets; j++)
	DeleteRows();
  }
  
  if (old_num_sol < new_num_sol) {
    for (i = old_num_sol+1; i <= new_num_sol; i++)
      for (j = 1; j <= num_isets; j++)
	AppendRows();
  }

  num_solutions = solns.VisibleLength();
  UpdateValues();

  // make sure we do not try to access non-displayed solutions
  if (cur_soln > num_solutions) 
    cur_soln = 0;

  //  UpdateSoln(new_soln, -1);
  SetGridCursor(SolnPos(new_soln), 0);
#endif // NOT_PORTED_YET
}
#endif  // NOT_PORTED_YET

