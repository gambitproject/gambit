//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to printouts for extensive forms 
//

#ifndef EFGPRINT_H
#define EFGPRINT_H

#include "wx/print.h"
#include "treewin.h"

class EfgPrintout : public wxPrintout {
private:
  TreeWindow *m_treeWindow;
    
public:
  EfgPrintout(TreeWindow *, const wxString &);
  virtual ~EfgPrintout() { }

  bool OnPrintPage(int page);
  bool HasPage(int page);
  bool OnBeginDocument(int startPage, int endPage);
  void GetPageInfo(int *minPage, int *maxPage,
		   int *selPageFrom, int *selPageTo);
};

#endif  // EFGPRINT_H
