///////////////////////////////////////////////////////////////////////////
// Name:        sheetctl.h
// Purpose:     wxSheet controls
// Author:      Paul Gammans, Roger Gammans
// Modified by: John Labenski
// Created:     11/04/2001
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, The Computer Surgery (paul@compsurg.co.uk)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_SHEETCTRL_H__
#define __WX_SHEETCTRL_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "sheetctl.h"
#endif

#include "wx24defs.h"  // wx24 backwards compatibility

#if wxUSE_GRID

#include "wx/string.h"
#include "wx/datetime.h"
#include "wx/bitmap.h"
#if wxMINOR_VERSION > 4
    #include "wx/arrstr.h"
#endif

class wxSheet;
class wxSheetCellAttr;
class wxSheetCoords;
class wxSheetCellEditorRefData;
class wxSheetCellRendererRefData;

class wxCheckBox;
class wxComboBox;
class wxTextCtrl;
class wxSpinCtrl;

// classes must define a "bool Copy(const classname& other)" function to copy all
// the data in the class so that Clone can create a "new" copied instance.
#define DECLARE_SHEETOBJREFDATA_COPY_CLASS(classname, basename)         \
    DECLARE_DYNAMIC_CLASS(classname)                                    \
    public:                                                             \
    virtual basename * Clone() const { classname * aclass = new classname(); aclass->Copy( * ((classname * )this)); return (basename * )aclass; } 

// ----------------------------------------------------------------------------
// wxSheetCellEditor
// ----------------------------------------------------------------------------
// This class is responsible for providing and manipulating the in-place edit 
// controls for the grid.  The refdata instances of wxSheetCellEditor 
// (actually, instances of derived classes since it is an ABC) can be
// associated with the cell attributes for individual cells, rows, columns, or
// even for the entire grid (the default).
// ----------------------------------------------------------------------------

class wxSheetCellEditor : public wxObject
{
public:
    wxSheetCellEditor( wxSheetCellEditorRefData *editor = NULL );

    void Destroy() { UnRef(); }
    bool Ok() const { return m_refData != NULL; }
    
    // Is this editor ready to use (typically means the control is created)
    bool IsCreated() const;
    // Is the control currently shown
    bool IsShown() const;
    
    // Get a pointer to the control, null if !HasControl
    wxWindow* GetControl() const;
    // Destroy the old control (if any) and set the new one
    void SetControl(wxWindow* control);
    // Is the control created (maybe some new editor types won't use a
    //   the control so this explicitly checks that)
    bool HasControl() const;
    // Destroy the control
    void DestroyControl();

    // Creates the actual edit control using the parent, id, and 
    //  pushing the evtHandler onto it if !NULL.
    void CreateEditor(wxWindow* parent, wxWindowID id, wxEvtHandler* evtHandler);

    // Size and position the edit control, uses attr.GetAlignment if Ok.
    void SetSize(const wxRect& rect, const wxSheetCellAttr& attr);
    // Show or hide the edit control, use the specified attributes to set
    // colours/fonts for it if attr is Ok.
    void Show(bool show, const wxSheetCellAttr &attr);
    // Draws the part of the cell not occupied by the control: the base class
    // version just fills it with background colour from the attribute
    void PaintBackground(wxSheet& grid, const wxSheetCellAttr& attr, 
                         wxDC& dc, const wxRect& rect, 
                         const wxSheetCoords& coords, bool isSelected);
    // Fetch the value from the table and prepare the edit control
    // to begin editing.  Set the focus to the edit control.
    void BeginEdit(const wxSheetCoords& coords, wxSheet* grid);
    // Complete the editing of the current cell. Returns true if the value has
    // changed.  If necessary, the control may be destroyed.
    bool EndEdit(const wxSheetCoords& coords, wxSheet* grid);
    // Reset the value in the control back to its starting value
    void Reset();

    // return TRUE to allow the given key to start editing: the base class
    // version only checks that the event has no modifiers. The derived
    // classes are supposed to do "if ( base::IsAcceptedKey() && ... )" in
    // their IsAcceptedKey() implementation, although, of course, it is not a
    // mandatory requirment.
    //
    // NB: if the key is F2 (special), editing will always start and this
    //     method will not be called at all (but StartingKey() will)
    bool IsAcceptedKey(wxKeyEvent& event);
    // If the editor is enabled by pressing keys on the grid, this will be
    // called to let the editor do something about that first key if desired
    void StartingKey(wxKeyEvent& event);
    // if the editor is enabled by clicking on the cell, this method will be
    // called
    void StartingClick();
    // Some types of controls on some platforms may need some help
    // with the Return key.
    void HandleReturn(wxKeyEvent& event);

    // the wxSheetCellEditorEvtHandler passes the key event here first
    // return FALSE to stop default EvtHandler processing
    bool OnKeyDown(wxKeyEvent& event);
    // the wxSheetCellEditorEvtHandler passes the key event here first
    // return FALSE to stop default EvtHandler processing
    bool OnChar(wxKeyEvent& event);

    // interpret renderer parameters: arbitrary string whose interpretation is
    // left to the derived classes
    void SetParameters(const wxString& params);
    
    // Get the current value of the control
    wxString GetValue() const;
    // Get the value that the editor started with
    wxString GetInitValue() const;

    bool Copy(const wxSheetCellEditor& other);    
    DECLARE_SHEETOBJ_COPY_CLASS(wxSheetCellEditor)
};

extern const wxSheetCellEditor wxNullSheetCellEditor;

// ----------------------------------------------------------------------------    
// wxSheetCellEditorRefData
// ----------------------------------------------------------------------------    
class wxSheetCellEditorRefData : public wxObject, public wxObjectRefData
{
public:
    wxSheetCellEditorRefData() : m_control(NULL) {}
    virtual ~wxSheetCellEditorRefData();

    virtual bool IsCreated() const { return (GetControl() != NULL); }
    virtual bool IsShown() const { return (GetControl() != NULL) && GetControl()->IsShown(); }

    wxWindow* GetControl() const { return m_control; }
    virtual void SetControl(wxWindow* control);
    virtual bool HasControl() const { return m_control != NULL; }
    virtual void DestroyControl();

    virtual void CreateEditor(wxWindow* parent, wxWindowID id, 
                              wxEvtHandler* evtHandler);
    virtual void SetSize(const wxRect& rect, const wxSheetCellAttr &attr);
    virtual void Show(bool show, const wxSheetCellAttr &attr);
    virtual void PaintBackground(wxSheet& grid, const wxSheetCellAttr& attr, 
                                 wxDC& dc, const wxRect& rect, 
                                 const wxSheetCoords& coords, bool isSelected);
    virtual void BeginEdit(const wxSheetCoords& WXUNUSED(coords), wxSheet* WXUNUSED(grid)) {}
    virtual bool EndEdit(const wxSheetCoords& WXUNUSED(coords), wxSheet* WXUNUSED(grid)) { return FALSE; }
    virtual void Reset() {}

    virtual bool IsAcceptedKey(wxKeyEvent& event);
    virtual void StartingKey(wxKeyEvent& event) { event.Skip(); }
    virtual bool OnKeyDown(wxKeyEvent& WXUNUSED(event)) { return TRUE; }
    virtual bool OnChar(wxKeyEvent& WXUNUSED(event)) { return TRUE; }
    virtual void StartingClick() {}
    virtual void HandleReturn(wxKeyEvent& event) { event.Skip(); }

    virtual void SetParameters(const wxString& WXUNUSED(params)) {}
    virtual wxString GetValue() const { return wxEmptyString; }
    virtual wxString GetInitValue() const { return wxEmptyString; }

    bool Copy(const wxSheetCellEditorRefData& WXUNUSED(other)) { return TRUE; }
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellEditorRefData, 
                                       wxSheetCellEditorRefData)

    wxWindow* m_control;    // the control we show on screen

    // FIXME - Why were people bothering with restoring the old colours? they're reset when shown!
    // if we change the colours/font of the control from the default ones, we
    // must restore the default later and we save them here between calls to
    // Show(TRUE) and Show(FALSE)
    wxColour m_colFgOld,
             m_colBgOld;
    wxFont   m_fontOld;
};

//-----------------------------------------------------------------------------
// wxSheetCellTextEditorRefData - the editor for string/text data
//-----------------------------------------------------------------------------
#if wxUSE_TEXTCTRL

class wxSheetCellTextEditorRefData : public wxSheetCellEditorRefData
{
public:
    wxSheetCellTextEditorRefData() : m_maxChars(0) {}

    virtual void CreateEditor(wxWindow* parent, wxWindowID id,
                              wxEvtHandler* evtHandler);
    virtual void SetSize(const wxRect& rect, const wxSheetCellAttr &attr);

    virtual void PaintBackground(wxSheet& , const wxSheetCellAttr& , 
                                 wxDC& , const wxRect& , 
                                 const wxSheetCoords& , bool ) {}

    virtual bool IsAcceptedKey(wxKeyEvent& event);
    virtual void BeginEdit(const wxSheetCoords& coords, wxSheet* grid);
    virtual bool EndEdit(const wxSheetCoords& coords, wxSheet* grid);

    virtual void Reset();
    virtual void StartingKey(wxKeyEvent& event);
    virtual void HandleReturn(wxKeyEvent& event);
    virtual bool OnChar(wxKeyEvent& event);

    // parameters string format is "max_width"
    virtual void SetParameters(const wxString& params);

    virtual wxString GetValue() const;
    virtual wxString GetInitValue() const { return m_startValue; }

    bool Copy(const wxSheetCellTextEditorRefData& other) 
        { return wxSheetCellEditorRefData::Copy(other); }
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellTextEditorRefData, 
                                       wxSheetCellEditorRefData)

    wxTextCtrl *Text() const { return (wxTextCtrl *)GetControl(); }

    // parts of our virtual functions reused by the derived classes
    void DoBeginEdit(const wxString& startValue);
    void DoReset(const wxString& startValue);
    
    size_t   m_maxChars;        // max number of chars allowed
    wxString m_startValue;    
    wxString m_longestValue;
};

// ----------------------------------------------------------------------------
// wxSheetCellAutoWrapStringEditorRefData
// ----------------------------------------------------------------------------

class wxSheetCellAutoWrapStringEditorRefData : public wxSheetCellTextEditorRefData
{
public:
    wxSheetCellAutoWrapStringEditorRefData() : wxSheetCellTextEditorRefData() { }
    
    virtual void CreateEditor(wxWindow* parent, wxWindowID id,
                              wxEvtHandler* evtHandler);

    bool Copy(const wxSheetCellAutoWrapStringEditorRefData& other) 
        { return wxSheetCellTextEditorRefData::Copy(other); }
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellAutoWrapStringEditorRefData, 
                                       wxSheetCellEditorRefData)                
};

#endif // wxUSE_TEXTCTRL

//-----------------------------------------------------------------------------
// wxSheetCellNumberEditorRefData : the editor for numeric (long) data
//-----------------------------------------------------------------------------
#if defined(wxUSE_TEXTCTRL) && defined(wxUSE_SPINCTRL)

class wxSheetCellNumberEditorRefData : public wxSheetCellTextEditorRefData
{
public:
    // specify the range - if min == max == -1, no range checking is done
    wxSheetCellNumberEditorRefData(int min = -1, int max = -1)
        : m_min(min), m_max(max), m_valueOld(0) {}

    virtual void CreateEditor(wxWindow* parent, wxWindowID id, 
                              wxEvtHandler* evtHandler);

    virtual bool IsAcceptedKey(wxKeyEvent& event);
    virtual void BeginEdit(const wxSheetCoords& coords, wxSheet* grid);
    virtual bool EndEdit(const wxSheetCoords& coords, wxSheet* grid);

    virtual void Reset();
    virtual void StartingKey(wxKeyEvent& event);

    // parameters string format is "min,max"
    virtual void SetParameters(const wxString& params);

    virtual wxString GetValue() const;
    int GetValueInt() const;
    virtual wxString GetInitValue() const;

    bool Copy(const wxSheetCellNumberEditorRefData& other);
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellNumberEditorRefData, 
                                       wxSheetCellEditorRefData)
    
    wxSpinCtrl *Spin() const { return (wxSpinCtrl *)GetControl(); }

    // if HasRange(), we use wxSpinCtrl - otherwise wxTextCtrl
    bool HasRange() const { return m_min != m_max; }
    
    int m_min,
        m_max;

    long m_valueOld;    
};
#endif //defined(wxUSE_TEXTCTRL) && defined(wxUSE_SPINCTRL)

//-----------------------------------------------------------------------------
// wxSheetCellFloatEditorRefData: the editor for floating point numbers (double) data
//-----------------------------------------------------------------------------
#if wxUSE_TEXTCTRL

class wxSheetCellFloatEditorRefData : public wxSheetCellTextEditorRefData
{
public:
    wxSheetCellFloatEditorRefData(int width = -1, int precision = -1)
        : m_width(width), m_precision(precision), m_valueOld(0) {}

    virtual void CreateEditor(wxWindow* parent, wxWindowID id,
                              wxEvtHandler* evtHandler);

    virtual bool IsAcceptedKey(wxKeyEvent& event);
    virtual void BeginEdit(const wxSheetCoords& coords, wxSheet* grid);
    virtual bool EndEdit(const wxSheetCoords& coords, wxSheet* grid);

    virtual void Reset();
    virtual void StartingKey(wxKeyEvent& event);

    // parameters string format is "width,precision"
    virtual void SetParameters(const wxString& params);

    // string representation of m_valueOld, initial value
    virtual wxString GetInitValue() const;
    
    bool Copy(const wxSheetCellFloatEditorRefData& other);
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellFloatEditorRefData, 
                                       wxSheetCellEditorRefData)    
    
    int m_width,
        m_precision;
    double m_valueOld;    
};

#endif // wxUSE_TEXTCTRL

//-----------------------------------------------------------------------------
// wxSheetCellBoolEditorRefData: the editor for boolean data
//-----------------------------------------------------------------------------
#if wxUSE_CHECKBOX

class wxSheetCellBoolEditorRefData : public wxSheetCellEditorRefData
{
public:
    wxSheetCellBoolEditorRefData() : m_startValue(FALSE) {}

    virtual void CreateEditor(wxWindow* parent, wxWindowID id,
                              wxEvtHandler* evtHandler);

    virtual void SetSize(const wxRect& rect, const wxSheetCellAttr &attr);
    virtual void Show(bool show, const wxSheetCellAttr &attr);

    virtual bool IsAcceptedKey(wxKeyEvent& event);
    virtual void BeginEdit(const wxSheetCoords& coords, wxSheet* grid);
    virtual bool EndEdit(const wxSheetCoords& coords, wxSheet* grid);

    virtual void Reset();
    virtual void StartingClick();

    virtual wxString GetValue() const;

    bool Copy(const wxSheetCellBoolEditorRefData& other) 
        { return wxSheetCellEditorRefData::Copy(other); }    
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellBoolEditorRefData, 
                                       wxSheetCellEditorRefData)    

    wxCheckBox *CBox() const { return (wxCheckBox *)GetControl(); }
    
    bool m_startValue;
};

#endif // wxUSE_CHECKBOX

//-----------------------------------------------------------------------------
// wxSheetCellChoiceEditorRefData: choose from a list of strings
//-----------------------------------------------------------------------------
#if wxUSE_COMBOBOX

class wxSheetCellChoiceEditorRefData : public wxSheetCellEditorRefData
{
public:
    // if !allowOthers, user can't type a string not in choices array
    wxSheetCellChoiceEditorRefData(size_t count = 0,
                                   const wxString choices[] = NULL,
                                   bool allowOthers = FALSE);
    wxSheetCellChoiceEditorRefData(const wxArrayString& choices,
                                   bool allowOthers = FALSE);

    virtual void CreateEditor(wxWindow* parent, wxWindowID id,
                              wxEvtHandler* evtHandler);

    virtual void PaintBackground(wxSheet& grid, const wxSheetCellAttr& attr, 
                                 wxDC& dc, const wxRect& rect, 
                                 const wxSheetCoords& coords, bool isSelected);

    virtual void BeginEdit(const wxSheetCoords& coords, wxSheet* grid);
    virtual bool EndEdit(const wxSheetCoords& coords, wxSheet* grid);

    virtual void Reset();

    // parameters string format is "item1[,item2[...,itemN]]"
    virtual void SetParameters(const wxString& params);

    virtual wxString GetValue() const;

    bool Copy(const wxSheetCellChoiceEditorRefData& other);    
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellChoiceEditorRefData, 
                                       wxSheetCellEditorRefData)    

    wxComboBox *Combo() const { return (wxComboBox *)GetControl(); }
    wxString        m_startValue;
    wxArrayString   m_choices;
    bool            m_allowOthers;
};

// ----------------------------------------------------------------------------
// wxSheetCellEnumEditorRefData
// ----------------------------------------------------------------------------

class wxSheetCellEnumEditorRefData : public wxSheetCellChoiceEditorRefData
{
public:
    wxSheetCellEnumEditorRefData( const wxString& choices = wxEmptyString );

    virtual bool EndEdit(const wxSheetCoords& coords, wxSheet* grid);
    virtual void BeginEdit(const wxSheetCoords& coords, wxSheet* grid);
    
    bool Copy(const wxSheetCellEnumEditorRefData& other) 
        { return wxSheetCellChoiceEditorRefData::Copy(other); }
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellEnumEditorRefData, 
                                       wxSheetCellEditorRefData)            

    long int m_startint;    
};

#endif // wxUSE_COMBOBOX

// ----------------------------------------------------------------------------
// wxSheetCellEditorEvtHandler
// ----------------------------------------------------------------------------
class wxSheetCellEditorEvtHandler : public wxEvtHandler
{
public:
    wxSheetCellEditorEvtHandler() : m_sheet(NULL), m_editor(NULL) { }
    wxSheetCellEditorEvtHandler(wxSheet* sheet, const wxSheetCellEditor &editor);

    void OnKeyDown(wxKeyEvent& event);
    void OnChar(wxKeyEvent& event);
    
    wxSheet                  *m_sheet;
    // This is a pointer, not a refed editor since editor's destructor 
    //  pops event handler which deletes this
    wxSheetCellEditorRefData *m_editor;

private:
    DECLARE_DYNAMIC_CLASS(wxSheetCellEditorEvtHandler)
    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(wxSheetCellEditorEvtHandler)
};

// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************

// ----------------------------------------------------------------------------
// wxSheetCellRenderer: 
// ----------------------------------------------------------------------------
// This class is responsible for actually drawing the cell in the grid.
// You may pass it to a wxSheetCellAttr to change the format of one given cell 
// or to wxSheet::SetDefaultRenderer() to change the view of all cells. 
// 
// Rendering is done by the wxObject::m_refData which must be derived from
// wxSheetCellRendererRefData.
// ----------------------------------------------------------------------------
class wxSheetCellRenderer : public wxObject
{
public:
    wxSheetCellRenderer(wxSheetCellRendererRefData *renderer = NULL);

    void Destroy() { UnRef(); }

    bool Ok() const { return m_refData != NULL; }
    
    // draw the given cell on the provided DC inside the given rectangle
    // using the style specified by the attribute and the default or selected
    // state corresponding to the isSelected value.
    void Draw(wxSheet& grid, const wxSheetCellAttr& attr, 
              wxDC& dc, const wxRect& rect, 
              const wxSheetCoords& coords, bool isSelected);

    // get the preferred size of the cell for its contents
    wxSize GetBestSize(wxSheet& grid, const wxSheetCellAttr& attr,
                       wxDC& dc, const wxSheetCoords& coords);

    // interpret renderer parameters: arbitrary string whose interpretation is
    // left to the derived classes
    void SetParameters(const wxString& params);
  
    bool Copy(const wxSheetCellRenderer& other);    
    DECLARE_SHEETOBJ_COPY_CLASS(wxSheetCellRenderer)    
};

extern const wxSheetCellRenderer wxNullSheetCellRenderer;

// ----------------------------------------------------------------------------
// wxSheetCellRendererRefData - base class for rendering a cell
// ----------------------------------------------------------------------------
class wxSheetCellRendererRefData : public wxObjectRefData, public wxObject
{
public:
    wxSheetCellRendererRefData() {}

    // this pure virtual function has a default implementation which will
    // prepare the DC using the given attribute: it will draw the rectangle
    // with the bg colour from attr and set the text colour and font
    virtual void Draw(wxSheet& grid, const wxSheetCellAttr& attr, 
                      wxDC& dc, const wxRect& rect, 
                      const wxSheetCoords& coords, bool isSelected);

    // get the preferred size of the cell for its contents
    virtual wxSize GetBestSize(wxSheet& grid, const wxSheetCellAttr& attr,
                               wxDC& dc, const wxSheetCoords& coords);

    // interpret renderer parameters: arbitrary string whose interpretatin is
    // left to the derived classes
    virtual void SetParameters(const wxString& WXUNUSED(params)) {}
  
    bool Copy(const wxSheetCellRendererRefData& WXUNUSED(other)) { return TRUE; }    
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellRendererRefData, 
                                       wxSheetCellRendererRefData)    
};

// ----------------------------------------------------------------------------
// wxSheetCellStringRendererRefData
// ----------------------------------------------------------------------------

// the default renderer for the cells containing string data
class wxSheetCellStringRendererRefData : public wxSheetCellRendererRefData
{
public:
    wxSheetCellStringRendererRefData();
    
    // draw the string
    virtual void Draw(wxSheet& grid, const wxSheetCellAttr& attr,
                      wxDC& dc, const wxRect& rect, 
                      const wxSheetCoords& coords, bool isSelected);

    // return the string extent
    virtual wxSize GetBestSize(wxSheet& grid, const wxSheetCellAttr& attr,
                               wxDC& dc, const wxSheetCoords& coords);

    void DoDraw(wxSheet& grid, const wxSheetCellAttr& attr,
                wxDC& dc, const wxRect& rect, 
                const wxSheetCoords& coords, bool isSelected);

    // set the text colours before drawing
    void SetTextColoursAndFont(wxSheet& grid, const wxSheetCellAttr& attr,
                               wxDC& dc, bool isSelected);

    // calc the string extent for given string/font
    wxSize DoGetBestSize(wxSheet& grid, const wxSheetCellAttr& attr, 
                         wxDC& dc, const wxString& text);

    bool Copy(const wxSheetCellStringRendererRefData& other) 
        { return wxSheetCellRendererRefData::Copy(other); }
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellStringRendererRefData, 
                                       wxSheetCellRendererRefData)
};

// ----------------------------------------------------------------------------
// wxSheetCellNumberRendererRefData
// ----------------------------------------------------------------------------

// the default renderer for the cells containing numeric (long) data
class wxSheetCellNumberRendererRefData : public wxSheetCellStringRendererRefData
{
public:
    wxSheetCellNumberRendererRefData() {}
    
    virtual void Draw(wxSheet& grid, const wxSheetCellAttr& attr, 
                      wxDC& dc, const wxRect& rect, 
                      const wxSheetCoords& coords, bool isSelected);

    virtual wxSize GetBestSize(wxSheet& grid, const wxSheetCellAttr& attr,
                               wxDC& dc, const wxSheetCoords& coords);
    
    wxString GetString(wxSheet& grid, const wxSheetCoords& coords);
        
    bool Copy(const wxSheetCellNumberRendererRefData& other) 
        { return wxSheetCellStringRendererRefData::Copy(other); }
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellNumberRendererRefData, 
                                       wxSheetCellRendererRefData)
};

// ----------------------------------------------------------------------------
// wxSheetCellFloatRendererRefData
// ----------------------------------------------------------------------------
class wxSheetCellFloatRendererRefData : public wxSheetCellStringRendererRefData
{
public:
    wxSheetCellFloatRendererRefData(int width = -1, int precision = -1)
        : m_width(width), m_precision(precision) { }

    // get/change formatting parameters
    int GetWidth() const             { return m_width; }
    void SetWidth(int width)         { m_width = width; m_format.clear(); }
    int GetPrecision() const         { return m_precision; }
    void SetPrecision(int precision) { m_precision = precision; m_format.clear(); }

    virtual void Draw(wxSheet& grid, const wxSheetCellAttr& attr, 
                      wxDC& dc, const wxRect& rect, 
                      const wxSheetCoords& coords, bool isSelected);

    virtual wxSize GetBestSize(wxSheet& grid, const wxSheetCellAttr& attr,
                               wxDC& dc, const wxSheetCoords& coords);

    // parameters string format is "width[,precision]"
    virtual void SetParameters(const wxString& params);

    wxString GetString(wxSheet& grid, const wxSheetCoords& coords);
    
    bool Copy(const wxSheetCellFloatRendererRefData& other);
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellFloatRendererRefData, 
                                       wxSheetCellRendererRefData)
    
protected:
    int m_width,
        m_precision;

    wxString m_format;
};

// ----------------------------------------------------------------------------
// wxSheetCellBitmapRendererRefData
// ----------------------------------------------------------------------------

class wxSheetCellBitmapRendererRefData : public wxSheetCellStringRendererRefData
{
public:
    wxSheetCellBitmapRendererRefData() {}
    wxSheetCellBitmapRendererRefData(const wxBitmap& bitmap) : m_bitmap(bitmap) {}
    
    // draw a the bitmap
    virtual void Draw(wxSheet& grid, const wxSheetCellAttr& attr, 
                      wxDC& dc, const wxRect& rect, 
                      const wxSheetCoords& coords, bool isSelected);

    // return the bitmap size
    virtual wxSize GetBestSize(wxSheet& grid, const wxSheetCellAttr& attr,
                               wxDC& dc, const wxSheetCoords& coords);

    // Get/Set the bitmap to draw
    wxBitmap GetBitmap() const { return m_bitmap; }
    void SetBitmap(const wxBitmap& bitmap) { m_bitmap = bitmap; }
        
    bool Copy(const wxSheetCellBitmapRendererRefData& other) 
        { SetBitmap(other.GetBitmap()); return wxSheetCellStringRendererRefData::Copy(other); }
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellBitmapRendererRefData, 
                                       wxSheetCellRendererRefData)
        
    wxBitmap m_bitmap;
};

// ----------------------------------------------------------------------------
// wxSheetCellBoolRendererRefData
// ----------------------------------------------------------------------------

class wxSheetCellBoolRendererRefData : public wxSheetCellRendererRefData
{
public:
    wxSheetCellBoolRendererRefData() {}
    
    // draw a check mark or nothing
    virtual void Draw(wxSheet& grid, const wxSheetCellAttr& attr, 
                      wxDC& dc, const wxRect& rect, 
                      const wxSheetCoords& coords, bool isSelected);

    // return the checkmark size
    virtual wxSize GetBestSize(wxSheet& grid, const wxSheetCellAttr& attr,
                               wxDC& dc, const wxSheetCoords& coords);

    bool Copy(const wxSheetCellBoolRendererRefData& other) 
        { return wxSheetCellRendererRefData::Copy(other); }
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellBoolRendererRefData, 
                                       wxSheetCellRendererRefData)
    
protected:
    static wxSize ms_sizeCheckMark;        
};

// ----------------------------------------------------------------------------
// wxSheetCellDateTimeRendererRefData
// ----------------------------------------------------------------------------
#if wxUSE_DATETIME

// the default renderer for the cells containing Time and dates..
class wxSheetCellDateTimeRendererRefData : public wxSheetCellStringRendererRefData
{
public:
    wxSheetCellDateTimeRendererRefData(wxString outFormat = _T("%c"),
                                       wxString inFormat  = _T("%c"))
        : m_outFormat(outFormat), m_inFormat(inFormat), 
          m_dateTime(wxDefaultDateTime), m_tz(wxDateTime::Local) {}

    virtual void Draw(wxSheet& grid, wxSheetCellAttr& attr, 
                      wxDC& dc, const wxRect& rect, 
                      const wxSheetCoords& coords, bool isSelected);

    virtual wxSize GetBestSize(wxSheet& grid, wxSheetCellAttr& attr, wxDC& dc,
                               const wxSheetCoords& coords);

    // parameters string format is "width[,precision]"
    virtual void SetParameters(const wxString& params);

    wxString GetInFormat() const  { return m_inFormat; }
    wxString GetOutFormat() const { return m_outFormat; }
    void SetInFormat(const wxString& inFormat)   { m_inFormat = inFormat; }
    void SetOutFormat(const wxString& outFormat) { m_outFormat = outFormat; }
    
    wxString GetString(wxSheet& grid, const wxSheetCoords& coords);
    
    bool Copy(const wxSheetCellDateTimeRendererRefData& other);
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellDateTimeRendererRefData, 
                                       wxSheetCellRendererRefData) 
    
protected:    
    wxString m_outFormat;
    wxString m_inFormat;
    wxDateTime m_dateTime;
    wxDateTime::TimeZone m_tz;    
};

#endif // wxUSE_DATETIME

// ----------------------------------------------------------------------------
// wxSheetCellEnumRendererRefData - renderers Time and dates..
// ----------------------------------------------------------------------------
class wxSheetCellEnumRendererRefData : public wxSheetCellStringRendererRefData
{
public:
    wxSheetCellEnumRendererRefData( const wxString& choices = wxEmptyString );

    virtual void Draw(wxSheet& grid, wxSheetCellAttr& attr, 
                      wxDC& dc, const wxRect& rect, 
                      const wxSheetCoords& coords, bool isSelected);

    virtual wxSize GetBestSize(wxSheet& grid, wxSheetCellAttr& attr, wxDC& dc,
                               const wxSheetCoords& coords);

    // parameters string format is "item1[,item2[...,itemN]]"
    virtual void SetParameters(const wxString& params);

    wxArrayString& GetChoices() { return m_choices; }
    wxString GetString(wxSheet& grid, const wxSheetCoords& coords);

    bool Copy(const wxSheetCellEnumRendererRefData& other);
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellEnumRendererRefData, 
                                       wxSheetCellRendererRefData)        
protected:
    wxArrayString m_choices;    
};

// ----------------------------------------------------------------------------
// wxSheetCellAutoWrapStringRendererRefData
// ----------------------------------------------------------------------------

class wxSheetCellAutoWrapStringRendererRefData : public wxSheetCellStringRendererRefData
{
public:
    wxSheetCellAutoWrapStringRendererRefData() : wxSheetCellStringRendererRefData() { }

    virtual void Draw(wxSheet& grid, wxSheetCellAttr& attr, 
                      wxDC& dc, const wxRect& rect, 
                      const wxSheetCoords& coords, bool isSelected);

    virtual wxSize GetBestSize(wxSheet& grid, wxSheetCellAttr& attr,
                               wxDC& dc, const wxSheetCoords& coords);

    wxArrayString GetTextLines( wxSheet& grid, wxDC& dc, wxSheetCellAttr& attr,
                                const wxRect& rect, const wxSheetCoords& coords);
    
    bool Copy(const wxSheetCellAutoWrapStringRendererRefData& other) 
        { return wxSheetCellStringRendererRefData::Copy(other); }
    DECLARE_SHEETOBJREFDATA_COPY_CLASS(wxSheetCellAutoWrapStringRendererRefData,
                                       wxSheetCellRendererRefData)                
};

// ----------------------------------------------------------------------------
// wxSheetCellRolColLabelRendererRefData
// ----------------------------------------------------------------------------

// the default renderer for the cells containing string data
class wxSheetCellRolColLabelRendererRefData : public wxSheetCellStringRendererRefData
{
public:
    wxSheetCellRolColLabelRendererRefData() {}
    
    virtual void Draw( wxSheet& grid, const wxSheetCellAttr& attr,
                       wxDC& dc, const wxRect& rect, 
                       const wxSheetCoords& coords, bool isSelected );

    bool Copy(const wxSheetCellRolColLabelRendererRefData& other) 
        { return wxSheetCellStringRendererRefData::Copy(other); }
    DECLARE_SHEETOBJREFDATA_COPY_CLASS( wxSheetCellRolColLabelRendererRefData, 
                                        wxSheetCellRendererRefData)
};

#endif  // #if wxUSE_GRID

#endif //__WX_SHEETCTRL_H__
