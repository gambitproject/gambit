//
// File: efgsoln.h -- declaration of classes for displaying the extensive
// form solutions.
//
// $Id$
//

#ifndef EFGSOLN_H
#define EFGSOLN_H

#include "wx/grid.h"
#include "efg.h"
#include "efgconst.h"
#include "efgshow.h"
#include "bsolnsf.h"

#ifdef NOT_PORTED_YET

//****************************************************************************
//                       BEHAV SOLUTION PICKER (multiple)
//****************************************************************************

class EfgSolnPicker: public EfgSolnShow
{
private:
    gArray<bool> picked;
    void PickSoln(int row);
    wxButton *pick_all_button;
    bool pick_all;
    static void pick_all_func(wxButton &ob, wxEvent &);
    void OnPickAll(void);

public:
    EfgSolnPicker(const Efg &ef_, BehavSolutionList &soln,
                  const GambitDrawSettings &draw_settings,
                  BSolnSortFilterOptions    &sf_options,
                  EfgShow *parent_ = 0);
    // Double clicking on a solution will toggle it.
    void OnDoubleClick(int row, int col, int level, const gText &value);
    // Override parent class feature
    void OnSelectedMoved(int row, int col, SpreadMoveDir how);
    // OnOk must be defined to inform parent that I am killed
    void OnOk(void);
    // Override the help system
    virtual void OnHelp(int help_type = 0);
};

//****************************************************************************
//                       BEHAV SOLUTION PICKER (single)
//****************************************************************************

class Ext1SolnPicker: public EfgSolnShow
{
private:
    int picked;
    void PickSoln(int row);

protected:
    virtual void OnRemove(bool all);

public:
    Ext1SolnPicker(const Efg &ef_, BehavSolutionList &soln,
                   const GambitDrawSettings &draw_settings,
                   BSolnSortFilterOptions   &sf_options,
                   EfgShow *parent_ = 0);

    // Double clicking on a solution will select it
    void OnDoubleClick(int row, int col, int level, const gText &value);
    // Override parent class feature
    void OnSelectedMoved(int row, int col, SpreadMoveDir how);
    // OnOk must be defined to inform parent that I am killed
    void OnOk(void);
    // Override the help system
    virtual void OnHelp(int help_type = 0);
    // Return the pick
    int Picked(void) const;
};

#endif // NOT_PORTED_YET

#endif // EFGSOLN_H
