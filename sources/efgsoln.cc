//
// FILE: efgsoln.cc -- all solution display related routines for the efg
//
// $Id$
//

#include "wx/wx.h"
#include "wxmisc.h"
#include "base/base.h"
#include "rational.h"
#include "efgsoln.h"
#include "treedraw.h"
#include "treewin.h"

#ifdef NOT_PORTED_YET

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
