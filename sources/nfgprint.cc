//
// FILE: nfgprint.cc -- Printout class for normal forms
//
// $Id$
//

#include "nfgprint.h"

NfgPrintout::NfgPrintout(NfgTable *p_table, const char *p_title)
  : wxPrintout((char *) p_title), m_table(p_table)
{ }

bool NfgPrintout::OnPrintPage(int)
{
  wxDC *dc = GetDC();
  if (!dc) return false;
    
  dc->SetBackgroundMode(wxTRANSPARENT);

  int pageWidth, pageHeight;
  wxCoord w, h;
  dc->GetSize(&w, &h);
  GetPageSizePixels(&pageWidth, &pageHeight);
  float pageScaleX = (float)w/pageWidth;
  float pageScaleY = (float)h/pageHeight;

  int ppiPrinterX, ppiPrinterY;
  GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);

  float marginX = 1*ppiPrinterX;
  float marginY = 1*ppiPrinterY;
  dc->SetDeviceOrigin(marginX*pageScaleX, marginY*pageScaleY);

  m_table->DrawGridCellArea(*dc);
  return true;
}

bool NfgPrintout::HasPage(int page)
{
  return (page <= 1);
}

bool NfgPrintout::OnBeginDocument(int startPage, int endPage)
{
  if (!wxPrintout::OnBeginDocument(startPage, endPage))
    return false;
    
  return true;
}

// Since we can not get at the actual device context in this function, we
// have no way to tell how many pages will be used in the wysiwyg mode. So,
// we have no choice but to disable the From:To page selection mechanism.
void NfgPrintout::GetPageInfo(int *minPage, int *maxPage,
			      int *selPageFrom, int *selPageTo)
{
  m_numPages = 1;
  *minPage = 0;
  *maxPage = m_numPages;
  *selPageFrom = 0;
  *selPageTo = 0;
}


