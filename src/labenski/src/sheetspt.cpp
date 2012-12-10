///////////////////////////////////////////////////////////////////////////////
// Name:        sheetspt.cpp
// Purpose:     wxSheetSplitter and related classes
// Author:      John Labenski
// Modified by: 
// Created:     4/1/2004
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/sheet/sheetspt.h"

#ifndef WX_PRECOMP
    #include "wx/dcclient.h"
    #include "wx/settings.h"
    #include "wx/dcscreen.h"
#endif

#include "wx/renderer.h"

#define PRINT_RECT(s, b)  wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), s, b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());

// ----------------------------------------------------------------------------
// wxSheetSplitter
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( wxSheetSplitter, wxWindow )

BEGIN_EVENT_TABLE( wxSheetSplitter, wxWindow )
    EVT_PAINT              ( wxSheetSplitter::OnPaint )
    EVT_SIZE               ( wxSheetSplitter::OnSize )
    EVT_MOUSE_EVENTS       ( wxSheetSplitter::OnMouse )
    EVT_SHEET_SPLIT_BEGIN  ( wxID_ANY, wxSheetSplitter::OnSplit )
    EVT_SHEET_VIEW_CHANGED ( wxID_ANY, wxSheetSplitter::OnViewChanged )
    
    WX_EVENT_TABLE_CONTROL_CONTAINER(wxSheetSplitter)
END_EVENT_TABLE()

#if wxCHECK_VERSION(2, 7, 0)
WX_DELEGATE_TO_CONTROL_CONTAINER(wxSheetSplitter, wxWindow);
#else
WX_DELEGATE_TO_CONTROL_CONTAINER(wxSheetSplitter);
#endif  // wxCHECK_VERSION

void wxSheetSplitter::Init()
{
    m_container.SetContainerWindow(this);

    m_tlSheet = NULL;
    m_trSheet = NULL;
    m_blSheet = NULL;
    m_brSheet = NULL;
    
    m_splitMode   = wxSHEET_SPLIT_NONE;
    m_splitCursor = wxSHEET_SPLIT_NONE;
    //m_sash_width  = 4;
    
    m_enable_split_vert = true;
    m_enable_split_horiz = true;
}

bool wxSheetSplitter::Create(wxWindow *parent, wxWindowID id,
                             const wxPoint& pos, const wxSize& size,
                             long style, const wxString& name)
{
    // allow TABbing from one window to the other
    style |= wxTAB_TRAVERSAL;

    // we draw our border ourselves to blend the sash with it
    style &= ~wxBORDER_MASK;
    style |= wxBORDER_NONE;

    if (!wxWindow::Create(parent, id, pos, size, style|wxCLIP_CHILDREN, name))
        return false;
    
    //SetForegroundColour(*wxBLACK);

    // don't erase the splitter background, it's pointless as we overwrite it
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
   
    return true;
}
    
wxSheetSplitter::~wxSheetSplitter() 
{
    // stop paint events, probably not necessary anymore, doesn't hurt though
    if (m_tlSheet) m_tlSheet->GetSheetRefData()->RemoveSheet(m_tlSheet);
    if (m_trSheet) m_trSheet->GetSheetRefData()->RemoveSheet(m_trSheet);
    if (m_blSheet) m_blSheet->GetSheetRefData()->RemoveSheet(m_blSheet);
    if (m_brSheet) m_brSheet->GetSheetRefData()->RemoveSheet(m_brSheet);
}

bool wxSheetSplitter::Destroy()
{
    // stop paint events, probably not necessary anymore, doesn't hurt though
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
    // In >= wx25 a window's min size is set when created with a size, breaks splitting
    m_tlSheet->SetSizeHints(-1, -1); 
    ConfigureWindows();
    LayoutWindows();
}

wxSheet* wxSheetSplitter::CreateSheet(wxWindowID id)
{
    wxCHECK_MSG(m_tlSheet, NULL, wxT("Unable to create wxSheet child in splitter, splitter not initialized yet."));
    wxSheetSplitterEvent event(GetId(), wxEVT_SHEET_SPLIT_CREATE_SHEET);
    event.SetEventObject(this);
    event.SetExtraLong(id);
    GetEventHandler()->ProcessEvent(event);
    if (event.GetSheet())
    {
        wxSheet *sheet = event.GetSheet();
        wxCHECK_MSG(sheet->GetParent() == this, sheet, wxT("Invalid parent for wxSheet in splitter"));
        return sheet; 
    }

    return m_tlSheet->Clone(id);
}

wxSheet* wxSheetSplitter::CreateTopRightSheet(wxWindowID id)
{
    wxSheet *sheet = CreateSheet(id);
    wxCHECK_MSG(sheet, NULL, wxT("Unable to create wxSheet child in splitter"));
    sheet->RefSheet(m_tlSheet);
    sheet->GetRowLabelWindow()->Show(false);
    sheet->GetCornerLabelWindow()->Show(false);
    sheet->EnableSplitHorizontally(false);
    sheet->SetGridOrigin(m_tlSheet->GetGridOrigin());
    return sheet;
}
wxSheet* wxSheetSplitter::CreateBottomLeftSheet(wxWindowID id)
{
    wxSheet *sheet = CreateSheet(id);
    wxCHECK_MSG(sheet, NULL, wxT("Unable to create wxSheet child in splitter"));
    sheet->RefSheet(m_tlSheet);
    sheet->GetColLabelWindow()->Show(false);
    sheet->GetCornerLabelWindow()->Show(false);
    sheet->EnableSplitVertically(false);
    sheet->SetGridOrigin(m_tlSheet->GetGridOrigin());    
    return sheet;
}
wxSheet* wxSheetSplitter::CreateBottomRightSheet(wxWindowID id)
{
    wxSheet *sheet = CreateSheet(id);
    wxCHECK_MSG(sheet, NULL, wxT("Unable to create wxSheet child in splitter"));
    sheet->RefSheet(m_tlSheet);
    sheet->GetRowLabelWindow()->Show(false);
    sheet->GetColLabelWindow()->Show(false);
    sheet->GetCornerLabelWindow()->Show(false);
    sheet->EnableSplitVertically(false);
    sheet->EnableSplitHorizontally(false);
    sheet->SetGridOrigin(m_trSheet->GetGridOrigin());
    return sheet;
}

void wxSheetSplitter::SplitVertically(int y_pos, bool sendEvt)
{
    wxCHECK_RET(m_tlSheet, wxT("Unable to split vertically, not initialized yet."));
    wxCHECK_RET(!IsSplitVertically(), wxT("Already vertically split"));
    wxCHECK_RET((y_pos >= m_minSize.y) || 
                (y_pos < GetClientSize().GetHeight() - m_minSize.y),
                wxT("Invalid vertical split position"));
    
    m_blSheet = CreateBottomLeftSheet();
    wxCHECK_RET(m_blSheet, wxT("Unable to create wxSheet child in splitter"));

    if (m_trSheet && !m_brSheet)
    {
        m_brSheet = CreateBottomRightSheet();
        wxCHECK_RET(m_brSheet, wxT("Unable to create wxSheet child in splitter"));
    }

    m_splitPos.y = y_pos;
    ConfigureWindows();
    LayoutWindows();
    Refresh(false);

    if (sendEvt)
        SendEvent( wxEVT_SHEET_SPLIT_CHANGED, true );
}
void wxSheetSplitter::SplitHorizontally(int x_pos, bool sendEvt)
{
    wxCHECK_RET(m_tlSheet, wxT("Unable to split horizontally, not initialized yet."));
    wxCHECK_RET(!IsSplitHorizontally(), wxT("Already horizontally split"));
    wxCHECK_RET((x_pos >= m_minSize.x) || 
                (x_pos < GetClientSize().GetWidth() - m_minSize.x),
                wxT("Invalid horizontal split position"));

    m_trSheet = CreateTopRightSheet();
    wxCHECK_RET(m_trSheet, wxT("Unable to create wxSheet child in splitter"));
    
    if (m_blSheet && !m_brSheet)
    {
        m_brSheet = CreateBottomRightSheet();
        wxCHECK_RET(m_brSheet, wxT("Unable to create wxSheet child in splitter"));
    }
    
    m_splitPos.x = x_pos;
    ConfigureWindows();
    LayoutWindows();
    Refresh(false);
    
    if (sendEvt)
        SendEvent( wxEVT_SHEET_SPLIT_CHANGED, false );
}

void wxSheetSplitter::UnsplitVertically(bool remove_bottom, bool sendEvt)
{
    wxCHECK_RET(IsSplitVertically(), wxT("Not vertically split"));

    wxPoint blOrigin = m_blSheet->GetGridOrigin();
    m_blSheet->Show(false);
    m_blSheet->Destroy();
    m_blSheet = NULL;
            
    if (m_brSheet)
    {
        wxPoint brOrigin = m_brSheet->GetGridOrigin();
        m_brSheet->Show(false);
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
    Refresh(false);
    
    if (sendEvt)
        SendEvent( wxEVT_SHEET_SPLIT_UNSPLIT, true );
}
void wxSheetSplitter::UnsplitHorizontally(bool remove_right, bool sendEvt)
{
    wxCHECK_RET(IsSplitHorizontally(), wxT("Not horizontally split"));
    
    wxPoint trOrigin = m_trSheet->GetGridOrigin();
    m_trSheet->Show(false);
    m_trSheet->Destroy();
    m_trSheet = NULL;
    
    if (m_brSheet)
    {
        wxPoint brOrigin = m_brSheet->GetGridOrigin();
        m_brSheet->Show(false);
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
    Refresh(false);

    if (sendEvt)
        SendEvent( wxEVT_SHEET_SPLIT_UNSPLIT, false );
}

void wxSheetSplitter::SetVerticalSplitPosition(int y_pos, bool sendEvt)
{
    wxCHECK_RET(m_tlSheet, wxT("wxSheetSplitter not initialized"));
    y_pos = GetAdjustedVerticalSashPosition(y_pos);
    wxSize clientSize(GetClientSize());
    int border_size = GetBorderSize();   
    
    if ((y_pos <= border_size) || (y_pos >= clientSize.y-border_size))
    {
        if (IsSplitVertically())
            UnsplitVertically(!(y_pos <= border_size), sendEvt);
    }
    else if (!IsSplitVertically())
    {
        SplitVertically(y_pos, sendEvt);
    }
    else if (m_splitPos.y != y_pos)
    {    
        m_splitPos.y = y_pos;    
        LayoutWindows();
        Refresh(false);
        
        if (sendEvt)
            SendEvent( wxEVT_SHEET_SPLIT_CHANGED, true );
    }
}

void wxSheetSplitter::SetHorizontalSplitPosition(int x_pos, bool sendEvt)
{
    wxCHECK_RET(m_tlSheet, wxT("wxSheetSplitter not horizontally split"));
    x_pos = GetAdjustedHorizontalSashPosition(x_pos);
    wxSize clientSize(GetClientSize());
    int border_size = GetBorderSize();
    
    if ((x_pos <= border_size) || (x_pos >= clientSize.x-border_size))
    {
        if (IsSplitHorizontally())
            UnsplitHorizontally(!(x_pos <= border_size), sendEvt);
    }
    else if (!IsSplitHorizontally())
    {
        SplitHorizontally(x_pos, sendEvt);
    }
    else if (m_splitPos.x != x_pos)
    {
        m_splitPos.x = x_pos;
        LayoutWindows();
        Refresh(false);
        
        if (sendEvt)
            SendEvent( wxEVT_SHEET_SPLIT_CHANGED, false );
    }
}

int wxSheetSplitter::GetAdjustedVerticalSashPosition(int pos) const
{
    int width = GetClientSize().GetWidth();

    if (width < m_minSize.x)
        pos = width/2;
    else if (pos <= m_minSize.x)
        pos = m_minSize.x;
    else if (pos >= width - m_minSize.x)
        pos = width - m_minSize.x;
    
    return pos;    
}

int wxSheetSplitter::GetAdjustedHorizontalSashPosition(int pos) const
{
    int height = GetClientSize().GetHeight();
    
    if (height < m_minSize.y)
        pos = height/2;
    else if (pos <= m_minSize.y)
        return m_minSize.y;
    else if (pos >= height - m_minSize.y)
        return height - m_minSize.y;

    return pos;    
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
    
    wxSize clientSize(GetClientSize());
    
    int x = m_splitPos.x;
    int y = m_splitPos.y;
    int sash_size   = GetSashSize();
    int border_size = GetBorderSize();

    if (!m_trSheet)
        x = clientSize.x - 2*border_size;
/*    
    else if (m_trSheet && (x > clientSize.x - m_minSize.x))
    {
        // fix sash position to stay within visible part of window
        x = clientSize.x - m_minSize.x;
        if (x < sash_size + border_size + 2)
            x = clientSize.x/2;
        
        m_splitPos.x = x;
    }
*/  
    
    if (!m_blSheet)
        y = clientSize.y - 2*border_size;
/*    
    else if (m_blSheet && (y > clientSize.y - m_minSize.y))
    {
        // fix sash position to stay within visible part of window
        y = clientSize.y - m_minSize.y;
        if (y < sash_size + border_size + 2)
            y = clientSize.y/2;
        
        m_splitPos.y = y;
    }
*/
    
    wxRect rect = wxRect(border_size, border_size, x, y);
    if (m_tlSheet->GetRect() != rect)
        m_tlSheet->SetSize(rect);
    
    if (m_trSheet)
    {
        rect = wxRect(x+sash_size, border_size, clientSize.x-x-sash_size-border_size, y);
        if (m_trSheet->GetRect() != rect)
            m_trSheet->SetSize(rect);
    }
    if (m_blSheet)
    {
        rect = wxRect(border_size, y+sash_size, x, clientSize.y-y-sash_size-border_size);
        if (m_blSheet->GetRect() != rect)
            m_blSheet->SetSize(rect);
    }
    if (m_brSheet)
    {
        rect = wxRect(x+sash_size, y+sash_size, clientSize.x-x-sash_size-border_size, clientSize.y-y-sash_size-border_size);
        if (m_brSheet->GetRect() != rect)
            m_brSheet->SetSize(rect);
    }
}

void wxSheetSplitter::OnSize( wxSizeEvent& event )
{    
    m_splitPos.x = GetAdjustedVerticalSashPosition(m_splitPos.x);
    m_splitPos.y = GetAdjustedHorizontalSashPosition(m_splitPos.y);
    LayoutWindows();

    event.Skip();
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
    // pass event to parent to see if they want to veto it first
    GetParent()->GetEventHandler()->ProcessEvent(event);
    if (!event.IsAllowed())
        return;
    
    if (!HasCapture())
        CaptureMouse();
    
    m_splitMode = event.IsVerticalSplit() ? wxSHEET_SPLIT_VERTICAL : wxSHEET_SPLIT_HORIZONTAL;
    SetMouseCursor(m_splitMode);
    // out of bounds, don't draw initial tracker
    m_mousePos = wxPoint(-10, -10); 
}

void wxSheetSplitter::OnMouse(wxMouseEvent& event)
{
    wxPoint mousePos(event.GetPosition());
    
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
        
        DrawSashTracker(m_mousePos.x, m_mousePos.y);
        m_mousePos = mousePos;

        if (m_splitMode == wxSHEET_SPLIT_VERTICAL)
            SetVerticalSplitPosition(m_mousePos.y, true);
        else if (m_splitMode == wxSHEET_SPLIT_HORIZONTAL)
            SetHorizontalSplitPosition(m_mousePos.x, true);

        m_mousePos = wxPoint(-10, -10);
        m_splitMode = wxSHEET_SPLIT_NONE;
    }    
    else if (event.LeftDClick())
    {
        m_splitMode = SashHitTest(mousePos);
        
        if (SendEvent(wxEVT_SHEET_SPLIT_DOUBLECLICKED, 
                      m_splitMode == wxSHEET_SPLIT_VERTICAL))
        {
            if ((m_minSize.y == 0) && (m_splitMode == wxSHEET_SPLIT_VERTICAL))
                UnsplitVertically(true, true);
            else if ((m_minSize.x == 0) && (m_splitMode == wxSHEET_SPLIT_HORIZONTAL))
                UnsplitHorizontally(true, true);
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
    DrawSash(dc);
}

void wxSheetSplitter::DrawSash(wxDC &dc)
{
    wxSize clientSize(GetClientSize());
    bool draw_horiz = (m_splitPos.x > 0) && (m_splitPos.x < clientSize.x);
    bool draw_vert  = (m_splitPos.y > 0) && (m_splitPos.y < clientSize.y);

#if 0 // old method where we just draw a black line
    if (draw_horiz || draw_vert)
    {
        int sash_size = GetSashSize();
        dc.SetPen(wxPen(GetForegroundColour(), sash_size, wxSOLID));
        
        if (draw_horiz)
            dc.DrawLine(m_splitPos.x + sash_size/2, 0, m_splitPos.x + sash_size/2, clientSize.y);
        if (draw_vert)
            dc.DrawLine(0, m_splitPos.y + sash_size/2, clientSize.x, m_splitPos.y + sash_size/2);
    }
#else
    wxRendererNative& ren = wxRendererNative::Get();

    if (HasFlag(wxSP_3DBORDER))
        ren.DrawSplitterBorder(this, dc, GetClientRect());

    // don't draw sash if we're configured to not show it
    if ( HasFlag(wxSP_NOSASH) )
        return;

    if (draw_horiz)
        ren.DrawSplitterSash(this, dc, clientSize, m_splitPos.x, wxVERTICAL, 
              (m_splitCursor == wxSHEET_SPLIT_HORIZONTAL) ? (int)wxCONTROL_CURRENT : 0);
    if (draw_vert)
        ren.DrawSplitterSash(this, dc, clientSize, m_splitPos.y, wxHORIZONTAL,
              (m_splitCursor == wxSHEET_SPLIT_VERTICAL) ? (int)wxCONTROL_CURRENT : 0);
#endif
}

void wxSheetSplitter::DrawSashTracker(int x, int y)
{
    if ((x < 0) && (y < 0))
        return;
    
    int w, h;
    GetClientSize(&w, &h);

    int x1, y1;
    int x2, y2;

    int sash_size = GetSashSize();
    int border_size = GetBorderSize();

    if ( m_splitMode == wxSHEET_SPLIT_HORIZONTAL )
    {
        x1 = x2 = ((x > w) ? w : ((x < 0) ? 0 : x)) + sash_size/2; 
        y1 = border_size;
        y2 = h-border_size;
    }
    else
    {
        x1 = border_size;   
        x2 = w-border_size; 
        y1 = y2 = ((y > h) ? h : ((y < 0) ? 0 : y)) + sash_size/2; 
    }

    ClientToScreen(&x1, &y1);
    ClientToScreen(&x2, &y2);

    wxScreenDC screenDC;
    screenDC.SetLogicalFunction(wxINVERT);
    screenDC.SetPen(wxPen(GetForegroundColour(), sash_size, wxSOLID));
    screenDC.SetBrush(*wxTRANSPARENT_BRUSH);
    screenDC.DrawLine(x1, y1, x2, y2);
    screenDC.SetLogicalFunction(wxCOPY);
}

int wxSheetSplitter::GetSashSize() const
{
    return wxRendererNative::Get().GetSplitterParams(this).widthSash;
}

int wxSheetSplitter::GetBorderSize() const
{
    return wxRendererNative::Get().GetSplitterParams(this).border;
}

int wxSheetSplitter::SashHitTest(const wxPoint& pt) const
{
    if ((m_splitPos.x <= 0) && (m_splitPos.y <= 0))
        return wxSHEET_SPLIT_NONE;
    
    wxSize clientSize = GetClientSize();
    int sash_size   = GetSashSize();
    int border_size = GetBorderSize();

    if (m_trSheet && (m_splitPos.x > 0))
    {
        wxRect rect(m_splitPos.x, border_size, sash_size, clientSize.y-2*border_size);
#if wxCHECK_VERSION(2,7,0)
        if (rect.Contains(pt))
#else
        if (rect.Inside(pt))
#endif
            return wxSHEET_SPLIT_HORIZONTAL;
    }

    if (m_blSheet && (m_splitPos.y > 0))
    {
        wxRect rect(border_size, m_splitPos.y, clientSize.x-2*border_size, sash_size);
#if wxCHECK_VERSION(2,7,0)
        if (rect.Contains(pt))
#else
        if (rect.Inside(pt))
#endif
            return wxSHEET_SPLIT_VERTICAL;
    }

    return wxSHEET_SPLIT_NONE;
}

void wxSheetSplitter::SetMouseCursor(int sheet_split_mode)
{
    if (m_splitCursor == sheet_split_mode)
        return;

    m_splitCursor = sheet_split_mode;
    wxClientDC dc(this);
    DrawSash(dc);
    
    switch (sheet_split_mode)
    {
        case wxSHEET_SPLIT_VERTICAL   : SetCursor(wxCURSOR_SIZENS); break;
        case wxSHEET_SPLIT_HORIZONTAL : SetCursor(wxCURSOR_SIZEWE); break;
        case wxSHEET_SPLIT_NONE       :
        default                       : SetCursor(*wxSTANDARD_CURSOR);
    }
}

bool wxSheetSplitter::SendEvent( wxEventType type, bool vert_split )
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
DEFINE_EVENT_TYPE(wxEVT_SHEET_SPLIT_CREATE_SHEET)
