//
// FILE: treewin.cc -- Drawing functions for TreeWindow
//
// $Id$
//

#include "wx.h"
#include "wxmisc.h"
#include "gmisc.h"
#include "efg.h"
#include "legendc.h"
#include "treewin.h"
#include "twflash.h"
#include "efgshow.h"

#include "glist.imp"

#define INFOSET_SPACING         10
#define SUBGAME_LARGE_ICON_SIZE 20
#define SUBGAME_SMALL_ICON_SIZE 10
#define SUBGAME_PICK_SIZE       30

#define DRAG_NODE_START         0       // What are we dragging
#define DRAG_NODE_END           1
#define DRAG_ISET_START         2
#define DRAG_ISET_END           3
#define DRAG_BRANCH_START       4
#define DRAG_BRANCH_END         5
#define DRAG_OUTCOME_START      6       // also defined in btreewn1.cc
#define DRAG_OUTCOME_END        7

#define DRAG_NONE               0       // Current drag state
#define DRAG_START              1
#define DRAG_CONTINUE           2
#define DRAG_STOP               3

#define TOP_MARGIN              40
#define DELTA                   8
#define MAX_TW                  60
#define MAX_TH                  20


wxFont   *outcome_font;
wxBrush  *white_brush;
wxCursor *scissor_cursor;


//-----------------------------------------------------------------------
//                    MISCELLANEOUS FUNCTIONS
//-----------------------------------------------------------------------

extern Bool LongStringConstraint(int type, char *value, char *label,
                 char *msg_buffer);

//
// Draw Line.  A quick and dirty way of easily drawing lines with a set color.
// If the color is == -1, the current color is used.
// If -1 < color < WX_COLOR_LIST_LENGTH, the corresponding color from the
// wx_color_list is used.
// If WX_COLOR_LIST_LENGTH <= color < 2*WX_COLOR_LIST_LENGTH,
// a color equal to color % WX_COLOR_LIST_LENGTH is used from the
// wx_hilight_color_list.
//

inline void DrawLine(wxDC &dc, double x_s, double y_s, double x_e, double y_e,
                     int color = 0, int thick = 0)
{
    if (dc.Colour)  
    {
        if (color > -1 && color < WX_COLOR_LIST_LENGTH)
        {
            dc.SetPen(wxThePenList->FindOrCreatePen((char *) wx_color_list[color],
                                                    (thick) ? 8 : 2, wxSOLID));
        }
        
        if (color >= WX_COLOR_LIST_LENGTH && color < 2 * WX_COLOR_LIST_LENGTH)
        {
            dc.SetPen(wxThePenList->FindOrCreatePen(
                    (char *) wx_hilight_color_list[color % WX_COLOR_LIST_LENGTH],
                    2, wxSOLID));
        }
    }
    else
        dc.SetPen(wxThePenList->FindOrCreatePen("BLACK", 2, wxSOLID));
    
    dc.DrawLine(x_s, y_s, x_e, y_e);
}


//
// Draw Rectangle.
// A quick and dirty way of easily drawing rectangles with a set color.
//

inline void DrawRectangle(wxDC &dc, int x_s, int y_s, int w, int h,
                          int color = 0)
{
    if (color > -1)
    {
        dc.SetPen(wxThePenList->FindOrCreatePen((char *) wx_color_list[color], 
                                                2, wxSOLID));
    }

    dc.DrawRectangle(x_s, y_s, w, h);
}


//
// Draw Thin Line.  A quick and dirty way of easily drawing lines with a set color.
//


inline void DrawThinLine(wxDC &dc, int x_s, int y_s, int x_e, int y_e,
                         int color = 0)
{
    if (dc.Colour && color > -1)
        dc.SetPen(wxThePenList->FindOrCreatePen((char *)wx_color_list[color], 1, wxSOLID));
    else
        dc.SetPen(wxThePenList->FindOrCreatePen("BLACK", 1, wxSOLID));
    
    dc.DrawLine(x_s, y_s, x_e, y_e);
}

//
// Draw Dashed Line.  
// A quick and dirty way of easily drawing lines with a set color.
//

inline void DrawDashedLine(wxDC &dc, int x_s, int y_s, int x_e, int y_e,
                           int color = 0)
{
    if (dc.Colour && color > -1)
        dc.SetPen(wxThePenList->FindOrCreatePen((char *) wx_color_list[color],
                                                1, wxSHORT_DASH));
    else
        dc.SetPen(wxThePenList->FindOrCreatePen("BLACK", 1, wxSOLID));
    
    dc.DrawLine(x_s, y_s, x_e, y_e);
}

//
// Draw Circle. A quick and dirty way of easily drawing a circle with a set color.
//

inline void DrawCircle(wxDC &dc, int x, int y, int r, int color = 0)
{
    if (color > -1 && dc.Colour)
        dc.SetPen(wxThePenList->FindOrCreatePen((char *) wx_color_list[color],
                                                3, wxSOLID));
    else
        dc.SetPen(wxThePenList->FindOrCreatePen("BLACK", 3, wxSOLID));
    dc.DrawEllipse(x-r, y-r, 2*r, 2*r);
}



//
// Draw collapsed subgame icons
//

void DrawLargeSubgameIcon(wxDC &dc, const NodeEntry &entry, int nl)
{
    dc.SetPen(wxThePenList->FindOrCreatePen("INDIAN RED", 2, wxSOLID));
    dc.SetBrush(wxTheBrushList->FindOrCreateBrush("RED", wxSOLID));
    wxPoint points[3];
    int x0 = entry.x + nl + entry.nums * INFOSET_SPACING - SUBGAME_LARGE_ICON_SIZE;
    int y0 = entry.y;
    points[0].x = x0;
    points[0].y = y0;
    points[1].x = x0 + SUBGAME_LARGE_ICON_SIZE;
    points[1].y = y0 - SUBGAME_LARGE_ICON_SIZE/2;
    points[2].x = x0 + SUBGAME_LARGE_ICON_SIZE;
    points[2].y = y0 + SUBGAME_LARGE_ICON_SIZE/2;
    dc.DrawPolygon(3, points);
}

void DrawSmallSubgameIcon(wxDC &dc, const NodeEntry &entry)
{
    dc.SetPen(wxThePenList->FindOrCreatePen("INDIAN RED", 2, wxSOLID));
    dc.SetBrush(wxTheBrushList->FindOrCreateBrush("RED", wxSOLID));
    wxPoint points[3];
    points[0].x = entry.x;
    points[0].y = entry.y;
    points[1].x = entry.x+SUBGAME_SMALL_ICON_SIZE;
    points[1].y = entry.y-SUBGAME_SMALL_ICON_SIZE/2;
    points[2].x = entry.x+SUBGAME_SMALL_ICON_SIZE;
    points[2].y = entry.y+SUBGAME_SMALL_ICON_SIZE/2;
    dc.DrawPolygon(3, points);
}


void DrawSubgamePickIcon(wxDC &dc, const NodeEntry &entry)
{
    dc.SetPen(wxThePenList->FindOrCreatePen("BLACK", 2, wxSOLID));
    dc.DrawLine(entry.x, entry.y - SUBGAME_PICK_SIZE/2,
                entry.x, entry.y + SUBGAME_PICK_SIZE/2);
    dc.DrawLine(entry.x, entry.y - SUBGAME_PICK_SIZE/2,
                entry.x + SUBGAME_PICK_SIZE/2, entry.y - SUBGAME_PICK_SIZE/2);
    dc.DrawLine(entry.x, entry.y + SUBGAME_PICK_SIZE/2,
                entry.x + SUBGAME_PICK_SIZE/2, entry.y + SUBGAME_PICK_SIZE/2);
}


//-----------------------------------------------------------------------
//                      TreeRender: Member functions
//-----------------------------------------------------------------------

TreeRender::TreeRender(wxFrame *frame, TreeWindow *parent_,
                       const gList<NodeEntry *> &node_list_,
                       const Infoset * &hilight_infoset_,
                       const Infoset * &hilight_infoset1_,
                       const Node *&mark_node_,
                       const Node *&cursor_, const Node *&subgame_node_,
                       const TreeDrawSettings &draw_settings_)
    : wxCanvas(frame, -1, -1, -1, -1, 0),
      node_list(node_list_),
      hilight_infoset(hilight_infoset_), hilight_infoset1(hilight_infoset1_),
      mark_node(mark_node_), subgame_node(subgame_node_), cursor(cursor_),
      parent(parent_), draw_settings(draw_settings_),
      flasher(0), painting(false)
{ }


//
// OnPaint: Handle drawing events
//

void TreeRender::OnPaint(void)
{
    if (painting) 
        return; // prevent re-entry
    painting = true;
    Render(*GetDC());
    painting = false;
}

//
// RenderLabels:  Draws all the text labels for the tree according to the
// settings in draw_settings.  Currently takes care of:labels node/branch,
// outcomes.  Note: this function is getting very long, but I see no real
// reason to split it at this point...
//

void TreeRender::RenderLabels(wxDC &dc, const NodeEntry *child_entry,
                              const NodeEntry *entry)
{
    gText label = "";     // temporary to hold the label
    const Node *n = child_entry->n;
    float tw, th;
    bool hilight = false;
    
    // First take care of labeling the node on top.
    
    switch (draw_settings.LabelNodeAbove())   
    {
    case NODE_ABOVE_NOTHING:
        label ="";  
        break;
        
    case NODE_ABOVE_LABEL:
        label = n->GetName();  
        break;
        
    case NODE_ABOVE_PLAYER:
        if (n->GetPlayer()) 
            label = n->GetPlayer()->GetName();
        else label = "";
        break;
        
    case NODE_ABOVE_ISETLABEL:
        if (n->GetInfoset()) 
            label = n->GetInfoset()->GetName();
        break;
        
    case NODE_ABOVE_ISETID:
        if (n->GetInfoset())
            label = "(" + ToText(n->GetPlayer()->GetNumber()) +
                "," + ToText(n->GetInfoset()->GetNumber()) + ")";
        break;
        
    case NODE_ABOVE_OUTCOME:
        label = parent->OutcomeAsString(n, hilight);
        break;
        
    case NODE_ABOVE_REALIZPROB:
        label = parent->AsString(tRealizProb, n);
        break;
        
    case NODE_ABOVE_BELIEFPROB:
        label = parent->AsString(tBeliefProb, n);
        break;
        
    case NODE_ABOVE_VALUE:
        label = parent->AsString(tNodeValue, n);
        break;
        
    default:
        label = "";
        break;
    }
    
    if (label != "")
    {
        dc.SetFont(draw_settings.NodeAboveFont());
        dc.GetTextExtent("0", &tw, &th);
        gDrawText(dc, label, 
                  child_entry->x + child_entry->nums * INFOSET_SPACING + 3,
                  child_entry->y - th - 9);
    }
    
    // Take care of labeling the node on the bottom.
    
    label = "";
    
    switch (draw_settings.LabelNodeBelow())   
    {
    case NODE_BELOW_NOTHING:
        label = "";
        break;
        
    case NODE_BELOW_LABEL:
        label = n->GetName();
        break;
        
    case NODE_BELOW_PLAYER:
        if (n->GetPlayer()) 
            label = n->GetPlayer()->GetName(); 
        else 
            label = ""; 
        break;
        
    case NODE_BELOW_ISETLABEL:
        if (n->GetInfoset()) 
            label = n->GetInfoset()->GetName();
        break;
        
    case NODE_BELOW_ISETID:
        if (n->GetInfoset())
            label = "(" + ToText(n->GetPlayer()->GetNumber()) +
                "," + ToText(n->GetInfoset()->GetNumber()) + ")";
        break;
        
    case NODE_BELOW_OUTCOME:
        label = parent->OutcomeAsString(n, hilight);
        break;
        
    case NODE_BELOW_REALIZPROB:
        label = parent->AsString(tRealizProb, n);
        break;
        
    case NODE_BELOW_BELIEFPROB:
        label = parent->AsString(tBeliefProb, n);
        break;
        
    case NODE_BELOW_VALUE:
        label = parent->AsString(tNodeValue, n);
        break;
        
    default:
        label = "";
        break;
    }
    
    if (label != "")
    {
        dc.SetFont(draw_settings.NodeBelowFont());
        gDrawText(dc, label, 
                  child_entry->x + child_entry->nums * INFOSET_SPACING + 3,
                  child_entry->y + 5);
    }
    
    if (child_entry->n != entry->n)   // no branches for root
    {   
        // Now take care of branches....
        
        // Take care of labeling the branch on the top.
        
        label = "";
        
        switch (draw_settings.LabelBranchAbove())
        {
        case BRANCH_ABOVE_NOTHING:
            label = "";
            break;
            
        case BRANCH_ABOVE_LABEL:
            if (child_entry->child_number != 0)
                label = entry->n->GetInfoset()->GetActionName(child_entry->child_number);
            else
                label = "";
            break;
            
        case BRANCH_ABOVE_PLAYER:
            if (entry->n->GetPlayer()) 
                label = entry->n->GetPlayer()->GetName();
            break;
            
        case BRANCH_ABOVE_PROBS:
            label = parent->AsString(tBranchProb, entry->n, child_entry->child_number);
            break;
            
        case BRANCH_ABOVE_VALUE:
            label = parent->AsString(tBranchVal, entry->n, child_entry->child_number);
            break;
            
        default:
            label = "";
            break;
        }
        
        if (label != "")  
        {
            dc.SetFont(draw_settings.BranchAboveFont());
            dc.GetTextExtent("0", &tw, &th);
            gDrawText(dc, label, 
                      entry->x + entry->nums * INFOSET_SPACING + 
                      draw_settings.ForkLength() + draw_settings.NodeLength() + 3,
                      child_entry->y - th - 5);
        }
        
        // Take care of labeling the branch on the bottom.
        
        label = "";
        
        switch (draw_settings.LabelBranchBelow())    
        {
        case BRANCH_BELOW_NOTHING:
            label = "";
            break;
            
        case BRANCH_BELOW_LABEL:
            if (child_entry->child_number != 0)
                label = entry->n->GetInfoset()->GetActionName(child_entry->child_number);
            else
                label = "";
            break;
            
        case BRANCH_BELOW_PLAYER:
            if (entry->n->GetPlayer()) 
                label = entry->n->GetPlayer()->GetName();
            break;
            
        case BRANCH_BELOW_PROBS:
            label = parent->AsString(tBranchProb, entry->n, child_entry->child_number);
            break;
            
        case BRANCH_BELOW_VALUE:
            label = parent->AsString(tBranchVal, entry->n, child_entry->child_number);
            break;
            
        default:
            label = "";
            break;
        }
        
        if (label != "")
        {
            dc.SetFont(draw_settings.BranchBelowFont());
            gDrawText(dc, label,
                      entry->x + entry->nums * INFOSET_SPACING + 
                      draw_settings.ForkLength() + draw_settings.NodeLength() + 3,
                      child_entry->y + 5);
        }
    }
    
    // Now take care of displaying the terminal node labels.
    
    label = "";
    hilight = false;
    
    switch (draw_settings.LabelNodeRight()) { 
    case NODE_RIGHT_NOTHING:
      label = "";
      break;
            
    case NODE_RIGHT_OUTCOME:
      label = parent->OutcomeAsString(n, hilight);
      break;
            
    case NODE_RIGHT_NAME:
      if (n->GetOutcome()) 
	label = n->GetOutcome()->GetName();
      break;
            
    default:
      label = "";
      break;
    }
        
    if (label != "") { 
      dc.SetFont(draw_settings.NodeRightFont());
      gDrawText(dc, label,
		child_entry->x + draw_settings.NodeLength() +
		child_entry->nums * INFOSET_SPACING + 10,
		child_entry->y - 12);
    }
}


//
// RenderSubtree: Render branches and labels
//
// The following speed optimizations have been added:
// The algorithm now traverses the tree as a linear linked list, eliminating
// expensive searches.  Since the region clipping implemented by wxwin seems
// to be less than optimal, I add rudimentary clipping of my own.
// The offset is used to simulate scrollbars in the
// zoom window.  It might be used for the main window if scrollbars prove to
// be a limitation.
//
void TreeRender::RenderSubtree(wxDC &dc)
{
    // x-start, x-end, y-start, y-end: coordinates for drawing branches 
    int xs, xe, ys, ye;
    NodeEntry entry, child_entry;
    // Determine the visible region on screen to implement clipping
    int x_start, y_start, width, height;
    
    ViewStart(&x_start, &y_start);
    GetClientSize(&width, &height);

    // go through the list of nodes, plotting them
    for (int pos = 1; pos <= node_list.Length(); pos++)
    {
        child_entry = *node_list[pos];    // must make a copy to use Translate
        entry = *child_entry.parent;
        
        // If we are just a renderer, there can be no zoom! 
        // For zoom, override JustRender().
        float zoom = (JustRender()) ? 1.000 : draw_settings.Zoom();

        // Check if this node/labels are visible
        if (!(child_entry.x+dc.device_origin_x < x_start*PIXELS_PER_SCROLL  ||
              entry.x+dc.device_origin_x > x_start*PIXELS_PER_SCROLL+width/zoom ||
              (entry.y+dc.device_origin_y > y_start*PIXELS_PER_SCROLL+height/zoom &&
               child_entry.y+dc.device_origin_y > y_start*PIXELS_PER_SCROLL+height/zoom) ||
              (entry.y+dc.device_origin_y < y_start*PIXELS_PER_SCROLL && 
               child_entry.y+dc.device_origin_y < y_start*PIXELS_PER_SCROLL)) ||
            (entry.infoset.y+dc.device_origin_y < y_start*PIXELS_PER_SCROLL+height/zoom))
        {
            // draw the labels
            RenderLabels(dc, &child_entry, &entry);
            // Only draw the node line once for all children.
            if (child_entry.child_number == 1) 
            {  
                // draw the 'node' line
                bool hilight = 
                    (hilight_infoset  && (entry.n->GetInfoset() == hilight_infoset)) ||
                    (hilight_infoset1 && (entry.n->GetInfoset() == hilight_infoset1));
                ::DrawLine(dc, entry.x, entry.y,
                           entry.x + draw_settings.NodeLength() + 
                             entry.nums * INFOSET_SPACING, entry.y, 
                           entry.color, hilight ? 1 : 0);

                // show the infoset lines, if required by draw settings
                ::DrawCircle(dc, entry.x + entry.num * INFOSET_SPACING, entry.y, 
                             3, entry.color);

                // FIXME! When infosets are shown, branch lines (and infoset
                // lines) often disappear at different zoom levels and positions.

                if (draw_settings.ShowInfosets())
                {
                    if (entry.infoset.y != -1)
                    {
                        ::DrawThinLine(dc, 
                                       entry.x + entry.num * INFOSET_SPACING, 
                                       entry.y, 
                                       entry.x + entry.num * INFOSET_SPACING, 
                                       entry.infoset.y, 
                                       entry.color);
                    }

                    if (entry.infoset.x != -1)  
                    {
                        // Draw a little arrow in the direction of the iset.
                        if (entry.infoset.x > entry.x) // iset is to the right
                        {
                            ::DrawLine(dc, 
                                       entry.x+entry.num*INFOSET_SPACING, 
                                       entry.infoset.y, 
                                       entry.x+(entry.num+1)*INFOSET_SPACING, 
                                       entry.infoset.y, entry.color);
                        }
                        else // iset is to the left
                        {
                            ::DrawLine(dc, 
                                       entry.x + entry.num * INFOSET_SPACING, 
                                       entry.infoset.y, 
                                       entry.x + (entry.num - 1) * INFOSET_SPACING, 
                                       entry.infoset.y, 
                                       entry.color);
                        }
                    }
                }

                // Draw a triangle to show sugame roots
                if (entry.n->GetSubgameRoot() == entry.n)
                {
                    if (entry.expanded) 
                        DrawSmallSubgameIcon(dc, entry);
                }
                
            }
            if (child_entry.n == subgame_node)
                DrawSubgamePickIcon(dc, child_entry);
            // draw the 'branches'

            if (child_entry.n->GetParent() && child_entry.in_sup) // no branches for root node
            {  
                xs = entry.x+draw_settings.NodeLength()+entry.nums*INFOSET_SPACING;
                ys = entry.y;
                xe = xs+draw_settings.ForkLength();
                ye = child_entry.y;
                ::DrawLine(dc, xs, ys, xe, ye, entry.color);

                // Draw the highlight... y = a + bx = ys + (ye-ys) / (xe-xs) * x
                double prob = parent->ProbAsDouble(entry.n, child_entry.child_number);
                if (prob > 0)
                {
                    ::DrawLine(dc, xs, ys, (xs + draw_settings.ForkLength() * prob), 
                               (ys + (ye - ys) * prob), WX_COLOR_LIST_LENGTH - 1);
                }

                xs = xe;
                ys = ye;
                xe = child_entry.x;
                ye = ys;
                ::DrawLine(dc, xs, ye, xe, ye, entry.color);
            }
            else
            {
                xe = entry.x;
                ye = entry.y;
            }
           
            // Take care of terminal nodes (either real terminal or collapsed subgames)
            if (!child_entry.has_children)
            {
                ::DrawLine(dc, xe, ye, 
                           xe + draw_settings.NodeLength() + 
                             child_entry.nums * INFOSET_SPACING, 
                           ye, draw_settings.GetPlayerColor(-1));

                // Collapsed subgame: subgame icon is drawn at this terminal node.
                if ((child_entry.n->GetSubgameRoot() == child_entry.n) && 
                    !child_entry.expanded)
                    DrawLargeSubgameIcon(dc, child_entry, draw_settings.NodeLength());

                // Marked Node: a circle is drawn at this terminal node
                if (child_entry.n == mark_node)
                {
                    ::DrawCircle(dc, 
                                 xe + child_entry.nums * INFOSET_SPACING +
                                   draw_settings.NodeLength(), ye, 
                                 4, draw_settings.CursorColor());
                }
            }

            // Draw a circle to show the marked node
            if ((entry.n == mark_node) && 
                (child_entry.child_number == entry.n->NumChildren()))
            {
                ::DrawCircle(dc, entry.x + entry.nums * INFOSET_SPACING + 
                               draw_settings.NodeLength(), entry.y, 
                             4, draw_settings.CursorColor());
            }
        }
    }
}


void TreeRender::UpdateCursor(const NodeEntry *entry)
{
    if (entry->n->GetSubgameRoot() == entry->n && !entry->expanded)
    {
        flasher->SetFlashNode(entry->x + draw_settings.NodeLength() +
                                entry->nums*INFOSET_SPACING - SUBGAME_LARGE_ICON_SIZE,
                              entry->y,
                              entry->x + draw_settings.NodeLength() +
                                entry->nums*INFOSET_SPACING,
                              entry->y, subgameCursor);
    }
    else
    {
        flasher->SetFlashNode(entry->x + entry->nums*INFOSET_SPACING, entry->y,
                              entry->x + draw_settings.NodeLength() + 
                                entry->nums*INFOSET_SPACING - 8,
                              entry->y, nodeCursor);
    }

    flasher->Flash();
}

void TreeRender::Render(wxDC &dc)
{ 
    RenderSubtree(dc);
}

Bool TreeRender::JustRender(void) const 
{
    return TRUE;
}

void TreeRender::MakeFlasher(void)
{
    flasher = (draw_settings.FlashingCursor()) ? new TreeNodeFlasher(GetDC()) :
        new TreeNodeCursor(GetDC());
}

TreeRender::~TreeRender(void)
{
    if (flasher) delete flasher;
}

//----------------------------------------------------------------------
//                   TreeZoomWindow: Member functions
//----------------------------------------------------------------------

class TreeZoomFrame : public wxFrame {
private:
  TreeWindow *m_parent;

public:
  TreeZoomFrame(wxFrame *p_frame, TreeWindow *p_parent);
  
  Bool OnClose(void);
};

TreeZoomFrame::TreeZoomFrame(wxFrame *p_frame, TreeWindow *p_parent)
  : wxFrame(p_frame, "Zoom Window", -1, -1, 250, 250), m_parent(p_parent)
{ }

Bool TreeZoomFrame::OnClose(void)
{
  m_parent->delete_zoom_win();
  return TRUE;
}

TreeZoomWindow::TreeZoomWindow(wxFrame *frame, TreeWindow *parent,
                               const gList<NodeEntry *> &node_list_,
                               const Infoset * &hilight_infoset_,
                               const Infoset * &hilight_infoset1_,
                               const Node *&mark_node_,
                               const Node *&cursor_,
                               const Node *&subgame_node_,
                               const TreeDrawSettings &draw_settings_,
                               const NodeEntry *cursor_entry)
    : TreeRender(new TreeZoomFrame(frame, parent), parent,
                 node_list_, hilight_infoset_, hilight_infoset1_,
                 mark_node_, cursor_, subgame_node_, draw_settings_),
      m_parent(parent)
{
    MakeFlasher();
    UpdateCursor(cursor_entry);
    GetParent()->Show(TRUE);
#ifdef wx_x
    SetSize(250, 250);
#endif
}

//
// This Render function takes into account the current position of the cursor.
// Calculates ox and oy so that the cursor is located in the middle of the
// window.
//
void TreeZoomWindow::Render(wxDC &dc)
{
    int width, height;
    GetClientSize(&width, &height);
    int xm = (xs+xe)/2, ym = (ys+ye)/2;   // coordinates of the middle of the cursor
    int ox = width/2-xm;
    int oy = height/2-ym;
    dc.SetDeviceOrigin(0, 0); // should not be necessary, but its a bug
    Clear();
    dc.SetDeviceOrigin(ox, oy);
    TreeRender::Render(dc);
    flasher->Flash();
}

void TreeZoomWindow::UpdateCursor(const NodeEntry *entry)
{
    TreeRender::UpdateCursor(entry);
    flasher->GetFlashNode(xs, ys, xe, ye);
    Render(*GetDC());
}

void TreeZoomWindow::OnChar(wxKeyEvent &p_event)
{
  m_parent->OnChar(p_event);
}

//----------------------------------------------------------------------
//                      TreeWindow: Member classes
//----------------------------------------------------------------------

//-------------------
// Draggers
//-------------------

// Classes to take care of drag and drop features of the TreeWindow
// Note that all of these depend on the parent having a function GotObject
// that would return a node if the mouse drag activated the process that is
// provided by the class.

//--------------------
// NodeDragger
//--------------------

//
// This class enables the user to grab a node and drag it to a terminal node.
// When the mouse is released, a copy or move (if Control is down) action is
// performed.
//
class TreeWindow::NodeDragger
{
private:
    Efg &ef;
    wxBitmap *m_b, *c_b;
    wxMemoryDC *move_dc, *copy_dc;
    TreeWindow *parent;
    wxCanvasDC *dc;
    int drag_now;
    float x, y, ox, oy;  // position and old position
    int c, oc; // control pressed and old control pressed
    Node *start_node, *end_node;
    
    void RedrawObject(void);
    
public:
    NodeDragger(TreeWindow *parent, Efg &ef);
    ~NodeDragger();
    
    int OnEvent(wxMouseEvent &ev, Bool &nodes_changed);
    int ControlDown(void) const;
    int Dragging(void) const;
    
    Node *StartNode(void);
    Node *EndNode(void);
};


TreeWindow::NodeDragger::NodeDragger(TreeWindow *parent_, Efg &ef_)
    : ef(ef_), parent(parent_), dc(parent_->GetDC()), drag_now(0),
      start_node(0), end_node(0)
{
#include "bitmaps/copy.xpm"
#include "bitmaps/move.xpm"
    c_b = new wxBitmap(copy_xpm);
    m_b = new wxBitmap(move_xpm);
    copy_dc = new wxMemoryDC(dc);
    copy_dc->SelectObject(c_b);
    move_dc = new wxMemoryDC(dc);
    move_dc->SelectObject(m_b);
}

TreeWindow::NodeDragger::~NodeDragger()
{
    copy_dc->SelectObject(0);
    move_dc->SelectObject(0);
    delete c_b;
    delete m_b;
    delete move_dc;
    delete copy_dc;
}

void TreeWindow::NodeDragger::RedrawObject(void)
{
    static const int /*x_off = 0, */y_off = 21;
    if (ox >= 0) dc->Blit(ox, oy-y_off, 32, 32, (oc) ? move_dc : copy_dc, 0, 0, wxXOR);
    dc->Blit(x, y-y_off, 32, 32, (c) ? move_dc : copy_dc, 0, 0, wxXOR);
}

int TreeWindow::NodeDragger::OnEvent(wxMouseEvent &ev, Bool &nodes_changed)
{
    int ret = (drag_now) ? DRAG_CONTINUE : DRAG_NONE;
    if (ev.Dragging())
    {
        ox = x; oy = y; oc = c;
        ev.Position(&x, &y);
        c = ev.ControlDown();
        if (!drag_now)
        {
            start_node = parent->GotObject(x, y, DRAG_NODE_START);
            if (start_node)
            {
                int wx = (int)(x*parent->DrawSettings().Zoom());
                int wy = (int)(y*parent->DrawSettings().Zoom());
                parent->WarpPointer(wx, wy);
                ox = -1; oc = 0; c = 0; drag_now = 1; ret = DRAG_START;
            }
        }
        if (drag_now)
        {
            RedrawObject();
            if (ret != DRAG_START) ret = DRAG_CONTINUE;
        }
    }
    if (ev.LeftUp() && drag_now)
    {
        ox = -1; drag_now = 0;
        RedrawObject();
        end_node = parent->GotObject(x, y, DRAG_NODE_END);
        ev.Position(&x, &y); c = ev.ControlDown();
        ret = DRAG_STOP;
        if (start_node && end_node && start_node != end_node) {
      try {
        if (c)
          ef.MoveTree(start_node, end_node);    // move
        else
          ef.CopyTree(start_node, end_node);    // copy
        nodes_changed = TRUE;
      }
      catch (gException &E) {
        guiExceptionDialog(E.Description(), parent->Parent());
      }
      parent->OnPaint();
        }
    }
    return ret;
}

int TreeWindow::NodeDragger::ControlDown(void) const
{
    return c;
}

int TreeWindow::NodeDragger::Dragging(void) const
{
    return drag_now;
}

Node *TreeWindow::NodeDragger::StartNode(void)
{
    return start_node;
}

Node *TreeWindow::NodeDragger::EndNode(void)
{
    return end_node;
}



//--------------------
// IsetDragger
//--------------------

//
// This class enables the user to merge infosets by dragging a line from the
// first infoset's maker to the second one's.
//

class TreeWindow::IsetDragger
{
private:
    Efg &ef;
    TreeWindow *parent;
    wxCanvasDC *dc;
    int drag_now;
    float x, y, ox, oy, sx, sy;    // current, previous, start positions
    Node *start_node, *end_node;
    
    void RedrawObject(void);
    
public:
    IsetDragger(TreeWindow *parent, Efg &ef);
    ~IsetDragger();
    
    int Dragging(void) const;
    int OnEvent(wxMouseEvent &ev, Bool &infosets_changed);
    
    Node *StartNode(void);
    Node *EndNode(void);
};

TreeWindow::IsetDragger::IsetDragger(TreeWindow *parent_, Efg &ef_)
    : ef(ef_), parent(parent_), dc(parent_->GetDC()), drag_now(0),
      start_node(0), end_node(0)
{ }

TreeWindow::IsetDragger::~IsetDragger()
{ }

void TreeWindow::IsetDragger::RedrawObject(void)
{
    dc->SetLogicalFunction(wxXOR);
    if (ox > 0) dc->DrawLine(sx, sy, ox, oy);
    dc->DrawLine(sx, sy, x, y);
    dc->SetLogicalFunction(wxCOPY);
}

// Event Handler
int TreeWindow::IsetDragger::OnEvent(wxMouseEvent &ev, Bool &infosets_changed)
{
    int ret = (drag_now) ? DRAG_CONTINUE : DRAG_NONE; ;
    if (ev.Dragging())
    {
        ox = x; oy = y;
        ev.Position(&x, &y);
        if (!drag_now)
        {
            start_node = parent->GotObject(x, y, DRAG_ISET_START);
            if (start_node)
            {
                int wx = (int)(x*parent->DrawSettings().Zoom());
                int wy = (int)(y*parent->DrawSettings().Zoom());
                parent->WarpPointer(wx, wy);
                sx = x; sy = y; ox = -1; drag_now = 1; ret = DRAG_START;
            }
        }
        if (drag_now)
        {
            RedrawObject();
            if (ret != DRAG_START) ret = DRAG_CONTINUE;
        }
    }
    if (ev.LeftUp() && drag_now)
    {
        ox = -1; drag_now = 0;
        RedrawObject();
        end_node = parent->GotObject(x, y, DRAG_ISET_END);
        ev.Position(&x, &y);
        ret = DRAG_STOP;
        if (start_node && end_node && start_node != end_node)
        {
            Infoset *to = start_node->GetInfoset();
            Infoset *from = end_node->GetInfoset();
            if (to && from)
            {
                if (to->GetPlayer() == from->GetPlayer())
                {
                    gText iset_name = from->GetName();
                    Infoset *miset = ef.MergeInfoset(to, from);
                    miset->SetName(iset_name+":1");
                    infosets_changed = TRUE;
                    parent->OnPaint();
                }
            }
        }
    }
    return ret;
}

int TreeWindow::IsetDragger::Dragging(void) const
{
    return drag_now;
}

Node *TreeWindow::IsetDragger::StartNode(void)
{
    return start_node;
}
Node *TreeWindow::IsetDragger::EndNode(void)
{
    return end_node;
}


//--------------------
// BranchDragger
//--------------------

//
// This class allows the user to add branches to a node/infoset by dragging a
// line from the base of the branches to the position the new branch is to
// occupy.  If the node was terminal, a player dialog is opened to 
// choose a player.
//

//
// A dialog box to select the player
//
class BranchDraggerDialog : public wxDialogBox {
private:
  Efg &m_efg;
  int m_playerSelected, m_completed;
  wxListBox *m_playerNameList;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((BranchDraggerDialog *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((BranchDraggerDialog *) p_object.GetClientData())->OnCancel(); }
  static void CallbackHelp(wxButton &, wxEvent &)
    { wxHelpContents(EFG_TREE_HELP); }

  void OnOK(void);
  void OnCancel(void);

public:
  BranchDraggerDialog(Efg &, wxWindow * = 0);
  virtual ~BranchDraggerDialog();

  int Completed(void) const { return m_completed; }
  EFPlayer *GetPlayer(void);
};

BranchDraggerDialog::BranchDraggerDialog(Efg &p_efg, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Select Player", TRUE), m_efg(p_efg)
{
  m_playerNameList = new wxListBox(this, 0, "Player");
  m_playerNameList->Append("Chance");

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerNameList->Append(m_efg.Players()[pl]->GetName());
  }

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);
  wxButton *helpButton = new wxButton(this, (wxFunction) CallbackHelp,
				      "Help");
  helpButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

BranchDraggerDialog::~BranchDraggerDialog()
{ }

void BranchDraggerDialog::OnOK(void)
{
  m_playerSelected = m_playerNameList->GetSelection();
  m_completed = wxOK;
  Show(FALSE);
}

void BranchDraggerDialog::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

EFPlayer *BranchDraggerDialog::GetPlayer(void)
{
  if (m_playerSelected == 0)
    return m_efg.GetChance();
  else
    return m_efg.Players()[m_playerSelected];
}


class TreeWindow::BranchDragger
{
private:
    Efg &ef;
    TreeWindow *parent;
    wxCanvasDC *dc;
    int drag_now;
    int br;
    float x, y, ox, oy, sx, sy;    // current, previous, start positions
    Node *start_node;
    
    void RedrawObject(void);
    
public:
    BranchDragger(TreeWindow *parent, Efg &ef);
    ~BranchDragger();
    
    int Dragging(void) const;
    int OnEvent(wxMouseEvent &ev, Bool &infosets_changed);
    
    Node *StartNode(void);
    int BranchNum(void);
};


TreeWindow::BranchDragger::BranchDragger(TreeWindow *parent_, Efg &ef_)
    : ef(ef_), parent(parent_), dc(parent_->GetDC()), drag_now(0),
      br(0), start_node(0)
{ }

TreeWindow::BranchDragger::~BranchDragger()
{ }

void TreeWindow::BranchDragger::RedrawObject(void)
{
    dc->SetLogicalFunction(wxXOR);
    if (ox > 0) dc->DrawLine(sx, sy, ox, oy);
    dc->DrawLine(sx, sy, x, y);
    dc->SetLogicalFunction(wxCOPY);
}


int TreeWindow::BranchDragger::OnEvent(wxMouseEvent &ev,
                                       Bool &infosets_changed)
{
    int ret = (drag_now) ? DRAG_CONTINUE : DRAG_NONE;
    if (ev.Dragging())
    {
        ox = x; oy = y;
        ev.Position(&x, &y);
        if (!drag_now)
        {
            
            start_node = parent->GotObject(x, y, DRAG_BRANCH_START);
            if (start_node)
            {
                int wx = (int)(x*parent->DrawSettings().Zoom());
                int wy = (int)(y*parent->DrawSettings().Zoom());
                parent->WarpPointer(wx, wy);
                sx = x; sy = y; ox = -1; drag_now = 1; ret = DRAG_START;
            }
        }
        if (drag_now)
        {
            RedrawObject();
            if (ret != DRAG_START) ret = DRAG_CONTINUE;
        }
    }
    if (ev.LeftUp() && drag_now)
    {
        ox = -1; drag_now = 0;
        RedrawObject();
        if (parent->GotObject(x, y, DRAG_BRANCH_END))
            br = (int)(x+0.5); // round x to an integer -- branch # is passed back this way
        else
            br = 0;
        ev.Position(&x, &y);
        ret = DRAG_STOP;
        if (start_node && br)
        {
            Infoset *iset = start_node->GetInfoset();
            if (iset)
            {
                if (br > iset->NumActions())
                    ef.InsertAction(iset);
                else
                    ef.InsertAction(iset, iset->Actions()[br]);
            }
            else
            {
                BranchDraggerDialog dialog(ef, parent->GetParent());
                EFPlayer *player = dialog.GetPlayer();
                if (player) ef.AppendNode(start_node, player, 1);
            }
            infosets_changed = TRUE;
            parent->OnPaint();
        }
    }
    return ret;
}

int TreeWindow::BranchDragger::Dragging(void) const
{
    return drag_now;
}

Node *TreeWindow::BranchDragger::StartNode(void)
{
    return start_node;
}

int TreeWindow::BranchDragger::BranchNum(void)
{
    return br;
}



//--------------------
// OutcomeDragger
//--------------------

class TreeWindow::OutcomeDragger
{
private:
    Efg &ef;
    TreeWindow *parent;
    int drag_now;
    EFOutcome *outcome;
    Node *start_node;
    float x, y;
    wxCursor *outcome_cursor;
    
public:
    OutcomeDragger(TreeWindow *parent, Efg &ef);
    ~OutcomeDragger();
    
    int Dragging(void) const;
    int OnEvent(wxMouseEvent &ev, Bool &outcomes_changed);
};

TreeWindow::OutcomeDragger::OutcomeDragger(TreeWindow *parent_, Efg &ef_)
    : ef(ef_), parent(parent_), drag_now(0), outcome(0),
      outcome_cursor(new wxCursor("OUTCOMECUR"))
{ }

TreeWindow::OutcomeDragger::~OutcomeDragger()
{ }

int TreeWindow::OutcomeDragger::OnEvent(wxMouseEvent &ev,
                                        Bool &outcomes_changed)
{
    int ret = (drag_now) ? DRAG_CONTINUE : DRAG_NONE;
    if (ev.Dragging())
    {
        if (!drag_now)
        {
            ev.Position(&x, &y); outcome = 0;
            start_node = parent->GotObject(x, y, DRAG_OUTCOME_START);
            if (start_node)
            {
                outcome = start_node->GetOutcome();
                if (outcome)
                {
                    parent->SetCursor(outcome_cursor);
                    drag_now = 1; ret = DRAG_START;
                }
            }
        }
    }
    else if (drag_now)
    {
        parent->SetCursor(wxSTANDARD_CURSOR);
        ev.Position(&x, &y);
        Bool c = ev.ControlDown();
        ret = DRAG_STOP;
        Node *end_node = parent->GotObject(x, y, DRAG_OUTCOME_END);
        if (end_node)
        {
            end_node->SetOutcome(outcome);
            if (c) start_node->SetOutcome(0); // move
            outcomes_changed = 1;
            parent->OnPaint();
        }
        drag_now = 0;
    }
    return ret;
}

int TreeWindow::OutcomeDragger::Dragging(void) const
{
    return drag_now;
}


//----------------------------------------------------------------------
//                      TreeWindow: Member functions
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//                  TreeWindow: Constructor and destructor
//----------------------------------------------------------------------

TreeWindow::TreeWindow(Efg &ef_, EFSupport * &disp, EfgShow *frame_) 
    : TreeRender(frame_, this, node_list, hilight_infoset, hilight_infoset1,
                 mark_node, cursor, subgame_node, draw_settings),
      ef(ef_), disp_sup(disp), frame(frame_), pframe(frame_)
{
    // Set the cursor to the root node
    cursor = ef.RootNode();
    // Make sure that Chance player has a name
    ef.GetChance()->SetName("Chance");
    // Add the first subgame -- root subgame
    subgame_list.Append(SubgameEntry(ef.RootNode()));
    // Create the flasher to flash the cursor or just a steady cursor
    MakeFlasher();
    // Create provision for drag'n dropping nodes
    node_drag = new NodeDragger(this, ef);
    // Create provision for merging isets by drag'n dropping
    iset_drag = new IsetDragger(this, ef);
    // Create provision for adding/creating braches by drag'n dropping
    branch_drag = new BranchDragger(this, ef);
    // Create provision for copying/moving outcomes by drag'n dropping
    outcome_drag = new OutcomeDragger(this, ef);
    // No node has been marked yet--mark_node is invalid
    mark_node = 0; old_mark_node = 0;
    // No isets are being hilighted
    hilight_infoset = 0; hilight_infoset1 = 0;
    // No zoom window or outcome dialog
    zoom_window = 0;
    outcome_font = wxTheFontList->FindOrCreateFont(9, wxSWISS, wxNORMAL, wxNORMAL);
    white_brush = wxTheBrushList->FindOrCreateBrush("WHITE", wxSOLID);
    
#ifdef wx_msw
    scissor_cursor = new wxCursor("SCISSORCUR");
#else
#include "bitmaps/scissor.xbm"
#ifndef LINUX_WXXT
    scissor_cursor = new wxCursor(scissor_bits, scissor_width, scissor_height,
                                  -1, -1, scissor_bits);
#else
    // wxxt uses an older constructor
    scissor_cursor = new wxCursor(scissor_bits, scissor_width, scissor_height,
                                  scissor_width/2, scissor_height/2);
#endif // LINUX_WXXT
#endif // wx_msw
    
    GetDC()->SetBackgroundMode(wxTRANSPARENT);
    AllowDoubleClick(TRUE);
    // Make sure the node_list gets recalculated the first time
    nodes_changed = TRUE;
    infosets_changed = TRUE;
    outcomes_changed = FALSE;
    must_recalc = FALSE;
    log = FALSE;

    // Create scrollbars
    SetScrollbars(PIXELS_PER_SCROLL, PIXELS_PER_SCROLL, 60, 60, 4, 4);
    draw_settings.set_x_steps(60);
    draw_settings.set_y_steps(60);
    
    // Create a popup menu
    MakeMenus();
}

TreeWindow::~TreeWindow(void)
{
    delete node_drag;
    delete iset_drag;
    delete branch_drag;
    delete outcome_drag;
    Show(FALSE);
}

void TreeWindow::MakeMenus(void)
{
  edit_menu = new wxMenu(NULL, (wxFunction)OnPopup);

  wxMenu *node_menu = new wxMenu;
  node_menu->Append(NODE_ADD, "&Add Move", "Add a move");
  node_menu->Append(NODE_DELETE, "&Delete Move", "Remove move at cursor");
  node_menu->Append(NODE_INSERT, "&Insert Move", "Insert move at cursor");
  node_menu->Append(NODE_LABEL, "&Label", "Label cursor node");
  node_menu->AppendSeparator();
  node_menu->Append(NODE_SET_MARK, "Set &Mark", "Mark cursor node");
  node_menu->Append(NODE_GOTO_MARK, "Go&to Mark", "Goto marked node");
    
  wxMenu *action_menu = new wxMenu;
  action_menu->Append(ACTION_DELETE, "&Delete", "Delete an action from cursor iset");
  action_menu->Append(ACTION_INSERT, "&Insert", "Delete an action to cursor iset");
  action_menu->Append(ACTION_LABEL, "&Label");
  action_menu->Append(ACTION_PROBS, "&Probs", "Set the chance player probs");
    
  wxMenu *infoset_menu = new wxMenu;
  infoset_menu->Append(INFOSET_MERGE, "&Merge", "Merge cursor iset w/ marked");
  infoset_menu->Append(INFOSET_BREAK, "&Break", "Make cursor a new iset");
  infoset_menu->Append(INFOSET_SPLIT, "&Split", "Split iset at cursor");
  infoset_menu->Append(INFOSET_JOIN, "&Join", "Join cursor to marked iset");
  infoset_menu->Append(INFOSET_LABEL, "&Label", "Label cursor iset & actions");
  infoset_menu->Append(INFOSET_SWITCH_PLAYER, "&Player", "Change player of cursor iset");
  infoset_menu->Append(INFOSET_REVEAL, "&Reveal", "Reveal infoset to players");

  wxMenu *outcome_menu = new wxMenu;
  outcome_menu->Append(TREE_OUTCOMES_NEW, "&New",
		       "Create a new outcome");
  outcome_menu->Append(TREE_OUTCOMES_DELETE, "Dele&te",
		       "Delete an outcome");
  outcome_menu->Append(TREE_OUTCOMES_ATTACH, "&Attach",
		       "Attach an outcome to the node at cursor");
  outcome_menu->Append(TREE_OUTCOMES_DETACH, "&Detach",
		       "Detach the outcome from the node at cursor");
  outcome_menu->Append(TREE_OUTCOMES_LABEL, "&Label",
		       "Label the outcome at the node at cursor");
  outcome_menu->Append(TREE_OUTCOMES_PAYOFFS, "&Payoffs",
		       "Set the payoffs for the outcome at the cursor");
  
  wxMenu *tree_menu = new wxMenu;
  tree_menu->Append(TREE_COPY, "&Copy", "Copy tree from marked node");
  tree_menu->Append(TREE_MOVE, "&Move", "Move tree from marked node");
  tree_menu->Append(TREE_DELETE, "&Delete", "Delete recursively from cursor");
  tree_menu->Append(TREE_LABEL, "&Label", "Set the game label");
  tree_menu->Append(TREE_PLAYERS, "&Players", "Edit/View player names");
  tree_menu->Append(TREE_INFOSETS, "&Infosets", "Edit/View infosets");
    
  edit_menu->Append(EDIT_NODE, "&Node", node_menu, "Edit the node");
  edit_menu->Append(EDIT_ACTIONS, "&Actions", action_menu, "Edit actions");
  edit_menu->Append(EDIT_INFOSET, "&Infoset", infoset_menu, "Edit infosets");
  edit_menu->Append(TREE_OUTCOMES, "&Outcomes", outcome_menu,
		    "Edit outcomes and payoffs");
  edit_menu->Append(EDIT_TREE, "&Tree", tree_menu, "Edit the tree");
  edit_menu->SetClientData((char *)frame); // call back to parent later
}


gText TreeWindow::Title(void) const
{
    return ef.GetTitle();
}

Bool TreeWindow::JustRender(void) const 
{
    return FALSE;
}

gText TreeWindow::AsString(TypedSolnValues what, const Node *n, int br) const
{
    return frame->AsString(what, n, br);
}

double TreeWindow::ProbAsDouble(const Node *n, int action) const
{
    return (double)frame->BranchProb(n, action);
}

gText TreeWindow::OutcomeAsString(const Node *n, bool &/*hilight*/) const
{
    if (n->GetOutcome())
    {
        EFOutcome *tv = n->GetOutcome();
        const gArray<gNumber> &v = ef.Payoff(tv);
        gText tmp = "(";

        for (int i = v.First(); i <= v.Last(); i++)
        {
            if (i != 1) 
                tmp += ",";

            if (draw_settings.ColorCodedOutcomes())
                tmp += ("\\C{"+ToText(draw_settings.GetPlayerColor(i))+"}");

            tmp += ToText(v[i]);
        }

        if (draw_settings.ColorCodedOutcomes()) 
            tmp += ("\\C{"+ToText(WX_COLOR_LIST_LENGTH-1)+"}");

        tmp += ")";
        
        return tmp;
    }
    else
        return "";
}

//---------------------------------------------------------------------
//                  TreeWindow: Event-hook members
//---------------------------------------------------------------------

static Node *PriorSameLevel(NodeEntry *e, const gList<NodeEntry *> &node_list)
{
    int e_n = node_list.Find(e);
    assert(e_n && "Node not found");
    for (int i = e_n-1; i >= 1; i--)
        if (node_list[i]->level == e->level)
            return (Node *) node_list[i]->n;
    return 0;
}

static Node *NextSameLevel(NodeEntry *e, const gList<NodeEntry *> &node_list)
{
    int e_n = node_list.Find(e);
    assert(e_n && "Node not found");
    for (int i = e_n+1; i <= node_list.Length(); i++)
        if (node_list[i]->level == e->level) 
            return (Node *) node_list[i]->n;
    return 0;
}

static Node *PriorSameIset(const Node *n)
{
    Infoset *iset = n->GetInfoset();
    if (!iset) return 0;
    for (int i = 1; i <= iset->NumMembers(); i++)
        if (iset->Members()[i] == n)
            if (i > 1)
                return iset->Members()[i-1];
            else 
                return 0;
    return 0;
}

static Node *NextSameIset(const Node *n)
{
    Infoset *iset = n->GetInfoset();
    if (!iset) return 0;
    for (int i = 1; i <= iset->NumMembers(); i++)
        if (iset->Members()[i] == n)
            if (i < iset->NumMembers()) 
                return iset->Members()[i+1]; 
            else
                return 0;
    return 0;
}

//
// OnChar -- handle keypress events
// Currently we support the following keys:
//     left arrow:   go to parent of current node
//     right arrow:  go to first child of current node
//     up arrow:     go to previous sibling of current node
//     down arrow:   go to next sibling of current node
// Since the addition of collapsible subgames, a node's parent may not
// be visible in the current display.  Thus, find the first predecessor
// that is visible (ROOT is always visible)
//
void TreeWindow::OnChar(wxKeyEvent& ch)
{
    //
    // Accelerators:
    // Note that accelerators are provided for in the wxwin code but only for the
    // windows platform.  In order to make this more portable, accelerators for
    // this program are coded in the header file and processed in OnChar
    //
    
    frame->CheckAccelerators(ch);
    
    if (ch.ShiftDown() == FALSE)
    {
        bool c = false;   // set to true if cursor position has changed
        switch (ch.KeyCode())
        {
        case WXK_LEFT:
            if (cursor->GetParent())
            {
                cursor = (Node *) GetValidParent(cursor)->n;
                c = true;
            }
            break;
        case WXK_RIGHT:
            if (GetValidChild(cursor))
            {
                cursor = (Node *) GetValidChild(cursor)->n;
                c = true;
            }
            break;
        case WXK_UP:
        {
            Node *prior = (!ch.ControlDown()) ? 
                PriorSameLevel(GetNodeEntry(cursor), node_list) : PriorSameIset(cursor);
            if (prior)
            {
                cursor = prior;
                c = true;
            }
            break;
        }
        case WXK_DOWN:
        {
            Node *next = (!ch.ControlDown()) ?
                NextSameLevel(GetNodeEntry(cursor), node_list) : NextSameIset(cursor);
            if (next)
            {
                cursor = next;
                c = true;
            }
            break;
        }
        case WXK_SPACE:
            c = true;
            break;
        }
        
        if (c) 
            ProcessCursor(); // cursor moved

        // Implement the behavior that when control+cursor key is pressed, the
        // nodes belonging to the iset are hilighted.
        if (c && ch.ControlDown())
        {
            if (hilight_infoset1 != cursor->GetInfoset())
            {
                hilight_infoset1 = cursor->GetInfoset();
                OnPaint();
            }
        }
        if (!ch.ControlDown() && hilight_infoset1)
        {
            hilight_infoset1 = 0;
            OnPaint();
        }
    }
    else
        wxCanvas::OnChar(ch);
}

//
// OnEvent -- handle mouse events
// Currently we support selecting a node by clicking on it
//
Action *LastAction(Node *node);

void TreeWindow::OnEvent(wxMouseEvent& ev)
{
    // Check all the draggers.  Note that they are mutually exclusive
    if (!iset_drag->Dragging() && !branch_drag->Dragging() 
        && !outcome_drag->Dragging())
        if (node_drag->OnEvent(ev, nodes_changed) != DRAG_NONE) return;
    
    if (!node_drag->Dragging() && !branch_drag->Dragging() &&
        !outcome_drag->Dragging())
        if (iset_drag->OnEvent(ev, infosets_changed) != DRAG_NONE) return;
    
    if (!node_drag->Dragging() && !iset_drag->Dragging() &&
        !outcome_drag->Dragging())
        if (branch_drag->OnEvent(ev, infosets_changed) != DRAG_NONE) return;
    
    if (!node_drag->Dragging() && !iset_drag->Dragging() &&
        !branch_drag->Dragging())
        if (outcome_drag->OnEvent(ev, outcomes_changed) != DRAG_NONE) return;
    
    // Implements the 'cutting' behavior
    if (ProcessShift(ev)) return;
    
    // Double clicking hilights iset and toggles subgames
    if (ev.LeftDClick() || (ev.LeftDown() && ev.ControlDown()))
        ProcessDClick(ev);
    
    // Clicking on a node will move the cursor there
    if (ev.LeftDown())
    {
        Node *old_cursor = cursor;
        ProcessClick(ev);
        if (cursor != old_cursor) 
            ProcessCursor();
        SetFocus(); // click on the canvas to restore keyboard focus
    }
    
    // Right click implements a popup menu (edit), legend display
    if (ev.RightDown()) ProcessRClick(ev);
    
    // Right double click implements legend modification
    if (ev.RightDClick() || (ev.RightDown() && ev.ControlDown()))
        ProcessRDClick(ev);
}

void TreeWindow::OnPaint(void)
{
    TreeRender::OnPaint();
    if (zoom_window) 
        zoom_window->OnPaint();
    AdjustScrollbarSteps();
}

//---------------------------------------------------------------------
//                   TreeWindow: Drawing functions
//---------------------------------------------------------------------

// these global variables are convinient to use in the recursive rendering
// of course, they will cause problems if parallel rendering ever occurs 
int maxlev, maxy, miny, ycoord;

int TreeWindow::PlayerNum(const EFPlayer *p) const
{
    if (p->IsChance()) return 0;
    for (int i = 1; i <= ef.NumPlayers(); i++)
        if (ef.Players()[i] == p) return i;
    assert(0 && "Player not found");
    return -1;
}

int TreeWindow::IsetNum(const Infoset *s) const
{
    for (int i = 1; i <= s->GetPlayer()->NumInfosets(); i++)
        if (s->GetPlayer()->Infosets()[i] == s) return i;
    return 0;
}

NodeEntry *TreeWindow::GetNodeEntry(const Node *n)
{
    for (int i = 1; i <= node_list.Length(); i++)
        if (node_list[i]->n == n) return node_list[i];
    return 0;
}

TreeWindow::SubgameEntry &TreeWindow::GetSubgameEntry(const Node *n)
{
    for (int i = 1; i <= subgame_list.Length(); i++)
        if (subgame_list[i].root == n) return subgame_list[i];
    return subgame_list[1]; // root subgame
}


// Added support for EFSupports.  We can now display in two modes: either all
// the nodes reachable from the root node that are in this support, or, all of
// the nodes that are in this support, with no connections if the connections
// are not in the support.
int TreeWindow::FillTable(const Node *n, int level)
{
    int y1 = -1, yn=0;
    
    SubgameEntry &subgame_entry = GetSubgameEntry(n->GetSubgameRoot());
    
    NodeEntry *entry = new NodeEntry;
    entry->n = n;   // store the node the entry is for
    node_list += entry;
    entry->in_sup = true;
    if (n->NumChildren()>0 && subgame_entry.expanded)
    {
        for (int i = 1; i <= n->NumChildren(); i++)
        {
            bool in_sup = true;
            if (PlayerNum(n->GetPlayer()))        // pn == 0 for chance nodes
                in_sup = disp_sup->Find(n->GetInfoset()->Actions()[i]);
            if (in_sup)
            {
                yn = FillTable(n->GetChild(i), level+1);
                if (y1 == -1)  y1 = yn;
            }
            else   // not in the support.
            {
                if (!draw_settings.RootReachable())  // show only nodes reachable from root
                {
                    // still consider this node, but mark it as invisible
                    yn = FillTable(n->GetChild(i), level+1);
                    if (y1 == -1)  y1 = yn;
                    GetNodeEntry(n->GetChild(i))->in_sup = false;
                }
            }
        }
        entry->y = (y1 + yn) / 2;
    }
    else
    {
        entry->y = ycoord;
        ycoord += draw_settings.YSpacing();
    }
    
    entry->level = level;
    entry->has_children = n->NumChildren();
    // Find out what branch of the parent this node is on
    if (n == ef.RootNode())
        entry->child_number = 0;
    else
    {
        Node *parent = n->GetParent();
        for (int i = 1; i <= parent->NumChildren(); i++)
            if (parent->GetChild(i) == n)
                entry->child_number = i;
    }
    
    entry->infoset.y = -1;
    entry->infoset.x = -1;
    entry->num = 0;
    entry->nums = 0;
    entry->x = level * 
        (draw_settings.NodeLength() + draw_settings.BranchLength() +
         draw_settings.ForkLength());
    if (n->GetPlayer())
        entry->color = draw_settings.GetPlayerColor(n->GetPlayer()->GetNumber());
    else
        entry->color = draw_settings.GetPlayerColor(-1);
    
    entry->expanded = subgame_entry.expanded;
    
    maxlev = gmax(level, maxlev);
    maxy = gmax(entry->y, maxy);
    miny = gmin(entry->y, miny);
    
    return entry->y;
}

//
// CheckInfosetEntry.  Checks how many infoset lines are to be drawn at each
// level, spaces them by setting each infoset's node's num to the previous
// infoset node+1.  Also lengthens the nodes by the amount of space taken up
// by the infoset lines.
//
void TreeWindow::CheckInfosetEntry(NodeEntry *e)
{
    int pos;
    NodeEntry *infoset_entry, *e1;
    // Check if the infoset this entry belongs to (on this level) has already
    // been processed.  If so, make this entry->num the same as the one already
    // processed and return
    infoset_entry = NextInfoset(e);
    for (pos = 1; pos <= node_list.Length(); pos++)
    {
        e1 = node_list[pos];
        // if the infosets are the same and they are on the same level and e1 has been processed
        if (e->n->GetInfoset() == e1->n->GetInfoset() && 
            e->level == e1->level && e1->num)
        {
            e->num = e1->num;
            if (infoset_entry)
            {
                e->infoset.y = infoset_entry->y;
                if (draw_settings.ShowInfosets() == SHOWISET_ALL)
                    e->infoset.x = infoset_entry->x;
            }
            return;
        }
    }
    
    // If we got here, this entry does not belong to any processed infoset yet.
    // Check if it belongs to ANY infoset, if not just return
    if (!infoset_entry) return;
    
    // If we got here, then this entry is new and is connected to other entries
    // find the entry on the same level with the maximum num.
    // This entry will have num = num+1.
    int num = 0;
    for (pos = 1; pos <= node_list.Length(); pos++)
    {
        e1 = node_list[pos];
        // Find the max num for this level
        if (e->level == e1->level) num = gmax(e1->num, num);
    }
    num++;
    e->num = num;
    e->infoset.y = infoset_entry->y;
    if (draw_settings.ShowInfosets() == SHOWISET_ALL) 
        e->infoset.x = infoset_entry->x;
}

//
// Checks if there are any nodes in the same infoset as e that are either
// on the same level (if SHOWISET_SAME) or on any level (if SHOWISET_ALL)
//
NodeEntry *TreeWindow::NextInfoset(const NodeEntry * const e)
{
    NodeEntry *e1;
    
    for (int pos = node_list.Find((NodeEntry * const)e)+1;
         pos <= node_list.Length(); pos++)
    {
        e1 = node_list[pos];
        // infosets are the same and the nodes are on the same level
        if (e->n->GetInfoset() == e1->n->GetInfoset())  
            if (draw_settings.ShowInfosets() == SHOWISET_ALL)
                return e1;
            else if (e->level == e1->level) return e1;
    }
    return 0;
}

void TreeWindow::FillInfosetTable(const Node *n)
{
    NodeEntry *entry = GetNodeEntry(n);
    if (n->NumChildren()>0)
        for (int i = 1; i <= n->NumChildren(); i++)
        {
            bool in_sup = true;
            if (PlayerNum(n->GetPlayer()))        // pn == 0 for chance nodes
                in_sup = disp_sup->Find(n->GetInfoset()->Actions()[i]);
            
            if (in_sup || !draw_settings.RootReachable())
                FillInfosetTable(n->GetChild(i));
        }
    if (entry) CheckInfosetEntry(entry);
}

void TreeWindow::UpdateTableInfosets(void)
{
    // Note that levels are numbered from 0, not 1.
    // create an array to hold max num for each level
    gArray<int> nums(0, maxlev+1); 
    
    for (int i = 0; i <= maxlev + 1; nums[i++] = 0);
    NodeEntry *e;
    // find the max e->num for each level
    for (int pos = 1; pos <= node_list.Length(); pos++)
    {
        e = node_list[pos];
        nums[e->level] = gmax(e->num+1, nums[e->level]);
    }
    
    // record the max e->num for each level for each node
    for (int pos = 1; pos <= node_list.Length(); pos++)
    {
        e = node_list[pos];
        e->nums = nums[e->level];
    }
    
    for (int i = 0; i <= maxlev; i++)  nums[i+1] += nums[i];
    
    // now add the needed length to each level
    for (int pos = 1; pos <= node_list.Length(); pos++)
    {
        e = node_list[pos];
        if (e->level != 0) 
            e->x += nums[e->level-1]*INFOSET_SPACING;
    }
}

NodeEntry *TreeWindow::GetValidParent(const Node *e)
{
    assert(e && "Parent not found");
    NodeEntry *n = GetNodeEntry(e->GetParent());
    if (n) 
        return n;
    else 
        return GetValidParent(e->GetParent());
}

NodeEntry *TreeWindow::GetValidChild(const Node *e)
{
    for (int i = 1; i <= e->NumChildren(); i++)
    {
        NodeEntry *n = GetNodeEntry(e->GetChild(i));
        if (n)
            return n;
        else
        {
            n = GetValidChild(e->GetChild(i));
            if (n) return n;
        }
    }
    return 0;
}

void TreeWindow::UpdateTableParents(void)
{
    NodeEntry *e;
    for (int pos = 1; pos <= node_list.Length(); pos++)
    {
        e = node_list[pos];
        e->parent = (e->n == ef.RootNode()) ? e : GetValidParent(e->n);
        if (!GetValidChild(e->n)) e->has_children = 0;
    }
}


//
// Render: The main rendering routine
//

void TreeWindow::Render(wxDC &dc)
{
    int width, height, x_start, y_start;
    if (nodes_changed || infosets_changed || must_recalc)
    {
        // Recalculate only if needed.
        // Note that node_table is preserved until the next recalculation.
        node_list.Flush();

        // If we modify the structure of the game, revert back to the full support
        // for the time being.  Otherwise, we run into weird problems.
        if (nodes_changed || infosets_changed) 
            frame->GameChanged();
        
        maxlev = miny = maxy = 0;
        ViewStart(&x_start, &y_start);
        GetClientSize(&width, &height);
        ycoord = TOP_MARGIN;
        FillTable(ef.RootNode(), 0);

        if (draw_settings.ShowInfosets())
        {
            // FIXME! This causes lines to disappear... sometimes.
            FillInfosetTable(ef.RootNode());
            UpdateTableInfosets();
        }

        UpdateTableParents();
        draw_settings.SetMaxX((maxlev + 1) * 
                              (draw_settings.BranchLength() + 
                               draw_settings.ForkLength() + 
                               draw_settings.NodeLength()) + 
                              draw_settings.OutcomeLength());
        draw_settings.SetMaxY(maxy+25);

        if (must_recalc)
        {
            must_recalc = FALSE;
            need_clear = TRUE;
        }
    }

    if (nodes_changed || infosets_changed || outcomes_changed)
    {
        frame->RemoveSolutions();
        nodes_changed = FALSE;
        infosets_changed = FALSE;
        outcomes_changed = FALSE;
        need_clear = TRUE;
    }

    char *dc_type = dc.GetClassInfo()->GetClassName();
    
    if (strcmp(dc_type, "wxCanvasDC") == 0)  // if drawing to screen
    {
        if (cursor)
        {
            NodeEntry *entry = GetNodeEntry(cursor);

            if (!entry)
            {
                cursor = ef.RootNode(); entry = GetNodeEntry(cursor);
            }

            UpdateCursor(entry);
        }
        
        if (need_clear)
        {
            dc.SetBrush(wxWHITE_BRUSH);
#ifdef wx_x  // a bug in wxwin/motif prevents Clear from working correctly.
            dc.DrawRectangle(0, 0, 10000, 10000);
#else
            dc.Clear();
#endif
        }
        dc.BeginDrawing();
    }
    else
    {
        flasher->SetFlashing(FALSE);
    }
    
    TreeRender::Render(dc);

    if (strcmp(dc_type, "wxCanvasDC") != 0)
        flasher->SetFlashing(TRUE); 
    else
        dc.EndDrawing();

    flasher->Flash();
}


// Adjust number of scrollbar steps if necessary.
void TreeWindow::AdjustScrollbarSteps(void)
{
    int width, height;
    int x_steps, y_steps;
    float zoom = draw_settings.Zoom();

    GetParent()->GetClientSize(&width, &height);
    // width and height are the dimensions of the visible canvas.
    height -= 50; // This compensates for a bug in GetClientSize().
    height = int(height / zoom);
    width  = int(width  / zoom);

    // x_steps and y_steps are the maximum number of scrollbar
    // steps in the x and y directions.
    if (draw_settings.MaxX() < width)
    {
        x_steps = 1;
    }
    else
    {
        //x_steps = draw_settings.MaxX() / PIXELS_PER_SCROLL + 1;
        x_steps = int((draw_settings.MaxX() * zoom) / PIXELS_PER_SCROLL) + 1;
    }

    if (draw_settings.MaxX() < height)
    {
        y_steps = 1;
    }
    else
    {
        //y_steps = draw_settings.MaxY() / PIXELS_PER_SCROLL + 1;
        y_steps = int((draw_settings.MaxY() * zoom) / PIXELS_PER_SCROLL) + 1;
    }

    if ((x_steps != draw_settings.x_steps()) ||
        (y_steps != draw_settings.y_steps()))
    {
        draw_settings.set_x_steps(x_steps);
        draw_settings.set_y_steps(y_steps);
        SetScrollbars(PIXELS_PER_SCROLL, PIXELS_PER_SCROLL, 
                    x_steps, y_steps, 4, 4);
    }
}



void TreeWindow::ProcessCursor(void)
{
    // A little scrollbar magic to insure the focus stays with the cursor.  This
    // can probably be optimized much further.  Consider using SetClippingRegion().
    // This also makes sure that the virtual canvas is large enough for the entire
    // tree.
    
    int x_start, y_start;
    int width, height;
    int x_steps, y_steps;
    int xs, xe, ys, ye;
    
    ViewStart(&x_start, &y_start);
    GetParent()->GetClientSize(&width, &height);
    height -= 50; // This compensates for a bug in GetClientSize().
    height  = int(height / draw_settings.Zoom());
    width   = int(width  / draw_settings.Zoom());
    x_steps = draw_settings.x_steps();
    y_steps = draw_settings.y_steps();
    
    // Make sure the cursor is visible.
    NodeEntry *entry = GetNodeEntry(cursor);
    if (!entry)
    {
        cursor = ef.RootNode();
        entry = GetNodeEntry(cursor);
    }
    
    // Check if the cursor is in the visible x-dimension.
    // xs, xe and NodeLength are in pixels (absolute units).

    xs = entry->x;
    xe = xs + draw_settings.NodeLength();

    if (xs < (x_start * PIXELS_PER_SCROLL))
        x_start = xs / PIXELS_PER_SCROLL - 1;

    if (xe > (x_start * PIXELS_PER_SCROLL + width))
        x_start = xe / PIXELS_PER_SCROLL - (width / 2) / PIXELS_PER_SCROLL;

    if (x_start < 0) 
        x_start = 0;

    if (x_start > x_steps)
        x_start = x_steps;
    
    // Check if the cursor is in the visible y-dimension.
    ys = entry->y-10;
    ye = entry->y+10;
    if (ys < y_start*PIXELS_PER_SCROLL)
        y_start = ys/PIXELS_PER_SCROLL-1;
    if (ye > y_start*PIXELS_PER_SCROLL+height)
        y_start = ye/PIXELS_PER_SCROLL-height/PIXELS_PER_SCROLL;
    if (y_start < 0) 
        y_start = 0;
    if (y_start > y_steps)
        y_start = y_steps;
    
    // now update the flasher
    UpdateCursor(entry);
    if (zoom_window)
        zoom_window->UpdateCursor(entry);

    if ((x_start != draw_settings.get_x_scroll()) ||
        (y_start != draw_settings.get_y_scroll()))
    {
        Scroll(x_start, y_start);
        draw_settings.set_x_scroll(x_start);
        draw_settings.set_y_scroll(y_start);
    }

    frame->OnSelectedMoved(cursor);
}


void TreeWindow::ProcessClick(wxMouseEvent &ev)
{
    float x, y;
    ev.Position(&x, &y);
    
    for (int i = 1; i <= node_list.Length(); i++)
    {
        NodeEntry *entry = node_list[i];
        // check if clicked on a node
        if(x > entry->x &&
           x < entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING &&
           y > entry->y-DELTA && y < entry->y+DELTA)
            cursor = (Node *)entry->n;
    }
}

//
// Double clicking on a node will activate the iset hilight function
// Double clicking on any of the displayed labels will call up dialogs to modify
// them. This function is similar to RenderLabels
//

void TreeWindow::ProcessDClick(wxMouseEvent &ev)
{
    float x, y;
    ev.Position(&x, &y);
    int id = -1;
    for (int i = 1; i <= node_list.Length(); i++)
    {
        NodeEntry *entry = node_list[i];
        // Check if double clicked on a node
        if (x > entry->x+entry->nums*INFOSET_SPACING &&
            x < entry->x+entry->nums*INFOSET_SPACING+
            draw_settings.NodeLength()-SUBGAME_LARGE_ICON_SIZE &&
            y > entry->y-DELTA && y < entry->y+DELTA)
        {
            cursor = (Node *)entry->n;
            if (cursor->GetInfoset()) // implement iset hilighting
                frame->HilightInfoset(cursor->GetPlayer()->GetNumber(),
                                      cursor->GetInfoset()->GetNumber(), 1);
            OnPaint();
            return;
        }
        // implement subgame toggle (different for collapsed and expanded)
        if (entry->n->GetSubgameRoot() == entry->n)
            if (entry->expanded)
            {
                if(x > entry->x && x < entry->x+SUBGAME_SMALL_ICON_SIZE &&
                   y > entry->y-SUBGAME_SMALL_ICON_SIZE/2 &&
                   y < entry->y+SUBGAME_SMALL_ICON_SIZE/2)
                {
                    cursor = (Node *)entry->n;
                    subgame_toggle();
                    OnPaint();
                    return;
                }
            }
            else
            {
                if (x > entry->x+draw_settings.NodeLength()+
                    entry->nums*INFOSET_SPACING-SUBGAME_LARGE_ICON_SIZE &&
                    x < entry->x+draw_settings.NodeLength()+
                    entry->nums*INFOSET_SPACING+SUBGAME_LARGE_ICON_SIZE &&
                    y > entry->y-SUBGAME_LARGE_ICON_SIZE/2 &&
                    y < entry->y+SUBGAME_LARGE_ICON_SIZE/2)
                {
                    cursor = (Node *)entry->n;
                    subgame_toggle();
                    OnPaint();
                    return;
                }
            }
        
        // Check if clicked on a Node Above/Below
        if(x > entry->x+entry->nums*INFOSET_SPACING+3 && x < entry->x+MAX_TW)
        {
            if (y > entry->y-MAX_TH && y < entry->y+DELTA)
                id = draw_settings.LabelNodeAbove();
            if (y > entry->y+DELTA && y < entry->y+MAX_TH+DELTA)
                id = draw_settings.LabelNodeBelow();
        }
        if (id != -1)
        {
            cursor = (Node *)entry->n;
            switch (id)
            {
            case NODE_ABOVE_NOTHING: break;
            case NODE_ABOVE_LABEL: node_label(); break;
            case NODE_ABOVE_PLAYER: tree_players(); break;
            case NODE_ABOVE_ISETLABEL: infoset_switch_player(); break;
            case NODE_ABOVE_ISETID:  infoset_switch_player(); break;
            case NODE_ABOVE_OUTCOME: ChangePayoffs(); break;
            case NODE_ABOVE_REALIZPROB: break;
            case NODE_ABOVE_BELIEFPROB: break;
            case NODE_ABOVE_VALUE: break;
            }
            OnPaint();
            return;
        }
        
        // Check if clicked on a branch Above/Below
        if (entry->parent)  // no branches for root
        {
            if(x > entry->x-draw_settings.BranchLength() &&
               x < entry->x-draw_settings.BranchLength()+MAX_TW)
            {
                if (y > entry->y-MAX_TH && y < entry->y+DELTA)
                    id = draw_settings.LabelBranchAbove();
                if (y > entry->y+DELTA && y < entry->y+MAX_TH)
                    id = draw_settings.LabelBranchBelow();
            }
        }
        if (id != -1)
        {
            cursor = (Node *)entry->parent->n;
            switch (id)
            {
            case BRANCH_ABOVE_NOTHING: break;
            case BRANCH_ABOVE_LABEL: action_label(); break;
            case BRANCH_ABOVE_PLAYER: tree_players(); break;
            case BRANCH_ABOVE_PROBS: action_probs(); break;
            case BRANCH_ABOVE_VALUE: break;
            }
            OnPaint();
            return;
        }
        
        // Check if clicked to the right of a node
        if(x > entry->x+draw_settings.NodeLength()+10 &&
           x < entry->x+draw_settings.NodeLength()+10+
           draw_settings.OutcomeLength()*ef.NumPlayers() &&
           y > entry->y-DELTA && y < entry->y+DELTA)
        {
	  id = draw_settings.LabelNodeRight();
        }
        if (id != -1)
        {
            cursor = (Node *)entry->n;
            switch (id) {
            case NODE_RIGHT_NOTHING:
	      break;
            case NODE_RIGHT_OUTCOME:
            case NODE_RIGHT_NAME:
	      ChangePayoffs();
	      break;
            }
            OnPaint();
            return;
        }
    }
}

//
// Right Clicking on a label will tell you what the label is refering to
// If the click was not on top of a text label, a popup menu (Buid) is created
//
void TreeWindow::ProcessRClick(wxMouseEvent &ev)
{
// This #define is a hack to prevent a compiler warning:
#define  DONT_USE_LEGENDS_SRC
#include "legend.h"
    float x, y;
    ev.Position(&x, &y);
    NodeEntry *entry;
    char *s = 0;
    for (int i = 1; i <= node_list.Length(); i++)
    {
        entry = node_list[i];
        // Check if clicked on a Node Above/Below
        if(x > entry->x+entry->nums*INFOSET_SPACING+3 && x < entry->x+MAX_TW)
        {
            if (y > entry->y-MAX_TH && y < entry->y+DELTA)
                s = node_above_src[draw_settings.LabelNodeAbove()].l_name;
            if (y > entry->y+DELTA && y < entry->y+MAX_TH+DELTA)
                s = node_below_src[draw_settings.LabelNodeBelow()].l_name;
        }
        // Check if clicked on a branch Above/Below
        if (entry->parent)  // no branches for root
        {
            if(x > entry->x-draw_settings.BranchLength() &&
               x < entry->x-draw_settings.BranchLength()+MAX_TW)
            {
                if (y > entry->y-MAX_TH && y < entry->y+DELTA)
                    s = branch_above_src[draw_settings.LabelBranchAbove()].l_name;
                if (y > entry->y+DELTA && y < entry->y+MAX_TH)
                    s = branch_below_src[draw_settings.LabelBranchBelow()].l_name;
            }
        }
        // Check if clicked to the right of a node
        if(x > entry->x+draw_settings.NodeLength()+10 &&
           x < entry->x+draw_settings.NodeLength()+10+
           draw_settings.OutcomeLength()*ef.NumPlayers() &&
           y > entry->y-DELTA && y < entry->y+DELTA)
        {
	  s = node_right_src[draw_settings.LabelNodeRight()].l_name;
        }
        if (s)
        {
            frame->SetStatusText(s);
            return;
        }
    }
    frame->SetStatusText("");
    // If we got here, the click was NOT on top of a text label, do the menu
    int x_start, y_start;
    ViewStart(&x_start, &y_start);
    PopupMenu(edit_menu, GetDC()->LogicalToDeviceX(x-x_start*PIXELS_PER_SCROLL),
              GetDC()->LogicalToDeviceY(y-y_start*PIXELS_PER_SCROLL));

#undef  DONT_USE_LEGENDS_SRC
}

// Double Right Clicking on a label will let you change what is displayed
void TreeWindow::ProcessRDClick(wxMouseEvent &ev)
{
    float x, y;
    ev.Position(&x, &y);
    NodeEntry *entry;
    int id = -1;
    for (int i = 1; i <= node_list.Length(); i++)
    {
        entry = node_list[i];
        // Check if clicked on a Node Above/Below
        if(x > entry->x+entry->nums*INFOSET_SPACING+3 && x < entry->x+MAX_TW)
        {
            if (y > entry->y-MAX_TH && y < entry->y+DELTA)
                id = NODE_ABOVE_LEGEND;
            if (y > entry->y+DELTA && y < entry->y+MAX_TH+DELTA) 
                id = NODE_BELOW_LEGEND;
        }
        // Check if clicked on a branch Above/Below
        if (entry->parent)  // no branches for root
        {
            if(x > entry->x-draw_settings.BranchLength() && 
               x < entry->x-draw_settings.BranchLength()+MAX_TW)
            {
                if (y > entry->y-MAX_TH && y < entry->y+DELTA) 
                    id = BRANCH_ABOVE_LEGEND;
                if (y > entry->y+DELTA && y < entry->y+MAX_TH)
                    id = BRANCH_BELOW_LEGEND;
            }
        }
        // Check if clicked to the right of a node
        if(x > entry->x+draw_settings.NodeLength()+10 &&
           x < entry->x+draw_settings.NodeLength()+10+
           draw_settings.OutcomeLength()*ef.NumPlayers() &&
           y > entry->y-DELTA && y < entry->y+DELTA)
        {
	  id = NODE_RIGHT_LEGEND;
        }
        if (id != -1)
        {
            draw_settings.SetLegends(id);
            OnPaint();
            return;
        }
    }
}

//
// Process Shift
// In Gambit, holding down shift initiates a 'cut' function.  If the cursor
// is located over a 'cuttable' object (node, branch, iset line), it will change
// to a 'scissors.'  Pressing the left mouse button while the cursor is
// 'scissors' will cut the object under the cursor. Returns true if an actual
// cut took place.
//
bool TreeWindow::ProcessShift(wxMouseEvent &ev)
{
    if (!ev.ShiftDown())
    {
#ifndef LINUX_WXXT
        if (wx_cursor == scissor_cursor)
            SetCursor(wxSTANDARD_CURSOR);
#else
        // wxxt doesn't have a wx_cursor field; this is a hack.
        SetCursor(wxSTANDARD_CURSOR);
#endif
        return false;
    }
    
    float x, y;
    ev.Position(&x, &y);
    NodeEntry *iset_cut_entry = 0, *node_cut_entry = 0, *branch_cut_entry = 0,
        *outcome_cut_entry = 0;
    bool cut_cursor = false;

    for (int i = 1; i <= node_list.Length() && !iset_cut_entry; i++)
    {
        NodeEntry *entry = node_list[i];
        // Check if the cursor is on top of a infoset line
        if (entry->infoset.y != -1 && entry->n->GetInfoset())
            if (x > entry->x+entry->num*INFOSET_SPACING-2 &&
                x < entry->x+entry->num*INFOSET_SPACING+2)
                if (y > entry->y && y < entry->infoset.y)
                {
                    // next iset is below this one
                    iset_cut_entry = entry;
                    cut_cursor = true;
                    break;
                }
                else if (y > entry->infoset.y && y < entry->y)
                {
                    // next iset is above this one
                    iset_cut_entry = entry;
                    cut_cursor = true;
                    break;
                }

        // Check if the cursor is on top of a node
        if (x > entry->x+entry->num*INFOSET_SPACING+10 && 
            x < entry->x+draw_settings.NodeLength()+entry->num*INFOSET_SPACING &&
            y > entry->y-2 && y < entry->y+2)
        {
            node_cut_entry = entry;
            cut_cursor = true;
            break;
        }

        // Check if the cursor is on top of an outcome
        if (entry->has_children == 0 && entry->n->GetOutcome())
        {
            if (x > entry->x+entry->num*INFOSET_SPACING+10+draw_settings.NodeLength() &&
                x < entry->x+draw_settings.NodeLength()+
                entry->num*INFOSET_SPACING+draw_settings.OutcomeLength() &&
                y > entry->y-2 && y < entry->y+2)
            {
                outcome_cut_entry = entry;
                cut_cursor = true;
                break;
            }
        }
        
        // Check if the cursor is on top of a branch
        NodeEntry *parent_entry = GetNodeEntry(entry->n->GetParent());
        if (parent_entry)
        {
            if (x > parent_entry->x+draw_settings.NodeLength()+
                parent_entry->num*INFOSET_SPACING+10 &&
                x < parent_entry->x+draw_settings.NodeLength()+
                draw_settings.ForkLength()+parent_entry->num*INFOSET_SPACING)
            {
                // Good old slope/intercept method for finding a point on a line
                int y0 = parent_entry->y + 
                    (int) (x - parent_entry->x - 
                           draw_settings.NodeLength() -
                           parent_entry->nums*INFOSET_SPACING) *
                    (entry->y-parent_entry->y)/draw_settings.ForkLength();

                if (y > y0-2 && y < y0+2)
                {
                    branch_cut_entry = entry;
                    cut_cursor = true;
                    break;
                }
            }
        }
    }

    if (ev.LeftDown() && cut_cursor)
    {
        // clicking the left mouse button will ...
        if (iset_cut_entry)  // cut an infoset
        {
            Infoset *siset = ef.SplitInfoset((Node *)iset_cut_entry->n);
            siset->SetName("Infoset"+ToText(siset->GetPlayer()->NumInfosets()));
            infosets_changed = TRUE;
            OnPaint();
            return true;
        }

        if (node_cut_entry)  // cut a node
        {
            ef.DeleteTree((Node *)node_cut_entry->n);
            nodes_changed = TRUE;
            OnPaint();
            return true;
        }

        if (outcome_cut_entry)
        {
            ((Node *) outcome_cut_entry->n)->SetOutcome(0);
            outcomes_changed = TRUE;
            OnPaint();
            return true;
        }

        if (branch_cut_entry)  // cut a branch
        {
            ef.DeleteAction(branch_cut_entry->n->GetParent()->GetInfoset(),
                            LastAction((Node *)branch_cut_entry->n));
            nodes_changed = TRUE;
            OnPaint();
            return true;
        }
    }
    
#ifndef LINUX_WXXT
    if (!cut_cursor && wx_cursor == scissor_cursor)
        SetCursor(wxSTANDARD_CURSOR);

    if (cut_cursor && wx_cursor != scissor_cursor) 
        SetCursor(scissor_cursor);
#else
    // wxxt doesn't have a wx_cursor field; this is a hack.
    if (!cut_cursor)
        SetCursor(wxSTANDARD_CURSOR);

    if (cut_cursor) 
        SetCursor(scissor_cursor);
#endif // LINUX_WXXT
    
    return false;
}

void TreeWindow::HilightInfoset(int pl, int iset)
{
    hilight_infoset = 0;

    if (pl >= 1 && pl <= ef.NumPlayers())
    {
        EFPlayer *p = ef.Players()[pl];
        if (iset >= 1 && iset <= p->NumInfosets())
            hilight_infoset = p->Infosets()[iset];
    }

    OnPaint();
}

//
// SupportChanged -- must be called by parent every time the disp_sup
// changes.  Note that since it is a reference, it needs not be passed here.
//
void TreeWindow::SupportChanged(void)
{
    must_recalc = TRUE;
    // Check if the cursor is still valid
    NodeEntry *ne = GetNodeEntry(cursor);
    if (ne->child_number)
    {
        if (!disp_sup->Find(cursor->GetInfoset()->Actions()[ne->child_number]))
            cursor = ef.RootNode();
    }

    OnPaint();
}

void TreeWindow::SetSubgamePickNode(const Node *n)
{
    if (n)
    {
        Node *cur_cursor = cursor; // save the REAL cursor
        cursor = (Node *)n;   // fake the cursor movement to ensure that the node is visible
        ProcessCursor();
        NodeEntry *ne = GetNodeEntry(n);
        DrawSubgamePickIcon(*GetDC(), *ne);
        cursor = cur_cursor;
    }
    subgame_node = n;
}

void TreeWindow::OnPopup(wxMenu &ob, wxCommandEvent &ev)
{
    ((EfgShow *)ob.GetClientData())->OnMenuCommand(ev.commandInt);
}


Node *TreeWindow::GotObject(float &x, float &y, int what)
{
    for (int i = 1; i <= node_list.Length(); i++)
    {
        NodeEntry *entry = node_list[i];
        
        if (what == DRAG_NODE_START) // check if clicked a non terminal node
            if (entry->n->NumChildren() != 0)
                if(x > entry->x+entry->nums*INFOSET_SPACING &&
                   x < entry->x+draw_settings.NodeLength()+
                   entry->nums*INFOSET_SPACING-10 &&
                   y > entry->y-DELTA && y < entry->y+DELTA) 
                    return (Node *)entry->n;
        
        if (what == DRAG_NODE_END) // check if clicked on a terminal node
            if (entry->n->NumChildren() == 0)
                if(x > entry->x+entry->nums*INFOSET_SPACING &&
                   x < entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING &&
                   y > entry->y-DELTA && y < entry->y+DELTA)
                    return (Node *)entry->n;
        
        if (what == DRAG_OUTCOME_START) // check if clicked on a terminal node
            if (entry->n->NumChildren() == 0 && entry->n->GetOutcome())
                if(x > entry->x+entry->nums*INFOSET_SPACING+draw_settings.NodeLength() &&
                   x < entry->x+draw_settings.NodeLength()+
                   entry->nums*INFOSET_SPACING + draw_settings.OutcomeLength()&&
                   y > entry->y-DELTA && y < entry->y+DELTA)
                    return (Node *)entry->n;
        
        if (what == DRAG_OUTCOME_END) // check if clicked on any valid node
            if(x > entry->x+entry->nums*INFOSET_SPACING && 
               x < entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING &&
               y > entry->y-DELTA && y < entry->y+DELTA)
                return (Node *)entry->n;
        
        if (what == DRAG_ISET_START || what == DRAG_ISET_END)
            // check if clicked on a non terminal node
            if (entry->n->NumChildren() != 0)
                if(x > entry->x+entry->num*INFOSET_SPACING-4 && 
                   x < entry->x+entry->num*INFOSET_SPACING+4 &&
                   y > entry->y-4 && y < entry->y+4)
                {
                    x = entry->x+entry->num*INFOSET_SPACING;
                    y = entry->y;
                    return (Node *)entry->n;
                }
        
        if (what == DRAG_BRANCH_START)
            // check if clicked on the very end of a node
            if (x > entry->x+draw_settings.NodeLength()+
                entry->nums*INFOSET_SPACING-4 &&
                x < entry->x+draw_settings.NodeLength()+
                entry->nums*INFOSET_SPACING+4 &&
                y > entry->y-4 && y < entry->y+4)
            {
                x = entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING;
                y = entry->y;
                return (Node *)entry->n;
            }
        
        if (what == DRAG_BRANCH_END)  // check if released in a valid position
        {
            NodeEntry *start_entry = GetNodeEntry(branch_drag->StartNode());
            int xs = start_entry->x+draw_settings.NodeLength()+
                draw_settings.ForkLength()+start_entry->nums*INFOSET_SPACING;
            if (x > xs && x < xs+draw_settings.BranchLength() &&
                y < start_entry->y+(start_entry->n->NumChildren()+1)*draw_settings.YSpacing() &&
                y > start_entry->y-(start_entry->n->NumChildren()+1)*draw_settings.YSpacing())
            {
                // figure out at what branch # the mouse was released
                int br = 1;
                NodeEntry *child_entry, *child_entry1;
                for (int ii = 1; ii <= start_entry->n->NumChildren()-1; ii++)
                {
                    child_entry = GetNodeEntry(start_entry->n->GetChild(ii));
                    if (ii == 1) 
                        if (y < child_entry->y)
                        {
                            br = 1;
                            break;
                        }
                    child_entry1 = GetNodeEntry(start_entry->n->GetChild(ii+1));
                    if (y > child_entry->y && y < child_entry1->y)
                    {
                        br = ii+1;
                        break;
                    }
                    if (ii == start_entry->n->NumChildren()-1 && y > child_entry1->y)
                    {
                        br = start_entry->n->NumChildren()+1;
                        break;
                    }
                }
                x = br;
                return (Node *)start_entry->n;
            }
            else
                return 0;
        }
    }
    return 0;
}

//***********************************************************************
//                      FILE-SAVE MENU HANDLER
//***********************************************************************
Efg *CompressEfg(const Efg &, const EFSupport &);

void TreeWindow::file_save(void)
{
  static int s_nDecimals = 6;
  gText filename = frame->Filename();
  gText s = wxFileSelector("Save data file", (char *)gPathOnly(filename),
               (char *)gFileNameFromPath(filename), ".efg", "*.efg",
               wxSAVE | wxOVERWRITE_PROMPT);

#ifdef __GNUG__
    // Overwrite protection doesn't work in Unix, so we
    // have to check explicitly.

    if (wxFileExists((char *) s))  {  // Ask for confirmation.
      if (wxMessageBox("File exists.  Overwrite?", "Confirm", wxOK | wxCANCEL) 
      != wxOK) {
    return;
      }
    }
#endif  // __GNUG__

    if (s != "") {
      // Allow to change description 
      if (filename != "untitled.efg") {
    char *label = 0;
    MyDialogBox *efg_save_dialog = 0;

    try {
      label = new char[256];
      strcpy(label, ef.GetTitle());

      efg_save_dialog = new MyDialogBox(pframe, "Label Game",
			      EFG_TREE_HELP);
      wxFormItem *label_item = 
        wxMakeFormString("Label", &label, wxFORM_DEFAULT,
	  new wxList(wxMakeConstraintFunction(LongStringConstraint), 0), 
	  0, 0, 350);
      efg_save_dialog->Add(label_item);
      efg_save_dialog->Add(wxMakeFormNewLine());
      wxFormItem *decimals_item =
        wxMakeFormShort("Decimals", &s_nDecimals, wxFORM_DEFAULT,
          new wxList(wxMakeConstraintRange(0, 25), 0));
      efg_save_dialog->Add(decimals_item);
      efg_save_dialog->AssociatePanel();
      ((wxText *)label_item->PanelItem)->SetFocus();
      efg_save_dialog->Go1();
    
      if (efg_save_dialog->Completed() == wxOK)
        ef.SetTitle(label);
    
      delete efg_save_dialog;
      delete [] label;
    }
    catch (gException &E) {
      if (label)
        delete [] label;
      if (efg_save_dialog)
        delete efg_save_dialog;
    
      guiExceptionDialog(E.Description(), pframe);
      return;
    }
      }
      gFileOutput out((const char *)s);
      // Compress the efg to the current support
      Efg *E = CompressEfg(ef, *frame->GetSupport(0));
      E->WriteEfgFile(out, s_nDecimals);
      delete E;
      frame->SetFileName(s);
    }
}

template class gList<NODEENTRY *>;
template class gList<TreeWindow::SUBGAMEENTRY>;
gOutput &operator<<(gOutput &o, const TreeWindow::SUBGAMEENTRY &)
{
    return o;
}

