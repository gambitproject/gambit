//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of PXI graphing canvas window
//

#ifndef PXICANVAS_H
#define PXICANVAS_H

#include "pxi.h"
#include "pxiconf.h"

class PxiAxisScaleProperties {
public:
  wxString m_minimum, m_maximum;
  int m_divisions;
  bool m_useLog, m_canUseLog;

  double GetMinimum(void) const;
  double GetMaximum(void) const;
};

inline double PxiAxisScaleProperties::GetMinimum(void) const
{
  double d;
  m_minimum.ToDouble(&d);
  return d;
}

inline double PxiAxisScaleProperties::GetMaximum(void) const
{
  double d;
  m_maximum.ToDouble(&d);
  return d;
}

class PxiAxisProperties  {
public:
  wxFont m_font;
  wxColour m_color;
  PxiAxisScaleProperties m_scale;
};

class PxiCanvas : public wxScrolledWindow {
friend class PxiChild;
public:
  typedef struct LABELSTRUCT {
    friend gOutput &operator<<(gOutput &op,const LABELSTRUCT &l);
    wxString label;
    double x,y;
    LABELSTRUCT(void):label(""),x(0.0),y(0.0) {}
    LABELSTRUCT &operator=(const LABELSTRUCT &l)
      {label=l.label;x=l.x;y=l.y; return *this;}
    int operator==(const LABELSTRUCT &l)
      {return (label==l.label && x==l.x && y==l.y);}
    int operator!=(const LABELSTRUCT &l)
      {return !(*this==l);}
  } label_struct;

private:
  const FileHeader &m_header;
  PxiDrawSettings m_drawSettings;
  ExpData *exp_data;                    // experimental data overlay
  gBlock<label_struct> labels;          // labels for generic text
  DataLine probs;                       // a line of data
  Bool	stopped;
  double cur_e;
  bool m_landscape;                     // landscap mode if true
  int m_width, m_height;                // width, height of page
  double m_scale;                        // scaling factor
  double m_ppu;                        // pixels per scroll unit
  wxMemoryDC *m_dc;        // stored DC
  int m_page;
  PxiAxisProperties m_lambdaAxisProp, m_probAxisProp;

  int Width(void) const 
    {if(m_landscape) return m_height; return m_width;}
  int Height(void) const 
    {if(m_landscape) return m_width; return m_height;}
  void SetLandscape(bool flag) 
    {m_landscape = flag;}
  bool GetLandscape(void) 
    {return m_landscape;}
  double GetScale(void) const {return m_scale;} 
  void SetScale(double x);

  void DoPlot_X(wxDC& dc,const PlotInfo &thisplot,
		int x0, int y0, int cw,int ch, int level=1);
  void DoPlot_2(wxDC& dc,const PlotInfo &thisplot,
		int x0, int y0, int cw,int ch, int level=1);
  void DoPlot_3(wxDC& dc,const PlotInfo &thisplot,
		int x0, int y0, int cw,int ch, int level=1);

  void PlotData_X(wxDC& dc, const PlotInfo &thisplot, 
		  int x0, int y0, int cw,int ch,
		  const FileHeader &f_header, int level);
  void PlotData_2(wxDC& dc, const PlotInfo &thisplot, 
		  int x0, int y0, int cw,int ch,
		  const FileHeader &f_header, int level=1);
  void PlotData_3(wxDC& dc, const PlotInfo &thisplot, 
		  int x0, int y0, int cw,int ch,
		  const FileHeader &f_header, int level=1);

  void DrawExpPoint_X(wxDC &dc, const PlotInfo &thisplot, 
		      double cur_e,int iset,int st,
		      int x0, int y0, int ch,int cw);
  void DrawExpPoint_2(wxDC &dc, const PlotInfo &thisplot, 
		      double cur_e,int pl1,int st1,
		      int pl2,int st2,int x0, int y0, int cw, int ch);
  
  void DrawExpPoint_3(wxDC &dc,const PlotInfo &thisplot, 
		      double cur_e,int iset,int st1,int st2,
		      int x0, int y0, int cw,int ch);

  void PlotLabels(wxDC &dc,int ch,int cw);


  double CalcY_X(double y,int x0, int ch, const PlotInfo &thisplot);
  double CalcX_X(double x,int y0, int cw, const PlotInfo &thisplot);
  double CalcY_3(double p1,int x0, int y0, int ch,int cw);
  double CalcX_3(double p1,double p2,int x0, int y0, int ch,int cw, const PlotInfo &thisplo);
  
  void DrawToken(wxDC &dc, int x, int y, int st);

  // Event handlers
  void OnPaint(wxPaintEvent &);
  void OnChar(wxKeyEvent &ev);
  void OnEvent(wxMouseEvent &ev);

public:
  PxiCanvas(wxWindow *p_parent, const wxPoint &p_position,
	    const wxSize &p_size, const FileHeader &, int p_page);
  virtual ~PxiCanvas();

  void Update(wxDC& dc,int device);

  void Render(void);
  void SetPage(int page) { }
  void SetNextPage(void) { }
  void SetPreviousPage(void) { }

  void NewExpData(ExpDataParams &P);
  wxString PxiName(void) const { return m_header.FileName(); }
  const FileHeader &Header(void) { return m_header; }

  // Interface to property classes
  PxiDrawSettings &DrawSettings(void) { return m_drawSettings; }
  PxiAxisProperties &GetLambdaAxisProperties(void) { return m_lambdaAxisProp; }
  PxiAxisProperties &GetProbAxisProperties(void) { return m_probAxisProp; }

  DECLARE_EVENT_TABLE()
};

#endif  // PXICANVAS_H

