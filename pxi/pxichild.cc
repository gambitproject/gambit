//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of PXI child window
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP
#include "wx/notebook.h"
#include "wx/fontdlg.h"
#include "wx/colordlg.h"
#include "wx/printdlg.h"

#include "pxichild.h"
#include "dlgpxi.h"

//
// wxWindows is "supposed to" pass unhandled menu commands on to
// the parent window.  In 2.2.7, this does not appear to be happening,
// so PxiChild provides pass-through functions to call the corresponding
// parent handlers.
//
BEGIN_EVENT_TABLE(PxiChild, wxFrame)
  EVT_MENU(wxID_OPEN, PxiChild::OnFileOpen)
  EVT_MENU(wxID_CLOSE, PxiChild::OnCloseWindow)
  EVT_MENU(wxID_PRINT_SETUP, PxiChild::OnFilePageSetup)
  EVT_MENU(wxID_PREVIEW, PxiChild::OnFilePrintPreview)
  EVT_MENU(wxID_PRINT, PxiChild::OnFilePrint)
  EVT_MENU(wxID_EXIT, PxiChild::OnFileExit)
  EVT_MENU(PXI_VIEW_DETAIL, PxiChild::OnViewDetail)
  EVT_MENU(PXI_VIEW_OPTIONS, PxiChild::OnViewOptions)
  EVT_MENU(PXI_VIEW_ZOOM_IN, PxiChild::OnViewZoomIn)
  EVT_MENU(PXI_VIEW_ZOOM_OUT, PxiChild::OnViewZoomOut)
  EVT_MENU(PXI_VIEW_ZOOM, PxiChild::OnViewZoom)
  EVT_MENU(PXI_DATA_OVERLAY_DATA, PxiChild::OnDataOverlayData)
  EVT_MENU(PXI_DATA_OVERLAY_FILE, PxiChild::OnDataOverlayFile)
  EVT_MENU(PXI_FORMAT_AXIS, PxiChild::OnFormatAxis)
  EVT_MENU(PXI_FORMAT_LABEL, PxiChild::OnFormatLabel)
  EVT_MENU(PXI_FORMAT_OVERLAY, PxiChild::OnFormatOverlay)
  EVT_MENU(wxID_HELP_CONTENTS, PxiChild::OnHelpContents)
  EVT_MENU(wxID_HELP_INDEX, PxiChild::OnHelpIndex)
  EVT_MENU(wxID_ABOUT, PxiChild::OnHelpAbout)
  EVT_SIZE(PxiChild::OnSize)
  EVT_CLOSE(PxiChild::OnCloseWindow)
  EVT_CHAR_HOOK(PxiChild::OnChar)
  EVT_LEFT_DOWN(PxiChild::OnEvent)
END_EVENT_TABLE()

//=========================================================================
//          class PxiChild: Lifecycle and related auxiliaries
//=========================================================================

PxiChild::PxiChild(PxiFrame *p_parent, const wxString &p_filename) :
  wxFrame(p_parent, -1, p_filename, wxPoint(0,0), wxSize(480,480)),
  m_parent(p_parent), m_fileHeader(p_filename)
{
  SetSizeHints(300, 300);

  // Give the frame an icon
#ifdef __WXMSW__
  SetIcon(wxIcon("pxi_icn"));
#else
#include "pxi.xpm"
  SetIcon(wxIcon(pxi_xpm));
#endif

  CreateStatusBar();
  MakeMenus();
  MakeToolbar();

  int width, height;
  GetClientSize(&width, &height);

  m_plotBook = new wxNotebook(this, -1, wxDefaultPosition,
			      wxDefaultSize, wxNB_BOTTOM);
  
  for (int i = 1; i <= m_fileHeader.NumInfosets(); i++) {
    PxiCanvas *canvas = new PxiCanvas(m_plotBook,
				      wxPoint(0, 0), wxSize(width, height),
				      m_fileHeader, i);
    m_plotBook->AddPage(canvas, wxString::Format("Plot %d", i));
  }

  Show(true);
}

PxiChild::~PxiChild(void)
{ }

void PxiChild::MakeMenus(void)
{
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(wxID_OPEN, "&Open", "Open data file");
  fileMenu->Append(wxID_CLOSE, "&Close", "Close child window");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_PRINT_SETUP, "Page Se&tup",
		   "Set up preferences for printing");
  fileMenu->Append(wxID_PREVIEW, "Print Pre&view", 
		   "View a preview of the printout");
  fileMenu->Append(wxID_PRINT, "Print", "Print graphs");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT, "E&xit\tCtrl-X", "Exit PXI");

  wxMenu *viewMenu = new wxMenu;
  viewMenu->Append(PXI_VIEW_DETAIL, "&Details", "File Details");
  viewMenu->Append(PXI_VIEW_OPTIONS, "&Options...", "Change plot options");
  viewMenu->AppendSeparator();
  viewMenu->Append(PXI_VIEW_ZOOM_IN, "Zoom &In", "Zoom in");
  viewMenu->Append(PXI_VIEW_ZOOM_OUT, "Zoom &Out", "Zoom out");
  viewMenu->Append(PXI_VIEW_ZOOM, "&Zoom", "Set magnification");

  wxMenu *dataMenu = new wxMenu;
  dataMenu->Append(PXI_DATA_OVERLAY_DATA, "Overlay &Data",
		   "Overlay experimental data on plot");
  dataMenu->Append(PXI_DATA_OVERLAY_FILE, "Overlay &File",
		   "Overlay another pxi file");
  
  wxMenu *formatMenu = new wxMenu;
  formatMenu->Append(PXI_FORMAT_AXIS, "&Axis", "Format axes");
  formatMenu->Append(PXI_FORMAT_LABEL,"&Label", "Format label");
  formatMenu->Append(PXI_FORMAT_OVERLAY,"&Overlay", "Format overlay");
  formatMenu->AppendSeparator();
  formatMenu->Append(PXI_FORMAT_COLORS, "&Colors", "Change Colors");

  wxMenu *helpMenu = new wxMenu;
  helpMenu->Append(wxID_HELP_CONTENTS, "&Contents", "Table of contents");
  helpMenu->Append(wxID_HELP_INDEX, "&Index", "Index of help file");
  helpMenu->AppendSeparator();
  helpMenu->Append(wxID_ABOUT, "&About", "About PXI");

  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(fileMenu, "&File");
  menuBar->Append(viewMenu, "&View");
  menuBar->Append(dataMenu, "&Data");
  menuBar->Append(formatMenu, "&Format");
  menuBar->Append(helpMenu, "&Help");

  // Associate the menu bar with the frame
  SetMenuBar(menuBar);
}

#include "../sources/bitmaps/open.xpm"
#include "../sources/bitmaps/print.xpm"
#include "../sources/bitmaps/zoomin.xpm"
#include "../sources/bitmaps/zoomout.xpm"
#include "../sources/bitmaps/help.xpm"

void PxiChild::MakeToolbar(void)
{
  wxToolBar *toolBar = CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_DOCKABLE |
				     wxTB_HORIZONTAL);

  toolBar->SetMargins(4, 4);

  toolBar->AddTool(wxID_OPEN, wxBITMAP(open), wxNullBitmap, false,
		   -1, -1, 0, "Open", "Open a saved datafile");
  toolBar->AddTool(wxID_PREVIEW, wxBITMAP(print), wxNullBitmap, false,
		   -1, -1, 0, "Preview", "View a preview of the printout");
  toolBar->AddSeparator();
  toolBar->AddTool(PXI_VIEW_ZOOM_IN, wxBITMAP(zoomin), wxNullBitmap, false,
		   -1, -1, 0, "Zoom in", "Zoom in");
  toolBar->AddTool(PXI_VIEW_ZOOM_OUT, wxBITMAP(zoomout), wxNullBitmap, false,
		   -1, -1, 0, "Zoom out", "Zoom out");
  toolBar->AddSeparator();
  toolBar->AddTool(wxID_HELP_CONTENTS, wxBITMAP(help), wxNullBitmap, false,
		   -1, -1, 0, "Help", "Table of contents");

  toolBar->Realize();
}

//=========================================================================
//                class PxiChild: Menu event handlers
//=========================================================================

//-------------------------------------------------------------------------
//                        File menu handlers
//-------------------------------------------------------------------------

void PxiChild::OnFileOpen(wxCommandEvent &p_event)
{
  m_parent->OnFileOpen(p_event);
}

void PxiChild::OnFilePageSetup(wxCommandEvent &)
{
  wxPageSetupDialog dialog(this, &m_pageSetupData);
  if (dialog.ShowModal() == wxID_OK) {
    m_printData = dialog.GetPageSetupData().GetPrintData();
    m_pageSetupData = dialog.GetPageSetupData();
  }
}

void PxiChild::OnFilePrintPreview(wxCommandEvent &)
{
  PxiCanvas *canvas =
    (PxiCanvas *) m_plotBook->GetPage(m_plotBook->GetSelection());

  wxPrintDialogData data(m_printData);
  wxPrintPreview *preview 
    = new wxPrintPreview(new PxiPrintout(*canvas, "PXI printout"), 
			 new PxiPrintout(*canvas, "PXI printout"), &data);

  if (!preview->Ok()) {
    delete preview;
    return;
  }
  
  wxPreviewFrame *frame = new wxPreviewFrame(preview, this,  
					     "PXI Print Preview",
					     wxPoint(100, 100),
					     wxSize(600, 650));
  frame->Initialize();
  frame->Show(true);
}

void PxiChild::OnFilePrint(wxCommandEvent &)
{
  PxiCanvas *canvas =
    (PxiCanvas *) m_plotBook->GetPage(m_plotBook->GetSelection());

  wxPrintDialogData data(m_printData);
  wxPrinter printer(&data);
  PxiPrintout printout(*canvas, "PXI printout");

  if (!printer.Print(this, &printout, TRUE)) {
    wxMessageBox("There was an error in printing", "Error", wxOK);
    return;
  }
  else {
    m_printData = printer.GetPrintDialogData().GetPrintData();
  }
}

void PxiChild::OnFileExit(wxCommandEvent &)
{
  wxCloseEvent event;
  m_parent->OnCloseWindow(event);
}

//-------------------------------------------------------------------------
//                        View menu handlers
//-------------------------------------------------------------------------

void PxiChild::OnViewDetail(wxCommandEvent &)
{
  wxString message;

  message += "Detail for: ";
  message += m_fileHeader.FileName();
  message += "\n";

  message += wxString::Format("Error (lambda) step: %4.4f\n",
			      m_fileHeader.EStep());
  message += wxString::Format("Error (lambda) start: %4.4f\n",
			      m_fileHeader.EStart());
  message += wxString::Format("Error (lambda) stop: %4.4f\n",
			      m_fileHeader.EStop());
  message += wxString::Format("Minimum data value: %4.4f\n",
			      m_fileHeader.DataMin());
  message += wxString::Format("Maximum data value: %4.4f\n",
			      m_fileHeader.DataMax());
  message += wxString::Format("Data type: %s\n",
			      (m_fileHeader.DataType() == DATA_TYPE_ARITH) ? "Arithmetic" : "Logarithmic");
  message += "\n";

  if (m_fileHeader.MError() > -.99) {
    message += wxString::Format("Probability step: %4.4f\n",
				m_fileHeader.QStep());
    message += wxString::Format("Margin of error: %4.4f\n",
				m_fileHeader.MError());
  }

  wxMessageDialog dialog(this, message, "File Details", wxOK);
  dialog.ShowModal();
}

void PxiChild::OnViewZoomOut(wxCommandEvent &event)
{
#ifdef NOT_PORTED_YET
  double scale = canvas->GetScale();
  int i=1;
  while(scale > scaleValues[i] && i< scaleValues.Length()) {i++;}
  if(i>1)i--;
  canvas->SetScale(scaleValues[i]);
  MarkScaleMenu();
  canvas->Render();
#endif  // NOT_PORTED_YET
}

void PxiChild::OnViewZoomIn(wxCommandEvent &event)
{
#ifdef NOT_PORTED_YET
  double scale = canvas->GetScale();
  int i=scaleValues.Length();
  while(scale < scaleValues[i] && i>1) {i--;}
  if(i<scaleValues.Length())i++;
  canvas->SetScale(scaleValues[i]);
  MarkScaleMenu();
  canvas->Render()
#endif // NOT_PORTED_YET
}

void PxiChild::OnViewZoom(wxCommandEvent &event)
{
#ifdef NOT_PORTED_YET
  double scale = scaleValues[event.GetSelection() - PXI_PREFS_SCALE_1+1];
  canvas->SetScale(scale);
  MarkScaleMenu();
  canvas->Render();
#endif  // NOT_PORTED_YET
}

void PxiChild::OnViewOptions(wxCommandEvent &)
{
#ifdef NOT_PORTED_YET
  canvas->DrawSettings()->SetOptions(this);
  canvas->Render();
#endif  // NOT_PORTED_YET
}


//-------------------------------------------------------------------------
//                        Data menu handlers
//-------------------------------------------------------------------------

void PxiChild::OnDataOverlayData(wxCommandEvent &)
{
  //  dialogOverlayData dialog(this, canvas);
  //  canvas->Render();
  //  wxClientDC dc(this);
  //  canvas->Update(dc,PXI_UPDATE_SCREEN);
}

void PxiChild::OnDataOverlayFile(wxCommandEvent &)
{
#ifdef NOT_PORTED_YET
  char *s=copystring(wxFileSelector("Load Overlay",NULL,NULL,NULL,"*.out"));
  if (s) {
    FileHeader temp_header(s);
    if ( (temp_header.NumStrategies()!=(canvas->Header(1)).NumStrategies()) ||
	 (temp_header.NumInfosets()!=(canvas->Header(1)).NumInfosets()) )
      wxMessageBox("These data files do not\nhave the same structure!");
    else
      canvas->AppendHeader(temp_header);
  }
  wxClientDC dc(this);
  canvas->Update(dc,PXI_UPDATE_SCREEN);
#endif  // NOT_PORTED_YET
}

//-------------------------------------------------------------------------
//                       Format menu handlers
//-------------------------------------------------------------------------

void PxiChild::OnFormatColors(wxCommandEvent &)
{
  wxColourData data;
  wxColourDialog dialog(this, &data);
 
  if (dialog.ShowModal() == wxID_OK) {
    // dialog.GetColourData().GetColour();
  }
}

void PxiChild::OnFormatLabel(wxCommandEvent &)
{
#ifdef NOT_PORTED_YET
  wxFontData data;
  data.SetInitialFont(canvas->draw_settings->GetLabelFont());  
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    canvas->draw_settings->SetLabelFont(dialog.GetFontData().GetChosenFont());
    canvas->Render();
  }
#endif  // NOT_PORTED_YET
}

void PxiChild::OnFormatAxis(wxCommandEvent &)
{
#ifdef NOT_PORTED_YET
  wxFontData data;
  data.SetInitialFont(canvas->draw_settings->GetAxisFont());  
  wxFontDialog dialog(this, &data);
  if (dialog.ShowModal() == wxID_OK) {
    canvas->draw_settings->SetAxisFont(dialog.GetFontData().GetChosenFont());
    canvas->Render();
  }
#endif  // NOT_PORTED_YET
}

void PxiChild::OnFormatOverlay(wxCommandEvent &)
{
#ifdef NOT_PORTED_YET
  wxFontData data;
  data.SetInitialFont(canvas->draw_settings->GetOverlayFont());  
  wxFontDialog dialog(this, &data);
  
  if (dialog.ShowModal() == wxID_OK) {
    canvas->draw_settings->SetOverlayFont(dialog.GetFontData().GetChosenFont());
    canvas->Render();
  }
#endif  // NOT_PORTED_YET
}

//-------------------------------------------------------------------------
//                       Help menu handlers
//-------------------------------------------------------------------------

void PxiChild::OnHelpContents(wxCommandEvent &p_event)
{
  m_parent->OnHelpContents(p_event);
}

void PxiChild::OnHelpIndex(wxCommandEvent &p_event)
{
  m_parent->OnHelpIndex(p_event);
}

void PxiChild::OnHelpAbout(wxCommandEvent &p_event)
{
  m_parent->OnHelpAbout(p_event);
}


