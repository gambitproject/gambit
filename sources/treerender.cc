//
// FILE: treerender.cc -- Implementation of class TreeRender
//
// $Id$
//

#include "wx.h"
#include "wxmisc.h"
#ifdef wx_msw
#include "wx_mf.h"
#endif  // wx_msw

#include "efg.h"

#include "legendc.h"
#include "twflash.h"
#include "treewin.h"
#include "efgshow.h"

int INFOSET_SPACING = 10;
int SUBGAME_LARGE_ICON_SIZE = 20;
int SUBGAME_SMALL_ICON_SIZE = 10;
int SUBGAME_PICK_SIZE = 30;

//-----------------------------------------------------------------------
//                    MISCELLANEOUS FUNCTIONS
//-----------------------------------------------------------------------

inline void DrawLine(wxDC &dc, double x_s, double y_s, double x_e, double y_e,
                     int color = 0, int thick = 0)
{
  if (dc.Colour) { 
    if (color > -1 && color < WX_COLOR_LIST_LENGTH) {
      dc.SetPen(wxThePenList->FindOrCreatePen((char *) wx_color_list[color],
					      (thick) ? 8 : 2, wxSOLID));
    }
        
    if (color >= WX_COLOR_LIST_LENGTH && color < 2 * WX_COLOR_LIST_LENGTH) {
      dc.SetPen(wxThePenList->FindOrCreatePen
		((char *) wx_hilight_color_list[color % WX_COLOR_LIST_LENGTH],
		 2, wxSOLID));
    }
  }
  else
    dc.SetPen(wxThePenList->FindOrCreatePen("BLACK", 2, wxSOLID));
    
  dc.DrawLine(x_s, y_s, x_e, y_e);
}

inline void DrawRectangle(wxDC &dc, int x_s, int y_s, int w, int h,
                          int color = 0)
{
  if (color > -1) {
    dc.SetPen(wxThePenList->FindOrCreatePen((char *) wx_color_list[color], 
					    2, wxSOLID));
  }
  
  dc.DrawRectangle(x_s, y_s, w, h);
}

inline void DrawThinLine(wxDC &dc, int x_s, int y_s, int x_e, int y_e,
                         int color = 0)
{
  if (dc.Colour && color > -1)
    dc.SetPen(wxThePenList->FindOrCreatePen((char *)wx_color_list[color],
					    1, wxSOLID));
  else
    dc.SetPen(wxThePenList->FindOrCreatePen("BLACK", 1, wxSOLID));
    
  dc.DrawLine(x_s, y_s, x_e, y_e);
}

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

inline void DrawCircle(wxDC &dc, int x, int y, int r, int color = 0)
{
  if (color > -1 && dc.Colour)
    dc.SetPen(wxThePenList->FindOrCreatePen((char *) wx_color_list[color],
					    3, wxSOLID));
  else
    dc.SetPen(wxThePenList->FindOrCreatePen("BLACK", 3, wxSOLID));
  dc.DrawEllipse(x-r, y-r, 2*r, 2*r);
}


void DrawLargeSubgameIcon(wxDC &dc, const NodeEntry &entry, int nl)
{
  dc.SetPen(wxThePenList->FindOrCreatePen("INDIAN RED", 2, wxSOLID));
  dc.SetBrush(wxTheBrushList->FindOrCreateBrush("RED", wxSOLID));
  wxPoint points[3];
  int x0 = (entry.x + nl +
	    entry.nums * INFOSET_SPACING - SUBGAME_LARGE_ICON_SIZE);
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
//                        TreeRender: Life cycle
//-----------------------------------------------------------------------

TreeRender::TreeRender(wxFrame *frame, TreeWindow *parent_,
                       const gList<NodeEntry *> &node_list_,
                       const Infoset * &hilight_infoset_,
                       const Infoset * &hilight_infoset1_,
                       const Node *&mark_node_, const Node *&subgame_node_,
                       const TreeDrawSettings &draw_settings_)
  : wxCanvas(frame, -1, -1, -1, -1, 0),
    node_list(node_list_),
    hilight_infoset(hilight_infoset_), hilight_infoset1(hilight_infoset1_),
    mark_node(mark_node_), subgame_node(subgame_node_),
    parent(parent_), draw_settings(draw_settings_),
    flasher(0), painting(false)
{ }

TreeRender::~TreeRender(void)
{
  if (flasher) delete flasher;
}

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
  switch (draw_settings.LabelNodeAbove()) {
  case NODE_ABOVE_NOTHING:
    label ="";  
    break;
        
  case NODE_ABOVE_LABEL:
    label = n->GetName();  
    break;
        
  case NODE_ABOVE_PLAYER:
    if (n->GetPlayer()) 
      label = n->GetPlayer()->GetName();
    else
      label = "";
    break;
        
  case NODE_ABOVE_ISETLABEL:
    if (n->GetInfoset()) 
      label = n->GetInfoset()->GetName();
    break;
        
  case NODE_ABOVE_ISETID:
    if (n->GetInfoset())
      label = ("(" + ToText(n->GetPlayer()->GetNumber()) +
	       "," + ToText(n->GetInfoset()->GetNumber()) + ")");
    break;
    
  case NODE_ABOVE_OUTCOME:
    label = OutcomeAsString(n, hilight);
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
    
  if (label != "") {
    dc.SetFont(draw_settings.NodeAboveFont());
    dc.GetTextExtent("0", &tw, &th);
    gDrawText(dc, label, 
	      child_entry->x + child_entry->nums * INFOSET_SPACING + 3,
	      child_entry->y - th - 9);
  }
  
  // Take care of labeling the node on the bottom.
  switch (draw_settings.LabelNodeBelow()) { 
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
      label = ("(" + ToText(n->GetPlayer()->GetNumber()) +
	       "," + ToText(n->GetInfoset()->GetNumber()) + ")");
    break;
        
  case NODE_BELOW_OUTCOME:
    label = OutcomeAsString(n, hilight);
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
    
  if (label != "") {
    dc.SetFont(draw_settings.NodeBelowFont());
    gDrawText(dc, label, 
	      child_entry->x + child_entry->nums * INFOSET_SPACING + 3,
	      child_entry->y + 5);
  }
  
  if (child_entry->n != entry->n) {   // no branches for root
    // Now take care of branches....
    // Take care of labeling the branch on the top.
    switch (draw_settings.LabelBranchAbove()) {
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
    
    if (label != "") {
      dc.SetFont(draw_settings.BranchAboveFont());
      dc.GetTextExtent("0", &tw, &th);
      gDrawText(dc, label, 
		entry->x + entry->nums * INFOSET_SPACING + 
		draw_settings.ForkLength() + draw_settings.NodeLength() + 3,
		child_entry->y - th - 5);
    }
        
    // Take care of labeling the branch on the bottom.
    switch (draw_settings.LabelBranchBelow()) { 
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
        
    if (label != "") {
      dc.SetFont(draw_settings.BranchBelowFont());
      gDrawText(dc, label,
		entry->x + entry->nums * INFOSET_SPACING + 
		draw_settings.ForkLength() + draw_settings.NodeLength() + 3,
		child_entry->y + 5);
    }
  }
    
  // Now take care of displaying the terminal node labels.
  hilight = false;
    
  switch (draw_settings.LabelNodeRight()) { 
  case NODE_RIGHT_NOTHING:
    label = "";
    break;
            
  case NODE_RIGHT_OUTCOME:
    label = OutcomeAsString(n, hilight);
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
  for (int pos = 1; pos <= node_list.Length(); pos++) {
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
	(entry.infoset.y+dc.device_origin_y < y_start*PIXELS_PER_SCROLL+height/zoom)) {
      // draw the labels
      RenderLabels(dc, &child_entry, &entry);

      // Draw a triangle to show subgame roots
      if (entry.n->GetSubgameRoot() == entry.n) {
	if (entry.expanded) 
	  DrawSmallSubgameIcon(dc, entry);
      }

      // Only draw the node line once for all children.
      if (child_entry.child_number == 1) {
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
      }
      
      if (child_entry.n == subgame_node)
	DrawSubgamePickIcon(dc, child_entry);
    
      // draw the 'branches'
      if (child_entry.n->GetParent() && child_entry.in_sup) {
	// no branches for root node
	xs = entry.x+draw_settings.NodeLength()+entry.nums*INFOSET_SPACING;
	ys = entry.y;
	xe = xs+draw_settings.ForkLength();
	ye = child_entry.y;
	::DrawLine(dc, xs, ys, xe, ye, entry.color);

	// Draw the highlight... y = a + bx = ys + (ye-ys) / (xe-xs) * x
	double prob = parent->GetEfgFrame()->ActionProb(entry.n,
							child_entry.child_number);
	if (prob > 0) {
	  ::DrawLine(dc, xs, ys, (xs + draw_settings.ForkLength() * prob), 
		     (ys + (ye - ys) * prob), WX_COLOR_LIST_LENGTH - 1);
	}
	
	xs = xe;
	ys = ye;
	xe = child_entry.x;
	ye = ys;
	::DrawLine(dc, xs, ye, xe, ye, entry.color);
      }
      else {
	xe = entry.x;
	ye = entry.y;
      }
      
      // Take care of terminal nodes
      // (either real terminal or collapsed subgames)
      if (!child_entry.has_children) { 
	::DrawLine(dc, xe, ye, 
		   xe + draw_settings.NodeLength() + 
		   child_entry.nums * INFOSET_SPACING, 
		   ye, draw_settings.GetPlayerColor(-1));
      
	// Collapsed subgame: subgame icon is drawn at this terminal node.
	if ((child_entry.n->GetSubgameRoot() == child_entry.n) && 
	    !child_entry.expanded)
	  DrawLargeSubgameIcon(dc, child_entry, draw_settings.NodeLength());
      
	// Marked Node: a circle is drawn at this terminal node
	if (child_entry.n == mark_node) {
	  ::DrawCircle(dc, 
		       xe + child_entry.nums * INFOSET_SPACING +
		       draw_settings.NodeLength(), ye, 
		       4, draw_settings.CursorColor());
	}
      }

      // Draw a circle to show the marked node
      if ((entry.n == mark_node) && 
	  (child_entry.child_number == entry.n->Game()->NumChildren(entry.n))) {
	::DrawCircle(dc, entry.x + entry.nums * INFOSET_SPACING + 
		     draw_settings.NodeLength(), entry.y, 
		     4, draw_settings.CursorColor());
      }
    }

    if (child_entry.child_number == 1) {
      if (draw_settings.ShowInfosets()) {
	if (entry.infoset.y != -1) {
	  ::DrawThinLine(dc, 
			 entry.x + entry.num * INFOSET_SPACING, 
			 entry.y, 
			 entry.x + entry.num * INFOSET_SPACING, 
			 entry.infoset.y, 
			 entry.color);
	}
	  
	if (entry.infoset.x != -1) {
	  // Draw a little arrow in the direction of the iset.
	  if (entry.infoset.x > entry.x) { // iset is to the right
	    ::DrawLine(dc, 
		       entry.x+entry.num*INFOSET_SPACING, 
		       entry.infoset.y, 
		       entry.x+(entry.num+1)*INFOSET_SPACING, 
		       entry.infoset.y, entry.color);
	  }
	  else {  // iset is to the left
	    ::DrawLine(dc, 
		       entry.x + entry.num * INFOSET_SPACING, 
		       entry.infoset.y, 
		       entry.x + (entry.num - 1) * INFOSET_SPACING, 
		       entry.infoset.y, 
		       entry.color);
	  }
	}
      }
    }
  }
}

void TreeRender::UpdateCursor(const NodeEntry *entry)
{
  if (entry->n->GetSubgameRoot() == entry->n && !entry->expanded) {
    flasher->SetFlashNode(entry->x + draw_settings.NodeLength() +
			  entry->nums*INFOSET_SPACING - SUBGAME_LARGE_ICON_SIZE,
			  entry->y,
			  entry->x + draw_settings.NodeLength() +
			  entry->nums*INFOSET_SPACING,
			  entry->y, subgameCursor);
  }
  else {
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
  flasher = ((draw_settings.FlashingCursor()) ? new TreeNodeFlasher(GetDC()) :
	     new TreeNodeCursor(GetDC()));
}

gText TreeRender::OutcomeAsString(const Node *n, bool &/*hilight*/) const
{
  if (n->GetOutcome()) {
    EFOutcome *tv = n->GetOutcome();
    const gArray<gNumber> &v = n->Game()->Payoff(tv);
    gText tmp = "(";

    for (int i = v.First(); i <= v.Last(); i++) {
      if (i != 1) 
	tmp += ",";
      
      if (draw_settings.ColorCodedOutcomes())
	tmp += ("\\C{"+ToText(draw_settings.GetPlayerColor(i))+"}");
      
      tmp += ToText(v[i], NumDecimals());
    }

    if (draw_settings.ColorCodedOutcomes()) 
      tmp += ("\\C{"+ToText(WX_COLOR_LIST_LENGTH-1)+"}");

    tmp += ")";
        
    return tmp;
  }
  else
    return "";
}



//----------------------------------------------------------------------
//                   TreeZoomWindow: Member functions
//----------------------------------------------------------------------

class TreeZoomFrame : public wxFrame {
private:
  TreeWindow *m_parent;

public:
  TreeZoomFrame(wxFrame *p_frame, TreeWindow *p_parent);
  virtual ~TreeZoomFrame() { }
  
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
                               const Node *&subgame_node_,
                               const TreeDrawSettings &draw_settings_,
                               const NodeEntry *cursor_entry)
  : TreeRender(new TreeZoomFrame(frame, parent), parent,
	       node_list_, hilight_infoset_, hilight_infoset1_,
	       mark_node_, subgame_node_, draw_settings_),
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


