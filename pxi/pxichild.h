//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to child plot window
//

#ifndef PXICHILD_H
#define PXICHILD_H

#include "pxi.h"
#include "pxicanvas.h"

class PxiChild : public wxFrame {
private:
  PxiFrame *m_parent;
  FileHeader m_fileHeader;
  wxNotebook *m_plotBook;

  wxPrintData m_printData;
  wxPageSetupData m_pageSetupData;

  void OnFileOpen(wxCommandEvent &);
  void OnFilePageSetup(wxCommandEvent &);
  void OnFilePrintPreview(wxCommandEvent &);
  void OnFilePrint(wxCommandEvent &); 
  void OnFileExit(wxCommandEvent &);

  void OnViewDetail(wxCommandEvent &);
  void OnViewOptions(wxCommandEvent &);
  void OnViewZoomIn(wxCommandEvent &);
  void OnViewZoomOut(wxCommandEvent &);
  void OnViewZoom(wxCommandEvent &);

  void OnDataOverlayData(wxCommandEvent &);
  void OnDataOverlayFile(wxCommandEvent &);

  void OnFormatLambdaAxis(wxCommandEvent &);
  void OnFormatProfileAxis(wxCommandEvent &);
  void OnFormatTitle(wxCommandEvent &);
  void OnFormatLegend(wxCommandEvent &);
  void OnFormatOverlay(wxCommandEvent &);
  void OnFormatColors(wxCommandEvent &);

  void OnHelpContents(wxCommandEvent &);
  void OnHelpIndex(wxCommandEvent &);
  void OnHelpAbout(wxCommandEvent &);

  void OnChar(wxKeyEvent &ev) { /* canvas->OnChar(ev); */ }
  void OnEvent(wxMouseEvent &ev) { /* canvas->OnEvent(ev); */ }

  void MakeMenus(void);
  void MakeToolbar(void);

  PxiCanvas *GetShownCanvas(void) const;

public:
  PxiChild(PxiFrame *p_parent, const wxString &p_title);
  ~PxiChild(void);

  DECLARE_EVENT_TABLE()
};

#endif  // PXICHILD_H

