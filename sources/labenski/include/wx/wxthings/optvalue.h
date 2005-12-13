/////////////////////////////////////////////////////////////////////////////
// Name:        optvalue.h
// Purpose:     An string option & value pair class
// Author:      John Labenski
// Created:     07/01/02
// Copyright:   John Labenski, 2002
// License:     wxWidgets v2
/////////////////////////////////////////////////////////////////////////////

#ifndef __WXIMAGER_OPTVALU_H__
#define __WXIMAGER_OPTVALU_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "optvalue.h"
#endif

#include "wx/wxthings/thingdef.h"
class WXDLLIMPEXP_THINGS wxOptionValue;

#include "wx/dynarray.h"
WX_DECLARE_OBJARRAY(wxOptionValue, wxArrayOptionValue);

//----------------------------------------------------------------------------
// Global wxString utilities
//----------------------------------------------------------------------------

//extern wxArrayString wxStringToWords( const wxString &string );

//----------------------------------------------------------------------------
// wxOptionValue
//----------------------------------------------------------------------------

class WXDLLIMPEXP_THINGS wxOptionValue : public wxObject
{
public:
    wxOptionValue():wxObject() { Create(); }

    wxOptionValue( const wxOptionValue &optvalue ):wxObject() { Create(optvalue); }
    
    // must set the type with the first entry [Type]
    wxOptionValue( const wxString &string ):wxObject() { Create(string); }
    
    bool Create();
    bool Create( const wxOptionValue &optvalue ); // increases ref count
    bool Create( const wxString &string );
    
    // make a true copy of this (not refed)
    bool Copy( const wxOptionValue &source );

    bool Ok() const; 
    void Destroy();

    //-------------------------------------------------------------------------
    
    wxString GetType() const;
    void SetType( const wxString &type );
    
    //-------------------------------------------------------------------------
    
    // does this have a wxOptionValueArray filled with children
    int HasChildren() const;
    wxArrayOptionValue *GetChildren() const;
    
    bool AddChild( wxOptionValue *child );
    void DeleteChildren(); 
    
    //-------------------------------------------------------------------------

    // Get the number of different option name/value combinations
    int GetOptionCount() const;
    // Access the arrays themselves
    wxArrayString GetOptionNames() const;
    wxArrayString GetOptionValues() const; 
    // Get a specific option name or value
    wxString GetOptionName( int n ) const;
    wxString GetOptionValue( int n ) const;

    // returns the index of the option >= 0 or -1 (wxNOT_FOUND) if not found
    int HasOption(const wxString &option) const;
    // Search through the option names for this part returning the first match
    int FindOption(const wxString &part_of_option_name) const;
    // delete this option, returns sucess
    bool DeleteOption(const wxString &name);
    bool DeleteOption( int n );

    // Option functions (arbitrary name/value mapping)
    void SetOption(const wxString& name, const wxString& value, bool force=true );
    void SetOption(const wxString& name, int value,             bool force=true ) { SetOption(name, wxString::Format(wxT("%d"), value), force); }
    void SetOption(const wxString& name, float value,           bool force=true ) { SetOption(name, wxString::Format(wxT("%f"), value), force); }
    void SetOption(const wxString& name, double value,          bool force=true ) { SetOption(name, wxString::Format(wxT("%lf"), value), force); }
   
    // printf style for numeric values SetOption("Name", true, "%d %f", 2, 2.5)
    void SetOption(const wxString& name, bool update, const wxChar* format, ...);
    
    void SetOption(const wxString& name, int v1,   int v2,   int v3,    bool force=true ) { SetOption(name, wxString::Format(wxT("%d %d %d"), v1, v2, v3), force); }
    void SetOption(const wxString& name, float v1, float v2, float v3,  bool force=true ) { SetOption(name, wxString::Format(wxT("%f %f %f"), v1, v2, v3), force); }
    void SetOption(const wxString& name, int *v,   int count,           bool force=true ) { if(v) { wxString s; for (int i=0; i<count; i++) s += wxString::Format(wxT("%d "), v[i]); SetOption(name, s, force); }}
    void SetOption(const wxString& name, float *v, int count,           bool force=true ) { if(v) { wxString s; for (int i=0; i<count; i++) s += wxString::Format(wxT("%f "), v[i]); SetOption(name, s, force); }}
    void SetOption(const wxString& name, const wxPoint &value,          bool force=true ) { SetOption(name, wxString::Format(wxT("%d %d"), value.x, value.y), force); }
    void SetOption(const wxString& name, const wxSize &value,           bool force=true ) { SetOption(name, wxString::Format(wxT("%d %d"), value.x, value.y), force); }
    void SetOption(const wxString& name, const wxRect &value,           bool force=true ) { SetOption(name, wxString::Format(wxT("%d %d %d %d"), value.x, value.y, value.width, value.height), force); }

    wxString GetOption(const wxString& name) const; // returns wxEmptyString if not found
    int GetOptionInt(const wxString& name) const;   // returns 0 if not found

    // These return true on sucess otherwise the value is not modified
    bool GetOption(const wxString& name, wxString &value ) const;
    bool GetOption(const wxString& name, int *value ) const;
    bool GetOption(const wxString& name, float *value ) const;
    bool GetOption(const wxString& name, double *value ) const;

    // sscanf style for numeric values GetOption("Name", "%d %f", &n_int, &n_float)
    int GetOption(const wxString& name, const wxChar* format, ...) const;

    int GetOption(const wxString& name, wxArrayInt &values) const;
        
    bool GetOption(const wxString& name, unsigned char *value, int count ) const;
    bool GetOption(const wxString& name, int *value, int count ) const;
    bool GetOption(const wxString& name, long *value, int count ) const;
    bool GetOption(const wxString& name, float *value, int count ) const;
    bool GetOption(const wxString& name, double *value, int count ) const;

    bool GetOption(const wxString& name, int *v1, int *v2, int *v3 ) const;
    bool GetOption(const wxString& name, float *v1, float *v2, float *v3 ) const;
    
    bool GetOption(const wxString& name, wxPoint &value ) const;
    bool GetOption(const wxString& name, wxSize &value ) const;
    bool GetOption(const wxString& name, wxRect &value ) const;

    //-------------------------------------------------------------------------
    // Operators
    
    wxOptionValue& operator = (const wxOptionValue& optvalue)
    {
        if ( (*this) != optvalue ) wxObject::Ref(optvalue);
        return *this;
    }
    bool operator == (const wxOptionValue& optvalue) { return m_refData == optvalue.m_refData; }
    bool operator != (const wxOptionValue& optvalue) { return m_refData != optvalue.m_refData; }
    
private :
    
    DECLARE_DYNAMIC_CLASS(wxOptionValue);
};

#endif // __WXIMAGER_OPTVALU_H__
