//
// FILE: nfgsoln.cc --  all solution display related routines for the normal
// form.
//
// $Id$
//

#include "wx/wx.h"

#include "gambit.h"
#include "nfgconst.h"
#include "nfgshow.h"
#include "nfgsoln.h"
#include "nfplayer.h"

#ifdef NOT_PORTED_YET

//=========================================================================
//                      class dialogMixedSortFilter
//=========================================================================

class dialogMixedSortFilter : public guiAutoDialog {
private:
  MSolnSortFilterOptions &options;
  wxRadioBox *m_sortBy;
  wxListBox *m_filterCreator, *m_filterNash, *m_filterPerfect, *m_filterProper;

public:
  dialogMixedSortFilter(MSolnSortFilterOptions &options);
  virtual ~dialogMixedSortFilter();
};


dialogMixedSortFilter::dialogMixedSortFilter(MSolnSortFilterOptions &options_)
  : guiAutoDialog(0, "Sort & Filter"), options(options_)
{
  wxString sortByChoices[] = { "ID", "Creator", "Nash", "Perfect", 
			       "Qre Value", "Qre Lambda", "Liap Value" };
  m_sortBy = new wxRadioBox(this, -1, "Sort By",
			    wxDefaultPosition, wxDefaultSize,
			    7, sortByChoices);
  m_sortBy->SetSelection(options.SortBy() - 1);
  m_sortBy->SetConstraints(new wxLayoutConstraints);
  m_sortBy->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_sortBy->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_sortBy->GetConstraints()->width.AsIs();
  m_sortBy->GetConstraints()->height.AsIs();

  wxStaticBox *filterGroup = new wxStaticBox(this, -1, "Filter By");

  m_filterCreator = new wxListBox(this, -1, wxDefaultPosition, wxDefaultSize,
				  NUM_MCREATORS, options.filter_cr_str + 1,
				  wxLB_MULTIPLE);
  int listWidth = (int) m_filterCreator->GetCharWidth() * 14;
  for (int i = 1; i <= NUM_MCREATORS; i++)
    m_filterCreator->SetSelection(i - 1, options.FilterCr()[i]);
  m_filterCreator->SetConstraints(new wxLayoutConstraints);
  m_filterCreator->GetConstraints()->top.SameAs(filterGroup, wxTop, 20);
  m_filterCreator->GetConstraints()->left.SameAs(filterGroup, wxLeft, 10);
  m_filterCreator->GetConstraints()->height.AsIs();
  m_filterCreator->GetConstraints()->width.Absolute(listWidth);

  m_filterNash = new wxListBox(this, -1, wxDefaultPosition, wxDefaultSize,
			       3, options.filter_tri_str + 1,
			       wxLB_MULTIPLE);
  m_filterNash->SetConstraints(new wxLayoutConstraints);
  m_filterNash->GetConstraints()->top.SameAs(m_filterCreator, wxTop);
  m_filterNash->GetConstraints()->left.SameAs(m_filterCreator, wxRight, 10);
  m_filterNash->GetConstraints()->height.AsIs();
  m_filterNash->GetConstraints()->width.PercentOf(m_filterCreator,
						  wxWidth, 75);

  m_filterPerfect = new wxListBox(this, -1, wxDefaultPosition, wxDefaultSize,
				  3, options.filter_tri_str + 1,
				  wxLB_MULTIPLE);
  m_filterPerfect->SetConstraints(new wxLayoutConstraints);
  m_filterPerfect->GetConstraints()->top.SameAs(m_filterNash, wxTop);
  m_filterPerfect->GetConstraints()->left.SameAs(m_filterNash, wxRight, 10);
  m_filterPerfect->GetConstraints()->height.AsIs();
  m_filterPerfect->GetConstraints()->width.PercentOf(m_filterCreator,
						     wxWidth, 75);

  m_filterProper = new wxListBox(this, -1, wxDefaultPosition, wxDefaultSize,
				 3, options.filter_tri_str + 1,
				 wxLB_MULTIPLE);
  m_filterProper->SetConstraints(new wxLayoutConstraints);
  m_filterProper->GetConstraints()->top.SameAs(m_filterPerfect, wxTop);
  m_filterProper->GetConstraints()->left.SameAs(m_filterPerfect, wxRight, 10);
  m_filterProper->GetConstraints()->height.AsIs();
  m_filterProper->GetConstraints()->width.PercentOf(m_filterCreator,
						    wxWidth, 75);

  filterGroup->SetConstraints(new wxLayoutConstraints);
  filterGroup->GetConstraints()->left.SameAs(this, wxLeft, 10);
  filterGroup->GetConstraints()->top.SameAs(m_sortBy, wxBottom, 10);
  filterGroup->GetConstraints()->right.SameAs(m_filterProper, wxRight, -10);
  filterGroup->GetConstraints()->bottom.SameAs(m_filterCreator, wxBottom, -10);

  m_okButton->GetConstraints()->top.SameAs(filterGroup, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(m_cancelButton, wxLeft, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->centreX.SameAs(this, wxCentreX);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  for (int i = 1; i <= 3; i++) {
    m_filterNash->SetSelection(i - 1, options.FilterNash()[i]);
    m_filterPerfect->SetSelection(i - 1, options.FilterPerfect()[i]);
    m_filterProper->SetSelection(i - 1, options.FilterProper()[i]);
  }

  Go();
}

dialogMixedSortFilter::~dialogMixedSortFilter()
{
  if (Completed() == wxOK) {
    options.SortBy() = m_sortBy->GetSelection() + 1;

    for (int i = 1; i <= NUM_MCREATORS; i++)
      options.FilterCr()[i] = m_filterCreator->Selected(i - 1);

    for (int i = 1; i <= 3; i++) {
      options.FilterNash()[i] = m_filterNash->Selected(i - 1);
      options.FilterPerfect()[i] = m_filterPerfect->Selected(i - 1);
      options.FilterProper()[i] = m_filterProper->Selected(i - 1);
    }
  }
}
#endif // NOT_PORTED_YET

#ifdef NOT_PORTED_YET
class MixedSolnEdit : public SpreadSheet3D {
private:
  MixedSolution &soln;
  gArray<int> dim;

public:
  MixedSolnEdit(MixedSolution &soln_, wxFrame *parent, int decimals);
  void OnSelectedMoved(int row, int col, SpreadMoveDir /*how*/);
  void OnOk(void);
  bool OnClose(void);
};

MixedSolnEdit::MixedSolnEdit(MixedSolution &soln_, wxFrame *parent,
			     int decimals)
  :  SpreadSheet3D(soln_.Game().NumPlayers() + 1,
		   gmax(NFSupport(soln_.Game()).NumStrats()) + 1,
		   1, 2, "Edit Mixed Solution", parent, ANY_BUTTON),
     soln(soln_)
{
  Show(FALSE);
  int i, j;
  NFSupport sup(soln.Game());
  dim = sup.NumStrats();
  int max_dim = gmax(dim);
  int num_players = dim.Length();

  DrawSettings()->SetColWidth(8, 1);

  for (i = 1; i <= num_players; i++) {   // label rows
    gText tmp_str = sup.Game().Players()[i]->GetName();

    if (tmp_str == "") 
      tmp_str = "Player " + ToText(i);

    SetCell(i + 1, 1, tmp_str);
    Bold(i + 1, 1, 0, TRUE);
  }
  
  for (i = 1; i <= max_dim; i++) {  // label cols
    SetCell(1, i + 1, ToText(i));
    Bold(1, i + 1, 0, TRUE);
    DrawSettings()->SetColWidth(2 + decimals, i + 1);
  }

  for (i = 1; i <= num_players; i++)  {
    // enter values
    for (j = 1; j <= dim[i]; j++) {
      Strategy *strategy = soln.Game().Players()[i]->Strategies()[j];
      SetCell(i + 1, j + 1, ToText(soln(strategy), decimals));
      SetType(i + 1, j + 1, gSpreadStr);
    }
    
    for (j = dim[i] + 1; j <= max_dim; j++) 
      HiLighted(i + 1, j + 1, 1, TRUE);
  }

  SetCurCol(2);
  SetCurRow(2);
  MakeButtons(OK_BUTTON|CANCEL_BUTTON|PRINT_BUTTON|HELP_BUTTON);
  Redraw();
  Show(TRUE);
}

bool MixedSolnEdit::OnClose(void)
{
  SetCompleted(wxCANCEL);
  Show(FALSE);
  return FALSE;
}

void MixedSolnEdit::OnSelectedMoved(int row, int col, SpreadMoveDir /*how*/)
{
  if (row == 1) {
    SetCurRow(2);
    row = 2;
  }

  if (col - 1 > dim[row-1]) 
    SetCurCol(dim[row-1] + 1);

  if (col == 1) 
    SetCurCol(2);
}

// OnOK
void MixedSolnEdit::OnOk(void)
{
  for (int i = 1; i <= dim.Length(); i++)
    for (int j = 1; j <= dim[i]; j++) {
      gNumber value;
      FromText(GetCell(i + 1, j + 1), value);
      soln.Set(soln.Game().Players()[i]->Strategies()[j], value);
    }
  SetCompleted(wxOK);
  Show(FALSE);
}
#endif  // NOT_PORTED_YET


#ifdef NOT_PORTED_YET


//****************************************************************************
//                       MIXED SOLUTION PICKER (single)
//****************************************************************************

Nfg1SolnPicker::Nfg1SolnPicker(gSortList<MixedSolution > &soln, int num_players, int max_strats,
                               int cur_soln_, NormalDrawSettings    &ds,
                               MSolnSortFilterOptions &sf_options,
                               NfgShow *parent_, wxFrame *parent_frame)
    : NfgSolnShow(soln, num_players, max_strats, cur_soln_, ds, 
                  sf_options, parent_, parent_frame, MSOLN_O_EDIT)
{
    SetTitle("Pick a profile to start with");
    picked = 0;
}


void Nfg1SolnPicker::PickSoln(int row)
{
    // figure out which solution was clicked on
    int new_soln = SolnNum(row);

    if (new_soln != picked)
    {
        if (picked) 
            HiLighted(SolnPos(picked), 1, 0, FALSE);   // un-highlight this solution

        if (new_soln) 
            HiLighted(SolnPos(new_soln), 1, 0, TRUE);  // highlight this solution

        picked = new_soln;
    }

    Repaint();
}


void Nfg1SolnPicker::OnDoubleClick(int row, int col, int , const gText &)
{
    if (col > FeaturePos(MSOLN_NUM_FEATURES - 1))   // edit solution
    {
        SetCurRow(row);
        SetCurCol(col);
        OnEdit();
    }

    if (col == 1) 
        PickSoln(row);
}


void Nfg1SolnPicker::OnSelectedMoved(int , int , SpreadMoveDir )
{ }


// Override help system

void Nfg1SolnPicker::OnHelp(int help_type)
{
    if (!help_type) // contents
        wxHelpContents(NFG_SOLNSTART_HELP);
    else
        wxHelpAbout();
}


void Nfg1SolnPicker::OnOk(void)
{
    if (!picked && solns.Length() != 0)
        wxMessageBox("You must pick a starting point");
    else
        SetCompleted(wxOK);
}


// OnRemove

void Nfg1SolnPicker::OnRemove(bool all)
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

    NfgSolnShow::OnRemove(all);
}


int Nfg1SolnPicker::Picked(void) const
{
    return picked;
}

// OnClose -- Close the window, as if OK was pressed
bool Nfg1SolnPicker::OnClose(void)
{
  OnOk();
  return FALSE;
}

#endif  // NOT_PORTED_YET



