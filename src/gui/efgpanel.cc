//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/efgpanel.cc
// Main viewing panel for extensive forms
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

#include <algorithm>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif                // WX_PRECOMP
#include <wx/print.h> // for printing support
#include <wx/dcsvg.h> // for SVG output

#include "efgpanel.h"

#include "efgdisplay.h" // FIXME: communicate with tree window via events.
#include "menuconst.h"

namespace Gambit::GUI {

//=====================================================================
//              Implementation of class EfgPanel
//=====================================================================

BEGIN_EVENT_TABLE(EfgPanel, wxPanel)
EVT_MENU(GBT_MENU_VIEW_ZOOMIN, EfgPanel::OnViewZoomIn)
EVT_MENU(GBT_MENU_VIEW_ZOOMOUT, EfgPanel::OnViewZoomOut)
EVT_MENU(GBT_MENU_VIEW_ZOOM100, EfgPanel::OnViewZoom100)
EVT_MENU(GBT_MENU_VIEW_ZOOMFIT, EfgPanel::OnViewZoomFit)
END_EVENT_TABLE()

EfgPanel::EfgPanel(wxWindow *p_parent, GameDocument *p_doc)
  : wxPanel(p_parent, wxID_ANY), GameView(p_doc), m_treeWindow(new EfgDisplay(this, m_doc))
{
  auto *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_treeWindow, 1, wxEXPAND, 0);
  SetSizer(topSizer);
  wxWindowBase::Layout();
}

namespace {

constexpr int kMinZoom = 10;
constexpr int kMaxZoom = 150;
constexpr int kZoomStep = 10;

int ClampZoom(int p_zoom) { return std::clamp(p_zoom, kMinZoom, kMaxZoom); }

} // namespace

void EfgPanel::OnViewZoomIn(wxCommandEvent &)
{
  m_treeWindow->SetZoom(ClampZoom(m_treeWindow->GetZoom() + kZoomStep));
}

void EfgPanel::OnViewZoomOut(wxCommandEvent &)
{
  m_treeWindow->SetZoom(ClampZoom(m_treeWindow->GetZoom() - kZoomStep));
}

void EfgPanel::OnViewZoom100(wxCommandEvent &) { m_treeWindow->SetZoom(100); }

void EfgPanel::OnViewZoomFit(wxCommandEvent &) { m_treeWindow->FitZoom(); }

class gbtEfgPrintout : public wxPrintout {
private:
  EfgPanel *m_efgPanel;

public:
  gbtEfgPrintout(EfgPanel *p_efgPanel, const wxString &p_label)
    : wxPrintout(p_label), m_efgPanel(p_efgPanel)
  {
  }
  ~gbtEfgPrintout() override = default;

  bool OnPrintPage(int) override
  {
    m_efgPanel->RenderGame(*GetDC(), 50, 50);
    return true;
  }
  bool HasPage(int page) override { return (page <= 1); }
  void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo) override
  {
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
  }
};

wxPrintout *EfgPanel::GetPrintout()
{
  return new gbtEfgPrintout(this, wxString::FromUTF8(m_doc->GetGame()->GetTitle()));
}

bool EfgPanel::GetBitmap(wxBitmap &p_bitmap, int p_marginX, int p_marginY)
{
  if (m_treeWindow->GetLayout().MaxX() > 65000 || m_treeWindow->GetLayout().MaxY() > 65000) {
    // This is just too huge to export to graphics
    return false;
  }

  wxMemoryDC dc;
  p_bitmap = wxBitmap(m_treeWindow->GetLayout().MaxX() + 2 * p_marginX,
                      m_treeWindow->GetLayout().MaxY() + 2 * p_marginY);
  dc.SelectObject(p_bitmap);
  RenderGame(dc, p_marginX, p_marginY);
  return true;
}

void EfgPanel::GetSVG(const wxString &p_filename, int p_marginX, int p_marginY)
{
  // The size of the image to be drawn
  const int maxX = m_treeWindow->GetLayout().MaxX();
  const int maxY = m_treeWindow->GetLayout().MaxY();

  wxSVGFileDC dc(p_filename, maxX + 2 * p_marginX, maxY + 2 * p_marginY);
  // For some reason, this needs to be initialized
  dc.SetLogicalScale(1.0, 1.0);
  RenderGame(dc, p_marginX, p_marginY);
}

void EfgPanel::RenderGame(wxDC &p_dc, int p_marginX, int p_marginY)
{
  // The size of the image to be drawn
  const int maxX = m_treeWindow->GetLayout().MaxX();
  const int maxY = m_treeWindow->GetLayout().MaxY();

  // Get the size of the DC in pixels
  wxCoord w, h;
  p_dc.GetSize(&w, &h);

  // Calculate a scaling factor
  const double scaleX = static_cast<double>(w) / static_cast<double>(maxX + 2 * p_marginX);
  const double scaleY = static_cast<double>(h) / static_cast<double>(maxY + 2 * p_marginY);
  double scale = (scaleX < scaleY) ? scaleX : scaleY;
  // Never zoom in
  if (scale > 1.0) {
    scale = 1.0;
  }
  p_dc.SetUserScale(scale, scale);

  // Calculate the position on the DC to center the tree
  auto posX = ((w - (maxX * scale)) / 2.0);
  auto posY = ((h - (maxY * scale)) / 2.0);
  p_dc.SetDeviceOrigin(static_cast<int>(posX), static_cast<int>(posY));

  // Draw!
  m_treeWindow->OnDraw(p_dc, scale);
}
} // namespace Gambit::GUI
