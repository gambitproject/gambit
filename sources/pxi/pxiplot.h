//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of PXI plot base class
//

#ifndef PXIPLOT_H
#define PXIPLOT_H

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

class PxiTitleProperties {
public:
  wxString m_title;
  wxFont m_font;
  wxColour m_color;
};

class PxiLegendProperties {
public:
  bool m_showLegend;
  wxFont m_font;
  wxColour m_color;
};

class PxiPlot : public wxScrolledWindow {
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

protected:
  const FileHeader &m_header;
  PxiDrawSettings m_drawSettings;
  gBlock<label_struct> labels;          // labels for generic text
  const ExpData &m_expData;             // reference to experimental data
  bool m_landscape;                     // landscap mode if true
  int m_width, m_height;                // width, height of page
  double m_scale;                        // scaling factor
  wxMemoryDC *m_dc;        // stored DC
  int m_page;
  PxiAxisProperties m_lambdaAxisProp, m_probAxisProp;
  PxiTitleProperties m_titleProp;
  PxiLegendProperties m_legendProp;

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

  virtual void DoPlot(wxDC& dc,const PlotInfo &thisplot,
		      int x0, int y0, int cw,int ch, int level=1) = 0;
  void PlotLabels(wxDC &dc,int ch,int cw);

  void DrawToken(wxDC &dc, int x, int y, int st);

  // Event handlers
  void OnPaint(wxPaintEvent &);

public:
  PxiPlot(wxWindow *p_parent, const wxPoint &p_position,
	  const wxSize &p_size, const FileHeader &, int p_page,
	  const ExpData &p_expData);
  virtual ~PxiPlot();

  void Update(wxDC& dc,int device);

  void Render(void);

  //  wxString PxiName(void) const { return m_header.FileName(); }
  const FileHeader &Header(void) { return m_header; }

  // Interface to property classes
  PxiDrawSettings &DrawSettings(void) { return m_drawSettings; }
  PxiAxisProperties &GetLambdaAxisProperties(void) { return m_lambdaAxisProp; }
  PxiAxisProperties &GetProbAxisProperties(void) { return m_probAxisProp; }
  PxiTitleProperties &GetTitleProperties(void) { return m_titleProp; }
  PxiLegendProperties &GetLegendProperties(void) { return m_legendProp; }

  DECLARE_EVENT_TABLE()
};

#endif  // PXIPLOT_H

