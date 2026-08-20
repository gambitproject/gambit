//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/efgdisplay.cc
// Implementation of window class to display extensive form tree
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <algorithm> // for std::min

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP

#include "games.h"

#include "efgdisplay.h"
#include "menuconst.h"

namespace Gambit::GUI {

//----------------------------------------------------------------------
//                      EfgDisplay: Member functions
//----------------------------------------------------------------------

BEGIN_EVENT_TABLE(EfgDisplay, wxScrolledWindow)
EVT_MOTION(EfgDisplay::OnMouseMotion)
EVT_LEAVE_WINDOW(EfgDisplay::OnLeaveWindow)
EVT_LEFT_DOWN(EfgDisplay::OnLeftClick)
EVT_LEFT_DCLICK(EfgDisplay::OnLeftDoubleClick)
EVT_MAGNIFY(EfgDisplay::OnMagnify)
EVT_RIGHT_DOWN(EfgDisplay::OnRightClick)
EVT_SIZE(EfgDisplay::OnSize)
EVT_MENU_RANGE(GBT_MENU_EDIT_SET_PLAYER_BASE,
               GBT_MENU_EDIT_SET_PLAYER_BASE + gbtSetPlayerMenuCount - 1,
               EfgDisplay::OnSetPlayerMenu)
EVT_MENU(GBT_MENU_EDIT_INSERT_MOVE, EfgDisplay::OnEditInsertMove)
EVT_MENU(GBT_MENU_EDIT_DELETE_TREE, EfgDisplay::OnEditDeleteTree)
EVT_MENU(GBT_MENU_EDIT_DELETE_PARENT, EfgDisplay::OnEditDeleteParent)
EVT_MENU(GBT_MENU_EDIT_REMOVE_OUTCOME, EfgDisplay::OnEditRemoveOutcome)
EVT_MENU(GBT_MENU_EDIT_REVEAL, EfgDisplay::OnEditReveal)
EVT_MENU(GBT_MENU_EDIT_NODE, EfgDisplay::OnEditNode)
EVT_MENU(GBT_MENU_EDIT_MOVE, EfgDisplay::OnEditMove)
END_EVENT_TABLE()

//----------------------------------------------------------------------
//                EfgDisplay: Constructor and destructor
//----------------------------------------------------------------------

EfgDisplay::EfgDisplay(wxWindow *p_parent, GameDocument *p_doc)
  : wxScrolledWindow(p_parent), GameView(p_doc), m_layout(p_doc), m_zoom(100)
{
  wxWindow::SetBackgroundColour(wxColour(250, 250, 250));

  InitDropTarget();
  InitPopups();

  m_hoverTimer.SetOwner(this);
  Bind(wxEVT_TIMER, &EfgDisplay::OnHoverTimer, this);
  MakeMenus();
  OnUpdate();
}

//---------------------------------------------------------------------
//                  EfgDisplay: Event-hook members
//---------------------------------------------------------------------

void EfgDisplay::OnSize(wxSizeEvent &p_event)
{
  if (m_pendingInitialZoom) {
    const wxSize size = p_event.GetSize();
    if (size.GetWidth() > 50 && size.GetHeight() > 50) {
      FitZoom();
      m_pendingInitialZoom = false;
      FocusNode(m_doc->GetGame()->GetRoot(), 0.18, 0.5);
    }
  }

  p_event.Skip();
}

void EfgDisplay::OnLeftClick(wxMouseEvent &)
{
  // Just claims keyboard focus for the canvas (e.g. for scroll/zoom); there's no longer
  // any notion of a node being "selected" by a plain click.
  SetFocus();
}

void EfgDisplay::OnMagnify(wxMouseEvent &p_event)
{
  if (const double factor = 1.0 + p_event.GetMagnification(); factor > 0.0) {
    ZoomByFactor(factor, p_event.GetPosition());
  }
}

//---------------------------------------------------------------------
//           EfgDisplay: Implementing GameView members
//---------------------------------------------------------------------

void EfgDisplay::OnUpdate()
{
  // The popup may be showing stale data (or a node that no longer exists) after
  // whatever changed; the next hover will bring it back.
  DismissNodeInfo();

  // Force a rebuild on every change for now.
  RefreshTree();
}

//---------------------------------------------------------------------
//                   EfgDisplay: Drawing functions
//---------------------------------------------------------------------

void EfgDisplay::RefreshTree()
{
  m_layout.Layout(m_doc->GetGame());
  AdjustScrollbarSteps();
  Refresh();
}

constexpr int kScrollPixelsPerUnit = 1;

void EfgDisplay::AdjustScrollbarSteps()
{
  int oldPixelsPerUnitX, oldPixelsPerUnitY;
  GetScrollPixelsPerUnit(&oldPixelsPerUnitX, &oldPixelsPerUnitY);

  int scrollX, scrollY;
  GetViewStart(&scrollX, &scrollY);

  const int currentPixelX = scrollX * oldPixelsPerUnitX;
  const int currentPixelY = scrollY * oldPixelsPerUnitY;

  int clientWidth, clientHeight;
  GetClientSize(&clientWidth, &clientHeight);

  const int virtualWidth = LayoutToDevice(m_layout.MaxX());
  const int virtualHeight = LayoutToDevice(m_layout.MaxY());

  const int maxPixelX = std::max(0, virtualWidth - clientWidth);
  const int maxPixelY = std::max(0, virtualHeight - clientHeight);

  const int clampedPixelX = std::clamp(currentPixelX, 0, maxPixelX);
  const int clampedPixelY = std::clamp(currentPixelY, 0, maxPixelY);

  SetScrollbars(kScrollPixelsPerUnit, kScrollPixelsPerUnit,
                virtualWidth / kScrollPixelsPerUnit + 1, virtualHeight / kScrollPixelsPerUnit + 1,
                clampedPixelX / kScrollPixelsPerUnit, clampedPixelY / kScrollPixelsPerUnit);
}

void EfgDisplay::FitZoom()
{
  int width, height;
  GetClientSize(&width, &height);

  double zoomx = static_cast<double>(width) / static_cast<double>(m_layout.MaxX());
  double zoomy = static_cast<double>(height) / static_cast<double>(m_layout.MaxY());

  zoomx = std::min(zoomx, 1.0);
  zoomy = std::min(zoomy, 1.0); // never zoom in (only out)
  const int fittedZoom = static_cast<int>(100.0 * (std::min(zoomx, zoomy) * .9));
  m_zoom = std::max(50, fittedZoom);
  AdjustScrollbarSteps();
  Refresh();
}

namespace {

constexpr int kMinZoom = 10;
constexpr int kMaxZoom = 150;
constexpr int kZoomStep = 10;
constexpr int kScrollPixelsPerUnit = 1;

int ClampZoom(int p_zoom) { return std::clamp(p_zoom, kMinZoom, kMaxZoom); }

} // namespace

void EfgDisplay::SetZoom(int p_zoom)
{
  const int zoom = ClampZoom(p_zoom);
  if (zoom == m_zoom) {
    return;
  }

  m_zoom = zoom;
  AdjustScrollbarSteps();
  Refresh();
}

void EfgDisplay::ZoomByFactor(double p_factor, const wxPoint &p_clientPoint)
{
  if (p_factor <= 0.0) {
    return;
  }

  const int oldZoom = GetZoom();
  const int newZoom = ClampZoom(static_cast<int>(std::lround(oldZoom * p_factor)));

  if (newZoom == oldZoom) {
    return;
  }

  int unscrolledX, unscrolledY;
  CalcUnscrolledPosition(p_clientPoint.x, p_clientPoint.y, &unscrolledX, &unscrolledY);

  const double oldScale = GetZoom() / 100.0;
  const double layoutX = unscrolledX / oldScale;
  const double layoutY = unscrolledY / oldScale;

  SetZoom(newZoom);

  const double newScale = GetZoom() / 100.0;
  const int targetUnscrolledX = static_cast<int>(std::lround(layoutX * newScale));
  const int targetUnscrolledY = static_cast<int>(std::lround(layoutY * newScale));

  int pixelsPerUnitX, pixelsPerUnitY;
  GetScrollPixelsPerUnit(&pixelsPerUnitX, &pixelsPerUnitY);

  if (pixelsPerUnitX <= 0 || pixelsPerUnitY <= 0) {
    return;
  }

  const int targetScrollX = targetUnscrolledX - p_clientPoint.x;
  const int targetScrollY = targetUnscrolledY - p_clientPoint.y;

  int clientWidth, clientHeight;
  GetClientSize(&clientWidth, &clientHeight);

  int virtualWidth, virtualHeight;
  GetVirtualSize(&virtualWidth, &virtualHeight);

  const int maxPixelX = std::max(0, virtualWidth - clientWidth);
  const int maxPixelY = std::max(0, virtualHeight - clientHeight);

  const int clampedScrollX = std::clamp(targetScrollX, 0, maxPixelX);
  const int clampedScrollY = std::clamp(targetScrollY, 0, maxPixelY);

  Scroll(clampedScrollX / pixelsPerUnitX, clampedScrollY / pixelsPerUnitY);
}

void EfgDisplay::OnDraw(wxDC &p_dc)
{
  p_dc.SetUserScale(GetScale(), GetScale());
  p_dc.Clear();
  m_layout.Render(p_dc, false);
  DrawDragOverHighlight(p_dc);
}

void EfgDisplay::OnDraw(wxDC &p_dc, double p_zoom)
{
  // A bit of a hack: this allows us to set zoom separately in printout code
  const int saveZoom = m_zoom;
  m_zoom = static_cast<int>(100.0 * p_zoom);

  p_dc.SetUserScale(GetScale(), GetScale());
  p_dc.Clear();
  // This is usually only called by functions for hardcopy output (printouts
  // or graphics images).  We want to suppress the use of the "hints" for these.
  m_layout.Render(p_dc, true);

  m_zoom = saveZoom;
}

void EfgDisplay::FocusNode(const GameNode &p_node, double p_xFrac, double p_yFrac)
{
  if (!p_node) {
    return;
  }

  auto entry = m_layout.GetNodeEntry(p_node);
  if (!entry) {
    return;
  }

  int clientWidth, clientHeight;
  GetClientSize(&clientWidth, &clientHeight);

  const int targetX = LayoutToDevice(entry->GetX()) - clientWidth * p_xFrac;
  const int targetY = LayoutToDevice(entry->GetY()) - clientHeight * p_yFrac;

  int pixelsPerUnitX, pixelsPerUnitY;
  GetScrollPixelsPerUnit(&pixelsPerUnitX, &pixelsPerUnitY);

  int virtualWidth, virtualHeight;
  GetVirtualSize(&virtualWidth, &virtualHeight);

  const int maxPixelX = std::max(0, virtualWidth - clientWidth);
  const int maxPixelY = std::max(0, virtualHeight - clientHeight);

  const int clampedX = std::clamp(targetX, 0, maxPixelX);
  const int clampedY = std::clamp(targetY, 0, maxPixelY);

  Scroll(clampedX / pixelsPerUnitX, clampedY / pixelsPerUnitY);
}
} // namespace Gambit::GUI
