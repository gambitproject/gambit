/////////////////////////////////////////////////////////////////////////////
// Name:        plotcurv.h
// Purpose:     wxPlotCurve for wxPlotWindow
// Author:      John Labenski
// Modified by:
// Created:     12/1/2000
// Copyright:   (c) John Labenski
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PLOTCURVE_H_
#define _WX_PLOTCURVE_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "plotcurv.h"
#endif

#include "wx/defs.h"
#include "wx/object.h"
#include "wx/geometry.h"
#include "wx/wxthings/genergdi.h"
#include "wx/plotctrl/plotdefs.h"

#ifdef GetYValue   // Visual Studio 7 defines this
    #undef GetYValue
#endif

//-----------------------------------------------------------------------------
// Utility functions
//-----------------------------------------------------------------------------
// Find y at point x along the line from (x0,y0)-(x1,y1), x0 must != x1
extern double LinearInterpolateY( double x0, double y0, 
                                  double x1, double y1,
                                  double x );
// Find x at point y along the line from (x0,y0)-(x1,y1), y0 must != y1
extern double LinearInterpolateX( double x0, double y0,
                                  double x1, double y1,
                                  double y );

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------

// defines wxArrayDouble for use as necessary
WX_DEFINE_TYPEARRAY_WITH_DECL_PTR(double, wxArrayDouble, wxBaseArrayDouble, class WXDLLIMPEXP_PLOTLIB);

// wxNullPlotBounds = wxRect2DDouble(0,0,0,0)
WXDLLIMPEXP_DATA_PLOTLIB(extern const wxRect2DDouble) wxNullPlotBounds;


extern wxBitmap wxPlotSymbolNormal;
extern wxBitmap wxPlotSymbolActive;
extern wxBitmap wxPlotSymbolSelected;

enum wxPlotSymbol_Type
{
    wxPLOTSYMBOL_ELLIPSE,
    wxPLOTSYMBOL_RECTANGLE,
    wxPLOTSYMBOL_CROSS,
    wxPLOTSYMBOL_PLUS,
    wxPLOTSYMBOL_MAXTYPE
};

enum wxPlotPen_Type
{
    wxPLOTPEN_NORMAL,
    wxPLOTPEN_ACTIVE,
    wxPLOTPEN_SELECTED,
    wxPLOTPEN_MAXTYPE
};

#define wxPLOTCURVE_OPTION_FILENAME      wxT("FileName")
#define wxPLOTCURVE_OPTION_EOLMODE       wxT("EOLMode")
#define wxPLOTCURVE_OPTION_MODIFIED      wxT("Modified")
#define wxPLOTCURVE_OPTION_HEADER        wxT("Header")
#define wxPLOTCURVE_OPTION_DATASEPARATOR wxT("DataSeparator")

#define wxPLOTCURVE_DATASEPARATOR_SPACE wxT(" ")
#define wxPLOTCURVE_DATASEPARATOR_COMMA wxT(",")
#define wxPLOTCURVE_DATASEPARATOR_TAB   wxT("\t")

//----------------------------------------------------------------------------
// wxPlotCurveRefData - the wxObject::m_refData used for wxPlotCurves
//   this should be the base class for ref data for your subclassed curves
//----------------------------------------------------------------------------

class WXDLLIMPEXP_PLOTLIB wxPlotCurveRefData: public wxObjectRefData
{
public:
    wxPlotCurveRefData();
    virtual ~wxPlotCurveRefData() {}

    virtual void Copy(const wxPlotCurveRefData &source);

    wxRect2DDouble m_boundingRect;

    wxGenericPen m_pens[wxPLOTPEN_MAXTYPE]; 

    wxArrayString m_optionNames;
    wxArrayString m_optionValues;
};

//-----------------------------------------------------------------------------
// wxPlotCurve - generic base plot curve class that must be subclassed for use
// 
// GetY must be overridden as it "is" the curve
//-----------------------------------------------------------------------------
class WXDLLIMPEXP_PLOTLIB wxPlotCurve: public wxObject
{
public:
    // see the remmed out code in this function if you subclass it
    wxPlotCurve();
    virtual ~wxPlotCurve() {}
    
    // override as necessary so that Ok means that GetY works
    virtual bool Ok() const;

    virtual double GetY( double WXUNUSED(x) ) { return 0.0; } // Override for output
    
    // Bounding rect used for drawing the curve and...
    //   if the width or height <= 0 then there's no bounds (or unknown)
    //   wxPlotCurve/Function : may be unknown and should probably be (0,0,0,0)
    //                you can limit the extent by setting to a smaller rect
    //   wxPlotData : calculated from CalcBoundingRect and is well defined
    //                DON'T call SetBoundingRect unless you know what you're doing
    virtual wxRect2DDouble GetBoundingRect() const;
    virtual void SetBoundingRect( const wxRect2DDouble &rect );

    // Get/Set Pens for Normal, Active, Selected drawing
    // if these are not set it resorts to the defaults
    wxPen GetPen(wxPlotPen_Type colour_type) const;
    wxGenericPen GetGenericPen(wxPlotPen_Type colour_type) const;
    void SetPen(wxPlotPen_Type colour_type, const wxPen &pen);
    void SetGenericPen(wxPlotPen_Type colour_type, const wxGenericPen &pen);
    
    // Get/Set Default Pens for Normal, Active, Selected drawing for all curves
    //   these are the pens that are used when a wxPlotCurve/Function/Data is created
    //   default: Normal(0,0,0,1,wxSOLID), Active(0,0,255,1,wxSOLID), Selected(255,0,0,1,wxSOLID)
    static wxPen GetDefaultPen(wxPlotPen_Type colour_type);
    static wxGenericPen GetDefaultGenericPen(wxPlotPen_Type colour_type);
    static void SetDefaultPen(wxPlotPen_Type colour_type, const wxPen &pen);
    static void SetDefaultGenericPen(wxPlotPen_Type colour_type, const wxGenericPen &pen);
    
    //-------------------------------------------------------------------------
    // Get/Set Option names/values
    //-------------------------------------------------------------------------

    // return the index of the option or wxNOT_FOUND (-1)
    int HasOption(const wxString& name) const;
    // Get the name/value at the index position
    wxString GetOptionName( int index ) const;
    wxString GetOptionValue( int index ) const;
    // Set an option, if update=true then force it, else only set it if not found
    void SetOption(const wxString& name, const wxString& value, bool update=true);
    void SetOption(const wxString& name, int option, bool update=true);
    // Get an option returns the index if found
    //   returns wxNOT_FOUND (-1) if it doesn't exist and value isn't changed
    int GetOption(const wxString& name, wxString& value) const;
    // returns wxEmptyString if not found
    wxString GetOption(const wxString& name) const;
    // returns 0 if not found
    int GetOptionInt(const wxString& name) const;
    // get the arrays of option values
    wxArrayString GetOptionNames() const;
    wxArrayString GetOptionValues() const; 
    int GetOptionCount() const;

    // operators
    bool operator == (const wxPlotCurve& plotCurve)
        { return m_refData == plotCurve.m_refData; }
    bool operator != (const wxPlotCurve& plotCurve)
        { return m_refData != plotCurve.m_refData; }

    wxPlotCurve& operator = (const wxPlotCurve& plotCurve)
    {
        if ( (*this) != plotCurve )
            Ref(plotCurve);
        return *this;
    }

private :
    DECLARE_ABSTRACT_CLASS(wxPlotCurve);
};

#endif // _WX_PLOTCURVE_H_
