//
// FILE: efgsoln.cc -- all solution display related routines for the efg
//
// $Id$
//

#include "wx.h"
#include "wxmisc.h"
#include "spread3d.h"
#include "garray.h"
#include "rational.h"
#include "gmisc.h"
#include "efgsoln.h"
#include "treedraw.h"
#include "treewin.h"

#define EFG_NODE_INSP_OPTIONS       1999
#define NODESOLN_SECTION    "NodeSolnShow"  // for parameter reading


class BehavSolnEdit : public SpreadSheet3D {
private:
  BehavSolution &soln;
  gPVector<int> dim;
  int num_isets;

public:
  BehavSolnEdit(BehavSolution &soln, int iset_disp, wxFrame *parent, int);

  void OnSelectedMoved(int row, int col, SpreadMoveDir how);
  void OnOk(void);
  Bool OnClose(void);
};

//****************************************************************************
//                       NODE INSPECT WINDOW
//****************************************************************************

#define NUM_FEATURES    8
char *NodeSolnShow::feature_names[NUM_FEATURES+1] = 
{ "BlankEntry", "Infoset", "RealizProb", "IsetProb", "BeliefProb", "NodeValue",
  "IsetValue", "BranchProb", "BranchValue" };

#define INFOSET         1
#define REALIZPROB      2
#define ISETPROB        3
#define BELIEFPROB      4
#define NODEVALUE       5
#define ISETVALUE       6
#define BRANCHPROB      7
#define BRANCHVALUE     8

NodeSolnShow::NodeSolnShow(int num_players, const EfgShow *parent_)
  : SpreadSheet3D(2, 1, 1, 2, "Inspect Node", (wxFrame *)parent_, 0), 
    parent(parent_), features(NUM_FEATURES)
{
  // Read in the defaults
  char *defaults_file = gambitApp.ResourceFile();

  for (int i = 1; i <= NUM_FEATURES; i++)
    wxGetResource(NODESOLN_SECTION, feature_names[i], &features[i], 
		  defaults_file);

  int rows = 0;

  for (int i = 1; i <= NUM_FEATURES; i++) { 
    if (features[i]) 
      rows++;
  }

  int width = ((features[NODEVALUE]) ? 2+num_players*(4+parent->NumDecimals()) : 
	       (2+parent->NumDecimals()));

  SetEditable(FALSE);
  SetDimensions(rows, 1);
  DrawSettings()->SetColWidth(width);
  DrawSettings()->SetLabels(S_LABEL_ROW);

  // Give the frame an icon
#ifdef wx_msw
  wxIcon *frame_icon = new wxIcon("efg_icn");
#else
#include "efg.xbm"
  wxIcon *frame_icon = new wxIcon(efg_bits, efg_width, efg_height);
#endif

  SetIcon(frame_icon);

  SetMenuBar(MakeMenuBar(0));

  for (int i = 1; i <= NUM_FEATURES; i++) {
    if (features[i]) 
      SetLabelRow(Pos(i), feature_names[i]);
  }

  Redraw();
  Show(FALSE);
}


wxMenuBar *NodeSolnShow::MakeMenuBar(long menus)
{
  assert(menus == 0); // we do not use menus and expect it to be 0

  wxMenu *file_menu = new wxMenu;
  file_menu->Append(OUTPUT_MENU, "Out&put", "Output to any device");
  wxMenu *display_menu = new wxMenu;
  display_menu->Append(OPTIONS_MENU, "&Options", "Configure display options");
  display_menu->Append(EFG_NODE_INSP_OPTIONS, "&Features",
		       "Set Extra Features");
  wxMenu *help_menu = new wxMenu;
  help_menu->Append(HELP_MENU_ABOUT, "&About");
  help_menu->Append(HELP_MENU_CONTENTS, "&Contents");
  wxMenuBar *tmp_menubar = new wxMenuBar;
  tmp_menubar->Append(file_menu, "&File");
  tmp_menubar->Append(display_menu, "&Display");
  tmp_menubar->Append(help_menu, "&Help");
  return tmp_menubar;
}


int NodeSolnShow::Pos(int feature)
{
  int pos = 0;

  for (int i = 1; i <= feature; i++) { 
    if (features[i]) 
      pos++;
  }

  if (pos == 0) 
    pos = 1;

  return pos;
}

class dialogFeatures : public guiAutoDialog {
private:
  int m_numFeatures;
  wxCheckBox **m_features;

public:
  dialogFeatures(wxWindow *, const gText &, const gArray<Bool> &, char **);
  virtual ~dialogFeatures();
  
  gArray<Bool> GetFeatures(void) const;
};

dialogFeatures::dialogFeatures(wxWindow *p_parent, const gText &p_caption,
			       const gArray<Bool> &p_settings,
			       char **p_featureNames)
  : guiAutoDialog(p_parent, p_caption), m_numFeatures(p_settings.Last())
{
  m_features = new wxCheckBox *[m_numFeatures];
  for (int i = 1; i <= m_numFeatures; i++) {
    m_features[i - 1] = new wxCheckBox(this, 0, p_featureNames[i]);
    m_features[i - 1]->SetValue(p_settings[i]);
  }

  m_features[0]->SetConstraints(new wxLayoutConstraints);
  m_features[0]->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_features[0]->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_features[0]->GetConstraints()->width.AsIs();
  m_features[0]->GetConstraints()->height.AsIs();

  for (int i = 1; i < m_numFeatures / 2; i++) {
    m_features[i*2]->SetConstraints(new wxLayoutConstraints);
    m_features[i*2]->GetConstraints()->left.SameAs(this, wxLeft, 10);
    m_features[i*2]->GetConstraints()->top.SameAs(m_features[(i-1)*2],
						  wxBottom, 10);
    m_features[i*2]->GetConstraints()->width.AsIs();
    m_features[i*2]->GetConstraints()->height.AsIs();
  }

  for (int i = 0; i < m_numFeatures / 2; i++) {
    m_features[i*2+1]->SetConstraints(new wxLayoutConstraints);
    m_features[i*2+1]->GetConstraints()->left.SameAs(m_features[6],
						     wxRight, 10);
    m_features[i*2+1]->GetConstraints()->top.SameAs(m_features[i*2], wxTop);
    m_features[i*2+1]->GetConstraints()->width.AsIs();
    m_features[i*2+1]->GetConstraints()->height.AsIs();
  }

  m_okButton->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_okButton->GetConstraints()->top.SameAs(m_features[7], wxBottom, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  Go();
}

dialogFeatures::~dialogFeatures()
{
  delete [] m_features;
}

gArray<Bool> dialogFeatures::GetFeatures(void) const
{
  gArray<Bool> ret(m_numFeatures);
  for (int i = 1; i <= m_numFeatures; i++) {
    ret[i] = m_features[i - 1]->GetValue();
  }
  return ret;
}

void NodeSolnShow::SetOptions(void)
{
  dialogFeatures dialog(this, "Node Inspect Features", 
			features, feature_names);

  if (dialog.Completed() == wxOK) {
    gArray<Bool> newFeatures(dialog.GetFeatures());
    
    for (int i = NUM_FEATURES; i >= 1; i--) {
      if (features[i] && !newFeatures[i]) {
	DelRow(Pos(i));
	features[i] = 0;
      }
    }

    for (int i = 1; i <= NUM_FEATURES; i++) {
      if (newFeatures[i] && !features[i]) {
	AddRow(Pos(i)+1);
	SetLabelRow(Pos(i)+1, feature_names[i]);
	features[i] = 1;
      }
    }

    for (int i = 1; i <= NUM_FEATURES; i++)
      wxWriteResource(NODESOLN_SECTION, feature_names[i], 
		      features[i], gambitApp.ResourceFile());

    Redraw();
  }
}

void NodeSolnShow::OnMenuCommand(int id)
{
  switch (id) {
  case EFG_NODE_INSP_OPTIONS: 
    SetOptions();
    break;

  default: 
    SpreadSheet3D::OnMenuCommand(id);
    break;
  }
}


// Overide help system
void NodeSolnShow::OnHelp(int help_type)
{
  if (!help_type) // contents
    wxHelpContents(EFG_INSPECT_HELP);
  else
    wxHelpAbout();
}

Bool NodeSolnShow::OnClose(void)
{
  Show(FALSE);
  return FALSE;
}

void NodeSolnShow::Set(const Node *n) 
{
  cur_n = n;
  
  if (!n) { // no data available
    for (int i = 1; i <= NUM_FEATURES; i++) { 
      if (features[i]) 
	SetCell(Pos(i), 1, "N/A");
    }

    return;
  }

  // if we got here, the node is valid.
  gText tmpstr;

  for (int feature = 1; feature <= NUM_FEATURES; feature++) {
    if (features[feature]) {
      try {
	switch(feature) {
	case INFOSET:
	  if (!n->GetPlayer()) {
	    tmpstr = "TERMINAL";
	  }
	  else {
	    if (n->GetPlayer()->IsChance())
	      tmpstr = "CHANCE";
	    else
	      tmpstr = ("(" + ToText(n->GetPlayer()->GetNumber()) + "," +
			ToText(n->GetInfoset()->GetNumber()) + ")");
	  }
	  
	  SetCell(Pos(feature), 1, tmpstr);
	  break;

	case REALIZPROB: 
	  SetCell(Pos(feature), 1, parent->AsString(tRealizProb, n));
	  break;
	  
	case ISETPROB: 
	  SetCell(Pos(feature), 1, parent->AsString(tIsetProb, n));
	  break;

	case BELIEFPROB: 
	  SetCell(Pos(feature), 1, parent->AsString(tBeliefProb, n));
	  break;

	case NODEVALUE:   
	  SetCell(Pos(feature), 1, parent->AsString(tNodeValue, n));
	  break;

	case ISETVALUE:   
	  SetCell(Pos(feature), 1, parent->AsString(tIsetValue, n));
	  break;
	
	case BRANCHPROB:  {  // figure out which branch # I am.
	  Node *p = n->GetParent();
	  
	  if (p) {
	    int branch = 1;

	    for (int i = 1; i <= p->Game()->NumChildren(p); i++) {
	      if (p->GetChild(i) == n) 
		branch = i;
	    }

	    SetCell(Pos(feature), 1, parent->AsString(tBranchProb, p, branch));
	  }
	  else {
	    SetCell(Pos(feature), 1, ToText(1));
	  }
	
	  break;
	}
	
	case BRANCHVALUE: {
	  Node *p = n->GetParent();
	  
	  if (p) {
	    int branch = 1;
	    
	    for (int i = 1; i <= p->Game()->NumChildren(p); i++) {
	      if (p->GetChild(i) == n) 
		branch = i;
	    }

	    SetCell(Pos(feature), 1, parent->AsString(tBranchVal, p, branch));
	  }
	  else {
	    SetCell(Pos(feature), 1, "N/A");
	  }
	
	  break;
	}

	default: 
	  assert(0 && "Invalid feature for node solndisp");
	  break;
	}
      }
      catch (gNumber::DivideByZero &) { }
    }
  }
  
  Repaint();
}


void NodeSolnShow::OnOptionsChanged(unsigned int options)
{
  if (options & S_PREC_CHANGED) {
    Set(cur_n);
    int width = ((features[NODEVALUE]) ? 2+num_players*(4+parent->NumDecimals()) : 
		 (2+parent->NumDecimals()));
    DrawSettings()->SetColWidth(width);
    Resize();
    Repaint();
  }
}


//****************************************************************************
//                   SORTER FILTER OPTIONS DIALOG
//****************************************************************************

class BSolnSortFilterDialog:public wxDialogBox
{
private:
    BSolnSortFilterOptions &options;
    wxRadioBox *sortby;
    wxListBox *filter_creator, *filter_nash, *filter_perfect, *filter_seq;

    static void ok_func(wxButton &ob, wxEvent &)
    { ((BSolnSortFilterDialog *)ob.GetClientData())->OnOk(); }

    static void cancel_func(wxButton &ob, wxEvent &)
    { ((BSolnSortFilterDialog *)ob.GetClientData())->OnCancel(); }

    static void help_func(wxButton &, wxEvent &)
    { wxHelpContents(EFG_SOLN_SORT_HELP); }

    void OnOk(void);
    void OnCancel(void);
    int completed;

public:
    BSolnSortFilterDialog(BSolnSortFilterOptions &options);
    int Completed(void);
};


BSolnSortFilterDialog::BSolnSortFilterDialog(BSolnSortFilterOptions &options_)
    : wxDialogBox(0, "Sort & Filter", TRUE), options(options_) 
{
    SetLabelPosition(wxVERTICAL);
    char *sort_by_str[] =
    { "Id", "Creator", "Nash", "Perfect", "Sequential", "G Value", "G Lambda", "L Value" };

    sortby = new wxRadioBox(this, 0, "Sort By", -1, -1, -1, -1, 8, 
                            sort_by_str, 2, wxVERTICAL, "sortby");
    NewLine();
    NewLine();
    (void)new wxGroupBox(this, "Filter By", 12, 95, 464, 150, 0, "gbox");
    filter_creator = new wxListBox(this, 0, "Creator", wxMULTIPLE,
                                   55, -1, 140, 85, NUM_BCREATORS, options.filter_cr_str+1);
    filter_nash = new wxListBox(this, 0, "Nash", wxMULTIPLE, 
                                -1, -1, 44, 85, 3, options.filter_tri_str+1);
    filter_perfect = new wxListBox(this, 0, "Perfect", wxMULTIPLE, 
                                   -1, -1, 56, 85, 3, options.filter_tri_str+1);
    filter_seq = new wxListBox(this, 0, "Sequential", wxMULTIPLE, 
                               -1, -1, 78, 85, 3, options.filter_tri_str+1);

    NewLine();
    wxButton *ok = new wxButton(this, (wxFunction)ok_func, "Ok");
    ok->SetClientData((char *)this);
    wxButton *cancel = new wxButton(this, (wxFunction)cancel_func, "Cancel");
    cancel->SetClientData((char *)this);
    wxButton *help = new wxButton(this, (wxFunction)help_func, "Help");
    help->SetClientData((char *)this);
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
// Note: buttons should be specified in the constructor, but due to a
// bug in wx_motif, they fail to appear.  So, we create them later by
// calling MakeButtons.  Fix this once wx_motif is fixed!

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

EfgSolnShow::EfgSolnShow(const Efg &ef_, BehavSolutionList &soln,
                         int cur_soln_, const GambitDrawSettings &draw_settings_,
                         BSolnSortFilterOptions &sf_options_,
                         EfgShow *parent_, unsigned int opts_)
    : SpreadSheet3D(1+soln.Length()*ef_.NumPlayerInfosets(), 4, 1, 2, "Solutions", parent_, ANY_BUTTON),
      ef(ef_), parent(parent_), gamb_draw_settings(draw_settings_),
      solns(soln), dim(ef_.NumActions()),
      num_players(ef_.NumPlayers()), num_isets(ef_.NumPlayerInfosets()),
      num_solutions(soln.Length()), cur_soln(cur_soln_),
      features(0, BSOLN_NUM_FEATURES-1), opts(opts_),
      sf_options(sf_options_)
{
    int i;
    Show(FALSE);
    SetEditable(FALSE);
    // figure out the max # of actions in any infoset
    int max_actions = 0;

    for (i = 1; i <= num_players; i++)
        for (int j = 1; j <= dim.Lengths()[i]; j++) 
            max_actions = gmax(dim(i, j), max_actions);

    if (cur_soln)
    {
        HiLighted(SolnPos(cur_soln), 1, 0, TRUE);
        SetCurRow(SolnPos(cur_soln));
    }


    // Give the frame an icon
    wxIcon *frame_icon;

#ifdef wx_msw
    frame_icon = new wxIcon("efg_icn");
#else
#include "efg.xbm"
    frame_icon = new wxIcon(efg_bits, efg_width, efg_height);
#endif

    SetIcon(frame_icon);
    // Read in the default features
    char *defaults_file = gambitApp.ResourceFile();

    if (opts&BSOLN_O_OPTIONS)
    {
        for (i = 0; i < BSOLN_NUM_FEATURES; i++)
        {
            features[i] = 0;
            Bool tmp;
            wxGetResource(BSOLN_SHOW_SECT, feature_names[i], &tmp, defaults_file);

            if (tmp && feature_width[i])
            {
                int col = FeaturePos(i)+1;
                AddCol(col);
                SetCell(1, col, feature_names[i]);
                Bold(1, col, 0, TRUE);

                if (feature_width[i] == -1) // precision dependent
                    DrawSettings()->SetColWidth(2+parent->NumDecimals(), col);
                else                                            // precision independent
                    DrawSettings()->SetColWidth(feature_width[i], col);
            }
            features[i] = tmp;
        }
    }
    else
    {
        for (i = 0; i < BSOLN_NUM_FEATURES; i++) features[i] = 0;
    }

    DrawSettings()->SetColWidth(4, FeaturePos(BSOLN_ID));      // Id # "Id" = 3 chars
    DrawSettings()->SetColWidth(8, FeaturePos(BSOLN_PLAYER));  // Player name "Player #" = 8 chars
    DrawSettings()->SetColWidth(5, FeaturePos(BSOLN_ISET));    // Iset name (assume 5 letters average);
    DrawSettings()->SetColWidth(gmin(max_actions*(2+parent->NumDecimals())+4, 
                                     MAX_SOLNSHOW_WIDTH), FeaturePos(BSOLN_DATA));
    SetCell(1, FeaturePos(BSOLN_ID), "ID");
    Bold(FeaturePos(BSOLN_ID), 1, 0, TRUE);
    SetCell(1, FeaturePos(BSOLN_PLAYER), "Player");
    Bold(1, FeaturePos(BSOLN_PLAYER), 0, TRUE);
    SetCell(1, FeaturePos(BSOLN_ISET), "Iset");
    Bold(1, FeaturePos(BSOLN_ISET), 0, TRUE);

    // Create all the buttons
    MakeButtons(OK_BUTTON|PRINT_BUTTON|OPTIONS_BUTTON|HELP_BUTTON);

    if (opts & BSOLN_O_OPTIONS)
        AddButton("Opt", (wxFunction)settings_button);

    if (opts & BSOLN_O_EFGNFG)
    {
        wxButton *extensive_button = AddButton("EF->NF", (wxFunction)normal_button);
        //if (parent)
        //  if (!parent->InterfaceOk()) extensive_button->Enable(FALSE);
        //if (!parent) extensive_button->Enable(FALSE);
        extensive_button->Enable(FALSE); // NOT IMPLEMENTED
    }

    if (opts & BSOLN_O_SORTFILT)
        AddButton("Sort/Filter", (wxFunction)EfgSolnShow::sortfilt_button);

    if (opts&BSOLN_O_EDIT)  // see the header file
    {
        Panel()->NewLine();
        AddButton("Add", (wxFunction)EfgSolnShow::add_button);
        AddButton("Edit", (wxFunction)EfgSolnShow::edit_button);
        AddButton("Delete", (wxFunction)EfgSolnShow::delete_button);
        AddButton("Delete All", (wxFunction)EfgSolnShow::delete_all_button);
    }

    SortFilter(false);
    UpdateValues();
    Redraw();
    Show(TRUE);
}


Bool EfgSolnShow::HilightInfoset(int pl, int iset)
{
    int row = 2;

    for (int i = 1; i <= num_solutions; i++)
        for (int j = 1; j <= num_players; j++)
            for (int k = 1; k <= dim.Lengths()[j]; k++, row++)
                HiLighted(row, 3+features[BSOLN_EQUVALS], 1, (pl == j && iset == k) ? TRUE : FALSE);

    Repaint();
    return FALSE;
}


// Overide help system
void EfgSolnShow::OnHelp(int help_type)
{
    if (!help_type) // contents
        wxHelpContents(EFG_INSPECT_HELP);
    else
        wxHelpAbout();
}


int EfgSolnShow::FeaturePos(int feature)
{
    switch (feature)
    {
        // These features are always on and always in the same columns
    case BSOLN_ID: 
        return 1;

    case BSOLN_PLAYER: 
        return 2;

    case BSOLN_ISET: 
        return 3;

        // The data is always the LAST column
    case BSOLN_DATA: 
        return (FeaturePos(BSOLN_NUM_FEATURES-1)+1);

        // The rest of the features are optional and may change location
    default:
    {
        int pos = 3;

        for (int i = 0; i <= feature; i++)
        {
            if (features[i] && feature_width[i]) 
                pos++;
        }

        return pos;
    }
    }
}


void EfgSolnShow::SetOptions(void)
{
  dialogFeatures dialog(this, "Settings", features, feature_names);

  if (dialog.Completed() == wxOK) {
    gArray<Bool> newFeatures(dialog.GetFeatures());

    for (int i = BSOLN_NUM_FEATURES-1; i > 0; i--) {
      if (features[i] && !newFeatures[i]) {
	if (feature_width[i]) 
	  DelCol(FeaturePos(i));

	features[i] = 0;
      }
    }

    for (int i = 1; i < BSOLN_NUM_FEATURES; i++) {
      if (newFeatures[i] && !features[i]) {
	if (feature_width[i]) {
	  int col = FeaturePos(i)+1;
	  AddCol(col);
	  SetCell(1, col, feature_names[i]);
	  Bold(1, col, 0, TRUE);

	  if (feature_width[i] == -1) // precision dependent
	    DrawSettings()->SetColWidth(2+parent->NumDecimals(), col);
	  else                        // precision independent
	    DrawSettings()->SetColWidth(feature_width[i], col);
	}
	features[i] = 1;
      }
    }

    for (int i = 0; i < BSOLN_NUM_FEATURES; i++) {
      wxWriteResource(BSOLN_SHOW_SECT, feature_names[i], 
		      features[i], gambitApp.ResourceFile());
    }

    UpdateValues();
    Repaint();
  }
}


void EfgSolnShow::OnOptionsChanged(unsigned int options)
{
    if (options&S_PREC_CHANGED)
    {
        int max_actions = 0;

        for (int i = 1; i <= num_players; i++)
            for (int j = 1; j <= dim.Lengths()[i]; j++) 
                max_actions = gmax(dim(i, j), max_actions);

        DrawSettings()->SetColWidth(gmin(max_actions*(2+parent->NumDecimals())+4, 
                                         MAX_SOLNSHOW_WIDTH), FeaturePos(BSOLN_DATA));
        UpdateValues();
        Resize();
        Repaint();
    }
}


void EfgSolnShow::settings_button(wxButton &ob, wxEvent &)
{
    ((EfgSolnShow *)ob.GetClientData())->SetOptions();
}


int EfgSolnShow::SolnNum(int row)
{
    assert(row > 0 && row <= GetRows());

    if (row == 1) return 0;     // clicking on title row unselects any solutions
    // figure out which solution was clicked on
    int new_soln = (row-2) / num_isets + 1;
    assert(new_soln >= 1 && new_soln <= num_solutions);
    return new_soln;
}


int EfgSolnShow::SolnPos(int soln_num)
{
    assert(soln_num >= 0 && soln_num <= num_solutions);
    return (2+((soln_num) ? (soln_num-1)*num_isets : -1));
}


#include "legendc.h" // needed for NODE_BELOW for iset_label
void EfgSolnShow::UpdateValues(void)
{
    gText tmp_str;
    int cur_pos = 2;

    for (int i = 1; i <= num_solutions; i++)
    {
        const BehavSolution &cur = solns[i];
        SetCell(cur_pos, FeaturePos(BSOLN_ID), ToText((int)cur.Id()));

        if (features[BSOLN_CREATOR])
            SetCell(cur_pos, FeaturePos(BSOLN_CREATOR), NameEfgAlgType(cur.Creator()));

        if (features[BSOLN_ISNASH])
            SetCell(cur_pos, FeaturePos(BSOLN_ISNASH), Name(cur.IsNash()));

        if (features[BSOLN_ISPERF])
            SetCell(cur_pos, FeaturePos(BSOLN_ISPERF), Name(cur.IsSubgamePerfect()));

        if (features[BSOLN_ISSEQ])
            SetCell(cur_pos, FeaturePos(BSOLN_ISSEQ), Name(cur.IsSequential()));

        if (features[BSOLN_GLAMBDA]) {
	  if (cur.Creator() == algorithmEfg_QRE_EFG ||
	      cur.Creator() == algorithmEfg_QRE_NFG)
	    SetCell(cur_pos, FeaturePos(BSOLN_GLAMBDA), ToText(cur.QreLambda(), parent->NumDecimals()));
	  else
	    SetCell(cur_pos, FeaturePos(BSOLN_GLAMBDA), "---------");
        }

        if (features[BSOLN_GVALUE]) {
	  if (cur.Creator() == algorithmEfg_QRE_EFG ||
	      cur.Creator() == algorithmEfg_QRE_NFG)
	    SetCell(cur_pos, FeaturePos(BSOLN_GVALUE), ToText(cur.QreValue(), parent->NumDecimals()));
	  else
	    SetCell(cur_pos, FeaturePos(BSOLN_GVALUE), "---------");
        }

        if (features[BSOLN_LVALUE])
            SetCell(cur_pos, FeaturePos(BSOLN_LVALUE), ToText(cur.LiapValue(), parent->NumDecimals()));

        for (int j = 1; j <= num_players; j++) {
	  if (dim.Lengths()[j] == 0) continue;
	  SetCell(cur_pos, FeaturePos(BSOLN_PLAYER),
		  cur.Game().Players()[j]->GetName()); // print the player 's

	  if (features[BSOLN_EQUVALS])        // print equ values if requested
	    SetCell(cur_pos, FeaturePos(BSOLN_EQUVALS), ToText(cur.Payoff(j), parent->NumDecimals()));
	  
	  for (int k = 1; k <= dim.Lengths()[j]; k++) {
	    // Display ISET in the same format as that selected for the main tree
	    // display below the node.  That is, either the infoset name or the
	    // infoset id.  Check the TreeDrawSettings for the current value.
	    gText iset_label;

	    if (parent->tw->DrawSettings().LabelNodeBelow() == NODE_BELOW_ISETID)
	      iset_label = "("+ToText(j)+","+ToText(k)+")";
	    else
	      iset_label = cur.Game().Players()[j]->Infosets()[k]->GetName();

	    SetCell(cur_pos, FeaturePos(BSOLN_ISET), iset_label);   // print the infoset #s
	    tmp_str = "\\C{"+ToText(gamb_draw_settings.GetPlayerColor(j))+"}{";
	    
	    for (int l = 1; l <= dim(j, k); l++)   {  // print actual values
	      Action *action = cur.Game().Players()[j]->Infosets()[k]->Actions()[l];
	      tmp_str += (ToText(cur(action))+((l == dim(j, k)) ? "}" : ","));
	    }
	    
	    SetCell(cur_pos, FeaturePos(BSOLN_DATA), tmp_str);
	    cur_pos++;
	  }
        }
    }
}


void EfgSolnShow::UpdateSoln(int row, int col)
{
    // passing -1 for the col indicates the row is the new solution #
    int new_soln;

    if (col != -1) 
        new_soln = SolnNum(row);
    else
        new_soln = row;

    if (new_soln != cur_soln)
    {
        // remove the old hilight, hilight the new cell
        HiLighted(SolnPos(cur_soln), 1, 0, FALSE);
        cur_soln = new_soln;
        HiLighted(SolnPos(cur_soln), 1, 0, TRUE);

        // inform the parent
        if (parent) 
            parent->ChangeSolution(cur_soln);

        Repaint();
    }
}


void EfgSolnShow::SolutionToNormal(void)
{
    assert(0);
    /*  THIS FUNCTION IS NOT YET IMPLEMENTED, AND MAY NEVER BE
    assert(parent);     // we must have a parent if we got here
    assert(parent->InterfaceOk());  // and it must have someone to send solutions to
    int new_soln = SolnNum(CurRow());
    parent->SolutionToExtensive(solns[new_soln]);
    */
}


// OnClose -- Close the window, as if OK was pressed
Bool EfgSolnShow:: OnClose(void)
{
  OnOk();
  return FALSE;
}


void EfgSolnShow::OnDoubleClick(int row, int col, int , const gText &)
{
    if (col == FeaturePos(BSOLN_ID))
        UpdateSoln(row, col);      // change solution

    if (col == FeaturePos(BSOLN_ISET)) // hilight infoset
    {
        int pl = 1, iset = 1;

        if (row == 1)
        {
            pl = 0;
            iset = 0;
        }
        else
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

        parent->HilightInfoset(pl, iset, 2);
    }

    if (col == FeaturePos(BSOLN_DATA))  // edit solution
    {
        SetCurRow(row);
        SetCurCol(col);
        OnEdit();
    }

    // Double clicking on the first row in any optional feature col, sorts by that col
    if (row == 1)
    {
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


void EfgSolnShow::OnSelectedMoved(int row, int col, SpreadMoveDir how)
{
    if (col == FeaturePos(BSOLN_ID))
    {
        int soln_num, prow = row;

        if (how == SpreadMoveLeft || how == SpreadMoveJump)
        {
            soln_num = SolnNum(row);
            prow = SolnPos(soln_num);
        }

        if (how == SpreadMoveUp)
        {
            soln_num = SolnNum(row+1);
            prow = SolnPos(soln_num-1);
        }

        if (how == SpreadMoveDown)
        {
            soln_num = SolnNum(row-1);
            prow = SolnPos((soln_num < num_solutions) ? soln_num+1 : num_solutions);
        }

        if (row != prow)
        {
            row = prow;
            SetCurRow(row);
        }
    }

    if (features[BSOLN_DYNAMIC] && parent) 
        UpdateSoln(row, col);
}


void EfgSolnShow::OnOk(void)
{
    Show(FALSE);

    if (parent) 
        parent->InspectSolutions(DESTROY_DIALOG);

    delete this;
}


// On Sort Filter

void EfgSolnShow::sortfilt_button(wxButton &ob, wxEvent &)
{
    ((EfgSolnShow *)ob.GetClientData())->SortFilter();
}


void EfgSolnShow::SortFilter(bool inter)
{
    int completed;

    if (inter)  // interactive or automatic
    {
        BSolnSortFilterDialog D(sf_options);
        completed = D.Completed();
    }
    else
    {
        completed = wxOK;
    }

    if (completed == wxOK)
    {
        int old_num_sol = num_solutions;  // current state
        const BehavSolution *cur_solnp = 0;

        if (cur_soln) 
            cur_solnp = &solns[cur_soln];

        BSolnSorterFilter SF(solns, sf_options); 
        int new_num_sol = solns.VisibleLength();    // new state
        int i, j;
        int new_soln = 0;                           // try to find the new pos of cur_soln

        for (i = 1; i <= solns.VisibleLength(); i++) 
        {
            if (cur_solnp == &solns[i]) 
                new_soln = i;
        }

        if (old_num_sol > new_num_sol)
        {
            for (i = old_num_sol; i > new_num_sol; i--)
                for (j = 1; j <= num_isets; j++)
                    DelRow();
        }

        if (old_num_sol < new_num_sol)
        {
            for (i = old_num_sol+1; i <= new_num_sol; i++)
                for (j = 1; j <= num_isets; j++)
                    AddRow();
        }

        num_solutions = solns.VisibleLength();
        UpdateValues();

        // make sure we do not try to access non-displayed solutions
        if (cur_soln > num_solutions) 
            cur_soln = 0;

        UpdateSoln(new_soln, -1);
        SetCurRow(SolnPos(new_soln));
        SetCurCol(1);

        if (old_num_sol != new_num_sol) 
            Redraw();

        Repaint();
    }
}


// OnRemove

void EfgSolnShow::OnRemove(bool all)
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
}


// OnAdd
void EfgSolnShow::OnAdd(void)
{
    BehavSolution temp_soln(parent->CreateSolution());
    BehavSolnEdit *add_dialog =
        new BehavSolnEdit(temp_soln, 
                          parent->tw->DrawSettings().LabelNodeBelow(), 
                          this, parent->NumDecimals());

    Enable(FALSE);  // disable this window until the edit window is closed

    while (add_dialog->Completed() == wxRUNNING) 
        wxYield();

    Enable(TRUE);
    CanvasFocus();

    if (add_dialog->Completed() == wxOK)
    {
        solns.Append(temp_soln);
        SortFilter(false);
    }

    delete add_dialog;
}


// OnEdit
void EfgSolnShow::OnEdit(void)
{
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
                          parent->tw->DrawSettings().LabelNodeBelow(), 
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
}



void EfgSolnShow::normal_button(wxButton &, wxEvent &)
{
    //((EfgSolnShow *)ob.GetClientData())->SolutionToNormal();
}


void EfgSolnShow::add_button(wxButton &ob, wxEvent &)
{
    ((EfgSolnShow *)ob.GetClientData())->OnAdd();
}


void EfgSolnShow::edit_button(wxButton &ob, wxEvent &)
{
    ((EfgSolnShow *)ob.GetClientData())->OnEdit();
}



void EfgSolnShow::delete_button(wxButton &ob, wxEvent &)
{
    ((EfgSolnShow *)ob.GetClientData())->OnRemove(false);
}


void EfgSolnShow::delete_all_button(wxButton &ob, wxEvent &)
{
    ((EfgSolnShow *)ob.GetClientData())->OnRemove(true);
}


/*****************************************************************************
 *                           BEHAV SOLUTION EDIT
 *****************************************************************************/

// Constructor

BehavSolnEdit::BehavSolnEdit(BehavSolution &soln_,
                             int iset_disp, wxFrame *parent,
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

Bool BehavSolnEdit::OnClose(void)
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
    char *defaults_file = gambitApp.ResourceFile();
    wxGetResource(SOLN_SECT, "Efg-Interactive-Solns-All", &pick_all, defaults_file);
    Panel()->NewLine();
    //wxCheckBox *pick_all_box = new wxCheckBox(Panel(), (wxFunction)pick_all_func, "All");
    pick_all_button = new wxButton(Panel(), (wxFunction)pick_all_func, "     ");
    pick_all_button->SetClientData((char *)this);
    (void)new wxMessage(Panel(), "       Double click on a solution # to toggle it");
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


