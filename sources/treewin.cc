//
// FILE: treewin.cc -- Implementation of extensive form windows
//
// $Id$
//
#define MAX_LABEL_LENGTH	25
#include <stdlib.h>
#include "wx.h"
#include "wx_mf.h"
#include "wx_clipb.h"
#include "wx_form.h"
#include "wxmisc.h"
#include "flasher.h"
#include "extform.h"
#include "const.h"
#include "treewin.h"
#include "player.h"
#include "infoset.h"


#define RENDER_DEVICE_SCREEN		0
#define RENDER_DEVICE_PS				1
#define RENDER_DEVICE_PRINTER		2
#define	RENDER_DEVICE_METAFILE	3	

wxFont *label_font;
wxList *gambit_color_list;

// Validation function for string dialogs
Bool StringConstraint(int type, char *value, char *label, char *msg_buffer)
{
  if (value && (strlen(value) >= MAX_LABEL_LENGTH))
  {
    sprintf(msg_buffer, "Value for %s should be %d characters or less",
            label,MAX_LABEL_LENGTH-1);
    return FALSE;
  }
  else return TRUE;
}


//=====================================================================
//                   EXTENSIVEFRAME MEMBER FUNCTIONS
//=====================================================================

//---------------------------------------------------------------------
//             EXTENSIVEFRAME: CONSTRUCTOR AND DESTRUCTOR
//---------------------------------------------------------------------

ExtensiveFrame::ExtensiveFrame(wxFrame *frame, char *title, int x, int y, int w, int h, int type, ExtForm *p):
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

	tw = new TreeWindow(this, 0, 0, w, h,p);
	// Give it scrollbars: the virtual canvas is 20 * 50 = 1000 pixels in each direction
  tw->SetScrollbars(20, 20, 60, 60, 4, 4);

  if (p != 0)
    SetTitle(p->TreeLabel().stradr());
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

		case FILE_PRINT_EPS:         // To use the special wxWindows EPS driver
                                 // under Windows 3.1, specify "PostScript"
   {
     wxPostScriptDC dc(NULL,TRUE);
     if (dc.Ok())
     {
	      dc.StartDoc("Hello printout");
 		    dc.StartPage();
				tw->Render(dc,RENDER_DEVICE_PS);
      	dc.EndPage();
				dc.EndDoc();
		}
		break;
    }
#ifdef wx_msw
	case FILE_PRINT:
    {
			wxPrinterDC dc(NULL, NULL, NULL);  // Defaults to EPS under UNIX,
																				// normal Windows printing under Win 3.1
      if (dc.Ok())
      {
        dc.StartDoc("PLOTX printout");
        dc.StartPage();
				tw->Render(dc,RENDER_DEVICE_PRINTER);
        dc.EndPage();
        dc.EndDoc();
			}
			break;
		}
		case FILE_COPY_MF:
    {
      wxMetaFileDC dc;
      if (dc.Ok())
			{
				tw->Render(dc,RENDER_DEVICE_METAFILE);
				wxMetaFile *mf = dc.Close();
        if (mf)
        {
					mf->SetClipboard((int)(dc.MaxX()+10),(int)(dc.MaxY()+10));
          delete mf;
        }
			}
			break;
    }
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
//		frame->OnMenuCommand(id);
    break;

	}
}

//=====================================================================
//                      TREEWINDOW MEMBER FUNCTIONS
//=====================================================================

//---------------------------------------------------------------------
//                TREEWINDOW: CONSTRUCTOR AND DESTRUCTOR
//---------------------------------------------------------------------

TreeWindow::TreeWindow(wxFrame *_frame,int x,int y,int w,int h,ExtForm *p,int style):
								wxCanvas(_frame, x, y, w, h, style)
{
	frame=_frame;
  if (p)
    the_problem = p;
  else
    the_problem = new ExtForm;

  iterator = new ExtFormIter(the_problem);

	node_table=NULL;
	zoom_factor=1.0;
	draw_settings.SetWindow(gRect(0, 0, w,h));
	_flasher=new TreeNodeFlasher;
  _flasher->SetDC(GetDC());
	_flasher->SetFlashNode(-1,-1,-1,-1);
	_flasher->SetFlashing(TRUE);
	label_font=new wxFont(11,wxSWISS,wxNORMAL,wxNORMAL);
	GetDC()->SetBackgroundMode(wxTRANSPARENT);
	// ###
	gambit_color_list=new wxList((wxObject *)"TURQUOISE",(wxObject *)"RED",(wxObject *)"BLUE",
	(wxObject *)"GREEN",(wxObject *)"YELLOW",(wxObject *)"VIOLET",(wxObject *)"SALMON",(wxObject *)"ORCHID",
												(wxObject *)"PALE GREEN",(wxObject *)"BROWN",NULL);
}

TreeWindow::~TreeWindow()
{
	delete the_problem;
  delete iterator;
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
	draw_settings.SetIterator(iterator);
	Render(*(GetDC()),RENDER_DEVICE_SCREEN);
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

//-----------------------------------------------------------------------
//                    NODE MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//***********************************************************************
//                        NODE-ADD MENU HANDLER
//***********************************************************************
#define MAX_PLAYER	10
#define MAX_BRANCHES	20
int TreeWindow::node_add(void)
{
	int player_num=0;
	int branches=1;

	MyDialogBox *form_dialog = new MyDialogBox(frame, "Add Node");
	form_dialog->form->Add(wxMakeFormShort("Player #     ", &player_num, wxFORM_TEXT,
											 new wxList(wxMakeConstraintRange(0, MAX_PLAYER), 0)));
	form_dialog->form->Add(wxMakeFormNewLine());
	form_dialog->form->Add(wxMakeFormShort("# of Branches",&branches, wxFORM_TEXT,
											 new wxList(wxMakeConstraintRange(0, MAX_BRANCHES), 0)));
	form_dialog->form->AssociatePanel(form_dialog);
	form_dialog->Fit();
	form_dialog->Show(TRUE);

	if (form_dialog->Completed()==wxOK)
		the_problem->AddNode(iterator->Cursor(),player_num,branches);
	delete form_dialog;
	return 1;
}

//***********************************************************************
//                       NODE-LABEL MENU HANDLER
//***********************************************************************
int TreeWindow::node_label(void)
{
	char *label=new char[MAX_LABEL_LENGTH];
	strcpy(label,iterator->Cursor()->Name());
	MyDialogBox *form_dialog = new MyDialogBox(frame, "Label Node");
	form_dialog->form->Add(wxMakeFormString("Label", &label,wxFORM_DEFAULT,
		new wxList(wxMakeConstraintFunction(StringConstraint))));
	form_dialog->form->AssociatePanel(form_dialog);
  form_dialog->Fit();
	form_dialog->Show(TRUE);
	if (form_dialog->Completed()==wxOK)
	{
		gString t=label;
		the_problem->LabelNode(iterator->Cursor(), t);
	}
  delete form_dialog;
	return 1;
}

//***********************************************************************
//                      NODE-INSERT MENU HANDLER
//***********************************************************************
int TreeWindow::node_insert(void)
{
	int player_num=0;
	int branches=1;

	MyDialogBox *form_dialog = new MyDialogBox(frame, "Insert Node");
	form_dialog->form->Add(wxMakeFormShort("Player #     ", &player_num, wxFORM_TEXT,
											 new wxList(wxMakeConstraintRange(0, MAX_PLAYER), 0)));
	form_dialog->form->Add(wxMakeFormNewLine());
	form_dialog->form->Add(wxMakeFormShort("# of Branches",&branches, wxFORM_TEXT,
											 new wxList(wxMakeConstraintRange(0, MAX_BRANCHES), 0)));
	form_dialog->form->AssociatePanel(form_dialog);
  form_dialog->Fit();
	form_dialog->Show(TRUE);
	if (form_dialog->Completed()==wxOK)
		the_problem->InsertNode(iterator->Cursor(),player_num,branches);
  delete form_dialog;
  return 1;
}

//***********************************************************************
//                       NODE-SET-MARK MENU HANDLER
//***********************************************************************

int TreeWindow::node_set_mark(void)
{
	mark_node = draw_settings.Iterator()->Cursor();
	return 1;
}

//***********************************************************************
//                      NODE-GOTO-MARK MENU HANDLER
//***********************************************************************

int TreeWindow::node_goto_mark(void)
{
		if (mark_node.IsNonNull())
    {
			draw_settings.Iterator()->SetCursor(mark_node);
			ProcessCursor();
		}
		else
			wxMessageBox("Mark not set!","Error",wxOK | wxCENTRE);
	return 1;
}

//***********************************************************************
//                       NODE-OUTCOME MENU HANDLER
//***********************************************************************
int TreeWindow::node_outcome(void)
{
  int outcome_num=the_problem->GetOutcome(iterator->Cursor());

  MyDialogBox *form_dialog = new MyDialogBox(frame, "Node Outcome");
	form_dialog->form->Add(wxMakeFormShort("Outcome #     ", &outcome_num, wxFORM_TEXT));
	form_dialog->form->AssociatePanel(form_dialog);
  form_dialog->Fit();
  form_dialog->Show(TRUE);
	if (form_dialog->Completed()==wxOK)
		the_problem->SetOutcome(iterator->Cursor(), outcome_num);
  delete form_dialog;

  return 1;
}

//***********************************************************************
//                      NODE-DELETE MENU HANDLER
//***********************************************************************

int TreeWindow::node_delete(void)
{
/*
	if (iterator->Cursor()->NumBranches() == 0)
    return 1;

  if (mark_node == 0 || !the_problem->ContainsNode(mark_node))  {
		wxMessageBox("The mark is not set", "Error",wxOK | wxCENTRE);
		return 1;
  }

	if (mark_node->Parent() != iterator->Cursor())  {
		wxMessageBox("The mark is not a child of this node","Error",wxOK | wxCENTRE);
		return 1;
  }

	the_problem->DeleteNode(iterator->Cursor(), mark_node);
	iterator->SetCursor(mark_node);
*/
	return 1;
}

//-----------------------------------------------------------------------
//                   INFOSET MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//***********************************************************************
//                    INFOSET-MEMBER MENU HANDLER
//***********************************************************************

int TreeWindow::infoset_member(void)
{
/*
	if (mark_node == 0 || !the_problem->ContainsNode(mark_node))  {
		wxMessageBox("The mark is not set", "Error",wxOK | wxCENTRE);
		return 1;
  }

	Node *cursor = iterator->Cursor();

  if (cursor->Infoset() == 0)  {
		wxMessageBox("This node has no information set","Infoset Member",wxOK | wxCENTRE);
		return 1;
  }

  if (mark_node->NumBranches() == 0)   {  // terminal, make new nodes
    the_problem->AddBranches(mark_node, cursor->NumBranches());
    for (int i = 1; i <= cursor->NumBranches(); i++)
      mark_node->BranchNumber(i)->SetBranchName(cursor->BranchNumber(i)->BranchName());
  }
  else  {
    if (mark_node->NumBranches() != cursor->Infoset()->NumBranches())  {
			wxMessageBox("Wrong number of branches for this information set",
				 "Infoset Member",wxOK | wxCENTRE);
			return 1;
    }
    for (int i = 1; i <= cursor->NumBranches(); i++)  {
      mark_node->BranchNumber(i)->SetBranchName(cursor->BranchNumber(i)->BranchName());
      mark_node->BranchNumber(i)->SetOutcome(cursor->BranchNumber(i)->Outcome());
      mark_node->BranchNumber(i)->SetProbability(cursor->BranchNumber(i)->Probability());
    }
  }

	the_problem->AddToInfoset(mark_node, cursor->Infoset());
*/
	return 1;
}

//***********************************************************************
//                      INFOSET-BREAK MENU HANDLER
//***********************************************************************

int TreeWindow::infoset_break(void)
{
/*
	the_problem->BreakInfoset(iterator->Cursor());
*/
	return 1;
}

//***********************************************************************
//                       INFOSET-JOIN MENU HANDLER
//***********************************************************************

int TreeWindow::infoset_join(void)
{
/*
	if (mark_node == 0 || !the_problem->ContainsNode(mark_node))  {
		wxMessageBox("The mark is not set", "Error",wxOK | wxCENTRE);
		return 1;
  }

	the_problem->JoinInfosets(iterator->Cursor()->Infoset(),
			    mark_node->Infoset());

*/
	return 1;
}

//-----------------------------------------------------------------------
//                    BRANCH MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//***********************************************************************
//                      BRANCH-LABEL MENU HANDLER
//***********************************************************************
int TreeWindow::branch_label(void)
{
/*
	char *label=new char[MAX_LABEL_LENGTH];
	strcpy(label,iterator->Cursor()->BranchName());

	MyDialogBox *form_dialog = new MyDialogBox(frame, "Node Outcome");
	form_dialog->form->Add(wxMakeFormString("Label", &label,wxFORM_DEFAULT,
		new wxList(wxMakeConstraintFunction(StringConstraint))));
	form_dialog->form->AssociatePanel(form_dialog);
	form_dialog->Fit();
	form_dialog->Show(TRUE);

	// not precisely right: need to do whole infoset...
	if (form_dialog->Completed()==wxOK)
		iterator->Cursor()->SetBranchName(dlg->BranchLabel());
  delete form_dialog;
*/
	return 1;
}

//***********************************************************************
//                     BRANCH-INSERT MENU HANDLER
//***********************************************************************

int TreeWindow::branch_insert(void)
{
/*
	int branch_num=-1;

	if (iterator->Cursor()->NumBranches() == 0)   {
		wxMessageBox("Terminal node: cannot insert branch.","Error",wxOK | wxCENTRE);
		return 1;
  }

	wxDialogBox *form_dialog = new wxDialogBox(frame, "Node Outcome", TRUE);
	wxForm *my_form = new wxForm(wxFORM_BUTTON_OK | wxFORM_BUTTON_CANCEL,wxFORM_BUTTON_AT_BOTTOM);
	my_form->Add(wxMakeFormShort("Branch #", &branch_num,
	 new wxList(wxMakeConstraintRange(-1,iterator->Cursor()->NumBranches()+1), 0)));
	my_form->AssociatePanel(form_dialog);
	form_dialog->Fit();
	form_dialog->Show(TRUE);

	if (branch_num<=0)  {
			return 1;
    }

		the_problem->InsertBranch(iterator->Cursor(), dlg->BranchNumber());

		if (iterator->Cursor()->Infoset() == 0)   {
    // enter_probabilities(cursor);
    }


*/
  return 1;
}

//***********************************************************************
//                       BRANCH-DELETE MENU HANDLER
//***********************************************************************
int TreeWindow::branch_delete(void)
{
/*
	if (iterator->Cursor()->NumBranches() == 0)   return 1;

  if (mark_node == 0 || !the_problem->ContainsNode(mark_node))  {
		wxMessageBox("The mark is not set", "Error",wxOK | wxCENTRE);
		return 1;
  }

	if (mark_node->Parent() != iterator->Cursor())  {
		wxMessageBox("The mark is not a child of this node","Error",wxOK | wxCENTRE);
		return 1;
  }

	Node *cursor = the_problem->DeleteBranch(iterator->Cursor(),
					 iterator->Cursor()->GetBranchNumber(mark_node));
  iterator->SetCursor(cursor);

	if (cursor->Infoset() == 0)   {
		// enter_probabilities(cursor);
  }

*/
	return 1;
}

//-----------------------------------------------------------------------
//                     TREE MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//***********************************************************************
//                       TREE-LABEL MENU HANDLER
//***********************************************************************

int TreeWindow::tree_label(void)
{
/*
char *label=new char[MAX_LABEL_LENGTH];
strcpy(label,"Testing");

	wxDialogBox *form_dialog = new wxDialogBox(frame, "Node Outcome", TRUE);
	wxForm *my_form = new wxForm(wxFORM_BUTTON_OK | wxFORM_BUTTON_CANCEL,wxFORM_BUTTON_AT_BOTTOM);
	my_form->Add(wxMakeFormShort("Label", &label);
	my_form->AssociatePanel(form_dialog);
	form_dialog->Fit();
	form_dialog->Show(TRUE);

	the_problem->LabelTree(label);

*/
  return 1;
}

//***********************************************************************
//                      TREE-DELETE MENU HANDLER
//***********************************************************************

int TreeWindow::tree_delete(void)
{
/*
	if (iterator->Cursor()->NumBranches() == 0)
    return 1;

	the_problem->DeleteTree(iterator->Cursor());
*/
	return 1;
}

//***********************************************************************
//                       TREE-COPY MENU HANDLER
//***********************************************************************

int TreeWindow::tree_copy(void)
{
/*
  if (mark_node == 0 || !the_problem->ContainsNode(mark_node))  {
		wxMessageBox("The mark is not set", "Error",wxOK | wxCENTRE);
		return 1;
  }

  the_problem->CopyTree(iterator->Cursor(), mark_node);
*/
	return 1;
}

//***********************************************************************
//                      TREE-OUTCOME MENU HANDLER
//***********************************************************************

int TreeWindow::tree_outcomes(void)
{
	char tempstr[40];
	int outcome_num=-1;
	int num_players=5; // ###
  int temp;

	wxDialogBox *form_dialog = new wxDialogBox(frame, "Edit Outcome", TRUE);
	wxForm *my_form = new wxForm(wxFORM_BUTTON_OK | wxFORM_BUTTON_CANCEL,wxFORM_BUTTON_AT_BOTTOM);
	my_form->Add(wxMakeFormShort("Outcome #", &outcome_num));
	for (int i=0;i<num_players;i++)
	{
		sprintf(tempstr,"Player #%d ",i);
		my_form->Add(wxMakeFormNewLine());
		my_form->Add(wxMakeFormShort(tempstr,&temp));
  }
	my_form->AssociatePanel(form_dialog);
	form_dialog->Fit();
	form_dialog->Show(TRUE);

/*
  OutcomeWindow *ow = new OutcomeWindow(this, the_problem);

  delete ow;
*/
  return 1;
}

//***********************************************************************
//                      TREE-PLAYERS MENU HANDLER
//***********************************************************************
int TreeWindow::tree_players(void)
{
// ###
int i,num_players=6;
char *name=new char[50];
char *colr=new char[50];
char tempstr[200];
wxDialogBox *form_dialog = new wxDialogBox(frame, "Edit Players", TRUE);
wxForm *my_form = new wxForm(wxFORM_BUTTON_OK | wxFORM_BUTTON_CANCEL,wxFORM_BUTTON_AT_BOTTOM);
my_form->Add(wxMakeFormMessage("#     Name          Color"));
sprintf(colr,"");
for (i=0;i<num_players;i++)
{
	my_form->Add(wxMakeFormNewLine());
	sprintf(tempstr,"%2d",i);
	my_form->Add(wxMakeFormMessage(tempstr));
	my_form->Add(wxMakeFormString("",&name,wxFORM_TEXT,NULL,NULL,NULL,100,30));
	my_form->Add(wxMakeFormString("",&colr,wxFORM_CHOICE,
											 new wxList(wxMakeConstraintStrings(gambit_color_list), 0),NULL,
											 wxVERTICAL,160,30));
}
my_form->AssociatePanel(form_dialog);
form_dialog->Fit();
form_dialog->Show(TRUE);
delete name;
delete colr;
}


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

int maxlev, maxy, miny, ycoord;

int TreeWindow::FillTable(NodeEntry *table, gHandle<Node> &n, int level)
{
  int myy, y1, yn;
	NodeEntry *entry = table + (the_problem->_nodes->ElNumber(n) - 1);

  if (n->NumChildren() > 0)  {
    for (uint i = 1; i <= n->NumChildren(); i++)  {
			yn = FillTable(table, (*n)(i), level + 1);
      if (i == 1)  y1 = yn;
    }
    entry->y = (y1 + yn) / 2;
  } else {
    entry->y = ycoord;
    ycoord += draw_settings.YSpacing();
  }

  entry->level = level;
	entry->x = level * (draw_settings.NodeLength() + draw_settings.BranchLength());

  if (n->Player().IsNonNull()) 
		entry->color = the_problem->_players->ElNumber(n->Player()) % 16;
  else 
		entry->color = ::ColorNum(draw_settings.ChanceColor());

  if (level > maxlev)    maxlev = level;
  if (entry->y > maxy)   maxy = entry->y;
  else if (entry->y < miny)   miny = entry->y;

  return entry->y;
}


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
void DrawLine(DisplayDevice &device,int x_s,int y_s,int x_e,int y_e,int color=-1)
{
if (color>-1)
	device.SetPen(wxThePenList->FindOrCreatePen(GetPlayerColor(color),2,wxSOLID));
device.DrawLine(x_s,y_s,x_e,y_e);
}
//###
//Draw Text. A quick and dirty way of easily drawing text w/ set color
void DrawText(DisplayDevice &device,char *s,int x,int y,int color=-1)
{
if (color>-1)
	device.SetPen(wxThePenList->FindOrCreatePen(GetPlayerColor(color),2,wxSOLID));
device.DrawText(s,x,y);
}


void TreeWindow::RenderSubtree(DisplayDevice &device,
					gHandle<Node> &root, NodeEntry *table)
{
  NodeEntry *entry = table + (the_problem->_nodes->ElNumber(root) - 1);

	::DrawLine(device,entry->x, entry->y,
			entry->x + draw_settings.NodeLength(), entry->y, entry->color);
	device.SetFont(label_font);
  if (draw_settings.ShowLabels())
		::DrawText(device,root->Name().stradr(),entry->x, entry->y+3, entry->color);

  for (uint i = 1; i <= root->NumChildren(); i++)  {
		::DrawLine(device,entry->x + draw_settings.NodeLength(), entry->y,
				entry->x + draw_settings.NodeLength() + draw_settings.BranchLength() / 2,
				table[the_problem->_nodes->ElNumber((*root)(i)) - 1].y,entry->color);
		::DrawLine(device,entry->x + draw_settings.NodeLength() + draw_settings.BranchLength() / 2,
				table[the_problem->_nodes->ElNumber((*root)(i)) - 1].y,
				table[the_problem->_nodes->ElNumber((*root)(i)) - 1].x,
				table[the_problem->_nodes->ElNumber((*root)(i)) - 1].y, entry->color);
    RenderSubtree(device, (*root)(i), table);
  }
}

void TreeWindow::Render(DisplayDevice &device,int render_device)
{
	if (node_table) delete node_table;
  // Note that node_table is preserved until the next redraw
	node_table = new NodeEntry[(int)(the_problem->_nodes->Length())];

  maxlev = miny = maxy = 0;
  ycoord = draw_settings.Window().height() / 2;

  FillTable(node_table, the_problem->RootNode(), 0);
	//	_flasher->SetDC(&device);
	// now update the flasher
	if (render_device==RENDER_DEVICE_SCREEN)
  {
		if (draw_settings.Iterator() != 0)
		{
			int cursor_index = the_problem->_nodes->ElNumber(draw_settings.Iterator()->Cursor()) - 1;
			_flasher->SetFlashNode(node_table[cursor_index].x-4,node_table[cursor_index].y-4,
			node_table[cursor_index].x+draw_settings.NodeLength(), node_table[cursor_index].y-4);
		}
  }
	else
	{
		_flasher->SetFlashing(FALSE);
  }
	device.Clear();
	device.BeginDrawing();
	RenderSubtree(device,the_problem->RootNode(), node_table);
	if (render_device!=RENDER_DEVICE_SCREEN) _flasher->SetFlashing(TRUE);
  device.EndDrawing();
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
#define PIXELS_PER_SCROLL	20

ViewStart(&x_start,&y_start);
GetParent()->GetClientSize(&width,&height);
x_scroll=((maxlev + 1) * (draw_settings.BranchLength() + draw_settings.NodeLength()))/PIXELS_PER_SCROLL+1;
y_scroll=maxy/PIXELS_PER_SCROLL+1;

// If cursor is active, make sure it is visible
if (draw_settings.Iterator() != 0)
{
	int cursor_index = the_problem->_nodes->ElNumber(draw_settings.Iterator()->Cursor()) - 1;
	// check if in the visible x-dimention
	if ((node_table[cursor_index].x - draw_settings.XOrigin())*zoom_factor < x_start*PIXELS_PER_SCROLL ||
	 (node_table[cursor_index].x - draw_settings.XOrigin()+ draw_settings.NodeLength()/2)*zoom_factor > x_start*PIXELS_PER_SCROLL+width)
		x_start=node_table[cursor_index].x*zoom_factor/(PIXELS_PER_SCROLL*zoom_factor)-1;
		if (x_start<0) x_start=0;
	// check if in the visible y-dimention
	if ((node_table[cursor_index].y - draw_settings.YOrigin())*zoom_factor < y_start*PIXELS_PER_SCROLL ||
		(node_table[cursor_index].y - draw_settings.YOrigin())*zoom_factor > y_start*PIXELS_PER_SCROLL+height*3/4)
			y_start=(node_table[cursor_index].y*zoom_factor-height/2)/(PIXELS_PER_SCROLL*zoom_factor)-1;
			if (y_start<0) y_start=0;
	// now update the flasher
		_flasher->SetFlashNode(node_table[cursor_index].x-4,node_table[cursor_index].y-4,
		node_table[cursor_index].x+draw_settings.NodeLength(), node_table[cursor_index].y-4);

}

if (x_start!=draw_settings.get_x_scroll() ||
		y_start!=draw_settings.get_y_scroll())
{
	Scroll(x_start,y_start);
	draw_settings.set_x_scroll(x_start);
	draw_settings.set_y_scroll(y_start);
}
}

#define DELTA	8
void TreeWindow::ProcessClick(int x,int y)
{
for (int i=1;i<=the_problem->_nodes->Length();i++)
{
	 if(x>node_table[i].x && x<node_table[i].x+draw_settings.NodeLength() &&
			y>node_table[i].y-DELTA && y<node_table[i].y+DELTA)
				iterator->SetCursor((*(the_problem->_nodes))[i+1]);
}
}


void TreeWindow::SetZoom(void)
{
#define MAX_ZOOM	10
MyDialogBox *zoom_dialog = new MyDialogBox(frame, "Set Zoom");
zoom_dialog->form->Add(wxMakeFormFloat("Zoom [0.1-10]", &zoom_factor, wxFORM_DEFAULT,
						 new wxList(wxMakeConstraintRange(0.1,MAX_ZOOM), 0)));
zoom_dialog->form->AssociatePanel(zoom_dialog);
zoom_dialog->Fit();
zoom_dialog->Show(TRUE);
if (zoom_dialog->Completed()==wxOK)
	GetDC()->SetUserScale(zoom_factor,zoom_factor);
delete zoom_dialog;
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

