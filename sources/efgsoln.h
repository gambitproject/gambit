// File: efgsoln.h -- declaration of classes for displaying the extensive
// form solutions.
//
// $Id$
#ifndef EXTSOLN_H
#define EXTSOLN_H
#include "efg.h"
#include "efgconst.h"
#include "efgshow.h"
#include "spread.h"
#include "bsolnsf.h"

//****************************************************************************
//                       NODE INSPECT WINDOW
//****************************************************************************
class NodeSolnShow:public SpreadSheet3D
{
private:
	const EfgShow *parent;
	const Node *cur_n; // use to refresh data
	gArray<Bool> features;
	static char *feature_names[];
	int num_players;
	int	Pos(int feature);
	void SetOptions(void);
	wxMenuBar *MakeMenuBar(long menus);
public:
	NodeSolnShow(int num_players,const EfgShow *parent);
	// Data entry functions
	void Set(const Node *n);
	// Take care of some options changes
	void OnOptionsChanged(unsigned int options=0);
	// Override menu handling
	void OnMenuCommand(int id);
	// Override the help system
	void OnHelp(int =0);
};


//****************************************************************************
//                       BEHAV SOLUTION SHOW
//****************************************************************************
#define		BSOLN_O_OPTIONS		1
#define		BSOLN_O_EDIT			2
#define		BSOLN_O_SORTFILT	4
#define		BSOLN_O_PICKER		(8|BSOLN_O_OPTIONS|BSOLN_O_SORTFILT)
#define		BSOLN_O_EFGNFG		16

class GambitDrawSettings;
class EfgSolnShow: public SpreadSheet3D
{
private:
	const Efg &ef;
	EfgShow *parent;
	void	UpdateValues(void);
	const GambitDrawSettings &gamb_draw_settings;
	static void normal_button(wxButton &ob,wxEvent &ev);
	static void add_button(wxButton &ob,wxEvent &ev);
	static void edit_button(wxButton &ob,wxEvent &ev);
	static void delete_button(wxButton &ob,wxEvent &ev);
	static void delete_all_button(wxButton &ob,wxEvent &ev);
	static void sortfilt_button(wxButton &ob,wxEvent &ev);
	// Options button
	static void settings_button(wxButton &ob,wxEvent &ev);
	void	UpdateSoln(int row,int col);
protected:
	BehavSolutionList &solns;
	const gPVector<int> dim;		// dimensionality dim[player][iset]
	int 	num_players,num_isets,num_solutions,cur_soln;
	gBlock<Bool> features;
	unsigned int opts;
	static char *feature_names[];
	static int   feature_width[];
	BSolnSortFilterOptions	&sf_options;
	virtual void SetOptions(void);
	int	 	FeaturePos(int feature);
	int		SolnNum(int row);
	int		SolnPos(int soln);
	virtual void	OnRemove(bool all);
	virtual void	OnAdd(void);
	virtual void	OnEdit(void);
	virtual void  SortFilter(bool inter=true);

public:
	EfgSolnShow(const Efg &ef_,BehavSolutionList &soln,
						int cur_soln_,const GambitDrawSettings &draw_settings,
						BSolnSortFilterOptions	&sf_options,
						EfgShow *parent_=0,
						unsigned int opts=BSOLN_O_OPTIONS|BSOLN_O_EDIT|BSOLN_O_SORTFILT|BSOLN_O_EFGNFG);
	// Implement the infoset hilighting behavior.  If iset=0, just return
	// the option state.
	Bool HilightInfoset(int pl=0,int iset=0);
	// Overide help system
	void OnHelp(int help_type=0);
	// Take care of some options changes
	void OnOptionsChanged(unsigned int options=0);
	// Double clicking on a solution will update the parent
	void OnDoubleClick(int row,int col,int level,const gString &value);
	// Moving a cell updates the parent, if the dynamic option is set
	void OnSelectedMoved(int row,int col,SpreadMoveDir how);
	// OnOk must be defined to inform parent that I am killed
	void OnOk(void);
	// Send one BehavProfile to a MixedProfile NF (Not implemented)
	void SolutionToNormal(void);
};

//****************************************************************************
//                       BEHAV SOLUTION PICKER (multiple)
//****************************************************************************

class EfgSolnPicker: public EfgSolnShow
{
private:
	gArray<bool> picked;
	void PickSoln(int row);
	wxButton *pick_all_button;
	Bool pick_all;
	static void pick_all_func(wxButton &ob,wxEvent &);
	void OnPickAll(void);
public:
	EfgSolnPicker(const Efg &ef_,BehavSolutionList &soln,
						const GambitDrawSettings &draw_settings,
						BSolnSortFilterOptions	&sf_options,
						EfgShow *parent_=0);
	// Double clicking on a solution will toggle it.
	void OnDoubleClick(int row,int col,int level,const gString &value);
	// Override parent class feature
	void OnSelectedMoved(int row,int col,SpreadMoveDir how);
	// OnOk must be defined to inform parent that I am killed
	void OnOk(void);
	// Override the help system
	virtual void OnHelp(int help_type=0);
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
	virtual void	OnRemove(bool all);
public:
	Ext1SolnPicker(const Efg &ef_,BehavSolutionList &soln,
						const GambitDrawSettings &draw_settings,
						BSolnSortFilterOptions	&sf_options,
						EfgShow *parent_=0);
	// Double clicking on a solution will select it
	void OnDoubleClick(int row,int col,int level,const gString &value);
	// Override parent class feature
	void OnSelectedMoved(int row,int col,SpreadMoveDir how);
	// OnOk must be defined to inform parent that I am killed
	void OnOk(void);
	// Override the help system
	virtual void OnHelp(int help_type=0);
	// Return the pick
	int Picked(void) const;
};


class BehavSolnEdit : public SpreadSheet3D
{
private:
	BehavSolutionT &soln;
	gPVector<int> dim;
	int	num_isets;
public:
	// Constructor
	BehavSolnEdit(BehavSolutionT &soln,int iset_disp,wxFrame *parent);
	void OnSelectedMoved(int row,int col,SpreadMoveDir how);
	void OnOk(void);
};

#endif
