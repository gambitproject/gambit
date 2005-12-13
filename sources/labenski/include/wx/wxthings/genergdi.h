/////////////////////////////////////////////////////////////////////////////
// Name:        genergdi.h
// Purpose:     Generic gdi pen and colour 
// Author:      John Labenski
// Modified by:
// Created:     12/1/2000
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_GENERGDI_H_
#define _WX_GENERGDI_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "genergdi.h"
#endif

#include "wx/colour.h"
#include "wx/pen.h"
#include "wx/wxthings/thingdef.h"

class WXDLLIMPEXP_THINGS wxGenericColour;
class WXDLLIMPEXP_THINGS wxGenericPen;

WXDLLIMPEXP_DATA_THINGS(extern const wxGenericColour) wxNullGenericColour; // black
WXDLLIMPEXP_DATA_THINGS(extern const wxGenericPen) wxNullGenericPen;       // black

#include "wx/dynarray.h"
#ifndef WX_DECLARE_OBJARRAY_WITH_DECL // for wx2.4 backwards compatibility
    #define WX_DECLARE_OBJARRAY_WITH_DECL(T, name, expmode) WX_DECLARE_USER_EXPORTED_OBJARRAY(T, name, WXDLLIMPEXP_THINGS)
#endif
WX_DECLARE_OBJARRAY_WITH_DECL(wxGenericPen, wxArrayGenericPen, class WXDLLIMPEXP_THINGS);

//----------------------------------------------------------------------------
// wxGenericColour
//----------------------------------------------------------------------------

class WXDLLIMPEXP_THINGS wxGenericColour
{
public:
    wxGenericColour() : m_r(0), m_g(0), m_b(0), m_a(255) { }
  
    // Construct from RGB
    wxGenericColour( unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha=255 ) 
        : m_r(red), m_g(green), m_b(blue), m_a(alpha) {}
    wxGenericColour( unsigned long colRGBA ) { Set(colRGBA); }

    // Implicit conversion from the colour name
    wxGenericColour( const wxString &colourName ) { InitFromName(colourName); }
    wxGenericColour( const char *colourName )     { InitFromName( wxString::FromAscii(colourName) ); }
#if wxUSE_UNICODE
    wxGenericColour( const wxChar *colourName ) { InitFromName( wxString(colourName) ); }
#endif

    wxGenericColour( const wxColour& col ) { Set(col); }

    virtual ~wxGenericColour() {}

    inline void Copy( const wxGenericColour &c ) { m_r=c.m_r; m_g=c.m_g; m_b=c.m_b; m_a=c.m_a; }
    
    inline bool Ok() const { return true; } // compatibility w/ wxColour

    inline wxColour GetColour() const { return wxColour(m_r, m_g, m_b); }
    
    inline void Set( unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha=255 )
        { m_r=red; m_g=green; m_b=blue; m_a=alpha; }
    inline void Set( unsigned long colRGBA )
        { m_r=(unsigned char)(0xFF&colRGBA); 
          m_g=(unsigned char)(0xFF&(colRGBA >> 8)); 
          m_b=(unsigned char)(0xFF&(colRGBA >> 16)); 
          m_a=(unsigned char)(0xFF&(colRGBA >> 24)); }
    inline void Set( const wxColour& col )
        { m_r = col.Red(); m_g = col.Green(); m_b = col.Blue(); }
        
    inline void SetRed(unsigned char   r) { m_r = r; }
    inline void SetGreen(unsigned char g) { m_g = g; }
    inline void SetBlue(unsigned char  b) { m_b = b; }
    inline void SetAlpha(unsigned char a) { m_a = a; }

    inline unsigned char GetRed() const   { return m_r; }
    inline unsigned char GetGreen() const { return m_g; }
    inline unsigned char GetBlue() const  { return m_b; }
    inline unsigned char GetAlpha() const { return m_a; }
    
    // wxWidgets compatibility functions
    inline unsigned char Red() const   { return m_r; }
    inline unsigned char Green() const { return m_g; }
    inline unsigned char Blue() const  { return m_b; }
    inline unsigned char Alpha() const { return m_a; }

    // Get a "hot to cold" colour where i ranges from 0 to 255
    wxGenericColour GetHotColdColour(double i) const;

    // Read colour to and from string
    //wxString WriteString(const wxString& format = wxT("%d,%d,%d,%d")) const;
    //bool ReadString(const wxString& str, const wxString& format = wxT("%d,%d,%d,%d"));
    
    // operators
    inline wxGenericColour& operator = ( const wxColour& col ) { Set(col); return *this; }
    inline wxGenericColour& operator = ( const wxGenericColour& col ) { Copy(col); return *this; }
    inline bool operator == ( const wxGenericColour& c ) const 
        { return ((m_r==c.m_r)&&(m_g==c.m_g)&&(m_b==c.m_b)&&(m_a==c.m_a)); }
    inline bool operator != ( const wxGenericColour& c ) const 
        { return !(*this == c); }
    
    // implementation
    unsigned char m_r, m_g, m_b, m_a;
    
protected:
    // Helper functions
    void InitFromName(const wxString& colourName) { Set(wxColour(colourName)); }
};

//----------------------------------------------------------------------------
// wxGenericPen
//----------------------------------------------------------------------------

class WXDLLIMPEXP_THINGS wxGenericPen
{
public:
    inline wxGenericPen() : m_width(1), m_style(wxSOLID),
                            m_cap(wxCAP_ROUND), m_join(wxJOIN_ROUND), 
                            m_dash_count(0), m_dash(NULL) {}
    inline wxGenericPen( unsigned char r, unsigned char g, unsigned char b,
                         int width=1, int style=wxSOLID )
                       : m_colour(r, g, b), m_width(width), m_style(style),
                         m_cap(wxCAP_ROUND), m_join(wxJOIN_ROUND), 
                         m_dash_count(0), m_dash(NULL) {}

    inline wxGenericPen( const wxColour &colour, int width, int style )
                       : m_colour(colour), m_width(width), m_style(style),
                         m_cap(wxCAP_ROUND), m_join(wxJOIN_ROUND), 
                         m_dash_count(0), m_dash(NULL) {}

    inline wxGenericPen( const wxGenericColour &colour, int width, int style )
                       : m_colour(colour), m_width(width), m_style(style),
                         m_cap(wxCAP_ROUND), m_join(wxJOIN_ROUND), 
                         m_dash_count(0), m_dash(NULL) {}

    inline wxGenericPen( const wxPen &pen )
                       : m_dash_count(0), m_dash(NULL)
                       { SetPen( pen ); }

    inline wxGenericPen( const wxGenericPen &pen ) 
                       : m_dash_count(0), m_dash(NULL)
                       { Copy(pen); }
    
    virtual ~wxGenericPen();

    inline bool Ok() const { return true; } // compatibility w/ wxPen
    // Make a full copy of the pen
    void Copy(const wxGenericPen &pen);
    bool IsEqual(const wxGenericPen& pen) const;

    wxPen GetPen() const;
    void SetPen(const wxPen &pen);

    // wxPen methods
    inline void SetColour( const wxColour &colour ) { m_colour.Set(colour); }
    inline void SetColour( int red, int green, int blue, int alpha=255 )
        { m_colour.Set((unsigned char)red, (unsigned char)green, (unsigned char)blue, (unsigned char)alpha); }
    inline void SetColour( const wxGenericColour &colour ) { m_colour = colour; }
    inline void SetCap( int capStyle )   { m_cap = capStyle; }
    inline void SetJoin( int joinStyle ) { m_join = joinStyle; }
    inline void SetStyle( int style )    { m_style = style; }
    inline void SetWidth( int width )    { m_width = width; }
    void SetDashes( int number_of_dashes, const wxDash *dash );

    inline wxColour GetColour() const          { return m_colour.GetColour(); }
    inline const wxGenericColour& GetGenericColour() const { return m_colour; }
    inline wxGenericColour& GetGenericColour() { return m_colour; }
    inline int GetCap() const                  { return m_cap; }
    inline int GetJoin() const                 { return m_join; }
    inline int GetStyle() const                { return m_style; }
    inline int GetWidth() const                { return m_width; }
    inline int GetDashes(wxDash **ptr) const   { *ptr = (wxDash*)m_dash; return m_dash_count; }
    inline int GetDashCount() const            { return m_dash_count; }
    inline wxDash* GetDash() const             { return m_dash; }

    // Read pen to and from string
    //wxString WriteString() const;
    //bool ReadString(const wxString& str);
    
    // operators
    inline wxGenericPen& operator = ( const wxPen& pen )        { this->SetPen(pen); return *this; }
    inline wxGenericPen& operator = ( const wxGenericPen& pen ) { this->Copy(pen); return *this; }
    inline bool operator == ( const wxGenericPen& pen ) const { return IsEqual(pen); }
    inline bool operator != ( const wxGenericPen& pen ) const { return !(*this == pen); }

    // members
    wxGenericColour m_colour;
    int m_width;
    int m_style;
    int m_cap;
    int m_join;
    
private:    
    int m_dash_count;
    wxDash *m_dash;
};

#endif // _WX_GENERGDI_H_
