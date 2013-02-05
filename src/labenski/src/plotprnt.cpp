/////////////////////////////////////////////////////////////////////////////
// Name:        plotprnt.cpp
// Purpose:     wxPlotPrintout
// Author:      John Labenski
// Modified by:
// Created:     8/27/2002
// Copyright:   (c) John Labenski
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/msgdlg.h"
    #include "wx/dcmemory.h"
#endif // WX_PRECOMP

#include "wx/module.h"
#include "wx/printdlg.h"

#include "wx/plotctrl/plotctrl.h"
#include "wx/plotctrl/plotprnt.h"

#define RINT(x) int((x) >= 0 ? ((x) + 0.5) : ((x) - 0.5))

//-----------------------------------------------------------------------------
// wxPlotPrintout
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxPlotPrintout, wxPrintout)

wxPrintData *wxPlotPrintout::s_wxPlotPrintData = NULL;
wxPageSetupData *wxPlotPrintout::s_wxPlotPageSetupData = NULL;
bool wxPlotPrintout::s_wxPlotPrintdata_static = false;
bool wxPlotPrintout::s_wxPlotPagesetupdata_static = false;

wxPrintData *wxPlotPrintout::GetPrintData(bool create_on_demand)
{
    if (create_on_demand && (s_wxPlotPrintData == NULL))
    {
        wxPrintData *printData = new wxPrintData;
        printData->SetPaperId(wxPAPER_LETTER);
        SetPrintData(printData, false);
    }

    return s_wxPlotPrintData;
}
wxPageSetupData *wxPlotPrintout::GetPageSetupData(bool create_on_demand)
{
    if (create_on_demand && (s_wxPlotPageSetupData == NULL))
    {
        wxPageSetupData *pageSetupData = new wxPageSetupData;
        pageSetupData->SetPaperSize(wxPAPER_LETTER);
        pageSetupData->SetMarginTopLeft(wxPoint(20, 20));
        pageSetupData->SetMarginBottomRight(wxPoint(20, 20));
        SetPageSetupData(pageSetupData, false);
    }

    return s_wxPlotPageSetupData;
}

bool wxPlotPrintout::GetPrintDataStatic()     { return s_wxPlotPrintdata_static; }
bool wxPlotPrintout::GetPageSetupDataStatic() { return s_wxPlotPagesetupdata_static; }

void wxPlotPrintout::SetPrintData( wxPrintData *printData, bool is_static )
{
    if (s_wxPlotPrintData && !s_wxPlotPrintdata_static)
        delete s_wxPlotPrintData;

    s_wxPlotPrintData = printData;
    s_wxPlotPrintdata_static = is_static;
}
void wxPlotPrintout::SetPageSetupData( wxPageSetupData *pageSetupData, bool is_static )
{
    if (s_wxPlotPageSetupData && !s_wxPlotPagesetupdata_static)
        delete s_wxPlotPageSetupData;

    s_wxPlotPageSetupData = pageSetupData;
    s_wxPlotPagesetupdata_static = is_static;
}


wxPlotPrintout::wxPlotPrintout( wxPlotCtrl* plotWin, const wxString &title )
               : wxPrintout(title), m_plotWin(plotWin)
{
}

bool wxPlotPrintout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument(startPage, endPage))
        return false;

    return true;
}

bool wxPlotPrintout::OnPrintPage(int page)
{
    wxCHECK_MSG(m_plotWin, false, wxT("Invalid plotctrl"));

    if (page != 1) return false;

    wxPoint pagePix;
    //GetPageSizePixels(&pagePix.x, &pagePix.y); // no good for preview when scaled
    GetDC()->GetSize(&pagePix.x, &pagePix.y);

    wxPoint tl = GetPageSetupData(true)->GetMarginTopLeft();
    wxPoint br = GetPageSetupData(true)->GetMarginBottomRight();

    wxPoint ppi;

    if (IsPreview())
        GetPPIScreen(&ppi.x, &ppi.y);
    else
        GetPPIPrinter(&ppi.x, &ppi.y);

    //wxPrintf(wxT("ppi %d %d dc size %d %d  margin tl %d %d br %d %d\n"), ppi.x, ppi.y, pagePix.x, pagePix.y, tl.x, tl.y, br.x, br.y);

    tl.x = int((tl.x*ppi.x)/25.4);
    tl.y = int((tl.y*ppi.y)/25.4);
    br.x = int((br.x*ppi.x)/25.4);
    br.y = int((br.y*ppi.y)/25.4);

    wxRect rect(tl.x, tl.y, pagePix.x - tl.x - br.x, pagePix.y - tl.y - br.y);

    //wxPrintf(wxT("dc size %d %d  margin tl %d %d br %d %d\n"), rect.width, rect.height, tl.x, tl.y, br.x, br.y);

    int dpi = ppi.x;

    //set dpi of the drawwholeplot function
    if (IsPreview())
    {
        //dpi is the (screen dpi) * (percent of screen used by preview)
        dpi = RINT(ppi.x * ((double)rect.width / pagePix.x));
    }

    m_plotWin->DrawWholePlot( GetDC(), rect, dpi );
    return true;
}

bool wxPlotPrintout::ShowPrintDialog()
{
    wxCHECK_MSG(GetPlotCtrl(), false, wxT("Invalid plot window"));
    wxPrintDialogData printDialogData(*wxPlotPrintout::GetPrintData(true));

    wxPrinter printer(& printDialogData);
    if (!printer.Print(GetPlotCtrl(), this, true))
    {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
            wxMessageBox(wxT("There was a problem printing.\n")
                           wxT("Perhaps your printer is not setup correctly?"),
                         _("Printing"), wxOK, GetPlotCtrl());
    }

    if (wxPrinter::GetLastError() != wxPRINTER_CANCELLED)
        *wxPlotPrintout::GetPrintData(true) = printer.GetPrintDialogData().GetPrintData();

    return wxPrinter::GetLastError() == wxPRINTER_NO_ERROR;
}
bool wxPlotPrintout::ShowPrintPreviewDialog(const wxString& frameTitle)
{
    wxCHECK_MSG(GetPlotCtrl(), false, wxT("Invalid plot window"));
    // Pass two printout objects: for preview, and possible printing.
    wxPrintDialogData printDialogData(*wxPlotPrintout::GetPrintData(true));
    wxPrintPreview *preview = new wxPrintPreview(new wxPlotPrintout(GetPlotCtrl(), GetTitle()),
                                                 new wxPlotPrintout(GetPlotCtrl(), GetTitle()),
                                                 &printDialogData);
    if (!preview->Ok())
    {
        delete preview;
        wxMessageBox(wxT("There was a problem previewing.\n")
                       wxT("Perhaps your printer is not setup correctly?"),
                     _("Previewing"), wxOK, GetPlotCtrl());
        return false;
    }

    wxRect r(wxGetClientDisplayRect());
    r.width  = wxMin(r.width,  600);
    r.height = wxMin(r.height, 650);
    wxPreviewFrame *frame = new wxPreviewFrame(preview, GetPlotCtrl(),
                                               frameTitle,
                                               wxDefaultPosition, r.GetSize());
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show(true);
    return true;
}
bool wxPlotPrintout::ShowPrintSetupDialog()
{
    wxCHECK_MSG(GetPlotCtrl(), false, wxT("Invalid plot window"));
    wxPrintDialogData printDialogData(*wxPlotPrintout::GetPrintData(true));
    wxPrintDialog printerDialog(GetPlotCtrl(), & printDialogData);
#if !wxCHECK_VERSION(2,7,0)
    printerDialog.GetPrintDialogData().SetSetupDialog(true);
#endif //!wxCHECK_VERSION(2,7,0)

    if (printerDialog.ShowModal() != wxID_CANCEL)
    {
        *wxPlotPrintout::GetPrintData(true) = printerDialog.GetPrintDialogData().GetPrintData();
        return true;
    }

    return false;
}
bool wxPlotPrintout::ShowPrintPageSetupDialog()
{
    wxCHECK_MSG(GetPlotCtrl(), false, wxT("Invalid plot window"));
    *wxPlotPrintout::GetPageSetupData(true) = *wxPlotPrintout::GetPrintData();
    wxPageSetupDialog pageSetupDialog(GetPlotCtrl(), wxPlotPrintout::GetPageSetupData());

    if (pageSetupDialog.ShowModal() != wxID_CANCEL)
    {
        *wxPlotPrintout::GetPrintData(true) = pageSetupDialog.GetPageSetupData().GetPrintData();
        *wxPlotPrintout::GetPageSetupData(true) = pageSetupDialog.GetPageSetupData();
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
// wxPlotCtrlModule - setup anything after init and delete before closing
//-----------------------------------------------------------------------------

class wxPlotCtrlModule : public wxModule
{
DECLARE_DYNAMIC_CLASS(wxPlotCtrlModule)
public:
    wxPlotCtrlModule() : wxModule() {}
    bool OnInit()
    {
        return true;
    }
    void OnExit()
    {
        wxPlotPrintout::SetPrintData(NULL, false);
        wxPlotPrintout::SetPageSetupData(NULL, false);
    }
};

IMPLEMENT_DYNAMIC_CLASS(wxPlotCtrlModule, wxModule)
