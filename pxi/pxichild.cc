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
#include "gui/dlabout.h"

BEGIN_EVENT_TABLE(PxiChild, wxFrame)
  //  EVT_MENU(PXI_LOAD_FILE, PxiChild::On)
  EVT_MENU(wxID_PRINT_SETUP, PxiChild::OnFilePageSetup)
  EVT_MENU(wxID_PREVIEW, PxiChild::OnFilePrintPreview)
  EVT_MENU(wxID_PRINT, PxiChild::OnFilePrint)
  EVT_MENU(PXI_CHILD_CLOSE, PxiChild::OnCloseWindow)
  EVT_MENU(PXI_CHILD_QUIT, PxiChild::OnQuit)
  EVT_MENU(PXI_DATA_OVERLAY_DATA, PxiChild::OnOverlayData)
  EVT_MENU(PXI_DATA_OVERLAY_FILE, PxiChild::OnOverlayFile)
  EVT_MENU(PXI_FILE_DETAIL, PxiChild::OnFileDetail)
  EVT_MENU(PXI_DISPLAY_OPTIONS, PxiChild::OnDisplayOptions)
  EVT_MENU(PXI_PREFS_FONT_AXIS, PxiChild::OnPrefsFontAxis)
  EVT_MENU(PXI_PREFS_FONT_LABEL, PxiChild::OnPrefsFontLabel)
  EVT_MENU(PXI_PREFS_FONT_OVERLAY, PxiChild::OnPrefsFontOverlay)
  EVT_MENU(PXI_PREFS_ZOOM_IN, PxiChild::OnPrefsZoomIn)
  EVT_MENU(PXI_PREFS_ZOOM_OUT, PxiChild::OnPrefsZoomOut)
  //  EVT_MENU(PXI_PREFS_SCALE, PxiChild::OnPrefsScaleMenu)
  EVT_MENU_RANGE(PXI_PREFS_SCALE_1, PXI_PREFS_SCALE_8, PxiChild::OnPrefsScale)
  EVT_MENU(PXI_PREFS_COLORS, PxiChild::OnPrefsColors)
  EVT_MENU(wxID_ABOUT, PxiChild::OnHelpAbout)
  EVT_MENU(wxID_HELP_CONTENTS, PxiChild::OnHelpContents)
  EVT_SIZE(PxiChild::OnSize)
  EVT_CLOSE(PxiChild::OnCloseWindow)
  EVT_CHAR_HOOK(PxiChild::OnChar)
  EVT_LEFT_DOWN(PxiChild::OnEvent)
END_EVENT_TABLE()

void PxiChild::OnQuit(wxCommandEvent &)
{
  GetParent()->Close();
}

void PxiChild::OnOverlayData(wxCommandEvent &)
{
  //  dialogOverlayData dialog(this, canvas);
  //  canvas->Render();
  //  wxClientDC dc(this);
  //  canvas->Update(dc,PXI_UPDATE_SCREEN);
}

void PxiChild::OnOverlayFile(wxCommandEvent &)
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

void PxiChild::OnFileDetail(wxCommandEvent &)
{
  //  canvas->ShowDetail();
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
#ifdef NOT_PORTED_YET
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
#endif  // NOT_PORTED_YET
}

void PxiChild::OnFilePrint(wxCommandEvent &)
{
#ifdef NOT_PORTED_YET
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
#endif  // NOT_PORTED_YET
}

void PxiChild::OnPrefsColors(wxCommandEvent &)
{
  wxColourData data;
  wxColourDialog dialog(this, &data);
 
  if (dialog.ShowModal() == wxID_OK) {
    // dialog.GetColourData().GetColour();
  }
}

void PxiChild::OnPrefsFontLabel(wxCommandEvent &)
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

void PxiChild::OnPrefsFontAxis(wxCommandEvent &)
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

void PxiChild::OnPrefsFontOverlay(wxCommandEvent &)
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

void PxiChild::OnPrefsZoomOut(wxCommandEvent &event)
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

void PxiChild::OnPrefsZoomIn(wxCommandEvent &event)
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

void PxiChild::OnPrefsScale(wxCommandEvent &event)
{
#ifdef NOT_PORTED_YET
  double scale = scaleValues[event.GetSelection() - PXI_PREFS_SCALE_1+1];
  canvas->SetScale(scale);
  MarkScaleMenu();
  canvas->Render();
#endif  // NOT_PORTED_YET
}

void PxiChild::MarkScaleMenu(void)
{
#ifdef NOT_PORTED_YET
  for(int i=0;i<8;i++) {
    GetMenuBar()->Check(PXI_PREFS_SCALE_1+i,false);
    if(canvas->GetScale() == scaleValues[i+1])
      GetMenuBar()->Check(PXI_PREFS_SCALE_1+i,true);
  }
#endif  // NOT_PORTED_YET
}

void PxiChild::OnHelpAbout(wxCommandEvent &)
{
  dialogAbout dialog(this, "About PXI...",
		     "PXI Quantal Response Plotting Program",
		     "Version 0.97 (alpha)");
  dialog.ShowModal();
}

void PxiChild::OnHelpContents(wxCommandEvent &)
{
}

void PxiChild::OnDisplayOptions(wxCommandEvent &)
{
#ifdef NOT_PORTED_YET
  canvas->DrawSettings()->SetOptions(this);
  canvas->Render();
#endif  // NOT_PORTED_YET
}

PxiChild::PxiChild(PxiFrame *p_parent, const wxString &p_filename) :
  wxFrame(p_parent, -1, p_filename, wxPoint(0,0), wxSize(480,480)),
  parent(p_parent), m_fileHeader(p_filename)
{
  scaleValues=gBlock<double>(8);
  for(int i=1;i<=scaleValues.Length();i++)
    scaleValues[i]=.25*i;

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

  wxNotebook *plotBook = new wxNotebook(this, -1, wxDefaultPosition,
					wxDefaultSize, wxNB_BOTTOM);
  
  for (int i = 1; i <= m_fileHeader.NumInfosets(); i++) {
    PxiCanvas *canvas = new PxiCanvas(plotBook,
				      wxPoint(0, 0), wxSize(width, height),
				      m_fileHeader, i);
    plotBook->AddPage(canvas, wxString::Format("Plot %d", i));
  }

  MarkScaleMenu();
  Show(true);
}

PxiChild::~PxiChild(void)
{ }

void PxiChild::MakeMenus(void)
{
  wxMenu *fileMenu = new wxMenu;
  fileMenu->Append(PXI_LOAD_FILE, "&Open", "Open data file");
  fileMenu->Append(PXI_CHILD_CLOSE, "&Close", "Close child window");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_PRINT_SETUP, "Page Se&tup",
		   "Set up preferences for printing");
  fileMenu->Append(wxID_PREVIEW, "Print Pre&view", 
		   "View a preview of the printout");
  fileMenu->Append(wxID_PRINT, "Print", "Print graphs");
  fileMenu->AppendSeparator();
  fileMenu->Append(wxID_EXIT, "E&xit\tCtrl-X", "Exit PXI");

  wxMenu *data_menu= new wxMenu;
  data_menu->Append(PXI_DATA_OVERLAY_DATA, "&Overlay Data", "Overlay experiment data on plot");
  data_menu->Append(PXI_DATA_OVERLAY_FILE, "&Overlay File", "Overlay another pxi file");
  
  wxMenu *prefs_menu = new wxMenu;
  wxMenu *prefs_font_menu = new wxMenu;
  wxMenu *prefs_scale_menu = new wxMenu;

  prefs_font_menu->Append(PXI_PREFS_FONT_AXIS,"&Axes", "Change Axes Font");
  prefs_font_menu->Append(PXI_PREFS_FONT_LABEL,"&Label", "Change Lable Font");
  prefs_font_menu->Append(PXI_PREFS_FONT_OVERLAY,"&Overlay", "Change Overlay Font");

  prefs_menu->Append(PXI_PREFS_FONTS, "&Fonts", prefs_font_menu,
		     "Set display fonts");

  prefs_menu->Append(PXI_PREFS_COLORS,"&Colors", "Change Colors");

 
  for(int i=0;i<8;i++) {
    wxString tmp;
    tmp.Printf("%4.2f",scaleValues[i+1]);
    prefs_scale_menu->Append(PXI_PREFS_SCALE_1+i,tmp,tmp,true);
  }  
  prefs_menu->Append(PXI_PREFS_SCALE, "&Scale", prefs_scale_menu,
		     "Set display scale");

  prefs_menu->AppendSeparator();
  prefs_menu->Append(PXI_PREFS_ZOOM_OUT,"Zoom &Out", "Zoom Out");
  prefs_menu->Append(PXI_PREFS_ZOOM_IN,"Zoom &In", "Zoom In");
  
  wxMenu *help_menu = new wxMenu;
  help_menu->Append(wxID_HELP_CONTENTS, "&Contents", "Table of contents");
  help_menu->Append(wxID_HELP_INDEX, "&Index", "Index of help file");
  help_menu->AppendSeparator();
  help_menu->Append(wxID_ABOUT, "&About", "About PXI");

  wxMenu *display_menu = new wxMenu;
  display_menu->Append(PXI_FILE_DETAIL,"&Details", "File Details");
  display_menu->Append(PXI_DISPLAY_OPTIONS,"&Options...", "Change plot options");
  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(fileMenu, "&File");
  menu_bar->Append(data_menu, "&Data");
  menu_bar->Append(display_menu, "&Display");
  menu_bar->Append(prefs_menu, "&Prefs");
  menu_bar->Append(help_menu, "&Help");

  // Associate the menu bar with the frame
  SetMenuBar(menu_bar);
}

void PxiChild::MakeToolbar(void)
{
  wxToolBar *toolBar = CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_DOCKABLE |
				     wxTB_HORIZONTAL);

#ifdef __WXMSW__
  wxBitmap printBitmap("PRINT_BITMAP");
  wxBitmap zoominBitmap("ZOOMIN_BITMAP");
  wxBitmap zoomoutBitmap("ZOOMOUT_BITMAP");
  wxBitmap helpBitmap("HELP_BITMAP");
  wxBitmap optionsBitmap("OPTIONS_BITMAP");
  wxBitmap inspectBitmap("INSPECT_BITMAP");
#else
#include "../sources/bitmaps/print.xpm"
#include "../sources/bitmaps/zoomin.xpm"
#include "../sources/bitmaps/zoomout.xpm"
#include "../sources/bitmaps/help.xpm"
#include "../sources/bitmaps/options.xpm"
#include "../sources/bitmaps/inspect.xpm"
  wxBitmap printBitmap(print_xpm);
  wxBitmap zoominBitmap(zoomin_xpm);
  wxBitmap zoomoutBitmap(zoomout_xpm);
  wxBitmap helpBitmap(help_xpm);
  wxBitmap optionsBitmap(options_xpm);
  wxBitmap inspectBitmap(inspect_xpm);
#endif  // __WXMSW__
    
  toolBar->SetMargins(4, 4);

  toolBar->AddTool(wxID_PREVIEW, printBitmap, "", "");
  toolBar->AddSeparator();
  toolBar->AddTool(PXI_DISPLAY_OPTIONS, inspectBitmap, "", "");
  toolBar->AddSeparator();
  toolBar->AddTool(PXI_PREFS_ZOOM_IN, zoominBitmap, "", "");
  toolBar->AddTool(PXI_PREFS_ZOOM_OUT, zoomoutBitmap, "", "");
  toolBar->AddTool(PXI_PREFS_COLORS, optionsBitmap, "", "");
  toolBar->AddSeparator();
  toolBar->AddTool(wxID_HELP_CONTENTS, helpBitmap, "", "");

  toolBar->Realize();
}

