//
// FILE: treewin.h -- Interface for TreeWindow class
//
// $Id$
//

#ifndef TREEWINDOW_H
#define TREEWINDOW_H
#include "garray.h"
#include "glist.h"
#include "treedraw.h"
#include "efgconst.h"

typedef struct NODEENTRY {
		int x, y, level, color;
		struct {int x,y;} infoset;	// pos of the next node in the infoset to connect to
		int	num;				// # of the infoset line on this level
		int nums;				// sum of infosets previous to this level
		int has_children;	// how many children this node has
		int child_number;	// what branch # is this node from the parent
		bool in_sup;			// is this node in disp_sup
		const Node *n;
		NODEENTRY *parent;
		bool expanded;		// is this subgame root expanded or collapsed
		NODEENTRY(void) { }
		NODEENTRY(const NODEENTRY &e): x(e.x),y(e.y),level(e.level),color(e.color),
																	 infoset(e.infoset),num(e.num),
																	 nums(e.nums),has_children(e.has_children),
																	 child_number(e.child_number),in_sup(e.in_sup),
																	 n(e.n),expanded(e.expanded) { }
} NodeEntry;

class BaseExtensiveShow;
class BaseTreeWindow;
class TreeNodeCursor;
// This class can render an extensive form tree using a pre-calculated
// (in BaseTreeWindow) list of NodeEntry.  It is used for rendering
// functions for the main TreeWindow display and for the optional
// 'unity zoom' zoom window.  Note that it does not have any data
// members of its own, but just references to those of its parent. This
// way we do not duplicate the data and only need one assignment regardless
// of the number of renderers.
class TreeRender : public wxCanvas
{
private:
	const BaseTreeWindow *parent;
	const gList<NodeEntry *> &node_list;
	const Infoset * &hilight_infoset;		// Hilight infoset from the solution disp
	const Infoset * &hilight_infoset1;	// Hilight infoset by pressing control
	const Node	*&mark_node;							// Used in mark/goto node operations
	const Node *&subgame_node;
	const Node	 *&cursor;								// Used to process cursor keys, stores current pos
	// Private Functions
	void	RenderLabels(wxDC &dc,const NodeEntry *child_entry,const NodeEntry *entry);
	void 	RenderSubtree(wxDC &dc);
protected:
	const TreeDrawSettings &draw_settings;		// Stores drawing parameters
	TreeNodeCursor *flasher;			// Used to flash/display the cursor
  bool painting;								// Used to prevent re-entry.
public:
	TreeRender(wxFrame *frame,const BaseTreeWindow *parent,const gList<NodeEntry *> &node_list,
						const Infoset * &hilight_infoset_,const Infoset * &hilight_infoset1_,
						const Node *&mark_node_,const Node *&cursor,const Node *&subgame_node,
						const TreeDrawSettings &draw_settings_);
	~TreeRender(void);
	// Windows event handlers
	virtual void OnPaint(void);
	virtual void Render(wxDC &dc);
	// Call this every time the cursor moves
	virtual void UpdateCursor(const NodeEntry *entry);
	// Override this if extra functionality is desired
	virtual Bool JustRender(void) const;
	// This must be here since we do not have draw_settings at constructor time
	void MakeFlasher(void);
};

class TreeZoomWindow : public TreeRender
{
private:
// Real xs,ys,xe,ye: true (untranslated) coordinates of the cursor.  Need this
// if we repaint due to events other than cursor movement.  See ::Render()
	int xs,ys,xe,ye;
public:
	TreeZoomWindow(wxFrame *frame,const BaseTreeWindow *parent,const gList<NodeEntry *> &node_list,
						const Infoset * &hilight_infoset_,const Infoset * &hilight_infoset1_,
						const Node *&mark_node_,const Node *&cursor,const Node *&subgame_node,
						const TreeDrawSettings &draw_settings_,const NodeEntry *cursor_entry);
	virtual void Render(wxDC &dc);
	// Makes sure the cursor is always in the center of the window
	virtual void UpdateCursor(const NodeEntry *entry);
};

class BaseOutcomeDialog;
class BaseTreeWindow: public TreeRender
{
friend class ExtensivePrintout;
public:
typedef struct SUBGAMEENTRY {
					const Node *root;
					bool expanded;
					SUBGAMEENTRY(void):root(0),expanded(true) { }
					SUBGAMEENTRY(const Node *r,bool e=true):root(r),expanded(e) { }
					SUBGAMEENTRY(const SUBGAMEENTRY &s):root(s.root),
														expanded(s.expanded) { }
					SUBGAMEENTRY &operator=(const SUBGAMEENTRY &s)
						{root=s.root;expanded=s.expanded;return (*this);}
					// need these to make a list
					int operator==(const SUBGAMEENTRY &s) {return (s.root==root);}
					int operator!=(const SUBGAMEENTRY &s) {return (s.root!=root);}
					friend gOutput &operator<<(gOutput &,const SUBGAMEENTRY &);
					} SubgameEntry;
private:
	BaseEfg		&ef;
	EFSupport * &disp_sup;	// we only need to know the displayed sup
	BaseExtensiveShow	*frame;
	Node	*mark_node,*old_mark_node;		// Used in mark/goto node operations
	const Node	*subgame_node;		 		// Used to mark the 'picking' subgame root
	gList<NodeEntry *> node_list;		// Data for display coordinates of nodes
	gList<SubgameEntry> subgame_list; // Keeps track of collapsed/expanded subgames
	Bool		nodes_changed;    		// Used to determine if a node_list recalc
	Bool		infosets_changed,must_recalc;			// is needed
	Bool		need_clear;						// Do we need to clear the screen?
	gOutput	*log;									// Are we saving each action to a file?
	Infoset *hilight_infoset;			// Hilight infoset from the solution disp
	Infoset *hilight_infoset1;	  // Hilight infoset by pressing control
	TreeRender *zoom_window;
	wxMenu	*build_menu;					// a popup menu, equivalent to top level build
	class NodeDragger;						// Class to take care of tree copy/move by
	NodeDragger *node_drag;				// drag and dropping nodes.
	class IsetDragger;						// Class to take care of iset join by
	IsetDragger *iset_drag;				// drag and dropping.
	class BranchDragger;					// Class to take care of branch addition by
	BranchDragger *branch_drag;		// drag and dropping
	class OutcomeDragger;					// Class to take care of outcome copy/move by
	OutcomeDragger *outcome_drag;	// drag and dropping
	// Private Functions
	int 	FillTable(const Node *n,int level);
	void 	ProcessCursor(void);
	void 	ProcessClick(wxMouseEvent &ev);
	void 	ProcessDClick(wxMouseEvent &ev);
	void 	ProcessRClick(wxMouseEvent &ev);
	void 	ProcessRDClick(wxMouseEvent &ev);
	bool	ProcessShift(wxMouseEvent &ev);
	NodeEntry *GetNodeEntry(const Node *n);
	NodeEntry *NextInfoset(const NodeEntry * const e);
	NodeEntry *GetValidParent(const Node *n);
	NodeEntry *GetValidChild(const Node *n);
	SubgameEntry &GetSubgameEntry(const Node *n);
	void	FillInfosetTable(const Node *n);
	void	CheckInfosetEntry(NodeEntry *e);
	void	UpdateTableInfosets(void);
	void	UpdateTableParents(void);
	void	Log(const gString &s);
	int PlayerNum(const EFPlayer *p) const ;
	int IsetNum(const Infoset *i) const ;
	static void OnPopup(wxMenu &ob,wxCommandEvent &ev);
protected:
	Node	 *cursor;										// Used to process cursor keys, stores current pos
	Bool		outcomes_changed;
	TreeDrawSettings draw_settings;		// Stores drawing parameters
	BaseOutcomeDialog *outcome_dialog;		// do we have outcomes open for this window?
public:
	virtual double 	ProbAsDouble(const Node *n,int action) const =0;
	gString	AsString(TypedSolnValues what,const Node *n,int br=0) const;
	virtual gString	OutcomeAsString(const Node *n) const =0;
	virtual Bool JustRender(void) const;
	// Constructor
	BaseTreeWindow(BaseEfg &ef_,EFSupport * &disp, BaseExtensiveShow *frame);
	// Destructor
	~BaseTreeWindow(void);
	// Event Handlers
	void OnEvent(wxMouseEvent& event);
	void OnChar(wxKeyEvent& ch);
	void OnPaint(void);
	// Menu event handlers (these are mostly in btreewn1.cc)
	void node_add(void);
	void node_game(void);
	void node_label(void);
	void node_delete(void);
	void node_set_mark(void);
	void node_goto_mark(void);
	void node_outcome(int out,int x=-1,int y=-1);

	void action_label(void);
	void action_insert(void);
	void action_delete(void);
	virtual void action_probs(void) =0;

	void tree_delete(void);
	void tree_copy(void);
	void tree_move(void);
	void tree_label(void);
	void tree_players(void);
	void tree_infosets(void);
	virtual void tree_outcomes(const gString out_name=gString()) =0;

	void infoset_merge(void);
	void infoset_break(void);
	void infoset_split(void);
	void infoset_join(void);
	void infoset_label(void);
	void infoset_switch_player(void);
	void infoset_reveal(void);

	void subgame_clear_one(void);
	void subgame_clear_all(void);
	void subgame_solve(void);
	void subgame_collapse_one(void);
	void subgame_collapse_all(void);
	void subgame_expand_one(void);
  void subgame_expand_branch(void);
	void subgame_expand_all(void);
	void subgame_toggle(void);
	void subgame_set(void);

	void edit_outcome(void);
	void display_legends(void);
	void display_options(void);
	void display_colors(void);
	void display_save_options(Bool def=TRUE);
	void display_load_options(Bool def=TRUE);
	void display_set_zoom(float z=-1);
  void display_zoom_fit(void);
	float display_get_zoom(void);
	Bool display_zoom_win(void);

	void 	output(void);
	void	print_eps(wxOutputOption fit);			// output to postscript file
	void	print(wxOutputOption fit,bool preview=false);	// output to printer (WIN3.1 only)
	void	print_mf(wxOutputOption fit,bool save_mf=false);				// copy to clipboard (WIN3.1 only)
	Bool	logging(void);

	virtual void	file_save(void) =0 ;

	gString Title(void) const;

	virtual void Render(wxDC &dc);
	void HilightInfoset(int pl,int iset);
	// Used by parent BaseExtensiveShow when disp_sup changes
	void SupportChanged(void);
	// Gives access to the parent to the private draw_settings. Used for SolnShow
	TreeDrawSettings &DrawSettings(void) {return draw_settings;}
	// Gives access to the parent to the current cursor node
	const Node *Cursor(void) const {return cursor;}
	// Hilight the subgame root for the currently active subgame
	void	SetSubgamePickNode(const Node *n);
	// Check if a drag'n'drop object has been activated
	Node *GotObject(float &mx,float &my,int what); 
	// Used by a child outcomes inspect when it is close
	void OutcomeDialogDied(void);
};

template <class T> class ExtensiveShow;
template <class T>
class TreeWindow : public BaseTreeWindow
{
	// Private variables
	Efg<T> &ef;
	ExtensiveShow<T> *frame;			// parent frame
	// The copy/assignment operators are private since they are NEVER
	// used or implemented.  Perhaps later...
	TreeWindow(const TreeWindow<T> &);
	void operator=(const TreeWindow<T> &);
public:
	double 	ProbAsDouble(const Node *n,int action) const ;
	gString	OutcomeAsString(const Node *n) const;
	// Constructor
	TreeWindow(Efg<T> &ef_,EFSupport * &disp,ExtensiveShow<T> *frame);
	// Destructor
	~TreeWindow();
		// Menu events
	void tree_outcomes(const gString out_name=gString());
	void action_probs(void);
	void node_outcome(const gString out_name);
	void	file_save(void);
};

#endif   // TREEWINDOW_H

