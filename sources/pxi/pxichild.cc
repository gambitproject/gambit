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

#include "guishare/valinteger.h"
#include "guishare/valnumber.h"

#include "pxichild.h"
#include "pxiplotn.h"

#include "dleditdata.h"
#include "dlformataxis.h"
#include "dlformattitle.h"
#include "dlformatlegend.h"
#include "overlay.h"
#include "series.h"

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
  EVT_MENU(PXI_SERIES_OVERLAY, PxiChild::OnSeriesOverlay)
  EVT_MENU(PXI_DATA_LOAD, PxiChild::OnDataLoad)
  EVT_MENU(PXI_DATA_SAVE, PxiChild::OnDataSave)
  EVT_MENU(PXI_DATA_EDIT, PxiChild::OnDataEdit)
  EVT_MENU(PXI_DATA_FIT, PxiChild::OnDataFit)
  EVT_MENU(PXI_FORMAT_LAMBDA_AXIS, PxiChild::OnFormatLambdaAxis)
  EVT_MENU(PXI_FORMAT_PROFILE_AXIS, PxiChild::OnFormatProfileAxis)
  EVT_MENU(PXI_FORMAT_TITLE, PxiChild::OnFormatTitle)
  EVT_MENU(PXI_FORMAT_LEGEND, PxiChild::OnFormatLegend)
  EVT_MENU(PXI_FORMAT_SERIES, PxiChild::OnFormatSeries)
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
  m_parent(p_parent)
{
  try {
    gFileInput file(p_filename);
    m_fileHeader.ReadFile(file);
  }
  catch (...) { }

  m_expData = ExpData(m_fileHeader.NumStrategies());
  
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
    PxiPlot *plot = new PxiPlotN(m_plotBook,
				   wxPoint(0, 0), wxSize(width, height),
				   m_fileHeader, i, m_expData);
    m_plotBook->AddPage(plot, wxString::Format("Plot %d", i));
    plot->Render();
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
  
  wxMenu *seriesMenu = new wxMenu;
  seriesMenu->Append(PXI_SERIES_OVERLAY, "&Overlay", "Overlay another file");

  wxMenu *dataMenu = new wxMenu;
  dataMenu->Append(PXI_DATA_LOAD, "&Load", "Load experimental data");
  dataMenu->Append(PXI_DATA_SAVE, "&Save", "Save experimental data");
  dataMenu->Append(PXI_DATA_EDIT, "&Edit", "Edit experimental data");
  dataMenu->AppendSeparator();
  dataMenu->Append(PXI_DATA_FIT, "&Fit", "Compute maximum likelihood "
		   "estimates");
  
  wxMenu *formatMenu = new wxMenu;
  formatMenu->Append(PXI_FORMAT_LAMBDA_AXIS, "Lambda &axis",
		     "Format lambda (horizontal) axis");
  formatMenu->Append(PXI_FORMAT_PROFILE_AXIS, "&Profile axis",
		     "Format profile (vertical) axis");
  formatMenu->Append(PXI_FORMAT_TITLE, "&Title", "Format graph title");
  formatMenu->Append(PXI_FORMAT_LEGEND, "&Legend", "Format graph legend");
  formatMenu->Append(PXI_FORMAT_SERIES, "&Series", "Format series");
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
  menuBar->Append(seriesMenu, "&Series");
  menuBar->Append(dataMenu, "&Data");
  menuBar->Append(formatMenu, "&Format");
  menuBar->Append(helpMenu, "&Help");

  // Associate the menu bar with the frame
  SetMenuBar(menuBar);
}

#include "../sources/bitmaps/open.xpm"
#include "../sources/bitmaps/preview.xpm"
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
  toolBar->AddTool(wxID_PREVIEW, wxBITMAP(preview), wxNullBitmap, false,
		   -1, -1, 0, "Preview", "View a preview of the printout");
  toolBar->AddTool(wxID_PRINT, wxBITMAP(print), wxNullBitmap, false,
		   -1, -1, 0, "Print", "Print this graph");
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
//              class PxiChild: General private members
//=========================================================================

PxiPlot *PxiChild::GetShownPlot(void) const
{
  return ((PxiPlot *) m_plotBook->GetPage(m_plotBook->GetSelection()));
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
  PxiPlot *plot = GetShownPlot();

  wxPrintDialogData data(m_printData);
  wxPrintPreview *preview 
    = new wxPrintPreview(new PxiPrintout(*plot, "PXI printout"), 
			 new PxiPrintout(*plot, "PXI printout"), &data);

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
  PxiPlot *plot = GetShownPlot();

  wxPrintDialogData data(m_printData);
  wxPrinter printer(&data);
  PxiPrintout printout(*plot, "PXI printout");

  if (!printer.Print(this, &printout, TRUE)) {
    if (wxPrinter::GetLastError() == wxPRINTER_ERROR) {
      wxMessageBox("There was an error in printing", "Error", wxOK);
    }
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
  /*
  message += "Detail for: ";
  message += m_fileHeader.FileName();
  message += "\n";
  */

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
  /*
  message += wxString::Format("Data type: %s\n",
			      (m_fileHeader.DataType() == DATA_TYPE_ARITH) ? "Arithmetic" : "Logarithmic");
  */
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

void PxiChild::OnViewZoomIn(wxCommandEvent &)
{
  PxiPlot *plot = GetShownPlot();
  double scale = plot->GetScale();
  scale += 0.1;
  if (scale > 2.0) {
    scale = 2.0;
  }
  plot->SetScale(scale);
  plot->Render();
}

void PxiChild::OnViewZoomOut(wxCommandEvent &)
{
  PxiPlot *plot = GetShownPlot();
  double scale = plot->GetScale();
  scale -= 0.1;
  if (scale < 0.1) {
    scale = 0.1;
  }
  plot->SetScale(scale);
  plot->Render();
}

class dialogZoom : public wxDialog {
private:
  wxTextCtrl *m_data;
  wxString m_value;

public:
  dialogZoom(wxWindow *p_parent, int p_default, int p_min, int p_max);
  
  int GetValue(void) const { return atoi(m_data->GetValue()); }
};

dialogZoom::dialogZoom(wxWindow *p_parent, int p_default, int p_min, int p_max)
  : wxDialog(p_parent, -1, "Set zoom")
{
  wxBoxSizer *dataSizer = new wxBoxSizer(wxHORIZONTAL);
  dataSizer->Add(new wxStaticText(this, wxID_STATIC, "Zoom factor (10-200)"),
		 0, wxALL, 5);
  m_value = wxString::Format("%d", p_default);
  m_data = new wxTextCtrl(this, -1, m_value,
			  wxDefaultPosition, wxDefaultSize, 0,
			  gIntegerValidator(&m_value, p_min, p_max),
			  "zoom factor");
  dataSizer->Add(m_data, 0, wxALL, 5);

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(dataSizer, 0, wxALL | wxCENTER, 0);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 0);

  SetAutoLayout(true);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();
}

void PxiChild::OnViewZoom(wxCommandEvent &)
{
  PxiPlot *plot = GetShownPlot();
  double scale = plot->GetScale();
  
  dialogZoom dialog(this, (int) (scale * 100), 10, 200);
  if (dialog.ShowModal() == wxID_OK) {
    plot->SetScale((double) dialog.GetValue() / 100.0);
    plot->Render();
  }
}

void PxiChild::OnViewOptions(wxCommandEvent &)
{
#ifdef NOT_PORTED_YET
  plot->DrawSettings()->SetOptions(this);
  plot->Render();
#endif  // NOT_PORTED_YET
}

//-------------------------------------------------------------------------
//                        Series menu handlers
//-------------------------------------------------------------------------

void PxiChild::OnSeriesOverlay(wxCommandEvent &)
{
#ifdef NOT_PORTED_YET
  char *s=copystring(wxFileSelector("Load Overlay",NULL,NULL,NULL,"*.out"));
  if (s) {
    FileHeader temp_header(s);
    if ( (temp_header.NumStrategies()!=(plot->Header(1)).NumStrategies()) ||
	 (temp_header.NumInfosets()!=(plot->Header(1)).NumInfosets()) )
      wxMessageBox("These data files do not\nhave the same structure!");
    else
      plot->AppendHeader(temp_header);
  }
  wxClientDC dc(this);
  plot->Update(dc,PXI_UPDATE_SCREEN);
#endif  // NOT_PORTED_YET
}

//-------------------------------------------------------------------------
//                        Data menu handlers
//-------------------------------------------------------------------------

void PxiChild::OnDataLoad(wxCommandEvent &)
{
  wxFileDialog dialog(this, "Choose data file", "", m_expDatafile, "*.agg");
  
  if (dialog.ShowModal() == wxID_OK) {
    m_expDatafile = dialog.GetPath();
    try {
      gFileInput file(m_expDatafile);
      m_expData.LoadData(file);
    }
    catch (...) {
      return;
    }

    for (int i = 0; i < m_plotBook->GetPageCount(); i++) {
      ((PxiPlot *) m_plotBook->GetPage(i))->Render();
    }
  }
}

void PxiChild::OnDataEdit(wxCommandEvent &)
{
  dialogEditData dialog(this, m_expData);

  if (dialog.ShowModal() == wxID_OK) {
    m_expData = dialog.GetData();
    for (int i = 0; i < m_plotBook->GetPageCount(); i++) {
      ((PxiPlot *) m_plotBook->GetPage(i))->Render();
    }
  }
}

void PxiChild::OnDataSave(wxCommandEvent &)
{
  wxFileDialog dialog(this, "Choose data file", "", m_expDatafile, "*.agg",
		      wxSAVE | wxOVERWRITE_PROMPT);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_expDatafile = dialog.GetPath();
    try {
      gFileOutput file(m_expDatafile);
      m_expData.SaveData(file);
    }
    catch (...) {
      wxMessageBox("There was an error in writing to " + m_expDatafile,
		   "Error", wxOK | wxCENTRE | wxICON_ERROR);
      return;
    }
  }
}

void PxiChild::OnDataFit(wxCommandEvent &)
{
  wxFileDialog dialog(this, "Choose output file", 
		      wxPathOnly(m_expDatafile), "", "*.out",
		      wxSAVE | wxOVERWRITE_PROMPT);

  if (dialog.ShowModal() == wxID_OK) {
    try {
      gFileOutput file(dialog.GetPath().c_str());
      m_expData.ComputeMLEs(m_fileHeader, file);

    }
    catch (...) {
      wxMessageBox(wxString::Format("There was an error in writing to '%s'",
				    dialog.GetPath().c_str()),
		   "Error", wxOK | wxCENTRE | wxICON_ERROR);
      return;
    }

    for (int i = 0; i < m_plotBook->GetPageCount(); i++) {
      ((PxiPlot *) m_plotBook->GetPage(i))->Render();
    }
  }
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

void PxiChild::OnFormatLambdaAxis(wxCommandEvent &)
{
  PxiPlot *plot = GetShownPlot();
  dialogFormatAxis dialog(this, plot->GetLambdaAxisProperties());

  if (dialog.ShowModal() == wxID_OK) {
    plot->GetLambdaAxisProperties() = dialog.GetProperties();
    plot->Render();
  }
}

void PxiChild::OnFormatProfileAxis(wxCommandEvent &)
{
  PxiPlot *plot = GetShownPlot();
  dialogFormatAxis dialog(this, plot->GetProbAxisProperties());

  if (dialog.ShowModal() == wxID_OK) {
    plot->GetProbAxisProperties() = dialog.GetProperties();
    plot->Render();
  }
}

void PxiChild::OnFormatTitle(wxCommandEvent &)
{
  PxiPlot *plot = GetShownPlot();
  dialogFormatTitle dialog(this, plot->GetTitleProperties());

  if (dialog.ShowModal() == wxID_OK) {
    plot->GetTitleProperties() = dialog.GetProperties();
    plot->Render();
  }
}

void PxiChild::OnFormatLegend(wxCommandEvent &)
{
  PxiPlot *plot = GetShownPlot();
  dialogFormatLegend dialog(this, plot->GetLegendProperties());
  
  if (dialog.ShowModal() == wxID_OK) {
    plot->GetLegendProperties() = dialog.GetProperties();
    plot->Render();
  }
}

void PxiChild::OnFormatSeries(wxCommandEvent &)
{
  PxiPlot *plot = GetShownPlot();
  Series::Dialog dialog(this, plot->GetSeriesProperties());

  if (dialog.ShowModal() == wxID_OK) {
    plot->GetSeriesProperties() = dialog.GetProperties();
    plot->Render();
  }
}

void PxiChild::OnFormatOverlay(wxCommandEvent &)
{
  PxiPlot *plot = GetShownPlot();
  Overlay::Dialog dialog(this, plot->GetOverlayProperties());

  if (dialog.ShowModal() == wxID_OK) {
    plot->GetOverlayProperties() = dialog.GetProperties();
    plot->Render();
  }
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


