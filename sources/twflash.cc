//
// FILE: twflash.cc -- implements the cursor for navigating the tree
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "twflash.h"
#include "treewin.h"

//-----------------------------------------------------------------------
//                         class TreeNodeCursor
//-----------------------------------------------------------------------

TreeNodeCursor::TreeNodeCursor(TreeWindow *p_parent)
  : m_parent(p_parent)
{
  SetType(myCursor);
  old_x_s = -1;
  old_x_e = -1;
  old_y_s = -1;
  old_y_e = -1;
  cursor_type = nodeCursor;
  old_cursor_type = nodeCursor;
  SetFlashNode(-1, -1, -1, -1, nodeCursor);
  SetFlashing(TRUE);
}

TreeNodeCursor::~TreeNodeCursor()
{ }

void TreeNodeCursor::SetFlashNode(int _x_s, int _y_s, int _x_e, int _y_e,
				  CursorType type)
{
  x_s = _x_s;
  y_s = _y_s;
  x_e = _x_e;
  y_e = _y_e;
  cursor_type = type;
}

void TreeNodeCursor::GetFlashNode(int & _x_s, int & _y_s, int & _x_e,
				  int & _y_e) const
{
  _x_s = x_s;
  _y_s = y_s;
  _x_e = x_e;
  _y_e = y_e;
}

void TreeNodeCursor::DrawCursor(wxDC &p_dc, int xs, int ys, int xe, int ye, 
				CursorType type)
{
  switch (type) {
  case nodeCursor:    // just a line
    p_dc.DrawLine(xs+5, ys-4, xe, ye-4);
    break;
  case subgameCursor: // an isosceles triangle w/ height=base=(xe-xs)
    int icon_size = xe-xs;
    p_dc.DrawLine(xs-4, ys-4, xe+4, ys-6-icon_size/2);
    p_dc.DrawLine(xs-4, ys+4, xe+4, ys+6+icon_size/2);
    p_dc.DrawLine(xe+4, ys+6+icon_size/2, xe+4, ys-6-icon_size/2);
    break;
  }
}

void TreeNodeCursor::Flash(wxDC &p_dc)
{
  static int cur_color;
  char        *cur_clr;
    
  if (flashing && x_s != -1) {
    // Alternate between black and white colors
    if (__mytype == myFlasher) {
      cur_color = (cur_color+1)%2;
      
      if (cur_color == 0) cur_clr = "BLACK";
      else cur_clr = "WHITE";
    }
    else    // __type==wxCURSOR
      cur_clr = "BLACK";
    
    if (x_s != old_x_s || x_e != old_x_e || y_s != old_y_s || y_e != old_y_e) {
      if (old_x_s != -1)  { // if this is not the first time
	// if we move to a different node, erase the old cursor
	p_dc.SetPen(*wxThePenList->FindOrCreatePen("WHITE", 4, wxSOLID));
	DrawCursor(p_dc, old_x_s, old_y_s, old_x_e, old_y_e, old_cursor_type);

	// after moving to a new node, flash black right away!
	cur_clr = "BLACK";
	cur_color = 1;
      }

      old_x_s = x_s;
      old_y_s = y_s;
      old_x_e = x_e;
      old_y_e = y_e;
      old_cursor_type = cursor_type;
    }

    // Draw the new cursor
    p_dc.SetPen(*wxThePenList->FindOrCreatePen(cur_clr, 4, wxSOLID));
    DrawCursor(p_dc, x_s, y_s, x_e, y_e, cursor_type);
  }
}

