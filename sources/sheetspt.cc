///////////////////////////////////////////////////////////////////////////
// Name:        sheetspt.cpp
// Purpose:     wxSheetSplitter and related classes
// Author:      John Labenski
// Modified by: John Labenski
// Created:     4/1/2004
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "sheetspt.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "wx/defs.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_GRID

#include "sheetspt.h"

#ifndef WX_PRECOMP
    #include "wx/dcclient.h"
    #include "wx/settings.h"
    #include "wx/dcscreen.h"
#endif

#define PRINT_RECT(s, b)  wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), s, b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());

// ----------------------------------------------------------------------------
// wxSheetSplitter
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( wxSheetSplitter, wxWindow )

BEGIN_EVENT_TABLE( wxSheetSplitter, wxWindow )
    EVT_PAINT              ( wxSheetSplitter::OnPaint )
    EVT_MOUSE_EVENTS       ( wxSheetSplitter::OnMouse )
    EVT_SHEET_SPLIT_BEGIN  ( -1, wxSheetSplitter::OnSplit )
    EVT_SHEET_VIEW_CHANGED ( -1, wxSheetSplitter::OnViewChanged )
END_EVENT_TABLE()

void wxSheetSplitter::Init()
{
    m_tlSheet = NULL;
    m_trSheet = NULL;
    m_blSheet = NULL;
    m_brSheet = NULL;
    
    m_splitMode   = wxSHEET_SPLIT_NONE;
    m_splitCursor = wxSHEET_SPLIT_NONE;
    m_sash_width  = 4;
    
    m_enable_split_vert = TRUE;
    m_enable_split_horiz = TRUE;
}

bool wxSheetSplitter::Create(wxWindow *parent, wxWindowID id,
                             const wxPoint& pos, const wxSize& size,
                             long style, const wxString& name)
{
    if (!wxWindow::Create(parent, id, pos, size, style|wxCLIP_CHILDREN, name))
        return FALSE;
    
    SetForegroundColour(*wxBLACK);
    
    return TRUE;
}
    
wxSheetSplitter::~wxSheetSplitter() 
{
    // probably not necessary anymore, doesn't hurt though
    if (m_tlSheet) m_tlSheet->GetSheetRefData()->RemoveSheet(m_tlSheet);
    if (m_trSheet) m_trSheet->GetSheetRefData()->RemoveSheet(m_trSheet);
    if (m_blSheet) m_blSheet->GetSheetRefData()->RemoveSheet(m_blSheet);
    if (m_brSheet) m_brSheet->GetSheetRefData()->RemoveSheet(m_brSheet);
}

bool wxSheetSplitter::Destroy()
{
    // probably not necessary anymore, doesn't hurt though
    if (m_tlSheet) m_tlSheet->GetSheetRefData()->RemoveSheet(m_tlSheet);
    if (m_trSheet) m_trSheet->GetSheetRefData()->RemoveSheet(m_trSheet);
    if (m_blSheet) m_blSheet->GetSheetRefData()->RemoveSheet(m_blSheet);
    if (m_brSheet) m_brSheet->GetSheetRefData()->RemoveSheet(m_brSheet);
    
    return wxWindow::Destroy();
}

void wxSheetSplitter::Initialize(wxSheet* sheet)
{
    wxCHECK_RET(sheet && (sheet->GetParent() == this), wxT("Invalid sheet or parent"));
    m_tlSheet = sheet;
    ConfigureWindows();
    LayoutWindows();
}

void wxSheetSplitter::SplitVertically(int y_pos, bool sendEvt)
{
    wxCHECK_RET(!IsSplitVertically(), wxT("Already vertically split"));
    wxCHECK_RET((y_pos > 0) || (y_pos < GetClientSize().GetHeight()),
                wxT("Invalid vertical split position"));
    
    m_blSheet = new wxSheet(this, -1);
    m_blSheet->RefSheet(m_tlSheet);
    m_blSheet->GetColLabelWindow()->Show(FALSE);
    m_blSheet->GetCornerLabelWindow()->Show(FALSE);
    m_blSheet->EnableSplitVertically(FALSE);
    m_blSheet->SetGridOrigin(m_tlSheet->GetGridOrigin());
        
    if (m_trSheet && !m_brSheet)
    {
        m_brSheet = new wxSheet(this, -1);
        m_brSheet->RefSheet(m_tlSheet);
        m_brSheet->GetRowLabelWindow()->Show(FALSE);
        m_brSheet->GetColLabelWindow()->Show(FALSE);
        m_brSheet->GetCornerLabelWindow()->Show(FALSE);
        m_brSheet->EnableSplitVertically(FALSE);
        m_brSheet->EnableSplitHorizontally(FALSE);
        m_brSheet->SetGridOrigin(m_trSheet->GetGridOrigin());
    }

    m_splitPos.y = y_pos;
    ConfigureWindows();
    LayoutWindows();
    Refresh(FALSE);

    if (sendEvt)
        SendEvent( wxEVT_SHEET_SPLIT_CHANGED, TRUE );
}
void wxSheetSplitter::SplitHorizontally(int x_pos, bool sendEvt)
{
    wxCHECK_RET(!IsSplitHorizontally(), wxT("Already horizontally split"));
    wxCHECK_RET((x_pos > 0) || (x_pos < GetClientSize().GetWidth()),
                wxT("Invalid horizontal split position"));

    m_trSheet = new wxSheet(this, -1);
    m_trSheet->RefSheet(m_tlSheet);
    m_trSheet->GetRowLabelWindow()->Show(FALSE);
    m_trSheet->GetCornerLabelWindow()->Show(FALSE);
    m_trSheet->EnableSplitHorizontally(FALSE);
    m_trSheet->SetGridOrigin(m_tlSheet->GetGridOrigin());
    
    if (m_blSheet && !m_brSheet)
    {
        m_brSheet = new wxSheet(this, -1);
        m_brSheet->RefSheet(m_tlSheet);
        m_brSheet->GetRowLabelWindow()->Show(FALSE);
        m_brSheet->GetColLabelWindow()->Show(FALSE);
        m_brSheet->GetCornerLabelWindow()->Show(FALSE);
        m_brSheet->EnableSplitVertically(FALSE);
        m_brSheet->EnableSplitHorizontally(FALSE);
        m_brSheet->SetGridOrigin(m_blSheet->GetGridOrigin());
    }
    
    m_splitPos.x = x_pos;
    ConfigureWindows();
    LayoutWindows();
    Refresh(FALSE);
    
    if (sendEvt)
        SendEvent( wxEVT_SHEET_SPLIT_CHANGED, FALSE );
}

void wxSheetSplitter::UnsplitVertically(bool remove_bottom, bool sendEvt)
{
    wxCHECK_RET(IsSplitVertically(), wxT("Not vertically split"));

    wxPoint blOrigin = m_blSheet->GetGridOrigin();
    m_blSheet->Show(FALSE);
    m_blSheet->Destroy();
    m_blSheet = NULL;
            
    if (m_brSheet)
    {
        wxPoint brOrigin = m_brSheet->GetGridOrigin();
        m_brSheet->Show(FALSE);
        m_brSheet->Destroy();
        m_brSheet = NULL;
                
        if (!remove_bottom)
            m_trSheet->SetGridOrigin(brOrigin);
    }

    if (!remove_bottom)
        m_tlSheet->SetGridOrigin(blOrigin);

    m_splitPos.y = 0;
    ConfigureWindows();
    LayoutWindows();
    Refresh(FALSE);
    
    if (sendEvt)
        SendEvent( wxEVT_SHEET_SPLIT_UNSPLIT, TRUE );
}
void wxSheetSplitter::UnsplitHorizontally(bool remove_right, bool sendEvt)
{
    wxCHECK_RET(IsSplitHorizontally(), wxT("Not horizontally split"));
    
    wxPoint trOrigin = m_trSheet->GetGridOrigin();
    m_trSheet->Show(FALSE);
    m_trSheet->Destroy();
    m_trSheet = NULL;
    
    if (m_brSheet)
    {
        wxPoint brOrigin = m_brSheet->GetGridOrigin();
        m_brSheet->Show(FALSE);
        m_brSheet->Destroy();
        m_brSheet = NULL;
        
        if (!remove_right)
            m_blSheet->SetGridOrigin(brOrigin);
    }
    
    if (!remove_right)
        m_tlSheet->SetGridOrigin(trOrigin);

    m_splitPos.x = 0;
    ConfigureWindows();
    LayoutWindows();
    Refresh(FALSE);

    if (sendEvt)
        SendEvent( wxEVT_SHEET_SPLIT_UNSPLIT, FALSE );
}

void wxSheetSplitter::SetVerticalSplitPosition(int y_pos, bool sendEvt)
{
    wxCHECK_RET(m_tlSheet, wxT("wxSheetSplitter not initialized"));
    wxSize clientSize = GetClientSize();
    
    if ((y_pos <= 1) || (y_pos >= clientSize.y-1))
    {
        if (IsSplitVertically())
            UnsplitVertically(!(y_pos <= 1), sendEvt);
    }
    else if (!IsSplitVertically())
    {
        SplitVertically(y_pos, sendEvt);
    }
    else
    {    
        m_splitPos.y = y_pos;    
        LayoutWindows();
        Refresh(FALSE);
        
        if (sendEvt)
            SendEvent( wxEVT_SHEET_SPLIT_CHANGED, TRUE );
    }
}

void wxSheetSplitter::SetHorizontalSplitPosition(int x_pos, bool sendEvt)
{
    wxCHECK_RET(m_tlSheet, wxT("wxSheetSplitter not horizontally split"));
    wxSize clientSize = GetClientSize();
    
    if ((x_pos <= 1) || (x_pos >= clientSize.x-1))
    {
        if (IsSplitHorizontally())
            UnsplitHorizontally(!(x_pos <= 1), sendEvt);
    }
    else if (!IsSplitHorizontally())
    {
        SplitHorizontally(x_pos, sendEvt);
    }
    else
    {
        m_splitPos.x = x_pos;
        LayoutWindows();
        Refresh(FALSE);
        
        if (sendEvt)
            SendEvent( wxEVT_SHEET_SPLIT_CHANGED, FALSE );
    }
}

void wxSheetSplitter::ConfigureWindows()
{
    if (!m_tlSheet)
        return;
    
    if (m_tlSheet)
    {
        int sb_mode = (m_trSheet ? wxSheet::SB_VERT_NEVER  : wxSheet::SB_AS_NEEDED) |
                      (m_blSheet ? wxSheet::SB_HORIZ_NEVER : wxSheet::SB_AS_NEEDED);
        
        m_tlSheet->SetScrollBarMode(sb_mode);
        m_tlSheet->EnableSplitVertically(!m_blSheet && m_enable_split_vert);
        m_tlSheet->EnableSplitHorizontally(!m_trSheet && m_enable_split_horiz);
    }
    if (m_trSheet)
    {
        int sb_mode = (m_brSheet ? wxSheet::SB_HORIZ_NEVER : wxSheet::SB_AS_NEEDED);
        
        m_trSheet->SetHorizontalScrollBarMode(sb_mode);
        m_trSheet->EnableSplitVertically(!m_brSheet && m_enable_split_vert);
    }
    if (m_blSheet)
    {
        int sb_mode = (m_brSheet ? wxSheet::SB_VERT_NEVER : wxSheet::SB_AS_NEEDED);
        
        m_blSheet->SetVerticalScrollBarMode(sb_mode);
        m_blSheet->EnableSplitHorizontally(!m_brSheet && m_enable_split_horiz);
    }    
}

void wxSheetSplitter::LayoutWindows()
{
    if (!m_tlSheet)
        return;
    
    wxSize clientSize = GetClientSize();
    
    //if (!m_trSheet && !m_blSheet)
    //{
    //    m_splitPos = wxPoint(-1, -1); // reset
    //    m_tlSheet->SetSize(0, 0, clientSize.x, clientSize.y);
    //    return;
    //}

    int x = m_splitPos.x;
    int y = m_splitPos.y;
    
    if (!m_trSheet)
        x = clientSize.x;
    else if (m_trSheet && (x > clientSize.x))
    {
        // fix sash position to stay within visible part of window
        x = clientSize.x - 20;
        if (x < 2)
            x = clientSize.x/2;
        
        m_splitPos.x = x;
    }
    
    if (!m_blSheet)
        y = clientSize.y;
    else if (m_blSheet && (y > clientSize.y))
    {
        // fix sash position to stay within visible part of window
        y = clientSize.y - 20;
        if (y < 2)
            y = clientSize.y/2;
        
        m_splitPos.y = y;
    }

    int vb = m_trSheet ? m_sash_width/2 : 0; 
    int hb = m_blSheet ? m_sash_width/2 : 0;
    
    wxRect rect(0, 0, x-vb, y-hb);
    //PRINT_RECT(wxT("m_tlSheet "), rect);
    if (m_tlSheet->GetRect() != rect)
        m_tlSheet->SetSize(rect);
    
    if (m_trSheet)
    {
        rect = wxRect(x+vb, 0, clientSize.x-x-vb, y-hb);
        //PRINT_RECT(wxT("m_trSheet "), rect);
        if (m_trSheet->GetRect() != rect)
            m_trSheet->SetSize(rect);
    }
    if (m_blSheet)
    {
        rect = wxRect(0, y+hb, x-vb, clientSize.y-y-hb);
        //PRINT_RECT(wxT("m_blSheet "), rect);
        if (m_blSheet->GetRect() != rect)
            m_blSheet->SetSize(rect);
    }
    if (m_brSheet)
    {
        rect = wxRect(x+vb, y+hb, clientSize.x-x-vb, clientSize.y-y-hb);
        //PRINT_RECT(wxT("m_brSheet "), rect);
        if (m_brSheet->GetRect() != rect)
            m_brSheet->SetSize(rect);
    }

    if (m_tlSheet) m_tlSheet->CalcWindowSizes();
    if (m_trSheet) m_trSheet->CalcWindowSizes();
    if (m_blSheet) m_blSheet->CalcWindowSizes();
    if (m_brSheet) m_brSheet->CalcWindowSizes();
}

void wxSheetSplitter::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
    wxWindow::DoSetSize(x, y, width, height, sizeFlags);
    LayoutWindows();
}

void wxSheetSplitter::OnViewChanged(wxSheetEvent& event)
{
    wxSheet *sheet = (wxSheet*)event.GetEventObject();
    wxPoint origin = sheet->GetGridOrigin();
    
    if (sheet == m_tlSheet)
    {
        if (m_trSheet)
            m_trSheet->SetGridOrigin(-1, origin.y);
        if (m_blSheet)
            m_blSheet->SetGridOrigin(origin.x, -1);
    }
    else if (sheet == m_trSheet)
    {
        if (m_tlSheet)
            m_tlSheet->SetGridOrigin(-1, origin.y);
        if (m_brSheet)
            m_brSheet->SetGridOrigin(origin.x, -1);
    }
    else if (sheet == m_blSheet)
    {
        if (m_brSheet)
            m_brSheet->SetGridOrigin(-1, origin.y);
        if (m_tlSheet)
            m_tlSheet->SetGridOrigin(origin.x, -1);
    }
    else if (sheet == m_brSheet)
    {
        if (m_blSheet)
            m_blSheet->SetGridOrigin(-1, origin.y);
        if (m_trSheet)
            m_trSheet->SetGridOrigin(origin.x, -1);
    }
}

void wxSheetSplitter::OnSplit(wxSheetSplitterEvent& event)
{
    if (!HasCapture())
        CaptureMouse();
    
    m_splitMode = event.IsVerticalSplit() ? wxSHEET_SPLIT_VERTICAL : wxSHEET_SPLIT_HORIZONTAL;
    SetMouseCursor(m_splitMode);
    m_mousePos = wxPoint(-1, -1); // out of bounds, don't draw initial tracker
}

void wxSheetSplitter::OnMouse(wxMouseEvent& event)
{
    wxPoint mousePos = event.GetPosition();
    
    if (event.LeftDown())
    {
        m_splitMode = SashHitTest(mousePos);
        SetMouseCursor(m_splitMode);
        if ((m_splitMode != wxSHEET_SPLIT_NONE) && !HasCapture())
            CaptureMouse();
    }
    else if (event.Moving())
    {
        SetMouseCursor(SashHitTest(mousePos));
    }
    else if (event.Dragging() && HasCapture())
    {
        DrawSashTracker(m_mousePos.x, m_mousePos.y);
        
        bool vert = (m_splitMode == wxSHEET_SPLIT_VERTICAL);

        wxSheetSplitterEvent sEvent(GetId(), wxEVT_SHEET_SPLIT_CHANGING);
        sEvent.SetEventObject(this);
        sEvent.m_vert_split = vert;
        sEvent.SetSashPosition(vert ? m_mousePos.y : m_mousePos.x);
        if (DoSendEvent(sEvent))
        {
            m_mousePos = mousePos;
            if (vert)
                m_mousePos.x = sEvent.GetSashPosition();
            else
                m_mousePos.y = sEvent.GetSashPosition();
        }
        
        DrawSashTracker(m_mousePos.x, m_mousePos.y);
    }
    else if (event.LeftUp() && HasCapture())
    {
        ReleaseMouse();
        
        m_mousePos = mousePos;
        DrawSashTracker(m_mousePos.x, m_mousePos.y);

        if (m_splitMode == wxSHEET_SPLIT_VERTICAL)
            SetVerticalSplitPosition(m_mousePos.y, TRUE);
        else if (m_splitMode == wxSHEET_SPLIT_HORIZONTAL)
            SetHorizontalSplitPosition(m_mousePos.x, TRUE);

        m_mousePos = wxPoint(-1, -1);
        m_splitMode = wxSHEET_SPLIT_NONE;
    }    
    else if (event.LeftDClick())
    {
        m_splitMode = SashHitTest(mousePos);
        
        if (SendEvent(wxEVT_SHEET_SPLIT_DOUBLECLICKED, 
                      m_splitMode == wxSHEET_SPLIT_VERTICAL))
        {
            if (m_splitMode == wxSHEET_SPLIT_VERTICAL)
                UnsplitVertically(TRUE, TRUE);
            else if (m_splitMode == wxSHEET_SPLIT_HORIZONTAL)
                UnsplitHorizontally(TRUE, TRUE);
        }
    }
    else if (event.Leaving() || event.Entering())
    {
        SetMouseCursor(wxSHEET_SPLIT_NONE);
    }
}

void wxSheetSplitter::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    wxPaintDC dc(this);
    
    wxSize clientSize = GetClientSize();
    bool draw_horiz = (m_splitPos.x > 0) && (m_splitPos.x < clientSize.x);
    bool draw_vert  = (m_splitPos.y > 0) && (m_splitPos.y < clientSize.y);

    if (draw_horiz || draw_vert)
    {
        dc.SetPen(wxPen(GetForegroundColour(), m_sash_width, wxSOLID));
        
        if (draw_horiz)
            dc.DrawLine(m_splitPos.x, 0, m_splitPos.x, clientSize.y);
        if (draw_vert)
            dc.DrawLine(0, m_splitPos.y, clientSize.x, m_splitPos.y);
    }        
}

void wxSheetSplitter::DrawSashTracker(int x, int y)
{
    if ((x < 0) && (y < 0))
        return;
    
    int w, h;
    GetClientSize(&w, &h);

    int x1, y1;
    int x2, y2;

    if ( m_splitMode == wxSHEET_SPLIT_HORIZONTAL )
    {
        x1 = x2 = ((x > w) ? w : ((x < 0) ? 0 : x)); 
        y1 = 2;
        y2 = h-2;
    }
    else
    {
        x1 = 2;   
        x2 = w-2; 
        y1 = y2 = ((y > h) ? h : ((y < 0) ? 0 : y)); 
    }

    ClientToScreen(&x1, &y1);
    ClientToScreen(&x2, &y2);

    wxScreenDC screenDC;
    screenDC.SetLogicalFunction(wxINVERT);
    screenDC.SetPen(wxPen(GetForegroundColour(), m_sash_width, wxSOLID));
    screenDC.SetBrush(*wxTRANSPARENT_BRUSH);
    screenDC.DrawLine(x1, y1, x2, y2);
    screenDC.SetLogicalFunction(wxCOPY);
}

int wxSheetSplitter::SashHitTest(const wxPoint& pt)
{
    if ((m_splitPos.x <= 0) && (m_splitPos.y <= 0))
        return wxSHEET_SPLIT_NONE;
    
    wxSize clientSize = GetClientSize();
    
    if (m_trSheet && (m_splitPos.x > 0))
    {
        wxRect rect(m_splitPos.x-m_sash_width/2, 0, m_sash_width, clientSize.y);
        if (rect.Inside(pt))
            return wxSHEET_SPLIT_HORIZONTAL;
    }

    if (m_blSheet && (m_splitPos.y > 0))
    {
        wxRect rect(0, m_splitPos.y-m_sash_width/2, clientSize.x, m_sash_width);
        if (rect.Inside(pt))
            return wxSHEET_SPLIT_VERTICAL;
    }

    return wxSHEET_SPLIT_NONE;
}

void wxSheetSplitter::SetMouseCursor(int sheet_split_mode)
{
    if (m_splitCursor == sheet_split_mode)
        return;
    
    m_splitCursor = sheet_split_mode;
    
    switch (sheet_split_mode)
    {
        case wxSHEET_SPLIT_VERTICAL   : SetCursor(wxCURSOR_SIZENS); break;
        case wxSHEET_SPLIT_HORIZONTAL : SetCursor(wxCURSOR_SIZEWE); break;
        case wxSHEET_SPLIT_NONE :
        default :
            SetCursor(*wxSTANDARD_CURSOR);
    }
}

bool wxSheetSplitter::SendEvent( const wxEventType type, bool vert_split )
{
    wxSheetSplitterEvent event(GetId(), type);
    event.SetEventObject(this);
    event.m_vert_split = vert_split;
    event.m_sash_pos = vert_split ? m_splitPos.y : m_splitPos.x;
    return DoSendEvent(event);
}

bool wxSheetSplitter::DoSendEvent( wxSheetSplitterEvent& event )
{
    bool claimed = GetEventHandler()->ProcessEvent(event);
    bool vetoed  = !event.IsAllowed();
    return !vetoed && !claimed;
}

// ----------------------------------------------------------------------------
// events
// ----------------------------------------------------------------------------

DEFINE_EVENT_TYPE(wxEVT_SHEET_SPLIT_BEGIN)
DEFINE_EVENT_TYPE(wxEVT_SHEET_SPLIT_CHANGING)
DEFINE_EVENT_TYPE(wxEVT_SHEET_SPLIT_CHANGED)
DEFINE_EVENT_TYPE(wxEVT_SHEET_SPLIT_DOUBLECLICKED)
DEFINE_EVENT_TYPE(wxEVT_SHEET_SPLIT_UNSPLIT)

#endif // wxUSE_GRID
