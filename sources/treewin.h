//
// FILE: treewin.h -- Interface for TreeWindow class
//
// @(#)treewin.h	1.4 4/7/94
//

#ifndef TREEWINDOW_H
#define TREEWINDOW_H
#include "treecons.h"		//	Default values for tree display
#include "twiniter.h"
#include "flasher.h"

class TreeDrawParams   {
friend class DisplayOptionsForm;
	private:
  // These are user-definable
		int branch_length, node_length, y_spacing,x_origin, y_origin;
		char *chance_color, *cursor_color;
    Bool show_labels,show_outcomes,show_infosets;
	// These are used internally
  	int x_scroll,y_scroll;
		gRect window;

  public:
		TreeDrawParams(void)
		{
			branch_length=BRANCH_LENGTH_DEFAULT;
			node_length=NODE_LENGTH_DEFAULT;
			y_spacing=Y_SPACING_DEFAULT;
			x_origin=0;y_origin=0;
			chance_color=new char[20];
      cursor_color=new char[20];
			strcpy(chance_color,CHANCE_COLOR_DEFAULT);
			strcpy(cursor_color,CURSOR_COLOR_DEFAULT);
			show_labels=TRUE;show_outcomes=TRUE;show_infosets=FALSE;
		}

		void SetXOrigin(int xo)        { x_origin = xo; }
    int XOrigin(void) const        { return x_origin; }

    void SetYOrigin(int yo)        { y_origin = yo; }
    int YOrigin(void) const        { return y_origin; }

		int BranchLength(void) 			   { return branch_length; }
		int NodeLength(void) 			     { return node_length; }
		int YSpacing(void) 			       { return y_spacing; }
		char *ChanceColor(void) 		   { return chance_color; }
		char *CursorColor(void) 		   { return cursor_color; }

//    void SetIterator(ExtFormIter *i)     { iterator = i; }
//    ExtFormIter *Iterator(void) const    { return iterator; }

    void SetWindow(const gRect &r)    { window = r; }
		gRect Window(void) const          { return window; }

		void 	SetShowLabels(Bool _s) {show_labels=_s;}
		int		ShowLabels(void) {return show_labels;}
		void 	SetShowOutcomes(Bool _s) {show_outcomes=_s;}
    int		ShowOutcomes(void) {return show_outcomes;}
		void	SetShowInfosets(Bool _s) {show_infosets=_s;}
		int		ShowInfosets(void) {return show_infosets;}

		// Functions for internal use by gambit
		void set_x_scroll(int xs) {x_scroll=xs;}
		int get_x_scroll(void) {return x_scroll;}
		void set_y_scroll(int ys) {y_scroll=ys;}
    int get_y_scroll(void) {return y_scroll;}
};




class TreeWindow : public wxCanvas
{
friend class DisplayOptionsForm;
	typedef struct   {
			int x, y, level, color;
			Node n;
	}  NodeEntry;
		// Private variables
		Problem *the_problem;
		wxFrame *frame;								// parent frame
		Node mark_node;								// Used in mark/goto node operations
		TreeDrawParams draw_settings;	// Stored drawing parameters
    TreeWinIter *iterator;				// Used to process cursor keys
		TreeNodeFlasher *flasher;			// Used to flash the cursor
		wxList *node_list;						// Data for display coordinates of nodes
		float zoom_factor;

    // Private Functions
		void 	RenderSubtree(wxDC &dc,const Node &n,wxList *node_list);
		int 	FillTable(wxList *node_list,const Node &n, int level);
		void 	ProcessCursor(void);
		void 	ProcessClick(int x,int y);
    NodeEntry *GetNodeEntry(const Node &n);
	public:
		TreeWindow(wxFrame *frame, int x, int y, int w, int h, Problem *p = NULL,int style = wxRETAINED);
		void OnPaint(void);
		void OnEvent(wxMouseEvent& event);
    void OnChar(wxKeyEvent& ch);

		void node_add(void);
		void node_outcome(void);
		void node_game(void);
		void node_label(void);
		void node_delete(void);
		void node_insert(void);
		void node_set_mark(void);
		void node_goto_mark(void);

		void branch_label(void);
		void branch_insert(void);
		void branch_delete(void);

		void tree_delete(void);
		void tree_copy(void);
		void tree_label(void);
		void tree_outcomes(void);
		void tree_players(void);

		void infoset_member(void);
		void infoset_break(void);
		void infoset_join(void);

    void edit_outcome(void);

    gString Title(void) const   { return the_problem->GetTitle(); }

		void Render(wxDC &dc);
		void SetZoom(void);
		float GetZoom(void) {return zoom_factor;}

		void	print_eps(void);			// output to postscript file
		void	print(void);					// output to printer (WIN3.1 only)
		void	print_mf(void);				// copy to clipboard (WIN3.1 only)
    ~TreeWindow();
};

class ExtensiveFrame : public wxFrame
{
	private:

  public:
		TreeWindow *tw;                                                                          
		ExtensiveFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int type, Problem *p=NULL);
		Bool OnClose(void);
		void OnMenuCommand(int id);
    ~ExtensiveFrame();
};

class DisplayOptionsForm: public wxForm
{
 public:
	void EditForm(TreeWindow *object, wxPanel *panel);
};

#endif   // TREEWINDOW_H

