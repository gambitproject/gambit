//
// FILE: treewin.h -- Interface for TreeWindow class
//
// @(#)treewin.h	1.19 7/4/95
//

#ifndef TREEWINDOW_H
#define TREEWINDOW_H
#include "garray.h"
#include "glist.h"
#include "treedraw.h"
#include "twflash.h"
#include "efgconst.h"

typedef struct NODEENTRY {
		int x, y, level, color;
		int infoset_y;	// y of the next node in the infoset to connect to
		int	num;				// # of the infoset line on this level
		int nums;				// sum of infosets previous to this level
		int has_children;	// how many children this node has
		int child_number;	// what branch # is this node from the parent
		const Node *n;
		NODEENTRY *parent;
} NodeEntry;

class BaseExtensiveShow;
class BaseTreeWindow;

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
	const Node	*&mark_node;							// Used in mark/goto node operations
	const Node	 *&cursor;								// Used to process cursor keys, stores current pos
	const TreeDrawSettings &draw_settings;		// Stores drawing parameters
	// Private Functions
	void	RenderLabels(wxDC &dc,NodeEntry *entry);
	void 	RenderSubtree(wxDC &dc);
protected:
	TreeNodeCursor *flasher;			// Used to flash/display the cursor
public:
	TreeRender(wxFrame *frame,const BaseTreeWindow *parent,const gList<NodeEntry *> &node_list,
						const Infoset * &hilight_infoset_,
						const Node *&mark_node_,const Node *&cursor,const TreeDrawSettings &draw_settings_);
	~TreeRender(void);
	// Windows event handlers
	void OnPaint(void);
	virtual void Render(wxDC &dc);
	// Call this every time the cursor moves
	void UpdateCursor(const NodeEntry *entry);
	// Override this if extra functionality is desired
	virtual Bool JustRender(void) const;
	// This must be here since we do not have draw_settings at constructor time
	void MakeFlasher(void);
};

class TreeZoomWindow : public TreeRender
{
public:
	TreeZoomWindow(wxFrame *frame,const BaseTreeWindow *parent,const gList<NodeEntry *> &node_list,
						const Infoset * &hilight_infoset_,
						const Node *&mark_node_,const Node *&cursor,const TreeDrawSettings &draw_settings_);
};

class BaseTreeWindow: public TreeRender
{
friend class ExtensivePrintout;

private:
	BaseExtensiveShow	*frame;
	BaseEfg		&ef;
	Node	*mark_node,*old_mark_node;		// Used in mark/goto node operations
	gList<NodeEntry *> node_list;		// Data for display coordinates of nodes
	Bool		nodes_changed;    		// Used to determine if a node_list recalc
	Bool		infosets_changed;			// is needed
	Bool		need_clear;						// Do we need to clear the screen?
	gOutput	*log;									// Are we saving each action to a file?
	Infoset *hilight_infoset;			// Hilight infoset from the solution disp
	TreeRender *zoom_window;
	// Private Functions
	int 	FillTable(const Node *n, int level);
	void 	ProcessCursor(void);
	void 	ProcessClick(int x,int y);
	NodeEntry *GetNodeEntry(const Node *n);
	NodeEntry *NextInfoset(const NodeEntry * const e);
	void	FillInfosetTable(const Node *n);
	void	CheckInfosetEntry(NodeEntry *e);
	void	UpdateTableInfosets(void);
	void	UpdateTableParents(void);
	void	Log(const gString &s);
	// These functions are type dependent and are defined in TreeWindow
protected:
	Node	 *cursor;										// Used to process cursor keys, stores current pos
	Bool		outcomes_changed;
	TreeDrawSettings draw_settings;		// Stores drawing parameters
public:
	virtual double 	ProbAsDouble(const Node *n,int action) const =0;
	virtual gString	ProbAsString(const Node *n,int action) const =0;
	virtual gString	OutcomeAsString(const Node *n) const =0;
	virtual Bool JustRender(void) const;
	// Constructor
//	BaseTreeWindow(const BaseTreeWindow &bt);
	BaseTreeWindow(BaseEfg &ef_,BaseExtensiveShow *frame);
	// Destructor
	~BaseTreeWindow(void);
	void OnEvent(wxMouseEvent& event);
	void OnChar(wxKeyEvent& ch);
	// Menu event handlers (these are mostly in treewin1.cc)
	void node_add(void);
	void node_game(void);
	void node_label(void);
	void node_delete(void);
	void node_set_mark(void);
	void node_goto_mark(void);
	void node_outcome(int out);
#ifdef SUBGAMES
	void node_subgame(int _game);
#endif

	void action_label(void);
	void action_insert(void);
	void action_delete(void);
	virtual void action_probs(void) =0;

	void tree_delete(void);
	void tree_copy(void);
	void tree_move(void);
	void tree_label(void);
	void tree_players(void);
	virtual void tree_outcomes(const gString out_name=gString(),int save_num=0) =0;
#ifdef SUBGAMES
	void tree_subgames(void);
	void tree_subgame_make(void);
	void tree_subgame_open(int _game);
	void tree_subgame_delete(int _game);
	void tree_subgame_name(int _game,gString _label);
#endif

	void infoset_merge(void);
	void infoset_break(void);
	void infoset_join(void);
	void infoset_label(void);
	void infoset_switch_player(void);

	void edit_outcome(void);
	void display_legends(void);
	void display_options(void);
	void display_colors(void);
	void display_save_options(Bool def=TRUE);
	void display_load_options(Bool def=TRUE);
	void display_set_zoom(float z=-1);
	float display_get_zoom(void);
	Bool display_zoom_win(void);

	void 	output(void);
	void	print_eps(wxOutputOption fit);			// output to postscript file
	void	print(wxOutputOption fit,bool preview=false);	// output to printer (WIN3.1 only)
	void	print_mf(wxOutputOption fit,bool save_mf=false);				// copy to clipboard (WIN3.1 only)
	Bool	logging(void);

	virtual void	file_save(void) =0;

	gString Title(void) const;

	void Render(wxDC &dc);
  void HilightInfoset(int pl,int iset);
	// Gives access to the parent to the private draw_settings. Used for SolnShow
	const TreeDrawSettings &DrawSettings(void) {return draw_settings;}
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
	gString	ProbAsString(const Node *n,int action) const ;
	gString	OutcomeAsString(const Node *n) const;
	// Constructor
	TreeWindow(Efg<T> &ef_,ExtensiveShow<T> *frame);
	// Destructor
	~TreeWindow();
		// Menu events
	void tree_outcomes(const gString out_name=gString(),int save_num=0);
	void action_probs(void);
	void node_outcome(const gString out_name);
	void file_save(void);
#ifdef SUBGAMES
	// Subgame handlers
	void OpenSubgame(int num);
	void CloseSubgame(int num);
	void MakeSubgame(void);
#endif
};

#endif   // TREEWINDOW_H

