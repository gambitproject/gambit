//
// FILE: treewin.h -- Interface for TreeWindow class
//
// @(#)treewin.h	1.4 4/7/94
//

#ifndef TREEWINDOW_H
#define TREEWINDOW_H
#include "twiniter.h"
#include "flasher.h"
#include "treedraw.h"

typedef struct   {
		int x, y, level, color;
		int infoset_y;	// y of the next node in the infoset to connect to
		int	num;				// # of the infoset line on this level
    int nums;				// sum of infosets previous to this level
		Node n;
}  NodeEntry;

class ExtensiveFrame;
class TreeWindow : public wxCanvas
{
friend class DisplayOptionsForm;
		// Private variables
		Problem *the_problem;
		ExtensiveFrame *frame;								// parent frame
		Node mark_node;								// Used in mark/goto node operations
		TreeDrawParams draw_settings;	// Stored drawing parameters
    TreeWinIter *iterator;				// Used to process cursor keys
		TreeNodeFlasher *flasher;			// Used to flash the cursor
		wxList *node_list;						// Data for display coordinates of nodes
		float zoom_factor;

		// Private Functions
		void	RenderLabels(wxDC &dc,const Node &n);
		void 	RenderSubtree(wxDC &dc,const Node &n);
		int 	FillTable(const Node &n, int level);
		void 	ProcessCursor(void);
		void 	ProcessClick(int x,int y);
		NodeEntry *GetNodeEntry(const Node &n);
		NodeEntry *NextInfoset(NodeEntry *e);
		void	FillInfosetTable(const Node &n);
		void	CheckInfosetEntry(NodeEntry *e);
		void	UpdateTableInfosets(void);
	public:
		TreeWindow(ExtensiveFrame *frame, int x, int y, int w, int h, Problem *p = NULL,int style = wxRETAINED);
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
		void node_probs(void);

		void branch_label(void);
		void branch_insert(void);
		void branch_delete(void);

		void tree_delete(void);
		void tree_copy(void);
		void tree_move(void);
		void tree_label(void);
		void tree_outcomes(int out=-1);
		void tree_players(void);

		void infoset_merge(void);
		void infoset_break(void);
		void infoset_join(void);

    void edit_outcome(void);

		void display_legends(void);
		void display_set_zoom(float z=-1);
		float display_get_zoom(void) {return zoom_factor;}

		void	print_eps(void);			// output to postscript file
		void	print(void);					// output to printer (WIN3.1 only)
		void	print_mf(void);				// copy to clipboard (WIN3.1 only)

		void	file_save(void);

		gString Title(void) const   { return the_problem->GetTitle(); }

		void Render(wxDC &dc);

		~TreeWindow();
};

class ExtensiveFrame : public wxFrame
{
	private:
		wxList *accelerators;					// Used to process accelerator keys
	public:
		TreeWindow *tw;                                                                          
		ExtensiveFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int type, char *filename=NULL);
		Bool 		OnClose(void);
		wxList 	*MakeAccelerators(void);
		Bool		CheckAccelerators(int ch);
		void 		OnMenuCommand(int id);
		~ExtensiveFrame();
};

class DisplayOptionsForm: public wxForm
{
 public:
	void EditForm(TreeWindow *object, wxPanel *panel);
};

#endif   // TREEWINDOW_H

