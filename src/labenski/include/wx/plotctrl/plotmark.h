/////////////////////////////////////////////////////////////////////////////
// Name:        plotmark.h
// Purpose:     wxPlotMarker
// Author:      John Labenski
// Modified by:
// Created:     6/5/2002
// Copyright:   (c) John Labenski
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PLOTMARK_H_
#define _WX_PLOTMARK_H_

#include "wx/bitmap.h"
#include "wx/geometry.h"
#include "wx/plotctrl/plotdefs.h"
#include "wx/wxthings/genergdi.h"

class WXDLLIMPEXP_PLOTCTRL wxPlotCtrl;
class WXDLLIMPEXP_PLOTCTRL wxPlotMarker;

WX_DECLARE_OBJARRAY_WITH_DECL(wxPlotMarker, wxArrayPlotMarker, class WXDLLIMPEXP_PLOTCTRL);

//-----------------------------------------------------------------------------
// wxPlotMarker - a marker to draw in the plot window
//   This is a catch all class, containing info to draw various marker types.
//   Therefore, some parts may not be used for some markers.
//   Use a two part construction to ensure it's created properly
//      wxPlotMarker marker; marker.CreateXXX(...)
//-----------------------------------------------------------------------------

enum wxPlotMarkerType
{
    wxPLOTMARKER_NONE,      // invalid, don't draw it
    wxPLOTMARKER_POINT,     // single pixel point, only position & pen used
                            //   size, brush, bitmap unused
    wxPLOTMARKER_LINE,      // line from upper left to lower right of rect
                            //   rect may be inverted to draw at any angle
                            //   size, brush, bitmap unused
    wxPLOTMARKER_HORIZ_LINE,// horizontal line, full width
                            //   only vert position and pen used
                            //   size, brush, bitmap unused
    wxPLOTMARKER_VERT_LINE, // vertical line, full height
                            //   only horiz position and pen used
                            //   size, brush, bitmap unused
    wxPLOTMARKER_CROSS,     // vertical and horizontal line, full height & width
                            //   position used for center of cross
                            //   size, brush, bitmap unused
    wxPLOTMARKER_RECT,      // rectangle - see plot rect conditions
                            //   pen draws outline and brush fills (if set)
                            //   bitmap unused
                            //   rect drawn centered on position
                            //   size is size of the rect
    wxPLOTMARKER_ELLIPSE,   // ellipse   - see plot rect conditions
    wxPLOTMARKER_BITMAP     // the bitmap is drawn at the position
};

class WXDLLIMPEXP_PLOTCTRL wxPlotMarker : public wxObject
{
public:
    wxPlotMarker() : wxObject() {}
    // Create a full marker (see CreateXXX functions)
    wxPlotMarker(int marker_type,
                 const wxRect2DDouble& rect,
                 const wxSize& size,
                 const wxGenericPen& pen,
                 const wxGenericBrush& brush = wxNullGenericBrush,
                 const wxBitmap& bitmap = wxNullBitmap) : wxObject()
        { Create(marker_type, rect, size, pen, brush, bitmap); }
    // Create a shape marker to be drawn at the point with the given size
    //   in pixels
    wxPlotMarker(int marker_type,
                 const wxPoint2DDouble& pt,
                 const wxSize& size,
                 const wxGenericPen& pen,
                 const wxGenericBrush& brush = wxNullGenericBrush) : wxObject()
        { Create(marker_type, wxRect2DDouble(pt.m_x, pt.m_y, 0, 0), size, pen, brush); }
    // Create a bitmap marker
    wxPlotMarker(const wxPoint2DDouble& pt,
                 const wxBitmap& bitmap) : wxObject()
        { CreateBitmapMarker(pt, bitmap); }

    virtual ~wxPlotMarker() {}

    // is the marker created
    bool Ok() const { return m_refData != NULL; }

    // Generic create function
    void Create(int marker_type, const wxRect2DDouble& rect,
                const wxSize& size, const wxGenericPen& pen,
                const wxGenericBrush& brush = wxNullGenericBrush,
                const wxBitmap& bitmap = wxNullBitmap);

    // Simplified methods (use these)
    void CreatePointMarker(const wxPoint2DDouble& pt,
                           const wxGenericPen& pen)
        { Create(wxPLOTMARKER_POINT, wxRect2DDouble(pt.m_x, pt.m_y, 0, 0), wxSize(-1, -1), pen); }
    void CreateLineMarker(const wxRect2DDouble& rect, const wxGenericPen& pen)
        { Create(wxPLOTMARKER_LINE, rect, wxSize(-1, -1), pen); }
    void CreateHorizLineMarker(double y, const wxGenericPen& pen)
        { Create(wxPLOTMARKER_HORIZ_LINE, wxRect2DDouble(0, y, -1, 0), wxSize(-1, -1), pen); }
    void CreateVertLineMarker(double x, const wxGenericPen& pen)
        { Create(wxPLOTMARKER_VERT_LINE, wxRect2DDouble(x, 0, 0, -1), wxSize(-1, -1), pen); }
    void CreateRectMarker(const wxRect2DDouble& rect,
                          const wxGenericPen& pen,
                          const wxGenericBrush& brush = wxNullGenericBrush)
        { Create(wxPLOTMARKER_RECT, rect, wxSize(-1, -1), pen, brush); }
    void CreateRectMarker(const wxPoint2DDouble& pt,
                          const wxSize& size,
                          const wxGenericPen& pen,
                          const wxGenericBrush& brush = wxNullGenericBrush)
        { Create(wxPLOTMARKER_RECT, wxRect2DDouble(pt.m_x, pt.m_y, 0, 0), size, pen, brush); }
//    void CreateEllipseMarker(const wxRect2DDouble& rect,
//                             const wxGenericPen& pen,
//                             const wxGenericBrush& brush = wxNullGenericBrush)
//        { Create(wxPLOTMARKER_ELLIPSE, rect, wxSize(-1, -1), pen, brush); }
    void CreateEllipseMarker(const wxPoint2DDouble& pt,
                             const wxSize& size,
                             const wxGenericPen& pen,
                             const wxGenericBrush& brush = wxNullGenericBrush)
        { Create(wxPLOTMARKER_ELLIPSE, wxRect2DDouble(pt.m_x, pt.m_y, 0, 0), size, pen, brush); }
    void CreateBitmapMarker(const wxPoint2DDouble& pt,
                            const wxBitmap& bitmap)
        { Create(wxPLOTMARKER_BITMAP, wxRect2DDouble(pt.m_x, pt.m_y, 0, 0), wxSize(-1, -1), wxNullGenericPen, wxNullGenericBrush, bitmap); }

    // Get/Set the marker type
    int  GetMarkerType() const;
    void SetMarkerType(int type);

    // Get/Set the rect to draw the marker into
    //   The meaning of the rect is different for each marker type
    //
    //   Bitmap markers
    //     Drawn centered at upper left if width = height = 0
    //     Drawn into the rect and scaled if width & height != 0
    //
    //   Shape markers
    //     Draws a full rect/ellipse if width & height > 0
    //     Draws a point if width & height = 0
    //     Draws a vertical line if width = 0 & height != 0
    //     Draws a horizontal line if height = 0 & width != 0
    //     Draws the full width if width < 0
    //     Draws the full height if height < 0
    wxRect2DDouble GetPlotRect() const;
    wxRect2DDouble& GetPlotRect();
    void SetPlotRect(const wxRect2DDouble& rect);

    wxPoint2DDouble GetPlotPosition() const;
    void SetPlotPosition(const wxPoint2DDouble& pos);

    // for rect/ellipse markers you can set the size in pixels
    wxSize GetSize() const;
    void SetSize(const wxSize& size);

    // Get/Set the pen to draw the lines with
    wxGenericPen GetPen() const;
    void SetPen(const wxGenericPen& pen);
    // Get/Set the brush to fill the area with (null for none)
    wxGenericBrush GetBrush() const;
    void SetBrush(const wxGenericBrush& brush);
    // Get/Set the bitmap to draw (null for none, ignored if not wxPLOTMARKER_BITMAP)
    wxBitmap GetBitmap() const;
    void SetBitmap(const wxBitmap& bitmap);

    // operators
    bool operator == (const wxPlotMarker& pm) const
        { return m_refData == pm.m_refData; }
    bool operator != (const wxPlotMarker& pm) const
        { return m_refData != pm.m_refData; }

    wxPlotMarker& operator = (const wxPlotMarker& pm)
    {
        if ( (*this) != pm )
            Ref(pm);
        return *this;
    }

private:
    // ref counting code
    virtual wxObjectRefData *CreateRefData() const;
    virtual wxObjectRefData *CloneRefData(const wxObjectRefData *data) const;

    DECLARE_DYNAMIC_CLASS(wxPlotMarker);
};

#endif // _WX_PLOTMARK_H_
