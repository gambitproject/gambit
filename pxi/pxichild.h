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
  PxiFrame *parent;
  PxiCanvas *canvas;

  wxPrintData m_printData;
  wxPageSetupData m_pageSetupData;
  gBlock<double>  scaleValues;

  void OnFilePageSetup(wxCommandEvent &);
  void OnFilePrintPreview(wxCommandEvent &);
  void OnFilePrint(wxCommandEvent &); 

  void OnQuit(wxCommandEvent &);
  void OnOverlayData(wxCommandEvent &);
  void OnOverlayFile(wxCommandEvent &);
  void OnFileDetail(wxCommandEvent &);
  void OnFileOutput(wxCommandEvent &);
  void OnDisplayOptions(wxCommandEvent &);
  void OnPrefsFontAxis(wxCommandEvent &);
  void OnPrefsFontLabel(wxCommandEvent &);
  void OnPrefsFontOverlay(wxCommandEvent &);
  void OnPrefsScale(wxCommandEvent &);
  void MarkScaleMenu(void);
  void OnPrefsColors(wxCommandEvent &);
  void OnPrefsZoomIn(wxCommandEvent &);
  void OnPrefsZoomOut(wxCommandEvent &);
  void OnNextPage(wxCommandEvent &);
  void OnPreviousPage(wxCommandEvent &);
  void OnHelpAbout(wxCommandEvent &);
  void OnHelpContents(wxCommandEvent &);
  void OnChar(wxKeyEvent &ev) {canvas->OnChar(ev);}
  void OnEvent(wxMouseEvent &ev) {canvas->OnEvent(ev);}

  void MakeMenus(void);
  void MakeToolbar(void);

public:
  PxiChild(PxiFrame *p_parent, const wxString &p_title);
  ~PxiChild(void);

  void  LoadFile(const wxString &file) {parent->LoadFile(file);}

  DECLARE_EVENT_TABLE()
};

#endif  // PXICHILD_H
