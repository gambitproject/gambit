//
// FILE: treewin.h -- Interface for TreeWindow class
//
// $Id$
//

#ifndef TREEWIN_H
#define TREEWIN_H

#include "garray.h"
#include "glist.h"
#include "treedraw.h"
#include "efgconst.h"

typedef struct NODEENTRY 
{
    int x, y, level, color;
    // pos of the next node in the infoset to connect to
    struct { int x, y; } infoset;
    int num;            // # of the infoset line on this level
    int nums;           // sum of infosets previous to this level
    int has_children;   // how many children this node has
    int child_number;   // what branch # is this node from the parent
    bool in_sup;        // is this node in disp_sup
    const Node *n;
    NODEENTRY *parent;
    bool expanded;      // Is this subgame root expanded or collapsed?
    
    NODEENTRY(void) { }
    NODEENTRY(const NODEENTRY &e): 
        x(e.x), y(e.y), level(e.level), color(e.color),
        infoset(e.infoset),num(e.num),
        nums(e.nums),has_children(e.has_children),
        child_number(e.child_number),in_sup(e.in_sup),
        n(e.n),expanded(e.expanded) { }
} NodeEntry;

class EfgShow;
class TreeWindow;
class TreeNodeCursor;


//
// This class can render an extensive form tree using a pre-calculated
// (in TreeWindow) list of NodeEntry.  It is used for rendering
// functions for the main TreeWindow display and for the optional
// 'unity zoom' zoom window.  Note that it does not have any data
// members of its own, but just references to those of its parent. This
// way we do not duplicate the data and only need one assignment regardless
// of the number of renderers.
//

class TreeRender : public wxCanvas
{
private:
    const gList<NodeEntry *> &node_list;
    const Infoset * &hilight_infoset;    // Hilight infoset from the solution display
    const Infoset * &hilight_infoset1;   // Hilight infoset by pressing control
    const Node  *&mark_node;             // Used in mark/goto node operations
    const Node *&subgame_node;
    
    // Private Functions
    
    void RenderLabels(wxDC &dc, const NodeEntry *child_entry,
                      const NodeEntry *entry);
    void RenderSubtree(wxDC &dc);
  gText OutcomeAsString(const Node *n, bool &hilight) const;
    
protected:
    TreeWindow *parent;
    const TreeDrawSettings &draw_settings;  // Stores drawing parameters
    TreeNodeCursor *flasher;                // Used to flash/display the cursor
    bool painting;                          // Used to prevent re-entry.

public:
    TreeRender(wxFrame *frame, TreeWindow *parent,
               const gList<NodeEntry *> &node_list,
               const Infoset * &hilight_infoset_,
               const Infoset * &hilight_infoset1_,
               const Node *&mark_node_, const Node *&subgame_node,
               const TreeDrawSettings &draw_settings_);
    virtual ~TreeRender(void);
    
    // Windows event handlers
    virtual void OnPaint(void);
    virtual void Render(wxDC &dc);
    
    // Call this every time the cursor moves.
    virtual void UpdateCursor(const NodeEntry *entry);
    
    // Override this if extra functionality is desired.
    virtual Bool JustRender(void) const;
    
    // This must be here since we do not have draw_settings at constructor time
    void MakeFlasher(void);
};


class TreeZoomWindow : public TreeRender {
private:
  // Real xs,ys,xe,ye: true (untranslated) coordinates of the cursor.  
  // Need this if we repaint due to events other than cursor movement.  
  // See ::Render()
  int xs, ys, xe, ye;
  TreeWindow *m_parent;

public:
  TreeZoomWindow(wxFrame *frame, TreeWindow *parent,
		 const gList<NodeEntry *> &node_list,
		 const Infoset * &hilight_infoset_,
		 const Infoset * &hilight_infoset1_,
		 const Node *&mark_node_, const Node *&subgame_node,
		 const TreeDrawSettings &draw_settings_,
		 const NodeEntry *cursor_entry);
  virtual void Render(wxDC &dc);
  // Makes sure the cursor is always in the center of the window
  virtual void UpdateCursor(const NodeEntry *entry);

  void OnChar(wxKeyEvent &);
};

class TreeWindow : public TreeRender   
{
    friend class ExtensivePrintout;

public:
    typedef struct SUBGAMEENTRY 
    {
        const Node *root;
        bool expanded;
        SUBGAMEENTRY(void) : root(0), expanded(true) { }
        SUBGAMEENTRY(const Node *r, bool e = true) : root(r), expanded(e) { }
        SUBGAMEENTRY(const SUBGAMEENTRY &s) : root(s.root),
            expanded(s.expanded) { }
        SUBGAMEENTRY &operator=(const SUBGAMEENTRY &s)
        { root = s.root; expanded = s.expanded; return (*this); }

        // Need these to make a list:
        int operator==(const SUBGAMEENTRY &s) { return (s.root == root); }
        int operator!=(const SUBGAMEENTRY &s) { return (s.root != root); }
        friend gOutput &operator<<(gOutput &, const SUBGAMEENTRY &);
    } SubgameEntry;

private:
    Efg &ef;
    EFSupport * &disp_sup;    // We only need to know the displayed support.
    EfgShow *frame;           // Actual extensive game show
    wxFrame *pframe;          // Our parent window

    Node  *mark_node,*old_mark_node;  // Used in mark/goto node operations
    const Node    *subgame_node;      // Used to mark the 'picking' subgame root
    gList<NodeEntry *> node_list;     // Data for display coordinates of nodes
    gList<SubgameEntry> subgame_list; // Keeps track of collapsed/expanded subgames

    Bool      nodes_changed;        // Used to determine if a node_list recalc is needed
    Bool      infosets_changed;
    Bool      must_recalc;          

    Bool      need_clear;           // Do we need to clear the screen?
    gOutput   *log;                 // Are we saving each action to a file?
    Infoset *hilight_infoset;       // Hilight infoset from the solution disp
    Infoset *hilight_infoset1;      // Hilight infoset by pressing control
    TreeRender *zoom_window;
    wxMenu    *edit_menu;           // a popup menu, equivalent to top level edit

    class NodeDragger;              // Class to take care of tree copy/move by
    NodeDragger *node_drag;         // drag and dropping nodes.

    class IsetDragger;              // Class to take care of iset join by
    IsetDragger *iset_drag;         // drag and dropping.

    class BranchDragger;            // Class to take care of branch addition by
    BranchDragger *branch_drag;     // drag and dropping

    // Private Functions
    int   FillTable(const Node *n,int level);
    void  ProcessCursor(void);
    void  ProcessClick(wxMouseEvent &ev);
    void  ProcessDClick(wxMouseEvent &ev);
    void  ProcessRClick(wxMouseEvent &ev);
    void  ProcessRDClick(wxMouseEvent &ev);
    bool  ProcessShift(wxMouseEvent &ev);
    NodeEntry *GetNodeEntry(const Node *n);
    NodeEntry *NextInfoset(const NodeEntry * const e);
    NodeEntry *GetValidParent(const Node *n);
    NodeEntry *GetValidChild(const Node *n);
    SubgameEntry &GetSubgameEntry(const Node *n);
    void  FillInfosetTable(const Node *n);
    void  CheckInfosetEntry(NodeEntry *e);
    void  UpdateTableInfosets(void);
    void  UpdateTableParents(void);
    static void OnPopup(wxMenu &ob,wxCommandEvent &ev);
    void MakeMenus(void);
    
protected:
    Node *m_cursor;  // Used to process cursor keys, stores current position.
    Bool    outcomes_changed;
    TreeDrawSettings draw_settings;  // Stores drawing parameters

  void SetCursorPosition(Node *p_cursor);
  void UpdateMenus(void);
    
public:
  // LIFECYCLE
  TreeWindow(Efg &ef_,EFSupport * &disp, EfgShow *frame);
  virtual ~TreeWindow(void);
    
  // EVENT HANDLERS
  void OnEvent(wxMouseEvent& event);
  void OnChar(wxKeyEvent& ch);
  void OnPaint(void);
    
  // MENU EVENT HANDLERS
  void node_add(void);
  void node_insert(void);
  void node_label(void);
  void node_delete(void);
  void node_set_mark(void);
  void node_goto_mark(void);
  void EditOutcomeAttach(void);
  void EditOutcomeDetach(void);
  void EditOutcomeNew(void);
  void EditOutcomeDelete(void);
  void EditOutcomeLabel(void);
  void EditOutcomePayoffs(void);
      
  void action_label(void);
  void action_insert(void);
  void action_append(void);
  void action_delete(void);
  void action_probs(void);
    
  void tree_delete(void);
  void tree_copy(void);
  void tree_move(void);
  void tree_label(void);
  void tree_players(void);
  void tree_infosets(void);
    
  void infoset_merge(void);
  void infoset_break(void);
  void infoset_split(void);
  void infoset_join(void);
  void infoset_label(void);
  void infoset_switch_player(void);
  void infoset_reveal(void);
    
  void SubgameMarkAll(void);
  void SubgameMark(void);
  void SubgameUnmarkAll(void);
  void SubgameUnmark(void);
  void SubgameCollapse(void);
  void SubgameCollapseAll(void);
  void SubgameExpand(void);
  void SubgameExpandBranch(void);
  void SubgameExpandAll(void);

  void subgame_toggle(void);
    
  void display_legends(void);
  void display_options(void);
  void display_colors(void);
  void display_save_options(Bool def=TRUE);
  void display_load_options(Bool def=TRUE);
  void display_set_zoom(float z=-1);
  void display_zoom_fit(void);
  float display_get_zoom(void);
  void display_zoom_win(void);
  void delete_zoom_win(void);
  
  Bool file_save(void);
  void  output(void);
  void  print_eps(wxOutputOption fit);                 // output to postscript file
  void  print(wxOutputOption fit,bool preview=false);  // output to printer (WIN3.1 only)
  
  // copy to clipboard (WIN3.1 only)
  void  print_mf(wxOutputOption fit,bool save_mf=false);
  
  gText Title(void) const { return ef.GetTitle(); }
  
  wxFrame *Parent(void) const { return pframe; }
  EfgShow *GetEfgFrame(void) const { return frame; }
  
  virtual void Render(wxDC &dc);
  void HilightInfoset(int pl,int iset);
  
  // Used by parent EfgShow when disp_sup changes
  void SupportChanged(void);
  
  // Gives access to the parent to the private draw_settings. Used for SolnShow
  TreeDrawSettings &DrawSettings(void) { return draw_settings; }
  
  // Gives access to the parent to the current cursor node
  Node *Cursor(void) const { return m_cursor; }
  
  // Hilight the subgame root for the currently active subgame
  void  SetSubgamePickNode(const Node *n);
  
  // Check if a drag'n'drop object has been activated
  Node *GotObject(float &mx, float &my, int what);
  virtual Bool JustRender(void) const { return FALSE; }
  
  // Adjust number of scrollbar steps if needed.
  void AdjustScrollbarSteps();
  
  // Access to the numeric values from the renderer
  gText AsString(TypedSolnValues what, const Node *n, int br = 0) const;
};

#endif   // TREEWIN_H

