/////////////////////////////////////////////////////////////////////////////
// Name:        plotdraw.h
// Purpose:     wxPlotDrawer and friends
// Author:      John Labenski
// Modified by:
// Created:     6/5/2002
// Copyright:   (c) John Labenski
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PLOTDRAW_H_
#define _WX_PLOTDRAW_H_

#include "wx/defs.h"
#include "wx/geometry.h"
#include "wx/bitmap.h"
#include "wx/plotctrl/plotdefs.h"
#include "wx/plotctrl/plotcurv.h"
#include "wx/plotctrl/plotdata.h"
#include "wx/plotctrl/plotfunc.h"
#include "wx/wxthings/range.h"

class WXDLLEXPORT wxDC;

class WXDLLIMPEXP_THINGS wxRangeIntSelection;
class WXDLLIMPEXP_THINGS wxRangeDoubleSelection;
class WXDLLIMPEXP_THINGS wxArrayRangeIntSelection;
class WXDLLIMPEXP_THINGS wxArrayRangeDoubleSelection;

class WXDLLIMPEXP_PLOTCTRL wxPlotCtrl;

class WXDLLIMPEXP_PLOTCTRL wxPlotCurve;
class WXDLLIMPEXP_PLOTCTRL wxPlotData;
class WXDLLIMPEXP_PLOTCTRL wxPlotFunction;
class WXDLLIMPEXP_PLOTCTRL wxPlotMarker;

//-----------------------------------------------------------------------------
// wxPlotDrawerBase
//-----------------------------------------------------------------------------

class wxPlotDrawerBase : public wxObject
{
public:
    wxPlotDrawerBase(wxPlotCtrl* owner) : wxObject(),
        m_owner(owner), m_pen_scale(1), m_font_scale(1) {}

    virtual void Draw(wxDC* dc, bool refresh) = 0;

    // Get/Set the owner plotctrl
    wxPlotCtrl* GetOwner() const { return m_owner; }
    void SetOwner(wxPlotCtrl* owner) { m_owner = owner; }

    // Get/Get the rect in the DC to draw on
    void SetDCRect(const wxRect& rect) { m_dcRect = rect; }
    const wxRect& GetDCRect() const { return m_dcRect; }

    // Get/Set the rect of the visible area in the plot window
    void SetPlotViewRect(const wxRect2DDouble& rect) { m_plotViewRect = rect; }
    const wxRect2DDouble& GetPlotViewRect() const { return m_plotViewRect; }

    // Get/Set the scaling for drawing, fonts, pens, etc are scaled
    void   SetPenScale(double scale) { m_pen_scale = scale; }
    double GetPenScale() const { return m_pen_scale; }
    void   SetFontScale(double scale) { m_font_scale = scale; }
    double GetFontScale() const { return m_font_scale; }

protected:
    wxPlotCtrl*  m_owner;
    wxRect         m_dcRect;
    wxRect2DDouble m_plotViewRect;
    double         m_pen_scale;    // width scaling factor for pens
    double         m_font_scale;   // scaling factor for font sizes

private:
    DECLARE_ABSTRACT_CLASS(wxPlotDrawerBase);
};

//-----------------------------------------------------------------------------
// wxPlotDrawerArea
//-----------------------------------------------------------------------------

class wxPlotDrawerArea : public wxPlotDrawerBase
{
public:
    wxPlotDrawerArea(wxPlotCtrl* owner) : wxPlotDrawerBase(owner) {}

    virtual void Draw(wxDC *dc, bool refresh);

private:
    DECLARE_ABSTRACT_CLASS(wxPlotDrawerArea);
};

//-----------------------------------------------------------------------------
// wxPlotDrawerAxisBase
//-----------------------------------------------------------------------------

class wxPlotDrawerAxisBase : public wxPlotDrawerBase
{
public:
    wxPlotDrawerAxisBase(wxPlotCtrl* owner);

    virtual void Draw(wxDC *dc, bool refresh) = 0;

    void SetTickFont( const wxFont& font ) { m_tickFont = font; }
    void SetLabelFont( const wxFont& font ) { m_labelFont = font; }

    void SetTickColour( const wxGenericColour& colour ) { m_tickColour = colour; }
    void SetLabelColour( const wxGenericColour& colour ) { m_labelColour = colour; }

    void SetTickPen( const wxGenericPen& pen ) { m_tickPen = pen; }
    void SetBackgroundBrush( const wxGenericBrush& brush ) { m_backgroundBrush = brush; }

    void SetTickPositions( const wxArrayInt& pos ) { m_tickPositions = pos; }
    void SetTickLabels( const wxArrayString& labels ) { m_tickLabels = labels; }

    void SetLabel( const wxString& label ) { m_label = label; }

    // implementation
    wxArrayInt    m_tickPositions;
    wxArrayString m_tickLabels;

    wxString m_label;

    wxFont          m_tickFont;
    wxFont          m_labelFont;
    wxGenericColour m_tickColour;
    wxGenericColour m_labelColour;

    wxGenericPen    m_tickPen;
    wxGenericBrush  m_backgroundBrush;

private:
    DECLARE_ABSTRACT_CLASS(wxPlotDrawerAxisBase);
};

//-----------------------------------------------------------------------------
// wxPlotDrawerXAxis
//-----------------------------------------------------------------------------

class wxPlotDrawerXAxis : public wxPlotDrawerAxisBase
{
public:
    wxPlotDrawerXAxis(wxPlotCtrl* owner) : wxPlotDrawerAxisBase(owner) {}

    virtual void Draw(wxDC *dc, bool refresh);

private:
    DECLARE_ABSTRACT_CLASS(wxPlotDrawerXAxis);
};

//-----------------------------------------------------------------------------
// wxPlotDrawerYAxis
//-----------------------------------------------------------------------------

class wxPlotDrawerYAxis : public wxPlotDrawerAxisBase
{
public:
    wxPlotDrawerYAxis(wxPlotCtrl* owner) : wxPlotDrawerAxisBase(owner) {}

    virtual void Draw(wxDC *dc, bool refresh);

private:
    DECLARE_ABSTRACT_CLASS(wxPlotDrawerYAxis);
};

//-----------------------------------------------------------------------------
// wxPlotDrawerKey
//-----------------------------------------------------------------------------

class wxPlotDrawerKey : public wxPlotDrawerBase
{
public:
    wxPlotDrawerKey(wxPlotCtrl* owner);

    virtual void Draw(wxDC *WXUNUSED(dc), bool WXUNUSED(refresh)) {} // unused
    virtual void Draw(wxDC *dc, const wxString& keyString);

    void SetFont(const wxFont& font) { m_font = font; }
    void SetFontColour(const wxGenericColour& colour) { m_fontColour = colour; }

    void SetKeyPosition(const wxPoint& pos) { m_keyPosition = pos; }

    // implementation
    wxFont          m_font;
    wxGenericColour m_fontColour;

    wxPoint m_keyPosition;
    bool    m_key_inside;
    int     m_border;
    int     m_key_line_width;  // length of line to draw for curve
    int     m_key_line_margin; // margin between line and key text

private:
    DECLARE_ABSTRACT_CLASS(wxPlotDrawerKey);
};

//-----------------------------------------------------------------------------
// wxPlotDrawerCurve
//-----------------------------------------------------------------------------

class wxPlotDrawerCurve : public wxPlotDrawerBase
{
public:
    wxPlotDrawerCurve(wxPlotCtrl* owner) : wxPlotDrawerBase(owner) {}

    virtual void Draw(wxDC *WXUNUSED(dc), bool WXUNUSED(refresh)) {} // unused
    virtual void Draw(wxDC *dc, wxPlotCurve *curve, int curve_index);

private:
    DECLARE_ABSTRACT_CLASS(wxPlotDrawerCurve);
};

//-----------------------------------------------------------------------------
// wxPlotDrawerDataCurve
//-----------------------------------------------------------------------------

class wxPlotDrawerDataCurve : public wxPlotDrawerBase
{
public:
    wxPlotDrawerDataCurve(wxPlotCtrl* owner) : wxPlotDrawerBase(owner) {}

    virtual void Draw(wxDC *WXUNUSED(dc), bool WXUNUSED(refresh)) {} // unused
    virtual void Draw(wxDC *dc, wxPlotData* plotData, int curve_index);

private:
    DECLARE_ABSTRACT_CLASS(wxPlotDrawerDataCurve);
};

//-----------------------------------------------------------------------------
// wxPlotDrawerMarkers
//-----------------------------------------------------------------------------

class wxPlotDrawerMarker : public wxPlotDrawerBase
{
public:
    wxPlotDrawerMarker(wxPlotCtrl* owner) : wxPlotDrawerBase(owner) {}

    virtual void Draw(wxDC *WXUNUSED(dc), bool WXUNUSED(refresh)) {} // unused
    virtual void Draw(wxDC *dc, const wxArrayPlotMarker& markers);
    virtual void Draw(wxDC *dc, const wxPlotMarker& marker);

private:
    DECLARE_ABSTRACT_CLASS(wxPlotDrawerMarker);
};

#endif // _WX_PLOTDRAW_H_
