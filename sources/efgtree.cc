//
// FILE: efgtree.cc -- Implementation of extensive form viewer
//
// $Id$
//

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/mdi.h"
#endif

#include "wx/splitter.h"

#include "efg.h"

#include "guiapp.h"
#include "efgview.h"
#include "efgtree.h"

class EfgDrawSettings {
public:
  int NodeLength(void) const { return 60; }
  int BranchLength(void) const { return 60; }
  int ForkLength(void) const { return 60; }
  int VerticalSpacing(void) const { return 50; }

  wxColour PlayerColor(EFPlayer *) const;
};

wxColour EfgDrawSettings::PlayerColor(EFPlayer *p_player) const
{
  static wxString colors[3] = { "RED", "BLUE", "GREEN" };

  if (p_player) {
    if (p_player->IsChance()) {
      return wxColour("YELLOW");
    }
    else {
      return wxColour(colors[p_player->GetNumber() % 3]);
    }
  }
  else {
    return wxColour("BLACK");
  }
}

typedef enum {
  treeNODE = 0,
  treeINFOSET = 1,
  treeACTION = 2,
  treeLABEL = 3,
  treeOUTCOME = 4
} treeObjectType;

class TreeObject {
private:
  bool m_selected;

protected:
  guiEfgTree *m_window;

  virtual void OnSelected(bool) { }

public:
  TreeObject(guiEfgTree *p_window) 
    : m_selected(false), m_window(p_window) { }
  virtual ~TreeObject() { }

  virtual void Draw(wxDC &, const EfgDrawSettings &) = 0;
  virtual bool HitTest(int x, int y, const EfgDrawSettings &) const = 0; 
  virtual treeObjectType Type(void) const = 0;

  void Select(bool p_selected) 
    { m_selected = p_selected; OnSelected(p_selected); } 
  bool Selected(void) const { return m_selected; }
};

class NodeEntry : public TreeObject {
private:
  Node *m_node;
  int m_x, m_y, m_length;
  wxColour m_color;

  void OnSelected(bool);

public:
  NodeEntry(guiEfgTree *, Node *, int, int, int);
  virtual ~NodeEntry() { }

  void Draw(wxDC &, const EfgDrawSettings &);
  bool HitTest(int x, int y, const EfgDrawSettings &) const;
  treeObjectType Type(void) const { return treeNODE; }

  Node *GetNode(void) const { return m_node; }
};

NodeEntry::NodeEntry(guiEfgTree *p_window, Node *p_node,
		     int p_x, int p_y, int p_length)
  : TreeObject(p_window), 
    m_node(p_node), m_x(p_x), m_y(p_y), m_length(p_length)
{ }

void NodeEntry::Draw(wxDC &p_dc, const EfgDrawSettings &p_settings)
{
  m_color = p_settings.PlayerColor(m_node->GetPlayer());
  p_dc.SetPen(wxPen(m_color, 2, wxSOLID));
  p_dc.DrawLine(m_x, m_y, m_x + m_length, m_y);
  p_dc.DrawEllipse(m_x - 3, m_y - 3, 6, 6);

  if (Selected()) {
    p_dc.SetPen(wxPen(wxColour("BLACK"), 2, wxSOLID));
    p_dc.DrawRectangle(m_x, m_y - 1, m_length, 2);
  }
}

void NodeEntry::OnSelected(bool p_selected)
{
  wxClientDC dc(m_window);
  m_window->PrepareDC(dc);
  dc.SetUserScale(m_window->GetZoom(), m_window->GetZoom());

  if (p_selected) {
    dc.SetPen(wxPen(wxColour("BLACK"), 2, wxSOLID));
    dc.DrawRectangle(m_x, m_y - 1, m_length, 2);
  }
  else {
    dc.SetPen(wxPen(wxColour("WHITE"), 2, wxSOLID));
    dc.DrawRectangle(m_x, m_y - 1, m_length, 2);
    dc.SetPen(wxPen(m_color, 2, wxSOLID));
    dc.DrawLine(m_x, m_y, m_x + m_length, m_y);
    dc.DrawEllipse(m_x - 3, m_y - 3, 6, 6);
  }

  m_window->OnSelectedNode(m_node, p_selected);
}

bool NodeEntry::HitTest(int p_x, int p_y,
			const EfgDrawSettings &p_settings) const
{
  static int DELTA = 8;

  return (p_x > m_x &&
	  p_x < m_x + p_settings.NodeLength() &&
	  p_y > m_y - DELTA &&
	  p_y < m_y + DELTA);
}

class NodeAboveEntry : public TreeObject {
private:
  Node *m_node;
  int m_x, m_y;

public:
  NodeAboveEntry(guiEfgTree *, Node *, int, int);
  virtual ~NodeAboveEntry() { }

  void Draw(wxDC &, const EfgDrawSettings &);
  bool HitTest(int x, int y, const EfgDrawSettings &) const;
  treeObjectType Type(void) const { return treeLABEL; }
};

NodeAboveEntry::NodeAboveEntry(guiEfgTree *p_window,
			       Node *p_node, int p_x, int p_y)
  : TreeObject(p_window), m_node(p_node), m_x(p_x), m_y(p_y)
{ }

void NodeAboveEntry::Draw(wxDC &p_dc, const EfgDrawSettings &p_settings)
{
  long width, height;
  p_dc.GetTextExtent("0", &width, &height);
  p_dc.DrawText((char *) m_node->GetName(), m_x, m_y - height - 9); 
}

bool NodeAboveEntry::HitTest(int p_x, int p_y,
			     const EfgDrawSettings &p_settings) const
{
  static int DELTA = 8;

  return (p_x > m_x &&
	  p_x < m_x + p_settings.NodeLength() &&
	  p_y > m_y - DELTA - 12 &&
	  p_y < m_y - DELTA);
}

class NodeBelowEntry : public TreeObject {
private:
  Node *m_node;
  int m_x, m_y;

public:
  NodeBelowEntry(guiEfgTree *, Node *, int, int);
  virtual ~NodeBelowEntry() { }

  void Draw(wxDC &, const EfgDrawSettings &);
  bool HitTest(int x, int y, const EfgDrawSettings &) const;
  treeObjectType Type(void) const { return treeLABEL; }
};

NodeBelowEntry::NodeBelowEntry(guiEfgTree *p_window,
			       Node *p_node, int p_x, int p_y)
  : TreeObject(p_window), m_node(p_node), m_x(p_x), m_y(p_y)
{ }

void NodeBelowEntry::Draw(wxDC &p_dc, const EfgDrawSettings &p_settings)
{
  if (m_node->GetInfoset()) {
    p_dc.DrawText((char *) m_node->GetInfoset()->GetName(), m_x, m_y + 5);
  }
}

bool NodeBelowEntry::HitTest(int p_x, int p_y,
			     const EfgDrawSettings &p_settings) const
{
  const int DELTA = 8;

  return (p_x > m_x &&
	  p_x < m_x + p_settings.NodeLength() &&
	  p_y > m_y + DELTA &&
	  p_y < m_y + DELTA + 12);
}

class OutcomeEntry : public TreeObject {
private:
  Node *m_node;
  int m_x, m_y;
  long m_width, m_height;

  void OnSelected(bool);

public:
  OutcomeEntry(guiEfgTree *, Node *, int, int);
  virtual ~OutcomeEntry() { }

  void Draw(wxDC &, const EfgDrawSettings &);
  bool HitTest(int x, int y, const EfgDrawSettings &) const;
  treeObjectType Type(void) const { return treeOUTCOME; }

  EFOutcome *GetOutcome(void) const { return m_node->GetOutcome(); }
};

OutcomeEntry::OutcomeEntry(guiEfgTree *p_window,
			   Node *p_node, int p_x, int p_y)
  : TreeObject(p_window), m_node(p_node), m_x(p_x), m_y(p_y)
{ }

void OutcomeEntry::Draw(wxDC &p_dc, const EfgDrawSettings &p_settings)
{
  EFOutcome *outcome = m_node->GetOutcome();
 
  if (outcome) {
    p_dc.GetTextExtent((char *) outcome->GetName(), &m_width, &m_height);
    p_dc.DrawText((char *) outcome->GetName(),
		  m_x + p_settings.NodeLength() + 10,
		  m_y - 12);

    if (Selected()) {
      wxPoint points[] = { wxPoint(m_x + p_settings.NodeLength() + 8,
				   m_y - 14),
			   wxPoint(m_x + p_settings.NodeLength() + m_width + 12,
				   m_y - 14),
			   wxPoint(m_x + p_settings.NodeLength() + m_width + 12,
				   m_y + m_height - 10),
			   wxPoint(m_x + p_settings.NodeLength() + 8,
				   m_y + m_height - 10),
			   wxPoint(m_x + p_settings.NodeLength() + 8,
				   m_y - 14) };
      p_dc.DrawLines(5, points);
    }
  }
}

bool OutcomeEntry::HitTest(int p_x, int p_y, 
			   const EfgDrawSettings &p_settings) const
{
  if (m_node->GetOutcome()) {
    return (p_x > m_x + p_settings.NodeLength() + 10 &&
            p_x < m_x + p_settings.NodeLength() + 10 + m_width &&
	    p_y > m_y - 12 &&
	    p_y < m_y - 12 + m_height);
  }
  return false;
}

void OutcomeEntry::OnSelected(bool p_selected)
{
  const int nodeLength = 60;

  wxPoint points[] = { wxPoint(m_x + nodeLength + 8, m_y - 14),
		       wxPoint(m_x + nodeLength + m_width + 12, m_y - 14),
		       wxPoint(m_x + nodeLength + m_width + 12,
			       m_y + m_height - 10),
		       wxPoint(m_x + nodeLength + 8,
			       m_y + m_height - 10),
		       wxPoint(m_x + nodeLength + 8,
			       m_y - 14) };
  
  wxClientDC dc(m_window);
  m_window->PrepareDC(dc);
  dc.SetUserScale(m_window->GetZoom(), m_window->GetZoom());
  if (p_selected) {
    dc.SetPen(wxPen(wxColour("BLACK"), 2, wxSOLID));
  }
  else {
    dc.SetPen(wxPen(wxColour("WHITE"), 2, wxSOLID));
  }
  dc.DrawLines(5, points);

  m_window->OnSelectedOutcome(m_node->GetOutcome(), p_selected);
}


class BranchEntry : public TreeObject {
private:
  Node *m_parent;
  int m_branch, m_x, m_startY, m_endY;
  wxColour m_color;

  void OnSelected(bool);

public:
  BranchEntry(guiEfgTree *, Node *, int, int, int, int);
  virtual ~BranchEntry() { }

  void Draw(wxDC &, const EfgDrawSettings &);
  bool HitTest(int x, int y, const EfgDrawSettings &) const;
  treeObjectType Type(void) const { return treeACTION; }

  Node *GetParent(void) const { return m_parent; }
  Action *GetAction(void) const
    { return m_parent->GetInfoset()->Actions()[m_branch]; }
};

BranchEntry::BranchEntry(guiEfgTree *p_window, Node *p_parent, int p_branch,
			 int p_x, int p_startY, int p_endY)
  : TreeObject(p_window),
    m_parent(p_parent), m_branch(p_branch), m_x(p_x),
    m_startY(p_startY), m_endY(p_endY)
{ }

void BranchEntry::Draw(wxDC &p_dc, const EfgDrawSettings &p_settings)
{
  Action *action = m_parent->GetInfoset()->Actions()[m_branch];
  if (m_parent->GetInfoset()->GetPlayer()->IsChance() ||
      m_window->Support()->Find(action)) {
    m_color = p_settings.PlayerColor(m_parent->GetPlayer());
    p_dc.SetPen(wxPen(m_color, 2, wxSOLID));
    p_dc.DrawLine(m_x, m_startY, m_x + p_settings.ForkLength(), m_endY);
    
    p_dc.DrawLine(m_x + p_settings.ForkLength(), m_endY,
		  m_x + p_settings.ForkLength() + p_settings.BranchLength(),
		  m_endY);
  }

  if (Selected()) {
    p_dc.SetPen(wxPen(wxColour("BLACK"), 2, wxSOLID));
    p_dc.DrawLine(m_x, m_startY, m_x + p_settings.ForkLength(), m_endY);
  }
}

bool BranchEntry::HitTest(int p_x, int p_y,
			  const EfgDrawSettings &p_settings) const
{
  Action *action = m_parent->GetInfoset()->Actions()[m_branch];
  if (!m_parent->GetInfoset()->GetPlayer()->IsChance() &&
      !m_window->Support()->Find(action)) {
    return false;
  }
  
  double y = m_startY + (int) (p_x - m_x) * (m_endY - m_startY) / p_settings.ForkLength();

  return (p_x >= m_x && p_x <= m_x + p_settings.ForkLength() &&
	  p_y >= y - 2 && p_y <= y + 2);
}

void BranchEntry::OnSelected(bool p_selected)
{
  wxClientDC dc(m_window);
  m_window->PrepareDC(dc);
  dc.SetUserScale(m_window->GetZoom(), m_window->GetZoom());

  if (p_selected) {
    dc.SetPen(wxPen(wxColour("BLACK"), 2, wxSOLID));
  }
  else {
    dc.SetPen(wxPen(m_color, 2, wxSOLID));
  }

  dc.DrawLine(m_x, m_startY, m_x + 60, m_endY);
}

BEGIN_EVENT_TABLE(guiEfgTree, wxScrolledWindow)
  EVT_LEFT_DOWN(OnLeftClick)
  EVT_LEFT_UP(OnLeftUp)
  EVT_LEFT_DCLICK(OnDoubleLeftClick)
  EVT_MOTION(OnMouseMotion)
END_EVENT_TABLE()

guiEfgTree::guiEfgTree(guiEfgView *p_parent, Efg &p_efg)
  : wxScrolledWindow(p_parent, -1, wxDefaultPosition, wxDefaultSize,
		     wxVSCROLL | wxHSCROLL), m_efg(p_efg), m_parent(p_parent),
    m_selection(0), m_zoomFactor(1.0)
{
  m_settings = new EfgDrawSettings;

  LayoutTree();
  SetBackgroundColour(wxColour("WHITE"));
  Show(TRUE);
}

guiEfgTree::~guiEfgTree()
{
  delete m_settings;
}

void guiEfgTree::OnDraw(void)
{
  wxClientDC dc(this);
  PrepareDC(dc);
  dc.SetUserScale(m_zoomFactor, m_zoomFactor);
  OnDraw(dc);
}

void guiEfgTree::OnDraw(wxDC &p_dc)
{
  p_dc.BeginDrawing();
  p_dc.SetUserScale(m_zoomFactor, m_zoomFactor);
  p_dc.SetPen(wxPen("BLACK", 2, wxSOLID));
  p_dc.Clear();

  for (int i = 1; i <= m_objects.Length(); i++) {
    m_objects[i]->Draw(p_dc, *m_settings);
  }
  p_dc.EndDrawing();
}

int guiEfgTree::LayoutSubtree(Node *p_node, int p_level,
			      int &p_maxX, int &p_maxY, int &p_minY,
			      int &p_ycoord)
{
  int numChildren = m_efg.NumChildren(p_node);
  gArray<int> childCoords(numChildren);
  int xCoord, yCoord;


  if (numChildren > 0) {
    for (int child = 1; child <= numChildren; child++) {
      childCoords[child] = LayoutSubtree(p_node->GetChild(child), 
					 p_level + 1, p_maxX,
					 p_maxY, p_minY, p_ycoord);
    }
    yCoord = (childCoords[1] + childCoords[numChildren]) / 2;
  }
  else {
    yCoord = p_ycoord;
    p_ycoord += m_settings->VerticalSpacing();
  }

  xCoord = p_level * (m_settings->NodeLength() + m_settings->BranchLength() +
		      m_settings->ForkLength());
  TreeObject *object = new NodeEntry(this, p_node, xCoord, yCoord,
				     m_settings->NodeLength());
  m_objects.Append(object);
  object = new NodeAboveEntry(this, p_node, xCoord, yCoord);
  m_objects.Append(object);
  object = new NodeBelowEntry(this, p_node, xCoord, yCoord);
  m_objects.Append(object);
  object = new OutcomeEntry(this, p_node, xCoord, yCoord);
  m_objects.Append(object);

  for (int child = 1; child <= numChildren; child++) {
    object = new BranchEntry(this,
			     p_node, child, xCoord + m_settings->NodeLength(),
			     yCoord, childCoords[child]);
    m_objects.Append(object);
  }

  p_maxX = gmax(xCoord + m_settings->NodeLength(), p_maxX);
  p_maxY = gmax(yCoord, p_maxY);
  p_minY = gmin(yCoord, p_minY);

  return yCoord;
}

void guiEfgTree::LayoutTree(void)
{
  const int TOP_MARGIN = 40;
  m_maxX = 0;
  m_maxY = 0;
  m_minY = 0;
  int ycoord = TOP_MARGIN;

  LayoutSubtree(m_efg.RootNode(), 0, m_maxX, m_maxY, m_minY, ycoord);
  ComputeScrollbars();
}

void guiEfgTree::ComputeScrollbars(void)
{
  SetScrollbars(20, 20, 
		(m_maxX + m_settings->NodeLength() + 50) * m_zoomFactor / 20 + 1,
		m_maxY * m_zoomFactor / 20 + 1);
}

TreeObject *guiEfgTree::HitTest(int p_x, int p_y) const
{
  int scrollX, scrollY;
  ViewStart(&scrollX, &scrollY);
  int unitsX, unitsY;
  GetScrollPixelsPerUnit(&unitsX, &unitsY);
  scrollX *= unitsX;
  scrollY *= unitsY;

  for (int i = 1; i <= m_objects.Length(); i++) {
    if (m_objects[i]->HitTest((p_x + scrollX) / m_zoomFactor,
			      (p_y + scrollY) / m_zoomFactor, *m_settings)) {
      return m_objects[i];
    }
  }

  return 0;
}

void guiEfgTree::OnLeftClick(wxMouseEvent &p_event)
{
  if (m_selection) {
    m_selection->Select(false);
    m_selection = 0;
  }

  int scrollX, scrollY;
  ViewStart(&scrollX, &scrollY);
  scrollX *= (m_maxX / 15);
  scrollY *= (m_maxY / 15);

  m_curX = (int) ((p_event.GetX() + scrollX) / m_zoomFactor);
  m_curY = (int) ((p_event.GetY() + scrollY) / m_zoomFactor);
  m_oldX = 0;
  m_oldY = 0;
  m_startX = m_curX;
  m_startY = m_curY;

  m_selection = HitTest(p_event.GetX(), p_event.GetY());
  if (m_selection) {
    m_selection->Select(true);
    return;
  }
}

void guiEfgTree::OnLeftUp(wxMouseEvent &p_event)
{
  SetCursor(*wxSTANDARD_CURSOR);
  TreeObject *object = HitTest(p_event.GetX(), p_event.GetY());
  if (object && object->Type() == treeNODE) {
    NodeEntry *nodeEntry = (NodeEntry *) object;
    if (nodeEntry != m_selection && 
	m_selection->Type() == treeNODE &&
	m_efg.NumChildren(nodeEntry->GetNode()) == 0) {
      m_parent->GetEfg()->CopyTree(((NodeEntry *) m_selection)->GetNode(),
				   nodeEntry->GetNode());
      OnTreeChanged();
      return;
    }
  }
}

void guiEfgTree::OnDoubleLeftClick(wxMouseEvent &p_event)
{
  if (HitTest(p_event.GetX(), p_event.GetY())) {
    m_parent->OnProperties();
    return;
  }
}

void guiEfgTree::OnMouseMotion(wxMouseEvent &p_event)
{
  /*
  if (p_event.Dragging()) {
    int scrollX, scrollY;
    ViewStart(&scrollX, &scrollY);
    scrollX *= (m_maxX / 15);
    scrollY *= (m_maxY / 15);

    m_oldX = m_curX;
    m_oldY = m_curY;
    m_curX = p_event.GetX() / m_zoomFactor + scrollX;
    m_curY = p_event.GetY() / m_zoomFactor + scrollY;

    wxClientDC dc(this);
    PrepareDC(dc);
    dc.SetLogicalFunction(wxXOR);
    if (m_oldX > 0) {
      dc.DrawLine(m_startX, m_startY, m_oldX, m_oldY);
    }
    dc.DrawLine(m_startX, m_startY, m_curX, m_curY);
  }
  */
}

Node *guiEfgTree::SelectedNode(void) const
{
  if (m_selection == 0 || m_selection->Type() != treeNODE) {
    return 0;
  }

  return ((NodeEntry *) m_selection)->GetNode();
}

EFOutcome *guiEfgTree::SelectedOutcome(void) const
{
  if (!m_selection || m_selection->Type() != treeOUTCOME) {
    return 0;
  }

  return ((OutcomeEntry *) m_selection)->GetOutcome();
}

Action *guiEfgTree::SelectedAction(void) const
{
  if (!m_selection || m_selection->Type() != treeACTION) {
    return 0;
  }

  return ((BranchEntry *) m_selection)->GetAction();
}

void guiEfgTree::OnSelectedOutcome(EFOutcome *p_outcome, bool p_selected)
{
  m_parent->OnSelectedOutcome(p_outcome, p_selected);
}

void guiEfgTree::OnSelectedNode(Node *p_node, bool p_selected)
{
  m_parent->OnSelectedNode(p_node, p_selected);
}

void guiEfgTree::OnTreeChanged(void)
{
  for (int i = 1; i <= m_objects.Length(); i++) {
    delete m_objects[i];
  }
  m_objects.Flush();

  LayoutTree();
  m_selection = 0;
  OnDraw();
}

void guiEfgTree::OnOutcomeChanged(EFOutcome *)
{
  OnDraw();
}

void guiEfgTree::OnNodeChanged(Node *)
{
  OnDraw();
}

void guiEfgTree::OnSupportChanged(EFSupport *)
{
  OnDraw();
}

void guiEfgTree::SetZoom(double p_zoomFactor)
{
  m_zoomFactor = p_zoomFactor;
  ComputeScrollbars();
  OnDraw();
}

EFSupport *guiEfgTree::Support(void) const
{
  return m_parent->CurrentSupport();
}

#include "garray.imp"
template class gArray<TreeObject *>;

#include "gblock.imp"
template class gBlock<TreeObject *>;


