//
// FILE: flasher.cc -- implements the cursor for navigating the extensive
// form tree.  Can be set to flash at regular intervals or be solid.  If
// not flashing, make sure to call Flash after every move.
//
// $Id$
//

#include "wx.h"
#include "twflash.h"
/*****************************TREE NODE CURSOR*****************************/
TreeNodeCursor::TreeNodeCursor(wxDC *_dc)
{
SetType(myCursor);
old_x_s=-1;old_x_e=-1;old_y_s=-1;old_y_e=-1;ox=0;oy=0;dc=_dc;
cursor_type=nodeCursor;old_cursor_type=nodeCursor;
SetFlashNode(-1,-1,-1,-1,nodeCursor);
SetFlashing(TRUE);
}
TreeNodeCursor::~TreeNodeCursor(void)
{dc=NULL;flashing=FALSE;}

void TreeNodeCursor::SetFlashNode(int _x_s,int _y_s,int _x_e,int _y_e,CursorType type)
{x_s=_x_s;y_s=_y_s;x_e=_x_e;y_e=_y_e;cursor_type=type;}

void TreeNodeCursor::Translate(int _ox,int _oy)
{ox=_ox;oy=_oy;}
void TreeNodeCursor::GetFlashNode(int & _x_s,int & _y_s,int & _x_e,int & _y_e) const
{_x_s=x_s;_y_s=y_s;_x_e=x_e;_y_e=y_e;}

void TreeNodeCursor::DrawCursor(int xs,int ys,int xe,int ye,CursorType type)
{
switch (type)
{
	case nodeCursor:	// just a line
		dc->DrawLine(xs+5,ys-4,xe,ye-4);
		break;
	case subgameCursor: // an isosceles triangle w/ height=base=(xe-xs)
	{
		int icon_size=xe-xs;
		dc->DrawLine(xs-4,ys-4,xe+4,ys-6-icon_size/2);
		dc->DrawLine(xs-4,ys+4,xe+4,ys+6+icon_size/2);
		dc->DrawLine(xe+4,ys+6+icon_size/2,xe+4,ys-6-icon_size/2);
		break;
	}
}
}

void TreeNodeCursor::Flash(void)
{
static int cur_color;
char		*cur_clr;
if (flashing && dc && x_s!=-1)
{
	// Alternate between black and white colors
	if (__mytype==myFlasher)
	{
		cur_color=(cur_color+1)%2;
		if (cur_color==0) cur_clr="BLACK"; else cur_clr="WHITE";
	}
	else	// __type==wxCURSOR
		cur_clr="BLACK";
	if (x_s!=old_x_s || x_e!=old_x_e || y_s!=old_y_s || y_e!=old_y_e || old_ox!=ox || old_oy!=oy)
	{
		if (old_x_s!=-1)	// if this is not the first time
		{
			// if we move to a different node, erase the old cursor
			dc->SetPen(wxThePenList->FindOrCreatePen("WHITE",4,wxSOLID));
			DrawCursor(old_x_s+ox,old_y_s+oy,old_x_e+ox,old_y_e+oy,old_cursor_type);
			 // after moving to a new node, flash black right away!
			cur_clr="BLACK";cur_color=1;
		}
		old_x_s=x_s;old_y_s=y_s;old_x_e=x_e;old_y_e=y_e;old_cursor_type=cursor_type;
    old_ox=ox;old_oy=oy;
	}
	// Draw the new cursor
	dc->SetPen(wxThePenList->FindOrCreatePen(cur_clr,4,wxSOLID));
	DrawCursor(x_s+ox,y_s+oy,x_e+ox,y_e+oy,cursor_type);
}
}

/*************************** TREE NODE FLASHER ****************************/
TreeNodeFlasher::TreeNodeFlasher(wxDC *_dc):wxTimer(),TreeNodeCursor(_dc)
{SetType(myFlasher);SetFlashing(TRUE);}
TreeNodeFlasher::~TreeNodeFlasher(void)
{Stop();}

#define FLASHDELAY 200
void TreeNodeFlasher::SetFlashing(Bool _flashing)
{
TreeNodeCursor::SetFlashing(_flashing);
if (TreeNodeCursor::Flashing()==TRUE)
	Start(FLASHDELAY);
else
	Stop();
}

