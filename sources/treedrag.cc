//
// FILE: treedrag.cc -- Implementation of dragging classes
//
// $Id$
//

#include "wx/wx.h"
#include "wxmisc.h"

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
// NodeDragger
//--------------------

TreeWindow::NodeDragger::NodeDragger(TreeWindow *parent_, FullEfg &ef_)
  : ef(ef_), parent(parent_), drag_now(0),
    start_node(0), end_node(0)
{
#ifdef __WXMSW__
  c_b = new wxBitmap("COPY_BITMAP");
  m_b = new wxBitmap("MOVE_BITMAP");
#else
#include "bitmaps/copy.xpm"
#include "bitmaps/move.xpm"
  c_b = new wxBitmap(copy_xpm);
  m_b = new wxBitmap(move_xpm);
#endif // wx_msw
  copy_dc = new wxMemoryDC;
  copy_dc->SelectObject(*c_b);
  move_dc = new wxMemoryDC;
  move_dc->SelectObject(*m_b);
}

TreeWindow::NodeDragger::~NodeDragger()
{
  delete c_b;
  delete m_b;
  delete move_dc;
  delete copy_dc;
}

void TreeWindow::NodeDragger::RedrawObject(void)
{
  static const int /*x_off = 0, */y_off = 21;
  wxClientDC dc(parent);
  if (ox >= 0) {
    dc.Blit(ox, oy-y_off, 32, 32, (oc) ? move_dc : copy_dc, 0, 0, wxXOR);
  }
  dc.Blit(x, y-y_off, 32, 32, (c) ? move_dc : copy_dc, 0, 0, wxXOR);
}

int TreeWindow::NodeDragger::OnEvent(wxMouseEvent &ev)
{
  int ret = (drag_now) ? DRAG_CONTINUE : DRAG_NONE;
  if (ev.Dragging()) {
    ox = x; oy = y; oc = c;
    ev.GetPosition(&x, &y);
    c = ev.ControlDown();
    if (!drag_now) {
      start_node = parent->GotObject(x, y, DRAG_NODE_START);
      if (start_node) {
	int wx = (int)(x*parent->GetZoom());
	int wy = (int)(y*parent->GetZoom());
	parent->WarpPointer(wx, wy);
	ox = -1; oc = 0; c = 0; drag_now = 1; ret = DRAG_START;
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
    end_node = parent->GotObject(x, y, DRAG_NODE_END);
    ev.GetPosition(&x, &y);
    c = ev.ControlDown();
    ret = DRAG_STOP;
    if (start_node && end_node && start_node != end_node) {
      try {
        if (c)
          ef.MoveTree(start_node, end_node);    // move
        else
          ef.CopyTree(start_node, end_node);    // copy
      }
      catch (gException &E) {
        guiExceptionDialog(E.Description(), parent->Parent());
      }
      parent->Refresh();
    }
  }
  return ret;
}

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


//--------------------
// OutcomeDragger
//--------------------

TreeWindow::OutcomeDragger::OutcomeDragger(TreeWindow *parent_, FullEfg &ef_)
    : ef(ef_), parent(parent_), drag_now(0), outcome(0)
#ifdef NOT_PORTED_YET
      outcome_cursor(new wxCursor("OUTCOMECUR"))
#endif  // NOT_PORTED_YET
{ }

TreeWindow::OutcomeDragger::~OutcomeDragger()
{ }

int TreeWindow::OutcomeDragger::OnEvent(wxMouseEvent &ev,
                                        bool &outcomes_changed)
{
  int ret = (drag_now) ? DRAG_CONTINUE : DRAG_NONE;
  if (ev.Dragging()) {
    if (!drag_now) {
      ev.GetPosition(&x, &y); outcome = 0;
      start_node = parent->GotObject(x, y, DRAG_OUTCOME_START);
      if (start_node) {
	outcome = start_node->GetOutcome();
	if (outcome) {
#ifdef NOT_PORTED_YET
	  parent->SetCursor(outcome_cursor);
#endif  // NOT_PORTED_YET
	  drag_now = 1; ret = DRAG_START;
	}
      }
    }
  }
  else if (drag_now) {
#ifdef NOT_PORTED_YET
    parent->SetCursor(wxSTANDARD_CURSOR);
#endif  // NOT_PORTED_YET 
    ev.GetPosition(&x, &y);
    bool c = ev.ControlDown();
    ret = DRAG_STOP;
    Node *end_node = parent->GotObject(x, y, DRAG_OUTCOME_END);
    if (end_node) {
      end_node->SetOutcome(outcome);
      if (c) start_node->SetOutcome(0); // move
      outcomes_changed = 1;
      parent->Refresh();
    }
    drag_now = 0;
  }
  return ret;
}

