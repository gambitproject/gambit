//
// FILE: treedrag.cc -- Implementation of dragging classes
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "guishare/wxmisc.h"

#include "efg.h"
#include "treewin.h"
#include "efgshow.h"
#include "treedrag.h"

#include "dlefgplayer.h"

//-------------------
// Draggers
//-------------------

// Classes to take care of drag and drop features of the TreeWindow
// Note that all of these depend on the parent having a function GotObject
// that would return a node if the mouse drag activated the process that is
// provided by the class.

//--------------------
// IsetDragger
//--------------------

TreeWindow::IsetDragger::IsetDragger(TreeWindow *parent_, FullEfg &ef_)
  : ef(ef_), parent(parent_), drag_now(0), start_node(0), end_node(0)
{ }

TreeWindow::IsetDragger::~IsetDragger()
{ }

void TreeWindow::IsetDragger::RedrawObject(void)
{
  wxClientDC dc(parent);
  dc.SetLogicalFunction(wxXOR);
  if (ox > 0) {
    dc.DrawLine(sx, sy, ox, oy);
  }
  dc.DrawLine(sx, sy, x, y);
  dc.SetLogicalFunction(wxCOPY);
}

int TreeWindow::IsetDragger::OnEvent(wxMouseEvent &ev)
{
  int ret = (drag_now) ? DRAG_CONTINUE : DRAG_NONE;
  if (ev.Dragging()) {
    ox = x; oy = y;
    ev.GetPosition(&x, &y);
    if (!drag_now) {
      start_node = parent->GotObject(x, y, DRAG_ISET_START);
      if (start_node) {
	int wx = (int)(x*parent->GetZoom());
	int wy = (int)(y*parent->GetZoom());
	parent->WarpPointer(wx, wy);
	sx = x; sy = y; ox = -1; drag_now = 1; ret = DRAG_START;
      }
    }
    if (drag_now) {
      RedrawObject();
      if (ret != DRAG_START) ret = DRAG_CONTINUE;
    }
  }
  if (ev.LeftUp() && drag_now) {
    ox = -1; drag_now = 0;
    RedrawObject();
    end_node = parent->GotObject(x, y, DRAG_ISET_END);
    ev.GetPosition(&x, &y);
    ret = DRAG_STOP;
    if (start_node && end_node && start_node != end_node) {
      Infoset *to = start_node->GetInfoset();
      Infoset *from = end_node->GetInfoset();
      if (to && from) {
	if (to->GetPlayer() == from->GetPlayer()) {
	  gText iset_name = from->GetName();
	  Infoset *miset = ef.MergeInfoset(to, from);
	  miset->SetName(iset_name+":1");
	  parent->Refresh();
	}
      }
    }
  }
  return ret;
}

//--------------------
// BranchDragger
//--------------------

TreeWindow::BranchDragger::BranchDragger(TreeWindow *parent_, FullEfg &ef_)
  : ef(ef_), parent(parent_), drag_now(0), br(0), start_node(0)
{ }

TreeWindow::BranchDragger::~BranchDragger()
{ }

void TreeWindow::BranchDragger::RedrawObject(void)
{
  wxClientDC dc(parent);
  dc.SetLogicalFunction(wxINVERT);
  dc.SetPen(*wxBLACK_PEN);
  if (ox > 0) {
    dc.DrawLine(sx, sy, ox, oy);
  }
  dc.DrawLine(sx, sy, x, y);
}


int TreeWindow::BranchDragger::OnEvent(wxMouseEvent &ev)
{
  int ret = (drag_now) ? DRAG_CONTINUE : DRAG_NONE;
  if (ev.Dragging()) {
    ox = x; oy = y;
    ev.GetPosition(&x, &y);
    if (!drag_now) {
      start_node = parent->GotObject(x, y, DRAG_BRANCH_START);
      if (start_node) {
	int wx = (int)(x*parent->GetZoom());
	int wy = (int)(y*parent->GetZoom());
	parent->WarpPointer(wx, wy);
	sx = x; sy = y; ox = -1; drag_now = 1; ret = DRAG_START;
      }
    }
    if (drag_now) {
      RedrawObject();
      if (ret != DRAG_START) ret = DRAG_CONTINUE;
    }
  }

  if (ev.LeftUp() && drag_now) {
    ox = -1; drag_now = 0;
    RedrawObject();
    if (parent->GotObject(x, y, DRAG_BRANCH_END))
      br = (int)(x+0.5); // round x to an integer -- branch # is passed back this way
    else
      br = 0;
    ev.GetPosition(&x, &y);
    ret = DRAG_STOP;
    if (start_node && br) {
      Infoset *iset = start_node->GetInfoset();
      if (iset) {
	if (br > iset->NumActions())
	  ef.InsertAction(iset);
	else
	  ef.InsertAction(iset, iset->Actions()[br]);
      }
      else {
	dialogEfgSelectPlayer dialog(ef, true, parent->GetParent());
	if (dialog.ShowModal() == wxID_OK) {
	  EFPlayer *player = dialog.GetPlayer();
	  if (player) ef.AppendNode(start_node, player, 1);
	}
      }
      parent->Refresh();
    }
  }
  return ret;
}
