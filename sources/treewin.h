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

class BaseTreeWindow: public wxCanvas
{
friend class ExtensivePrintout;

private:
	BaseExtensiveShow	*frame;
	BaseExtForm		&ef;
	Node	*mark_node,*old_mark_node;		// Used in mark/goto node operations
	TreeNodeCursor *flasher;			// Used to flash the cursor
	gList<NodeEntry *> node_list;		// Data for display coordinates of nodes
	Bool		nodes_changed;    		// Used to determine if a node_list recalc
	Bool		infosets_changed;			// is needed
	Bool		need_clear;						// Do we need to clear the screen?
#ifdef SUBGAMES
	// Subgames are no longer implemented, but the code is in there, just in case
	int			subgame;							// which subgame I am.  1 for root
		typedef struct SUBGAMESTRUCT{
		friend gOutput &operator<<(gOutput &op,const SUBGAMESTRUCT &S);
										int num;ExtensiveShow<T> *win;
										SUBGAMESTRUCT(void)
											{num=0;win=NULL;}
										SUBGAMESTRUCT(const int _num)
											{num=_num;win=NULL;}
										SUBGAMESTRUCT(int _num,ExtensiveShow<T> *_win)
											{num=_num;win=_win;}
										SUBGAMESTRUCT &operator=(const SUBGAMESTRUCT &S)
											{num=S.num;win=S.win;return (*this);}
										int operator==(const SUBGAMESTRUCT &S)
											{return (num==S.num);}
										int operator!=(const SUBGAMESTRUCT &S)
											{return (num!=S.num);}
										} subgame_struct;

		gArray<subgame_struct>	open_subgames;			// Used to keep track of which subgames are already open
#endif

		// Private Functions
		void	RenderLabels(wxDC &dc,NodeEntry *entry);
		void 	RenderSubtree(wxDC &dc);
		int 	FillTable(const Node *n, int level);
		void 	ProcessCursor(void);
		void 	ProcessClick(int x,int y);
		NodeEntry *GetNodeEntry(const Node *n);
		NodeEntry *NextInfoset(const NodeEntry * const e);
		void	FillInfosetTable(const Node *n);
		void	CheckInfosetEntry(NodeEntry *e);
		void	UpdateTableInfosets(void);
		void	UpdateTableParents(void);
	// These functions are type dependent and are defined in TreeWindow
		virtual double 	ProbAsDouble(const Node *n,int action) { }
		virtual gString	ProbAsString(const Node *n,int action) { }
		virtual gString	OutcomeAsString(const Node *n) { }
protected:
	Node	 *cursor;										// Used to process cursor keys, stores current pos
	TreeDrawSettings draw_settings;		// Stores drawing parameters
public:
	// Constructor
  BaseTreeWindow(const BaseTreeWindow &bt);
	BaseTreeWindow(BaseExtForm &ef_,BaseExtensiveShow *frame,int x=-1,int y=-1,int w=-1,int h=-1,int style=0);
	// Destructor
	~BaseTreeWindow(void);
	// Windows event handlers
	void OnPaint(void);
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

	void 	output(void);
	void	print_eps(wxOutputOption fit);			// output to postscript file
	void	print(wxOutputOption fit,bool preview=false);	// output to printer (WIN3.1 only)
	void	print_mf(wxOutputOption fit,bool save_mf=false);				// copy to clipboard (WIN3.1 only)

	virtual void	file_save(void) =0;

	gString Title(void) const;

	void Render(wxDC &dc);
	// Gives access to the parent to the private draw_settings. Used for SolnShow
	const TreeDrawSettings &DrawSettings(void) {return draw_settings;}
};

template <class T> class ExtensiveShow;

template <class T>
class TreeWindow : public BaseTreeWindow
{
		// Private variables
		ExtForm<T> &ef;
		ExtensiveShow<T> *frame;			// parent frame
		// The copy/assignment operators are private since they are NEVER
		// used or implemented.  Perhaps later...
		TreeWindow(const TreeWindow<T> &);
		void operator=(const TreeWindow<T> &);

		double 	ProbAsDouble(const Node *n,int action);
		gString	ProbAsString(const Node *n,int action);
		gString	OutcomeAsString(const Node *n);
	public:
		// Constructor
		TreeWindow(ExtForm<T> &ef_,ExtensiveShow<T> *frame,int subgame=1,
							int x=-1,int y=-1,int w=-1,int h=-1,int style=0);
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

