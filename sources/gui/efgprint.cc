//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of class to print out extensive forms
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "efgprint.h"
#include "math/gmath.h"

EfgPrintout::EfgPrintout(TreeWindow *p_treeWindow, const wxString &p_title)
  : wxPrintout(p_title), m_treeWindow(p_treeWindow)
{ }

//
// Does the actual work of printing, by computing an appropriate scaling
// for a full-page printout.
// This code is based on the code in the printing sample from wxWindows
//
bool EfgPrintout::OnPrintPage(int)
{
  wxDC *dc = GetDC();
  if (!dc) return false;
    
  dc->SetBackgroundMode(wxTRANSPARENT);
   
  // The actual size of the tree, in pixels
  int maxX = m_treeWindow->m_layout.MaxX();
  int maxY = m_treeWindow->m_layout.MaxY();

  // Margins
  int marginX = 50;
  int marginY = 50;

  maxX += 2 * marginX;
  maxY += 2 * marginY;

  // Get the size of the DC in pixels
  wxCoord w, h;
  dc->GetSize(&w, &h);

  // Calculate a scaling factor
  float scaleX = (float) w / (float) maxX;
  float scaleY = (float) h / (float) maxY;

  float actualScale = (scaleX < scaleY) ? scaleX : scaleY;

  // Calculate the position on the DC to center the tree
  float posX = (float) ((w - (m_treeWindow->m_layout.MaxX() * actualScale)) / 2.0);
  float posY = (float) ((h - (m_treeWindow->m_layout.MaxY() * actualScale)) / 2.0);

  // Set the scale and origin
  dc->SetUserScale(actualScale, actualScale);
  dc->SetDeviceOrigin((long) posX, (long) posY);

  // Draw!
  m_treeWindow->OnDraw(*dc, actualScale);
    
  return true;
}

bool EfgPrintout::HasPage(int page)
{
  return (page <= 1);
}

bool EfgPrintout::OnBeginDocument(int startPage, int endPage)
{
  if (!wxPrintout::OnBeginDocument(startPage, endPage))
    return false;
    
  return true;
}

// Since we can not get at the actual device context in this function, we
// have no way to tell how many pages will be used in the wysiwyg mode. So,
// we have no choice but to disable the From:To page selection mechanism.
void EfgPrintout::GetPageInfo(int *minPage, int *maxPage,
			      int *selPageFrom, int *selPageTo)
{
  *minPage = 1;
  *maxPage = 1;
  *selPageFrom = 1;
  *selPageTo = 1;
}


