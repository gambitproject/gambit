//
// FILE: treewin.cc -- Implementation of extensive form windows
//
// @(#)treewin.cc	1.3 4/7/94
//
#define MAX_LABEL_LENGTH	25
#include <stdlib.h>
#include <string.h>
#include "wx.h"
#include "wx_form.h"
#include "wxmisc.h"
#include "problem.h"
#include "const.h"
#pragma hdrstop
#include "display.h"
#include "gambit.h"
#include "treewin.h"


#define BAD_NODE Node(-1,-1,-1,-1)
extern GambitFrame *gambit_frame;

wxFont *label_font;
wxFont *fixed_font;
wxList *gambit_color_list;

//###
//GetPlayerColor. This should be put into some class--just need to decide
//where.  The current solution of color_table in treewin class is not to nice
char *GetPlayerColor(int player_num)
{
return (char *)gambit_color_list->Nth(player_num%gambit_color_list->Number())->Data();
/*
#define MAXPLAYERCOLORS	20
static	char *player_colors[MAXPLAYERCOLORS]={"RED","BLUE","GREEN","YELLOW","VIOLET","SALMON","ORCHID",
												"TURQUOISE","PALE GREEN","BROWN","RED","BLUE","GREEN","YELLOW","VIOLET","SALMON","ORCHID",
												"TURQUOISE","PALE GREEN","BROWN"}; // not pretty
return player_colors[player_num%MAXPLAYERCOLORS];
*/
}

//###
//Draw Line.  A quick and dirty way of easily drawing lines w/ set color
void DrawLine(wxDC &dc,int x_s,int y_s,int x_e,int y_e,int color=-1)
{
if (color>-1)
	dc.SetPen(wxThePenList->FindOrCreatePen(GetPlayerColor(color),2,wxSOLID));
dc.DrawLine(x_s,y_s,x_e,y_e);
}
//###
//Draw Text. A quick and dirty way of easily drawing text w/ set color
void DrawText(wxDC &dc,char *s,int x,int y,int color=-1)
{
if (color>-1)
	dc.SetPen(wxThePenList->FindOrCreatePen(GetPlayerColor(color),2,wxSOLID));
dc.DrawText(s,x,y);
}




//=====================================================================
//                   EXTENSIVEFRAME MEMBER FUNCTIONS
//=====================================================================

//---------------------------------------------------------------------
//             EXTENSIVEFRAME: CONSTRUCTOR AND DESTRUCTOR
//---------------------------------------------------------------------

ExtensiveFrame::ExtensiveFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int type, Problem *p):
	wxFrame(frame, title, x, y, w, h, type)
{
	//--------------Define all the menus----------------------------
	//Note: to insure greatest possible portability we will avoid using
	//      resource files which are inherently non-portable.
	wxMenu *file_menu=new wxMenu;
		file_menu->Append(FILE_NEW, "&New");
		file_menu->Append(FILE_LOAD,"&Load");
		file_menu->Append(FILE_SAVE,"&Save");
		file_menu->Append(FILE_PRINT,"&Print");
    file_menu->Append(FILE_PRINT_EPS,"Print to &EPS");
		file_menu->Append(FILE_COPY_MF,"&Copy to MF");
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
		tree_menu->Append(TREE_DELETE, "&Delete");
		tree_menu->Append(TREE_COPY, "&Copy");
		tree_menu->Append(TREE_LABEL, "&Label");
		tree_menu->Append(TREE_OUTCOMES, "&Outcomes");
    tree_menu->Append(TREE_PLAYERS, "&Players");
	wxMenu *infoset_menu=new wxMenu;
		infoset_menu->Append(INFOSET_MEMBER, "&Member");
		infoset_menu->Append(INFOSET_BREAK, "&Break");
		infoset_menu->Append(INFOSET_JOIN, "&Join");
	wxMenu *solve_menu=new wxMenu;
		solve_menu->Append(SOLVE_SET, "&Set");
		solve_menu->Append(SOLVE_GO, "&Go");
	wxMenu *display_menu=new wxMenu;
		display_menu->Append(DISPLAY_SET_ZOOM,"&Zoom...");
		display_menu->Append(DISPLAY_SET_OPTIONS,"&Options...");

	wxMenuBar *menu_bar=new wxMenuBar;
	menu_bar->Append(file_menu,"&File");
	menu_bar->Append(node_menu,"&Node");
	menu_bar->Append(branch_menu,"&Branch");
	menu_bar->Append(tree_menu,"&Tree");
	menu_bar->Append(infoset_menu,"&Infoset");
	menu_bar->Append(solve_menu,"&Solve");
  menu_bar->Append(display_menu,"&Display");
	this->SetMenuBar(menu_bar);

  // Create the canvas(TreeWindow) on which to draw the tree
	tw = new TreeWindow(this, 0, 0, w, h,p);
	// Give it scrollbars
  tw->SetScrollbars(20, 20, 60, 60, 4, 4);

  if (p != 0)
    SetTitle(p->GetTitle().stradr());
  else
		SetTitle("Untitled");

	this->Show(TRUE);

}

ExtensiveFrame::~ExtensiveFrame()
{
  delete tw;
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
			tw->OnPaint();
      break;
    case NODE_OUTCOME:
      tw->node_outcome();
			tw->OnPaint();
			break;
    case NODE_DELETE:
      tw->node_delete();
			tw->OnPaint();
			break;
    case NODE_INSERT:
      tw->node_insert();
			tw->OnPaint();
			break;
    case NODE_LABEL:
      tw->node_label();
			tw->OnPaint();
			break;
    case NODE_SET_MARK:
      tw->node_set_mark();
			tw->OnPaint();
			break;
    case NODE_GOTO_MARK:
      tw->node_goto_mark();
			tw->OnPaint();
			break;

    case BRANCH_DELETE:
      tw->branch_delete();
			tw->OnPaint();
			break;
    case BRANCH_INSERT:
      tw->branch_insert();
			tw->OnPaint();
			break;
    case BRANCH_LABEL:
      tw->branch_label();
			tw->OnPaint();
			break;

    case TREE_DELETE:
      tw->tree_delete();
			tw->OnPaint();
			break;
    case TREE_COPY:
      tw->tree_copy();
			tw->OnPaint();
			break;
    case TREE_LABEL:
      tw->tree_label();
			tw->OnPaint();
      SetTitle(tw->Title().stradr());
      break;
    case TREE_OUTCOMES:
      tw->tree_outcomes();
			tw->OnPaint();
			break;
		case TREE_PLAYERS:
			tw->tree_players();
			tw->OnPaint();
      break;

    case INFOSET_MEMBER:
      tw->infoset_member();
			tw->OnPaint();
			break;
    case INFOSET_BREAK:
      tw->infoset_break();
			tw->OnPaint();
			break;
    case INFOSET_JOIN:
      tw->infoset_join();
			tw->OnPaint();
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
	case DISPLAY_SET_ZOOM:
		tw->SetZoom();
		tw->OnPaint();
		break;
	case DISPLAY_SET_OPTIONS:
		wxDialogBox *form_dialog = new wxDialogBox(this, "Edit Draw Settings", TRUE);
    DisplayOptionsForm *display_options_form = new DisplayOptionsForm;
		display_options_form->EditForm(tw,form_dialog);
    form_dialog->Fit();
		form_dialog->Show(TRUE);
		tw->OnPaint();
		break;
	default:
		gambit_frame->OnMenuCommand(id);
    break;
	}
}

//=====================================================================
//                      TREEWINDOW MEMBER FUNCTIONS
//=====================================================================

//---------------------------------------------------------------------
//                TREEWINDOW: CONSTRUCTOR AND DESTRUCTOR
//---------------------------------------------------------------------

TreeWindow::TreeWindow(wxFrame *_frame,int x,int y,int w,int h,Problem *p,int style):
								wxCanvas(_frame, x, y, w, h, style)
{
	frame=_frame;				// store the parent frame

  if (p)
    the_problem = p;
  else
    the_problem = new Problem;	// create the problem
// $$$
	Node r=the_problem->RootNode();
	the_problem->AddNode(r, 1, 3);

	node_list = new wxList;
// $$$  node_list->DeleteContents(TRUE);	// make sure the client data is also deleted
	zoom_factor=1.0;
	draw_settings.SetWindow(gRect(0,0,w,h));

	iterator=new TreeWinIter(the_problem);
	flasher=new TreeNodeFlasher(GetDC());
	mark_node=BAD_NODE;

	label_font=new wxFont(10,wxSWISS,wxNORMAL,wxNORMAL);
	fixed_font=new wxFont(12,wxMODERN,wxNORMAL,wxBOLD);
	GetDC()->SetBackgroundMode(wxTRANSPARENT);
	// ###
	gambit_color_list=new wxList((wxObject *)"TURQUOISE",(wxObject *)"RED",(wxObject *)"BLUE",
	(wxObject *)"GREEN",(wxObject *)"YELLOW",(wxObject *)"VIOLET",(wxObject *)"SALMON",(wxObject *)"ORCHID",
												(wxObject *)"PALE GREEN",(wxObject *)"BROWN",NULL);
}

TreeWindow::~TreeWindow()
{
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
			iterator->GoFirstChild();
			ProcessCursor();
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
int ColorNum(char *clr)
{
for (int i=0;i<gambit_color_list->Number();i++)
	if (strcmp(clr,(char *)gambit_color_list->Nth(i)->Data())==0)
		return i;
return -1;
}

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

int TreeWindow::FillTable(wxList *node_list,const Node &n, int level)
{
  int myy, y1, yn;

	NodeEntry *entry=new NodeEntry;
  entry->n=n;	// store the node the entry is for
	node_list->Append((wxObject *)entry);
	if (the_problem->NumChildren(n) > 0)
	{
		for (int i = 1; i <= the_problem->NumChildren(n); i++)
		{
			yn = FillTable(node_list, the_problem->GetChildNumber(n,i), level + 1);
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
	entry->x = level * (draw_settings.NodeLength() + draw_settings.BranchLength());

	if (n[1]>0)		// if the player is a regular player
		entry->color = n[1] % 16;
	else
	{ 
		if (n[1]==0)	// if the player is chance
			entry->color = ::ColorNum(draw_settings.ChanceColor());
		if (n[1]==-1)// if the player is dummy
			entry->color = ::ColorNum(draw_settings.ChanceColor());
	}
	if (level > maxlev)    maxlev = level;

	if (entry->y > maxy)
		maxy = entry->y;
	else
		if (entry->y < miny)   miny = entry->y;

  return entry->y;
}


void TreeWindow::RenderSubtree(wxDC &dc,const Node &n, wxList *node_list)
{
	int xs,xe,ys,ye;	// x-start,x-end,y-start,y-end: coordinates for drawing branches
	NodeEntry *entry=GetNodeEntry(n);
	dc.SetFont(label_font);

	::DrawLine(dc,entry->x, entry->y,
			entry->x + draw_settings.NodeLength(), entry->y,entry->color);

	if (n==mark_node)
		::DrawLine(dc,entry->x-4, entry->y+4,
				entry->x + draw_settings.NodeLength(), entry->y+4,::ColorNum(draw_settings.CursorColor()));

  if (draw_settings.ShowLabels())
		::DrawText(dc,the_problem->GetNodeLabel(n).stradr(),entry->x, entry->y+3, entry->color);

	for (int i = 1;i<=the_problem->NumChildren(n);i++)
	{
  	Node child_node=the_problem->GetChildNumber(n,i);
		NodeEntry *child_entry=GetNodeEntry(child_node);
		xs=entry->x+draw_settings.NodeLength();
		ys=entry->y;
		xe=xs+draw_settings.BranchLength()/2;
    ye=child_entry->y;
		::DrawLine(dc,xs,ys,xe,ye,entry->color);
		xs=xe;
		ys=ye;
		xe=child_entry->x;
    ye=ys;
		::DrawLine(dc,xs,ye,xe,ye);
		RenderSubtree(dc,child_node,node_list);
  }
}

void TreeWindow::Render(wxDC &dc)
{
	// Note that node_table is preserved until the next redraw
  // $$$ Check for memory dealocation!!!!
	node_list->Clear();
  maxlev = miny = maxy = 0;
	ycoord = draw_settings.Window().height() / 2;

  FillTable(node_list, the_problem->RootNode(), 0);
	if (dc.__type==wxTYPE_DC)	// if drawing to screen
  {
		if (iterator != NULL)
		{
			NodeEntry *entry=GetNodeEntry(iterator->Cursor());
			flasher->SetFlashNode(entry->x-4,entry->y-4,
													entry->x+draw_settings.NodeLength(),entry->y-4);
		}
  }
	else
	{
		flasher->SetFlashing(FALSE);
	}
	dc.Clear();
	dc.BeginDrawing();
	RenderSubtree(dc,the_problem->RootNode(), node_list);
	if (dc.__type!=wxTYPE_DC) flasher->SetFlashing(TRUE);
  dc.EndDrawing();
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
#define PIXELS_PER_SCROLL	20

ViewStart(&x_start,&y_start);
GetParent()->GetClientSize(&width,&height);
x_scroll=((maxlev + 1) * (draw_settings.BranchLength() + draw_settings.NodeLength()))/PIXELS_PER_SCROLL+1;
y_scroll=maxy/PIXELS_PER_SCROLL+1;

// If cursor is active, make sure it is visible
if (iterator != NULL)
{
	NodeEntry *entry=GetNodeEntry(iterator->Cursor());
	// check if in the visible x-dimention
	xs=(entry->x-draw_settings.XOrigin())*zoom_factor;
	xe=(entry->x-draw_settings.XOrigin()+draw_settings.NodeLength()/2)*zoom_factor;
	if (xs<x_start*PIXELS_PER_SCROLL || xe > x_start*PIXELS_PER_SCROLL+width)
		x_start=entry->x*zoom_factor/(PIXELS_PER_SCROLL*zoom_factor)-1;
	if (x_start<0) x_start=0;
	// check if in the visible y-dimention
	ys=(entry->y-draw_settings.YOrigin())*zoom_factor;
	ye=(entry->y-draw_settings.YOrigin())*zoom_factor;
	if (ys< y_start*PIXELS_PER_SCROLL || ye > y_start*PIXELS_PER_SCROLL+height*3/4)
			y_start=(entry->y*zoom_factor-height/2)/(PIXELS_PER_SCROLL*zoom_factor)-1;
			if (y_start<0) y_start=0;
	// now update the flasher
		flasher->SetFlashNode(entry->x-4,entry->y-4,
													entry->x+draw_settings.NodeLength(),entry->y-4);

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
	 if(x>entry->x && x<entry->x+draw_settings.NodeLength() &&
			y>entry->y-DELTA && y<entry->y+DELTA)
			 {iterator->SetCursor(entry->n);return;}
   n=n->Next();
}
}



//
void DisplayOptionsForm::EditForm(TreeWindow *object,wxPanel *panel)
{
char	temp_str[40];
Add(wxMakeFormMessage("Size Settings"));
Add(wxMakeFormNewLine());

Add(wxMakeFormShort("Branch Length",&((object->draw_settings).branch_length),wxFORM_SLIDER,
											 new wxList(wxMakeConstraintRange(BRANCH_LENGTH_MIN, BRANCH_LENGTH_MAX), 0)));
Add(wxMakeFormNewLine());
Add(wxMakeFormShort("Node Length",&(object->draw_settings.node_length),wxFORM_SLIDER,
											 new wxList(wxMakeConstraintRange(NODE_LENGTH_MIN,NODE_LENGTH_MAX), 0)));
Add(wxMakeFormNewLine());
Add(wxMakeFormShort("Y Spacing",&(object->draw_settings.y_spacing),wxFORM_SLIDER,
											 new wxList(wxMakeConstraintRange(Y_SPACING_MIN, Y_SPACING_MAX), 0)));
Add(wxMakeFormNewLine());
Add(wxMakeFormBool("Show Labels",&(object->draw_settings.show_labels),wxFORM_CHECKBOX));
Add(wxMakeFormBool("Show Outcomes",&(object->draw_settings.show_outcomes),wxFORM_CHECKBOX));
Add(wxMakeFormBool("Show Infosets",&(object->draw_settings.show_infosets),wxFORM_CHECKBOX));
Add(wxMakeFormNewLine());
/*
Add(wxMakeFormString("Chance Color",&(object->draw_settings.chance_color),wxFORM_DEFAULT,
											 new wxList(wxMakeConstraintStrings(gambit_color_list), 0),NULL,
											 wxVERTICAL,160));
Add(wxMakeFormString("Cursor Color",&(object->draw_settings.cursor_color),wxFORM_DEFAULT,
											 new wxList(wxMakeConstraintStrings(gambit_color_list), 0),NULL,
											 wxVERTICAL,160));
*/
AssociatePanel(panel);
}

