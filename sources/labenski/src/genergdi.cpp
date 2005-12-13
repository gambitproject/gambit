/////////////////////////////////////////////////////////////////////////////
// Name:        genergdi.cpp
// Purpose:     Generic gdi pen and colour
// Author:      John Labenski
// Modified by:
// Created:     12/01/2000
// Copyright:   (c) John Labenski
// Licence:     wxWidgets license
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "genergdi.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/wxthings/genergdi.h"
#include "wx/tokenzr.h"

const wxGenericColour wxNullGenericColour;
const wxGenericPen    wxNullGenericPen; 
#include "wx/arrimpl.cpp"
WX_DEFINE_OBJARRAY(wxArrayGenericPen)

//----------------------------------------------------------------------------
// wxGenericColour
//----------------------------------------------------------------------------

// This code is assumed to be public domain, originally from Paul Bourke, July 1996
// http://astronomy.swin.edu.au/~pbourke/colour/colourramp/source1.c

wxGenericColour wxGenericColour::GetHotColdColour(double v) const
{
    wxGenericColour c(255, 255, 255);
    const double vmin = 0.0, vmax = 255.0, dv = vmax - vmin;

    if (v < vmin)
        v = vmin;
    if (v > vmax)
        v = vmax;

    if (v < (vmin + 0.25 * dv)) 
    {
        c.m_r = 0;
        c.m_g = int(255.0*(4.0 * (v - vmin) / dv) + 0.5);
    } 
    else if (v < (vmin + 0.5 * dv)) 
    {
        c.m_r = 0;
        c.m_b = int(255.0*(1.0 + 4.0 * (vmin + 0.25 * dv - v) / dv) + 0.5);
    } 
    else if (v < (vmin + 0.75 * dv)) 
    {
        c.m_r = int(255.0*(4.0 * (v - vmin - 0.5 * dv) / dv) + 0.5);
        c.m_b = 0;
    } 
    else 
    {
        c.m_g = int(255.0*(1.0 + 4.0 * (vmin + 0.75 * dv - v) / dv) + 0.5);
        c.m_b = 0;
    }
    
    return c;
}
/*
wxString wxGenericColour::WriteString(const wxString& format) const 
{ 
    return wxString::Format(format.c_str(), m_r, m_g, m_b, m_a); 
}
bool wxGenericColour::ReadString(const wxString& str, const wxString& format) 
{ 
    int r,g,b,a; 
    if (4 == wxSscanf(str, format.c_str(), &r, &g, &b, &a))
    {
        m_r = r;
        m_g = g;
        m_b = b;
        m_a = a;
        return true;
    }        
    
    return false;
}
*/
//----------------------------------------------------------------------------
// wxGenericPen
//----------------------------------------------------------------------------

wxGenericPen::~wxGenericPen()
{
    if (m_dash) free(m_dash);
}

void wxGenericPen::Copy(const wxGenericPen &pen)
{
    m_colour = pen.m_colour;
    m_width  = pen.m_width;
    m_style  = pen.m_style;
    m_cap    = pen.m_cap;
    m_join   = pen.m_join;
    SetDashes(pen.GetDashCount(), pen.GetDash());
}

bool wxGenericPen::IsEqual(const wxGenericPen &pen) const
{
    if ((m_colour != pen.m_colour) || (m_width != pen.m_width) ||
        (m_style != pen.m_style) || (m_cap != pen.m_cap) || 
        (m_join != pen.m_join) || (m_dash_count != pen.GetDashCount()))
        return false;

    if (m_dash_count > 0)
        return memcmp(m_dash, pen.GetDash(), m_dash_count*sizeof(wxDash)) == 0;
    
    return true;
}

wxPen wxGenericPen::GetPen() const
{
    wxPen pen(m_colour.GetColour(), m_width, m_style);
    pen.SetCap(m_cap);
    pen.SetJoin(m_join);
    if (m_dash_count)
        pen.SetDashes(m_dash_count, m_dash);

    return pen;
}

void wxGenericPen::SetPen(const wxPen &pen)
{
    SetColour(pen.GetColour());
    m_width = pen.GetWidth();
    m_style = pen.GetStyle();
    m_cap   = pen.GetCap();
    m_join  = pen.GetJoin();

    wxDash* dash;
    int n_dashes = pen.GetDashes(&dash);
    SetDashes(n_dashes, dash);    
    
    // or SetDashes(pen.GetDashCount(), pen.GetDash()); not in msw 2.4
}

void wxGenericPen::SetDashes( int number_of_dashes, const wxDash *dash )
{
    wxCHECK_RET(((number_of_dashes == 0) && !dash) || 
                ((number_of_dashes >  0) &&  dash), wxT("Invalid dashes for pen"));

    // internal double check to see if somebody's messed with this
    //wxCHECK_RET(((m_dash_count == 0) && !m_dash) || 
    //            ((m_dash_count != 0) &&  m_dash), wxT("Invalid internal dashes for pen"));
    
    if (m_dash) 
    {
        free(m_dash);
        m_dash = NULL;
        m_dash_count = 0;
    }

    if (!dash)
        return;

    m_dash_count = number_of_dashes;
    m_dash = (wxDash*)malloc(number_of_dashes*sizeof(wxDash));
    memcpy(m_dash, dash, number_of_dashes*sizeof(wxDash));
}
/*
wxString wxGenericPen::WriteString() const 
{ 
    wxString str;
    str.Printf(wxT("%s,%d,%d,%d,%d,%d"), m_colour.WriteString().c_str(),
                                         m_width, m_style, m_cap, m_join, 
                                         m_dash_count); 
    
    for (int i = 0; i < m_dash_count; i++)
        str += wxString::Format(wxT(",%d"), m_dash[i]);
    
    return str;
}

bool wxGenericPen::ReadString(const wxString& str) 
{ 
    wxArrayString tokens = wxStringTokenize(str, wxT(", "), wxTOKEN_DEFAULT);
    size_t n, count = tokens.GetCount();

    if (count < 9u)
        return false;

    long val;
    
    for (n = 0; n < count; n++)
    {
        if (!tokens[n].ToLong(&val))
            return false;
        
        values.Add(int(val));
    }

    m_colour.Set(values[0], values[1], values[2], values[3]);
    
    size_t num_dashes = values[8];
    
    if (num_dashes != count - 9) 
        return false;

    if (num_dashes > 0)
    {
        wxDash *dash = new wxDash[num_dashes];
        for (n = 0; n < num_dashes; n++)
            dash[n] = (wxDash)values[n];
        
        SetDashes(num_dashes, dash);
        delete dash;
    }
    
    
    m_width = values[4];
    m_style = values[5];
    m_cap   = values[6];
    m_join  = values[7];
    //m_dash_count = values[8];
    
    return true;
}
*/
