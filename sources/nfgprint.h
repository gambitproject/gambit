//
// FILE: nfgprint.h -- Declaration of printout class for normal forms
//
// $Id$
//

#ifndef NFGPRINT_H
#define NFGPRINT_H

#include "wx/wx.h"
#include "wx/print.h"

#include "nfgshow.h"
#include "nfgtable.h"

class NfgPrintout : public wxPrintout {
private:
  NfgTable *m_table;
  int m_numPages;
    
public:
  NfgPrintout(NfgTable *, const char *title = "Normal form printout");
  virtual ~NfgPrintout() { }

  bool OnPrintPage(int page);
  bool HasPage(int page);
  bool OnBeginDocument(int startPage, int endPage);
  void GetPageInfo(int *minPage, int *maxPage,
		   int *selPageFrom, int *selPageTo);
};

#endif  // NFGPRINT_H
