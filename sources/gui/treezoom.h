//
// FILE: treezoom.h -- Declaration of zoom window display
//
// $Id$
//

#ifndef TREEZOOM_H
#define TREEZOOM_H

#ifdef ZOOM_WINDOW

#include "treerender.h"

class TreeZoomWindow : public TreeRender {
private:
  // Real xs,ys,xe,ye: true (untranslated) coordinates of the cursor.  
  // Need this if we repaint due to events other than cursor movement.  
  // See ::Render()
  int xs, ys, xe, ye, ox, oy;
  TreeWindow *m_parent;
  float m_zoom;

  Infoset *HighlightInfoset(void) const;
  Infoset *HighlightInfoset1(void) const;

  Node *MarkNode(void) const;
  Node *SubgameNode(void) const;

  guiNodeList &NodeList(void);
  const TreeDrawSettings &DrawSettings(void) const;

public:
  TreeZoomWindow(wxFrame *frame, TreeWindow *parent);
  virtual ~TreeZoomWindow() { }

  virtual void Render(wxDC &dc);
  // Makes sure the cursor is always in the center of the window
  virtual void UpdateCursor(void);
  virtual Node *Cursor(void) const;

  int NumDecimals(void) const;

  void SetZoom(float zoom);
  float GetZoom(void) const;
};

#endif  // ZOOM_WINDOW

#endif  // TREEZOOM_H
