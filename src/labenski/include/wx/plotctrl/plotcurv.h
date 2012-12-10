/////////////////////////////////////////////////////////////////////////////
// Name:        plotcurv.h
// Purpose:     wxPlotCurve for wxPlotCtrl
// Author:      John Labenski
// Modified by:
// Created:     12/1/2000
// Copyright:   (c) John Labenski
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PLOTCURVE_H_
#define _WX_PLOTCURVE_H_

#include "wx/defs.h"
#include "wx/object.h"
#include "wx/clntdata.h"
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
#if !wxCHECK_VERSION(2,7,0)
WX_DEFINE_USER_EXPORTED_ARRAY_DOUBLE(double, wxArrayDouble, class WXDLLIMPEXP_PLOTCTRL);
#endif

// wxNullPlotBounds = wxRect2DDouble(0,0,0,0)
WXDLLIMPEXP_DATA_PLOTCTRL(extern const wxRect2DDouble) wxNullPlotBounds;

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

#define wxPLOTCURVE_OPTION_FILENAME      wxT("File.Name")
#define wxPLOTCURVE_OPTION_EOLMODE       wxT("EOL.Mode")
#define wxPLOTCURVE_OPTION_MODIFIED      wxT("Modified")
#define wxPLOTCURVE_OPTION_HEADER        wxT("Header")
#define wxPLOTCURVE_OPTION_DATASEPARATOR wxT("Data.Separator")

#define wxPLOTCURVE_DATASEPARATOR_SPACE wxT(" ")
#define wxPLOTCURVE_DATASEPARATOR_COMMA wxT(",")
#define wxPLOTCURVE_DATASEPARATOR_TAB   wxT("\t")

//----------------------------------------------------------------------------
// wxPlotCurveRefData - the wxObject::m_refData used for wxPlotCurves
//   this should be the base class for ref data for your subclassed curves
//
//   The ref data is also of class wxClientDataContainer so that you can
//     attach arbitrary data to it
//----------------------------------------------------------------------------

class WXDLLIMPEXP_PLOTCTRL wxPlotCurveRefData : public wxObjectRefData, public wxClientDataContainer
{
public:
    wxPlotCurveRefData();
    wxPlotCurveRefData(const wxPlotCurveRefData& data);
    virtual ~wxPlotCurveRefData() {}

    void Copy(const wxPlotCurveRefData &source);

    wxRect2DDouble m_boundingRect; // bounds the curve or part to draw
                                   // if width or height <= 0 then no bounds

    wxArrayGenericPen m_pens;
    static wxArrayGenericPen sm_defaultPens;

    wxSortedArrayString m_optionNames;
    wxArrayString       m_optionValues;
};

//-----------------------------------------------------------------------------
// wxPlotCurve - generic base plot curve class that must be subclassed for use
//
// GetY must be overridden as it "is" the curve
//-----------------------------------------------------------------------------
class WXDLLIMPEXP_PLOTCTRL wxPlotCurve: public wxObject
{
public:
    // see the remmed out code in this function if you subclass it
    wxPlotCurve();
    virtual ~wxPlotCurve() {}

    // override as necessary so that Ok means that GetY works
    virtual bool Ok() const;

    // This *is* the output of the curve y = f(x)
    virtual double GetY( double WXUNUSED(x) ) { return 0.0; }

    // Bounding rect used for drawing the curve and...
    //   if the width or height <= 0 then there's no bounds (or unknown)
    //   wxPlotCurve/Function : may be unknown and should probably be (0,0,0,0)
    //                you can limit the extent by setting to a smaller rect
    //   wxPlotData : calculated from CalcBoundingRect and is well defined
    //                DON'T call SetBoundingRect unless you know what you're doing
    virtual wxRect2DDouble GetBoundingRect() const;
    virtual void SetBoundingRect( const wxRect2DDouble &rect );

    // Get/Set Pens for Normal, Active, Selected drawing
    //  if these are not set it resorts to the defaults
    wxGenericPen GetPen(wxPlotPen_Type colour_type) const;
    void SetPen(wxPlotPen_Type colour_type, const wxGenericPen &pen);

    // Get/Set Default Pens for Normal, Active, Selected drawing for all curves
    //   these are the pens that are used when a wxPlotCurve/Function/Data is created
    //   default: Normal(0,0,0,1,wxSOLID), Active(0,0,255,1,wxSOLID), Selected(255,0,0,1,wxSOLID)
    static wxGenericPen GetDefaultPen(wxPlotPen_Type colour_type);
    static void SetDefaultPen(wxPlotPen_Type colour_type, const wxGenericPen &pen);

    //-------------------------------------------------------------------------
    // Get/Set Option names/values
    //-------------------------------------------------------------------------

    // Get the number of options set
    size_t GetOptionCount() const;
    // return the index of the option or wxNOT_FOUND (-1)
    int HasOption(const wxString& name) const;
    // Get the name/value at the index position
    wxString GetOptionName( size_t index ) const;
    wxString GetOptionValue( size_t index ) const;
    // Set an option, if update=true then force it, else only set it if not found
    //   returns the index of the option
    int SetOption(const wxString& name, const wxString& value, bool update=true);
    int SetOption(const wxString& name, int option, bool update=true);
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

    //-------------------------------------------------------------------------
    // Get/Set the ClientData in the ref data - see wxClientDataContainer
    //  You can store any extra info here.
    //-------------------------------------------------------------------------

    void SetClientObject( wxClientData *data );
    wxClientData *GetClientObject() const;

    void SetClientData( void *data );
    void *GetClientData() const;

    //-------------------------------------------------------------------------
    // operators
    bool operator == (const wxPlotCurve& plotCurve) const
        { return m_refData == plotCurve.m_refData; }
    bool operator != (const wxPlotCurve& plotCurve) const
        { return m_refData != plotCurve.m_refData; }

    wxPlotCurve& operator = (const wxPlotCurve& plotCurve)
    {
        if ( (*this) != plotCurve )
            Ref(plotCurve);
        return *this;
    }

private :
    // ref counting code
    virtual wxObjectRefData *CreateRefData() const;
    virtual wxObjectRefData *CloneRefData(const wxObjectRefData *data) const;

    DECLARE_DYNAMIC_CLASS(wxPlotCurve);
};

#endif // _WX_PLOTCURVE_H_
