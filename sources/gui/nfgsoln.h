//
// FILE: nfgsoln.h -- declaration of classes for displaying the normal
// form solutions.
//
// $Id$
//

#ifndef NFGSOLN_H
#define NFGSOLN_H

//****************************************************************************
//                       MIXED SOLUTION SHOW
//****************************************************************************
#define		MSOLN_O_OPTIONS		1
#define		MSOLN_O_EDIT			2
#define		MSOLN_O_SORTFILT	4
#define		MSOLN_O_EFGNFG		16

class NfgShow;

//****************************************************************************
//                       MIXED SOLUTION PICKER (single)
//****************************************************************************

#ifdef NOT_PORTED_YET

class Nfg1SolnPicker: public NfgSolnShow
{
private:
	int picked;
	void PickSoln(int row);
protected:
	virtual void OnRemove(bool all);
public:
	Nfg1SolnPicker(gSortList<MixedSolution> &soln,int num_players,int max_strats,
						int cur_soln_,NormalDrawSettings	&ds,
						MSolnSortFilterOptions &sf_options,
						NfgShow *parent_=0,wxFrame *parent_frame=0);
	// Double clicking on a solution will select it
	void OnDoubleClick(int row,int col,int level,const gText &value);
	// Override parent class feature
	void OnSelectedMoved(int row,int col,SpreadMoveDir how);
	// OnOk must be defined to inform parent that I am killed
	void OnOk(void);
	// Override the help system
	virtual void OnHelp(int help_type=0);
	// Return the pick
	int Picked(void) const;
  // OnClose calls OnOk
  virtual bool OnClose(void);
};

#endif // NOT_PORTED_YET

#endif
