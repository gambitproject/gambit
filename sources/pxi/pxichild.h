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
#include "pxiplot.h"

class PxiChild : public wxFrame {
private:
  PxiFrame *m_parent;
  gBlock<PxiFile *> m_qreFiles;
  wxNotebook *m_plotBook;

  wxString m_expDatafile;
  ExpData m_expData;

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

  void OnSeriesOverlay(wxCommandEvent &);
  void OnSeriesShow(wxCommandEvent &);

  void OnDataLoad(wxCommandEvent &);
  void OnDataSave(wxCommandEvent &);
  void OnDataEdit(wxCommandEvent &);
  void OnDataFit(wxCommandEvent &);

  void OnFormatLambdaAxis(wxCommandEvent &);
  void OnFormatProfileAxis(wxCommandEvent &);
  void OnFormatTitle(wxCommandEvent &);
  void OnFormatLegend(wxCommandEvent &);
  void OnFormatSeries(wxCommandEvent &);
  void OnFormatOverlay(wxCommandEvent &);
  void OnFormatColors(wxCommandEvent &);

  void OnHelpContents(wxCommandEvent &);
  void OnHelpIndex(wxCommandEvent &);
  void OnHelpAbout(wxCommandEvent &);

  void OnChar(wxKeyEvent &ev) { /* canvas->OnChar(ev); */ }
  void OnEvent(wxMouseEvent &ev) { /* canvas->OnEvent(ev); */ }

  void MakeMenus(void);
  void MakeToolbar(void);

  PxiPlot *GetShownPlot(void) const;

public:
  PxiChild(PxiFrame *p_parent, const wxString &);
  ~PxiChild(void);

  const ExpData &GetExpData(void) const { return m_expData; }

  DECLARE_EVENT_TABLE()
};

#endif  // PXICHILD_H

