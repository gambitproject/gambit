//
// FILE: efgsoln.cc -- all solution display related routines for the efg
//
// $Id$
//

#include "wx/wx.h"
#include "wxmisc.h"
#include "garray.h"
#include "rational.h"
#include "gmisc.h"
#include "efgsoln.h"
#include "treedraw.h"
#include "treewin.h"
#include "legendc.h" // needed for NODE_BELOW for iset_label

#ifdef NOT_PORTED_YET

class BehavSolnEdit : public SpreadSheet3D {
private:
  BehavSolution &soln;
  gPVector<int> dim;
  int num_isets;

public:
  BehavSolnEdit(BehavSolution &soln, int iset_disp, wxWindow *parent, int);

  void OnSelectedMoved(int row, int col, SpreadMoveDir how);
  void OnOk(void);
  bool OnClose(void);
};

#endif  // NOT_PORTED_YET

//****************************************************************************
//                   SORTER FILTER OPTIONS DIALOG
//****************************************************************************

class BSolnSortFilterDialog : public wxDialog {
private:
    BSolnSortFilterOptions &options;
    wxRadioBox *sortby;
    wxListBox *filter_creator, *filter_nash, *filter_perfect, *filter_seq;

    static void ok_func(wxButton &ob, wxEvent &)
    { ((BSolnSortFilterDialog *)ob.GetClientData())->OnOk(); }

    static void cancel_func(wxButton &ob, wxEvent &)
    { ((BSolnSortFilterDialog *)ob.GetClientData())->OnCancel(); }

    static void help_func(wxButton &, wxEvent &)
    { wxHelpContents("Sorting and Filtering Solutions"); }

    void OnOk(void);
    void OnCancel(void);
    int completed;

public:
    BSolnSortFilterDialog(BSolnSortFilterOptions &options);
    int Completed(void);
};


BSolnSortFilterDialog::BSolnSortFilterDialog(BSolnSortFilterOptions &options_)
  : wxDialog(0, -1, "Sort & Filter", wxDefaultPosition, wxDefaultSize),
    options(options_) 
{
  //    SetLabelPosition(wxVERTICAL);
  wxString sort_by_str[] =
  { "Id", "Creator", "Nash", "Perfect", "Sequential", "G Value", "G Lambda", "L Value" };

  sortby = new wxRadioBox(this, -1, "Sort By",
			  wxDefaultPosition, wxDefaultSize,
			  8, sort_by_str, 2);

  new wxStaticBox(this, -1, "Filter By",
		  wxPoint(12, 95), wxSize(464, 150));
  filter_creator = new wxListBox(this, -1, 
				 wxPoint(55, -1), wxSize(140, 85),
				 NUM_BCREATORS, options.filter_cr_str+1,
				 wxLB_MULTIPLE);
  filter_nash = new wxListBox(this, -1,
			      wxDefaultPosition, wxSize(44, 85),
			      3, options.filter_tri_str+1,
			      wxLB_MULTIPLE);
  filter_perfect = new wxListBox(this, -1,
				 wxDefaultPosition, wxSize(56, 85),
				 3, options.filter_tri_str+1,
				 wxLB_MULTIPLE);
  filter_seq = new wxListBox(this, -1,
			     wxDefaultPosition, wxSize(78, 85),
			     3, options.filter_tri_str+1,
			     wxLB_MULTIPLE);

  Fit();

  // Now setup the data
  sortby->SetSelection(options.SortBy()-1);
    int i;

    for (i = 1; i <= NUM_BCREATORS; i++)
        filter_creator->SetSelection(i-1, options.FilterCr()[i]);

    for (i = 1; i <= 3; i++)
    {
        filter_nash->SetSelection(i-1, options.FilterNash()[i]);
        filter_perfect->SetSelection(i-1, options.FilterPerfect()[i]);
        filter_seq->SetSelection(i-1, options.FilterSeq()[i]);
    }

    Show(TRUE);
}

void BSolnSortFilterDialog::OnOk(void)
{
    // update the BSolnSortFilterOptions
    options.SortBy() = sortby->GetSelection()+1;
    int i;

    for (i = 1; i <= NUM_BCREATORS; i++)
        options.FilterCr()[i] = filter_creator->Selected(i-1);

    for (i = 1; i <= 3; i++)
    {
        options.FilterNash()[i] = filter_nash->Selected(i-1);
        options.FilterPerfect()[i] = filter_perfect->Selected(i-1);
        options.FilterSeq()[i] = filter_seq->Selected(i-1);
    }

    completed = wxOK;
    Show(FALSE);
}

void BSolnSortFilterDialog::OnCancel(void)
{
    completed = wxCANCEL;
    Show(FALSE);
}

int BSolnSortFilterDialog::Completed(void)
{
    return completed;
}



/*****************************************************************************
 *                           BEHAV SOLUTION EDIT
 *****************************************************************************/

#ifdef NOT_PORTED_YET

// Constructor

BehavSolnEdit::BehavSolnEdit(BehavSolution &soln_,
                             int iset_disp, wxWindow *parent,
			     int p_decimals)
    : SpreadSheet3D(soln_.Game().NumPlayerInfosets()+1,
                    gmax(EFSupport(soln_.Game()).NumActions())+2,
                    1, 2, "Edit Behav Solution", parent, ANY_BUTTON),
      soln(soln_), dim(EFSupport(soln_.Game()).NumActions())
{
  num_isets = soln.Game().NumPlayerInfosets();
  Show(FALSE);
  int j;
  int max_dim = gmax(dim);
  int num_players = dim.Lengths().Length();

  DrawSettings()->SetColWidth(8, 1);  // Player name "Player #" = 8 chars
  DrawSettings()->SetColWidth(5, 2);  // Iset name (assume 5 letters average);
  SetCell(1, 1, "Player");
  Bold(1, 1, 0, TRUE);
  SetCell(1, 2, "Iset");
  Bold(1, 2, 0, TRUE);
  int cur_pos = 2;
  
  for (j = 1; j <= num_players; j++)  {    // print the players
    if (dim.Lengths()[j] == 0) 
      continue;

    SetCell(cur_pos, 1, soln.Game().Players()[j]->GetName());

    for (int k = 1; k <= dim.Lengths()[j]; k++) {  // print the infosets
      // Display ISET in the same format as that selected for the main tree
      // display below the node.  That is, either the infoset name or the
      // infoset id.  Check the TreeDrawSettings for the current value.
      gText iset_label;

      if (iset_disp == NODE_BELOW_ISETID)
	iset_label = "("+ToText(j)+","+ToText(k)+")";
      else
	iset_label = soln.Game().Players()[j]->Infosets()[k]->GetName();

      SetCell(cur_pos, 2, iset_label);
      
      for (int l = 1; l <= dim(j, k); l++) {
	// print actual values
	Action *action = soln.Game().Players()[j]->Infosets()[k]->Actions()[l];
	SetCell(cur_pos, 2+l, ToText(soln(action), p_decimals));
	SetType(cur_pos, 2+l, gSpreadStr);
      }
      for (int l = dim(j, k)+1; l <= max_dim; l++) 
	HiLighted(cur_pos, 2+l, 0, TRUE);

      cur_pos++;
    }
  }

  SetCurCol(3);
  SetCurRow(2);
  MakeButtons(OK_BUTTON|CANCEL_BUTTON|PRINT_BUTTON|HELP_BUTTON);
  Redraw();
  Show(TRUE);
}

bool BehavSolnEdit::OnClose(void)
{
  SetCompleted(wxCANCEL);
  Show(FALSE);
  return FALSE;
}

void BehavSolnEdit::OnSelectedMoved(int row, int col, SpreadMoveDir /*how*/)
{
    int pl = 1, iset = 1;
    int num_players = dim.Lengths().Length();

    if (row != 1)
    {
        row--;
        row = row%num_isets;

        if (row == 0)
        {
            pl = num_players;
            iset = dim.Lengths()[pl];
        }
        else
        {
            while (row-dim.Lengths()[pl] > 0)
            {
                row -= dim.Lengths()[pl];
                pl++;
                iset = row;
            }
        }

        if (row == dim.Lengths()[pl]) 
            iset = row;
    }
    // can not move to player/iset cols or outside the strat range
    else
    {
        SetCurRow(2);
    }

    if (col-2 > dim(pl, iset))
        SetCurCol(dim(pl, iset)+2);
    else
        if (col < 3) SetCurCol(3);
}


// OnOK
void BehavSolnEdit::OnOk(void)
{
  int cur_pos = 2;

  for (int i = 1; i <= dim.Lengths().Length(); i++) {
    for (int j = 1; j <= dim.Lengths()[i]; j++) {
      for (int k = 1; k <= dim(i, j); k++) {
	Action *action = soln.Game().Players()[i]->Infosets()[j]->Actions()[k];
	gNumber value;
	FromText(GetCell(cur_pos, 2+k), value);
	soln.Set(action, value);
      }

      cur_pos++;
    }
  }

  SetCompleted(wxOK);
  Show(FALSE);
}


//****************************************************************************
//                       BEHAV SOLUTION PICKER (multiple)
//****************************************************************************

EfgSolnPicker::EfgSolnPicker(const Efg &ef_, BehavSolutionList &soln,
                             const GambitDrawSettings &draw_settings,
                             BSolnSortFilterOptions &sf_options,
                             EfgShow *parent_)
    :
    EfgSolnShow(ef_, soln, 0, draw_settings, sf_options, parent_, BSOLN_O_PICKER),
    picked(soln.Length())
{
    SetTitle("Pick solutions to proceed");
    //    char *defaults_file = gambitApp.ResourceFile();
#ifdef NOT_PORTED_YET
    wxGetResource(SOLN_SECT, "Efg-Interactive-Solns-All", &pick_all, defaults_file);
#endif  // NOT_PORTED_YET

    //wxCheckBox *pick_all_box = new wxCheckBox(Panel(), (wxFunction)pick_all_func, "All");
    pick_all_button = new wxButton(Panel(), -1, "     ");
    //    (void)new wxMessage(Panel(), "       Double click on a solution # to toggle it");
    Redraw();
    pick_all = !pick_all;       // OnPickAll() will toggle automatically ...
    OnPickAll();
}


void EfgSolnPicker::PickSoln(int row)
{
    // figure out which solution was clicked on
    int new_soln = SolnNum(row);

    if (new_soln == 0) 
        return;

    if (picked[new_soln]) // un-highlight this solution
    {
        HiLighted(SolnPos(new_soln), 1, 0, FALSE);
        picked[new_soln] = false;
    }
    else                                    // highlight this solution
    {
        HiLighted(SolnPos(new_soln), 1, 0, TRUE);
        picked[new_soln] = true;
    }

    Repaint();
}


void EfgSolnPicker::OnDoubleClick(int row, int col, int , const gText &)
{
    if (col == 1) 
        PickSoln(row);
}


// OnOk: remove all of the unselected solutions
void EfgSolnPicker::OnOk(void)
{
    int l = solns.Length();
    bool ok = false;
    //make sure that at least one solution was picked
    int i;

    for (i = l; i >= 1; i--)
    {
        if (picked[i]) 
            ok = true;
    }

    if (!ok)
    {
        wxMessageBox("Must pick at least one solution", "Pick a solution", 
                     wxOK | wxCENTRE, this);
        return;
    }

    for (i = l; i >= 1; i--)
    {
        if (!picked[i]) 
            solns.Remove(i);
    }

    SetCompleted(wxOK);
}

void EfgSolnPicker::OnSelectedMoved(int , int , SpreadMoveDir )
{ }

// Overide help system

void EfgSolnPicker::OnHelp(int help_type)
{
    if (!help_type) // contents
        wxHelpContents(EFG_SUBGAMESOLN_HELP);
    else
        wxHelpAbout();
}


void EfgSolnPicker::pick_all_func(wxButton &ob, wxEvent &)
{
    ((EfgSolnPicker *)ob.GetClientData())->OnPickAll();
}


void EfgSolnPicker::OnPickAll(void)
{
    if (!pick_all) // now pick all
    {
        for (int i = 1; i <= picked.Length(); i++)
        {
            picked[i] = true;
            HiLighted(SolnPos(i), 1, 0, TRUE);
        }

        pick_all_button->SetLabel("None");
        pick_all = true;
    }
    else                    // now pick none
    {
        for (int i = 1; i <= picked.Length(); i++)
        {
            picked[i] = false;
            HiLighted(SolnPos(i), 1, 0, FALSE);
        }

        pick_all_button->SetLabel("All");
        pick_all = false;
    }

    Repaint();
}


//****************************************************************************
//                       BEHAV SOLUTION PICKER (single)
//****************************************************************************

Ext1SolnPicker::Ext1SolnPicker(const Efg &ef_, BehavSolutionList &soln,
                               const GambitDrawSettings &draw_settings,
                               BSolnSortFilterOptions   &sf_options,
                               EfgShow *parent_)
    : EfgSolnShow(ef_, soln, 0, draw_settings, sf_options, parent_, BSOLN_O_EDIT)

{
    SetTitle("Pick a profile to start with");
    picked = 0;
}


void Ext1SolnPicker::PickSoln(int row)
{
    // figure out which solution was clicked on
    int new_soln = SolnNum(row);

    if (new_soln != picked)
    {
        if (picked) 
            HiLighted(SolnPos(picked), 1, 0, FALSE); // un-highlight this solution

        if (new_soln) 
            HiLighted(SolnPos(new_soln), 1, 0, TRUE); // highlight this solution

        picked = new_soln;
    }
    Repaint();
}


void Ext1SolnPicker::OnDoubleClick(int row, int col, int , const gText &)
{
    if (col == 1) 
        PickSoln(row);

    if (col == FeaturePos(BSOLN_DATA))  // edit solution
    {
        SetCurRow(row);
        SetCurCol(col);
        OnEdit();
    }

}


void Ext1SolnPicker::OnSelectedMoved(int , int , SpreadMoveDir )
{ }


// Override help system

void Ext1SolnPicker::OnHelp(int help_type)
{
    if (!help_type) // contents
        wxHelpContents(EFG_SOLNSTART_HELP);
    else
        wxHelpAbout();
}


void Ext1SolnPicker::OnOk(void)
{
    if (!picked && solns.Length() != 0)
        wxMessageBox("You must pick a starting point");
    else
        SetCompleted(wxOK);
}


// OnRemove

void Ext1SolnPicker::OnRemove(bool all)
{
    if (solns.Length() == 0)
    {
        wxMessageBox("Solution list is empty");
        return;
    }

    if (!all)
    {
        int row = CurRow();

        if (row == 1)
        {
            wxMessageBox("Remove what?");
            return;
        }

        int soln_num = SolnNum(row);

        if (soln_num == picked) 
            picked = 0;
    }
    else
    {
        picked = 0;
    }

    EfgSolnShow::OnRemove(all);
}


int Ext1SolnPicker::Picked(void) const
{
    return picked;
}

#endif  // NOT_PORTED_YET
