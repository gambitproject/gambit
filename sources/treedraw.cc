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
  wxRadioBox *m_infosetLines;

public:
  dialogTreeOptions(const TreeDrawSettings &p_settings, wxWindow *p_parent);
  virtual ~dialogTreeOptions() { }

  int BranchLength(void) const { return m_branchLength->GetValue(); }
  int NodeLength(void) const { return m_nodeLength->GetValue(); }
  int ForkLength(void) const { return m_forkLength->GetValue(); }
  int YSpacing(void) const { return m_ySpacing->GetValue(); }

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

  char *lineChoices[] = { "None", "Same Level", "All Levels" };
  m_infosetLines = new wxRadioBox(this, 0, "Show Infoset Lines",
				  1, 1, -1, -1, 3, lineChoices);
  m_infosetLines->SetSelection(p_settings.ShowInfosets());
  m_infosetLines->SetConstraints(new wxLayoutConstraints);
  m_infosetLines->GetConstraints()->left.SameAs(layoutGroup, wxLeft);
  m_infosetLines->GetConstraints()->top.SameAs(layoutGroup, wxBottom, 10);
  m_infosetLines->GetConstraints()->width.AsIs();
  m_infosetLines->GetConstraints()->height.AsIs();

  m_okButton->GetConstraints()->top.SameAs(m_infosetLines, wxBottom, 10);
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
    show_infosets = dialog.InfosetStyle();
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

class dialogLegends : public guiAutoDialog {
private:
  wxChoice *m_nodeAbove, *m_nodeBelow, *m_nodeAfter;
  wxChoice *m_branchAbove, *m_branchBelow;

public:
  dialogLegends(const TreeDrawSettings &, wxWindow *);
  virtual ~dialogLegends() { }

  int GetNodeAbove(void) const { return m_nodeAbove->GetSelection(); }
  int GetNodeBelow(void) const { return m_nodeBelow->GetSelection(); }
  int GetNodeAfter(void) const { return m_nodeAfter->GetSelection(); }

  int GetBranchAbove(void) const { return m_branchAbove->GetSelection(); }
  int GetBranchBelow(void) const { return m_branchBelow->GetSelection(); }
};

dialogLegends::dialogLegends(const TreeDrawSettings &p_options,
			     wxWindow *p_parent)
  : guiAutoDialog(p_parent, "Legends")
{
  wxGroupBox *nodeGroup = new wxGroupBox(this, "Nodes");

  char *nodeLabelList[] = { "Nothing", "Node Label", "Player",
			    "Infoset Label", "Infoset ID",
			    "Outcome", "Realiz Prob", "Belief Prob",
			    "Value" };
  m_nodeAbove = new wxChoice(this, 0, "Above", 1, 1, -1, -1,
			     9, nodeLabelList);
  m_nodeAbove->SetSelection(p_options.LabelNodeAbove());
  m_nodeAbove->SetConstraints(new wxLayoutConstraints);
  m_nodeAbove->GetConstraints()->top.SameAs(nodeGroup, wxTop, 20);
  m_nodeAbove->GetConstraints()->left.SameAs(nodeGroup, wxLeft, 10);
  m_nodeAbove->GetConstraints()->width.AsIs();
  m_nodeAbove->GetConstraints()->height.AsIs();

  m_nodeBelow = new wxChoice(this, 0, "Below", 1, 1, -1, -1,
			     9, nodeLabelList);
  m_nodeBelow->SetSelection(p_options.LabelNodeBelow());
  m_nodeBelow->SetConstraints(new wxLayoutConstraints);
  m_nodeBelow->GetConstraints()->top.SameAs(m_nodeAbove, wxBottom, 10);
  m_nodeBelow->GetConstraints()->left.SameAs(m_nodeAbove, wxLeft);
  m_nodeBelow->GetConstraints()->width.AsIs();
  m_nodeBelow->GetConstraints()->height.AsIs();

  char *nodeAfterList[] = { "Nothing", "Payoffs", "Name" };
  m_nodeAfter = new wxChoice(this, 0, "After", 1, 1, -1, -1,
			     3, nodeAfterList);
  m_nodeAfter->SetSelection(p_options.LabelNodeRight());
  m_nodeAfter->SetConstraints(new wxLayoutConstraints);
  m_nodeAfter->GetConstraints()->top.SameAs(m_nodeBelow, wxBottom, 10);
  m_nodeAfter->GetConstraints()->left.SameAs(m_nodeBelow, wxLeft);
  m_nodeAfter->GetConstraints()->width.AsIs();
  m_nodeAfter->GetConstraints()->height.AsIs();

  nodeGroup->SetConstraints(new wxLayoutConstraints);
  nodeGroup->GetConstraints()->top.SameAs(this, wxTop, 10);
  nodeGroup->GetConstraints()->bottom.SameAs(m_nodeAfter, wxBottom, -10);
  nodeGroup->GetConstraints()->left.SameAs(this, wxLeft, 10);
  nodeGroup->GetConstraints()->right.SameAs(m_nodeAbove, wxRight, -10); 

  wxGroupBox *branchGroup = new wxGroupBox(this, "Branches");

  char *branchLabelList[] = { "Nothing", "Label", "Player", "Probs", "Value" };
  m_branchAbove = new wxChoice(this, 0, "Above", 1, 1, -1, -1,
			       5, branchLabelList);
  m_branchAbove->SetSelection(p_options.LabelBranchAbove());
  m_branchAbove->SetConstraints(new wxLayoutConstraints);
  m_branchAbove->GetConstraints()->top.SameAs(branchGroup, wxTop, 20);
  m_branchAbove->GetConstraints()->left.SameAs(branchGroup, wxLeft, 10);
  m_branchAbove->GetConstraints()->width.AsIs();
  m_branchAbove->GetConstraints()->height.AsIs();

  m_branchBelow = new wxChoice(this, 0, "Below", 1, 1, -1, -1,
			       5, branchLabelList);
  m_branchBelow->SetSelection(p_options.LabelBranchBelow());
  m_branchBelow->SetConstraints(new wxLayoutConstraints);
  m_branchBelow->GetConstraints()->top.SameAs(m_branchAbove, wxBottom, 10);
  m_branchBelow->GetConstraints()->left.SameAs(m_branchAbove, wxLeft);
  m_branchBelow->GetConstraints()->width.AsIs();
  m_branchBelow->GetConstraints()->height.AsIs();

  branchGroup->SetConstraints(new wxLayoutConstraints);
  branchGroup->GetConstraints()->top.SameAs(nodeGroup, wxTop);
  branchGroup->GetConstraints()->bottom.SameAs(m_branchBelow, wxBottom, -10);
  branchGroup->GetConstraints()->left.SameAs(nodeGroup, wxRight, 10);
  branchGroup->GetConstraints()->right.SameAs(m_branchAbove, wxRight, -10);

  m_okButton->GetConstraints()->top.SameAs(nodeGroup, wxBottom, 10);
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

  Go();
}

void TreeDrawSettings::SetLegends(void)
{
  dialogLegends dialog(*this, 0);

  if (dialog.Completed() == wxOK) {
    SetLabelNodeAbove(dialog.GetNodeAbove());
    SetLabelNodeBelow(dialog.GetNodeBelow());
    SetLabelNodeRight(dialog.GetNodeAfter());
    SetLabelBranchAbove(dialog.GetBranchAbove());
    SetLabelBranchBelow(dialog.GetBranchBelow());
  }
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
