//
// FILE: treewin.h -- Interface for TreeWindow class
//
// @(#)treewin.h	1.4 4/7/94
//

#ifndef TREEWINDOW_H
#define TREEWINDOW_H
#include "gblock.h"
#include "twiniter.h"
#include "twflash.h"
#include "treedraw.h"

typedef struct NODEENTRY {
		int x, y, level, color;
		int infoset_y;	// y of the next node in the infoset to connect to
		int	num;				// # of the infoset line on this level
		int nums;				// sum of infosets previous to this level
		int has_children;	// how many children this node has
		int child_number;	// what branch # is this node from the parent
		Node n;
		NODEENTRY *parent;
}  NodeEntry;

template <class T> class ExtensiveShow;

class BaseTreeWindow
{
public:
	virtual void node_outcome(int out=0) =0;
	virtual void tree_outcomes(int out=0) =0;
	virtual void OnPaint(void) =0;
};

template <class T>
class TreeWindow : public BaseTreeWindow, public wxCanvas
{
		// Private variables
		ExtForm<T> *the_problem;
		int			subgame;							// which subgame I am.  1 for root
		ExtensiveShow<T> *frame;			// parent frame
		Node mark_node,old_mark_node;	// Used in mark/goto node operations
		TreeDrawSettings draw_settings;	// Stores drawing parameters
		TreeWinIter *iterator;				// Used to process cursor keys
		TreeNodeCursor *flasher;			// Used to flash the cursor
		wxList *node_list;						// Data for display coordinates of nodes
		Bool		nodes_changed;    		// Used to determine if a node_list recalc
		Bool		infosets_changed;			// is needed
#ifdef SUBGAMES
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

		gBlock<subgame_struct>	open_subgames;			// Used to keep track of which subgames are already open
#endif
		// The copy/assignment operators are private since they are NEVER
		// used or implemented.  Perhaps later...
		TreeWindow(const TreeWindow<T> &);
		void operator=(const TreeWindow<T> &);
		// Private Functions
		void	RenderLabels(wxDC &dc,NodeEntry *entry);
		void 	RenderSubtree(wxDC &dc);
		int 	FillTable(const Node &n, int level);
		void 	ProcessCursor(void);
		void 	ProcessClick(int x,int y);
		NodeEntry *GetNodeEntry(const Node &n);
		NodeEntry *NextInfoset(NodeEntry *e);
		void	FillInfosetTable(const Node &n);
		void	CheckInfosetEntry(NodeEntry *e);
		void	UpdateTableInfosets(void);
		void	UpdateTableParents(void);
	public:
		// Constructor
		TreeWindow(ExtForm<T> *p,ExtensiveShow<T> *frame,int subgame=1,
							int x=-1,int y=-1,int w=-1,int h=-1,int style=wxRETAINED);
		// Destructor
		~TreeWindow();
		// Windows event handlers
		void OnPaint(void);
		void OnEvent(wxMouseEvent& event);
		void OnChar(wxKeyEvent& ch);
#ifdef SUBGAMES
		// Subgame handlers
		void OpenSubgame(int num);
		void CloseSubgame(int num);
		void MakeSubgame(void);
#endif
		// Menu event handlers (these are mostly in treewin1.cc)
		void node_add(void);
		void node_outcome(int out);
		void node_game(void);
		void node_label(void);
		void node_delete(void);
		void node_insert(void);
		void node_set_mark(void);
		void node_goto_mark(void);
		void node_probs(void);
		void node_subgame(int _game);

		void branch_label(void);
		void branch_insert(void);
		void branch_delete(void);

		void tree_delete(void);
		void tree_copy(void);
		void tree_move(void);
		void tree_label(void);
		void tree_outcomes(int out=0);
		void tree_players(void);
		void tree_subgames(void);
		void tree_subgame_make(void);
		void tree_subgame_open(int _game);
		void tree_subgame_delete(int _game);
		void tree_subgame_name(int _game,gString _label);

		void infoset_merge(void);
		void infoset_break(void);
		void infoset_join(void);

		void edit_outcome(void);

		void display_legends(void);
		void display_options(void);
		void display_save_options(Bool def=TRUE);
		void display_load_options(Bool def=TRUE);
		void display_set_zoom(float z=-1);
		float display_get_zoom(void);

		void 	output(void);
		void	print_eps(void);			// output to postscript file
		void	print(void);					// output to printer (WIN3.1 only)
		void	print_mf(void);				// copy to clipboard (WIN3.1 only)

		void	file_save(void);

		gString Title(void) const;

		void Render(wxDC &dc);

};

#endif   // TREEWINDOW_H

