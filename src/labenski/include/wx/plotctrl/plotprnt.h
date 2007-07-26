/////////////////////////////////////////////////////////////////////////////
// Name:        plotprnt.h
// Purpose:     wxPlotPrintout
// Author:      John Labenski
// Modified by:
// Created:     6/5/2002
// Copyright:   (c) John Labenski
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PLOTPRINT_H_
#define _WX_PLOTPRINT_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "plotprnt.h"
#endif

#include "wx/print.h"
#include "wx/plotctrl/plotdefs.h"

class WXDLLIMPEXP_PLOTCTRL wxPlotCtrl;

//-----------------------------------------------------------------------------
// wxPlotPrintout - simple mechanism to print the contents of the wxPlotCtrl
//                  on a single page
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_PLOTCTRL wxPlotPrintout : public wxPrintout
{
public:
    wxPlotPrintout(wxPlotCtrl* plotWin, const wxString &title = wxEmptyString);
    bool OnPrintPage(int page);
    bool HasPage(int page) { return page == 1; }
    bool OnBeginDocument(int startPage, int endPage);

    void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
    {
        if (minPage)     *minPage = 1;
        if (maxPage)     *maxPage = 1;
        if (selPageFrom) *selPageFrom = 1;
        if (selPageTo)   *selPageTo = 1;
    }

    // Simplified methods to show the standard print dialogs
    bool ShowPrintDialog();
    bool ShowPrintPreviewDialog(const wxString& frameTitle = wxT("Plot print preview"));
    bool ShowPrintSetupDialog();
    bool ShowPrintPageSetupDialog();

    wxPlotCtrl *GetPlotCtrl() const { return m_plotWin; }

    // Get/Set the wxPrintData/wxPageSetupData that will be used
    //   if NULL then one will be created on first use and deleted when the
    //   program exits. It may make sense to create a single instance of your
    //   printdata in the wxApp for all your program's printing needs and use
    //   SetPrintData/PageSetupData to share it with this.
    static wxPrintData *GetPrintData(bool create_on_demand = false);
    static wxPageSetupData *GetPageSetupData(bool create_on_demand = false);
    static bool GetPrintDataStatic();
    static bool GetPageSetupDataStatic();
    // Set the wxPrintData/PageSetupData, if !is_static then it will be deleted
    //   when the program exits
    static void SetPrintData( wxPrintData *printData, bool is_static );
    static void SetPageSetupData( wxPageSetupData *pageSetupData, bool is_static );

protected:
    wxPlotCtrl *m_plotWin;

    static wxPrintData *s_wxPlotPrintData;
    static wxPageSetupData *s_wxPlotPageSetupData;
    static bool s_wxPlotPrintdata_static;
    static bool s_wxPlotPagesetupdata_static;

private:
    DECLARE_ABSTRACT_CLASS(wxPlotPrintout)
};

#endif // _WX_PLOTPRINT_H_
