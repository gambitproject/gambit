#include "wx.h"
#include "wx_form.h"
#include "wxmisc.h"
#include "extform.h"
#include "gconvert.h"
#pragma hdrstop
#include "gmisc.h"
#include "treewin.h"
#include "legendc.h"

#define BAD_NODE Node(-1,-1,-1,-1)

#define INFOSET_SPACING	10
extern char *gambit_color_list[GAMBIT_COLOR_LIST_LENGTH];

wxFont *outcome_font;
wxFont *fixed_font;
wxBrush *white_brush;

char tempstr[20];

template <class T>
char *OutcomeToString(const gVector<T> &v,TreeDrawParams *draw_settings,Bool color_coded=TRUE)
{
char tempstr[20];
static gString gvts;
gvts="";
for (int i=v.First();i<=v.Last();i++)
{
	gvts+=(i==1) ? '(' : ',';
	if (color_coded)
	{
		sprintf(tempstr,"\\C{%d}",draw_settings->GetPlayerColor(i));
		gvts+=tempstr;
	}
	sprintf(tempstr,"%2.2f",(double)v[i]);
	gvts+=tempstr;
}
gvts+=')';

return (char *)gvts;
}

//###
//Draw Line.  A quick and dirty way of easily drawing lines w/ set color
inline void DrawLine(wxDC &dc,int x_s,int y_s,int x_e,int y_e,int color=0)
{
if (color>-1)
	dc.SetPen(wxThePenList->FindOrCreatePen(gambit_color_list[color],2,wxSOLID));
dc.DrawLine(x_s,y_s,x_e,y_e);
}
//###
//Draw Rectangle.  A quick and dirty way of easily drawing rectangles w/ set color
inline void DrawRectangle(wxDC &dc,int x_s,int y_s,int w,int h,int color=0)
{
if (color>-1)
	dc.SetPen(wxThePenList->FindOrCreatePen(gambit_color_list[color],2,wxSOLID));
dc.DrawRectangle(x_s,y_s,w,h);
}
//###
//Draw Thin Line.  A quick and dirty way of easily drawing lines w/ set color
inline void DrawThinLine(wxDC &dc,int x_s,int y_s,int x_e,int y_e,int color=0)
{
if (color>-1)
	dc.SetPen(wxThePenList->FindOrCreatePen(gambit_color_list[color],.5,wxSOLID));
dc.DrawLine(x_s,y_s,x_e,y_e);
}
//###
//Draw Text. A quick and dirty way of easily drawing text w/ set color
inline void DrawText(wxDC &dc,char *s,int x,int y,int color=0)
{
if (color>-1)
	dc.SetPen(wxThePenList->FindOrCreatePen(gambit_color_list[color],2,wxSOLID));
dc.DrawText(s,x,y);
}
//Draw Circle. A quick and dirty way of easily drawing a circle w/ set color
inline void DrawCircle(wxDC &dc,int x,int y,int r,int color=0)
{
	if (color>-1)
		dc.SetPen(wxThePenList->FindOrCreatePen(gambit_color_list[color],2,wxSOLID));
	else
		dc.SetPen(wxThePenList->FindOrCreatePen("WHITE",2,wxSOLID));
	dc.DrawEllipse(x-r,y-r,2*r,2*r);
}




//=====================================================================
//                      TREEWINDOW MEMBER FUNCTIONS
//=====================================================================

//---------------------------------------------------------------------
//                TREEWINDOW: CONSTRUCTOR AND DESTRUCTOR
//---------------------------------------------------------------------
//************************* private constructor & assignment *******
template <class T>
TreeWindow<T>::TreeWindow(const TreeWindow<T> &t)
{assert(0);}
template <class T>
void TreeWindow<T>::operator=(const TreeWindow<T> &t)
{assert(0);}

//************************* normal constructor ********************
template <class T>
TreeWindow<T>::TreeWindow(ExtForm<T> *ef,ExtensiveShow<T> *_frame,
												int _subgame,int x,int y,int w,int h,int style)
												:	wxCanvas(_frame, x, y, w, h, style)
{
assert(ef);
frame=_frame;				// store the parent frame
the_problem=ef;
subgame=_subgame;		// which subgame I AM
// $$$
node_list = new wxList;
// $$$  node_list->DeleteContents(TRUE);	// make sure the client data is also deleted

// Create the iterator to process cursor movement
iterator=new TreeWinIter(the_problem,subgame);
// Create the flasher to flash the cursor or just a steady cursor
if (draw_settings.FlashingCursor())
	flasher=new TreeNodeFlasher(GetDC());
else
	flasher=new TreeNodeCursor(GetDC());
// No node has been marked yet--mark_node is invalid
mark_node=BAD_NODE;old_mark_node=BAD_NODE;
outcome_font=wxTheFontList->FindOrCreateFont(9,wxSWISS,wxNORMAL,wxNORMAL);
fixed_font=wxTheFontList->FindOrCreateFont(12,wxMODERN,wxNORMAL,wxNORMAL);
white_brush=wxTheBrushList->FindOrCreateBrush("WHITE",wxSOLID);
GetDC()->SetBackgroundMode(wxTRANSPARENT);
// Make sure the node_list gets recalculated the first time
nodes_changed=TRUE;
infosets_changed=TRUE;
// Create scrollbars
SetScrollbars(PIXELS_PER_SCROLL,PIXELS_PER_SCROLL,60,60,4,4);
draw_settings.set_x_steps(60);
draw_settings.set_y_steps(60);
}

template <class T>
TreeWindow<T>::~TreeWindow()
{
if (draw_settings.FlashingCursor())
	delete (TreeNodeFlasher *)flasher;
else
	delete (TreeNodeCursor *)flasher;
delete iterator;

if (subgame!=1) ;		// if I am a subgame, notify parent of my death
//	 ((ExtensiveShow<T> *)frame->parent)->tw->CloseSubgame(subgame);
else
	delete the_problem;
}

//---------------------------------------------------------------------
//                  TREEWINDOW: EVENT-HOOK MEMBERS
//---------------------------------------------------------------------

//*********************************************************************
// draw -- Handle drawing events
// Note:  This right now does a complete redraw of the window.  This
//        is _very_ stupid, and we need to develop a good algorithm
//        to only redraw what we need to redraw
//*********************************************************************
template <class T>
void TreeWindow<T>::OnPaint(void)
{
	Render(*(GetDC()));
}

//*********************************************************************
// OnChar -- handle keypress events
// Currently we support the following keys:
//     left arrow:   go to parent of current node
//     right arrow:  go to first child of current node
//     up arrow:     go to previous sibling of current node
//     down arrow:   go to next sibling of current node
//*********************************************************************
// ### Note: this is very ugly--I redraw the entire tree to update the cursor
// info.  This is due to the fact that Render function does not keep node
// coordinates data after redrawing.
template <class T>
void TreeWindow<T>::OnChar(wxKeyEvent& ch)
{
// Redraw can be forced by using ^L
if (ch.KeyCode()=='L' && ch.ControlDown()) OnPaint();
//---------------------------------- Accelerators --------------------------
// Note that accelerators are provided for in the wxwin code but only for the
// windows platform.  In order to make this more portable, accelerators for
// this program are coded in the header file and processed in OnChar
frame->CheckAccelerators(ch.KeyCode());
//--------------------------------Cursor Code----------------------------
if (ch.ShiftDown()==FALSE)
	switch (ch.KeyCode())
	{
		case WXK_LEFT:
		{
			iterator->GoParent();
			ProcessCursor();
			break;
		}
    case WXK_RIGHT:
		{
			if (!ch.ControlDown())
      {
				iterator->GoFirstChild();
				ProcessCursor();
			}
			else		// if node has a subgame, open it by pressing Ctrl-Right
			{
				if (the_problem->NumChildren(iterator->Cursor())==0)	// if terminal
					if (the_problem->HasSuccessorGame(iterator->Cursor()))
						tree_subgame_open(the_problem->GetNextGame(iterator->Cursor())+1);
      }
			break;
		}
    case WXK_UP:
		{
			iterator->GoPriorSibling();
			ProcessCursor();
			break;
		}
    case WXK_DOWN:
		{
			iterator->GoNextSibling();
			ProcessCursor();
			break;
		}
		case WXK_SPACE:
		{
			ProcessCursor();
			break;
		}
	}
else
{
	wxCanvas::OnChar(ch);
}
}

//*********************************************************************
// OnEvent -- handle mouse events
// Currently we support selecting a node by clicking on it
//*********************************************************************
template <class T>
void TreeWindow<T>::OnEvent(wxMouseEvent& event)
{
	float x, y;
	event.Position(&x, &y);
	if (event.LeftDown())
	{
		ProcessClick((int)x,(int)y);
		ProcessCursor();
	}
}


//---------------------------------------------------------------------
//                  TREEWINDOW: DRAWING UTILITIES
//---------------------------------------------------------------------



//*************************************************************************
// TREE RENDERING FUNCTIONS.  MOVED HERE FORM EXTFORM.CC
//**************************************************************************
template <class T>
NodeEntry *TreeWindow<T>::GetNodeEntry(const Node &n)
{
wxNode *e=node_list->First();
while (e)
{
	if (((NodeEntry *)e->Data())->n==n) return (NodeEntry *)e->Data();
	e=e->Next();
}
/*
for (int i=0;i<node_list->Number();i++)
{
	if (((NodeEntry *)node_list->Nth(i)->Data())->n==n)
		return ((NodeEntry *)node_list->Nth(i)->Data());
}
*/
return NULL;
}

int maxlev, maxy, miny, ycoord;

template <class T>
int TreeWindow<T>::FillTable(const Node &n, int level)
{
	int myy, y1, yn;

	NodeEntry *entry=new NodeEntry;
	entry->n=n;	// store the node the entry is for
	node_list->Append((wxObject *)entry);

	if (the_problem->NumChildren(n) > 0)
	{
		for (int i = 1; i <= the_problem->NumChildren(n); i++)
		{
			yn = FillTable(the_problem->GetChildNumber(n,i),level+1);
      if (i == 1)  y1 = yn;
    }
    entry->y = (y1 + yn) / 2;
	}
	else
	{
    entry->y = ycoord;
    ycoord += draw_settings.YSpacing();
  }

	entry->level = level;
	entry->has_children=the_problem->NumChildren(n);
	// Find out what branch of the parent this node is on
	if (n==the_problem->RootNode(subgame))
	{
		entry->child_number=0;
	}
	else
  {
		Node parent=the_problem->GetParent(n);
		for (int i=1;i<=the_problem->NumChildren(parent);i++)
			if (the_problem->GetChildNumber(parent,i)==n)
				entry->child_number=i;
	}
                                                            
	entry->infoset_y=-1;entry->num=-1;entry->nums=0;
	entry->x = level * (draw_settings.NodeLength() + draw_settings.BranchLength());
	entry->color = draw_settings.GetPlayerColor(n[1]);
	if (level > maxlev)    maxlev = level;

	if (entry->y > maxy)
		maxy = entry->y;
	else
		if (entry->y < miny)   miny = entry->y;

	return entry->y;
}
// CheckInfosetEntry.  Checks how many infoset lines are to be drawn at each
// level, spaces them by setting each infoset's node's num to the previous
// infoset node+1.  Also lengthens the nodes by the amount of space taken up
// by the infoset lines.
template <class T>
void TreeWindow<T>::CheckInfosetEntry(NodeEntry *e)
{
wxNode 			*n;
NodeEntry 	*infoset_entry,*e1;
// Check if the infoset this entry belongs to on this level has already
// been processed.  If so, make this entry->num the same as the one already
// processed and return
infoset_entry=NextInfoset(e);
n=node_list->First();
while (n)
{
	e1=(NodeEntry *)n->Data();
			// if the player,infoset are the same
	if (((e1->n)[1]==(e->n)[1]) && ((e1->n)[2]==(e->n)[2]) &&
			// and they are on the same level and e1 has been processed 
			(e->level==e1->level) && (e1->num!=-1))
			{
				e->num=e1->num;
        if (infoset_entry!=NULL) e->infoset_y=infoset_entry->y;
				return;
			}
	n=n->Next();
}
// If we got here, this entry does not belong to any processed infoset yet.
// Check if it belongs to ANY infoset, if not just return
if (infoset_entry==NULL) return;
// If we got here, then this entry is new and is connected to other entries
// find the entry on the same level with the maximum num.
// This entry will have num=num+1.
int num=-1;
n=node_list->First();
while (n)
{
	e1=(NodeEntry *)n->Data();
  // Find the max num for this level
	if (e->level==e1->level) if (e1->num>num) num=e1->num;
	n=n->Next();
}
num++;
e->num=num;
e->infoset_y=infoset_entry->y;
}
        
 
// NextInfoset
// Checks if there are any nodes in the same infoset as e that e is connected
// to.
template <class T>
NodeEntry *TreeWindow<T>::NextInfoset(NodeEntry *e)
{
NodeEntry *e1;
wxNode *n=node_list->Member((wxObject *)e);
wxNode *n1=n->Next();

while (n1)
{
	e1=(NodeEntry *)n1->Data();
	// if players, infosets are the same and the nodes are on the same level
	if (((e1->n)[1]==(e->n)[1]) && ((e1->n)[2]==(e->n)[2]) && (e->level==e1->level))
		return e1;
	n1=n1->Next();
}
return NULL;
}

template <class T>
void TreeWindow<T>::FillInfosetTable(const Node &n)
{
NodeEntry *entry=GetNodeEntry(n);
if (the_problem->NumChildren(n) > 0)
	for (int i = 1; i <= the_problem->NumChildren(n); i++)
		FillInfosetTable(the_problem->GetChildNumber(n,i));
CheckInfosetEntry(entry);
}

template <class T>
void TreeWindow<T>::UpdateTableInfosets(void)
{
int *nums=new int[maxlev+1];	// create an array to hold max num for each level
int i;
for (i=0;i<=maxlev;i++) nums[i]=0;
wxNode *n=node_list->First();
NodeEntry *e;
// find the max e->num for each level
while (n)
{
	e=(NodeEntry *)n->Data();
	if (e->num+1>nums[e->level]) nums[e->level]=e->num+1;
	n=n->Next();
}
// record the max e->num for each level for each node
n=node_list->First();
while (n)
{
	e=(NodeEntry *)n->Data();
	e->nums=nums[e->level];
	n=n->Next();
}
for (i=0;i<maxlev;i++) nums[i+1]+=nums[i];
// now add the needed length to each level
n=node_list->First();
while (n)
{
	e=(NodeEntry *)n->Data();
	if (e->level!=0) e->x+=nums[e->level-1]*INFOSET_SPACING;
	n=n->Next();
}
delete [] nums;
}

template <class T>
void TreeWindow<T>::UpdateTableParents(void)
{
NodeEntry *e;
wxNode *n=node_list->First();
while (n)
{
	e=(NodeEntry *)n->Data();
	if (e->n!=the_problem->RootNode(subgame))
		e->parent=GetNodeEntry(the_problem->GetParent(e->n));
	else
		e->parent=e;
	n=n->Next();
}
}
		  	

// RenderLabels.  Draws all the text labels for the tree according to the
// settings in draw_settings.  Currently takes care of:labels node/branch,
// outcomes.  Note: this function is getting damn long, but I see no real
// reason to split it at this point...

template <class T>
void TreeWindow<T>::RenderLabels(wxDC &dc,NodeEntry *child_entry)
{
gString 		label;		// temporary to hold the label
NodeEntry 	*entry=child_entry->parent;
Node 				 n=child_entry->n;
int					 o;
float tw,th;
// First take care of labeling the node on top
switch (draw_settings.LabelNodeAbove())
{
	case NODE_ABOVE_NOTHING:
		label="";break;
	case NODE_ABOVE_LABEL:
		label=the_problem->GetNodeLabel(n);break;
	case NODE_ABOVE_PLAYER:
		label=the_problem->GetPlayerLabel(n[1]);break;
	case NODE_ABOVE_INFOSET:
		sprintf(tempstr,"(%d,%d)",n[1],n[2]);label=tempstr;break;
	case NODE_ABOVE_OUTCOME:
		if (o=the_problem->GetOutcome(n))
			label=OutcomeToString(the_problem->GetOutcomeValues(o),&draw_settings);
		else
			label="";break;
	default:
		label="";break;
}
if (label!="")
{
	dc.SetFont(draw_settings.NodeAboveFont());
	dc.GetTextExtent("0",&tw,&th);
	gDrawText(dc,label,child_entry->x+child_entry->nums*INFOSET_SPACING,child_entry->y-th-5);
}
// Take care of labeling the node on the bottom
switch (draw_settings.LabelNodeBelow())
{
	case NODE_BELOW_NOTHING:
		label="";break;
	case NODE_BELOW_LABEL:
		label=the_problem->GetNodeLabel(n);break;
	case NODE_BELOW_PLAYER:
		label=the_problem->GetPlayerLabel(n[1]);break;
	case NODE_BELOW_INFOSET:
		sprintf(tempstr,"(%d,%d)",n[1],n[2]);label=tempstr;break;
	case NODE_BELOW_OUTCOME:
		if (o=the_problem->GetOutcome(n))
			label=OutcomeToString(the_problem->GetOutcomeValues(o),&draw_settings);
		else
			label="";break;
	default:
		label="";break;
}
if (label!="")
{
	dc.SetFont(draw_settings.NodeBelowFont());
	gDrawText(dc,label,child_entry->x+child_entry->nums*INFOSET_SPACING,child_entry->y+3);
}
if (child_entry!=entry)	// no branches for root
{
// Now take care of branches....
// Take care of labeling the branch on the top
switch (draw_settings.LabelBranchAbove())
{
	case BRANCH_ABOVE_NOTHING:
		label="";break;
	case BRANCH_ABOVE_LABEL:
		if (child_entry->child_number!=0)
			label=the_problem->GetActionLabel(entry->n,child_entry->child_number);
		else
			label="";break;
	case BRANCH_ABOVE_PLAYER:
		label=the_problem->GetPlayerLabel(entry->n[1]);break;
	case BRANCH_ABOVE_INFOSET:
		sprintf(tempstr,"(%d,%d)",entry->n[1],entry->n[2]);label=tempstr;break;
	case BRANCH_ABOVE_PROBS:
		if (entry->n[1]==0)
			label=ToString(the_problem->GetActionProb(entry->n,child_entry->child_number));
		else label="";break;
	default:
		label="";break;
}
if (label!="")
{
	dc.SetFont(draw_settings.BranchAboveFont());
	dc.GetTextExtent("0",&tw,&th);
	gDrawText(dc,label,entry->x+entry->nums*INFOSET_SPACING+draw_settings.BranchLength()/2+draw_settings.NodeLength(),child_entry->y-th);
}
// Take care of labeling the branch on the bottom
switch (draw_settings.LabelBranchBelow())
{
	case BRANCH_BELOW_NOTHING:
		label="";break;
	case BRANCH_BELOW_LABEL:
		if (child_entry->child_number!=0)
			label=the_problem->GetActionLabel(entry->n,child_entry->child_number);
		else
			label="";break;
	case BRANCH_BELOW_PLAYER:
		label=the_problem->GetPlayerLabel(entry->n[1]);
		break;
	case BRANCH_BELOW_INFOSET:
		sprintf(tempstr,"(%d,%d)",entry->n[1],entry->n[2]);label=tempstr;break;
	case BRANCH_BELOW_PROBS:
		if (entry->n[1]==0)
			label=ToString(the_problem->GetActionProb(entry->n,child_entry->child_number));
		else label="";break;
	default:
		label="";break;
}
if (label!="")
{
	dc.SetFont(draw_settings.BranchBelowFont());
	gDrawText(dc,label,entry->x+entry->nums*INFOSET_SPACING+draw_settings.BranchLength()/2+draw_settings.NodeLength(),child_entry->y+3);
}
}
// Now take care of displaying the terminal node labels
if (!child_entry->has_children)	// if the node is terminal
{
	switch (draw_settings.LabelNodeTerminal())
	{
		case NODE_TERMINAL_NOTHING:
			label="";break;
		case NODE_TERMINAL_OUTCOME:
		if (o=the_problem->GetOutcome(n))
			label=OutcomeToString(the_problem->GetOutcomeValues(o),&draw_settings);
		else
			label="";break;
		case NODE_TERMINAL_NAME:
		if (o=the_problem->GetOutcome(n))
			label=the_problem->GetOutcomeLabel(o);
		else
			label="";break;
		default:
			label="";break;
	}
	if (label!="")
	{
		dc.SetFont(draw_settings.NodeTerminalFont());
		gDrawText(dc,label,child_entry->x+draw_settings.NodeLength()+child_entry->nums*INFOSET_SPACING+10,child_entry->y-12);
	}
}

}
//**************************************************************************
//*     RENDER SUBTREE--RENDERING ROUTINE TO DRAW BRANCHES AND LABELS      *
//**************************************************************************
// The following speed optimizations have been added:
// The algorithm now traverses the tree as a linear linked list, eliminating
// expensive searches.  Since the region clipping implemented by wxwin seems
// to be less than optimal, I add rudimentary clipping of my own
template <class T>
void TreeWindow<T>::RenderSubtree(wxDC &dc)
{
int xs,xe,ys,ye;	// x-start,x-end,y-start,y-end: coordinates for drawing branches
NodeEntry *entry,*child_entry;
wxNode *n=node_list->First();
// Determine the visible region on screen to implement clipping
int x_start,y_start,width,height;
ViewStart(&x_start,&y_start);
wxWindow::GetClientSize(&width,&height);
// go through the list of nodes, plotting them
while (n)
{
	child_entry=(NodeEntry *)n->Data();
	entry=child_entry->parent;
	// Check if this node/labels are visible
	if (!(child_entry->x<x_start*PIXELS_PER_SCROLL || entry->x>x_start*PIXELS_PER_SCROLL+width/draw_settings.Zoom()
			|| (entry->y>y_start*PIXELS_PER_SCROLL+height/draw_settings.Zoom() && child_entry->y>y_start*PIXELS_PER_SCROLL+height/draw_settings.Zoom())
			|| (entry->y<y_start*PIXELS_PER_SCROLL && child_entry->y<y_start*PIXELS_PER_SCROLL)))
	{
		// draw the labels
		RenderLabels(dc,child_entry);
		// draw the 'node' line
		::DrawLine(dc,entry->x, entry->y,
				entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING,entry->y,entry->color);
		// show the infoset lines, if required by draw settings
		if (draw_settings.ShowInfosets())
			if (entry->infoset_y!=-1)
			{
				::DrawCircle(dc,entry->x+entry->num*INFOSET_SPACING,entry->y,2,entry->color);
				::DrawThinLine(dc,entry->x+entry->num*INFOSET_SPACING,entry->y,entry->x+entry->num*INFOSET_SPACING,entry->infoset_y,entry->color);
				::DrawCircle(dc,entry->x+entry->num*INFOSET_SPACING,entry->infoset_y,2,entry->color);
			}
		// Draw a circle to show the marked node
		// since we do not want to erase/redraw the screen every time we mark
		// a node, store the old_node in old_mark_node so that we can erase it.
		if (child_entry->n==old_mark_node)
			::DrawCircle(dc,child_entry->x+child_entry->nums*INFOSET_SPACING,child_entry->y,4,-1);
		if (child_entry->n==mark_node)
			::DrawCircle(dc,child_entry->x+child_entry->nums*INFOSET_SPACING,child_entry->y,4,draw_settings.CursorColor());
		// Draw a square if this node has a subgame outcome
		if (the_problem->HasSuccessorGame(child_entry->n))
			::DrawRectangle(dc,child_entry->x+draw_settings.NodeLength()+child_entry->nums*INFOSET_SPACING-4,child_entry->y-4,8,8,draw_settings.CursorColor());
		// draw the 'branches'
		if (entry!=child_entry)	// no branches for root node
	  {
			xs=entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING;
			ys=entry->y;
			xe=xs+draw_settings.BranchLength()/2;
		  ye=child_entry->y;
			::DrawLine(dc,xs,ys,xe,ye,entry->color);
			xs=xe;
			ys=ye;
			xe=child_entry->x;
		  ye=ys;
			::DrawLine(dc,xs,ye,xe,ye,entry->color);
		}
		else
		{xe=entry->x;ye=entry->y;}
		if (!child_entry->has_children)
			::DrawLine(dc,xe,ye,xe+draw_settings.NodeLength(),ye,draw_settings.GetPlayerColor(-1));
  }
	n=n->Next();
}
}            

//**************************************************************************
//*                  RENDER--THE MAIN RENDERING ROUTINE                    *
//**************************************************************************
template <class T>
void TreeWindow<T>::Render(wxDC &dc)
{
Bool need_clear=TRUE;
int width,height,x_start,y_start;
if (nodes_changed || infosets_changed)	// Recalc only if needed
{
	nodes_changed=FALSE;infosets_changed=FALSE;need_clear=TRUE;
	// Note that node_table is preserved until the next redraw
  // $$$ Check for memory dealocation!!!!
	node_list->Clear();
	maxlev = miny = maxy = 0;
	ViewStart(&x_start,&y_start);
	((wxWindow *)this)->GetClientSize(&width,&height);
	ycoord = height/2;
	FillTable(the_problem->RootNode(subgame), 0);
	if (draw_settings.ShowInfosets())
	{
		FillInfosetTable(the_problem->RootNode(subgame));
		UpdateTableInfosets();
	}
	UpdateTableParents();
}
if (dc.__type==wxTYPE_DC)	// if drawing to screen
{
	if (iterator != NULL)
	{
		NodeEntry *entry=GetNodeEntry(iterator->Cursor());
		flasher->SetFlashNode(entry->x+(entry->num+1)*INFOSET_SPACING+5,entry->y-4,
													entry->x+entry->num*INFOSET_SPACING+draw_settings.NodeLength(),entry->y-4);
	}
	if (need_clear)	dc.Clear();
	dc.BeginDrawing();
}
else
{
	flasher->SetFlashing(FALSE);
	dc.SetTextBackground(wxLIGHT_GREY);
  #ifdef wx_msw
	if (dc.__type==wxTYPE_DC_PRINTER)
	{
		width=GetDeviceCaps(dc.cdc,HORZRES);
	  height=GetDeviceCaps(dc.cdc,VERTRES);
	  float printer_scale_x=(float)width/(float)((maxlev + 1)*(draw_settings.BranchLength()+draw_settings.NodeLength()));
		float printer_scale_y=(float)height/(float)maxy;
    float printer_scale=min(printer_scale_x,printer_scale_y);
		dc.SetUserScale(printer_scale,printer_scale);
	}
  #endif
}
RenderSubtree(dc);
if (dc.__type!=wxTYPE_DC)	flasher->SetFlashing(TRUE); else dc.EndDrawing();
flasher->Flash();
}

template <class T>
void TreeWindow<T>::ProcessCursor(void)
{
// A little scrollbar magic to insure the focus stays w/ cursor.  This
// can probably be optimized much further.  Consider using SetClippingRegion
// This also makes sure that the virtual canvas is large enough for the entire
// tree (not yet implemented)

int x_start,y_start;
int width,height;
int virt_width,y_size;
int	x_steps,y_steps;
int xs,xe,ys,ye;

ViewStart(&x_start,&y_start);
GetParent()->GetClientSize(&width,&height);
x_steps=((maxlev + 1)*(draw_settings.BranchLength()+draw_settings.NodeLength()))/PIXELS_PER_SCROLL+1;
y_steps=maxy/PIXELS_PER_SCROLL+1;
if (x_steps>draw_settings.x_steps() || y_steps>draw_settings.y_steps())
{
	draw_settings.set_x_steps(max(draw_settings.x_steps(),x_steps));
	draw_settings.set_y_steps(max(draw_settings.y_steps(),y_steps));
	SetScrollbars(PIXELS_PER_SCROLL,PIXELS_PER_SCROLL,x_steps,y_steps,4,4);
}

// If cursor is active, make sure it is visible
if (iterator != NULL)
{
	NodeEntry *entry=GetNodeEntry(iterator->Cursor());
	// check if in the visible x-dimention
	xs=entry->x;
	xe=(entry->x+draw_settings.NodeLength()/2)*draw_settings.Zoom();
	if (xs<x_start*PIXELS_PER_SCROLL || xe > x_start*PIXELS_PER_SCROLL+width)
		x_start=entry->x*draw_settings.Zoom()/(PIXELS_PER_SCROLL*draw_settings.Zoom())-1;
	if (x_start<0) x_start=0;
	// check if in the visible y-dimention
	ys=entry->y;
	ye=ys;
	if (ys-10<y_start*PIXELS_PER_SCROLL || ye+20>y_start*PIXELS_PER_SCROLL+height/draw_settings.Zoom())
			y_start=(entry->y*draw_settings.Zoom()-height/2)/(PIXELS_PER_SCROLL*draw_settings.Zoom())-1;
			if (y_start<0) y_start=0;
	// now update the flasher
		flasher->SetFlashNode(entry->x+(entry->num+1)*INFOSET_SPACING+5,entry->y-4,
													entry->x+draw_settings.NodeLength()+entry->num*INFOSET_SPACING,entry->y-4);
		flasher->Flash();
}
if (x_start!=draw_settings.get_x_scroll() || y_start!=draw_settings.get_y_scroll())
{
	Scroll(x_start,y_start);
	draw_settings.set_x_scroll(x_start);
	draw_settings.set_y_scroll(y_start);
}
}

#define DELTA	8
template <class T>
void TreeWindow<T>::ProcessClick(int x,int y)
{
wxNode *n=node_list->First();
for (int i=0;i<node_list->Number();i++)
{
	 NodeEntry *entry=(NodeEntry *)n->Data();
//-------------check if clicked on a node
	 if(x>entry->x && x<entry->x+draw_settings.NodeLength() &&
			y>entry->y-DELTA && y<entry->y+DELTA)
			 {iterator->SetCursor(entry->n);return;}
//------------check if clicked on an outcome
	 if(the_problem->NumChildren(entry->n)==0 && // if this is a terminal node
			the_problem->GetOutcome(entry->n) &&	// and the outcome is set
			(draw_settings.LabelNodeTerminal()==NODE_TERMINAL_OUTCOME ||
			 draw_settings.LabelNodeTerminal()==NODE_TERMINAL_NAME))		// and the outcomes are displayed
	 {
		 if(x>entry->x+draw_settings.NodeLength()+10 &&
				x<x+draw_settings.NodeLength()+10+draw_settings.OutcomeLength() &&
				y>entry->y-DELTA && y<entry->y+DELTA)
					{tree_outcomes(the_problem->GetOutcome(entry->n));return;}
	 }
	 n=n->Next();
}
}

// ##		Note: since gcc 2.6.3, I have not been able to split a template
// definition into two files.  I get duplicate definitions of the
// class virtual table.  Thus, for now, just include the other .cc file.
#ifndef __GNUG__
	#ifdef __GNUG__
		#define TEMPLATE template
	#elif defined __BORLANDC__
		#pragma option -Jgd
		#define TEMPLATE
	#endif   // __GNUG__, __BORLANDC__
	TEMPLATE class TreeWindow<double> ;
	TEMPLATE char *OutcomeToString(const gVector<double> &v,TreeDrawParams *draw_settings,Bool color_coded=TRUE);

	#ifdef GRATIONAL
		TEMPLATE class TreeWindow<gRational> ;
		TEMPLATE char *OutcomeToString(const gVector<gRational> &v,TreeDrawParams *draw_settings,Bool color_coded=TRUE);
	#endif
#else
	// ##
	#include "treewin1.cc"
#endif
