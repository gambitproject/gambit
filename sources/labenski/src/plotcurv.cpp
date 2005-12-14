/////////////////////////////////////////////////////////////////////////////
// Name:        plotcurv.cpp
// Purpose:     wxPlotWindow
// Author:      John Labenski
// Modified by:
// Created:     12/01/2000
// Copyright:   (c) John Labenski
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "plotcurv.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/bitmap.h"
    #include "wx/textdlg.h"
    #include "wx/msgdlg.h"
    #include "wx/dcmemory.h"
#endif // WX_PRECOMP

#include "wx/file.h"
#include "wx/wfstream.h"
#include "wx/textfile.h"
#include "wx/math.h"

#include "wx/plotctrl/plotcurv.h"
#include <math.h>

const wxRect2DDouble wxNullPlotBounds(0, 0, 0, 0);

#ifdef USE_BITMAPS_FOR_DRAWING

/* XPM */
static const char *normal_symbol_xpm_data[] = {
/* columns rows colors chars-per-pixel */
"5 5 2 1",
"  c None",
"b c #000000",
/* pixels */
" bbb ",
"b   b",
"b   b",
"b   b",
" bbb "
};

static const char *active_symbol_xpm_data[] = {
/* columns rows colors chars-per-pixel */
"5 5 2 1",
"  c None",
"b c #0000FF",
/* pixels */
" bbb ",
"b   b",
"b   b",
"b   b",
" bbb "
};

static const char *selected_symbol_xpm_data[] = {
/* columns rows colors chars-per-pixel */
"5 5 2 1",
"  c None",
"b c #FF0000",
/* pixels */
" bbb ",
"b   b",
"b   b",
"b   b",
" bbb "
};

#endif // USE_BITMAPS_FOR DRAWING

// Can't load these now since wxWindows must initialize first
wxBitmap wxPlotSymbolNormal;
wxBitmap wxPlotSymbolActive;
wxBitmap wxPlotSymbolSelected;

static wxGenericPen s_defaultPlotPens[wxPLOTPEN_MAXTYPE] =
{
    wxGenericPen(   0, 0,   0, 1, wxSOLID ),
    wxGenericPen(   0, 0, 255, 1, wxSOLID ),
    wxGenericPen( 255, 0,   0, 1, wxSOLID )   
};

//----------------------------------------------------------------------------
// Interpolate
//----------------------------------------------------------------------------
double LinearInterpolateX(double x0, double y0, double x1, double y1, double y)
{
    //wxCHECK_MSG( (y1 - y0) != 0.0, 0.0, wxT("Divide by zero, LinearInterpolateX()") );
    return ( (y - y0)*(x1 - x0)/(y1 - y0) + x0 );
}

double LinearInterpolateY(double x0, double y0, double x1, double y1, double x)
{
    //wxCHECK_MSG( (x1 - x0) != 0.0, 0.0, wxT("Divide by zero, LinearInterpolateY()") );
    double m = (y1 - y0) / (x1 - x0);
    return (m*x + (y0 - m*x0));
}

//----------------------------------------------------------------------------
// wxPlotCurveRefData
//----------------------------------------------------------------------------

wxPlotCurveRefData::wxPlotCurveRefData() : wxObjectRefData()
{
    for (int i=0; i<wxPLOTPEN_MAXTYPE; i++)
        m_pens[i] = s_defaultPlotPens[i];
}

void wxPlotCurveRefData::Copy(const wxPlotCurveRefData &source)
{
    m_boundingRect = source.m_boundingRect;

    for (int i=0; i<wxPLOTPEN_MAXTYPE; i++)
        m_pens[i] = source.m_pens[i];
    
    m_optionNames  = source.m_optionNames;
    m_optionValues = source.m_optionValues;
}

#define M_PLOTCURVEDATA ((wxPlotCurveRefData*)m_refData)

//-----------------------------------------------------------------------------
// wxPlotCurve
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxPlotCurve, wxObject);

wxPlotCurve::wxPlotCurve() : wxObject()
{
    // Note: You must do this in your constructor in order to use the the curve
    // m_refData = new wxPlotCurveRefData (or wxMySubclassedPlotCurveRefData)
    // 
    
/*    
    // FIXME : not sure where to put these or even if I really want them
    if (!wxPlotSymbolNormal.Ok())
        wxPlotSymbolNormal = wxBitmap( normal_symbol_xpm_data );
    if (!wxPlotSymbolActive.Ok())
        wxPlotSymbolActive = wxBitmap( active_symbol_xpm_data );
    if (!wxPlotSymbolSelected.Ok())
        wxPlotSymbolSelected = wxBitmap( selected_symbol_xpm_data );
*/
}

bool wxPlotCurve::Ok() const
{
    return (M_PLOTCURVEDATA != NULL);
}

wxRect2DDouble wxPlotCurve::GetBoundingRect() const
{
    wxCHECK_MSG(Ok(), wxRect2DDouble(0,0,0,0), wxT("invalid plotcurve"));
    return M_PLOTCURVEDATA->m_boundingRect;
}
void wxPlotCurve::SetBoundingRect( const wxRect2DDouble &rect )
{
    wxCHECK_RET(Ok(), wxT("invalid plotcurve"));
    M_PLOTCURVEDATA->m_boundingRect = rect;
}

//----------------------------------------------------------------------------
// Get/Set Pen
//----------------------------------------------------------------------------

wxGenericPen wxPlotCurve::GetGenericPen(wxPlotPen_Type colour_type) const
{
    wxCHECK_MSG(Ok(), wxGenericPen(), wxT("invalid plotcurve"));
    wxCHECK_MSG((colour_type >= 0) && (colour_type < wxPLOTPEN_MAXTYPE), wxGenericPen(), wxT("invalid plot colour"));

    return M_PLOTCURVEDATA->m_pens[colour_type];
}
wxPen wxPlotCurve::GetPen(wxPlotPen_Type colour_type) const
{
    return GetGenericPen(colour_type).GetPen();
}
void wxPlotCurve::SetGenericPen(wxPlotPen_Type colour_type, const wxGenericPen &pen )
{
    wxCHECK_RET(Ok(), wxT("invalid plotcurve"));
    wxCHECK_RET((colour_type >= 0) && (colour_type < wxPLOTPEN_MAXTYPE), wxT("invalid plot colour"));

    M_PLOTCURVEDATA->m_pens[colour_type] = pen;
}
void wxPlotCurve::SetPen(wxPlotPen_Type colour_type, const wxPen &pen )
{
    SetGenericPen(colour_type, wxGenericPen(pen));
}

wxGenericPen wxPlotCurve::GetDefaultGenericPen(wxPlotPen_Type colour_type)
{
    wxCHECK_MSG((colour_type >= 0) && (colour_type < wxPLOTPEN_MAXTYPE), wxGenericPen(), wxT("invalid plot colour"));

    return s_defaultPlotPens[colour_type];    
}
wxPen wxPlotCurve::GetDefaultPen(wxPlotPen_Type colour_type)
{
    return GetDefaultGenericPen(colour_type).GetPen();
}
void wxPlotCurve::SetDefaultGenericPen(wxPlotPen_Type colour_type, const wxGenericPen &pen )
{
    wxCHECK_RET((colour_type >= 0) && (colour_type < wxPLOTPEN_MAXTYPE), wxT("invalid plot colour"));
    s_defaultPlotPens[colour_type] = pen;
}
void wxPlotCurve::SetDefaultPen(wxPlotPen_Type colour_type, const wxPen &pen )
{
    SetDefaultGenericPen(colour_type, wxGenericPen(pen));
}

// ----------------------------------------------------------------------------
// Get/Set Option names/values
// ----------------------------------------------------------------------------

wxArrayString wxPlotCurve::GetOptionNames() const
{
    wxCHECK_MSG(M_PLOTCURVEDATA, wxArrayString(), wxT("invalid plotcurve"));
    return M_PLOTCURVEDATA->m_optionNames;
}
wxArrayString wxPlotCurve::GetOptionValues() const
{
    wxCHECK_MSG(M_PLOTCURVEDATA, wxArrayString(), wxT("invalid plotcurve"));
    return M_PLOTCURVEDATA->m_optionValues;
}
int wxPlotCurve::GetOptionCount() const
{
    wxCHECK_MSG(M_PLOTCURVEDATA, 0, wxT("invalid plotcurve"));
    return M_PLOTCURVEDATA->m_optionNames.GetCount();
}
wxString wxPlotCurve::GetOptionName( int i ) const
{
    wxCHECK_MSG(M_PLOTCURVEDATA&&(i>=0)&&(i<GetOptionCount()), wxEmptyString, wxT("invalid plotcurve") );
    return M_PLOTCURVEDATA->m_optionNames[i];
}
wxString wxPlotCurve::GetOptionValue( int i ) const
{
    wxCHECK_MSG(M_PLOTCURVEDATA&&(i>=0)&&(i<GetOptionCount()), wxEmptyString, wxT("invalid plotcurve") );
    return M_PLOTCURVEDATA->m_optionValues[i];
}

void wxPlotCurve::SetOption(const wxString& name, const wxString& value, bool update)
{
    wxCHECK_RET(M_PLOTCURVEDATA, wxT("invalid plotcurve"));
    int n = M_PLOTCURVEDATA->m_optionNames.Index(name, false);
    if (n == wxNOT_FOUND)
    {
        M_PLOTCURVEDATA->m_optionNames.Add(name);
        M_PLOTCURVEDATA->m_optionValues.Add(value);
    }
    else if (update)
    {
        M_PLOTCURVEDATA->m_optionNames[n] = name;
        M_PLOTCURVEDATA->m_optionValues[n] = value;
    }
}

void wxPlotCurve::SetOption(const wxString &name, int option, bool update)
{
    SetOption(name, wxString::Format(wxT("%d"), option), update);
}

wxString wxPlotCurve::GetOption(const wxString& name) const
{
    wxCHECK_MSG(M_PLOTCURVEDATA, wxEmptyString, wxT("invalid plotcurve"));
    int n = M_PLOTCURVEDATA->m_optionNames.Index(name, false);
    
    if (n == wxNOT_FOUND) 
        return wxEmptyString;
        
    return M_PLOTCURVEDATA->m_optionValues[n];    
}

int wxPlotCurve::GetOption(const wxString& name, wxString &value ) const
{
    wxCHECK_MSG(M_PLOTCURVEDATA, wxNOT_FOUND, wxT("invalid plotcurve"));
    
    int n = M_PLOTCURVEDATA->m_optionNames.Index(name, false);

    if (n == wxNOT_FOUND) return wxNOT_FOUND;

    value = M_PLOTCURVEDATA->m_optionValues[n];
    return n;
}

int wxPlotCurve::GetOptionInt(const wxString& name) const
{
    wxCHECK_MSG(M_PLOTCURVEDATA, 0, wxT("invalid plotcurve"));
    return wxAtoi(GetOption(name));
}

int wxPlotCurve::HasOption(const wxString& name) const
{
    wxCHECK_MSG(M_PLOTCURVEDATA, wxNOT_FOUND, wxT("invalid plotcurve"));
    return M_PLOTCURVEDATA->m_optionNames.Index(name, false);
}
