//
// FILE: flasher.cc
//
// @(#)flasher.cc	1.1 6/6/94
//

#include "wx.h"
#include "twflash.h"
/*****************************TREE NODE CURSOR*****************************/
TreeNodeCursor::TreeNodeCursor(wxDC *_dc)
{
SetType(myCURSOR);
old_x_s=-1;old_x_e=-1;old_y_s=-1;old_y_e=-1;dc=_dc;
SetFlashNode(-1,-1,-1,-1);
SetFlashing(TRUE);
}
TreeNodeCursor::~TreeNodeCursor(void)
{dc=NULL,flashing=FALSE;}

void TreeNodeCursor::SetFlashNode(int _x_s,int _y_s,int _x_e,int _y_e)
{x_s=_x_s;y_s=_y_s;x_e=_x_e;y_e=_y_e;}

void TreeNodeCursor::Flash(void)
{
static int cur_color;
char		*cur_clr;
if (flashing && dc && x_s!=-1)
{
	// Alternate between black and white colors
	if (__mytype==myFLASHER)
  {
		cur_color=(cur_color+1)%2;
		if (cur_color==0) cur_clr="BLACK"; else cur_clr="WHITE";
	}
	else	// __type==wxCURSOR
		cur_clr="BLACK";
	if (x_s!=old_x_s || x_e!=old_x_e || y_s!=old_y_s || y_e!=old_y_e)
	{
		if (old_x_s!=-1)	// if this is not the first time
		{
     	// if we move to a different node, erase the old cursor
			dc->SetPen(wxThePenList->FindOrCreatePen("WHITE",4,wxSOLID));
			dc->DrawLine(old_x_s,old_y_s,old_x_e,old_y_e);
       // after moving to a new node, flash black right away!
			cur_clr="BLACK";cur_color=1;
		}
		old_x_s=x_s;old_y_s=y_s;old_x_e=x_e;old_y_e=y_e;
	}
	// Draw the new cursor
	dc->SetPen(wxThePenList->FindOrCreatePen(cur_clr,4,wxSOLID));
	dc->DrawLine(x_s,y_s,x_e,y_e);
}
}

/*************************** TREE NODE FLASHER ****************************/
TreeNodeFlasher::TreeNodeFlasher(wxDC *_dc):wxTimer(),TreeNodeCursor(_dc)
{SetType(myFLASHER);SetFlashing(TRUE);}
TreeNodeFlasher::~TreeNodeFlasher(void) {}

#define FLASHDELAY 200
void TreeNodeFlasher::SetFlashing(Bool _flashing)
{
TreeNodeCursor::SetFlashing(_flashing);
if (TreeNodeCursor::Flashing()==TRUE)
	Start(FLASHDELAY);
else
	Stop();
}

