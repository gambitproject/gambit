/////////////////////////////////////////////////////////////////////////////
// Name:        plotmark.cpp
// Purpose:     wxPlotMarker
// Author:      John Labenski
// Modified by:
// Created:     8/27/2002
// Copyright:   (c) John Labenski
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "plotmark.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif // WX_PRECOMP

#include "wx/image.h"

#include "wx/plotctrl/plotmark.h"
#include "wx/plotctrl/plotctrl.h"

#include "wx/arrimpl.cpp"
WX_DEFINE_OBJARRAY( wxArrayPlotMarker );

//-----------------------------------------------------------------------------
// wxPlotMarkerRefData
//-----------------------------------------------------------------------------

class wxPlotMarkerRefData : public wxObjectRefData
{
public:
    wxPlotMarkerRefData(int type = 0, const wxRect2DDouble& rect = wxRect2DDouble())
        : wxObjectRefData(), m_markerType(type), m_rect(rect), m_size(wxSize(-1, -1)) {}

    wxPlotMarkerRefData( const wxPlotMarkerRefData& data )
        : wxObjectRefData(), m_markerType(data.m_markerType),
          m_rect(data.m_rect), m_size(data.m_size), m_bitmap(data.m_bitmap),
          m_pen(data.m_pen), m_brush(data.m_brush) {}

    int            m_markerType;
    wxRect2DDouble m_rect;
    wxSize         m_size;
    wxBitmap       m_bitmap;
    wxGenericPen   m_pen;
    wxGenericBrush m_brush;
};

#define M_PMARKERDATA ((wxPlotMarkerRefData*)m_refData)

//-----------------------------------------------------------------------------
// wxPlotMarker
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxPlotMarker, wxObject);

wxObjectRefData *wxPlotMarker::CreateRefData() const
{
    return new wxPlotMarkerRefData;
}
wxObjectRefData *wxPlotMarker::CloneRefData(const wxObjectRefData *data) const
{
    return new wxPlotMarkerRefData(*(const wxPlotMarkerRefData *)data);
}

void wxPlotMarker::Create(int marker_type, const wxRect2DDouble& rect,
                          const wxSize& size, const wxGenericPen& pen,
                          const wxGenericBrush& brush, const wxBitmap& bitmap)
{
    UnRef();
    m_refData = new wxPlotMarkerRefData(marker_type, rect);
    M_PMARKERDATA->m_size   = size;
    M_PMARKERDATA->m_pen    = pen;
    M_PMARKERDATA->m_brush  = brush;
    M_PMARKERDATA->m_bitmap = bitmap;
}

int wxPlotMarker::GetMarkerType() const
{
    wxCHECK_MSG(Ok(), wxPLOTMARKER_NONE, wxT("Invalid plot marker"));
    return M_PMARKERDATA->m_markerType;
}
void wxPlotMarker::SetMarkerType(int type)
{
    wxCHECK_RET(Ok(), wxT("Invalid plot marker"));
    M_PMARKERDATA->m_markerType = type;
}

wxRect2DDouble wxPlotMarker::GetPlotRect() const
{
    wxCHECK_MSG(Ok(), wxRect2DDouble(), wxT("Invalid plot marker"));
    return M_PMARKERDATA->m_rect;
}
wxRect2DDouble& wxPlotMarker::GetPlotRect()
{
    static wxRect2DDouble s_rect;
    wxCHECK_MSG(Ok(), s_rect, wxT("Invalid plot marker"));
    return M_PMARKERDATA->m_rect;
}
void wxPlotMarker::SetPlotRect(const wxRect2DDouble& rect)
{
    wxCHECK_RET(Ok(), wxT("Invalid plot marker"));
    M_PMARKERDATA->m_rect = rect;
}

wxPoint2DDouble wxPlotMarker::GetPlotPosition() const
{
    wxCHECK_MSG(Ok(), wxPoint2DDouble(), wxT("Invalid plot marker"));
    return M_PMARKERDATA->m_rect.GetLeftTop();
}
void wxPlotMarker::SetPlotPosition(const wxPoint2DDouble& pos)
{
    wxCHECK_RET(Ok(), wxT("Invalid plot marker"));
    M_PMARKERDATA->m_rect.m_x = pos.m_x;
    M_PMARKERDATA->m_rect.m_y = pos.m_y;
}

wxSize wxPlotMarker::GetSize() const
{
    wxCHECK_MSG(Ok(), wxSize(-1, -1), wxT("Invalid plot marker"));
    return M_PMARKERDATA->m_size;
}
void wxPlotMarker::SetSize(const wxSize& size)
{
    wxCHECK_RET(Ok(), wxT("Invalid plot marker"));
    M_PMARKERDATA->m_size = size;
}

wxGenericPen wxPlotMarker::GetPen() const
{
    wxCHECK_MSG(Ok(), wxNullGenericPen, wxT("Invalid plot marker"));
    return M_PMARKERDATA->m_pen;
}
void wxPlotMarker::SetPen(const wxGenericPen& pen)
{
    wxCHECK_RET(Ok(), wxT("Invalid plot marker"));
    M_PMARKERDATA->m_pen = pen;
}
wxGenericBrush wxPlotMarker::GetBrush() const
{
    wxCHECK_MSG(Ok(), wxNullGenericBrush, wxT("Invalid plot marker"));
    return M_PMARKERDATA->m_brush;
}
void wxPlotMarker::SetBrush(const wxGenericBrush& brush)
{
    wxCHECK_RET(Ok(), wxT("Invalid plot marker"));
    M_PMARKERDATA->m_brush = brush;
}
wxBitmap wxPlotMarker::GetBitmap() const
{
    wxCHECK_MSG(Ok(), wxNullBitmap, wxT("Invalid plot marker"));
    return M_PMARKERDATA->m_bitmap;
}
void wxPlotMarker::SetBitmap(const wxBitmap& bitmap)
{
    wxCHECK_RET(Ok(), wxT("Invalid plot marker"));
    M_PMARKERDATA->m_bitmap = bitmap;
}
