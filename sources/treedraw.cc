// File: treedraw.cc --
// Contains the configuration class for the extensive form.

// $Id$

#include "wx.h"
#include "wx_form.h"
#pragma hdrstop
#include "wxmisc.h"
#include "treedraw.h"
#include "legendc.h"

TreeDrawSettings::TreeDrawSettings(void)
{
    zoom_factor = 1.0;

    x_origin = 0;
    y_origin = 0;
    node_above_font = NULL;
    node_below_font = NULL;
    node_right_font = NULL;
    branch_above_font  = NULL;
    branch_below_font  = NULL;
    node_terminal_font = NULL;

    LoadOptions(INIFILE);
}

void TreeDrawSettings::SetOptions(void)
{
    wxFont fixed_font(12, wxMODERN, wxNORMAL, wxNORMAL);
    MyDialogBox *tree_options_dialog = new MyDialogBox(NULL, "Draw Options");
    tree_options_dialog->SetLabelFont(&fixed_font);
    tree_options_dialog->Form()->Add(wxMakeFormMessage("Size Settings"));
    tree_options_dialog->Form()->Add(wxMakeFormNewLine());
    tree_options_dialog->Form()->Add(wxMakeFormShort("Branch Length", 
        &branch_length, wxFORM_SLIDER,
        new wxList(wxMakeConstraintRange(BRANCH_LENGTH_MIN, BRANCH_LENGTH_MAX), 0),
        NULL, wxHORIZONTAL));
    tree_options_dialog->Form()->Add(wxMakeFormNewLine());
    tree_options_dialog->Form()->Add(wxMakeFormShort("Node Length  ", 
        &node_length, wxFORM_SLIDER,
        new wxList(wxMakeConstraintRange(NODE_LENGTH_MIN, NODE_LENGTH_MAX), 0),
        NULL, wxHORIZONTAL));
    tree_options_dialog->Form()->Add(wxMakeFormNewLine());
    tree_options_dialog->Form()->Add(wxMakeFormShort("Fork Length  ", 
        &fork_length, wxFORM_SLIDER,
        new wxList(wxMakeConstraintRange(FORK_LENGTH_MIN, FORK_LENGTH_MAX), 0),
        NULL, wxHORIZONTAL));
    tree_options_dialog->Form()->Add(wxMakeFormNewLine());
    tree_options_dialog->Form()->Add(wxMakeFormShort("Y Spacing    ", 
        &y_spacing, wxFORM_SLIDER,
        new wxList(wxMakeConstraintRange(Y_SPACING_MIN, Y_SPACING_MAX), 0),
        NULL, wxHORIZONTAL));
    tree_options_dialog->Form()->Add(wxMakeFormNewLine());
    tree_options_dialog->Form()->Add(wxMakeFormBool("Flashing Cursor", 
        &flashing_cursor, wxFORM_CHECKBOX));
    tree_options_dialog->Form()->Add(wxMakeFormBool("Color Coded Outcomes", 
        &color_coded_outcomes, wxFORM_CHECKBOX));
    tree_options_dialog->Form()->Add(wxMakeFormNewLine());
    wxStringList *iset_list = new wxStringList("None", "Same Level", "All Levels", 0);
    char *iset_str = new char[20];
    strcpy(iset_str, (char *)iset_list->Nth(show_infosets)->Data());
    tree_options_dialog->Add(wxMakeFormString("Show Infoset Lines", 
        &iset_str, wxFORM_RADIOBOX,
        new wxList(wxMakeConstraintStrings(iset_list), 0), 0, wxVERTICAL));
    tree_options_dialog->Form()->Add(wxMakeFormNewLine());
    tree_options_dialog->Add(wxMakeFormShort("Output precision", 
        &num_prec, wxFORM_SLIDER, new wxList(wxMakeConstraintRange(0, 16), 0)));

    tree_options_dialog->Go();
    show_infosets = wxListFindString(iset_list, iset_str);
}


// SetLegends
// Selects what get displayed at different points on the tree and in what
// font it will be displayed
void TreeDrawSettings::draw_params_legends_func(wxButton &ob, wxCommandEvent &)
{
    FontDialogBox *f;
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

    case NODE_TERMINAL_LEGEND: 
        f = new FontDialogBox((wxWindow *)ob.GetParent(), 
                              dpls->draw_settings->NodeTerminalFont());
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

        case NODE_TERMINAL_LEGEND: 
            dpls->draw_settings->SetNodeTerminalFont(the_font);
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
    wxStringList  *node_terminal_list = new wxStringList;
    wxStringList  *node_right_list    = new wxStringList;
    char          *node_above_str     = new char[20];
    char          *node_below_str     = new char[20];
    char          *branch_above_str   = new char[20];
    char          *branch_below_str   = new char[20];
    char          *node_terminal_str  = new char[20];
    char          *node_right_str     = new char[20];
    int            node_above_num;
    int            node_below_num;
    int            branch_above_num;
    int            branch_below_num;
    int            node_terminal_num;
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
    while (node_terminal_src[i].l_id != -1)
    {
        node_terminal_list->Add(node_terminal_src[i].l_name);
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
    strcpy(node_terminal_str, node_terminal_src[LabelNodeTerminal()].l_name);
    strcpy(node_right_str,    node_right_src[LabelNodeTerminal()].l_name);

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
    display_legend_dialog->Add(wxMakeFormNewLine());
    display_legend_dialog->Add(wxMakeFormString("At terminal", 
        &node_terminal_str, wxFORM_CHOICE,
        new wxList(wxMakeConstraintStrings(node_terminal_list), 0)));
    wxFormItem *node_terminal_button = 
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
    draw_params_legend_struct dpls5 = { NODE_TERMINAL_LEGEND, this };
    node_terminal_button->PanelItem->SetClientData((char *)&dpls5);
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
        node_terminal_num = wxListFindString(node_terminal_list, node_terminal_str);
        node_right_num    = wxListFindString(node_right_list,    node_right_str);

        SetLabelNodeAbove(node_above_src[node_above_num].l_id);
        SetLabelNodeBelow(node_below_src[node_below_num].l_id);
        SetLabelBranchAbove(branch_above_src[branch_above_num].l_id);
        SetLabelBranchBelow(branch_below_src[branch_below_num].l_id);
        SetLabelNodeTerminal(node_terminal_src[node_terminal_num].l_id);
        SetLabelNodeRight(node_right_src[node_right_num].l_id);
    }

    // Clean up.
    delete [] node_above_str;
    delete [] node_below_str;
    delete [] branch_above_str;
    delete [] branch_below_str;
    delete [] node_terminal_str;
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

    case NODE_TERMINAL_LEGEND: 
        legend_src = node_terminal_src;
        legend_id = node_terminal_label;
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

        case NODE_TERMINAL_LEGEND: 
            SetLabelNodeTerminal(legend_id);
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
    char *file_name = (s) ? s : INIFILE;
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
    wxWriteResource("Gambit", "Node-Terminal-Label", node_terminal_label, file_name);
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
    wxWriteResource("Gambit", "Node-Terminal-Font", 
                    wxFontToString(node_terminal_font), file_name);
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
    wxGetResource("Gambit", "Node-Terminal-Label", &node_terminal_label,  file_name);
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
    wxGetResource("Gambit", "Node-Terminal-Font", &l_tempstr, file_name);
    SetNodeTerminalFont(wxStringToFont(l_tempstr));
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
