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
//                       NODE INSPECT WINDOW
//****************************************************************************

const int NodeSolnShow::s_numFeatures = 8;
char *NodeSolnShow::s_featureNames[s_numFeatures+1] = 
{ "BlankEntry", "Infoset", "RealizProb", "IsetProb", "BeliefProb", "NodeValue",
  "IsetValue", "BranchProb", "BranchValue" };

NodeSolnShow::NodeSolnShow(EfgShow *p_efgShow, wxWindow *p_parent)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_parent(p_efgShow)
{
  Show(false);
  CreateGrid(s_numFeatures, 1);
  SetEditable(false);

  for (int i = 1; i <= s_numFeatures; i++) {
    SetLabelValue(wxVERTICAL, s_featureNames[i], i - 1);
  }

  SetLabelSize(wxHORIZONTAL, 0);
  SetLabelSize(wxVERTICAL, 100);
  AdjustScrollbars();
}

void NodeSolnShow::Set(const Node *p_cursor) 
{
  m_cursor = p_cursor;
  
  if (!m_cursor) { // no data available
    for (int i = 1; i <= s_numFeatures; i++) { 
      SetCellValue("N/A", i - 1, 0);
    }
    return;
  }

  // if we got here, the node is valid.
  try {
    gText tmpstr;
  
    if (!m_cursor->GetPlayer()) {
      tmpstr = "TERMINAL";
    }
    else {
      if (m_cursor->GetPlayer()->IsChance())
	tmpstr = "CHANCE";
      else
	tmpstr = ("(" + ToText(m_cursor->GetPlayer()->GetNumber()) + "," +
		  ToText(m_cursor->GetInfoset()->GetNumber()) + ")");
    }
	  
    SetCellValue((char *) tmpstr, 0, 0);

    SetCellValue((char *) m_parent->AsString(tRealizProb, m_cursor), 1, 0);
    SetCellValue((char *) m_parent->AsString(tIsetProb, m_cursor), 2, 0);
    SetCellValue((char *) m_parent->AsString(tBeliefProb, m_cursor), 3, 0);
    SetCellValue((char *) m_parent->AsString(tNodeValue, m_cursor), 4, 0);
    SetCellValue((char *) m_parent->AsString(tIsetValue, m_cursor), 5, 0);
	
    Node *p = m_cursor->GetParent();
    if (p) {
      int branch = 0;
      for (branch = 1; p->GetChild(branch) != m_cursor; branch++);
      SetCellValue((char *) m_parent->AsString(tBranchProb, p, branch), 6, 0);
    }
    else {
      SetCellValue("1", 6, 0);
    }
	
    if (p) {
      int branch = 0;
      for (branch = 1; p->GetChild(branch) != m_cursor; branch++);
      SetCellValue((char *) m_parent->AsString(tBranchVal, p, branch), 7, 0);
    }
    else {
      SetCellValue("N/A", 7, 0);
    }
  }	
  catch (gNumber::DivideByZero &) { }
}

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


//****************************************************************************
//                       BEHAV SOLUTION SHOW
//****************************************************************************

#define MAX_SOLNSHOW_WIDTH  25

#define BSOLN_DYNAMIC       0       // Optional features.
#define BSOLN_EQUVALS       1
#define BSOLN_CREATOR       2
#define BSOLN_ISNASH        3
#define BSOLN_ISPERF        4
#define BSOLN_ISSEQ         5
#define BSOLN_GLAMBDA       6
#define BSOLN_GVALUE        7
#define BSOLN_LVALUE        8
#define BSOLN_BELIEF        9

#define BSOLN_NUM_FEATURES  9

#define BSOLN_ID                20  // These are always on
#define BSOLN_PLAYER        21
#define BSOLN_ISET          22
#define BSOLN_DATA          23

char *EfgSolnShow::feature_names[BSOLN_NUM_FEATURES] = 
{
    "Update Dynamically", "Equ Values",
    "Creator", "Nash", "SubgPerf", "Sequential",
    "QRE Lambda", "QRE Value", "Liap Value"
};

int EfgSolnShow::feature_width[BSOLN_NUM_FEATURES] = 
{ 0, -1, 8, 2, 2, 2, 7, 7, 7};

BEGIN_EVENT_TABLE(EfgSolnShow, wxGrid)

END_EVENT_TABLE()

EfgSolnShow::EfgSolnShow(EfgShow *p_efgShow, wxWindow *p_parent,
			 BehavSolutionList &p_solutions,
                         BSolnSortFilterOptions &p_options)
  : wxGrid(p_parent, -1, wxDefaultPosition, wxDefaultSize),
    m_parent(p_efgShow), solns(p_solutions),
    num_solutions(p_solutions.Length()), cur_soln(1),
    features(0, BSOLN_NUM_FEATURES-1), sf_options(p_options)
{
  CreateGrid(solns.Length() * m_parent->Game()->NumPlayerInfosets(), 4);
  SetEditable(false);
  SetLabelSize(wxVERTICAL, 0);

  for (int i = 1; i < BSOLN_NUM_FEATURES; i++) {
    features[i] = 1;
    
    int col = FeaturePos(i);
    InsertCols(col, 1, false);
    SetLabelValue(wxHORIZONTAL, (char *) feature_names[i], col);
  }

  SetLabelValue(wxHORIZONTAL, "ID", FeaturePos(BSOLN_ID));
  SetLabelValue(wxHORIZONTAL, "Player", FeaturePos(BSOLN_PLAYER));
  SetLabelValue(wxHORIZONTAL, "Iset", FeaturePos(BSOLN_ISET));
  SetLabelValue(wxHORIZONTAL, "Probs", FeaturePos(BSOLN_DATA));

  if (cur_soln) {
    SetGridCursor(SolnPos(cur_soln) - 1, 0);
    OnSelectCell(SolnPos(cur_soln) - 1, 0);
  }

  SortFilter(false);
  AdjustScrollbars();
  UpdateValues();
}


bool EfgSolnShow::HilightInfoset(int pl, int iset)
{
#ifdef NOT_PORTED_YET
  int row = 2;

  for (int i = 1; i <= num_solutions; i++)
    for (int j = 1; j <= num_players; j++)
      for (int k = 1; k <= dim.Lengths()[j]; k++, row++)
	HiLighted(row, 3+features[BSOLN_EQUVALS], 1, (pl == j && iset == k) ? TRUE : FALSE);
  
  Repaint();
#endif  // NOT_PORTED_YET
  return FALSE;
}

int EfgSolnShow::FeaturePos(int feature)
{
  switch (feature) {
    // These features are always on and always in the same columns
  case BSOLN_ID: 
    return 0;
  case BSOLN_PLAYER: 
    return 1;
  case BSOLN_ISET: 
    return 2;
    // The data is always the LAST column
  case BSOLN_DATA: 
    return (FeaturePos(BSOLN_NUM_FEATURES-1)+1);

    // The rest of the features are optional and may change location
  default: {
      int pos = 3;

      for (int i = 0; i <= feature; i++) {
	if (features[i] && feature_width[i] > 0) 
	  pos++;
      }
      
      return pos;
    }
  }
}

int EfgSolnShow::SolnNum(int row)
{
  assert(row >= 0 && row < GetRows());

  // figure out which solution was clicked on
  int new_soln = row / m_parent->Game()->NumPlayerInfosets() + 1;
  assert(new_soln >= 1 && new_soln <= solns.Length());
  return new_soln;
}


int EfgSolnShow::SolnPos(int soln_num)
{
  assert(soln_num >= 0 && soln_num <= num_solutions);
  return (2+((soln_num) ? (soln_num-1)*m_parent->Game()->NumPlayerInfosets() : -1));
}

void EfgSolnShow::UpdateValues(void)
{
  int currentRow = 0;

  for (int i = 1; i <= solns.Length(); i++) {
    if (i > GetRows()) {
      SetEditable(true);
      AppendRows();
      AdjustScrollbars();
      SetEditable(false);
    }
    const BehavSolution &currentSolution = solns[i];
    SetCellValue((char *) ToText((int) currentSolution.Id()),
		 currentRow, FeaturePos(BSOLN_ID));

    if (features[BSOLN_CREATOR]) {
      SetCellValue((char *) NameEfgAlgType(currentSolution.Creator()),
		   currentRow, FeaturePos(BSOLN_CREATOR));
    }
    if (features[BSOLN_ISNASH]) {
      SetCellValue((char *) Name(currentSolution.IsNash()),
		   currentRow, FeaturePos(BSOLN_ISNASH));
    }
    if (features[BSOLN_ISPERF]) {
      SetCellValue((char *) Name(currentSolution.IsSubgamePerfect()),
		   currentRow, FeaturePos(BSOLN_ISPERF));
    }
    if (features[BSOLN_ISSEQ]) {
      SetCellValue((char *) Name(currentSolution.IsSequential()),
		   currentRow, FeaturePos(BSOLN_ISSEQ));
    }
    if (features[BSOLN_GLAMBDA]) {
      if (currentSolution.Creator() == algorithmEfg_QRE_EFG ||
	  currentSolution.Creator() == algorithmEfg_QRE_NFG) {
	SetCellValue((char *) ToText(currentSolution.QreLambda(),
				     m_parent->NumDecimals()),
		     currentRow, FeaturePos(BSOLN_GLAMBDA));
      }
      else {
	SetCellValue("--------", currentRow, FeaturePos(BSOLN_GLAMBDA));
      }
    }
    if (features[BSOLN_GVALUE]) {
      if (currentSolution.Creator() == algorithmEfg_QRE_EFG ||
	  currentSolution.Creator() == algorithmEfg_QRE_NFG) {
	SetCellValue((char *) ToText(currentSolution.QreValue(),
				     m_parent->NumDecimals()),
		     currentRow, FeaturePos(BSOLN_GVALUE));
      }
      else {
	SetCellValue("--------", currentRow, FeaturePos(BSOLN_GVALUE));
      }
    }

    if (features[BSOLN_LVALUE]) {
      SetCellValue((char *) ToText(currentSolution.LiapValue(), 
				   m_parent->NumDecimals()),
		   currentRow, FeaturePos(BSOLN_LVALUE));
    }

    for (int j = 1; j <= m_parent->Game()->NumPlayers(); j++) {
      if (m_parent->Game()->Players()[j]->NumInfosets() == 0) continue;
      SetCellValue((char *) currentSolution.Game().Players()[j]->GetName(),
		   currentRow, FeaturePos(BSOLN_PLAYER));
      
      if (features[BSOLN_EQUVALS]) {
	SetCellValue((char *) ToText(currentSolution.Payoff(j),
				     m_parent->NumDecimals()),
		     currentRow, FeaturePos(BSOLN_EQUVALS));
      }

      for (int k = 1; k <= m_parent->Game()->Players()[j]->NumInfosets(); k++) {
	gText isetLabel;

	if (m_parent->GetTreeWindow()->DrawSettings().LabelNodeBelow() ==
	    NODE_BELOW_ISETID) {
	  isetLabel = "(" + ToText(j) + "," + ToText(k) + ")";
	}
	else {
	  isetLabel = currentSolution.Game().Players()[j]->Infosets()[k]->GetName();
	}

	SetCellValue((char *) isetLabel, currentRow, FeaturePos(BSOLN_ISET));
	gText tmpStr = "{";
	    
	int numisets = m_parent->Game()->Players()[j]->Infosets()[k]->NumActions();
	for (int l = 1; l <= numisets; l++)   { 
	  Action *action = currentSolution.Game().Players()[j]->Infosets()[k]->Actions()[l];
	  tmpStr += (ToText(currentSolution(action)) +
		     ((l == numisets) ? "}" : ","));
	}
	    
	SetCellValue((char *) tmpStr, currentRow, FeaturePos(BSOLN_DATA));
	currentRow++;
      }
    }
  }
}

void EfgSolnShow::OnLabelLeftClick(int row, int col, int x, int y,
				   bool control, bool shift)
{
  if (row == -1) {
    int old_sort_by = sf_options.SortBy();
    
    if (col == FeaturePos(BSOLN_CREATOR) && features[BSOLN_CREATOR])
      sf_options.SortBy() = BSORT_BY_CREATOR;

    if (col == FeaturePos(BSOLN_ISNASH) && features[BSOLN_ISNASH])
      sf_options.SortBy() = BSORT_BY_NASH;

    if (col == FeaturePos(BSOLN_ISPERF) && features[BSOLN_ISPERF])
      sf_options.SortBy() = BSORT_BY_PERFECT;

    if (col == FeaturePos(BSOLN_ISSEQ) && features[BSOLN_ISSEQ])
      sf_options.SortBy() = BSORT_BY_SEQ;

    if (col == FeaturePos(BSOLN_GLAMBDA) && features[BSOLN_GLAMBDA])
      sf_options.SortBy() = BSORT_BY_GLAMBDA;

    if (col == FeaturePos(BSOLN_GVALUE) && features[BSOLN_GVALUE])
      sf_options.SortBy() = BSORT_BY_GVALUE;

    if (col == FeaturePos(BSOLN_LVALUE) && features[BSOLN_LVALUE])
      sf_options.SortBy() = BSORT_BY_LVALUE;

    if (old_sort_by != sf_options.SortBy()) 
      SortFilter(false);
  }
}

void EfgSolnShow::OnDoubleClick(int row, int col, int , const gText &)
{
#ifdef NOT_PORTED_YET
  if (col == FeaturePos(BSOLN_ID))
    UpdateSoln(row, col);      // change solution

  if (col == FeaturePos(BSOLN_ISET)) {  // hilight infoset
    int pl = 1, iset = 1;

    if (row == 1) {
      pl = 0;
      iset = 0;
    }
    else {
      row--;
      row = row%num_isets;

      if (row == 0) {
	pl = num_players;
	iset = dim.Lengths()[pl];
      }
      else {
	while (row-dim.Lengths()[pl] > 0) {
	  row -= dim.Lengths()[pl];
	  pl++;
	  iset = row;
	}
      }

      if (row == dim.Lengths()[pl]) 
	iset = row;
    }

    parent->HilightInfoset(pl, iset, 2);
  }

  if (col == FeaturePos(BSOLN_DATA)) {  // edit solution
    SetCurRow(row);
    SetCurCol(col);
    OnEdit();
  }

  // Double clicking on the first row in any optional feature col, sorts by that col
#endif  // NOT_PORTED_YET
}


void EfgSolnShow::OnSelectCell(int p_row, int p_col)
{
  for (int i = (cur_soln - 1) * m_parent->Game()->NumPlayerInfosets();
       i < cur_soln * m_parent->Game()->NumPlayerInfosets(); i++) {
    for (int j = 0; j < GetCols(); j++) {
      SetCellBackgroundColour(*wxWHITE, i, j);
    }
  }

  cur_soln = SolnNum(p_row);

  for (int i = (cur_soln - 1) * m_parent->Game()->NumPlayerInfosets();
       i < cur_soln * m_parent->Game()->NumPlayerInfosets(); i++) {
    for (int j = 0; j < GetCols(); j++) {
      SetCellBackgroundColour(*wxCYAN, i, j);
    }
  }

  if (m_parent) {
    m_parent->ChangeSolution(cur_soln);
  }
}

void EfgSolnShow::SortFilter(bool)
{
  int old_num_sol = num_solutions;  // current state
  const BehavSolution *cur_solnp = 0;

  if (cur_soln) 
    cur_solnp = &solns[cur_soln];
    
  BSolnSorterFilter SF(solns, sf_options); 
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
}


// OnRemove

void EfgSolnShow::OnRemove(bool all)
{
#ifdef NOT_PORTED_YET
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

        solns.Remove(soln_num);

        if (soln_num == cur_soln) 
            UpdateSoln(1, 1); // unselect all solutions

        for (int i = num_isets; i >= 1; i--) 
            DelRow(2+(soln_num-1)*num_players+(i-1));

        num_solutions--;
    }
    else
    {
        UpdateSoln(1, 1);       // unselect all solutions

        while (solns.Length()) 
            solns.Remove(1);    // del all solutions

        while (GetRows() > 1) 
            DelRow(2);          // del all rows after row 1

        num_solutions = 0;
    }

    CanvasFocus();
    UpdateValues();
    Redraw();
#endif  // NOT_PORTED_YET
}


// OnAdd
void EfgSolnShow::OnAdd(void)
{
#ifdef NOT_PORTED_YET
    BehavSolution temp_soln(parent->CreateSolution());
    BehavSolnEdit *add_dialog =
        new BehavSolnEdit(temp_soln, 
                          parent->GetTreeWindow()->DrawSettings().LabelNodeBelow(), 
                          this->GetCanvas(), parent->NumDecimals());

    if (add_dialog->ShowModal() == wxID_OK) {
      solns.Append(temp_soln);
      SortFilter(false);
    }

    delete add_dialog;
#endif  // NOT_PORTED_YET
}


// OnEdit
void EfgSolnShow::OnEdit(void)
{
#ifdef NOT_PORTED_YET
    if (solns.Length() == 0)
    {
        wxMessageBox("Solution list is empty");
        return;
    }

    int row = CurRow();

    if (row == 1)
    {
        wxMessageBox("Edit what?");
        return;
    }

    int soln_num = SolnNum(row);

    BehavSolution temp_soln = solns[soln_num];
    BehavSolnEdit *add_dialog = 
        new BehavSolnEdit(temp_soln, 
                          parent->GetTreeWindow()->DrawSettings().LabelNodeBelow(), 
                          this, parent->NumDecimals());

    Enable(FALSE);  // disable this window until the edit window is closed

    while (add_dialog->Completed() == wxRUNNING) 
        wxYield();

    Enable(TRUE);
    CanvasFocus();

    if (add_dialog->Completed() == wxOK)
    {
        solns[soln_num] = temp_soln;
        SortFilter(false);

        if (cur_soln == soln_num) 
            parent->ChangeSolution(cur_soln);
    }

    delete add_dialog;
#endif  // NOT_PORTED_YET
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
