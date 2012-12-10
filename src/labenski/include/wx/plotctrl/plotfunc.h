/////////////////////////////////////////////////////////////////////////////
// Name:        plotfunc.h
// Purpose:     wxPlotFunction curve for wxPlotCtrl
// Author:      John Labenski
// Modified by:
// Created:     12/1/2000
// Copyright:   (c) John Labenski
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PLOTFUNC_H_
#define _WX_PLOTFUNC_H_

#include "wx/plotctrl/plotcurv.h"

//-----------------------------------------------------------------------------
// wxPlotFunction - a compiled string function parsing PlotCurve
//
// see fparser.txt to see function string capability
// vars is "x" or "x,y" where x and y are the vars used in the function string
//
//-----------------------------------------------------------------------------
class WXDLLIMPEXP_PLOTCTRL wxPlotFunction: public wxPlotCurve
{
public:
    wxPlotFunction() : wxPlotCurve() {}
    wxPlotFunction(const wxPlotFunction &curve) : wxPlotCurve() { Create(curve); }
    wxPlotFunction(const wxString &function, const wxString &vars = wxT("x"),
                   bool useDegrees = false) : wxPlotCurve()
        { Create(function, vars, useDegrees); }

    virtual ~wxPlotFunction() {}

    // Initialize curve, returns -1 on sucess and errormsg is wxEmptyString.
    //   On error, the errormsg contains a message and the character location
    //   in the 'function' string where the error occurred is returned.
    //   Check if errormsg != wxEmptyString, or curve.Ok() not the returned int
    //   vars is a comma delimited list of variables used in the function "x,y,z"
    //   vars can be wxEmptyString, see GetValue(NULL), the function is a constant
    int Create( const wxString &function, const wxString &vars = wxT("x"),
                bool useDegrees = false );
    // Make a copy of the other plotFunction
    bool Create( const wxPlotFunction &curve );

    bool Ok() const;
    void Destroy();

    // Parse and use the function, see Create, does not destroy refdata
    int Parse( const wxString &function, const wxString &vars = wxT("x"),
               bool useDegrees = false );

    // Get the strings sent to Create
    wxString GetFunctionString() const;       // the function string "a*(b+3)/2 + c"
    wxString GetVariableString() const;       // the variable string "a,b,c"
    wxString GetVariableName(size_t n) const; // a single variable name n=0="a"
    int GetNumberVariables() const;
    bool GetUseDegrees() const;               // was this created to use degrees?

    // Get an error message (if any) if Create does not return -1 false
    wxString GetErrorMsg() const;

    // Get the f(x) value from the function parser, use this if only have x variable
    double GetY( double x );
    // Get the f(x,y...) using an arbitrary number of vars
    //   you absolutely MUST pass double x[GetNumberVariables()]
    //   if you sent in a wxEmptyString for the vars then use NULL
    double GetValue( double *x );

    // add a constant to use
    bool AddConstant(const wxString& name, double value);

    bool operator == (const wxPlotFunction& plotFunc) const
        { return m_refData == plotFunc.m_refData; }
    bool operator != (const wxPlotFunction& plotFunc) const
        { return m_refData != plotFunc.m_refData; }

    wxPlotFunction& operator = (const wxPlotFunction& plotFunc)
    {
        if ( (*this) != plotFunc )
            Ref(plotFunc);
        return *this;
    }

private :
    // ref counting code
    virtual wxObjectRefData *CreateRefData() const;
    virtual wxObjectRefData *CloneRefData(const wxObjectRefData *data) const;

    DECLARE_DYNAMIC_CLASS(wxPlotFunction);
};


//-----------------------------------------------------------------------------
// Functions for getting/setting a wxPlotFunction to/from the wxClipboard
//-----------------------------------------------------------------------------

#if wxUSE_DATAOBJ && wxUSE_CLIPBOARD

// Try to get a wxPlotFunction from the wxClipboard, return !Ok plotfunction on failure
wxPlotFunction wxClipboardGetPlotFunction();
// Set the plot function curve into the clipboard. Copies function and vars string
// separated by a ';' as a text object, use wxString::Before/AfterLast(wxT(';')).
// returns sucess
bool wxClipboardSetPlotFunction(const wxPlotFunction& plotFunction);


#endif // wxUSE_DATAOBJ && wxUSE_CLIPBOARD

#endif // _WX_PLOTFUNC_H_
