//
// FILE: efgprint.h -- Declaration of printout class for extensive forms
//
// $Id$
//

#ifndef EFGPRINT_H
#define EFGPRINT_H

#include "wx/wx.h"
#include "wx/print.h"

#include "treewin.h"

class EfgPrintout : public wxPrintout {
private:
  TreeWindow *tree;
  int num_pages;
    
public:
  EfgPrintout(TreeWindow *s, const char *title = "ExtensivePrintout");
  virtual ~EfgPrintout() { }

  bool OnPrintPage(int page);
  bool HasPage(int page);
  bool OnBeginDocument(int startPage, int endPage);
  void GetPageInfo(int *minPage, int *maxPage,
		   int *selPageFrom, int *selPageTo);
};

#endif  // EFGPRINT_H
