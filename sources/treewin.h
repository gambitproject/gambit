#define DisplayDevice wxDC
//
// FILE: treewin.h -- Interface for TreeWindow class
//
// $Id$
//

#ifndef TREEWINDOW_H
#define TREEWINDOW_H
#include <string.h>
#include "treecons.h"

class TreeDrawParams   {
friend class DisplayOptionsForm;
	private:
  // These are user-definable
		int branch_length, node_length, y_spacing,x_origin, y_origin;
		char *chance_color, *cursor_color;
    Bool show_labels,show_outcomes,show_infosets;
	// These are used internally
  	int x_scroll,y_scroll;
    ExtFormIter *iterator;
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
			iterator=0;
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

    void SetIterator(ExtFormIter *i)     { iterator = i; }
    ExtFormIter *Iterator(void) const    { return iterator; }

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

typedef struct   {
    int x, y, level, color;
}  NodeEntry;


class TreeNodeFlasher;

class TreeWindow : public wxCanvas
{
friend class DisplayOptionsForm;
		// Private variables
		ExtForm *the_problem;
    ExtFormIter *iterator;
// TLT
		Node mark_node;
    TreeDrawParams draw_settings;
		wxFrame *frame;				// parent frame
		TreeNodeFlasher *_flasher;
		// node_table has been moved here to facilitate the cursor and mouse control
		NodeEntry *node_table;
		float zoom_factor;

    // Private Functions
// TLT
		void RenderSubtree(DisplayDevice &, Node &, NodeEntry *);
		int TreeWindow::FillTable(NodeEntry *table, Node &n, int level);
		void ProcessCursor(void);
		void ProcessClick(int x,int y);
	public:
		TreeWindow(wxFrame *frame, int x, int y, int w, int h, ExtForm *p = 0,int style = wxRETAINED);
		void OnPaint(void);
		void OnEvent(wxMouseEvent& event);
    void OnChar(wxKeyEvent& ch);

    int node_add(void);
    int node_outcome(void);
    int node_game(void);
    int node_label(void);
    int node_delete(void);
    int node_insert(void);
    int node_set_mark(void);
    int node_goto_mark(void);

    int branch_label(void);
    int branch_insert(void);
    int branch_delete(void);

    int tree_delete(void);
		int tree_copy(void);
    int tree_label(void);
		int tree_outcomes(void);
		int tree_players(void);

    int infoset_member(void);
    int infoset_break(void);
    int infoset_join(void);

    int edit_outcome(void);

    gString Title(void) const   { return the_problem->TreeLabel(); }

		void Render(DisplayDevice &,int render_device);
		void SetZoom(void);
    float GetZoom(void) {return zoom_factor;}
    ~TreeWindow();
};

class ExtensiveFrame : public wxFrame
{
  private:

  public:
		TreeWindow *tw;
		ExtensiveFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int type, ExtForm * =0);
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

