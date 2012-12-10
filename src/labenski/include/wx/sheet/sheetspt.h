///////////////////////////////////////////////////////////////////////////////
// Name:        sheetspt.h
// Purpose:     wxSheetSplitter and related classes
// Author:      John Labenski
// Modified by: 
// Created:     4/1/2004
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WX_SHEETSPT_H__
#define __WX_SHEETSPT_H__

#include "wx/sheet/sheet.h"
#include "wx/splitter.h"

// ----------------------------------------------------------------------------
// wxSheetSplitter - a 4 way splitter class for the wxSheet
//
// To use the wxSheetSplitter, create an instance and a single child wxSheet.
// Attach the wxSheet to the splitter using Initialize() and the splitter 
// will create and ref additional sheets when the user splits the sheet. When
// the user unsplits the added sheets are destroyed. The TopLeft sheet
// is always the sheet that you Initialized it with, even if that is the sheet
// that is hidden by the user. The scrollbars are set to be shown as necessary 
// and splitting is not allowed when the scrollbars are not shown. 
//
// There are a few ways you can provide the new wxSheets that are required 
// when splitting. You'll need to do this if your sheet is derived and 
// the logic has been changed otherwise the splitter just uses a "new wxSheet."
// 1) Subclass wxSheetSplitter and override the virtual function
//    wxSheet* wxSheetSplitter::CreateSheet(wxWindowID id) to return your own. 
// 2) Intercept the wxEVT_SHEET_SPLIT_CREATE_SHEET sent from the wxSheetSplitter
//    and replace the event.Get/SetEventObject with a *new* wxSheet with the 
//    wxSheetSplitter (the original event.GetEventObject) as the parent. 
//    Note that event.GetExtraLong is the preferred id, probably wxID_ANY.
// 3) In your subclassed wxSheet override the virtual wxSheet* wxSheet::Clone
//    function to return a new instance of your wxSheet.
// ----------------------------------------------------------------------------
enum wxSheetSplitMode_Type
{
    wxSHEET_SPLIT_NONE = 0,
    wxSHEET_SPLIT_VERTICAL,
    wxSHEET_SPLIT_HORIZONTAL
};

// These window styles are used from wxSplitterWindow
//#define wxSP_NOBORDER         0x0000
//#define wxSP_NOSASH           0x0010
//#define wxSP_PERMIT_UNSPLIT   0x0040
//#define wxSP_LIVE_UPDATE      0x0080
//#define wxSP_3DSASH           0x0100
//#define wxSP_3DBORDER         0x0200
//#define wxSP_NO_XP_THEME      0x0400
//#define wxSP_BORDER           wxSP_3DBORDER
//#define wxSP_3D               (wxSP_3DBORDER | wxSP_3DSASH)

class WXDLLIMPEXP_SHEET wxSheetSplitter : public wxWindow
{
public:    
    wxSheetSplitter() : wxWindow() { Init(); }
    wxSheetSplitter(wxWindow *parent, wxWindowID id,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = wxSP_3D|wxSP_3DBORDER, 
                    const wxString& name = wxT("wxSheetSplitter"))
    {
        Init();
        Create(parent, id, pos, size, style, name);
    }
    
    bool Create(wxWindow *parent, wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxSP_3D|wxSP_3DBORDER,
                const wxString& name = wxT("wxSheetSplitter"));
    
    virtual ~wxSheetSplitter();
    virtual bool Destroy();

    // Initialize the splitter with one window, the splitter must be sheet's parent
    void Initialize(wxSheet* sheet);
    
    // Will the splitting buttons be shown for vert/horiz splitting
    //   you can still call SplitVertically/Horizontally however
    bool GetEnableSplitVertically()   const { return m_enable_split_vert; }
    bool GetEnableSplitHorizontally() const { return m_enable_split_horiz; }
    void EnableSplitVertically(bool can_split)   { m_enable_split_vert  = can_split; }
    void EnableSplitHorizontally(bool can_split) { m_enable_split_horiz = can_split; }
    
    bool IsSplitHorizontally() const { return m_tlSheet && m_trSheet; }
    bool IsSplitVertically()   const { return m_tlSheet && m_blSheet; }

    // Split the windows either vertically or horizontally
    virtual void SplitVertically(int y_pos, bool sendEvt = false);
    virtual void SplitHorizontally(int x_pos, bool sendEvt = false);
    // Unsplit the windows, note that the bottom/right window is the one
    //  that's actually removed, but the origin of the top/left will be set to
    //  that if !remove_bottom/right so it'll look like the top/left was removed
    virtual void UnsplitVertically(bool remove_bottom = true, bool sendEvt = false);
    virtual void UnsplitHorizontally(bool remove_right = true, bool sendEvt = false);

    // Get the position of the sash, 0 if not split
    int GetHorizontalSplitPosition() const { return m_splitPos.x; }
    int GetVerticalSplitPosition() const { return m_splitPos.y; }
    // Split, unsplit, and set the sash position
    // If pos <= 0 unsplit and set the origin of the left/top to val of right/bottom
    // If pos >= GetClientSize().GetWidth/Height() unsplit
    // If unsplit and pos in valid range then split
    // If split then move the sash to the position
    virtual void SetVerticalSplitPosition(int y_pos, bool sendEvt = false);
    virtual void SetHorizontalSplitPosition(int x_pos, bool sendEvt = false);

    // The top left window is the window you Initialized this with
    wxSheet* GetTopLeftSheet()     const { return m_tlSheet; }
    // The top right is valid when split vertically
    wxSheet* GetTopRightSheet()    const { return m_trSheet; }
    // the bottom left sheet is valid when split horizontally
    wxSheet* GetBottomLeftSheet()  const { return m_blSheet; }
    // The bottom right sheet is valid when split vertically and horizontally
    wxSheet* GetBottomRightSheet() const { return m_brSheet; }
    
    // Get the width of the sash 
    int GetSashSize() const;
    // Get the size of the border around the window
    int GetBorderSize() const;

    // For the given sash position adjust it so that it's valid for the 
    //  window given the min size and if it can unsplit.
    int GetAdjustedVerticalSashPosition(int pos) const;
    int GetAdjustedHorizontalSashPosition(int pos) const;
    
    // implementation
    void OnViewChanged(wxSheetEvent& event);
    void OnSplit(wxSheetSplitterEvent& event);
    void OnMouse( wxMouseEvent& event );
    void OnPaint( wxPaintEvent& event );
    void DrawSash(wxDC &dc);
    void DrawSashTracker(int x, int y);
    
    // returns wxSHEET_SPLIT_NONE/VERTICAL/HORIZONTAL
    int  SashHitTest(const wxPoint& pt) const;
    // Set the cursor to use wxSHEET_SPLIT_NONE/VERTICAL/HORIZONTAL
    void SetMouseCursor(int sheet_split_mode); 
    
    // setup the configuration of the sheets (hide/show labels/scrollbars)
    void ConfigureWindows();
    // Position the windows in this window
    void LayoutWindows();
    // Send the event and returns true if it wasn't vetoed
    bool SendEvent( wxEventType type, bool vert_split );
    bool DoSendEvent( wxSheetSplitterEvent& event );
    
protected:
    // Override this to return a derived wxSheet of your own making
    //  if you don't want to use the wxEVT_SHEET_SPLIT_CREATE_SHEET event
    virtual wxSheet* CreateSheet(wxWindowID id = wxID_ANY);
    // Overriding these functions are optional, they call CreateSheet to get a 
    //  sheet to use and then properly initialize by hiding the labels and 
    //  scrollbars as necessary.
    virtual wxSheet* CreateTopRightSheet(wxWindowID id = wxID_ANY);
    virtual wxSheet* CreateBottomLeftSheet(wxWindowID id = wxID_ANY);
    virtual wxSheet* CreateBottomRightSheet(wxWindowID id = wxID_ANY);

    void OnSize( wxSizeEvent& event );

    wxSheet *m_tlSheet;
    wxSheet *m_trSheet;
    wxSheet *m_blSheet;
    wxSheet *m_brSheet;

    wxPoint m_mousePos;  // mouse pos for sash tracker
    wxPoint m_splitPos;  // current position of the splitter
    int m_splitMode;     // currently active sash wxSHEET_SPLIT_NONE/VERTICAL/HORIZONTAL
    int m_splitCursor;   // currently active cursor wxSHEET_SPLIT_NONE/VERTICAL/HORIZONTAL
    //int m_sash_width;  FIXME maybe be backwards compatible to 2.4? 
    bool m_enable_split_vert;
    bool m_enable_split_horiz;
    wxSize m_minSize;

private:
    void Init();

    WX_DECLARE_CONTROL_CONTAINER();
    DECLARE_DYNAMIC_CLASS(wxSheetSplitter)
    DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(wxSheetSplitter)
};

// ----------------------------------------------------------------------------
// wxSheetSplitterEvent
// ----------------------------------------------------------------------------

class WXDLLEXPORT wxSheetSplitterEvent : public wxNotifyEvent
{
public:
    wxSheetSplitterEvent(int id = 0, wxEventType type = wxEVT_NULL)
        : wxNotifyEvent(type, id), m_sash_pos(0), m_vert_split(false) {}

    wxSheetSplitterEvent(const wxSheetSplitterEvent& event) : wxNotifyEvent(event), 
              m_sash_pos(event.m_sash_pos), m_vert_split(event.m_vert_split) { }
    
    // Get the current sash position, see also IsVerticalSplit for which sash.
    int GetSashPosition() const { return m_sash_pos; }
    // Set the sash position, during a wxEVT_SHEET_SPLIT_CHANGING you may 
    // replace the current value with a different one to force a new position.
    void SetSashPosition(int pos) { m_sash_pos = pos; }
    // Does this event pertain to a vertical or horizontal splitting
    bool IsVerticalSplit() const { return m_vert_split; }

    // The sheet is valid only for the wxEVT_SHEET_SPLIT_BEGIN event type
    wxSheet* GetSheet() const { return wxDynamicCast(GetEventObject(), wxSheet); }
    // The sheet splitter is valid for the rest of the splitter events
    wxSheetSplitter* GetSheetSplitter() const { return wxDynamicCast(GetEventObject(), wxSheetSplitter); }
    
    // implementation
    virtual wxEvent *Clone() const { return new wxSheetSplitterEvent(*this); }
    
    int  m_sash_pos;
    bool m_vert_split;
};

BEGIN_DECLARE_EVENT_TYPES()
    // The splitter buttons in the sheet have been clicked to begin splitting
    // this event comes from the wxSheet itself when the begins to drag the 
    // mouse on the splitter boxes. 
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_SPLIT_BEGIN, 1800)
    // The splitter sash position is changing
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_SPLIT_CHANGING, 1801)
    // The splitter sash position has changed
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_SPLIT_CHANGED, 1802)
    // The splitter sash position has been double clicked on, will unsplit
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_SPLIT_DOUBLECLICKED, 1803)
    // The splitter has been unsplit
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_SPLIT_UNSPLIT, 1804)
    // The splitter is splitting and is about to create a new wxSheet.
    // You can provide your own wxSheet derived class by calling 
    // event.SetEventObject(new MySheet(event.GetSheetSplitter(), ...))
    // where the parent of the sheet you provide is the wxSheetSplitter
    // which is the original event.GetEventObject. The sender wxSheetSplitter
    // takes ownership of the new wxSheet and destroys it when unsplit.
    // Please see the function wxSheetSplitter::CreateSheet for more info.
    // You cannot veto this event since it occurs too late in the chain of splitting,
    // use veto on wxEVT_SHEET_SPLIT_BEGIN to stop it in the first place.
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_SHEET, wxEVT_SHEET_SPLIT_CREATE_SHEET, 1805)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*wxSheetSplitterEventFunction)(wxSheetSplitterEvent&);

#define EVT_SHEET_SPLIT_BEGIN(id, fn)         DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SPLIT_BEGIN,         id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetSplitterEventFunction, &fn), NULL ),
#define EVT_SHEET_SPLIT_CHANGING(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SPLIT_CHANGING,      id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetSplitterEventFunction, &fn), NULL ),
#define EVT_SHEET_SPLIT_CHANGED(id, fn)       DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SPLIT_CHANGED,       id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetSplitterEventFunction, &fn), NULL ),
#define EVT_SHEET_SPLIT_DOUBLECLICKED(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SPLIT_DOUBLECLICKED, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetSplitterEventFunction, &fn), NULL ),
#define EVT_SHEET_SPLIT_UNSPLIT(id, fn)       DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SPLIT_UNSPLIT,       id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetSplitterEventFunction, &fn), NULL ),
#define EVT_SHEET_SPLIT_CREATE_SHEET(id, fn)  DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SPLIT_CREATE_SHEET,  id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetSplitterEventFunction, &fn), NULL ),

#endif  // __WX_SHEETSPT_H__
