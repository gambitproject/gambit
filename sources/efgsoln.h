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

class NodeSolnShow : public wxGrid {
private:
  EfgShow *m_parent;
  const Node *m_cursor;
  static const int s_numFeatures;
  static char *s_featureNames[];

public:
  NodeSolnShow(EfgShow *p_efgShow, wxWindow *p_parent);
  virtual ~NodeSolnShow() { }

  void Set(const Node *p_cursor);
};


#define     BSOLN_O_OPTIONS     1
#define     BSOLN_O_EDIT        2
#define     BSOLN_O_SORTFILT    4
#define     BSOLN_O_PICKER      (8|BSOLN_O_OPTIONS|BSOLN_O_SORTFILT)
#define     BSOLN_O_EFGNFG      16

class EfgSolnShow : public wxGrid {
private:
  EfgShow *m_parent;

protected:
  BehavSolutionList &solns;
  //  const gPVector<int> dim;        // dimensionality dim[player][iset]
  int num_solutions, cur_soln;
  gBlock<bool> features;
  static char *feature_names[];
  static int   feature_width[];
  BSolnSortFilterOptions  &sf_options;
  int     FeaturePos(int feature);
  int     SolnNum(int row);
  int     SolnPos(int soln);
  virtual void    OnRemove(bool all);
  virtual void    OnAdd(void);
  virtual void    OnEdit(void);
  virtual void  SortFilter(bool inter = true);

  // Overriding wxGrid event handling
  virtual void OnSelectCell(int row, int col);
  virtual void OnLabelLeftClick(int row, int col, int x, int y,
				bool control, bool shift);

public:
  EfgSolnShow(EfgShow *p_efgShow, wxWindow *p_parent,
	      BehavSolutionList &p_solutions,
	      BSolnSortFilterOptions  &p_options);
  virtual ~EfgSolnShow() { }

  // Implement the infoset hilighting behavior.  If iset = 0, just return
  // the option state.
  bool HilightInfoset(int pl = 0, int iset = 0);
  // Take care of some options changes
  void OnOptionsChanged(unsigned int options = 0) { } 
  // Double clicking on a solution will update the parent
  void OnDoubleClick(int row, int col, int level, const gText &value);

  void UpdateValues(void);

  DECLARE_EVENT_TABLE()
};

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
