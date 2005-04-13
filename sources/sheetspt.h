/////////////////////////////////////////////////////////////////////////////
// Name:        sheetspt.h
// Purpose:     wxSheetSplitter and related classes
// Author:      John Labenski
// Modified by: 
// Created:     4/1/2004
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_SHEETSPT_H__
#define __WX_SHEETSPT_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "sheetspt.h"
#endif

#include "wx24defs.h"   // wx2.4 backwards compatibility
#include "wx/defs.h"
#include "wx/object.h"
#include "wx/event.h"
#include "sheet.h"

// ----------------------------------------------------------------------------
// wxSheetSplitter - a 4 way splitter class for the wxSheet
//
// To use the wxSheetSplitter, create an instance and a single child wxSheet.
// Attach the wxSheet to the splitter using Initialize() and the splitter 
// will create and ref additional sheets when the user splits the sheet. When
// the user unsplits the added sheets are destroyed. The TopLeft sheet
// is always the sheet that you Initialized it with. The scrollbars are set to 
// be shown as necessary and splitting is not allowed when the scrollbars are
// not shown. 
// ----------------------------------------------------------------------------
enum wxSheetSplitMode
{
    wxSHEET_SPLIT_NONE = 0,
    wxSHEET_SPLIT_VERTICAL,
    wxSHEET_SPLIT_HORIZONTAL
};

class  wxSheetSplitter : public wxWindow
{
public:    
    wxSheetSplitter() : wxWindow() { Init(); }
    wxSheetSplitter(wxWindow *parent, wxWindowID id,
                    const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize,
                    long style = 0, 
                    const wxString& name = wxT("wxSheetSplitter"))
    {
        Init();
        Create(parent, id, pos, size, style, name);
    }
    bool Create(wxWindow *parent, wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxT("wxSheetSplitter"));
    
    virtual ~wxSheetSplitter();
    virtual bool Destroy();

    // Initialize the splitter with one window, this must be sheet's parent
    void Initialize(wxSheet* sheet);
    
    bool GetEnableSplitVertically()   const { return m_enable_split_vert; }
    bool GetEnableSplitHorizontally() const { return m_enable_split_horiz; }
    
    void EnableSplitVertically(bool can_split)   { m_enable_split_vert  = can_split; }
    void EnableSplitHorizontally(bool can_split) { m_enable_split_horiz = can_split; }
    
    bool IsSplitHorizontally() const { return m_tlSheet && m_trSheet; }
    bool IsSplitVertically()   const { return m_tlSheet && m_blSheet; }

    virtual void SplitVertically(int y_pos, bool sendEvt = FALSE);
    virtual void SplitHorizontally(int x_pos, bool sendEvt = FALSE);
    // Unsplit the windows, note that the bottom/right window is the one
    //  that's actually removed, but the origin of the top/left will be set to
    //  that if !remove_bottom/right so it'll look like the top/left was removed
    virtual void UnsplitVertically(bool remove_bottom = TRUE, bool sendEvt = FALSE);
    virtual void UnsplitHorizontally(bool remove_right = TRUE, bool sendEvt = FALSE);

    // Get the position of the sash, 0 if not split
    int GetHorizontalSplitPosition() const { return m_splitPos.x; }
    int GetVerticalSplitPosition() const { return m_splitPos.y; }
    // Split, unsplit, and set the sash position
    // If pos <= 0 unsplit and set the origin of the left/top to val of right/bottom
    // If pos >= GetClientSize().GetWidth/Height() unsplit
    // If unsplit and pos in valid range then split
    // If split then move the sash to the position
    virtual void SetVerticalSplitPosition(int y_pos, bool sendEvt = FALSE);
    virtual void SetHorizontalSplitPosition(int x_pos, bool sendEvt = FALSE);

    // The top left window is the window you Initialized this with
    wxSheet* GetTopLeftSheet()     const { return m_tlSheet; }
    // The top right is valid when split vertically
    wxSheet* GetTopRightSheet()    const { return m_trSheet; }
    // the bottom left sheet is valid when split horizontally
    wxSheet* GetBottomLeftSheet()  const { return m_blSheet; }
    // The bottom right sheet is valid when split vertically and horizontally
    wxSheet* GetBottomRightSheet() const { return m_brSheet; }
    
    // The wxColour of the sash is drawn using Get/SetForegroundColour
    
    // Get/Set the sash width, default is 4
    int  GetSashWidth() const { return m_sash_width; }
    void SetSashWidth(int sash_width) { m_sash_width = sash_width; }
    
    // implementation
    void OnViewChanged(wxSheetEvent& event);
    void OnSplit(wxSheetSplitterEvent& event);
    void OnMouse( wxMouseEvent& event );
    void OnPaint( wxPaintEvent& event );
    void DrawSashTracker(int x, int y);
    
    // returns wxSHEET_SPLIT_NONE/VERTICAL/HORIZONTAL
    int  SashHitTest(const wxPoint& pt);
    void SetMouseCursor(int sheet_split_mode); 
    
    void ConfigureWindows();
    void LayoutWindows();
    bool SendEvent( const wxEventType type, bool vert_split );
    bool DoSendEvent( wxSheetSplitterEvent& event );
    
protected:
    virtual void DoSetSize(int x, int y, int width, int height,
                           int sizeFlags = wxSIZE_AUTO);

    wxSheet *m_tlSheet;
    wxSheet *m_trSheet;
    wxSheet *m_blSheet;
    wxSheet *m_brSheet;

    wxPoint m_mousePos;
    wxPoint m_splitPos;
    int m_splitMode;
    int m_splitCursor;
    int m_sash_width;
    bool m_enable_split_vert;
    bool m_enable_split_horiz;

private:
    void Init();
    DECLARE_DYNAMIC_CLASS(wxSheetSplitter)
    DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------------
// wxSheetSplitterEvent
// ----------------------------------------------------------------------------

class wxSheetSplitterEvent : public wxNotifyEvent
{
public:
    wxSheetSplitterEvent(int id = 0, wxEventType type = wxEVT_NULL)
        : wxNotifyEvent(type, id), m_sash_pos(0), m_vert_split(FALSE) {}

    wxSheetSplitterEvent(const wxSheetSplitterEvent& event) : wxNotifyEvent(event), 
              m_sash_pos(event.m_sash_pos), m_vert_split(event.m_vert_split) { }
    
    int GetSashPosition() const { return m_sash_pos; }
    // Setting the sash position, during a wxEVT_SHEET_SPLIT_CHANGING this
    //  value is used
    void SetSashPosition(int pos) { m_sash_pos = pos; }

    bool IsVerticalSplit() const { return m_vert_split; }

    // The sheet is valid only for the wxEVT_SHEET_SPLIT_BEGIN event type
    wxSheet* GetSheet() const { return wxDynamicCast(GetEventObject(), wxSheet); }
    // The sheet splitter is valid for the rest of the splitter events
    wxSheetSplitter* GetSheetSplitter() const { return wxDynamicCast(GetEventObject(), wxSheetSplitter); }
    
    // implementation
    virtual wxEvent *Clone() const { return new wxSheetSplitterEvent(*this); }
    
    int m_sash_pos;
    bool m_vert_split;
};

BEGIN_DECLARE_EVENT_TYPES()
    // The splitter buttons in the sheet have been clicked to begin splitting
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_SPLIT_BEGIN, 1593)
    // The splitter sash position is changing
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_SPLIT_CHANGING, 1593)
    // The splitter sash position has changed
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_SPLIT_CHANGED, 1593)
    // The splitter sash position has been double clicked on, will unsplit
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_SPLIT_DOUBLECLICKED, 1593)
    // The splitter has been unsplit
    DECLARE_EXPORTED_EVENT_TYPE(, wxEVT_SHEET_SPLIT_UNSPLIT, 1593)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*wxSheetSplitterEventFunction)(wxSheetSplitterEvent&);

#define EVT_SHEET_SPLIT_BEGIN(id, fn)         DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SPLIT_BEGIN,         id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetSplitterEventFunction, &fn), NULL ),
#define EVT_SHEET_SPLIT_CHANGING(id, fn)      DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SPLIT_CHANGING,      id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetSplitterEventFunction, &fn), NULL ),
#define EVT_SHEET_SPLIT_CHANGED(id, fn)       DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SPLIT_CHANGED,       id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetSplitterEventFunction, &fn), NULL ),
#define EVT_SHEET_SPLIT_DOUBLECLICKED(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SPLIT_DOUBLECLICKED, id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetSplitterEventFunction, &fn), NULL ),
#define EVT_SHEET_SPLIT_UNSPLIT(id, fn)       DECLARE_EVENT_TABLE_ENTRY( wxEVT_SHEET_SPLIT_UNSPLIT,       id, -1, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent(wxSheetSplitterEventFunction, &fn), NULL ),


#endif  // __WX_SHEETSPT_H__
