/////////////////////////////////////////////////////////////////////////////
// Name:        plotdraw.cpp
// Purpose:     wxPlotDrawer and friends
// Author:      John Labenski
// Modified by:
// Created:     8/27/2002
// Copyright:   (c) John Labenski
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/dcmemory.h"
#endif // WX_PRECOMP

#include "wx/math.h"

#include "wx/plotctrl/plotctrl.h"
#include "wx/plotctrl/plotdraw.h"
#include "wx/plotctrl/plotmark.h"

#include <cmath>
#include <cfloat>
#include <climits>

// MSVC hogs global namespace with these min/max macros - remove them
#ifdef max
    #undef max
#endif
#ifdef min
    #undef min
#endif
#ifdef GetYValue   // Visual Studio 7 defines this
    #undef GetYValue
#endif

#define LONG_TO_WXCOLOUR(c) wxColour((unsigned char)((c>>16)&0xFF), (unsigned char)((c>>8 )&0xFF), (unsigned char)((c)&0xFF))
#define WXCOLOUR_TO_LONG(c) ((c.Red()<<16)|(c.Green()<<8)|(c.Blue()))

#define RINT(x) int((x) >= 0 ? ((x) + 0.5) : ((x) - 0.5))

#if !wxCHECK_VERSION(2,5,0)
    bool WXRECT2DDOUBLE_EQUAL(const wxRect2DDouble& r1, const wxRect2DDouble& r2)
    {
        return (r1.m_x == r1.m_x) && (r1.m_y == r1.m_y) &&
               (r1.m_width == r1.m_width) && (r1.m_height == r1.m_height);
    }
#else
    #define WXRECT2DDOUBLE_EQUAL(r1, r2) ((r1) == (r2))
#endif // wxCHECK_VERSION(2,5,0)

//-----------------------------------------------------------------------------
// Consts
//-----------------------------------------------------------------------------

// Skip the wxWidgets drawing routines since they calc an unnecessary bounding rect
// You may turn this off by defining wxPLOT_FAST_GRAPHICS=0 to the compilier
#ifndef wxPLOT_FAST_GRAPHICS
    #define wxPLOT_FAST_GRAPHICS 0
#endif // wxPLOT_FAST_GRAPHICS

#if defined(__WXGTK__) && wxPLOT_FAST_GRAPHICS

extern "C" {
    #include <gdk/gdk.h>
}
    #define INITIALIZE_FAST_GRAPHICS \
        double dc_scale_x = 1, dc_scale_y = 1; \
        dc->GetUserScale( &dc_scale_x, &dc_scale_y ); \
        wxPoint dc_origin = dc->GetDeviceOrigin(); \
        wxWindowDC *winDC = wxDynamicCast(dc, wxWindowDC); \
        GdkWindow *window = NULL; \
        GdkGC     *pen = NULL; \
        if (winDC && (dc_scale_x == 1.0) && (dc_scale_y == 1.0) && (dc_origin == wxPoint(0,0))) \
        { \
            window = winDC->m_window; \
            pen = winDC->m_penGC; \
        }

    // inline void wxPLOT_DRAW_LINE(wxDC *dc, GdkWindow *win, GdkGC *pen, int x0, int y0, int x1, int y1)
    #define wxPLOT_DRAW_LINE(dc, win, pen, x0, y0, x1, y1) \
        if (win && pen) \
            gdk_draw_line( win, pen, x0, y0, x1, y1 ); \
        else \
            dc->DrawLine( x0, y0, x1, y1 );

    // note : need to draw outline since the filled part isn't really a circle
    //        gdk_draw_arc( win, pen, false, x0-2, y0-2, 4, 4, 0, 360*64 ); // false for outline, true for inside
    //inline void wxPLOT_DRAW_CIRCLE(wxDC *dc, GdkWindow *win, GdkGC *pen, int x0, int y0)
    #define wxPLOT_DRAW_ELLIPSE(dc, win, pen, x0, y0, w, h) \
        if (win && pen) \
            gdk_draw_arc( win, pen, false, (x0)-(w), (y0)-(h), (w)*2, (h)*2, 0, 360*64 );  \
        else \
            dc->DrawEllipse(x0, y0, w, h);

#elif defined(__WXMSW__) && wxPLOT_FAST_GRAPHICS

    #define INITIALIZE_FAST_GRAPHICS \
        double dc_scale_x = 1, dc_scale_y = 1; \
        dc->GetUserScale( &dc_scale_x, &dc_scale_y ); \
        wxPoint dc_origin = dc->GetDeviceOrigin(); \
        HDC window = 0; \
        if ((dc_scale_x == 1.0) && (dc_scale_y == 1.0) && (dc_origin == wxPoint(0,0))) \
            window = (HDC)dc->GetHDC(); \
        int pen = 0; pen = 0;  // no unused var warning

    //inline void wxPLOT_DRAW_LINE(wxDC *dc, HDC win, int pen, int x0, int y0, int x1, int y1)
    #define wxPLOT_DRAW_LINE(dc, win, pen, x0, y0, x1, y1) \
        if (win) \
        { \
            (void)MoveToEx(win, x0, y0, NULL);  \
            (void)LineTo(win, x1, y1); \
        } \
        else \
            dc->DrawLine( x0, y0, x1, y1 );

    //inline void wxPLOT_DRAW_CIRCLE(wxDC *dc, HDC win, int pen, int x0, int y0)
    #define wxPLOT_DRAW_ELLIPSE(dc, win, pen, x0, y0, w, h) \
        if (win) \
            (void)Ellipse(win, (x0)-(w), (y0)-(w), (x0)+(w)*2, (y0)+(h)*2); \
        else \
            dc->DrawEllipse(x0, y0, w, h);

#else // !wxPLOT_FAST_GRAPHICS or not gtk/msw

    #define INITIALIZE_FAST_GRAPHICS \
        int window = 0; window = 0; \
        int pen = 0; pen = 0;

    //inline void wxPLOT_DRAW_LINE(wxDC *dc, int win, int pen, int x0, int y0, int x1, int y1)
    #define wxPLOT_DRAW_LINE(dc, win, pen, x0, y0, x1, y1) \
        dc->DrawLine( x0, y0, x1, y1 );

    //inline void wxPLOT_DRAW_CIRCLE(wxDC *dc, int win, int pen, int x0, int y0)
    #define wxPLOT_DRAW_ELLIPSE(dc, win, pen, x0, y0, w, h) \
        dc->DrawEllipse(x0, y0, w, h);

#endif // wxPLOT_FAST_GRAPHICS


// differs from wxRect2DDouble::Intersects by allowing for 0 width or height
inline bool wxPlotRect2DDoubleIntersects(const wxRect2DDouble &a, const wxRect2DDouble &b)
{
    return (wxMax(a.m_x, b.m_x) <= wxMin(a.GetRight(), b.GetRight())) &&
           (wxMax(a.m_y, b.m_y) <= wxMin(a.GetBottom(), b.GetBottom()));
}

// same as wxPlotRect2DDouble::Contains, but doesn't convert to wxPoint2DDouble
inline bool wxPlotRect2DDoubleContains(double x, double y, const wxRect2DDouble &rect)
{
    return ((x>=rect.m_x) && (y>=rect.m_y) && (x<=rect.GetRight()) && (y<=rect.GetBottom()));
}

// differs from wxRect2DDouble::GetOutCode by swaping top and bottom for plot origin
//inline wxOutCode wxPlotRect2DDoubleOutCode( double x, double y, const wxRect2DDouble &rect )
//{
//    return wxOutCode((x < rect.m_x         ? wxOutLeft   :
//                     (x > rect.GetRight()  ? wxOutRight  : wxInside )) +
//                     (y < rect.m_y         ? wxOutTop    :
//                     (y > rect.GetBottom() ? wxOutBottom : wxInside )) );
//}
#define wxPlotRect2DDoubleOutCode( x, y, rect ) \
           wxOutCode(((x) < rect.m_x         ? wxOutLeft   : \
                     ((x) > rect.GetRight()  ? wxOutRight  : wxInside )) + \
                     ((y) < rect.m_y         ? wxOutTop    : \
                     ((y) > rect.GetBottom() ? wxOutBottom : wxInside )) )


// modified Cohen-Sutherland Algorithm for line clipping in at most two passes
//   the the original endless loop is too unstable
//   http://www.cc.gatech.edu/grads/h/Hao-wei.Hsieh/Haowei.Hsieh/code1.html for psuedo code
// The line connecting (x0,y0)-(x1,y1) is clipped to rect and which
//   points were clipped is returned.

enum ClipLine_Type
{
    ClippedNeither = 0x0000,
    ClippedFirstX  = 0x0001,
    ClippedFirstY  = 0x0002,
    ClippedFirst   = ClippedFirstX | ClippedFirstY,
    ClippedSecondX = 0x0010,
    ClippedSecondY = 0x0020,
    ClippedSecond  = ClippedSecondX | ClippedSecondY,
    ClippedBoth    = ClippedFirst | ClippedSecond,
    ClippedOut     = 0x0100   // no intersection, so can't clip
};

int ClipLineToRect( double &x0, double &y0,
                    double &x1, double &y1,
                    const wxRect2DDouble &rect )
{
    if (!wxFinite(x0) || !wxFinite(y0) ||
        !wxFinite(x1) || !wxFinite(y1)) return ClippedOut;

    wxOutCode out0 = wxPlotRect2DDoubleOutCode( x0, y0, rect );
    wxOutCode out1 = wxPlotRect2DDoubleOutCode( x1, y1, rect );

    if ((out0 & out1) != wxInside) return ClippedOut;     // both outside on same side
    if ((out0 | out1) == wxInside) return ClippedNeither; // both inside

    int ret = ClippedNeither;

    if (x0 == x1) // vertical line
    {
        if      (out0 & wxOutTop)    {y0 = rect.GetTop();    ret |= ClippedFirstY;}
        else if (out0 & wxOutBottom) {y0 = rect.GetBottom(); ret |= ClippedFirstY;}
        if      (out1 & wxOutTop)    {y1 = rect.GetTop();    ret |= ClippedSecondY;}
        else if (out1 & wxOutBottom) {y1 = rect.GetBottom(); ret |= ClippedSecondY;}
        return ret;
    }
    if (y0 == y1) // horiz line
    {
        if      (out0 & wxOutLeft)  {x0 = rect.GetLeft();  ret |= ClippedFirstX;}
        else if (out0 & wxOutRight) {x0 = rect.GetRight(); ret |= ClippedFirstX;}
        if      (out1 & wxOutLeft)  {x1 = rect.GetLeft();  ret |= ClippedSecondX;}
        else if (out1 & wxOutRight) {x1 = rect.GetRight(); ret |= ClippedSecondX;}
        return ret;
    }

    double x = x0, y = y0;
    wxOutCode out = out0;
    int points_out = 2;
    bool out0_outside = true;
    if (out0 == wxInside)
    {
        out0_outside = false;
        points_out = 1;
        out = out1;
    }
    else if (out1 == wxInside)
        points_out = 1;

    for (int i=0; i<points_out; i++)
    {
        if (out & wxOutTop)
        {
            y = rect.GetTop();
            x = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            out = wxPlotRect2DDoubleOutCode( x, y, rect );
        }
        else if (out & wxOutBottom)
        {
            y = rect.GetBottom();
            x = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            out = wxPlotRect2DDoubleOutCode( x, y, rect );
        }
        // check left and right
        if (out & wxOutRight)
        {
            x = rect.GetRight();
            y = y0 + (y1 - y0) * (x - x0) / (x1 - x0);
            out = wxPlotRect2DDoubleOutCode( x, y, rect );
        }
        else if (out & wxOutLeft)
        {
            x = rect.GetLeft();
            y = y0 + (y1 - y0) * (x - x0) / (x1 - x0);
            out = wxPlotRect2DDoubleOutCode( x, y, rect );
        }
        // check top and bottom again
        if (out & wxOutTop)
        {
            y = rect.GetTop();
            x = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            out = wxPlotRect2DDoubleOutCode( x, y, rect );
        }
        else if (out & wxOutBottom)
        {
            y = rect.GetBottom();
            x = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            out = wxPlotRect2DDoubleOutCode( x, y, rect );
        }

        if (!wxFinite(x) || !wxFinite(y)) return ClippedOut;

        if ((i == 0) && out0_outside)
        {
            x0 = x;
            y0 = y;
            ret |= ClippedFirst;
            out = out1;
        }
        else
        {
            x1 = x;
            y1 = y;
            ret |= ClippedSecond;
            return ret;
        }
    }

    return ret;
}

// ----------------------------------------------------------------------------
// wxWindows spline drawing code see dcbase.cpp - inlined
//
// usage - Create a SplineDrawer, Create(...), DrawSpline(x,y), EndSpline()
// ----------------------------------------------------------------------------

#define SPLINE_STACK_DEPTH 20
#define THRESHOLD           4 // number of pixels between spline points

#define SPLINE_PUSH(x1_, y1_, x2_, y2_, x3_, y3_, x4_, y4_) \
    stack_top->x1 = x1_; stack_top->y1 = y1_; \
    stack_top->x2 = x2_; stack_top->y2 = y2_; \
    stack_top->x3 = x3_; stack_top->y3 = y3_; \
    stack_top->x4 = x4_; stack_top->y4 = y4_; \
    stack_top++; \
    m_stack_count++;

#define SPLINE_POP(x1_, y1_, x2_, y2_, x3_, y3_, x4_, y4_) \
    stack_top--; \
    m_stack_count--; \
    x1_ = stack_top->x1; y1_ = stack_top->y1; \
    x2_ = stack_top->x2; y2_ = stack_top->y2; \
    x3_ = stack_top->x3; y3_ = stack_top->y3; \
    x4_ = stack_top->x4; y4_ = stack_top->y4;

class SplineDrawer
{
public:
    SplineDrawer() : m_dc(NULL) { }
    // the wxRect2DDouble rect is the allowed dc area in pixel coords
    // wxRangeDoubleSelection is the ranges to use selPen, also in pixel coords
    // x1_, y1_, x2_, y2_ are the first 2 points to draw
    void Create(wxDC *dc, const wxPen &curPen, const wxPen &selPen,
                const wxRect2DDouble &rect, wxRangeDoubleSelection *rangeSel,
                double x1_, double y1_, double x2_, double y2_)
    {
        m_dc = dc;
        wxCHECK_RET( dc, wxT("invalid window dc") );

        m_selPen   = selPen;
        m_curPen   = curPen;
        m_rangeSel = rangeSel;

        m_rect = rect;

        m_stack_count = 0;

        m_x1 = x1_;
        m_y1 = y1_;
        m_x2 = x2_;
        m_y2 = y2_;
        m_cx1 = (m_x1  + m_x2) / 2.0;
        m_cy1 = (m_y1  + m_y2) / 2.0;
        m_cx2 = (m_cx1 + m_x2) / 2.0;
        m_cy2 = (m_cy1 + m_y2) / 2.0;

        m_last_x = m_x1;
        m_last_y = m_y1;
    }

    // actually do the drawing here
    void DrawSpline(double x, double y);

    // After the last point call this to finish the drawing
    void EndSpline()
    {
        wxCHECK_RET( m_dc, wxT("invalid window dc") );
        if (ClipLineToRect(m_cx1, m_cy1, m_x2, m_y2, m_rect) != ClippedOut)
            m_dc->DrawLine((int)m_cx1, (int)m_cy1, (int)m_x2, (int)m_y2);
    }

private:

    typedef struct SplineStack
    {
        double x1, y1, x2, y2, x3, y3, x4, y4;
    } SplineStack;

    wxDC *m_dc;
    wxRect2DDouble m_rect;

    SplineStack m_splineStack[SPLINE_STACK_DEPTH];
    int m_stack_count;

    double m_cx1, m_cy1, m_cx2, m_cy2, m_cx3, m_cy3, m_cx4, m_cy4;
    double m_x1, m_y1, m_x2, m_y2;
    double m_last_x, m_last_y;

    wxPen m_selPen, m_curPen;
    wxRangeDoubleSelection *m_rangeSel;
};

void SplineDrawer::DrawSpline( double x, double y )
{
    wxCHECK_RET( m_dc, wxT("invalid window dc") );
    wxPen oldPen = m_dc->GetPen();

    bool is_selected = (oldPen == m_selPen);

    m_x1 = m_x2;
    m_y1 = m_y2;
    m_x2 = x;
    m_y2 = y;
    m_cx4 = (m_x1 + m_x2) / 2.0;
    m_cy4 = (m_y1 + m_y2) / 2.0;
    m_cx3 = (m_x1 + m_cx4) / 2.0;
    m_cy3 = (m_y1 + m_cy4) / 2.0;

    double xmid, ymid;
    double xx1, yy1, xx2, yy2, xx3, yy3, xx4, yy4;

    SplineStack *stack_top = m_splineStack;
    m_stack_count = 0;

    SPLINE_PUSH(m_cx1, m_cy1, m_cx2, m_cy2, m_cx3, m_cy3, m_cx4, m_cy4);

    while (m_stack_count > 0)
    {
        SPLINE_POP(xx1, yy1, xx2, yy2, xx3, yy3, xx4, yy4);

        xmid = (xx2 + xx3)/2.0;
        ymid = (yy2 + yy3)/2.0;
        if ((fabs(xx1 - xmid) < THRESHOLD) && (fabs(yy1 - ymid) < THRESHOLD) &&
            (fabs(xmid - xx4) < THRESHOLD) && (fabs(ymid - yy4) < THRESHOLD))
        {
            // FIXME - is this really necessary, better safe than sorry?
            double t1_last_x = m_last_x;
            double t1_last_y = m_last_y;
            double t1_xx1    = xx1;
            double t1_yy1    = yy1;
            if (ClipLineToRect(t1_last_x, t1_last_y, t1_xx1, t1_yy1, m_rect) != ClippedOut)
            {
                if (m_rangeSel && (m_rangeSel->Contains((m_last_x + xx1)/2) != is_selected))
                {
                    is_selected = is_selected ? false : true;
                    if (is_selected)
                        m_dc->SetPen(m_selPen);
                    else
                        m_dc->SetPen(m_curPen);
                }

                m_dc->DrawLine((int)t1_last_x, (int)t1_last_y, (int)t1_xx1, (int)t1_yy1);
            }

            double t2_xx1  = xx1;
            double t2_yy1  = yy1;
            double t2_xmid = xmid;
            double t2_ymid = ymid;
            if (ClipLineToRect(t2_xx1, t2_yy1, t2_xmid, t2_ymid, m_rect) != ClippedOut)
            {
                if (m_rangeSel && (m_rangeSel->Contains((xx1+xmid)/2) != is_selected))
                {
                    is_selected = is_selected ? false : true;
                    if (is_selected)
                        m_dc->SetPen(m_selPen);
                    else
                        m_dc->SetPen(m_curPen);
                }

                m_dc->DrawLine((int)t2_xx1, (int)t2_yy1, (int)t2_xmid, (int)t2_ymid);
            }

            m_last_x = xmid;
            m_last_y = ymid;
        }
        else
        {
            wxCHECK_RET(m_stack_count < SPLINE_STACK_DEPTH - 2, wxT("Spline stack overflow"));
            SPLINE_PUSH(xmid, ymid, (xmid + xx3)/2.0, (ymid + yy3)/2.0,
                        (xx3 + xx4)/2.0, (yy3 + yy4)/2.0, xx4, yy4);
            SPLINE_PUSH(xx1, yy1, (xx1 + xx2)/2.0, (yy1 + yy2)/2.0,
                        (xx2 + xmid)/2.0, (yy2 + ymid)/2.0, xmid, ymid);
        }
    }

    m_cx1 = m_cx4;
    m_cy1 = m_cy4;
    m_cx2 = (m_cx1 + m_x2) / 2.0;
    m_cy2 = (m_cy1 + m_y2) / 2.0;

    m_dc->SetPen(oldPen);
}

//***************************************************************************


//-----------------------------------------------------------------------------
// wxPlotDrawerAxisBase
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxPlotDrawerBase, wxObject)

wxPlotDrawerAxisBase::wxPlotDrawerAxisBase(wxPlotCtrl* owner)
                     :wxPlotDrawerBase(owner)
{
    m_tickFont    = *wxNORMAL_FONT;
    m_labelFont   = *wxSWISS_FONT;
    m_tickColour  = wxGenericColour(0,0,0);
    m_labelColour = wxGenericColour(0,0,0);

    m_tickPen         = wxGenericPen(m_tickColour, wxSOLID);
    m_backgroundBrush = wxGenericBrush(wxGenericColour(255,255,255), wxSOLID);
}

//-----------------------------------------------------------------------------
// wxPlotDrawerArea
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxPlotDrawerArea, wxPlotDrawerBase)

void wxPlotDrawerArea::Draw(wxDC *dc, bool refresh)
{
}

//-----------------------------------------------------------------------------
// wxPlotDrawerAxisBase
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxPlotDrawerAxisBase, wxPlotDrawerBase)

//-----------------------------------------------------------------------------
// wxPlotDrawerXAxis
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxPlotDrawerXAxis, wxPlotDrawerAxisBase)

void wxPlotDrawerXAxis::Draw(wxDC *dc, bool refresh)
{
    wxCHECK_RET(dc, wxT("Invalid dc"));

    wxRect dcRect(GetDCRect());

    // Draw background
    if (refresh)
    {
        dc->SetBrush(m_backgroundBrush.GetBrush());
        dc->SetPen(*wxTRANSPARENT_PEN);
        dc->DrawRectangle(dcRect);
    }

    wxFont tickFont = m_tickFont;
    if (m_font_scale != 1)
        tickFont.SetPointSize( wxMax(2, RINT(tickFont.GetPointSize() * m_font_scale)) );

    wxPoint dcOrigin(dc->GetDeviceOrigin());
    dc->SetDeviceOrigin(dcRect.x, dcRect.y);
    dc->SetTextForeground( m_tickColour.GetColour() );
    dc->SetFont( tickFont );

    wxString label;

    // center the text in the window
    int x, y;
    dc->GetTextExtent(wxT("5"), &x, &y);
    int y_pos = (GetDCRect().height - y)/2 + 2; // FIXME I want to center this
//    double current = ceil(m_viewRect.GetLeft() / m_xAxisTick_step) * m_xAxisTick_step;
    int i, count = m_tickPositions.GetCount();
    for (i=0; i<count; i++)
    {
        dc->DrawText(m_tickLabels[i], m_tickPositions[i], y_pos);

//        if (!IsFinite(current, wxT("axis label is not finite")))
//            break;
//        label.Printf( m_xAxisTickFormat.c_str(), current );
//        dc->DrawText(label, m_xAxisTicks[i], y_pos);
//        current += m_xAxisTick_step;
    }

#ifdef DRAW_BORDERS
    // Test code for sizing to show the extent of the axes
    dc->SetBrush( *wxTRANSPARENT_BRUSH );
    dc->SetPen( wxPen(GetBorderColour(), 1, wxSOLID) );
    dc->DrawRectangle(wxRect(wxPoint(0,0), clientSize));
#endif // DRAW_BORDERS

    dc->SetDeviceOrigin(dcOrigin.x, dcOrigin.y);
}

//-----------------------------------------------------------------------------
// wxPlotDrawerYAxis
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxPlotDrawerYAxis, wxPlotDrawerAxisBase)

void wxPlotDrawerYAxis::Draw(wxDC *dc, bool refresh)
{
    wxCHECK_RET(dc, wxT("Invalid dc"));

    wxRect dcRect(GetDCRect());

    // Draw background
    if (refresh)
    {
        dc->SetBrush(m_backgroundBrush.GetBrush());
        dc->SetPen(*wxTRANSPARENT_PEN);
        dc->DrawRectangle(dcRect);
    }

    wxFont tickFont = m_tickFont;
    if (m_font_scale != 1)
        tickFont.SetPointSize( wxMax(2, RINT(tickFont.GetPointSize() * m_font_scale)) );

    wxPoint dcOrigin(dc->GetDeviceOrigin());
    dc->SetDeviceOrigin(dcRect.x, dcRect.y);
    dc->SetTextForeground( m_tickColour.GetColour() );
    dc->SetFont( tickFont );

    wxString label;

//    double current = ceil(m_viewRect.GetTop() / m_yAxisTick_step) * m_yAxisTick_step;
    int i, count = m_tickLabels.GetCount();
    for (i=0; i<count; i++)
    {
        dc->DrawText( m_tickLabels[i], 2, m_tickPositions[i] );

//        if (!IsFinite(current, wxT("axis label is not finite")))
//            break;
//        label.Printf( m_yAxisTickFormat.c_str(), current);
//        dc->DrawText( label, 2, m_yAxisTicks[i] );
//        current += m_yAxisTick_step;
    }

#ifdef DRAW_BORDERS
    // Test code for sizing to show the extent of the axes
    dc->SetBrush( *wxTRANSPARENT_BRUSH );
    dc->SetPen( wxPen(GetBorderColour(), 1, wxSOLID) );
    dc->DrawRectangle(wxRect(wxPoint(0,0), clientSize));
#endif // DRAW_BORDERS

    dc->SetDeviceOrigin(dcOrigin.x, dcOrigin.y);
}

//-----------------------------------------------------------------------------
// wxPlotDrawerKey
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxPlotDrawerKey, wxPlotDrawerBase)

wxPlotDrawerKey::wxPlotDrawerKey(wxPlotCtrl* owner)
                :wxPlotDrawerBase(owner)
{
    m_font = *wxNORMAL_FONT;
    m_fontColour = wxGenericColour(0, 0, 0);
    m_keyPosition = wxPoint(100, 100);
    m_border = 5;
    m_key_inside = true;
    m_key_line_width = 20;
    m_key_line_margin = 5;
}

void wxPlotDrawerKey::Draw(wxDC *dc, const wxString& keyString_)
{
    wxCHECK_RET(dc && m_owner, wxT("Invalid dc"));

    if (keyString_.IsEmpty())
        return;

    wxString keyString = keyString_;

/*
    // GTK - kills X if font size is too small
    double x_scale = 1, y_scale = 1;
    dc->GetUserScale( &x_scale, &y_scale );

    wxFont font = m_owner->GetKeyFont();
    if (0 && x_scale != 1)
    {
        font.SetPointSize(wxMax(int(font.GetPointSize()/x_scale), 4));
        if (!font.Ok())
            font = GetKeyFont();
    }
*/

    wxFont keyFont = m_font;
    if (m_font_scale != 1)
        keyFont.SetPointSize( wxMax(2, RINT(keyFont.GetPointSize() * m_font_scale)) );

    int key_line_width  = RINT(m_key_line_width  * m_pen_scale);
    int key_line_margin = RINT(m_key_line_margin * m_pen_scale);

    dc->SetFont(keyFont);
    dc->SetTextForeground(m_fontColour.GetColour());

    wxRect keyRect;
    int heightLine = 0;

    dc->GetMultiLineTextExtent(keyString, &keyRect.width, &keyRect.height, &heightLine);

    wxRect dcRect(GetDCRect());
    wxSize areaSize = dcRect.GetSize();

    keyRect.x = 30 + int((m_keyPosition.x*.01)*areaSize.x);
    keyRect.y = areaSize.y - int((m_keyPosition.y*.01)*areaSize.y);

    if (m_key_inside)
    {
        keyRect.x = wxMax(30, keyRect.x);
        keyRect.x = wxMin(areaSize.x - keyRect.width - m_border, keyRect.GetRight());

        keyRect.y = wxMax(m_border, keyRect.y);
        keyRect.y = wxMin(areaSize.y - keyRect.height - m_border, keyRect.y);
    }

    int h = keyRect.y;
    int i = 0;

    while (!keyString.IsEmpty())
    {
        wxString subkey = keyString.BeforeFirst(wxT('\n')).Strip(wxString::both);
        keyString = keyString.AfterFirst(wxT('\n'));
        if (subkey.IsEmpty()) break;

        if (m_owner && m_owner->GetCurve(i))
        {
            wxPen keyPen = m_owner->GetCurve(i)->GetPen(wxPLOTPEN_NORMAL).GetPen();
            if (m_pen_scale != 1)
                keyPen.SetWidth(int(keyPen.GetWidth() * m_pen_scale));

            if(keyPen.GetWidth() < 3) keyPen.SetWidth(3);
            dc->SetPen(keyPen);
            dc->DrawLine(keyRect.x - (key_line_width + key_line_margin), h + heightLine/2,
                        keyRect.x - key_line_margin, h + heightLine/2);
        }

        dc->DrawText(subkey, keyRect.x, h);

        h += heightLine;
        i++;
    }

    dc->SetPen(wxNullPen);
    dc->SetFont(wxNullFont);
}

//-----------------------------------------------------------------------------
// wxPlotDrawerCurve
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxPlotDrawerCurve, wxPlotDrawerBase)

void wxPlotDrawerCurve::Draw(wxDC *dc, wxPlotCurve *curve, int curve_index)
{
    wxCHECK_RET(dc && m_owner && curve && curve->Ok(), wxT("invalid curve"));
    INITIALIZE_FAST_GRAPHICS

    wxRect dcRect(GetDCRect());

    int i, j0, j1;
    double x0, y0, x1, y1, yy0, yy1;
    x0 = m_owner->GetPlotCoordFromClientX(0);
    y0 = yy0 = curve->GetY(x0);

    //wxRect2DDouble viewRect = m_viewRect;
    wxRect2DDouble subViewRect = m_owner->GetPlotRectFromClientRect( dcRect );
    //printf("curve rect %d %d %d %d\n", rect.x, rect.y, rect.width, rect.height); fflush(stdout);
    //printf("curve subviewrect %lf %lf %lf %lf\n", subViewRect.m_x, subViewRect.m_y, subViewRect.m_width, subViewRect.m_height); fflush(stdout);

    int right = dcRect.GetRight();

    wxPen currentPen = (curve_index == m_owner->GetActiveIndex()) ? curve->GetPen(wxPLOTPEN_ACTIVE).GetPen()
                                                                  : curve->GetPen(wxPLOTPEN_NORMAL).GetPen();
    wxPen selectedPen = curve->GetPen(wxPLOTPEN_SELECTED).GetPen();

    if (m_pen_scale != 1)
    {
        currentPen.SetWidth(int(currentPen.GetWidth() * m_pen_scale));
        selectedPen.SetWidth(int(selectedPen.GetWidth() * m_pen_scale));
    }

    dc->SetPen(currentPen);

    const wxRangeDoubleSelection *ranges = m_owner->GetCurveSelection(curve_index);
    bool selected = false;

    int clipped = ClippedNeither;

    for (i=dcRect.x; i<right; i++)
    {
        x1 = m_owner->GetPlotCoordFromClientX(i);
        y1 = yy1 = curve->GetY(x1);

        clipped = ClipLineToRect(x0, yy0, x1, yy1, subViewRect);

        if (selected != ranges->Contains(x1))
        {
            if (selected)
                dc->SetPen(currentPen);
            else
                dc->SetPen(selectedPen);

            selected = !selected;
        }

        if (clipped != ClippedOut)
        {
            j0 = m_owner->GetClientCoordFromPlotY(yy0);
            j1 = m_owner->GetClientCoordFromPlotY(yy1);
            wxPLOT_DRAW_LINE(dc, window, pen, i-1, j0, i, j1);

            if (selected && !((clipped & ClippedSecond) != 0))
            {
                wxPLOT_DRAW_ELLIPSE(dc, window, pen, i, j1, 2, 2);
            }
        }

        x0 = x1;
        y0 = yy0 = y1;
    }

    dc->SetPen(wxNullPen);
}

//-----------------------------------------------------------------------------
// wxPlotDrawerDataCurve
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxPlotDrawerDataCurve, wxPlotDrawerBase)

void wxPlotDrawerDataCurve::Draw(wxDC *dc, wxPlotData* curve, int curve_index)
{
    wxCHECK_RET(dc && m_owner && curve && curve->Ok(), wxT("invalid curve"));
    INITIALIZE_FAST_GRAPHICS

    wxRect dcRect(GetDCRect());

    wxRect2DDouble viewRect( GetPlotViewRect() ); //m_viewRect );
    wxRect2DDouble subViewRect( m_owner->GetPlotRectFromClientRect(dcRect) );
    wxRect2DDouble curveRect( curve->GetBoundingRect() );
    if (!wxPlotRect2DDoubleIntersects(curveRect, subViewRect)) return;

/*  // FIXME - drawing symbol bitmaps in MSW is very slow
    wxBitmap bitmap;
    if (curve == GetActiveCurve())
        bitmap = curve->GetSymbol(wxPLOTPEN_ACTIVE);
    else
        bitmap = curve->GetSymbol(wxPLOTPEN_NORMAL);

    if (!bitmap.Ok())
    {
        if (curve == GetActiveCurve())
            bitmap = wxPlotSymbolCurrent;
        else
            bitmap = wxPlotSymbolNormal;
    }

    int bitmapHalfWidth = bitmap.GetWidth()/2;
    int bitmapHalfHeight = bitmap.GetHeight()/2;
*/

    // find the starting and ending indexes into the data curve
    int n, n_start, n_end;
    bool x_ordered = curve->GetIsXOrdered();

    if (x_ordered)
    {
        n_start = curve->GetIndexFromX(subViewRect.GetLeft(), wxPlotData::index_floor);
        n_end   = curve->GetIndexFromX(subViewRect.GetRight(), wxPlotData::index_ceil);
        n_end++; // for statement comparison is <
    }
    else
    {
        n_start = 0;
        n_end   = curve->GetCount();
    }

    // set the pens to draw with
    wxPen currentPen = (curve_index == m_owner->GetActiveIndex()) ? curve->GetPen(wxPLOTPEN_ACTIVE).GetPen()
                                                                  : curve->GetPen(wxPLOTPEN_NORMAL).GetPen();
    wxPen selectedPen = curve->GetPen(wxPLOTPEN_SELECTED).GetPen();
    if (m_pen_scale != 1)
    {
        currentPen.SetWidth(int(currentPen.GetWidth() * m_pen_scale));
        selectedPen.SetWidth(int(selectedPen.GetWidth() * m_pen_scale));
    }

    dc->SetPen(currentPen);

    // handle the selected ranges and initialize the starting range
    const wxArrayRangeInt &ranges = m_owner->GetDataCurveSelection(curve_index)->GetRangeArray();
    int n_range = 0, range_count = ranges.GetCount();
    int min_sel = -1, max_sel = -1;
    for (n_range=0; n_range<range_count; n_range++)
    {
        const wxRangeInt& range = ranges[n_range];
        if ((range.m_max >= n_start) || (range.m_min >= n_start))
        {
            min_sel = range.m_min;
            max_sel = range.m_max;
            if (range.Contains(n_start))
                dc->SetPen( selectedPen );

            break;
        }
    }

    // data variables
    const double *x_data = &curve->GetXData()[n_start];
    const double *y_data = &curve->GetYData()[n_start];

    int i0, j0, i1, j1;        // curve coords in pixels
    double x0, y0, x1, y1;     // original curve coords
    double xx0, yy0, xx1, yy1; // clipped curve coords

    x0 = *x_data;
    y0 = *y_data;

    int clipped = ClippedNeither;

    bool draw_lines   = m_owner->GetDrawLines();
    bool draw_symbols = m_owner->GetDrawSymbols();
    bool draw_spline  = m_owner->GetDrawSpline();

    SplineDrawer sd;
    wxRangeDoubleSelection dblRangeSel;

    if (draw_spline)
    {
        wxRangeDouble viewRange(viewRect.m_x, viewRect.GetRight());
        wxRangeDouble dcRange(dcRect.x, dcRect.GetRight());

        for (int r = n_range; r < range_count; r++)
        {
            wxRangeDouble plotRange(curve->GetXValue(ranges[r].m_min),
                                    curve->GetXValue(ranges[r].m_max));

            if (viewRange.Intersects(plotRange))
            {
                double min_x = m_owner->GetClientCoordFromPlotX(plotRange.m_min);
                double max_x = m_owner->GetClientCoordFromPlotX(plotRange.m_max);
                dblRangeSel.SelectRange(wxRangeDouble(min_x, max_x));
            }
            else
                break;
        }

        // spline starts 2 points back for smoothness
        int s_start = n_start > 1 ? -2 : n_start > 0 ? -1 : 0;
        sd.Create(dc, currentPen, selectedPen,
                  wxRect2DDouble(dcRect.x, dcRect.y, dcRect.width, dcRect.height),
                  &dblRangeSel,
                  m_owner->GetClientCoordFromPlotX(x_data[s_start]),
                  m_owner->GetClientCoordFromPlotY(y_data[s_start]),
                  m_owner->GetClientCoordFromPlotX(x_data[s_start+1]),
                  m_owner->GetClientCoordFromPlotY(y_data[s_start+1]));
    }

    for (n = n_start; n < n_end; n++)
    {
        x1 = *x_data++;
        y1 = *y_data++;

        if (draw_spline)
            sd.DrawSpline(m_owner->GetClientCoordFromPlotX(x1),
                          m_owner->GetClientCoordFromPlotY(y1));

        xx0 = x0; yy0 = y0; xx1 = x1; yy1 = y1;
        clipped = ClipLineToRect(xx0, yy0, xx1, yy1, viewRect);
        if (clipped != ClippedOut)
        {
            i0 = m_owner->GetClientCoordFromPlotX(xx0);
            j0 = m_owner->GetClientCoordFromPlotY(yy0);
            i1 = m_owner->GetClientCoordFromPlotX(xx1);
            j1 = m_owner->GetClientCoordFromPlotY(yy1);

            if (draw_lines && ((i0 != i1) || (j0 != j1)))
            {
                wxPLOT_DRAW_LINE(dc, window, pen, i0, j0, i1, j1);
            }

            if (n == min_sel)
                dc->SetPen( selectedPen );

            if (draw_symbols && !((clipped & ClippedSecond) != 0) &&
                ((i0 != i1) || (j0 != j1) || (n == min_sel) || (n == n_start)))
            {
                //dc->DrawBitmap( bitmap, i1 - bitmapHalfWidth, j1 - bitmapHalfHeight, true );
                wxPLOT_DRAW_ELLIPSE(dc, window, pen, i1, j1, 2, 2);
            }
        }
        else if (n == min_sel)
        {
            dc->SetPen( selectedPen );
        }

        if (n == max_sel)
        {
            dc->SetPen( currentPen );
            if (n_range < range_count - 1)
            {
                n_range++;
                min_sel = ranges[n_range].m_min;
                max_sel = ranges[n_range].m_max;
            }
        }

        x0 = x1;
        y0 = y1;
    }

    if (draw_spline)
    {
        // want an extra point at the end to smooth it out
        if (n_end < (int)curve->GetCount() - 1)
            sd.DrawSpline(m_owner->GetClientCoordFromPlotX(*x_data),
                          m_owner->GetClientCoordFromPlotY(*y_data));

        sd.EndSpline();
    }

    dc->SetPen(wxNullPen);
}

//-----------------------------------------------------------------------------
// wxPlotDrawerMarkers
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxPlotDrawerMarker, wxPlotDrawerBase)

void wxPlotDrawerMarker::Draw(wxDC *dc, const wxPlotMarker& marker)
{
    // drawing multiple markers is faster, so just drawing a single one takes a hit
    wxArrayPlotMarker markers;
    markers.Add(marker);
    Draw(dc, markers);
}

void wxPlotDrawerMarker::Draw(wxDC *dc, const wxArrayPlotMarker& markers)
{
    wxCHECK_RET(dc && m_owner, wxT("dc or owner"));
    INITIALIZE_FAST_GRAPHICS

    wxRect dcRect(GetDCRect());
    wxRect2DDouble subViewRect = m_owner->GetPlotRectFromClientRect( dcRect );

    double x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    int n, count = markers.GetCount();
    for (n = 0; n < count; n++)
    {
        const wxPlotMarker &marker = markers[n];
        wxCHECK_RET(marker.Ok(), wxT("Invalid marker"));
        wxRect2DDouble r = marker.GetPlotRect();
        x0 = r.m_x;
        y0 = r.m_y;
        x1 = r.GetRight();
        y1 = r.GetBottom();

        if (marker.GetPen().Ok())
            dc->SetPen(marker.GetPen().GetPen());
        if (marker.GetBrush().Ok())
            dc->SetBrush(marker.GetBrush().GetBrush());

        // determine what to draw
        int marker_type = marker.GetMarkerType();
        wxSize size     = marker.GetSize();

        if (marker_type == wxPLOTMARKER_BITMAP)
        {
            wxBitmap bmp(marker.GetBitmap());
            int w = bmp.GetWidth(), h = bmp.GetHeight();
            // FIXME - add scaling and shifting later - maybe
            int i0 = m_owner->GetClientCoordFromPlotX(x0);
            int j0 = m_owner->GetClientCoordFromPlotY(y0);
            dc->DrawBitmap(bmp, RINT(i0 - w/2.0), RINT(j0 - h/2.0), true);
        }
        else if (marker_type == wxPLOTMARKER_LINE)
        {
            if (ClipLineToRect(x0, y0, x1, y1, subViewRect) != ClippedOut)
            {
                int i0 = m_owner->GetClientCoordFromPlotX(x0);
                int j0 = m_owner->GetClientCoordFromPlotY(y0);
                int i1 = m_owner->GetClientCoordFromPlotX(x1);
                int j1 = m_owner->GetClientCoordFromPlotY(y1);
                wxPLOT_DRAW_LINE(dc, window, pen, i0, j0, i1, j1);
            }
        }
        else if (marker_type == wxPLOTMARKER_ELLIPSE)
        {
            // fixed pixel size
            if ((size.x > 0) && (size.y > 0))
            {
                if (ClipLineToRect(x0, y0, x1, y1, subViewRect) != ClippedOut)
                {
                    int i0 = m_owner->GetClientCoordFromPlotX(x0);
                    int j0 = m_owner->GetClientCoordFromPlotY(y0);
                    wxPLOT_DRAW_ELLIPSE(dc, window, pen, i0, j0, size.x, size.y);
                }
            }
/*
            else if (ClipLineToRect(x0, y0, x1, y1, subViewRect) != ClippedOut)
            {
                int i0 = m_owner->GetClientCoordFromPlotX(x0);
                int j0 = m_owner->GetClientCoordFromPlotY(y0);
                int i1 = m_owner->GetClientCoordFromPlotX(x1);
                int j1 = m_owner->GetClientCoordFromPlotY(y1);
                wxPLOT_DRAW_ELLIPSE(dc, window, pen, i0, j0, i1, j1);
            }
*/
        }
        else // figure out the rest
        {
            // we may ignore type if rect is in certain states

            bool is_horiz = r.m_width  < 0;
            bool is_vert  = r.m_height < 0;

            bool cross = is_horiz && is_vert;

            if (is_horiz)
            {
                x0 = subViewRect.m_x - subViewRect.m_width; // push outside win
                x1 = subViewRect.GetRight() + subViewRect.m_width;
            }
            if (is_vert)
            {
                y0 = subViewRect.m_y - subViewRect.m_height;
                y1 = subViewRect.GetBottom() + subViewRect.m_height;
            }

            if ((marker_type == wxPLOTMARKER_POINT) || ((x0 == x1) && (y0 == y1)))
            {
                if (ClipLineToRect(x0, y0, x1, y1, subViewRect) != ClippedOut)
                {
                    int i0 = m_owner->GetClientCoordFromPlotX(x0);
                    int j0 = m_owner->GetClientCoordFromPlotY(y0);
                    dc->DrawPoint(i0, j0);
                }
            }
            else if ((marker_type == wxPLOTMARKER_VERT_LINE) || ((x0 == x1) && (y0 != y1)))
            {
                if (ClipLineToRect(x0, y0, x1, y1, subViewRect) != ClippedOut)
                {
                    int i0 = m_owner->GetClientCoordFromPlotX(x0);
                    int j0 = m_owner->GetClientCoordFromPlotY(y0);
                    int j1 = m_owner->GetClientCoordFromPlotY(y1);
                    wxPLOT_DRAW_LINE(dc, window, pen, i0, j0, i0, j1);
                }
            }
            else if ((marker_type == wxPLOTMARKER_HORIZ_LINE) || ((y0 == y1) && (x0 != x1)))
            {
                if (ClipLineToRect(x0, y0, x1, y1, subViewRect) != ClippedOut)
                {
                    int i0 = m_owner->GetClientCoordFromPlotX(x0);
                    int i1 = m_owner->GetClientCoordFromPlotX(x1);
                    int j0 = m_owner->GetClientCoordFromPlotY(y0);
                    wxPLOT_DRAW_LINE(dc, window, pen, i0, j0, i1, j0);
                }
            }
            else if ((marker_type == wxPLOTMARKER_CROSS) || cross)
            {


            }
            else                                // rectangle
            {
                wxRect2DDouble clippedRect(x0, y0, x1 - x0, y1 - y0);
                clippedRect.Intersect(subViewRect);
                int pen_width = dc->GetPen().GetWidth() + 2;

                int i0 = m_owner->GetClientCoordFromPlotX(clippedRect.m_x);
                int i1 = m_owner->GetClientCoordFromPlotX(clippedRect.GetRight());
                int j0 = m_owner->GetClientCoordFromPlotY(clippedRect.m_y);
                int j1 = m_owner->GetClientCoordFromPlotY(clippedRect.GetBottom());
                if (r.m_x < subViewRect.m_x)  i0 -= pen_width;
                if (r.m_y < subViewRect.m_y)  j0 -= pen_width;
                if (r.GetRight()  > subViewRect.GetRight())  i1 += pen_width;
                if (r.GetBottom() > subViewRect.GetBottom()) j1 += pen_width;

                dc->SetClippingRegion(dcRect);
                dc->DrawRectangle(i0, j0, i1 - i0 + 1, j1 - j0 + 1);
                dc->DestroyClippingRegion();
            }
        }
    }
}
