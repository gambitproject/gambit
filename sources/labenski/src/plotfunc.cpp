/////////////////////////////////////////////////////////////////////////////
// Name:        plotfunc.cpp
// Purpose:     wxPlotWindow
// Author:      John Labenski
// Modified by:
// Created:     12/01/2000
// Copyright:   (c) John Labenski
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "plotfunc.h"
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

#include "wx/plotctrl/plotfunc.h"
#include "wx/plotctrl/fparser.h"
#include <math.h>

//----------------------------------------------------------------------------
// wxPlotFuncRefData
//----------------------------------------------------------------------------

class wxPlotFuncRefData: public wxPlotCurveRefData
{
public:
    wxPlotFuncRefData() : wxPlotCurveRefData() {}

    wxFunctionParser m_parser;
};

#define M_PLOTFUNCDATA ((wxPlotFuncRefData*)m_refData)

//-----------------------------------------------------------------------------
// wxPlotFunction
//-----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS(wxPlotFunction, wxPlotCurve);

bool wxPlotFunction::Create( const wxPlotFunction& curve )
{
    wxCHECK_MSG(curve.Ok(), false, wxT("invalid plot function"));
    UnRef();
    Ref(curve);
    return true;
}

void wxPlotFunction::Destroy()
{
    UnRef();
}

bool wxPlotFunction::Ok() const
{
    return m_refData && M_PLOTFUNCDATA->m_parser.Ok();
}

int wxPlotFunction::Create(const wxString &function, const wxString &vars)
{
    UnRef();
    
    m_refData = new wxPlotFuncRefData();
    wxCHECK_MSG(m_refData, 0, wxT("can't allocate memory")); 
    
    int i = M_PLOTFUNCDATA->m_parser.Parse(function, vars);

    if (!M_PLOTFUNCDATA->m_parser.ErrorMsg().IsEmpty()) 
        return i;
    
    return -1;
}

wxString wxPlotFunction::GetFunctionString() const
{
    wxCHECK_MSG(Ok(), wxEmptyString, wxT("invalid plotfunction"));
    return M_PLOTFUNCDATA->m_parser.GetFunctionString();
}
wxString wxPlotFunction::GetVariableString() const
{
    wxCHECK_MSG(Ok(), wxEmptyString, wxT("invalid plotfunction"));
    return M_PLOTFUNCDATA->m_parser.GetVariableString();
}
wxString wxPlotFunction::GetVariableName(size_t n) const
{
    wxCHECK_MSG(Ok(), wxEmptyString, wxT("invalid plotfunction"));
    wxCHECK_MSG((int(n) < GetNumberVariables()), wxEmptyString, wxT("invalid variable index"));    
    return M_PLOTFUNCDATA->m_parser.GetVariableName(n);
}
int wxPlotFunction::GetNumberVariables() const
{
    wxCHECK_MSG(Ok(), 0, wxT("Invalid plotfunction"));
    return M_PLOTFUNCDATA->m_parser.GetNumberVariables();    
}

wxString wxPlotFunction::GetErrorMsg() const
{
    wxCHECK_MSG(m_refData, wxEmptyString, wxT("Invalid plotfunction"));
    return M_PLOTFUNCDATA->m_parser.ErrorMsg();
}

double wxPlotFunction::GetY( double x )
{
    wxCHECK_MSG(Ok(), 0.0, wxT("invalid plotfunction"));
    return M_PLOTFUNCDATA->m_parser.Eval( &x );
}

double wxPlotFunction::GetValue( double *x )
{
    wxCHECK_MSG(Ok(), 0.0, wxT("invalid plotfunction"));
    return M_PLOTFUNCDATA->m_parser.Eval( x );
}

//-----------------------------------------------------------------------------
// wxClipboardGet/SetPlotFunction 
//-----------------------------------------------------------------------------

#include "wx/clipbrd.h"
#if wxUSE_DATAOBJ && wxUSE_CLIPBOARD

wxPlotFunction wxClipboardGetPlotFunction()
{
    bool is_opened = wxTheClipboard->IsOpened();
    wxPlotFunction plotFunc;

    if (is_opened || wxTheClipboard->Open())
    {
        wxTextDataObject textDataObject;
        if (wxTheClipboard->IsSupported(wxDataFormat(wxDF_TEXT)) && 
            wxTheClipboard->GetData(textDataObject))
        {
            wxString str = textDataObject.GetText();
            plotFunc.Create(str.BeforeLast(wxT(';')), str.AfterLast(wxT(';')));
        }
        
        if (!is_opened)
            wxTheClipboard->Close();
    }    
    
    return plotFunc;
}
bool wxClipboardSetPlotFunction(const wxPlotFunction& plotFunc)
{
    wxCHECK_MSG(plotFunc.Ok(), false, wxT("Invalid wxPlotFunction to copy to clipboard"))
    bool is_opened = wxTheClipboard->IsOpened();
    
    if (is_opened || wxTheClipboard->Open())
    {
        wxString str = plotFunc.GetFunctionString() + wxT(";") + plotFunc.GetVariableString();
        wxTextDataObject *textDataObject = new wxTextDataObject(str);
        wxTheClipboard->SetData( textDataObject );
        
        if (!is_opened)
            wxTheClipboard->Close();
        
        return true;
    }
    
    return false;
}

#endif // wxUSE_DATAOBJ && wxUSE_CLIPBOARD
