// $Id$

#ifndef GRIDPRM_H
#define GRIDPRM_H
#include "algdlgs.h"

class GridParamsSettings:public virtual PxiParamsSettings
{
protected:
    float minLam, maxLam, delLam, delp1, tol1, delp2, tol2;
    int multi_grid;
    void SaveDefaults(void);

public:
    GridParamsSettings(const char *fn);
    ~GridParamsSettings();
    void GetParams(GridParams &P);
};


class GridSolveParamsDialog : public MyDialogBox, public GridParamsSettings
{
public:
    GridSolveParamsDialog(wxWindow *parent, const gText filename);
    //  ~GridSolveParamsDialog(void);
};


#ifdef GRID_PRM_INST    // instantiate only once
//******************************** Constructor ************************
GridParamsSettings::GridParamsSettings(const char *fn)
    : PxiParamsSettings("grid", fn)
{
    wxGetResource(PARAMS_SECTION, "Grid-minLam", &minLam, defaults_file);
    wxGetResource(PARAMS_SECTION, "Grid-maxLam", &maxLam, defaults_file);
    wxGetResource(PARAMS_SECTION, "Grid-delLam", &delLam, defaults_file);
    wxGetResource(PARAMS_SECTION, "Grid-delp1", &delp1, defaults_file);
    wxGetResource(PARAMS_SECTION, "Grid-tol1", &tol1, defaults_file);
    wxGetResource(PARAMS_SECTION, "Grid-delp2", &delp2, defaults_file);
    wxGetResource(PARAMS_SECTION, "Grid-tol2", &tol2, defaults_file);
    wxGetResource(PARAMS_SECTION, "Grid-multigrid", &multi_grid, defaults_file);
}


void GridParamsSettings::SaveDefaults(void)
{
    wxWriteResource(PARAMS_SECTION, "Grid-minLam", minLam, defaults_file);
    wxWriteResource(PARAMS_SECTION, "Grid-maxLam", maxLam, defaults_file);
    wxWriteResource(PARAMS_SECTION, "Grid-delLam", delLam, defaults_file);
    wxWriteResource(PARAMS_SECTION, "Grid-delp1", delp1, defaults_file);
    wxWriteResource(PARAMS_SECTION, "Grid-tol1", tol1, defaults_file);
    wxWriteResource(PARAMS_SECTION, "Grid-delp2", delp2, defaults_file);
    wxWriteResource(PARAMS_SECTION, "Grid-tol2", tol2, defaults_file);
    wxWriteResource(PARAMS_SECTION, "Grid-multigrid", multi_grid, defaults_file);
}


GridParamsSettings::~GridParamsSettings(void)
{
    SaveDefaults();
}


void GridParamsSettings::GetParams(GridParams &P)
{
    P.minLam = minLam;
    P.maxLam = maxLam;
    P.delLam = delLam;
    P.tol1 = tol1;
    P.delp1 = delp1;
    P.tol2 = tol2;
    P.delp2 = delp2;
    P.multi_grid = multi_grid;
    // Pxi stuff
    P.powLam = PxiType();
    P.pxifile = PxiFile();
    // Output stuff
    P.trace = TraceLevel();
    P.tracefile = OutFile();
}


GridSolveParamsDialog::GridSolveParamsDialog(wxWindow *parent, const gText filename)
    : MyDialogBox(parent, "Gobit All Solve Params", GOBIT_HELP),
      GridParamsSettings(filename), PxiParamsSettings("grid", filename)
{
    // Create panel items

    SetLabelPosition(wxVERTICAL);
    wxText *minLamt = new wxText(this, 0, "minLam", "", 24, 11, 104, 50, 
                                 wxVERTICAL_LABEL, "minLam");
    wxText *maxLamt = new wxText(this, 0, "maxLam", "", 339, 14, 118, 52,
                                 wxVERTICAL_LABEL, "maxLam");
    wxText *delLamt = new wxText(this, 0, "delLam", "", 188, 13, 126, 52, 
                                 wxVERTICAL_LABEL, "delLam");
    SetLabelPosition(wxHORIZONTAL);
    (void)new wxGroupBox(this, "Grid #1", 7, 83, 136, 126, 0, "box1");
    wxText *delp1t = new wxText(this, 0, "Del", "", 13, 107, 112, 30, 0, "delp1");
    wxText *tol1t = new wxText(this, 0, "Tol", "", 16, 156, 106, 30, 0, "tol1");
    (void)new wxGroupBox(this, "Grid # 2", 162, 83, 144, 124, 0, "box2");
    wxText *delp2t = new wxText(this, 0, "Del", "", 181, 109, 102, 30, 0, "delp2");
    wxText *tol2t = new wxText(this, 0, "Tol", "", 182, 151, 104, 30, 0, "tol2");
    char *stringArray46[] = { "Lin", "Log" };
    wxRadioBox *pxitypet = new wxRadioBox(this, 0, "Plot Type", 
                                          315, 94, -1, -1, 2, stringArray46, 2, 0, "pxitype");
    wxCheckBox *multigridt = new wxCheckBox(this, 0, "Use MultiGrid", 
                                            317, 171, -1, -1, 0, "multgrid");
    SetLabelPosition(wxVERTICAL);
    (void)new wxGroupBox(this, "PXI Output", 1, 209, 452, 162, 0, "box3");
    wxText *pxifilet = new wxText(this, 0, "Pxi File", "", 
                                  13, 236, 130, 54, wxVERTICAL_LABEL, "pxfile");
    SetLabelPosition(wxHORIZONTAL);
    char *stringArray47[] = { "Default", "Saved", "Prompt" };
    wxRadioBox *next_typet = new wxRadioBox(this, 0, "Next File", 
                                            157, 230, -1, -1, 3, stringArray47, 3, 0, "next_type");
    wxCheckBox *run_boxt = new wxCheckBox(this, 0, "Run PXI", 
                                          18, 319, -1, -1, 0, "run_box");
    wxText *pxi_commandt = new wxText(this, 0, "Pxi Command", "", 
                                      116, 312, 104, 30, 0, "pxi_command");
    char *stringArray48[] = { "1", "2", "3", "4" };
    (void)new wxGroupBox(this, "Debug Output", 2, 373, 456, 68, 0, "box4");
    wxChoice *tracet = new wxChoice(this, 0, "Trace Level", 
                                    7, 401, -1, -1, 4, stringArray48, 0, "trace_choice");
    wxText *tracefilet = new wxText(this, 0, "Trace File", "", 
                                    187, 402, 200, 30, 0, "trace_file");

    //  ok_button = new wxButton(this, (wxFunction)ok_func, "Ok", 
    //                           34, 455, -1, -1, 0, "ok_button");
    //  cancel_button = new wxButton(this, (wxFunction)ButtonProc42, "Cancel", 
    //                               114, 457, -1, -1, 0, "cancel_button");
    //  help_button = new wxButton(this, (wxFunction)ButtonProc44, "Help", 
    //                             224, 458, -1, -1, 0, "help_button");

    minLamt->SetValue(ToText(minLam));
    maxLamt->SetValue(ToText(maxLam));
    delLamt->SetValue(ToText(delLam));
    delp1t->SetValue(ToText(delp1));
    delp2t->SetValue(ToText(delp2));
    tol1t->SetValue(ToText(tol1));
    tol2t->SetValue(ToText(tol2));
    tracefilet->SetValue(outname);
    tracet->SetStringSelection(trace_str);
    pxitypet->SetStringSelection(type_str);
    pxifilet->SetValue(pxiname);
    next_typet->SetStringSelection(name_option_str);
    pxi_commandt->SetValue(pxi_command);
    run_boxt->SetValue(run_pxi);
    multigridt->SetValue(multi_grid);
    Go();

    if (Completed() == wxOK)
    {
        minLam = strtod(minLamt->GetValue(), 0);
        maxLam = strtod(maxLamt->GetValue(), 0);
        delLam = strtod(delLamt->GetValue(), 0);
        delp1  = strtod(delp1t->GetValue(),  0);
        tol1   = strtod(tol1t->GetValue(),   0);
        delp2  = strtod(delp2t->GetValue(),  0);
        tol2   = strtod(tol2t->GetValue(),   0);

        strcpy(outname,         tracefilet->GetValue());
        strcpy(trace_str,       tracet->GetStringSelection());
        strcpy(type_str,        pxitypet->GetStringSelection());
        strcpy(pxiname,         pxifilet->GetValue());
        strcpy(name_option_str, next_typet->GetStringSelection());
        strcpy(pxi_command,     pxi_commandt->GetValue());

        run_pxi    = run_boxt->GetValue();
        multi_grid = multigridt->GetValue();
    }

}

#endif // GRID_PRM_INST

#endif // GRIDPRM_H
