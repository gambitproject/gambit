#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <strstream.h>
#include "wx.h"
#include "wx_form.h"
#define PROBLEM_C
#include "extform.h"
#include "outcome.h"
#include "problem.h"
#pragma hdrstop
#include "const.h"
#include "gambit.h"
#include "treewin.h"
#include "legendc.h"

// To do:
// Graphical representation of infosets				DONE
// Graphical representation of outcomes				DONE
// Accelerators																DONE
// Help system																DONE
// Pretty dialog boxes
// Fix the zoom scrolling bug
// Add save/load functionality
#define BAD_NODE Node(-1,-1,-1,-1)

#define INFOSET_SPACING	10
extern GambitFrame *gambit_frame;
extern char *gambit_color_list[GAMBIT_COLOR_LIST_LENGTH];

wxFont *outcome_font;
wxFont *fixed_font;
wxBrush *white_brush;

char tempstr[20];

gOutput& operator<<(gOutput &op,const TreeWindow::SUBGAMESTRUCT &S){return op;}

//###
//Draw Line.  A quick and dirty way of easily drawing lines w/ set color
void DrawLine(wxDC &dc,int x_s,int y_s,int x_e,int y_e,int color=0)
{
if (color>-1)
	dc.SetPen(wxThePenList->FindOrCreatePen(gambit_color_list[color],2,wxSOLID));
dc.DrawLine(x_s,y_s,x_e,y_e);
}
//###
//Draw Rectangle.  A quick and dirty way of easily drawing rectangles w/ set color
void DrawRectangle(wxDC &dc,int x_s,int y_s,int w,int h,int color=0)
{
if (color>-1)
	dc.SetPen(wxThePenList->FindOrCreatePen(gambit_color_list[color],2,wxSOLID));
dc.DrawRectangle(x_s,y_s,w,h);
}
//###
//Draw Thin Line.  A quick and dirty way of easily drawing lines w/ set color
void DrawThinLine(wxDC &dc,int x_s,int y_s,int x_e,int y_e,int color=0)
{
if (color>-1)
	dc.SetPen(wxThePenList->FindOrCreatePen(gambit_color_list[color],.5,wxSOLID));
dc.DrawLine(x_s,y_s,x_e,y_e);
}
//###
//Draw Text. A quick and dirty way of easily drawing text w/ set color
void DrawText(wxDC &dc,char *s,int x,int y,int color=0)
{
if (color>-1)
	dc.SetPen(wxThePenList->FindOrCreatePen(gambit_color_list[color],2,wxSOLID));
dc.DrawText(s,x,y);
}
//Draw Circle. A quick and dirty way of easily drawing a circle w/ set color
void DrawCircle(wxDC &dc,int x,int y,int r,int color=0)
{
	dc.SetPen(wxThePenList->FindOrCreatePen(gambit_color_list[color],2,wxSOLID));
	dc.DrawEllipse(x-r,y-r,2*r,2*r);
}



//=====================================================================
//                   EXTENSIVEFRAME MEMBER FUNCTIONS
//=====================================================================

//---------------------------------------------------------------------
//             EXTENSIVEFRAME: CONSTRUCTOR AND DESTRUCTOR
//---------------------------------------------------------------------

ExtensiveFrame::ExtensiveFrame(wxFrame *frame, char *title, int x, int y,
							int w, int h, int type,char *filename,Problem *_p,int _subgame):
	wxFrame(frame, title, x, y, w, h, type)
{
	parent=frame;
	//--------------Define all the menus----------------------------
	//Note: to insure greatest possible portability we will avoid using
	//      resource files which are inherently non-portable.
	wxMenu *file_menu=new wxMenu;
		file_menu->Append(FILE_NEW, "&New");
		file_menu->Append(FILE_LOAD,"&Load");
		file_menu->Append(FILE_SAVE,"&Save");
		file_menu->Append(FILE_PRINT,"&Print");
		file_menu->Append(FILE_CLOSE,"&Close");
		file_menu->Append(FILE_PRINT_EPS,"Print to &EPS");
		file_menu->Append(FILE_COPY_MF,"Copy to &MF");
		file_menu->AppendSeparator();
		file_menu->Append(FILE_QUIT,"&Quit");
	wxMenu *node_menu=new wxMenu;
		node_menu->Append(NODE_ADD, "&Add");
		node_menu->Append(NODE_OUTCOME, "&Outcome");
		node_menu->Append(NODE_DELETE, "&Delete");
		node_menu->Append(NODE_INSERT, "&Insert");
		node_menu->Append(NODE_LABEL, "&Label");
		node_menu->Append(NODE_PROBS, "&Probs");
		node_menu->AppendSeparator();
		node_menu->Append(NODE_SET_MARK, "Set &Mark");
		node_menu->Append(NODE_GOTO_MARK, "Go&to Mark");
	wxMenu *branch_menu=new wxMenu;
		branch_menu->Append(BRANCH_DELETE, "&Delete");
		branch_menu->Append(BRANCH_INSERT, "&Insert");
		branch_menu->Append(BRANCH_LABEL, "&Label");
	wxMenu *tree_menu=new wxMenu;
		tree_menu->Append(TREE_COPY, "&Copy");
		tree_menu->Append(TREE_MOVE, "&Move");
		tree_menu->Append(TREE_DELETE, "&Delete");
		tree_menu->Append(TREE_LABEL, "&Label");
		tree_menu->Append(TREE_OUTCOMES, "&Outcomes");
		tree_menu->Append(TREE_PLAYERS, "&Players");
		tree_menu->Append(TREE_SUBGAMES,"&Subgames");
	wxMenu *infoset_menu=new wxMenu;
		infoset_menu->Append(INFOSET_MERGE, "&Merge");
		infoset_menu->Append(INFOSET_BREAK, "&Break");
		infoset_menu->Append(INFOSET_JOIN, "&Join");
	wxMenu *solve_menu=new wxMenu;
		solve_menu->Append(SOLVE_SET, "&Set");
		solve_menu->Append(SOLVE_GO, "&Go");
	wxMenu *display_menu=new wxMenu;
		display_menu->Append(DISPLAY_SET_ZOOM,"&Zoom...");
		display_menu->Append(DISPLAY_SET_OPTIONS,"&Options...");
		display_menu->Append(DISPLAY_LEGENDS,"&Legend...");
		display_menu->AppendSeparator();
		display_menu->Append(DISPLAY_SAVE_DEFAULT,"Save Default");
		display_menu->Append(DISPLAY_LOAD_DEFAULT,"Load Default");
		display_menu->Append(DISPLAY_SAVE_CUSTOM,"Save Custom");
		display_menu->Append(DISPLAY_LOAD_CUSTOM,"Load Custom");
	wxMenu *help_menu = new wxMenu;
		help_menu->Append(HELP_GAMBIT,"&Contents",					"Table of contents");
		help_menu->Append(HELP_ABOUT,"&About",							"About this program");

	wxMenuBar *menu_bar=new wxMenuBar;
	menu_bar->Append(file_menu,"&File");
	menu_bar->Append(node_menu,"&Node");
	menu_bar->Append(branch_menu,"&Branch");
	menu_bar->Append(tree_menu,"&Tree");
	menu_bar->Append(infoset_menu,"&Infoset");
	menu_bar->Append(solve_menu,"&Solve");
	menu_bar->Append(display_menu,"&Display");
	menu_bar->Append(help_menu,	"&Help");

	SetMenuBar(menu_bar);
	// Create the accelerators (to add an accelerator, see const.h)
	accelerators=MakeAccelerators();
	// Load the tree from a datafile if necessary
	Problem *p;
	if (_p==NULL)	// if starting from scratch
	{
		if (filename!=NULL)
		{
			p=new Problem;
			p->ReadEfgFile(filename);
			SetTitle(filename);
		}
		else
		{
			p=NULL;
			SetTitle("Untitled");
		}
	}
	else			// if starting from an existing problem, probably a subgame
	{
		p=_p;
		gString tmp=p->GetTitle();
		if (_subgame>1) tmp=tmp+':'+p->GetGameName(_subgame);
		SetTitle(tmp);
	}
	// Create the canvas(TreeWindow) on which to draw the tree
	tw = new TreeWindow(this,0,0,w,h,p,_subgame);
	// Give it scrollbars
	tw->SetScrollbars(20, 20, 60, 60, 4, 4);
}

ExtensiveFrame::~ExtensiveFrame()
{
	delete tw;
	delete accelerators;
}

Bool ExtensiveFrame::OnClose(void)
{
	return TRUE;
}
//---------------------------------------------------------------------
//             EXTENSIVEFRAME: EVENT-HANDLING HOOK MEMBERS
//---------------------------------------------------------------------

void ExtensiveFrame::OnMenuCommand(int id)
{
	switch (id)
	{
    case NODE_ADD:
			tw->node_add();
      break;
    case NODE_OUTCOME:
      tw->node_outcome();
			break;
    case NODE_DELETE:
      tw->node_delete();
			break;
    case NODE_INSERT:
      tw->node_insert();
			break;
    case NODE_LABEL:
      tw->node_label();
			break;
    case NODE_SET_MARK:
      tw->node_set_mark();
			break;
    case NODE_GOTO_MARK:
      tw->node_goto_mark();
			break;
		case NODE_PROBS:
			tw->node_probs();
			break;

    case BRANCH_DELETE:
      tw->branch_delete();
			break;
    case BRANCH_INSERT:
      tw->branch_insert();
			break;
		case BRANCH_LABEL:
      tw->branch_label();
			break;

    case TREE_DELETE:
      tw->tree_delete();
			break;
    case TREE_COPY:
      tw->tree_copy();
			break;
		case TREE_MOVE:
			tw->tree_move();
			break;
    case TREE_LABEL:
      tw->tree_label();
      SetTitle(tw->Title());
      break;
    case TREE_OUTCOMES:
      tw->tree_outcomes();
			break;
		case TREE_PLAYERS:
			tw->tree_players();
			break;
		case TREE_SUBGAMES:
			tw->tree_subgames();
			break;

		case INFOSET_MERGE:
      tw->infoset_merge();
			break;
    case INFOSET_BREAK:
      tw->infoset_break();
			break;
    case INFOSET_JOIN:
      tw->infoset_join();
			break;

		case FILE_PRINT_EPS:
			tw->print_eps();
			break;
#ifdef wx_msw
		case FILE_PRINT:
			tw->print();
			break;
		case FILE_COPY_MF:
			tw->print_mf();
			break;
#endif
		case FILE_SAVE:
			tw->file_save();
			break;
		case FILE_CLOSE:
			delete this;
			break;

		case DISPLAY_SET_ZOOM:
			#define ZOOM_DELTA	.2
			tw->display_set_zoom();
			break;
		case DISPLAY_INC_ZOOM:
			tw->display_set_zoom(tw->display_get_zoom()+ZOOM_DELTA);
			break;
		case DISPLAY_DEC_ZOOM:
			tw->display_set_zoom(tw->display_get_zoom()-ZOOM_DELTA);
			break;
		case DISPLAY_LEGENDS:
			tw->display_legends();
			break;
		case DISPLAY_SET_OPTIONS:
			tw->display_options();
			break;
		case DISPLAY_SAVE_DEFAULT:
			tw->display_save_options();
			break;
		case DISPLAY_LOAD_DEFAULT:
			tw->display_load_options();
      break;
		case DISPLAY_SAVE_CUSTOM:
			tw->display_save_options(FALSE);
			break;
		case DISPLAY_LOAD_CUSTOM:
			tw->display_load_options(FALSE);
      break;
		default:
			gambit_frame->OnMenuCommand(id);
			break;
	}
	if (id!=FILE_QUIT && id!=FILE_CLOSE) tw->OnPaint();
}

//=====================================================================
//                      TREEWINDOW MEMBER FUNCTIONS
//=====================================================================

//---------------------------------------------------------------------
//                TREEWINDOW: CONSTRUCTOR AND DESTRUCTOR
//---------------------------------------------------------------------

TreeWindow::TreeWindow(ExtensiveFrame *_frame,int x,int y,int w,int h,Problem *p,int _subgame,int style):
								wxCanvas(_frame, x, y, w, h, style)
{
	frame=_frame;				// store the parent frame
	if (p) the_problem = p; else the_problem=new Problem;	// create the problem
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
	mark_node=BAD_NODE;

	outcome_font=wxTheFontList->FindOrCreateFont(9,wxSWISS,wxNORMAL,wxNORMAL);
	fixed_font=wxTheFontList->FindOrCreateFont(12,wxMODERN,wxNORMAL,wxNORMAL);
	white_brush=wxTheBrushList->FindOrCreateBrush("WHITE",wxSOLID);
	GetDC()->SetBackgroundMode(wxTRANSPARENT);
}

TreeWindow::~TreeWindow()
{
	if (subgame!=1)		// if I am a subgame, notify parent of my death
	 ((ExtensiveFrame *)frame->parent)->tw->CloseSubgame(subgame);
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

void TreeWindow::OnPaint(void)
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
void TreeWindow::OnChar(wxKeyEvent& ch)
{
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
void TreeWindow::OnEvent(wxMouseEvent& event)
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
#define PIXELS_PER_SCROLL	20
NodeEntry *TreeWindow::GetNodeEntry(const Node &n)
{
for (int i=0;i<node_list->Number();i++)
{
	if (((NodeEntry *)node_list->Nth(i)->Data())->n==n)
		return ((NodeEntry *)node_list->Nth(i)->Data());
}
return NULL;
}

int maxlev, maxy, miny, ycoord;

int TreeWindow::FillTable(const Node &n, int level)
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
void TreeWindow::CheckInfosetEntry(NodeEntry *e)
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
NodeEntry *TreeWindow::NextInfoset(NodeEntry *e)
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

void TreeWindow::FillInfosetTable(const Node &n)
{
NodeEntry *entry=GetNodeEntry(n);
if (the_problem->NumChildren(n) > 0)
	for (int i = 1; i <= the_problem->NumChildren(n); i++)
		FillInfosetTable(the_problem->GetChildNumber(n,i));
CheckInfosetEntry(entry);
}

void TreeWindow::UpdateTableInfosets(void)
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

// RenderLabels.  Draws all the text labels for the tree according to the
// settings in draw_settings.  Currently takes care of:labels node/branch,
// outcomes.  Note: this function is getting damn long, but I see no real
// reason to split it at this point...
char *gTupleToString(const gTuple<gNumber> &v)
{
static gString gvts;
gvts="";
for (int i=v.First();i<=v.Last();i++)
{
 	gvts+=(i==1) ? '(' : ',';
	sprintf(tempstr,"%2.2f",(double)v[i]);
	gvts+=tempstr;
}
gvts+=')';

return (char *)gvts;
}


void TreeWindow::RenderLabels(wxDC &dc,const Node &n)
{
gString 		label;		// temporary to hold the label 
float tw,th;
NodeEntry 	*entry=GetNodeEntry(n);
// First take care of labeling the node on top
switch (draw_settings.LabelNodeAbove())
{
	case NODE_ABOVE_NOTHING:
  	label="";
		break;
	case NODE_ABOVE_LABEL:
  	label=the_problem->GetNodeLabel(n);
		break;
	case NODE_ABOVE_PLAYER:
		label=the_problem->GetPlayerName(n[1]);
		break;
	case NODE_ABOVE_INFOSET:
		sprintf(tempstr,"(%d,%d)",n[1],n[2]);
		label=tempstr;
		break;
	case NODE_ABOVE_OUTCOME:
		if (the_problem->IsOutcomeDefined(the_problem->GetOutcome(n)))
			label=gTupleToString(the_problem->GetOutcomeValues(the_problem->GetOutcome(n)));
		else
    	label="";
		break;
	default:
		label="Undef";
		break;
}
if (label!="")
{
	dc.SetFont(draw_settings.NodeAboveFont());
	dc.GetTextExtent("0",&tw,&th);
	::DrawText(dc,label,entry->x+entry->nums*INFOSET_SPACING,entry->y-th-5);
}
// Take care of labeling the node on the bottom
switch (draw_settings.LabelNodeBelow())
{
	case NODE_BELOW_NOTHING:
  	label="";
		break;
	case NODE_BELOW_LABEL:
  	label=the_problem->GetNodeLabel(n);
		break;
	case NODE_BELOW_PLAYER:
		label=the_problem->GetPlayerName(n[1]);
		break;
	case NODE_BELOW_INFOSET:
		sprintf(tempstr,"(%d,%d)",n[1],n[2]);
		label=tempstr;
		break;
	case NODE_BELOW_OUTCOME:
		if (the_problem->IsOutcomeDefined(the_problem->GetOutcome(n)))
			label=gTupleToString(the_problem->GetOutcomeValues(the_problem->GetOutcome(n)));
		else
			label="";
		break;
	default:
		label="Undef";
		break;
}
if (label!="")
{
	dc.SetFont(draw_settings.NodeBelowFont());
	::DrawText(dc,label,entry->x+entry->nums*INFOSET_SPACING,entry->y+3);
}
// Now take care of branches....
Node child_node;NodeEntry *child_entry;
int	i;
// Take care of labeling the branch on the top
// have to label each branch on this node
for (i=1;i<=the_problem->NumChildren(n);i++)
{
	switch (draw_settings.LabelBranchAbove())
	{
		case BRANCH_ABOVE_NOTHING:
			label="";
			break;
		case BRANCH_ABOVE_LABEL:
      if (the_problem->NumChildren(n)!=0)
				label=the_problem->GetActionLabel(n,i);
			else
				label="";
			break;
		case BRANCH_ABOVE_PLAYER:
			label=the_problem->GetPlayerName(n[1]);
			break;
		case BRANCH_ABOVE_INFOSET:
			sprintf(tempstr,"(%d,%d)",n[1],n[2]);
			label=tempstr;
			break;
		case BRANCH_ABOVE_PROBS:
			if (n[1]==0)	// if this is in fact a chance player ....
			{
				sprintf(tempstr,"%2.2f",(double)the_problem->GetActionProb(n,i));
				label=tempstr;
			}
			else
				label="";
      break;
		default:
			label="Undef";
			break;
	}
	child_node=the_problem->GetChildNumber(n,i);
	child_entry=GetNodeEntry(child_node);
	if (label!="")
  {
		dc.SetFont(draw_settings.BranchAboveFont());
		dc.GetTextExtent("0",&tw,&th);
		::DrawText(dc,label,entry->x+entry->nums*INFOSET_SPACING+draw_settings.BranchLength()/2+draw_settings.NodeLength(),child_entry->y-th);
  }
}
// Take care of labeling the branch on the bottom
	// have to label each branch on this node
for (i=1;i<=the_problem->NumChildren(n);i++)
{
	switch (draw_settings.LabelBranchBelow())
	{
		case BRANCH_BELOW_NOTHING:
			label="";
			break;
		case BRANCH_BELOW_LABEL:
			if (the_problem->NumChildren(n)!=0)
				label=the_problem->GetActionLabel(n,i);
			else
				label="";
			break;
		case BRANCH_BELOW_PLAYER:
			label=the_problem->GetPlayerName(n[1]);
			break;
		case BRANCH_BELOW_INFOSET:
			sprintf(tempstr,"(%d,%d)",n[1],n[2]);
			label=tempstr;
			break;
		case BRANCH_BELOW_PROBS:
			if (n[1]==0)	// if this is in fact a chance player ....
			{
				sprintf(tempstr,"%2.2f",(double)the_problem->GetActionProb(n,i));
				label=tempstr;
			}
			else
				label="";
      break;
		default:
			label="Undef";
			break;
	}
	child_node=the_problem->GetChildNumber(n,i);
	child_entry=GetNodeEntry(child_node);
	if (label!="")
  {
		dc.SetFont(draw_settings.BranchBelowFont());
		::DrawText(dc,label,entry->x+entry->nums*INFOSET_SPACING+draw_settings.BranchLength()/2+draw_settings.NodeLength(),child_entry->y+3);
  }
}
// Now take care of displaying the terminal node labels
if (the_problem->NumChildren(n)==0)	// if the node is terminal
{
	switch (draw_settings.LabelNodeTerminal())
	{
		case NODE_TERMINAL_NOTHING:
			label="";
      break;
		case NODE_TERMINAL_OUTCOME:
			if (the_problem->IsOutcomeDefined(the_problem->GetOutcome(n)))
				label=gTupleToString(the_problem->GetOutcomeValues(the_problem->GetOutcome(n)));
			else
				label="";
			break;
		case NODE_TERMINAL_NAME:
			label=the_problem->GetOutcomeName(the_problem->GetOutcome(n));
			break;
		default:
			label="Undef";
			break;
	}
	if (label!="")
  {
		dc.SetFont(draw_settings.NodeTerminalFont());
		::DrawText(dc,label,entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING+10,entry->y-12,entry->color);
  }
}

}

void TreeWindow::RenderSubtree(wxDC &dc,const Node &n)
{
	int xs,xe,ys,ye;	// x-start,x-end,y-start,y-end: coordinates for drawing branches
	NodeEntry *entry=GetNodeEntry(n);

  // draw the 'node' line
	::DrawLine(dc,entry->x, entry->y,
			entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING,entry->y,entry->color);
  // draw the labels
	RenderLabels(dc,n);
  // show the infoset lines, if required by draw settings
	if (draw_settings.ShowInfosets())
		if (entry->infoset_y!=-1)
		{
			::DrawCircle(dc,entry->x+entry->num*INFOSET_SPACING,entry->y,2,entry->color);
			::DrawThinLine(dc,entry->x+entry->num*INFOSET_SPACING,entry->y,entry->x+entry->num*INFOSET_SPACING,entry->infoset_y,entry->color);
			::DrawCircle(dc,entry->x+entry->num*INFOSET_SPACING,entry->infoset_y,2,entry->color);
		}
	// Draw a circle to show the marked node
	if (n==mark_node)
		::DrawCircle(dc,entry->x+entry->nums*INFOSET_SPACING,entry->y,4,draw_settings.CursorColor());
	// Draw a square if this node has a subgame outcome
	if (the_problem->HasSuccessorGame(n))
		::DrawRectangle(dc,entry->x+draw_settings.NodeLength()+entry->nums*INFOSET_SPACING-4,entry->y-4,8,8,draw_settings.CursorColor());
	// draw the 'branches'
	for (int i = 1;i<=the_problem->NumChildren(n);i++)
	{
  	Node child_node=the_problem->GetChildNumber(n,i);
		NodeEntry *child_entry=GetNodeEntry(child_node);
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
		RenderSubtree(dc,child_node);
  }
}
//---------------------------------$$$
void DumpNodeList(wxList *node_list)
{
FILE *f=fopen("list.dmp","w");
NodeEntry *e;
fprintf(f,"Length=%d\n",node_list->Number());
wxNode *n=node_list->First();
int i=0;
while (n)
{
	e=(NodeEntry *)n->Data();	
	fprintf(f,"Node %d={%d,%d,%d,%d}\n",i,(e->n)[0],(e->n)[1],(e->n)[2],(e->n)[3]);
	i++;
  n=n->Next();
}
fprintf(f,"\n");
fclose(f);
}


void TreeWindow::Render(wxDC &dc)
{
	// Note that node_table is preserved until the next redraw
  // $$$ Check for memory dealocation!!!!
	node_list->Clear();
	maxlev = miny = maxy = 0;
  int width,height,x_start,y_start;
	ViewStart(&x_start,&y_start);
	((wxWindow *)this)->GetClientSize(&width,&height);
	ycoord = height/2;

	FillTable(the_problem->RootNode(subgame), 0);
	if (draw_settings.ShowInfosets())
	{
		FillInfosetTable(the_problem->RootNode(subgame));
		UpdateTableInfosets();
	}
	if (dc.__type==wxTYPE_DC)	// if drawing to screen
  {
		if (iterator != NULL)
		{
			NodeEntry *entry=GetNodeEntry(iterator->Cursor());
			if (entry==NULL)
			{
				wxDebugMsg("Entry==NULL @ node %d,%d,%d,%d",
					iterator->Cursor()[0],iterator->Cursor()[1],iterator->Cursor()[2],
					iterator->Cursor()[3]);
				DumpNodeList(node_list);
				iterator->SetCursor(the_problem->RootNode(subgame));
				wxMessageBox("Error: Entry=NULL!");
				NodeEntry *entry=GetNodeEntry(iterator->Cursor());
			}
			flasher->SetFlashNode(entry->x+(entry->num+1)*INFOSET_SPACING+5,entry->y-4,
													entry->x+entry->num*INFOSET_SPACING+draw_settings.NodeLength(),entry->y-4);
		}
			dc.Clear();
			dc.SetClippingRegion(x_start*PIXELS_PER_SCROLL,y_start*PIXELS_PER_SCROLL,width,height);
			dc.BeginDrawing();
	}
	else
	{
		flasher->SetFlashing(FALSE);
//		dc.SetBackground(white_brush);
//    dc.Clear();
			dc.SetTextBackground(wxLIGHT_GREY);
	}
	RenderSubtree(dc,the_problem->RootNode(subgame));
	if (dc.__type!=wxTYPE_DC)
		flasher->SetFlashing(TRUE);
	else
	{
		dc.EndDrawing();
		dc.DestroyClippingRegion();
	}
  flasher->Flash();
}

void TreeWindow::ProcessCursor(void)
{
// A little scrollbar magic to insure the focus stays w/ cursor.  This
// can probably be optimized much further.  Consider using SetClippingRegion
// This also makes sure that the virtual canvas is large enough for the entire
// tree (not yet implemented)

int x_start,y_start;
int width,height;
int virt_width,y_size;
int	x_scroll,y_scroll;
int xs,xe,ys,ye;

ViewStart(&x_start,&y_start);
GetParent()->GetClientSize(&width,&height);
x_scroll=((maxlev + 1) * (draw_settings.BranchLength() + draw_settings.NodeLength()))/PIXELS_PER_SCROLL+1;
y_scroll=maxy/PIXELS_PER_SCROLL+1;

// If cursor is active, make sure it is visible
if (iterator != NULL)
{
	NodeEntry *entry=GetNodeEntry(iterator->Cursor());
	// check if in the visible x-dimention
	xs=(entry->x)*draw_settings.Zoom();
	xe=(entry->x+draw_settings.NodeLength()/2)*draw_settings.Zoom();
	if (xs<x_start*PIXELS_PER_SCROLL*draw_settings.Zoom() || xe > x_start*PIXELS_PER_SCROLL+width)
		x_start=entry->x*draw_settings.Zoom()/(PIXELS_PER_SCROLL*draw_settings.Zoom())-1;
	if (x_start<0) x_start=0;
	// check if in the visible y-dimention
	ys=(entry->y)*draw_settings.Zoom();
	ye=ys;
	if (ys<=y_start*PIXELS_PER_SCROLL*draw_settings.Zoom() || ye>y_start*PIXELS_PER_SCROLL+height*3/4)
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
void TreeWindow::ProcessClick(int x,int y)
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
			the_problem->IsOutcomeDefined(the_problem->GetOutcome(entry->n)) &&	// and the outcome is set
			draw_settings.LabelNodeTerminal()==NODE_TERMINAL_OUTCOME)		// and the outcomes are displayed
	 { 
		 if(x>entry->x+draw_settings.NodeLength()+10 &&
		 		x<x+draw_settings.NodeLength()+10+draw_settings.OutcomeLength() &&
				y>entry->y-DELTA && y<entry->y+DELTA)
					{tree_outcomes(the_problem->GetOutcome(entry->n));return;}
   }
	 n=n->Next();
}
}

wxList *ExtensiveFrame::MakeAccelerators(void)
{
	wxList *t=new wxList(wxKEY_INTEGER);
	int i=0;
	while (Accels[i].code!=-1)
	{
		t->Append(Accels[i].key,(wxObject *)(&Accels[i]));
		i++;
  }
  return t;
}

Bool ExtensiveFrame::CheckAccelerators(int ch)
{
	wxNode *n=accelerators->Find(ch);
	if (n)
	{
		OnMenuCommand(((Accel *)n->Data())->code);
		return TRUE;
	}
	else
		return FALSE;
}
