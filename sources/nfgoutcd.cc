// File: nfgoutcd.cc -- code for the NFG outcome editing dialog
// $Id$

#include "wx.h"
#include "wxmisc.h"
#include "spread.h"
#include "nfg.h"
#include "nfgshow.h"
#include "nfgoutcd.h"
#include "nfplayer.h"

/****************************************************************************
                           BASE OUTCOME DIALOG
****************************************************************************/

#define NFG_OUTCOME_HELP    "Outcomes GUI"

class NfgOutcomeDialogC: public SpreadSheet3D
{
private:
    static void outcome_attach_func(wxButton &ob, wxEvent &);
    static void outcome_detach_func(wxButton &ob, wxEvent &);
    static void outcome_delete_func(wxButton &ob, wxEvent &);
    static void settings_func(wxButton &ob, wxEvent &);

protected:
    NfgOutcomeDialog *parent;
    Nfg &nf;
    NfgShow *ns;
    int prev_outc_num;
    class OutcomeDragger;
    OutcomeDragger *outcome_drag;
    void OnAttach(void);
    void OnDetach(void);
    virtual void OnDelete(void);
    void OnSettings(void);
    void OnPolyval(void);
    void CheckOutcome(int outc_num);
    virtual void PayoffPos(int outc_num, int player, int *row, int *col) = 0;
    virtual void NamePos(int outc_num, int *row, int *col) = 0;
    virtual Bool OnEventNew(wxMouseEvent &ev);

public:
    NfgOutcomeDialogC(int rows, int cols, Nfg &nf,
                      NfgShow *ns, NfgOutcomeDialog *parent);
    void SetCurOutcome(const gText &out_name);
    void OnHelp(int );

    // This implements the behavior that a new row is created automatically
    // below the greatest ENTERED row.  Also, if we move to a new row, the
    // previous row is automatically saved in the nf.
    virtual void OnSelectedMoved(int , int , SpreadMoveDir ) { };
    virtual void OnDoubleClick(int , int , int , const gText &);
    virtual void UpdateValues(void);
    virtual void OnOk(void);
    virtual int  OutcomeNum(int row = 0, int col = 0) = 0;
    virtual int   PlayerNum(int row = 0, int col = 0) = 0;
    virtual Bool OnClose(void);
};


extern wxCursor *outcome_cursor;    // defined in efgoutcd.cc

#define DRAG_NONE       0           // Current drag state
#define DRAG_START      1
#define DRAG_CONTINUE   2
#define DRAG_STOP       3

class NfgOutcomeDialogC::OutcomeDragger
{
private:
    NfgOutcomeDialogC *parent;
    NfgShow *ns;
    int drag_now;
    int outcome;
    int x, y;

public:
    OutcomeDragger(NfgOutcomeDialogC *parent, NfgShow *ns);
    int OnEvent(wxMouseEvent &ev);
};


NfgOutcomeDialogC::OutcomeDragger::OutcomeDragger(NfgOutcomeDialogC *parent_,
                                                  NfgShow *ns_)
    : parent(parent_), ns(ns_), drag_now(0)
{
    if (!outcome_cursor)
    {
#ifdef wx_msw
        outcome_cursor = new wxCursor("OUTCOMECUR");
#else
#include "bitmaps/outcome.xbm"
        outcome_cursor = new wxCursor(outcome_bits, outcome_width, outcome_height, 
                                      -1, -1, outcome_bits);
#endif
    }
}


int NfgOutcomeDialogC::OutcomeDragger::OnEvent(wxMouseEvent &ev)
{
    int ret = (drag_now) ? DRAG_CONTINUE : DRAG_NONE;

    if (ev.Dragging())
    {
        if (!drag_now)
        {
            drag_now = 1;
            parent->GetSheet()->SetCursor(outcome_cursor);
            parent->GetSheet()->CaptureMouse();
            outcome = parent->OutcomeNum();
            ret = DRAG_START;
        }

    }

    if (ev.LeftUp() && drag_now)
    {
        drag_now = 0;
        parent->GetSheet()->SetCursor(wxSTANDARD_CURSOR);
        parent->GetSheet()->ReleaseMouse();
        float x, y;
        ev.Position(&x, &y);

        if (x > 2000) 
            x -= 65536.0;

        if (y > 2000) 
            y -= 65536.0; // negative integer overflow

        int xi = (int)x, yi = (int)y;
        parent->GetSheet()->ClientToScreen(&xi, &yi);
        ret = DRAG_STOP;
        ns->SetOutcome(outcome, xi, yi);
    }

    return ret;
}


// Constructor
NfgOutcomeDialogC::NfgOutcomeDialogC(int rows, int cols, Nfg &nf_,
                                     NfgShow *ns_, NfgOutcomeDialog *parent_)
    : SpreadSheet3D(rows, cols, 1, "Outcomes [S]", 
                    0/*(wxFrame *)ns_->GetParent()*/, ANY_BUTTON),
      parent(parent_), nf(nf_), ns(ns_)
{
    MakeButtons(OK_BUTTON|PRINT_BUTTON|OPTIONS_BUTTON|HELP_BUTTON);
    AddButton("Opt", (wxFunction)settings_func);
    AddButtonNewLine();
    AddButton("Attach", (wxFunction)outcome_attach_func);
    AddButton("Detach", (wxFunction)outcome_detach_func);
    AddButton("Delete", (wxFunction)outcome_delete_func);
    prev_outc_num = 1;
    outcome_drag = new OutcomeDragger(this, ns);
    CanvasFocus();
}

// Handler functions -> stubs to actual functions
void NfgOutcomeDialogC::outcome_attach_func(wxButton &ob, wxEvent &)
{
    ((NfgOutcomeDialogC *)ob.GetClientData())->OnAttach();
}


void NfgOutcomeDialogC::outcome_detach_func(wxButton &ob, wxEvent &)
{
    ((NfgOutcomeDialogC *)ob.GetClientData())->OnDetach();
}


void NfgOutcomeDialogC::outcome_delete_func(wxButton &ob, wxEvent &)
{
    ((NfgOutcomeDialogC *)ob.GetClientData())->OnDelete();
}


void NfgOutcomeDialogC::settings_func(wxButton &ob, wxEvent &)
{
    ((NfgOutcomeDialogC *)ob.GetClientData())->OnSettings();
}


// OnAttach
void NfgOutcomeDialogC::OnAttach(void)
{
    ns->SetOutcome(OutcomeNum());
    CanvasFocus();
}


// OnDetach
void NfgOutcomeDialogC::OnDetach(void)
{
    ns->SetOutcome(0);
    CanvasFocus();
}


// OnDelete
void NfgOutcomeDialogC::OnDelete(void)
{
    char tmp_str[256];
    int outc_num = OutcomeNum();
    gText outc_name = nf.Outcomes()[outc_num]->GetName();
    sprintf(tmp_str, "Delete Outcome '%s'?", (const char *)outc_name);

    if (wxMessageBox(tmp_str, "Confirm", wxOK|wxCANCEL) == wxOK)
    {
        if (outc_num <= nf.NumOutcomes()) // not the last, blank row
        {
            NFOutcome *tmp = nf.Outcomes()[outc_num];
            assert(tmp);
            nf.DeleteOutcome(tmp);
            ns->SetOutcome(-1);
        }

    }

    CanvasFocus();
}


// OnSettings
void NfgOutcomeDialogC::OnSettings(void)
{
    MyDialogBox *options_dialog = new MyDialogBox(this, "Outcome Settings", NFG_OUTCOME_HELP);
    wxStringList *opt_list = new wxStringList("Compact Format", "Long Entries", 0);
    char *opt_str = new char[25];
    int dialog_type;
    char *defaults_file = "gambit.ini";
    wxGetResource("Gambit", "NfgOutcome-Dialog-Type", &dialog_type, defaults_file);
    strcpy(opt_str, (char *)opt_list->Nth(dialog_type)->Data());
    options_dialog->Add(wxMakeFormString("Dialog Type", &opt_str, wxFORM_RADIOBOX,
                                         new wxList(wxMakeConstraintStrings(opt_list), 0)));
    options_dialog->Go();

    if (options_dialog->Completed() == wxOK)
    {
        int new_dialog_type = wxListFindString(opt_list, opt_str);

        if (new_dialog_type != dialog_type)
        {
            wxMessageBox("New dialog type will be used\n"
                         "next time the outcome window is created", 
                         "Outcome Display", wxOK | wxCENTRE, this);
            wxWriteResource("Gambit", "NfgOutcome-Dialog-Type", 
                            new_dialog_type, defaults_file);
        }
    }

    delete options_dialog;
}


// OnHelp
void NfgOutcomeDialogC::OnHelp(int )
{
    wxHelpContents(NFG_OUTCOME_HELP);
}


// SetCurOutcome
void NfgOutcomeDialogC::SetCurOutcome(const gText &out_name)
{
    int out = 0;

    if (out_name != "")
    {
        for (int i = 1; i <= nf.NumOutcomes(); i++)
        {
            if ((nf.Outcomes()[i])->GetName() == out_name) 
                out = i;
        }
    }

    if (out)
    {
        int row, col;
        NamePos(out, &row, &col);
        SetCurRow(row);
        SetCurCol(col);
        OnSelectedMoved(row, col, SpreadMoveJump);  // we could have been editing
    }
}


// OnOk -- check if the current outcome has changed
void NfgOutcomeDialogC::OnOk(void)
{
    CheckOutcome(OutcomeNum(CurRow(), CurCol()));
    parent->OnOk();
}


// OnClose -- close the window, as if OK was pressed.
Bool NfgOutcomeDialogC::OnClose(void)
{
    OnOk();
    return FALSE;
}


// OnEvent -- check if we are dragging an outcome
Bool NfgOutcomeDialogC::OnEventNew(wxMouseEvent &ev)
{
    if (outcome_drag->OnEvent(ev) != DRAG_NONE) 
        return TRUE;

    return FALSE;
}


void NfgOutcomeDialogC::UpdateValues(void)
{
    int row, col;
    NFOutcome *tmp;
    gText payoff;
    bool hilight;

    for (int i = 1; i <= nf.NumOutcomes(); i++)
    {
        tmp = nf.Outcomes()[i];

        for (int j = 1; j <= nf.NumPlayers(); j++)
        {
            PayoffPos(i, j, &row, &col);
            hilight = false;
            payoff = ToText(nf.Payoff(tmp, j));
            SetCell(row, col, payoff);
            HiLighted(row, col, 0, hilight);
        }

        NamePos(i, &row, &col);
        SetCell(row, col, tmp->GetName());
    }
}


void NfgOutcomeDialogC::CheckOutcome(int outc_num)
{
    assert(outc_num > 0 && outc_num <= nf.NumOutcomes() + 1);
    bool outcomes_changed = false;
    NFOutcome *tmp;

    // if a new outcome has created, append it to the list of outcomes

    if (outc_num > nf.NumOutcomes())
    {
        tmp = nf.NewOutcome();
        tmp->SetName("Outcome " + ToText(nf.NumOutcomes()));
    }
    else
    {
        tmp = nf.Outcomes()[outc_num];
    }

    assert(tmp);

    // check if the values have changed
    int prow, pcol;

    for (int j = 1; j <= nf.NumPlayers(); j++)
    {
        PayoffPos(outc_num, j, &prow, &pcol);
        gNumber payoff;
        FromText(GetCell(prow, pcol), payoff);

        if (nf.Payoff(tmp, j) != payoff)
        {
            nf.SetPayoff(tmp, j, payoff);
            outcomes_changed = true;
        }
    }

    // check if the name has changed
    NamePos(outc_num, &prow, &pcol);
    gText new_name = GetCell(prow, pcol);

    if (new_name != tmp->GetName())
    {
        if (new_name != "")
        {
            tmp->SetName(new_name);
            outcomes_changed = true;
        }
        else
        {
            SetCell(prow, pcol, tmp->GetName());
            OnPaint();
        }
    }

    if (outcomes_changed) 
        ns->SetOutcome(-1);
}


void NfgOutcomeDialogC::OnDoubleClick(int row, int col, 
                                      int /* level*/, const gText &)
{
    static bool busy = false;

    if (busy) 
        return;

    int outc_num = OutcomeNum(row, col);
    NFOutcome *tmp = nf.Outcomes()[outc_num];
    int pl = PlayerNum(row, col);
    busy = true;

    if (pl == 0)
        return; // double click only edits player payoffs.

    gText s0 = ToText(nf.Payoff(tmp, pl));

    int x = GetSheet()->MaxX(col - 1) + TEXT_OFF;
    int y = GetSheet()->MaxY(row - 1) + TEXT_OFF;
    GetSheet()->ClientToScreen(&x, &y);
    gText s1 = gGetTextLine(s0, this, x, y);

    if (s1 != "" && s0 != s1)
    {
        gNumber payoff;
        FromText(s1, payoff);
        nf.SetPayoff(tmp, pl, payoff);
        UpdateValues();
        ns->UpdateVals();
        Repaint();
    }

    busy = false;
}


/****************************************************************************
                         SHORT ENTRY OUTCOME DIALOG
****************************************************************************/

class NfgOutcomeDialogShort: public NfgOutcomeDialogC
{
protected:
    void OnDelete(void);
    int  OutcomeNum(int row = 0, int col = 0);
    int  PlayerNum(int row = 0, int col = 0);
    void PayoffPos(int outc_num, int player, int *row, int *col);
    void NamePos(int outc_num, int *row, int *col);

public:
    NfgOutcomeDialogShort(Nfg &nf, NfgShow *ns, NfgOutcomeDialog *parent);
    void OnSelectedMoved(int row, int col, SpreadMoveDir how);
    virtual void OnOptionsChanged(unsigned int options = 0);
};


NfgOutcomeDialogShort::NfgOutcomeDialogShort(Nfg &nf_,
                                             NfgShow *ns_,
                                             NfgOutcomeDialog *parent_)
    : NfgOutcomeDialogC((nf_.NumOutcomes()) ? nf_.NumOutcomes() : 1,
                        nf_.NumPlayers() + 1, nf_, ns_, parent_)
{
    DrawSettings()->SetLabels(S_LABEL_ROW|S_LABEL_COL);
    DataSettings()->SetChange(S_CAN_GROW_ROW);
    DataSettings()->SetAutoLabel(S_AUTO_LABEL_ROW);
    DataSettings()->SetAutoLabelStr("Out:%d", S_AUTO_LABEL_ROW);
    DrawSettings()->SetColWidth(9, GetCols()); // 'Outcome #' = 9 chars
    SetLabelCol(GetCols(), "Name");
    int i, j;

    for (j = 1; j <= nf.NumPlayers(); j++) 
        DrawSettings()->SetColWidth(2 + ToTextPrecision(), j);

    // make all the cells string input

    for (i = 1; i <= GetRows(); i++)
    {
        for (j = 1; j <= GetCols(); j++)
            SetType(i, j, gSpreadStr);
    }

    for (i = 1; i <= nf.NumPlayers(); i++)
        SetLabelCol(i, (nf.Players()[i])->GetName());

    UpdateValues();
    Redraw();
}


// OnDelete
void NfgOutcomeDialogShort::OnDelete(void)
{
    NfgOutcomeDialogC::OnDelete();
    int outc_num = OutcomeNum();
    DelRow(outc_num);
    Redraw();
}


// This implements the behavior that a new row is created automatically
// below the greatest ENTERED row.  Also, if we move to a new row, the
// previous row is automatically saved in the nf.

void NfgOutcomeDialogShort::OnSelectedMoved(int row, int col, SpreadMoveDir )
{
    if (OutcomeNum(row, col) != prev_outc_num)
    {
        CheckOutcome(prev_outc_num);
        prev_outc_num = OutcomeNum(row, col);
    }

    if (row == GetRows() && EnteredCell(row, 1))
    {
        AddRow();
        Redraw();
        OnPaint();
    }
}


// Functions that determine the window layout

int NfgOutcomeDialogShort::OutcomeNum(int row, int )
{
    if (row == 0) 
        row = CurRow();

    return row;
}


int NfgOutcomeDialogShort::PlayerNum(int , int col)
{
    if (col == 0) 
        col = CurCol();

    if (col == GetCols()) 
        return 0;

    return col;
}


void NfgOutcomeDialogShort::PayoffPos(int outc_num, int player, int *row, int *col)
{
    *row = outc_num;
    *col = player;
}


void NfgOutcomeDialogShort::NamePos(int outc_num, int *row, int *col)
{
    *row = outc_num;
    *col = nf.NumPlayers() + 1;
}


void NfgOutcomeDialogShort::OnOptionsChanged(unsigned int options)
{
    if (options&S_PREC_CHANGED)
    {
        UpdateValues();

        for (int j = 1; j <= nf.NumPlayers(); j++)
            DrawSettings()->SetColWidth(2 + ToTextPrecision(), j);

        Resize();
        Repaint();
    }
}


/****************************************************************************
                        LONG ENTRY OUTCOME DIALOG
****************************************************************************/

class NfgOutcomeDialogLong: public NfgOutcomeDialogC
{
protected:
    void OnDelete(void);
    int  OutcomeNum(int row = 0, int col = 0);
    int  PlayerNum(int row = 0, int col = 0);
    void PayoffPos(int outc_num, int player, int *row, int *col);
    void NamePos(int outc_num, int *row, int *col);

public:
    NfgOutcomeDialogLong(Nfg &nf, NfgShow *ns, NfgOutcomeDialog *parent);
    void OnSelectedMoved(int row, int col, SpreadMoveDir how);
    virtual void OnOptionsChanged(unsigned int options = 0);
};


NfgOutcomeDialogLong::NfgOutcomeDialogLong(Nfg &nf_,
                                           NfgShow *ns_, NfgOutcomeDialog *parent_)
    : NfgOutcomeDialogC((nf_.NumOutcomes() ? nf_.NumOutcomes() : 1) * nf_.NumPlayers(),
                        3, nf_, ns_, parent_)
{
    DrawSettings()->SetLabels(S_LABEL_ROW | S_LABEL_COL);
    DataSettings()->SetChange(S_CAN_GROW_ROW);
    DrawSettings()->SetColWidth(9, 1);   // "Player #" = 9 chars : player column
    DrawSettings()->SetColWidth(12, 2);  // values column
    DrawSettings()->SetColWidth(9, 3);   // "Outcome #" = 9 chars : outcome name column
    SetLabelCol(1, "Player");
    SetLabelCol(2, "Payoff");
    SetLabelCol(3, "Name");

    if (nf.NumOutcomes() == 0) 
        nf.NewOutcome();

    int i, j;

    for (i = 1; i <= GetRows(); i++) 
        SetType(i, 2, gSpreadStr);

    for (j = 1; j <= nf.NumOutcomes(); j++)  // set player and outcome names
    {
        for (i = 1; i <= nf.NumPlayers(); i++)
        {
            SetCell((j - 1) * nf.NumPlayers() + i, 1, (nf.Players()[i])->GetName());
            Bold((j - 1) * nf.NumPlayers() + i, 1, 0, TRUE);
        }

        SetCell((j - 1) * nf.NumPlayers() + 1, 3, nf.Outcomes()[j]->GetName());
        SetLabelRow((j - 1) * nf.NumPlayers() + 1, "Out:" + ToText(j));
        SetType((j - 1) * nf.NumPlayers() + 1, 3, gSpreadStr);
    }

    SetCurRow(1);
    SetCurCol(2);
    UpdateValues();
    Redraw();
}


// OnDelete

void NfgOutcomeDialogLong::OnDelete(void)
{
    NfgOutcomeDialogC::OnDelete();
    int outc_num = OutcomeNum();
    int i;

    for (i = 1; i <= nf.NumPlayers(); i++)
        DelRow(nf.NumPlayers()*outc_num - i + 1);

    for (i = outc_num + 1; i <= nf.NumOutcomes(); i++)
        SetLabelRow((i - 1) * nf.NumPlayers() + 1, "Out:" + ToText(i));

    Redraw();
}


// This implements the behavior that a new row is created automatically
// below the greatest ENTERED row.  Also, if we move to a new row, the
// previous row is automatically saved in the nf. Also prevents the user
// from going to non-modifiable cells (player name column, some outcome
// name cells)

void NfgOutcomeDialogLong::OnSelectedMoved(int row, int col, SpreadMoveDir how)
{
    if (col == 1) // player name column is off limits
    {
        SetCurCol(2);
        return;
    }  

    int outc_num = OutcomeNum(row, col);

    if (col == 3)  // outcome name column is special
    {
        if (how == SpreadMoveRight || how == SpreadMoveJump)
        {
            if (row != nf.NumPlayers() * (outc_num - 1) + 1)
            {
                row = nf.NumPlayers()*(outc_num - 1) + 1;
                SetCurRow(row);
            }
        }

        if (how == SpreadMoveUp && outc_num > 1)    // jump to a previous outcome
        {
            int prow, pcol;
            int n_outc_num = OutcomeNum(row + 1, col);
            NamePos(n_outc_num - 1, &prow, &pcol);
            SetCurRow(prow);
        }

        if (how == SpreadMoveDown) // jump to next outcome
        {
            if (row > GetRows() - nf.NumPlayers())
            {
                SetCurRow(GetRows() - nf.NumPlayers() + 1);
            }
            else
            {
                int prow, pcol;
                int n_outc_num = OutcomeNum(row - 1, col);
                NamePos(n_outc_num + 1, &prow, &pcol);
                SetCurRow(prow);
            }
        }
    }

    if (outc_num != prev_outc_num)                 // check/save outcome
    {
        CheckOutcome(prev_outc_num);
        prev_outc_num = outc_num;
    }

    if (row == GetRows() && EnteredCell(row, 2))   // add an outcome
    {
        for (int i = 1; i <= nf.NumPlayers(); i++)
        {
            AddRow();
            SetCell(GetRows(), 1, nf.Players()[i]->GetName());
            Bold(GetRows(), 1, 0, TRUE);
        }

        SetLabelRow(GetRows() - nf.NumPlayers() + 1, "Out:" + ToText(nf.NumOutcomes() + 1));
        Redraw();
        OnPaint();
    }
}


// Functions that determine the window layout

int NfgOutcomeDialogLong::OutcomeNum(int row, int )
{
    if (row == 0) 
        row = CurRow();

    return (row - 1) / nf.NumPlayers() + 1;
}


int NfgOutcomeDialogLong::PlayerNum(int row, int col)
{
    if (row == 0) 
        row = CurRow();

    if (col == 0) 
        col = CurCol();

    if (col != 2) 
        return 0;

    return (row - 1) % nf.NumPlayers() + 1;
}


void NfgOutcomeDialogLong::PayoffPos(int outc_num, int player, int *row, int *col)
{
    *row = (outc_num - 1)*nf.NumPlayers() + player;
    *col = 2;
}


void NfgOutcomeDialogLong::NamePos(int outc_num, int *row, int *col)
{
    *row = (outc_num - 1)*nf.NumPlayers() + 1;
    *col = 3;
}


void NfgOutcomeDialogLong::OnOptionsChanged(unsigned int options)
{
    if (options&S_PREC_CHANGED)
    {
        UpdateValues();
        DrawSettings()->SetColWidth(2 + ToTextPrecision(), 2);
        Resize();
        Repaint();
    }
}



/****************************************************************************
                            OUTCOME DIALOG
****************************************************************************/

NfgOutcomeDialog::NfgOutcomeDialog(Nfg &nf, NfgShow *ns_):ns(ns_)
{
    int dialog_type;
    char *defaults_file = "gambit.ini";
    wxGetResource("Gambit", "NfgOutcome-Dialog-Type", 
                  &dialog_type, defaults_file);

    if (dialog_type == SHORT_ENTRY_OUTCOMES)
        d = new NfgOutcomeDialogShort(nf, ns, this);
    else
        d = new NfgOutcomeDialogLong(nf, ns, this);

    d->Show(TRUE);
}


NfgOutcomeDialog::~NfgOutcomeDialog()
{
    d->Show(FALSE);
    delete d;
}


void NfgOutcomeDialog::SetOutcome(const gText &outc_name)
{
    d->SetCurOutcome(outc_name);
    d->SetFocus();
}


void NfgOutcomeDialog::UpdateVals(void)
{
    d->UpdateValues();
    d->Repaint();
}


void NfgOutcomeDialog::OnOk(void)
{
    ns->ChangeOutcomes(DESTROY_DIALOG);
}

