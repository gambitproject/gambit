//
// FILE: efgprint.cc -- Printout class for extensive forms
//
// $Id$
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "efgprint.h"
#include "math/math.h"

EfgPrintout::EfgPrintout(TreeWindow *t, const char *title)
  : wxPrintout((char *) title), tree(t)
{ }

bool EfgPrintout::OnPrintPage(int)
{
  // this is funky--I am playing around w/ the
  // different zoom settings.  So the zoom setting in draw_settings does not
  // equal to the zoom setting in the printer!
  wxDC *dc = GetDC();
  if (!dc) return false;
    
  dc->SetBackgroundMode(wxTRANSPARENT);
    
  int win_w, win_h;
  tree->GetClientSize(&win_w, &win_h);    // that is the size of the window
  // Now we have to check in case our real page size is reduced
  // (e.g. because we're drawing to a print preview memory DC)
  int pageWidth, pageHeight;
  wxCoord w, h;
  dc->GetSize(&w, &h);
  GetPageSizePixels(&pageWidth, &pageHeight);
  float pageScaleX = (float)w/pageWidth;
  float pageScaleY = (float)h/pageHeight;
  
  if (true) { // fit to page
    int maxX = tree->m_layout.MaxX();
    int maxY = tree->m_layout.MaxY();
    // Figure out the 'fake' window zoom
    float zoom_x = (float)win_w/(float)maxX, zoom_y = (float)win_h/(float)maxY;
    float real_zoom = gmin(zoom_x, zoom_y);

    // Figure out the 'real' printer zoom
    int ppiPrinterX, ppiPrinterY;
    GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);
    // Make the margins.  They are just 1" on all sides now.
    float marginX = 1*ppiPrinterX;
    float marginY = 1*ppiPrinterY;
        
    zoom_x = (float)((pageWidth-2*marginX)/(float)maxX)*pageScaleX;
    zoom_y = (float)((pageHeight-2*marginY)/(float)maxY)*pageScaleY;
    real_zoom = gmin(zoom_x, zoom_y);
        
    dc->SetUserScale(real_zoom, real_zoom);
    dc->SetDeviceOrigin(marginX*pageScaleX, marginY*pageScaleY);
  }
  else {  // WYSIWYG
    int ppiScreenX, ppiScreenY;
    GetPPIScreen(&ppiScreenX, &ppiScreenY);
    int ppiPrinterX, ppiPrinterY;
    GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);
        
    // This scales the DC so that the printout roughly represents the
    // the screen scaling. The text point size _should_ be the right size
    // but in fact is too small for some reason. This is a detail that will
    // need to be addressed at some point but can be fudged for the
    // moment.
    float scaleX = (float)((float)ppiPrinterX/(float)ppiScreenX);
    float scaleY = (float)((float)ppiPrinterY/(float)ppiScreenY);
    
    // If printer pageWidth == current DC width, then this doesn't
    // change. But w might be the preview bitmap width, so scale down.
    float overallScaleX = scaleX * pageScaleX;
    float overallScaleY = scaleY * pageScaleY;
    dc->SetUserScale(overallScaleX, overallScaleY);
    
    // Make the margins.  They are just 1" on all sides now.
    float marginX = 1*ppiPrinterX, marginY = 1*ppiPrinterY;
    dc->SetDeviceOrigin(marginX*pageScaleX, marginY*pageScaleY);
    // Figure out the 'fake' window zoom
    float real_width = (pageWidth-2*marginX)/scaleX;
    float real_height = (pageHeight-2*marginY)/scaleY;
    float zoom_x = win_w/real_width, zoom_y = win_h/real_height;
    float real_zoom = gmax(zoom_x, zoom_y);
    dc->SetUserScale(real_zoom, real_zoom);
  }
    
  tree->OnDraw(*dc);
    
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
  num_pages = 1;
  *minPage = 0;
  *maxPage = num_pages;
  *selPageFrom = 0;
  *selPageTo = 0;
}


