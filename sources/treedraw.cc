//
// FILE: treedraw.cc -- Display configuration class for the extensive form
//
// $Id$
//

#include "wx.h"
#include "wx_form.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif // __BORLANDC__

#include "wxmisc.h"
#include "gambit.h"
#include "treedraw.h"
#include "legendc.h"

TreeDrawSettings::TreeDrawSettings(void)
  : x_origin(0), y_origin(0), 
    node_above_font(NULL), node_below_font(NULL), node_right_font(NULL),
    branch_above_font(NULL), branch_below_font(NULL),
    zoom_factor(1.0)
{
  LoadOptions(INIFILE);
}

class dialogTreeOptions : public guiAutoDialog {
private:
  wxSlider *m_branchLength, *m_nodeLength, *m_forkLength, *m_ySpacing;
  wxSlider *m_decimals;
  wxCheckBox *m_flashingCursor, *m_coloredOutcomes;
  wxRadioBox *m_infosetLines;

public:
  dialogTreeOptions(const TreeDrawSettings &p_settings, wxWindow *p_parent);
  virtual ~dialogTreeOptions() { }

  int BranchLength(void) const { return m_branchLength->GetValue(); }
  int NodeLength(void) const { return m_nodeLength->GetValue(); }
  int ForkLength(void) const { return m_forkLength->GetValue(); }
  int YSpacing(void) const { return m_ySpacing->GetValue(); }
  int Decimals(void) const { return m_decimals->GetValue(); }

  bool FlashingCursor(void) const { return m_flashingCursor->GetValue(); }
  bool ColoredOutcomes(void) const { return m_coloredOutcomes->GetValue(); }
  
  int InfosetStyle(void) const { return m_infosetLines->GetSelection(); }
};

dialogTreeOptions::dialogTreeOptions(const TreeDrawSettings &p_settings,
				     wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Draw Options")
{
  m_branchLength = new wxSlider(this, 0, "Branch Length",
				p_settings.BranchLength(),
				BRANCH_LENGTH_MIN, BRANCH_LENGTH_MAX,
				250, 1, 1);
  m_nodeLength = new wxSlider(this, 0, "Node Length",
			      p_settings.NodeLength(),
			      NODE_LENGTH_MIN, NODE_LENGTH_MAX,
			      250, 1, 1);
  m_forkLength = new wxSlider(this, 0, "Fork Length",
			      p_settings.ForkLength(),
			      FORK_LENGTH_MIN, FORK_LENGTH_MAX,
			      250, 1, 1);
  m_ySpacing = new wxSlider(this, 0, "Vertical Spacing",
			    p_settings.YSpacing(),
			    Y_SPACING_MIN, Y_SPACING_MAX, 250, 1, 1);

  wxGroupBox *layoutGroup = new wxGroupBox(this, "Tree layout parameters");
  layoutGroup->SetConstraints(new wxLayoutConstraints);
  layoutGroup->GetConstraints()->left.SameAs(this, wxLeft, 10);
  layoutGroup->GetConstraints()->top.SameAs(this, wxTop, 10);
  layoutGroup->GetConstraints()->right.SameAs(m_ySpacing, wxRight, -10);
  layoutGroup->GetConstraints()->bottom.SameAs(m_ySpacing, wxBottom, -10);

  m_branchLength->SetConstraints(new wxLayoutConstraints);
  m_branchLength->GetConstraints()->left.SameAs(layoutGroup, wxLeft, 10);
  m_branchLength->GetConstraints()->top.SameAs(layoutGroup, wxTop, 20);
  m_branchLength->GetConstraints()->width.AsIs();
  m_branchLength->GetConstraints()->height.AsIs();

  m_nodeLength->SetConstraints(new wxLayoutConstraints);
  m_nodeLength->GetConstraints()->left.SameAs(m_branchLength, wxLeft);
  m_nodeLength->GetConstraints()->top.SameAs(m_branchLength, wxBottom, 10);
  m_nodeLength->GetConstraints()->width.AsIs();
  m_nodeLength->GetConstraints()->height.AsIs();

  m_forkLength->SetConstraints(new wxLayoutConstraints);
  m_forkLength->GetConstraints()->left.SameAs(m_nodeLength, wxLeft);
  m_forkLength->GetConstraints()->top.SameAs(m_nodeLength, wxBottom, 10);
  m_forkLength->GetConstraints()->width.AsIs();
  m_forkLength->GetConstraints()->height.AsIs();

  m_ySpacing->SetConstraints(new wxLayoutConstraints);
  m_ySpacing->GetConstraints()->left.SameAs(m_forkLength, wxLeft);
  m_ySpacing->GetConstraints()->top.SameAs(m_forkLength, wxBottom, 10);
  m_ySpacing->GetConstraints()->width.AsIs();
  m_ySpacing->GetConstraints()->height.AsIs();

  m_flashingCursor = new wxCheckBox(this, 0, "Flashing Cursor");
  m_flashingCursor->SetValue(p_settings.FlashingCursor());
  m_flashingCursor->SetConstraints(new wxLayoutConstraints);
  m_flashingCursor->GetConstraints()->left.SameAs(m_ySpacing, wxLeft);
  m_flashingCursor->GetConstraints()->top.SameAs(layoutGroup, wxBottom, 10);
  m_flashingCursor->GetConstraints()->width.AsIs();
  m_flashingCursor->GetConstraints()->height.AsIs();

  m_coloredOutcomes = new wxCheckBox(this, 0, "Color-coded Outcomes");
  m_coloredOutcomes->SetValue(p_settings.ColorCodedOutcomes());
  m_coloredOutcomes->SetConstraints(new wxLayoutConstraints);
  m_coloredOutcomes->GetConstraints()->left.SameAs(m_flashingCursor,
						   wxRight, 10);
  m_coloredOutcomes->GetConstraints()->top.SameAs(m_flashingCursor, wxTop);
  m_coloredOutcomes->GetConstraints()->width.AsIs();
  m_coloredOutcomes->GetConstraints()->height.AsIs();

  char *lineChoices[] = { "None", "Same Level", "All Levels" };
  m_infosetLines = new wxRadioBox(this, 0, "Show Infoset Lines",
				  1, 1, -1, -1, 3, lineChoices);
  m_infosetLines->SetSelection(p_settings.ShowInfosets());
  m_infosetLines->SetConstraints(new wxLayoutConstraints);
  m_infosetLines->GetConstraints()->left.SameAs(layoutGroup, wxLeft);
  m_infosetLines->GetConstraints()->top.SameAs(m_flashingCursor, wxBottom, 10);
  m_infosetLines->GetConstraints()->width.AsIs();
  m_infosetLines->GetConstraints()->height.AsIs();

  m_decimals = new wxSlider(this, 0, "Decimal Places",
			    p_settings.NumDecimals(), 0, 16, 250, 1, 1);
  m_decimals->SetConstraints(new wxLayoutConstraints);
  m_decimals->GetConstraints()->left.SameAs(m_infosetLines, wxLeft);
  m_decimals->GetConstraints()->top.SameAs(m_infosetLines, wxBottom, 10);
  m_decimals->GetConstraints()->width.AsIs();
  m_decimals->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->top.SameAs(m_decimals, wxBottom, 10);
  m_okButton->GetConstraints()->right.SameAs(m_cancelButton, wxLeft, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->centreX.SameAs(layoutGroup, wxCentreX);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  Go();
}

void TreeDrawSettings::SetOptions(void)
{
  dialogTreeOptions dialog(*this, 0);

  if (dialog.Completed() == wxOK) {
    branch_length = dialog.BranchLength();
    node_length = dialog.NodeLength();
    fork_length = dialog.ForkLength();
    y_spacing = dialog.YSpacing();
    flashing_cursor = dialog.FlashingCursor();
    color_coded_outcomes = dialog.ColoredOutcomes();
    show_infosets = dialog.InfosetStyle();
    num_prec = dialog.Decimals();
  }
}


// SetLegends
// Selects what get displayed at different points on the tree and in what
// font it will be displayed
void TreeDrawSettings::draw_params_legends_func(wxButton &ob, wxCommandEvent &)
{
    FontDialogBox *f = 0;
    draw_params_legend_struct *dpls = (draw_params_legend_struct *)ob.GetClientData();

    // The following case makes sure that the currently set font comes up first.
    switch (dpls->what_font)
    {
    case NODE_ABOVE_LEGEND: 
        f = new FontDialogBox((wxWindow *)ob.GetParent(), 
                              dpls->draw_settings->NodeAboveFont());
        break;

    case NODE_BELOW_LEGEND: 
        f = new FontDialogBox((wxWindow *)ob.GetParent(), 
                              dpls->draw_settings->NodeBelowFont());
        break;

    case BRANCH_ABOVE_LEGEND: 
        f = new FontDialogBox((wxWindow *)ob.GetParent(), 
                              dpls->draw_settings->BranchAboveFont());
        break;

    case BRANCH_BELOW_LEGEND: 
        f = new FontDialogBox((wxWindow *)ob.GetParent(), 
                              dpls->draw_settings->BranchBelowFont());
        break;

    case NODE_RIGHT_LEGEND: 
        f = new FontDialogBox((wxWindow *)ob.GetParent(), 
                              dpls->draw_settings->NodeRightFont());
        break;
    }

    if (f->Completed() == wxOK)
    {
        wxFont *the_font = f->MakeFont();
        switch (dpls->what_font)
        {
        case NODE_ABOVE_LEGEND: 
            dpls->draw_settings->SetNodeAboveFont(the_font);
            break;

        case NODE_BELOW_LEGEND: 
            dpls->draw_settings->SetNodeBelowFont(the_font);
            break;

        case BRANCH_ABOVE_LEGEND: 
            dpls->draw_settings->SetBranchAboveFont(the_font);
            break;

        case BRANCH_BELOW_LEGEND: 
            dpls->draw_settings->SetBranchBelowFont(the_font);
            break;

        case NODE_RIGHT_LEGEND: 
            dpls->draw_settings->SetNodeRightFont(the_font);
            break;
        }
    }

    delete f;
}


#include "legend.h"
void TreeDrawSettings::SetLegends(void)
{
    int i;
    wxStringList  *node_above_list    = new wxStringList;
    wxStringList  *node_below_list    = new wxStringList;
    wxStringList  *branch_above_list  = new wxStringList;
    wxStringList  *branch_below_list  = new wxStringList;
    wxStringList  *node_right_list    = new wxStringList;
    char          *node_above_str     = new char[20];
    char          *node_below_str     = new char[20];
    char          *branch_above_str   = new char[20];
    char          *branch_below_str   = new char[20];
    char          *node_right_str     = new char[20];
    int            node_above_num;
    int            node_below_num;
    int            branch_above_num;
    int            branch_below_num;
    int            node_right_num;
    wxFont         fixed_font(12, wxMODERN, wxNORMAL, wxNORMAL);

    // Create the appropriate lists from the "legends.h" file.
    i = 0;
    while (node_above_src[i].l_id != -1)
    {
        node_above_list->Add(node_above_src[i].l_name);
        i++;
    }

    i = 0;
    while (node_below_src[i].l_id != -1)
    {
        node_below_list->Add(node_below_src[i].l_name);
        i++;
    }

    i = 0;
    while (branch_above_src[i].l_id != -1)
    {
        branch_above_list->Add(branch_above_src[i].l_name);
        i++;
    }

    i = 0;
    while (branch_below_src[i].l_id != -1)
    {
        branch_below_list->Add(branch_below_src[i].l_name);
        i++;
    }

    i = 0;
    while (node_right_src[i].l_id != -1)
    {
        node_right_list->Add(node_right_src[i].l_name);
        i++;
    }

    // Copy the current settings into the new dialog.
    strcpy(node_above_str,    node_above_src[LabelNodeAbove()].l_name);
    strcpy(node_below_str,    node_below_src[LabelNodeBelow()].l_name);
    strcpy(branch_above_str,  branch_above_src[LabelBranchAbove()].l_name);
    strcpy(branch_below_str,  branch_below_src[LabelBranchBelow()].l_name);
    strcpy(node_right_str,    node_right_src[LabelNodeRight()].l_name);

    // Create the dialog box.
    MyDialogBox *display_legend_dialog = new MyDialogBox(NULL, "Display Legends");
    display_legend_dialog->SetLabelFont(&fixed_font);
    display_legend_dialog->Add(wxMakeFormMessage("                   What to Display"));
    display_legend_dialog->Add(wxMakeFormNewLine());
    display_legend_dialog->Add(wxMakeFormMessage("Above"));
    wxFormItem *node_above_button = 
        wxMakeFormButton("Font", (wxFunction)draw_params_legends_func);
    display_legend_dialog->Add(node_above_button);
    display_legend_dialog->Add(wxMakeFormString(NULL, &node_above_str, wxFORM_CHOICE,
        new wxList(wxMakeConstraintStrings(node_above_list), 0)));
    display_legend_dialog->Add(wxMakeFormString(NULL, &branch_above_str, wxFORM_CHOICE,
        new wxList(wxMakeConstraintStrings(branch_above_list), 0)));
    wxFormItem *branch_above_button = 
        display_legend_dialog->Add(wxMakeFormButton("Font", 
            (wxFunction)draw_params_legends_func));

    display_legend_dialog->Add(wxMakeFormNewLine());
    display_legend_dialog->Add(wxMakeFormMessage("              Node           "));
    display_legend_dialog->Add(wxMakeFormMessage("Branch"));
    display_legend_dialog->Add(wxMakeFormNewLine());
    display_legend_dialog->Add(wxMakeFormMessage("Below"));
    wxFormItem *node_below_button = 
        display_legend_dialog->Add(wxMakeFormButton("Font", 
            (wxFunction)draw_params_legends_func));

    display_legend_dialog->Add(wxMakeFormString(NULL, &node_below_str, wxFORM_CHOICE,
        new wxList(wxMakeConstraintStrings(node_below_list), 0)));
    display_legend_dialog->Add(wxMakeFormString(NULL, &branch_below_str, wxFORM_CHOICE,
        new wxList(wxMakeConstraintStrings(branch_below_list), 0)));
    wxFormItem *branch_below_button = 
        display_legend_dialog->Add(wxMakeFormButton("Font", 
            (wxFunction)draw_params_legends_func));
    display_legend_dialog->Add(wxMakeFormNewLine());
    display_legend_dialog->Add(wxMakeFormString("At right", &node_right_str, 
        wxFORM_CHOICE, new wxList(wxMakeConstraintStrings(node_right_list), 0)));
    wxFormItem *node_right_button = 
        display_legend_dialog->Add(wxMakeFormButton("Font", 
            (wxFunction)draw_params_legends_func));

    display_legend_dialog->AssociatePanel();

    // Set up the font buttons.
    draw_params_legend_struct dpls1 = { NODE_ABOVE_LEGEND, this };
    node_above_button->PanelItem->SetClientData((char *)&dpls1);
    draw_params_legend_struct dpls2 = { NODE_BELOW_LEGEND, this };
    node_below_button->PanelItem->SetClientData((char *)&dpls2);
    draw_params_legend_struct dpls3 = { BRANCH_ABOVE_LEGEND, this };
    branch_above_button->PanelItem->SetClientData((char *)&dpls3);
    draw_params_legend_struct dpls4 = { BRANCH_BELOW_LEGEND, this };
    branch_below_button->PanelItem->SetClientData((char *)&dpls4);
    draw_params_legend_struct dpls6 = { NODE_RIGHT_LEGEND, this };
    node_right_button->PanelItem->SetClientData((char *)&dpls6);

    // Start the dialog.
    display_legend_dialog->Go1();

    // Process the output of the dialog.
    if (display_legend_dialog->Completed() == wxOK)
    {
        node_above_num    = wxListFindString(node_above_list,    node_above_str);
        node_below_num    = wxListFindString(node_below_list,    node_below_str);
        branch_above_num  = wxListFindString(branch_above_list,  branch_above_str);
        branch_below_num  = wxListFindString(branch_below_list,  branch_below_str);
        node_right_num    = wxListFindString(node_right_list,    node_right_str);

        SetLabelNodeAbove(node_above_src[node_above_num].l_id);
        SetLabelNodeBelow(node_below_src[node_below_num].l_id);
        SetLabelBranchAbove(branch_above_src[branch_above_num].l_id);
        SetLabelBranchBelow(branch_below_src[branch_below_num].l_id);
        SetLabelNodeRight(node_right_src[node_right_num].l_id);
    }

    // Clean up.
    delete [] node_above_str;
    delete [] node_below_str;
    delete [] branch_above_str;
    delete [] branch_below_str;
    delete [] node_right_str;
    delete display_legend_dialog;
}


void TreeDrawSettings::SetLegends(int what)
{
    MyDialogBox *d = new MyDialogBox(0, "Display Legend");
    draw_params_legend_struct what_font = { what, this};
    l_struct *legend_src = 0;
    wxStringList *legend_list = new wxStringList;
    char *legend_str = new char[30];
    int legend_id;

    switch (what)
    {
    case NODE_ABOVE_LEGEND: 
        legend_src = node_above_src;
        legend_id = node_above_label;
        break;

    case NODE_BELOW_LEGEND: 
        legend_src = node_below_src;
        legend_id = node_below_label;
        break;

    case BRANCH_ABOVE_LEGEND: 
        legend_src = branch_above_src;
        legend_id = branch_above_label;
        break;

    case BRANCH_BELOW_LEGEND: 
        legend_src = branch_below_src;
        legend_id = branch_below_label;
        break;

    case NODE_RIGHT_LEGEND: 
        legend_src = node_right_src;
        legend_id = node_right_label;
        break;

    default: 
        assert(0 && "Invalid legend");
    }

    int i = 0;
    while (legend_src[i].l_id != -1) 
        legend_list->Add(legend_src[i++].l_name);

    strcpy(legend_str, legend_src[legend_id].l_name); // set the current value
    d->Add(wxMakeFormMessage("Legend For:"));
    d->Add(wxMakeFormMessage(legends_src[what]));
    d->Add(wxMakeFormNewLine());
    d->Add(wxMakeFormString(0, &legend_str, wxFORM_CHOICE,
                            new wxList(wxMakeConstraintStrings(legend_list), 0)));
    wxFormItem *font_button = 
        d->Add(wxMakeFormButton("Font", 
                                (wxFunction)draw_params_legends_func));
    d->AssociatePanel();

    // Set up the font buttons.
    font_button->GetPanelItem()->SetClientData((char *)&what_font);
    d->Go1();

    if (d->Completed() == wxOK)
    {
        int legend_num = wxListFindString(legend_list, legend_str);
        legend_id = legend_src[legend_num].l_id;

        switch (what)
        {
        case NODE_ABOVE_LEGEND: 
            SetLabelNodeAbove(legend_id);
            break;

        case NODE_BELOW_LEGEND: 
            SetLabelNodeBelow(legend_id);
            break;

        case BRANCH_ABOVE_LEGEND: 
            SetLabelBranchAbove(legend_id);
            break;

        case BRANCH_BELOW_LEGEND: 
            SetLabelBranchBelow(legend_id);
            break;

        case NODE_RIGHT_LEGEND: 
            SetLabelNodeRight(legend_id);
            break;

        default: 
            assert(0 && "Invalid legend");
        }
    }

    delete [] legend_str;
    delete d;
}


// Save options.  Uses the resource writing capability of wxwin to create
// the file "gambit.ini" to which to write all the current settings from
// draw_settings.  The file is ASCII and can be edited by hand if necessary.
void TreeDrawSettings::SaveOptions(char *s)
{
    char *file_name = (s) ? s : (char *) INIFILE;
    wxWriteResource("Gambit", "Branch-Length",    branch_length,    file_name);
    wxWriteResource("Gambit", "Node-Length",      node_length,      file_name);
    wxWriteResource("Gambit", "Fork-Length",      fork_length,      file_name);
    wxWriteResource("Gambit", "Outcome-Length",   outcome_length,   file_name);
    wxWriteResource("Gambit", "Y-Spacing",        y_spacing,        file_name);
    wxWriteResource("Gambit", "Chance-Color",     chance_color,     file_name);
    wxWriteResource("Gambit", "Cursor-Color",     cursor_color,     file_name);
    wxWriteResource("Gambit", "Show-Infosets",    show_infosets,    file_name);
    wxWriteResource("Gambit", "Node-Above-Label", node_above_label, file_name);
    wxWriteResource("Gambit", "Node-Below-Label", node_below_label, file_name);
    wxWriteResource("Gambit", "Node-Right-Label", node_right_label, file_name);
    wxWriteResource("Gambit", "Branch-Above-Label",  branch_above_label,  file_name);
    wxWriteResource("Gambit", "Branch-Below-Label",  branch_below_label,  file_name);
    wxWriteResource("Gambit", "Flashing-Cursor",  flashing_cursor,      file_name);
    wxWriteResource("Gambit", "Color-Outcomes",   color_coded_outcomes, file_name);
    wxWriteResource("Gambit", "Root-Reachable",   root_reachable,       file_name);
    wxWriteResource("Gambit", "Output-Precision", num_prec,             file_name);

    // Save the font settings.
    wxWriteResource("Gambit", "Node-Above-Font", 
                    wxFontToString(node_above_font), file_name);
    wxWriteResource("Gambit", "Node-Below-Font", 
                    wxFontToString(node_below_font), file_name);
    wxWriteResource("Gambit", "Branch-Above-Font", 
                    wxFontToString(branch_above_font), file_name);
    wxWriteResource("Gambit", "Branch-Below-Font", 
                    wxFontToString(branch_below_font), file_name);
    wxWriteResource("Gambit", "Node-Right-Font", 
                    wxFontToString(node_right_font), file_name);

    GambitDrawSettings::SaveOptions(file_name);
}

// Load options.  Uses the resource writing capability of wxwin to read from
// the file file_name all the current settings for the
// draw_settings.  The file is ASCII and can be edited by hand if necessary.
void TreeDrawSettings::LoadOptions(char *file_name)
{
    if (!file_name) file_name = INIFILE;
    wxGetResource("Gambit", "Branch-Length",       &branch_length,        file_name);
    wxGetResource("Gambit", "Node-Length",         &node_length,          file_name);
    wxGetResource("Gambit", "Fork-Length",         &fork_length,          file_name);
    wxGetResource("Gambit", "Outcome-Length",      &outcome_length,       file_name);
    wxGetResource("Gambit", "Y-Spacing",           &y_spacing,            file_name);
    wxGetResource("Gambit", "Chance-Color",        &chance_color,         file_name);
    wxGetResource("Gambit", "Cursor-Color",        &cursor_color,         file_name);
    wxGetResource("Gambit", "Show-Infosets",       &show_infosets,        file_name);
    wxGetResource("Gambit", "Node-Above-Label",    &node_above_label,     file_name);
    wxGetResource("Gambit", "Node-Below-Label",    &node_below_label,     file_name);
    wxGetResource("Gambit", "Branch-Above-Label",  &branch_above_label,   file_name);
    wxGetResource("Gambit", "Branch-Below-Label",  &branch_below_label,   file_name);
    wxGetResource("Gambit", "Node-Right-Label",    &node_right_label,     file_name);
    wxGetResource("Gambit", "Flashing-Cursor",     &flashing_cursor,      file_name);
    wxGetResource("Gambit", "Color-Outcomes",      &color_coded_outcomes, file_name);
    wxGetResource("Gambit", "Root-Reachable",      &root_reachable,       file_name);
    wxGetResource("Gambit", "Output-Precision",    &num_prec,             file_name);

    // Load the font settings.
    char *l_tempstr = new char[100];
    wxGetResource("Gambit", "Node-Above-Font", &l_tempstr, file_name);
    SetNodeAboveFont(wxStringToFont(l_tempstr));
    wxGetResource("Gambit", "Node-Below-Font", &l_tempstr, file_name);
    SetNodeBelowFont(wxStringToFont(l_tempstr));
    wxGetResource("Gambit", "Branch-Above-Font", &l_tempstr, file_name);
    SetBranchAboveFont(wxStringToFont(l_tempstr));
    wxGetResource("Gambit", "Branch-Below-Font", &l_tempstr, file_name);
    SetBranchBelowFont(wxStringToFont(l_tempstr));
    wxGetResource("Gambit", "Node-Right-Font", &l_tempstr, file_name);
    SetNodeRightFont(wxStringToFont(l_tempstr));
    GambitDrawSettings::LoadOptions(file_name);
    delete [] l_tempstr;
}


//********************************* SET ZOOM ***********************************
// Set force to true to set a zoom that is outside the MIN_ZOOM, MAX_ZOOM range.
void TreeDrawSettings::SetZoom(float z, bool force)
{
#define MAX_ZOOM    10.0
#define MIN_ZOOM    0.1

    if (z < -0.5) // if this is not an inc/dec action
    {
        float t_zoom_factor = zoom_factor;
        MyDialogBox *zoom_dialog = new MyDialogBox(NULL, "Set Zoom");
        zoom_dialog->Form()->Add(wxMakeFormFloat("Zoom [0.1-10]", 
            &t_zoom_factor, wxFORM_DEFAULT,
            new wxList(wxMakeConstraintRange(MIN_ZOOM, MAX_ZOOM), 0)));
        zoom_dialog->Go();

        if (zoom_dialog->Completed() == wxOK)
            zoom_factor = t_zoom_factor;
        delete zoom_dialog;
    }

    else
    {
        if ((z >= MIN_ZOOM) && (z <= MAX_ZOOM) || force)
            zoom_factor = z;
    }
}
