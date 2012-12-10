/////////////////////////////////////////////////////////////////////////////
// Name:        wxBitmapComboBox
// Purpose:     A wxComboBox type button for bitmaps and strings
// Author:      John Labenski
// Modified by:
// Created:     11/05/2002
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

/*

wxBitmapComboBox is a wxComboBox widget for bitmaps

You Append some bitmaps either individually or with an array. Since bitmaps are
refed this should be a fast process and you don't have to keep them around.
The size of the items is calculated from the max bitmap and max label so that
they will all line up nicely.

It acts just like a wxComboBox otherwise, sends a EVT_COMBOBOX when selections
are made with either the mouse on the pulldown list or by pressing the up/down
arrows.

*/

#ifndef _WX_BMPCOMBO_H_
#define _WX_BMPCOMBO_H_

#include "wx/wxthings/thingdef.h"
#include "wx/wxthings/dropdown.h"

class WXDLLEXPORT wxMenu;
class WXDLLEXPORT wxBitmap;
class WXDLLEXPORT wxKeyEvent;
class WXDLLEXPORT wxPaintEvent;
class WXDLLEXPORT wxDC;
class WXDLLIMPEXP_THINGS wxCustomButton;
class WXDLLIMPEXP_THINGS wxBitmapComboPopupChild;
class WXDLLIMPEXP_THINGS wxBitmapComboLabel;

enum wxBitmapComboBox_Style
{
    // Position of the labels relative to the bitmaps, use only one
    wxBMPCOMBO_LEFT           = wxCB_DROPDOWN,
    wxBMPCOMBO_RIGHT          = wxCB_SIMPLE
};

class WXDLLIMPEXP_THINGS wxBitmapComboBox : public DropDownBase
{
public:
    wxBitmapComboBox() : DropDownBase() { Init(); }

    // Compatible with a wxComboBox, uses strings only
    wxBitmapComboBox(wxWindow *parent, wxWindowID id,
                     const wxString& value = wxEmptyString, // used only if first choice is !null
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     int n = 0, const wxString choices[] = (const wxString *) NULL,
                     long style = wxBMPCOMBO_RIGHT,
                     const wxValidator& val = wxDefaultValidator,
                     const wxString& name = wxT("wxBitmapComboBox"))
                    :DropDownBase()
    {
        Init();
        if ((n > 0) || (!value.IsEmpty()))
        {
            if ((n > 0) && choices)
            {
                for (int i=0; i<n; i++)
                    Append(choices[i], wxNullBitmap);
            }
            else
                Append(value, wxNullBitmap);
        }
        Create(parent, id, pos, size, style, val, name);
    }

    // Native constructor (only adds first item) you probably want to
    // use the largest item first so that the size is correct from the start.
    wxBitmapComboBox(wxWindow* parent, wxWindowID id,
                     const wxString& label,
                     const wxBitmap& bitmap,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = wxBMPCOMBO_RIGHT,
                     const wxValidator& val = wxDefaultValidator,
                     const wxString& name = wxT("wxBitmapComboBox"))
                    :DropDownBase()
    {
        Init();
        Append(label, bitmap);
        Create(parent, id, pos, size, style, val, name);
    }

    virtual ~wxBitmapComboBox();

    bool Create(wxWindow* parent, wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxBMPCOMBO_RIGHT,
                const wxValidator& val = wxDefaultValidator,
                const wxString& name = wxT("wxBitmapComboBox"));

    // Style is either wxBMPCOMBO_LEFT or wxBMPCOMBO_RIGHT
    bool SetButtonStyle(long style);

    // Append/Insert an item, either label and bitmap may be empty/null
    void Append( const wxString &label, const wxBitmap &bitmap, int count = 1 );
    void Insert( int n, const wxString &label, const wxBitmap &bitmap );

    // Clear or delete a single or number of items starting from n
    void Clear();
    void Delete( int n, int count = 1 );

    int GetCount() const { return m_labels.GetCount(); }

    int  GetSelection() const { return m_selection; }
    void SetSelection( int n, bool send_event = false );
    void SetNextSelection(bool foward, bool send_event = false);

    wxString GetLabel( int n ) const;
    wxBitmap GetBitmap( int n ) const;

    void SetBitmap(int n, const wxBitmap &bitmap);
    void SetLabel(int n, const wxString &label);
    void SetItem(int n, const wxString &label, const wxBitmap &bitmap)
        { SetLabel(n, label); SetBitmap(n, bitmap); }

    virtual bool SetBackgroundColour(const wxColour &colour);

    virtual void HidePopup();

    // When adding/deleting many items freeze it and thaw when done
    void Freeze() { m_frozen = true; }
    void Thaw();

    // implementation
    wxBitmapComboLabel* GetLabelWindow() { return m_labelWin; }
    // Get the largest label, bitmap, item=(label+bitmap) size
    wxSize GetLabelSize() const  { return m_labelSize; }
    wxSize GetBitmapSize() const { return m_bitmapSize; }
    wxSize GetItemSize() const   { return m_itemSize; }
    void CalcLabelBitmapPos(int n, const wxSize &area, wxPoint &labelPos, wxPoint &bitmapPos) const;
    void CalcLayout();
    void DrawItem(wxDC &dc, int n) const;

protected:
    virtual void DoSetSize(int x, int y, int width, int height,
                           int sizeFlags = wxSIZE_AUTO);

    void OnSize( wxSizeEvent& event );
    virtual wxSize DoGetBestSize() const;

    virtual int DoGetBestDropHeight(int max_height);
    virtual bool DoShowPopup();

    wxBitmapComboLabel *m_labelWin;

    wxArrayPtrVoid m_bitmaps;   // the individual bitmaps
    wxArrayString m_labels;     // the individual labels

    wxSize m_labelSize;         // the max size of all the labels
    wxSize m_bitmapSize;        // the max size of all the bitmaps
    wxSize m_itemSize;          // the max size of all the items

    int m_selection;
    int m_win_border;           // the wxSUNKEN_BORDER size
    long m_label_style;
    bool m_frozen;

private:
    void Init();
    DECLARE_DYNAMIC_CLASS(wxBitmapComboBox)
    DECLARE_EVENT_TABLE()
};

// ==========================================================================
// wxBitmapComboLabel - the main "window" to the left of the dropdown button
// ==========================================================================
class WXDLLIMPEXP_THINGS wxBitmapComboLabel : public wxWindow
{
public:
    wxBitmapComboLabel(wxBitmapComboBox *parent = NULL, int style = wxSUNKEN_BORDER)
                    : wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style),
                      m_bmpCombo(parent) {}

protected:
    void OnPaint( wxPaintEvent &event );
    void OnChar( wxKeyEvent &event );

    wxBitmapComboBox *m_bmpCombo;
private:
    DECLARE_DYNAMIC_CLASS(wxBitmapComboLabel)
    DECLARE_EVENT_TABLE()
};

// ============================================================================
// wxBitmapComboPopupChild - the child of the popup window
// ============================================================================

class WXDLLIMPEXP_THINGS wxBitmapComboPopupChild : public wxScrolledWindow
{
public:
    wxBitmapComboPopupChild(wxWindow *parent, wxBitmapComboBox *owner);

    void DrawSelection( int n, wxDC& dc );

    void OnMouse( wxMouseEvent &event );
    void OnPaint( wxPaintEvent &event );
    void OnKeyDown( wxKeyEvent &event );

    wxBitmapComboBox *m_bmpCombo;
    int m_last_selection;

private:
    DECLARE_EVENT_TABLE()
};

#endif  // _WX_BMPCOMBO_H_
