//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Main viewing panel for extensive forms
//
// This file is part of Gambit
// Copyright (c) 2005, The Gambit Project
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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // WX_PRECOMP
#include <wx/dnd.h>           // for drag-and-drop features
#include <wx/image.h>         // for creating drag-and-drop cursor
#include <wx/print.h>         // for printing support
#include <wx/colordlg.h>      // for picking player colors

#include "dcsvg.h"         // for SVG output

#include "efgpanel.h"
#include "efgdisplay.h"  // FIXME: communicate with tree window via events.
#include "menuconst.h"
#include "edittext.h"

//=====================================================================
//                class gbtBehavDominanceToolbar
//=====================================================================

//!
//! This panel serves as a toolbar for interactively viewing
//! dominance information on extensive forms.
//!
class gbtBehavDominanceToolbar : public wxPanel, public gbtGameView {
private:
  wxButton *m_topButton, *m_prevButton, *m_nextButton, *m_allButton;
  wxStaticText *m_level;

  // Overriding gbtGameView members
  void OnUpdate(void);

  // Event handlers
  void OnStrength(wxCommandEvent &);
  void OnTopLevel(wxCommandEvent &);
  void OnPreviousLevel(wxCommandEvent &);
  void OnNextLevel(wxCommandEvent &);
  void OnLastLevel(wxCommandEvent &);
  void OnShowReachable(wxCommandEvent &);

public:
  gbtBehavDominanceToolbar(wxWindow *p_parent, gbtGameDocument *p_doc);
  virtual ~gbtBehavDominanceToolbar() { }
};

#include "bitmaps/next.xpm"
#include "bitmaps/prev.xpm"
#include "bitmaps/tobegin.xpm"
#include "bitmaps/toend.xpm"

gbtBehavDominanceToolbar::gbtBehavDominanceToolbar(wxWindow *p_parent,
						   gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc)
{
  wxBoxSizer *topSizer = new wxBoxSizer(wxHORIZONTAL);

  topSizer->Add(new wxStaticText(this, wxID_STATIC,
				 wxT("Hide actions which are ")),
		0, wxALL | wxALIGN_CENTER, 5);

  wxString domChoices[] = { wxT("strictly"), wxT("strictly or weakly") };
  wxChoice *choice = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize,
				  2, domChoices);
  choice->SetSelection(0);
  Connect(choice->GetId(), wxEVT_COMMAND_CHOICE_SELECTED,
	  wxCommandEventHandler(gbtBehavDominanceToolbar::OnStrength));
  topSizer->Add(choice, 0, wxALL | wxALIGN_CENTER, 5);

  topSizer->Add(new wxStaticText(this, wxID_STATIC, wxT("dominated:")),
		0, wxALL | wxALIGN_CENTER, 5);

  m_topButton = new wxBitmapButton(this, -1, wxBitmap(tobegin_xpm));
  m_topButton->SetToolTip(_("Show all strategies"));
  Connect(m_topButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtBehavDominanceToolbar::OnTopLevel));
  topSizer->Add(m_topButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_prevButton = new wxBitmapButton(this, -1, wxBitmap(prev_xpm));
  m_prevButton->SetToolTip(_("Previous round of elimination"));
  Connect(m_prevButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtBehavDominanceToolbar::OnPreviousLevel));
  topSizer->Add(m_prevButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_level = new wxStaticText(this, wxID_STATIC, 
			     wxT("All actions shown"),
			     wxDefaultPosition, wxDefaultSize,
			     wxALIGN_CENTER | wxST_NO_AUTORESIZE);
  topSizer->Add(m_level, 0, wxALL | wxALIGN_CENTER, 5);

  m_nextButton = new wxBitmapButton(this, -1, wxBitmap(next_xpm));
  m_nextButton->SetToolTip(_("Next round of elimination"));
  Connect(m_nextButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtBehavDominanceToolbar::OnNextLevel));
  topSizer->Add(m_nextButton, 0, wxALL | wxALIGN_CENTER, 5);

  m_allButton = new wxBitmapButton(this, -1, wxBitmap(toend_xpm));
  m_allButton->SetToolTip(_("Eliminate iteratively"));
  Connect(m_allButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED,
	  wxCommandEventHandler(gbtBehavDominanceToolbar::OnLastLevel));
  topSizer->Add(m_allButton, 0, wxALL | wxALIGN_CENTER, 5);

  wxCheckBox *showReachable = new wxCheckBox(this, -1,
					     wxT("Show only reachable nodes"));
  showReachable->SetValue(m_doc->GetStyle().RootReachable());
  Connect(showReachable->GetId(), wxEVT_COMMAND_CHECKBOX_CLICKED,
	  wxCommandEventHandler(gbtBehavDominanceToolbar::OnShowReachable));
  topSizer->Add(showReachable, 0, wxALL | wxALIGN_CENTER, 5);

  SetSizer(topSizer);
  Layout();
}

void gbtBehavDominanceToolbar::OnStrength(wxCommandEvent &p_event)
{
  m_doc->SetBehavElimStrength(p_event.GetSelection() == 0);
}

void gbtBehavDominanceToolbar::OnTopLevel(wxCommandEvent &)
{
  m_doc->TopBehavElimLevel();
}

void gbtBehavDominanceToolbar::OnPreviousLevel(wxCommandEvent &)
{
  m_doc->PreviousBehavElimLevel();
}

void gbtBehavDominanceToolbar::OnNextLevel(wxCommandEvent &)
{
  m_doc->NextBehavElimLevel();
}

void gbtBehavDominanceToolbar::OnLastLevel(wxCommandEvent &)
{
  while (m_doc->NextBehavElimLevel());
}

void gbtBehavDominanceToolbar::OnShowReachable(wxCommandEvent &)
{
  m_doc->GetStyle().SetRootReachable(!m_doc->GetStyle().RootReachable());
  m_doc->UpdateViews(GBT_DOC_MODIFIED_NONE);
}

void gbtBehavDominanceToolbar::OnUpdate(void)
{
  m_topButton->Enable(m_doc->GetBehavElimLevel() > 1);
  m_prevButton->Enable(m_doc->GetBehavElimLevel() > 1);
  m_nextButton->Enable(m_doc->CanBehavElim());
  m_allButton->Enable(m_doc->CanBehavElim());
  if (m_doc->GetBehavElimLevel() == 1) {
    m_level->SetLabel(wxT("All actions shown"));
  }
  else if (m_doc->GetBehavElimLevel() == 2) {
    m_level->SetLabel(wxT("Eliminated 1 level")); 
  }
  else {
    m_level->SetLabel(wxString::Format(wxT("Eliminated %d levels"), 
				       m_doc->GetBehavElimLevel()-1));
  }
  GetSizer()->Layout();
}


//=====================================================================
//              Implementation of class gbtEfgPanel
//=====================================================================

BEGIN_EVENT_TABLE(gbtEfgPanel, wxPanel)
  EVT_MENU(GBT_MENU_TOOLS_DOMINANCE, gbtEfgPanel::OnToolsDominance)
  EVT_MENU(GBT_MENU_VIEW_ZOOMIN, gbtEfgPanel::OnViewZoomIn)
  EVT_MENU(GBT_MENU_VIEW_ZOOMOUT, gbtEfgPanel::OnViewZoomOut)
  EVT_MENU(GBT_MENU_VIEW_ZOOM100, gbtEfgPanel::OnViewZoom100)
  EVT_MENU(GBT_MENU_VIEW_ZOOMFIT, gbtEfgPanel::OnViewZoomFit)
END_EVENT_TABLE()

gbtEfgPanel::gbtEfgPanel(wxWindow *p_parent, gbtGameDocument *p_doc)
  : wxPanel(p_parent, -1), gbtGameView(p_doc)
{
  m_treeWindow = new gbtEfgDisplay(this, m_doc);
  //  m_playerToolbar = new gbtTreePlayerToolbar(this, m_doc);
  m_dominanceToolbar = new gbtBehavDominanceToolbar(this, m_doc);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_dominanceToolbar, 0, wxEXPAND, 0);
  topSizer->Show(m_dominanceToolbar, false);

  wxBoxSizer *treeSizer = new wxBoxSizer(wxHORIZONTAL);
  //  treeSizer->Add(m_playerToolbar, 0, wxEXPAND, 0);
  treeSizer->Add(m_treeWindow, 1, wxEXPAND, 0);
  
  topSizer->Add(treeSizer, 1, wxEXPAND, 0);
  SetSizer(topSizer);
  Layout();
}

void gbtEfgPanel::OnToolsDominance(wxCommandEvent &p_event)
{
  GetSizer()->Show(m_dominanceToolbar, p_event.IsChecked(), true);
  GetSizer()->Layout();
}

void gbtEfgPanel::OnViewZoomIn(wxCommandEvent &)
{
  int zoom = m_treeWindow->GetZoom();
  if (zoom < 150)  zoom += 10;
  m_treeWindow->SetZoom(zoom);
}

void gbtEfgPanel::OnViewZoomOut(wxCommandEvent &)
{
  int zoom = m_treeWindow->GetZoom();
  if (zoom > 10)  zoom -= 10;
  m_treeWindow->SetZoom(zoom);
}

void gbtEfgPanel::OnViewZoom100(wxCommandEvent &)
{
  m_treeWindow->SetZoom(100);
}

void gbtEfgPanel::OnViewZoomFit(wxCommandEvent &)
{
  m_treeWindow->FitZoom();
}

class gbtEfgPrintout : public wxPrintout {
private:
  gbtEfgPanel *m_efgPanel;
    
public:
  gbtEfgPrintout(gbtEfgPanel *p_efgPanel, const wxString &p_label)
    : wxPrintout(p_label), m_efgPanel(p_efgPanel) { }
  virtual ~gbtEfgPrintout() { }

  bool OnPrintPage(int)
  { m_efgPanel->RenderGame(*GetDC(), 50, 50);  return true; }
  bool HasPage(int page) { return (page <= 1); }
  void GetPageInfo(int *minPage, int *maxPage,
		   int *selPageFrom, int *selPageTo)
  { *minPage = 1; *maxPage = 1; *selPageFrom = 1; *selPageTo = 1; }
};

wxPrintout *gbtEfgPanel::GetPrintout(void)
{
  return new gbtEfgPrintout(this,
			    wxString(m_doc->GetGame()->GetTitle().c_str(),
				     *wxConvCurrent));
}

wxBitmap gbtEfgPanel::GetBitmap(int p_marginX, int p_marginY)
{
  if (m_treeWindow->GetLayout().MaxX() > 65000 ||
      m_treeWindow->GetLayout().MaxY() > 65000) {
    // This is just too huge to export to graphics
    return wxNullBitmap;
  }

  wxMemoryDC dc;
  wxBitmap bitmap(m_treeWindow->GetLayout().MaxX() + 2 * p_marginX,
		  m_treeWindow->GetLayout().MaxY() + 2 * p_marginY);
  dc.SelectObject(bitmap);
  RenderGame(dc, p_marginX, p_marginY);
  return bitmap;
}

void gbtEfgPanel::GetSVG(const wxString &p_filename,
			 int p_marginX, int p_marginY)
{
  // The size of the image to be drawn
  int maxX = m_treeWindow->GetLayout().MaxX();
  int maxY = m_treeWindow->GetLayout().MaxY();

  wxSVGFileDC dc(p_filename, maxX + 2*p_marginX, maxY + 2*p_marginY);
  // For some reason, this needs to be initialized
  dc.SetLogicalScale(1.0, 1.0);
  RenderGame(dc, p_marginX, p_marginY);
}

void gbtEfgPanel::RenderGame(wxDC &p_dc, int p_marginX, int p_marginY)
{
  // The size of the image to be drawn
  int maxX = m_treeWindow->GetLayout().MaxX();
  int maxY = m_treeWindow->GetLayout().MaxY();

  // Get the size of the DC in pixels
  wxCoord w, h;
  p_dc.GetSize(&w, &h);

  // Calculate a scaling factor
  double scaleX = (double) w / (double) (maxX + 2*p_marginX);
  double scaleY = (double) h / (double) (maxY + 2*p_marginY);
  double scale = (scaleX < scaleY) ? scaleX : scaleY;
  // Never zoom in
  if (scale > 1.0)  scale = 1.0;
  p_dc.SetUserScale(scale, scale);

  // Calculate the position on the DC to center the tree
  double posX = (double) ((w - (maxX * scale)) / 2.0);
  double posY = (double) ((h - (maxY * scale)) / 2.0);
  p_dc.SetDeviceOrigin((int) posX, (int) posY);

  printf("Drawing with scale %f\n", scale);

  // Draw!
  m_treeWindow->OnDraw(p_dc, scale);
}
