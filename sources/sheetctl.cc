///////////////////////////////////////////////////////////////////////////
// Name:        sheetctl.cpp
// Purpose:     wxSheet controls
// Author:      Paul Gammans, Roger Gammans
// Modified by: John Labenski
// Created:     11/04/2001
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, The Computer Surgery (paul@compsurg.co.uk)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "sheetctl.h"
#endif

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_GRID

#ifndef WX_PRECOMP
    #include "wx/defs.h"
    #include "wx/utils.h"    
    #include "wx/textctrl.h"
    #include "wx/dc.h"
    #include "wx/dcclient.h"    
    #include "wx/textctrl.h"
    #include "wx/checkbox.h"
    #include "wx/combobox.h"
    #include "wx/log.h"
    #include "wx/intl.h"    
    #include "wx/valtext.h"
    #include "wx/settings.h"    
#endif // WX_PRECOMP

#include "sheet.h"
#include "sheetctl.h"
#include "wx/tokenzr.h"
#include "wx/spinctrl.h"
#include "wx/intl.h"

#define PRINT_RECT(s, b) wxPrintf("%s %d %d %d %d - w%d h%d\n", s, b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());

#if defined(__WXMOTIF__)
    #define WXUNUSED_MOTIF(identifier)  WXUNUSED(identifier)
#else
    #define WXUNUSED_MOTIF(identifier)  identifier
#endif // defined(__WXMOTIF__)

#if defined(__WXGTK__)
    #define WXUNUSED_GTK(identifier)    WXUNUSED(identifier)
#else
    #define WXUNUSED_GTK(identifier)    identifier
#endif // defined(__WXGTK__)

const wxSheetCellEditor wxNullSheetCellEditor;
const wxSheetCellRenderer wxNullSheetCellRenderer;

/* XPM */
static const char *s_overflow_arrow_xpm_data[] = {
/* columns rows colors chars-per-pixel */
"4 6 2 1",
"  c None",
"a c Black",
/* pixels */
"aa  ",
"aaa ",
"aaaa",
"aaaa",
"aaa ",
"aa  "};

static wxBitmap s_overflowBitmap; // a bitmap to indicate cell overflows

IMPLEMENT_DYNAMIC_CLASS(wxSheetCellRenderer, wxObject)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellRendererRefData, wxObject)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellStringRendererRefData, wxSheetCellRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellNumberRendererRefData, wxSheetCellStringRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellFloatRendererRefData, wxSheetCellStringRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellBitmapRendererRefData, wxSheetCellStringRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellBoolRendererRefData, wxSheetCellRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellDateTimeRendererRefData, wxSheetCellStringRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellEnumRendererRefData, wxSheetCellStringRendererRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellAutoWrapStringRendererRefData, wxSheetCellStringRendererRefData)

IMPLEMENT_DYNAMIC_CLASS(wxSheetCellEditor, wxObject)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellEditorRefData, wxObject)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellTextEditorRefData, wxSheetCellEditorRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellNumberEditorRefData, wxSheetCellTextEditorRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellFloatEditorRefData, wxSheetCellTextEditorRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellBoolEditorRefData, wxSheetCellEditorRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellChoiceEditorRefData, wxSheetCellEditorRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellEnumEditorRefData, wxSheetCellChoiceEditorRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellAutoWrapStringEditorRefData, wxSheetCellTextEditorRefData)

IMPLEMENT_DYNAMIC_CLASS(wxSheetCellRolColLabelRendererRefData, wxSheetCellEditorRefData)

// ----------------------------------------------------------------------------
// wxSheetCellEditorEvtHandler
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( wxSheetCellEditorEvtHandler, wxEvtHandler )

BEGIN_EVENT_TABLE( wxSheetCellEditorEvtHandler, wxEvtHandler )
    EVT_KEY_DOWN       ( wxSheetCellEditorEvtHandler::OnKeyDown )
    EVT_CHAR           ( wxSheetCellEditorEvtHandler::OnChar )
END_EVENT_TABLE()

wxSheetCellEditorEvtHandler::wxSheetCellEditorEvtHandler(wxSheet* sheet, 
                                               const wxSheetCellEditor &editor)
                            : m_sheet(sheet)
{
    m_editor = (wxSheetCellEditorRefData*)editor.GetRefData();
    wxCHECK_RET(m_sheet && m_editor, wxT("Invalid sheet or editor in wxSheetCellEditorEvtHandler"));
}

void wxSheetCellEditorEvtHandler::OnKeyDown(wxKeyEvent& event)
{
    if (!m_editor->OnKeyDown(event))
        return;
    
    switch ( event.GetKeyCode() )
    {
        case WXK_ESCAPE:
        {
            m_editor->Reset();
            m_sheet->DisableCellEditControl(FALSE);
            break;
        }
        case WXK_TAB:
        {
            m_sheet->GetEventHandler()->ProcessEvent( event );
            break;
        }
        case WXK_RETURN:
        case WXK_NUMPAD_ENTER:
        {
            if (!m_sheet->GetEventHandler()->ProcessEvent(event))
                m_editor->HandleReturn(event);
            break;
        }
        default:
            event.Skip();
    }
}

void wxSheetCellEditorEvtHandler::OnChar(wxKeyEvent& event)
{
    if (!m_editor->OnChar(event))
        return;
    
    switch ( event.GetKeyCode() )
    {
        case WXK_ESCAPE:
        case WXK_TAB:
        case WXK_RETURN:
        case WXK_NUMPAD_ENTER:
            break;

        default:
            event.Skip();
    }
}

// ----------------------------------------------------------------------------
// wxSheetCellEditor
// ----------------------------------------------------------------------------

#define M_CELLEDITORDATA ((wxSheetCellEditorRefData*)m_refData)

wxSheetCellEditor::wxSheetCellEditor( wxSheetCellEditorRefData *editor )
{ 
    m_refData = editor; 
}

bool wxSheetCellEditor::IsCreated() const
{
    wxCHECK_MSG(Ok(), FALSE, wxT("wxSheetCellEditor not created"));
    return M_CELLEDITORDATA->IsCreated();
}
bool wxSheetCellEditor::IsShown() const
{
    wxCHECK_MSG(Ok(), FALSE, wxT("wxSheetCellEditor not created"));
    return M_CELLEDITORDATA->IsShown();
}
wxWindow* wxSheetCellEditor::GetControl() const
{
    wxCHECK_MSG(Ok(), NULL, wxT("wxSheetCellEditor not created"));
    return M_CELLEDITORDATA->GetControl();
}
void wxSheetCellEditor::SetControl(wxWindow* control)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->SetControl(control);
}
bool wxSheetCellEditor::HasControl() const
{
    wxCHECK_MSG(Ok(), FALSE, wxT("wxSheetCellEditor not created"));
    return M_CELLEDITORDATA->HasControl();
}
void wxSheetCellEditor::DestroyControl()
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->DestroyControl();
}
void wxSheetCellEditor::CreateEditor(wxWindow* parent, wxWindowID id, wxEvtHandler* evtHandler)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->CreateEditor(parent, id, evtHandler);
}
void wxSheetCellEditor::SetSize(const wxRect& rect, const wxSheetCellAttr &attr)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->SetSize(rect, attr);
}
void wxSheetCellEditor::Show(bool show, const wxSheetCellAttr &attr)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->Show(show, attr);
}
void wxSheetCellEditor::PaintBackground(wxSheet& grid, const wxSheetCellAttr& attr, 
                                        wxDC& dc, const wxRect& rect, 
                                        const wxSheetCoords& coords, bool isSelected)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->PaintBackground(grid, attr, dc, rect, coords, isSelected);
}
void wxSheetCellEditor::BeginEdit(const wxSheetCoords& coords, wxSheet* grid)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->BeginEdit(coords, grid);
}
bool wxSheetCellEditor::EndEdit(const wxSheetCoords& coords, wxSheet* grid)
{
    wxCHECK_MSG(Ok(), FALSE, wxT("wxSheetCellEditor not created"));
    return M_CELLEDITORDATA->EndEdit(coords, grid);
}
void wxSheetCellEditor::Reset()
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->Reset();
}
bool wxSheetCellEditor::IsAcceptedKey(wxKeyEvent& event)
{
    wxCHECK_MSG(Ok(), FALSE, wxT("wxSheetCellEditor not created"));
    return M_CELLEDITORDATA->IsAcceptedKey(event);
}
void wxSheetCellEditor::StartingKey(wxKeyEvent& event)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->StartingKey(event);
}
void wxSheetCellEditor::StartingClick()
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->StartingClick();
}
void wxSheetCellEditor::HandleReturn(wxKeyEvent& event)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->HandleReturn(event);
}
bool wxSheetCellEditor::OnKeyDown(wxKeyEvent& event)
{
    wxCHECK_MSG(Ok(), TRUE, wxT("wxSheetCellEditor not created"));
    return M_CELLEDITORDATA->OnKeyDown(event);
}
bool wxSheetCellEditor::OnChar(wxKeyEvent& event)
{
    wxCHECK_MSG(Ok(), TRUE, wxT("wxSheetCellEditor not created"));
    return M_CELLEDITORDATA->OnChar(event);
}
void wxSheetCellEditor::SetParameters(const wxString& params)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->SetParameters(params);
}
wxString wxSheetCellEditor::GetValue() const
{
    wxCHECK_MSG(Ok(), wxEmptyString, wxT("wxSheetCellEditor not created"));
    return M_CELLEDITORDATA->GetValue();
}
wxString wxSheetCellEditor::GetInitValue() const
{
    wxCHECK_MSG(Ok(), wxEmptyString, wxT("wxSheetCellEditor not created"));
    return M_CELLEDITORDATA->GetInitValue();
}
bool wxSheetCellEditor::Copy(const wxSheetCellEditor& other)
{
    wxCHECK_MSG(other.Ok(), FALSE, wxT("wxSheetCellEditor not created"));
    
    UnRef();
    m_refData = ((wxSheetCellEditorRefData*)other.m_refData)->Clone();
    return TRUE;
}

// ----------------------------------------------------------------------------
// wxSheetCellEditorRefData
// ----------------------------------------------------------------------------
wxSheetCellEditorRefData::~wxSheetCellEditorRefData() 
{
    if (HasControl())
        DestroyControl(); 
}

void wxSheetCellEditorRefData::CreateEditor(wxWindow* WXUNUSED(parent),
                                            wxWindowID WXUNUSED(id),
                                            wxEvtHandler* evtHandler)
{
    if (evtHandler && GetControl())
        GetControl()->PushEventHandler(evtHandler);
}

void wxSheetCellEditorRefData::SetControl(wxWindow* control) 
{ 
    if (m_control)
        DestroyControl();
    
    m_control = control; 
}

void wxSheetCellEditorRefData::DestroyControl() 
{ 
    if (m_control != NULL)
    {
        m_control->PopEventHandler(TRUE); // delete handler
        m_control->Destroy();
        m_control = NULL;
    }
}

void wxSheetCellEditorRefData::PaintBackground(wxSheet& , const wxSheetCellAttr& attr, 
                                               wxDC& dc, const wxRect& rect, 
                                               const wxSheetCoords& , bool )
{
    wxCHECK_RET(IsCreated(), wxT("wxSheetCellEditor not created"));

    // erase the background because we might not fill the cell
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(attr.GetBackgroundColour(), wxSOLID));   
    
    // only draw exactly what's needed
/*  FIXME - the checkbox in GTK doesn't draw it's full background 
    wxRect ctrlRect(GetControl()->GetRect());
    
    // top
    if (ctrlRect.GetTop() > rect.GetTop())
        dc.DrawRectangle(wxRect(rect.x, rect.y, 
                                rect.width, 
                                ctrlRect.GetTop()-rect.GetTop()));
    // bottom
    if (ctrlRect.GetBottom() < rect.GetBottom())
        dc.DrawRectangle(wxRect(rect.x, ctrlRect.GetBottom(), 
                                rect.width, 
                                rect.GetBottom()-ctrlRect.GetBottom()));
    // left
    if (ctrlRect.GetLeft() > rect.GetLeft())
        dc.DrawRectangle(wxRect(rect.x, ctrlRect.y, 
                                ctrlRect.GetLeft() - rect.GetLeft(),
                                ctrlRect.height));
    // right
    if (ctrlRect.GetRight() < rect.GetRight())
        dc.DrawRectangle(wxRect(ctrlRect.GetRight(), ctrlRect.y, 
                                rect.GetRight() - ctrlRect.GetRight(),
                                ctrlRect.height));
*/
    dc.DrawRectangle(rect);
    // redraw the control we just painted over
    GetControl()->Refresh();
}

void wxSheetCellEditorRefData::Show(bool show, const wxSheetCellAttr &attr)
{
    wxCHECK_RET(IsCreated(), wxT("wxSheetCellEditor not created"));

    if ( show )
    {
        // set the colours/fonts if we have any
        if ( attr.Ok() )
        {
            m_colFgOld = GetControl()->GetForegroundColour();
            m_colBgOld = GetControl()->GetBackgroundColour();
            m_fontOld  = GetControl()->GetFont();
            
            GetControl()->SetForegroundColour(attr.GetForegroundColour());

            // FIXME this is weird, in GTK you can't set the background of a textctrl
            // to the default window background colour, ie. label textctrls
            wxColour c = attr.GetBackgroundColour();
            c.Set(wxMin(c.Red()+1, 255), c.Green(), c.Blue());
            GetControl()->SetBackgroundColour(c);
            
            GetControl()->SetFont(attr.GetFont());
        }
    }
    // FIXME - why bother saving default editor colours, will the Attr !Ok ever?
    else
    {
        // restore the standard colours fonts
        if ( m_colFgOld.Ok() )
        {
            GetControl()->SetForegroundColour(m_colFgOld);
            m_colFgOld = wxNullColour;
        }

        if ( m_colBgOld.Ok() )
        {
            GetControl()->SetBackgroundColour(m_colBgOld);
            m_colBgOld = wxNullColour;
        }

        if ( m_fontOld.Ok() )
        {
            GetControl()->SetFont(m_fontOld);
            m_fontOld = wxNullFont;
        }
    }
    
    GetControl()->Show(show);   
}

void wxSheetCellEditorRefData::SetSize(const wxRect& rect, const wxSheetCellAttr &)
{
    wxCHECK_RET(GetControl(), wxT("wxSheetCellEditor not created"));
    GetControl()->SetSize(rect, wxSIZE_ALLOW_MINUS_ONE);
}

bool wxSheetCellEditorRefData::IsAcceptedKey(wxKeyEvent& event)
{
    // accept the simple key presses, not anything with Ctrl/Alt
    return !(event.ControlDown() || event.AltDown());
}

// ----------------------------------------------------------------------------
// wxSheetCellTextEditorRefData
// ----------------------------------------------------------------------------
#if wxUSE_TEXTCTRL

void wxSheetCellTextEditorRefData::CreateEditor(wxWindow* parent,
                                                wxWindowID id,
                                                wxEvtHandler* evtHandler)
{
    SetControl(new wxTextCtrl(parent, id, wxEmptyString,
                              wxDefaultPosition, wxDefaultSize //,wxBORDER_NONE
#if defined(__WXMSW__)
                              , wxTE_PROCESS_TAB | wxTE_AUTO_SCROLL
#endif // defined(__WXMSW__)
                              ));

    // set max length allowed in the textctrl, if the parameter was set
    if (m_maxChars != 0)
        ((wxTextCtrl*)GetControl())->SetMaxLength(m_maxChars);

    wxSheetCellEditorRefData::CreateEditor(parent, id, evtHandler);
}

void wxSheetCellTextEditorRefData::SetSize(const wxRect& rectOrig, const wxSheetCellAttr &attr)
{
    wxCHECK_RET(IsCreated(), wxT("The wxSheetCellEditor must be Created first!"));
    wxRect rect(rectOrig);

    // Put the control inside the grid lines on all sides.
    
#if defined(__WXGTK__)
    // these are good for GTK 1.2, wx ver 2.5
    if (rect.x != 0) rect.x += 1;  
    if (rect.y != 0) rect.y += 1;
        
#elif defined(__WXMSW__) 
    // This works for wxMSW ver 2.5
    if (rect.x != 0) rect.x += 1;  
    if (rect.y != 0) rect.y += 1;

#elif defined(__WXMOTIF__)

    // This is untested ??? 
    int extra_x = ( rect.x > 2 ) ? 2 : 1;
    int extra_y = ( rect.y > 2 ) ? 2 : 1;
    extra_x *= 2;
    extra_y *= 2;
    
    rect.SetLeft( wxMax(0, rect.x - extra_x) );
    rect.SetTop( wxMax(0, rect.y - extra_y) );
    rect.SetRight( rect.GetRight() + 2*extra_x );
    rect.SetBottom( rect.GetBottom() + 2*extra_y );
#endif 

    wxSheetCellEditorRefData::SetSize(rect, attr);
}

void wxSheetCellTextEditorRefData::BeginEdit(const wxSheetCoords& coords, wxSheet* grid)
{
    wxCHECK_RET(IsCreated() && grid, wxT("The wxSheetCellEditor must be Created first!"));

    m_startValue = grid->GetCellValue(coords);
    m_longestValue.Clear();
    DoBeginEdit(m_startValue);
}

void wxSheetCellTextEditorRefData::DoBeginEdit(const wxString& startValue)
{
    wxCHECK_RET(IsCreated(), wxT("The wxSheetCellEditor must be Created first!"));
    Text()->SetValue(startValue);
    Text()->SetInsertionPointEnd();
    Text()->SetSelection(-1,-1);
    Text()->SetFocus();
}

bool wxSheetCellTextEditorRefData::EndEdit(const wxSheetCoords& coords, wxSheet* grid)
{
    wxCHECK_MSG(IsCreated(), FALSE, wxT("The wxSheetCellEditor must be Created first!"));

    wxString value = Text()->GetValue();
    bool changed = (value != m_startValue) ? TRUE : FALSE;

    if (changed)
    {
        if (grid->SendEvent(wxEVT_SHEET_CELL_VALUE_CHANGING, coords) == wxSheet::EVT_VETOED) 
            changed = FALSE;
        else
            grid->SetCellValue(coords, value);
    }

    m_startValue.Clear();
    m_longestValue.Clear();

    return changed;
}

void wxSheetCellTextEditorRefData::Reset()
{
    wxCHECK_RET(IsCreated(), wxT("The wxSheetCellEditor must be Created first!"));
    DoReset(m_startValue);
}

void wxSheetCellTextEditorRefData::DoReset(const wxString& startValue)
{
    wxCHECK_RET(IsCreated(), wxT("The wxSheetCellEditor must be Created first!"));
    Text()->SetValue(startValue);
    Text()->SetInsertionPointEnd();
}

bool wxSheetCellTextEditorRefData::IsAcceptedKey(wxKeyEvent& event)
{
    if ( wxSheetCellEditorRefData::IsAcceptedKey(event) )
    {
        int keycode = event.GetKeyCode();
        switch ( keycode )
        {
            case WXK_NUMPAD0:
            case WXK_NUMPAD1:
            case WXK_NUMPAD2:
            case WXK_NUMPAD3:
            case WXK_NUMPAD4:
            case WXK_NUMPAD5:
            case WXK_NUMPAD6:
            case WXK_NUMPAD7:
            case WXK_NUMPAD8:
            case WXK_NUMPAD9:
            case WXK_MULTIPLY:
            case WXK_NUMPAD_MULTIPLY:
            case WXK_ADD:
            case WXK_NUMPAD_ADD:
            case WXK_SUBTRACT:
            case WXK_NUMPAD_SUBTRACT:
            case WXK_DECIMAL:
            case WXK_NUMPAD_DECIMAL:
            case WXK_DIVIDE:
            case WXK_NUMPAD_DIVIDE:
                return TRUE;

            default:
                // accept 8 bit chars too if isprint() agrees
                if ( (keycode < 255) && (wxIsprint(keycode)) )
                    return TRUE;
        }
    }

    return FALSE;
}

void wxSheetCellTextEditorRefData::StartingKey(wxKeyEvent& event)
{
    wxCHECK_RET(IsCreated(), wxT("The wxSheetCellEditor must be Created first!"));
    if ( !Text()->EmulateKeyPress(event) )
        event.Skip();
}

void wxSheetCellTextEditorRefData::HandleReturn( wxKeyEvent&
                                         WXUNUSED_GTK(WXUNUSED_MOTIF(event)) )
{
    wxCHECK_RET(IsCreated(), wxT("The wxSheetCellEditor must be Created first!"));
    
#if defined(__WXMOTIF__) || defined(__WXGTK__)
    // wxMotif needs a little extra help...
    size_t pos = (size_t)( Text()->GetInsertionPoint() );
    wxString s( Text()->GetValue() );
    s = s.Left(pos) + wxT("\n") + s.Mid(pos);
    Text()->SetValue(s);
    Text()->SetInsertionPoint( pos );
#else
    // the other ports can handle a Return key press
    event.Skip();
#endif // defined(__WXMOTIF__) || defined(__WXGTK__)
}

bool wxSheetCellTextEditorRefData::OnChar( wxKeyEvent& WXUNUSED(event) )
{  
    wxCHECK_MSG(IsCreated(), TRUE, wxT("The wxSheetCellEditor must be Created first!"));
    // This function is used to expand the textctrl as you type
    // it doesn't stop the event, just looks at it
    wxString value = Text()->GetValue();
    if (value.Length() <= m_longestValue.Length())
        return TRUE;

    m_longestValue = value;
    
    wxRect rect(Text()->GetRect());
    int clientWidth = Text()->GetParent()->GetClientSize().x;
    // already at edge of window, maybe the win should should scroll?
    if (rect.GetRight() >= clientWidth)
        return TRUE;
    
    int w, h;
    Text()->GetTextExtent(value, &w, &h, NULL, NULL, &Text()->GetFont());
    
    // can't just use w/value.Len since it might be "iii", too small for "WWW"
    int chW;
    Text()->GetTextExtent(wxT("W"), &chW, &h, NULL, NULL, &Text()->GetFont());
    int extra = int(chW*1.5);
    //if (rect.width < w+extra)
        //Text()->SetSize(wxMin(w+extra, clientWidth-rect.x), rect.height);

    w += extra;
    
    wxSheet* sheet = wxDynamicCast(Text()->GetParent()->GetParent(), wxSheet);
    if (!sheet)
        return TRUE;
    
    wxSheetCoords c(sheet->GetEditControlCoords());
    if (!sheet->GetAttrOverflow(c))
        return TRUE;
    
    wxSheetCoords oneCell(1,1);
    int numCols = sheet->GetNumberCols();
    wxRect scrollRect(sheet->CalcUnscrolledRect(rect));
    for (c.m_col = sheet->XToGridCol(scrollRect.GetRight())+1; c.m_col < numCols; c.m_col++)
    {
        wxSheetCoords cellSize(sheet->GetCellSpan(c));
        // looks weird going over a spanned cell
        if ((rect.width < w) && (rect.GetRight() < clientWidth) && (cellSize == oneCell))
            rect.width += sheet->GetColWidth(c.m_col);
        else
            break;
    }
    
    // clip width to window size
    rect.width = wxMin(rect.width, clientWidth - rect.x);
    Text()->SetSize(rect.width, rect.height);
    
    return TRUE;
}

void wxSheetCellTextEditorRefData::SetParameters(const wxString& params)
{
    if ( !params )
        m_maxChars = 0; // reset to default
    else
    {
        long tmp;
        if ( !params.ToLong(&tmp) )
            wxLogDebug(_T("Invalid wxSheetCellTextEditor parameter string '%s' ignored"), params.c_str());
        else
            m_maxChars = (size_t)tmp;
    }
}

wxString wxSheetCellTextEditorRefData::GetValue() const
{
    wxCHECK_MSG(IsCreated(), wxEmptyString, wxT("The wxSheetCellEditor must be Created first!"));
    return Text()->GetValue();
}

#endif // wxUSE_TEXTCTRL

// ----------------------------------------------------------------------------
// wxSheetCellAutoWrapStringEditorRefData
// ----------------------------------------------------------------------------
#if wxUSE_TEXTCTRL

void wxSheetCellAutoWrapStringEditorRefData::CreateEditor(wxWindow* parent,
                                                          wxWindowID id,
                                                          wxEvtHandler* evtHandler)
{
    SetControl( new wxTextCtrl(parent, id, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE | wxTE_RICH) );

    wxSheetCellEditorRefData::CreateEditor(parent, id, evtHandler);
}
#endif // wxUSE_TEXTCTRL

// ----------------------------------------------------------------------------
// wxSheetCellNumberEditorRefData
// ----------------------------------------------------------------------------
#if defined(wxUSE_TEXTCTRL) && defined(wxUSE_SPINCTRL)

bool wxSheetCellNumberEditorRefData::Copy(const wxSheetCellNumberEditorRefData& other)
{
    m_min = other.m_min;
    m_max = other.m_max;
    return wxSheetCellTextEditorRefData::Copy(other);
}

void wxSheetCellNumberEditorRefData::CreateEditor(wxWindow* parent,
                                                  wxWindowID id,
                                                  wxEvtHandler* evtHandler)
{
    if ( HasRange() )
    {
        // create a spin ctrl
        SetControl( new wxSpinCtrl(parent, -1, wxEmptyString,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_ARROW_KEYS, m_min, m_max) );

        wxSheetCellEditorRefData::CreateEditor(parent, id, evtHandler);
    }
    else
    {
        // just a text control
        wxSheetCellTextEditorRefData::CreateEditor(parent, id, evtHandler);

#if wxUSE_VALIDATORS
        Text()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
#endif // wxUSE_VALIDATORS
    }
}

void wxSheetCellNumberEditorRefData::BeginEdit(const wxSheetCoords& coords, wxSheet* grid)
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    // first get the value
    wxSheetTableBase *table = grid->GetTable();
    if ( table && table->CanGetValueAs(coords, wxSHEET_VALUE_NUMBER) )
    {
        m_valueOld = table->GetValueAsLong(coords);
    }
    else
    {
        m_valueOld = 0;
        wxString sValue = grid->GetCellValue(coords);
        if (!sValue.IsEmpty() && !sValue.ToLong(&m_valueOld))
        {
            wxFAIL_MSG( _T("this cell doesn't have numeric value") );
            return;
        }
    }

    if ( HasRange() )
    {
        Spin()->SetValue((int)m_valueOld);
        Spin()->SetFocus();
    }
    else
    {
        DoBeginEdit(GetInitValue());
    }
}

bool wxSheetCellNumberEditorRefData::EndEdit(const wxSheetCoords& coords, wxSheet* grid)
{
    wxCHECK_MSG(IsCreated(), FALSE, wxT("The wxSheetCellEditor must be Created first!"));
    
    bool changed;
    long value = 0;
    long oldValue = m_valueOld;
    wxString text;
  
    if ( HasRange() )
    {
        value = Spin()->GetValue();
        changed = value != oldValue;
        if (changed)
            text = wxString::Format(wxT("%ld"), value);
    }
    else
    {
        text = Text()->GetValue();
        changed = (text.IsEmpty() || text.ToLong(&value)) && (value != oldValue);
    }

    if ( changed )
    {
        if (grid->SendEvent(wxEVT_SHEET_CELL_VALUE_CHANGING, coords) == wxSheet::EVT_VETOED) 
            return FALSE;
        
        if (grid->GetTable()->CanSetValueAs(coords, wxSHEET_VALUE_NUMBER))
            grid->GetTable()->SetValueAsLong(coords, value);
        else
            grid->GetTable()->SetValue(coords, text);
    }

    return changed;
}

void wxSheetCellNumberEditorRefData::Reset()
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    if ( HasRange() )
        Spin()->SetValue((int)m_valueOld);
    else
        DoReset(GetInitValue());
}

bool wxSheetCellNumberEditorRefData::IsAcceptedKey(wxKeyEvent& event)
{
    if ( wxSheetCellEditorRefData::IsAcceptedKey(event) )
    {
        int keycode = event.GetKeyCode();
        switch ( keycode )
        {
            case WXK_NUMPAD0:
            case WXK_NUMPAD1:
            case WXK_NUMPAD2:
            case WXK_NUMPAD3:
            case WXK_NUMPAD4:
            case WXK_NUMPAD5:
            case WXK_NUMPAD6:
            case WXK_NUMPAD7:
            case WXK_NUMPAD8:
            case WXK_NUMPAD9:
            case WXK_ADD:
            case WXK_NUMPAD_ADD:
            case WXK_SUBTRACT:
            case WXK_NUMPAD_SUBTRACT:
            case WXK_UP:
            case WXK_DOWN:
                return TRUE;

            default:
                if ( (keycode < 128) && wxIsdigit(keycode) )
                    return TRUE;
        }
    }

    return FALSE;
}

void wxSheetCellNumberEditorRefData::StartingKey(wxKeyEvent& event)
{
    if ( !HasRange() )
    {
        int keycode = event.GetKeyCode();
        if ( wxIsdigit(keycode) || keycode == '+' || keycode == '-'
            || keycode ==  WXK_NUMPAD0
            || keycode ==  WXK_NUMPAD1
            || keycode ==  WXK_NUMPAD2
            || keycode ==  WXK_NUMPAD3
            || keycode ==  WXK_NUMPAD4
            || keycode ==  WXK_NUMPAD5
            || keycode ==  WXK_NUMPAD6
            || keycode ==  WXK_NUMPAD7
            || keycode ==  WXK_NUMPAD8
            || keycode ==  WXK_NUMPAD9
            || keycode ==  WXK_ADD
            || keycode ==  WXK_NUMPAD_ADD
            || keycode ==  WXK_SUBTRACT
            || keycode ==  WXK_NUMPAD_SUBTRACT)
        {
            wxSheetCellTextEditorRefData::StartingKey(event);

            // skip Skip() below
            return;
        }
    }

    event.Skip();
}

void wxSheetCellNumberEditorRefData::SetParameters(const wxString& params)
{
    if ( !params )
    {
        m_min = m_max = -1; // reset to default
    }
    else
    {
        long tmp;
        if ( params.BeforeFirst(_T(',')).ToLong(&tmp) )
        {
            m_min = (int)tmp;

            if ( params.AfterFirst(_T(',')).ToLong(&tmp) )
            {
                m_max = (int)tmp;

                // skip the error message below
                return;
            }
        }

        wxLogDebug(_T("Invalid wxSheetCellNumberEditor parameter string '%s' ignored"), params.c_str());
    }
}

// return the value in the spin control if it is there (the text control otherwise)
wxString wxSheetCellNumberEditorRefData::GetValue() const
{
    wxCHECK_MSG(GetControl(), wxEmptyString, wxT("The wxSheetCellEditor must be Created first!"));
    wxString s;

    if ( HasRange() )
    {
        long value = Spin()->GetValue();
        s.Printf(wxT("%ld"), value);
    }
    else
    {
        s = Text()->GetValue();
    }
    return s;
}

wxString wxSheetCellNumberEditorRefData::GetInitValue() const
{ 
    return wxString::Format(_T("%ld"), m_valueOld); 
}

#endif //defined(wxUSE_TEXTCTRL) && defined(wxUSE_SPINCTRL)

// ----------------------------------------------------------------------------
// wxSheetCellFloatEditorRefData
// ----------------------------------------------------------------------------
#if wxUSE_TEXTCTRL

bool wxSheetCellFloatEditorRefData::Copy(const wxSheetCellFloatEditorRefData& other)
{
    m_width = other.m_width;
    m_precision = other.m_precision;
    return wxSheetCellTextEditorRefData::Copy(other);
}

void wxSheetCellFloatEditorRefData::CreateEditor(wxWindow* parent,
                                                 wxWindowID id,
                                                 wxEvtHandler* evtHandler)
{
    wxSheetCellTextEditorRefData::CreateEditor(parent, id, evtHandler);

#if wxUSE_VALIDATORS
    Text()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
#endif // wxUSE_VALIDATORS
}

void wxSheetCellFloatEditorRefData::BeginEdit(const wxSheetCoords& coords, wxSheet* grid)
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    // first get the value
    wxSheetTableBase *table = grid->GetTable();
    if ( table && table->CanGetValueAs(coords, wxSHEET_VALUE_FLOAT) )
    {
        m_valueOld = table->GetValueAsDouble(coords);
    }
    else
    {
        m_valueOld = 0.0;
        wxString sValue = grid->GetCellValue(coords);
        if (!sValue.ToDouble(&m_valueOld) && !sValue.IsEmpty())
        {
            wxFAIL_MSG( _T("this cell doesn't have float value") );
            return;
        }
    }

    DoBeginEdit(GetInitValue());
}

bool wxSheetCellFloatEditorRefData::EndEdit(const wxSheetCoords& coords, wxSheet* grid)
{
    wxCHECK_MSG(IsCreated(), FALSE, wxT("The wxSheetCellEditor must be Created first!"));
    
    double value = 0.0;
    wxString text(Text()->GetValue());

    if ( (text.IsEmpty() || text.ToDouble(&value)) && (value != m_valueOld) )
    {
        if (grid->SendEvent(wxEVT_SHEET_CELL_VALUE_CHANGING, coords) == wxSheet::EVT_VETOED) 
            return FALSE;
        
        if (grid->GetTable()->CanSetValueAs(coords, wxSHEET_VALUE_FLOAT))
            grid->GetTable()->SetValueAsDouble(coords, value);
        else
            grid->GetTable()->SetValue(coords, text);

        return TRUE;
    }
    return FALSE;
}

void wxSheetCellFloatEditorRefData::Reset()
{
    DoReset(GetInitValue());
}

void wxSheetCellFloatEditorRefData::StartingKey(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    
#if 0 && wxUSE_INTL    // FIXME undefined ref to wxLocal::GetInfo ???
    char tmpbuf[2];
    tmpbuf[0] = (char) keycode;
    tmpbuf[1] = '\0';  
    bool is_decimal_point = ( wxString(tmpbuf, *wxConvCurrent) == 
              wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT, wxLOCALE_CAT_NUMBER) );    
#else 
    bool is_decimal_point = (keycode == '.');
#endif // wxUSE_INTL
    
    if (   wxIsdigit(keycode) 
        || keycode == '+' 
        || keycode == '-'  
        || is_decimal_point
        || keycode ==  WXK_NUMPAD0
        || keycode ==  WXK_NUMPAD1
        || keycode ==  WXK_NUMPAD2
        || keycode ==  WXK_NUMPAD3
        || keycode ==  WXK_NUMPAD4
        || keycode ==  WXK_NUMPAD5
        || keycode ==  WXK_NUMPAD6
        || keycode ==  WXK_NUMPAD7
        || keycode ==  WXK_NUMPAD8
        || keycode ==  WXK_NUMPAD9
        || keycode ==  WXK_ADD
        || keycode ==  WXK_NUMPAD_ADD
        || keycode ==  WXK_SUBTRACT
        || keycode ==  WXK_NUMPAD_SUBTRACT)
    {
        wxSheetCellTextEditorRefData::StartingKey(event);
        return; // skip Skip() below
    }

    event.Skip();
}

void wxSheetCellFloatEditorRefData::SetParameters(const wxString& params)
{
    if ( !params )
    {
        m_width = m_precision = -1; // reset to default
    }
    else
    {
        long tmp;
        if ( params.BeforeFirst(_T(',')).ToLong(&tmp) )
        {
            m_width = (int)tmp;

            if ( params.AfterFirst(_T(',')).ToLong(&tmp) )
            {
                m_precision = (int)tmp;
                return; // skip the error message below
            }
        }

        wxLogDebug(_T("Invalid wxSheetCellFloatEditor parameter string '%s' ignored"), params.c_str());
    }
}

wxString wxSheetCellFloatEditorRefData::GetInitValue() const
{
    wxString fmt;
    if ( m_width == -1 )
    {
        // default width/precision
        fmt = _T("%f");
    }
    else if ( m_precision == -1 )
    {
        // default precision
        fmt.Printf(_T("%%%d.f"), m_width);
    }
    else
    {
        fmt.Printf(_T("%%%d.%df"), m_width, m_precision);
    }

    return wxString::Format(fmt, m_valueOld);
}

bool wxSheetCellFloatEditorRefData::IsAcceptedKey(wxKeyEvent& event)
{
    if ( wxSheetCellEditorRefData::IsAcceptedKey(event) )
    {
        int keycode = event.GetKeyCode();
        
        switch ( keycode )
        {
            case WXK_NUMPAD0:
            case WXK_NUMPAD1:
            case WXK_NUMPAD2:
            case WXK_NUMPAD3:
            case WXK_NUMPAD4:
            case WXK_NUMPAD5:
            case WXK_NUMPAD6:
            case WXK_NUMPAD7:
            case WXK_NUMPAD8:
            case WXK_NUMPAD9:
            case WXK_ADD:
            case WXK_NUMPAD_ADD:
            case WXK_SUBTRACT:
            case WXK_NUMPAD_SUBTRACT:
            case WXK_DECIMAL:
            case WXK_NUMPAD_DECIMAL:
                return TRUE;

            default:
                // additionally accept 'e' as in '1e+6', also '-', '+', and '.'
#if 0 && wxUSE_INTL    // FIXME undefined ref to wxLocal::GetInfo ???
                char tmpbuf[2];
                tmpbuf[0] = (char) keycode;
                tmpbuf[1] = '\0';  
                bool is_decimal_point = ( wxString(tmpbuf, *wxConvCurrent) == 
                      wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT, wxLOCALE_CAT_NUMBER) );    
#else 
                bool is_decimal_point = (keycode == '.');
#endif // wxUSE_INTL
                                
                if ( (keycode < 128) &&
                     (wxIsdigit(keycode) || wxTolower(keycode) == 'e' ||
                      is_decimal_point || keycode == '+' || keycode == '-') )
                    return TRUE;
        }
    }

    return FALSE;
}

#endif // wxUSE_TEXTCTRL

// ----------------------------------------------------------------------------
// wxSheetCellBoolEditorRefData
// ----------------------------------------------------------------------------
#if wxUSE_CHECKBOX
/*
class wxSheetCellBoolEditorControl : public wxControl 
{
public:
    wxSheetCellBoolEditorControl(wxWindow *parent, int align) 
        : wxControl(parent, -1), m_align(align) {}
    
    void OnPaint(wxPaintEvent& event)
    {
        
    }

    int m_align;
    DECLARE_EVENT_TABLE(wxSheetCellBoolEditorControl)
}

BEGIN_EVENT_TABLE( wxSheetCellBoolEditorControl, wxControl )
    EVT_PAINT ( wxSheetCellEditorEvtHandler::OnPaint )
END_EVENT_TABLE()
*/

void wxSheetCellBoolEditorRefData::CreateEditor(wxWindow* parent,
                                                wxWindowID id,
                                                wxEvtHandler* evtHandler)
{
    SetControl( new wxCheckBox(parent, id, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize,
                               wxNO_BORDER) );

    wxSheetCellEditorRefData::CreateEditor(parent, id, evtHandler);
}

void wxSheetCellBoolEditorRefData::SetSize(const wxRect& r, const wxSheetCellAttr &attr)
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    bool resize = FALSE;
    wxSize size = GetControl()->GetSize();
    wxCoord minSize = wxMin(r.width, r.height);

    // check if the checkbox is not too big/small for this cell
    wxSize sizeBest = GetControl()->GetBestSize();
    if ( !(size == sizeBest) )
    {
        // reset to default size if it had been made smaller
        size = sizeBest;
        resize = TRUE;
    }

    if ( size.x >= minSize || size.y >= minSize )
    {
        // leave 1 pixel margin
        size.x = size.y = minSize - 2;
        resize = TRUE;
    }

    if ( resize )
    {
        GetControl()->SetSize(size);
    }

    // position it in the centre of the rectangle (TODO: support alignment?)

#if defined(__WXGTK__) || defined (__WXMOTIF__)
    // the checkbox without label still has some space to the right in wxGTK,
    // so shift it to the right
    size.x -= 8;
#elif defined(__WXMSW__)
    // here too, but in other way
    size.x += 1;
    size.y -= 2;
#endif // defined(__WXGTK__) || defined (__WXMOTIF__)

    int align = attr.Ok() ? attr.GetAlignment() : wxALIGN_CENTRE;

    int x = 0, y = 0;
    if ((align & wxALIGN_RIGHT) != 0)
    {
        x = r.x + r.width - size.x - 2;
        y = r.y + r.height/2 - size.y/2;
    }
    else if ((align & wxALIGN_CENTRE_HORIZONTAL) != 0)
    {
        x = r.x + r.width/2 - size.x/2;
        y = r.y + r.height/2 - size.y/2;
    }
    else // wxALIGN_LEFT
    {
        x = r.x + 2;
#ifdef __WXMSW__
        x += 2;
#endif //__WXMSW__
        y = r.y + r.height/2 - size.y/2;
    }

    GetControl()->Move(x, y);
}

void wxSheetCellBoolEditorRefData::Show(bool show, const wxSheetCellAttr &attr)
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    GetControl()->Show(show);

    if ( show )
    {
        wxColour colBg = attr.Ok() ? attr.GetBackgroundColour() : *wxLIGHT_GREY;
        CBox()->SetBackgroundColour(colBg);
    }
}

void wxSheetCellBoolEditorRefData::BeginEdit(const wxSheetCoords& coords, wxSheet* grid)
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));

    if (grid->GetTable()->CanGetValueAs(coords, wxSHEET_VALUE_BOOL))
        m_startValue = grid->GetTable()->GetValueAsBool(coords);
    else
    {
        wxString cellval( grid->GetTable()->GetValue(coords) );
        m_startValue = !( !cellval || (cellval == wxT("0")) );
    }
    CBox()->SetValue(m_startValue);
    CBox()->SetFocus();
}

bool wxSheetCellBoolEditorRefData::EndEdit(const wxSheetCoords& coords, wxSheet* grid)
{
    wxCHECK_MSG(GetControl(), FALSE, wxT("The wxSheetCellEditor must be Created first!"));
    
    bool changed = FALSE;
    bool value = CBox()->GetValue();
    if ( value != m_startValue )
        changed = TRUE;

    if ( changed )
    {
        if (grid->SendEvent(wxEVT_SHEET_CELL_VALUE_CHANGING, coords) == wxSheet::EVT_VETOED) 
            return FALSE;
        
        if (grid->GetTable() && grid->GetTable()->CanGetValueAs(coords, wxSHEET_VALUE_BOOL))
            grid->GetTable()->SetValueAsBool(coords, value);
        else
            grid->SetCellValue(coords, value ? _T("1") : wxEmptyString);
    }

    return changed;
}

void wxSheetCellBoolEditorRefData::Reset()
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    CBox()->SetValue(m_startValue);
}

void wxSheetCellBoolEditorRefData::StartingClick()
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    CBox()->SetValue(!CBox()->GetValue());
}

bool wxSheetCellBoolEditorRefData::IsAcceptedKey(wxKeyEvent& event)
{
    if ( wxSheetCellEditorRefData::IsAcceptedKey(event) )
    {
        int keycode = event.GetKeyCode();
        switch ( keycode )
        {
            case WXK_MULTIPLY:
            case WXK_NUMPAD_MULTIPLY:
            case WXK_ADD:
            case WXK_NUMPAD_ADD:
            case WXK_SUBTRACT:
            case WXK_NUMPAD_SUBTRACT:
            case WXK_SPACE:
            case '+':
            case '-':
                return TRUE;
        }
    }

    return FALSE;
}

// return the value as "1" for true and the empty string for false
wxString wxSheetCellBoolEditorRefData::GetValue() const
{
    wxCHECK_MSG(GetControl(), wxEmptyString, wxT("The wxSheetCellEditor must be Created first!"));
    bool bSet = CBox()->GetValue();
    return bSet ? _T("1") : wxEmptyString;
}

#endif // wxUSE_CHECKBOX

// ----------------------------------------------------------------------------
// wxSheetCellChoiceEditorRefData
// ----------------------------------------------------------------------------
#if wxUSE_COMBOBOX

wxSheetCellChoiceEditorRefData::wxSheetCellChoiceEditorRefData(const wxArrayString& choices,
                                                               bool allowOthers)
                        : m_choices(choices), m_allowOthers(allowOthers) {}

wxSheetCellChoiceEditorRefData::wxSheetCellChoiceEditorRefData(size_t count,
                                                       const wxString choices[],
                                                       bool allowOthers) 
                        : m_allowOthers(allowOthers)
{
    if ( count > 0 )
    {
        m_choices.Alloc(count);
        for ( size_t n = 0; n < count; n++ )
            m_choices.Add(choices[n]);
    }
}

bool wxSheetCellChoiceEditorRefData::Copy(const wxSheetCellChoiceEditorRefData& other)
{
    m_choices = other.m_choices;
    m_allowOthers = other.m_allowOthers;
    return wxSheetCellEditorRefData::Copy(other);
}

void wxSheetCellChoiceEditorRefData::CreateEditor(wxWindow* parent,
                                                  wxWindowID id,
                                                  wxEvtHandler* evtHandler)
{
#if wxMINOR_VERSION > 4
    m_control = new wxComboBox(parent, id, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize,
                               m_choices,
                               m_allowOthers ? 0 : wxCB_READONLY);
#else 
    size_t count = m_choices.GetCount();
    wxString *choices = new wxString[count];
    for ( size_t n = 0; n < count; n++ )
        choices[n] = m_choices[n];

    SetControl( new wxComboBox(parent, id, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize,
                               count, choices,
                               m_allowOthers ? 0 : wxCB_READONLY) );

    delete []choices;
#endif // wxMINOR_VERSION > 4
    wxSheetCellEditorRefData::CreateEditor(parent, id, evtHandler);
}

void wxSheetCellChoiceEditorRefData::PaintBackground(wxSheet& grid, const wxSheetCellAttr& attr, 
                                                     wxDC& dc, const wxRect& rect, 
                                                     const wxSheetCoords& coords, bool isSelected)
{
    // as we fill the entire client area, don't do anything here to minimize
    // flicker

    // TODO: It doesn't actually fill the client area since the height of a
    // combo always defaults to the standard...  Until someone has time to
    // figure out the right rectangle to paint, just do it the normal way...
    wxSheetCellEditorRefData::PaintBackground(grid, attr, dc, rect, coords, isSelected);
}

void wxSheetCellChoiceEditorRefData::BeginEdit(const wxSheetCoords& coords, wxSheet* grid)
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    m_startValue = grid->GetTable()->GetValue(coords);

    if (m_allowOthers)
        Combo()->SetValue(m_startValue);
    else
    {
        // find the right position, or default to the first if not found
        int pos = Combo()->FindString(m_startValue);
        if (pos == -1)
            pos = 0;
        if (Combo()->GetCount() > pos)
            Combo()->SetSelection(pos);
    }
    Combo()->SetInsertionPointEnd();
    Combo()->SetFocus();
}

bool wxSheetCellChoiceEditorRefData::EndEdit(const wxSheetCoords& coords, wxSheet* grid)
{
    wxCHECK_MSG(GetControl(), FALSE, wxT("The wxSheetCellEditor must be Created first!"));
    wxString value = Combo()->GetValue();
    bool changed = value != m_startValue;

    if ( changed )
    {
        if (grid->SendEvent(wxEVT_SHEET_CELL_VALUE_CHANGING, coords) == wxSheet::EVT_VETOED) 
            return FALSE;
        
        grid->GetTable()->SetValue(coords, value);
    }

    m_startValue.Clear();
    if (m_allowOthers)
        Combo()->SetValue(m_startValue);
    else
        Combo()->SetSelection(0);

    return changed;
}

void wxSheetCellChoiceEditorRefData::Reset()
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    Combo()->SetValue(m_startValue);
    Combo()->SetInsertionPointEnd();
}

void wxSheetCellChoiceEditorRefData::SetParameters(const wxString& params)
{
    // what can we do?
    if ( !params )
        return;

    m_choices.Empty();

    wxStringTokenizer tk(params, _T(','));
    while ( tk.HasMoreTokens() )
        m_choices.Add(tk.GetNextToken());
}

// return the value in the text control
wxString wxSheetCellChoiceEditorRefData::GetValue() const
{
    wxCHECK_MSG(GetControl(), wxEmptyString, wxT("The wxSheetCellEditor must be Created first!"));
    return Combo()->GetValue();
}
#endif // wxUSE_COMBOBOX

// ----------------------------------------------------------------------------
// wxSheetCellEnumEditorRefData
// ----------------------------------------------------------------------------
#if wxUSE_COMBOBOX

// A cell editor which displays an enum number as a textual equivalent. eg
// data in cell is 0,1,2 ... n the cell could be displayed as
// "John","Fred"..."Bob" in the combo choice box

wxSheetCellEnumEditorRefData::wxSheetCellEnumEditorRefData(const wxString& choices)
                             : wxSheetCellChoiceEditorRefData(), m_startint(-1)
{
    if (!choices.IsEmpty())
        SetParameters(choices);
}

void wxSheetCellEnumEditorRefData::BeginEdit(const wxSheetCoords& coords, wxSheet* grid)
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEnumEditor must be Created first!"));
    wxSheetTableBase *table = grid->GetTable();

    if ( table && table->CanGetValueAs(coords, wxSHEET_VALUE_NUMBER) )
        m_startint = table->GetValueAsLong(coords);
    else
    {
        wxString startValue = grid->GetCellValue(coords);
        if (startValue.IsEmpty() || !startValue.ToLong(&m_startint))
            m_startint = -1;
    }

    Combo()->SetSelection(m_startint);
    Combo()->SetInsertionPointEnd();
    Combo()->SetFocus();
}

bool wxSheetCellEnumEditorRefData::EndEdit(const wxSheetCoords& coords, wxSheet* grid)
{
    wxCHECK_MSG(GetControl(), FALSE, wxT("The wxSheetCellEditor must be Created first!"));
    int pos = Combo()->GetSelection();
    bool changed = (pos != m_startint);
    if (changed)
    {
        if (grid->SendEvent(wxEVT_SHEET_CELL_VALUE_CHANGING, coords) == wxSheet::EVT_VETOED) 
            return FALSE;
        
        if (grid->GetTable() && grid->GetTable()->CanSetValueAs(coords, wxSHEET_VALUE_NUMBER))
            grid->GetTable()->SetValueAsLong(coords, pos);
        else
            grid->SetCellValue(coords, wxString::Format(wxT("%i"),pos));
    }

    return changed;
}

#endif // wxUSE_COMBOBOX

// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************
// ****************************************************************************

// ----------------------------------------------------------------------------
// wxSheetCellRenderer
// ----------------------------------------------------------------------------
#define M_CELLRENREFDATA ((wxSheetCellRendererRefData*)m_refData)

wxSheetCellRenderer::wxSheetCellRenderer(wxSheetCellRendererRefData *renderer)
{ 
    m_refData = renderer; 
}

void wxSheetCellRenderer::Draw(wxSheet& grid, const wxSheetCellAttr& attr,
                               wxDC& dc, const wxRect& rect,
                               const wxSheetCoords& coords, bool isSelected)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellRenderer is not created"));    
    M_CELLRENREFDATA->Draw(grid, attr, dc, rect, coords, isSelected);
}

wxSize wxSheetCellRenderer::GetBestSize(wxSheet& grid, const wxSheetCellAttr& attr,
                                        wxDC& dc, const wxSheetCoords& coords) 
{ 
    wxCHECK_MSG(Ok(), wxSize(grid.GetDefaultColWidth(), grid.GetDefaultRowHeight()), 
                wxT("wxSheetCellRenderer is not created"));
    return M_CELLRENREFDATA->GetBestSize(grid, attr, dc, coords);
}

void wxSheetCellRenderer::SetParameters(const wxString& params)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellRenderer is not created"));
    M_CELLRENREFDATA->SetParameters(params);
}

bool wxSheetCellRenderer::Copy(const wxSheetCellRenderer& other)
{
    wxCHECK_MSG(other.Ok(), FALSE, wxT("wxSheetCellRenderer is not created"));
    
    UnRef();
    m_refData = ((wxSheetCellRendererRefData*)other.m_refData)->Clone();
    return TRUE;
}

// ----------------------------------------------------------------------------
// wxSheetCellRendererRefData
// ----------------------------------------------------------------------------

// This is testing code to show the different selection blocks with different colours
//#define TEST_SELECTION_BLOCKS

#ifdef TEST_SELECTION_BLOCKS
    #include "sheet/sheetsel.h"
    #include "math.h"
    #define UNUSE(a) a
#else
    #define UNUSE(a) 
#endif

void wxSheetCellRendererRefData::Draw( wxSheet& grid, 
                                       const wxSheetCellAttr& attr,
                                       wxDC& dc, 
                                       const wxRect& rect,
                                       const wxSheetCoords& UNUSE(coords),
                                       bool isSelected )
{
    dc.SetBackgroundMode( wxSOLID );

    // grey out fields if the grid is disabled
    if ( grid.IsEnabled() )
    {
        if ( isSelected )
        {
#ifdef TEST_SELECTION_BLOCKS // really bad colouring for identifying different blocks
            int i = grid.GetSelection()->Index(coords);
            wxColour c(abs(sin(i/10.0)*255), abs(sin(.5+i/20.0)*255), 128);
            dc.SetBrush( wxBrush(c, wxSOLID) );
#else
            dc.SetBrush( wxBrush(grid.GetSelectionBackground(), wxSOLID) );
#endif
        }
        else
            dc.SetBrush( wxBrush(attr.GetBackgroundColour(), wxSOLID) );
    }
    else
        dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE), wxSOLID));

    dc.SetPen( *wxTRANSPARENT_PEN );
    dc.DrawRectangle(rect);
    
    //FIXME - border drawing code, maybe it goes here?
    //dc.SetPen( wxPen(grid.GetGridLineColour(), 1, wxSOLID) );
    //dc.DrawRectangle(rect.x-1, rect.y-1, rect.width+2, rect.height+2);
}

wxSize wxSheetCellRendererRefData::GetBestSize(wxSheet& grid, 
                                               const wxSheetCellAttr& WXUNUSED(attr),
                                               wxDC& WXUNUSED(dc), 
                                               const wxSheetCoords& coords) 
{ 
    return wxSize( grid.GetColWidth(coords.m_col), 
                   grid.GetRowHeight(coords.m_row) );
} 

// ----------------------------------------------------------------------------
// wxSheetCellStringRendererRefData
// ----------------------------------------------------------------------------
wxSheetCellStringRendererRefData::wxSheetCellStringRendererRefData() 
{
    if (!s_overflowBitmap.Ok())
        s_overflowBitmap = wxBitmap(s_overflow_arrow_xpm_data);
}

void wxSheetCellStringRendererRefData::SetTextColoursAndFont(wxSheet& grid,
                                                     const wxSheetCellAttr& attr,
                                                     wxDC& dc,
                                                     bool isSelected)
{
    dc.SetBackgroundMode( wxTRANSPARENT );

    // TODO some special colours for attr.IsReadOnly() case?

    // different coloured text when the grid is disabled
    if ( grid.IsEnabled() )
    {
        if ( isSelected )
        {
            dc.SetTextBackground( grid.GetSelectionBackground() );
            dc.SetTextForeground( grid.GetSelectionForeground() );
        }
        else
        {
            dc.SetTextBackground( attr.GetBackgroundColour() );
            dc.SetTextForeground( attr.GetForegroundColour() );
        }
    }
    else
    {
        dc.SetTextBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
        dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }
    
    dc.SetFont( attr.GetFont() );
}

wxSize wxSheetCellStringRendererRefData::DoGetBestSize(wxSheet& grid,
                                                       const wxSheetCellAttr& attr,
                                                       wxDC& dc,
                                                       const wxString& text)
{
    wxArrayString lines;
    long w=0, h=0;
    if (grid.StringToLines(text, lines) > 0)
    {
        dc.SetFont(attr.GetFont());
        grid.GetTextBoxSize(dc, lines, &w, &h);
    }
    
    int o = attr.GetOrientation();
    return wxSize((o==wxHORIZONTAL) ? w : h, (o==wxHORIZONTAL) ? h : w);
}

wxSize wxSheetCellStringRendererRefData::GetBestSize(wxSheet& grid,
                                                     const wxSheetCellAttr& attr,
                                                     wxDC& dc,
                                                     const wxSheetCoords& coords)
{
    return DoGetBestSize(grid, attr, dc, grid.GetCellValue(coords));
}

void wxSheetCellStringRendererRefData::Draw(wxSheet& grid,
                                            const wxSheetCellAttr& attr,
                                            wxDC& dc,
                                            const wxRect& rectCell,
                                            const wxSheetCoords& coords,
                                            bool isSelected)
{    
    wxRect rect = rectCell;
    rect.Inflate(-1);

    // erase only this cells background, overflow cells should have been erased
    wxSheetCellRendererRefData::Draw(grid, attr, dc, rectCell, coords, isSelected);
    DoDraw(grid, attr, dc, rectCell, coords, isSelected);
}

void wxSheetCellStringRendererRefData::DoDraw(wxSheet& grid,
                                              const wxSheetCellAttr& attr,
                                              wxDC& dc,
                                              const wxRect& rectCell,
                                              const wxSheetCoords& coords,
                                              bool isSelected)
{        
    wxRect rect = rectCell;
    rect.Inflate(-1);
    
    int align = attr.GetAlignment();

    wxString value = grid.GetCellValue(coords);
    int best_width = DoGetBestSize(grid, attr, dc, value).GetWidth();
    wxSheetCoords cellSpan(grid.GetCellSpan(coords)); // shouldn't get here if <=0
    int cell_rows = cellSpan.m_row;
    int cell_cols = cellSpan.m_col;
    
    bool is_grid_cell = wxSheet::IsGridCell(coords);
    // no overflow for row/col/corner labels
    bool overflow = is_grid_cell ? attr.GetOverflow() : FALSE;
    int overflowCols = 0;
    int num_cols = grid.GetNumberCols();
    // this is the right col which includes overflow
    int rightCol = coords.m_col + cell_cols - 1; 
    
    if (overflow)
    {
        bool is_editing = grid.IsCellEditControlShown();
        wxSheetCoords editorCell = is_editing ? grid.GetEditControlCoords() : wxNullSheetCoords;
        int row = coords.GetRow(), col = coords.GetCol();
        wxSheetCoords ownerCell;
        if ((best_width > rectCell.width) && (col < num_cols-1) && grid.GetTable())
        {
            wxSheetCoords cell;
            for (cell.m_col = col+cell_cols; cell.m_col < num_cols; cell.m_col++)
            {
                bool is_empty = TRUE;
                for (cell.m_row = row; cell.m_row < row+cell_rows; cell.m_row++)
                {
                    ownerCell = grid.GetCellOwner(cell);
                    // check w/ anchor cell for spanned cell block
                    if ( !grid.GetTable()->IsEmptyCell(ownerCell) || 
                         (ownerCell == editorCell) )
                    {
                        is_empty = FALSE;
                        break;
                    }
                }
                
                if (is_empty)
                    rect.width += grid.GetColWidth(cell.m_col);
                else
                {
                    cell.m_col--;
                    break;
                }
                
                if (rect.width >= best_width) 
                    break;
            }
            // this may extend out of grid
            overflowCols = cell.m_col - col - cell_cols + 1;
            rightCol = wxMin(coords.m_col+cell_cols-1+overflowCols, num_cols - 1);
        }

        // redraw overflow cells individually for proper selection hilight
        if (overflowCols > 0) 
        {
            // if overflowed then it's left aligned (yes I know ALIGN_LEFT=0)
            align &= ~wxSHEET_AttrAlignHoriz_Mask;
            align |= wxSHEET_AttrAlignLeft;    

            wxRect clip(rect);
            clip.x += rectCell.width;

            int col_width;
            wxSheetCoords cell(coords);
            // draw each cell individually since it may be selected or not
            for (cell.m_col = col+cell_cols; cell.m_col <= rightCol; cell.m_col++)
            {
                col_width = grid.GetColWidth(cell.m_col);
                clip.width = col_width - 1;
                dc.DestroyClippingRegion();
                dc.SetClippingRegion(clip);
                SetTextColoursAndFont(grid, attr, dc, grid.IsCellSelected(cell));
                grid.DrawTextRectangle(dc, value, rect, align);
                clip.x += col_width - 1;
            }

            rect = rectCell;
            rect.Inflate(-1);
            rect.width++;
            dc.DestroyClippingRegion();
        }
    }

    // Draw the text 
    SetTextColoursAndFont(grid, attr, dc, isSelected);
    grid.DrawTextRectangle(dc, value, rect, align);

    if (attr.GetOverflowMarker())
    {        
        // Draw a marker to show that the contents has been clipped off
        int cellRight = grid.GetColRight(rightCol);
        if (cellRight - rect.x < best_width)
        {
            int bmpWidth   = s_overflowBitmap.GetWidth();
            int bmpHeight  = s_overflowBitmap.GetHeight();
            int cellWidth  = grid.GetColWidth(rightCol);
            int cellHeight = grid.GetRowHeight(coords.m_row);
        
            if ((bmpWidth < cellWidth-3) && (bmpHeight < cellHeight-3))
            {
                int cellTop = grid.GetRowTop(coords.m_row);
            
                int x = cellRight - bmpWidth - 2;
                int y = cellTop + (cellHeight - bmpHeight)/2;
                wxRect r(x-2, cellTop, bmpWidth+4-1, cellHeight-1);
                wxSheetCellAttr rightAttr(attr);
                if (overflowCols > 0)
                {
                    wxSheetCoords clipCell(coords.m_row, rightCol);
                    isSelected = grid.IsCellSelected(clipCell);
                    rightAttr = grid.GetAttr(clipCell);
                }
            
                // clear background for bitmap
                wxSheetCellRendererRefData::Draw(grid, rightAttr, dc, r, coords, isSelected);
                dc.DrawBitmap( s_overflowBitmap, x, y, TRUE );
            }
        }
    }
}

// ----------------------------------------------------------------------------
// wxSheetCellNumberRendererRefData
// ----------------------------------------------------------------------------

wxString wxSheetCellNumberRendererRefData::GetString(wxSheet& grid, const wxSheetCoords& coords)
{
    wxSheetTableBase *table = grid.GetTable();
    if ( table && table->CanGetValueAs(coords, wxSHEET_VALUE_NUMBER) )
        return wxString::Format(_T("%ld"), table->GetValueAsLong(coords));

    return grid.GetCellValue(coords);
}

void wxSheetCellNumberRendererRefData::Draw(wxSheet& grid,
                                            const wxSheetCellAttr& attr,
                                            wxDC& dc,
                                            const wxRect& rectCell,
                                            const wxSheetCoords& coords,
                                            bool isSelected)
{
    wxSheetCellRendererRefData::Draw(grid, attr, dc, rectCell, coords, isSelected);

    SetTextColoursAndFont(grid, attr, dc, isSelected);

    // draw the text right aligned by default 
    int align = attr.GetAlignment(); // | wxALIGN_RIGHT;  //FIXME Why forced right?

    wxRect rect = rectCell;
    rect.Inflate(-1);

    grid.DrawTextRectangle(dc, GetString(grid, coords), rect, align);
}

wxSize wxSheetCellNumberRendererRefData::GetBestSize(wxSheet& grid,
                                                     const wxSheetCellAttr& attr,
                                                     wxDC& dc,
                                                     const wxSheetCoords& coords)
{
    return DoGetBestSize(grid, attr, dc, GetString(grid, coords));
}

// ----------------------------------------------------------------------------
// wxSheetCellFloatRendererRefData
// ----------------------------------------------------------------------------
bool wxSheetCellFloatRendererRefData::Copy(const wxSheetCellFloatRendererRefData &other)
{
    SetWidth(other.GetWidth());
    SetPrecision(other.GetPrecision());    
    return wxSheetCellStringRendererRefData::Copy(other);
}

wxString wxSheetCellFloatRendererRefData::GetString(wxSheet& grid, const wxSheetCoords& coords)
{
    wxSheetTableBase *table = grid.GetTable();

    bool hasDouble = FALSE;
    double val = 0;
    wxString text;
    if ( table && table->CanGetValueAs(coords, wxSHEET_VALUE_FLOAT) )
    {
        val = table->GetValueAsDouble(coords);
        hasDouble = TRUE;
    }
    else
    {
        text = grid.GetCellValue(coords);
        hasDouble = text.ToDouble(&val);
    }

    if ( hasDouble )
    {
        if ( !m_format )
        {
            if ( m_width < 0 )
            {
                if ( m_precision < 0 )
                    m_format = _T("%f"); // default width/precision
                else
                    m_format.Printf(_T("%%.%df"), m_precision);
            }
            else if ( m_precision < 0 )
                m_format.Printf(_T("%%%d.f"), m_width); // default precision
            else
                m_format.Printf(_T("%%%d.%df"), m_width, m_precision);
        }

        text.Printf(m_format, val);
    }

    return text;    
}

void wxSheetCellFloatRendererRefData::Draw( wxSheet& grid,
                                            const wxSheetCellAttr& attr,
                                            wxDC& dc,
                                            const wxRect& rectCell,
                                            const wxSheetCoords& coords,
                                            bool isSelected )
{
    wxSheetCellRendererRefData::Draw(grid, attr, dc, rectCell, coords, isSelected);

    SetTextColoursAndFont(grid, attr, dc, isSelected);

    // draw the text right aligned by default  -- FIXME ? WHY
    int align = attr.GetAlignment(); // | wxALIGN_RIGHT;     

    wxRect rect = rectCell;
    rect.Inflate(-1);

    grid.DrawTextRectangle(dc, GetString(grid, coords), rect, align);
}

wxSize wxSheetCellFloatRendererRefData::GetBestSize(wxSheet& grid,
                                                    const wxSheetCellAttr& attr,
                                                    wxDC& dc,
                                                    const wxSheetCoords& coords)
{
    return DoGetBestSize(grid, attr, dc, GetString(grid, coords));
}

void wxSheetCellFloatRendererRefData::SetParameters(const wxString& params)
{
    if ( !params )
    {
        // reset to defaults
        SetWidth(-1);
        SetPrecision(-1);
    }
    else
    {
        wxString tmp = params.BeforeFirst(_T(','));
        if ( !tmp.IsEmpty() )
        {
            long width;
            if ( tmp.ToLong(&width) )
                SetWidth((int)width);
            else
                wxLogDebug(_T("Invalid wxSheetCellFloatRenderer width parameter string '%s ignored"), params.c_str());
        }
        tmp = params.AfterFirst(_T(','));
        if ( !tmp.IsEmpty() )
        {
            long precision;
            if ( tmp.ToLong(&precision) )
                SetPrecision((int)precision);
            else
                wxLogDebug(_T("Invalid wxSheetCellFloatRenderer precision parameter string '%s ignored"), params.c_str());
        }
    }
}


// ----------------------------------------------------------------------------
// wxSheetCellBitmapRendererRefData
// ----------------------------------------------------------------------------

wxSize wxSheetCellBitmapRendererRefData::GetBestSize(wxSheet& grid,
                                                     const wxSheetCellAttr& attr,
                                                     wxDC& dc,
                                                     const wxSheetCoords& coords)
{
    wxSize bmpSize;
    if (m_bitmap.Ok())
        bmpSize = wxSize( m_bitmap.GetWidth(), m_bitmap.GetHeight() );
    
    wxSize strSize = wxSheetCellStringRendererRefData::GetBestSize(grid, attr, dc, coords);
   
    return wxSize(bmpSize.x + strSize.x + 5, wxMax(bmpSize.y, strSize.y));
}

void wxSheetCellBitmapRendererRefData::Draw(wxSheet& grid,
                                            const wxSheetCellAttr& attr,
                                            wxDC& dc, const wxRect& rect,
                                            const wxSheetCoords& coords,
                                            bool isSelected)
{
    wxSheetCellRendererRefData::Draw(grid, attr, dc, rect, coords, isSelected);

    int align = attr.GetAlignment();    
    wxSize bestSize = GetBestSize(grid, attr, dc, coords);
    wxRect bestRect(grid.AlignInRect(align, rect, bestSize), bestSize);
    //bestRect.Intersect(rect);
    
    wxRect bmpRect(0, 0, m_bitmap.GetWidth(), m_bitmap.GetHeight());
    wxRect textRect(bestRect);    
    
    //wxPrintf(wxT("Align %d\n"), align);
    
    if (m_bitmap.Ok())
    {
        bmpRect.width  = m_bitmap.GetWidth();
        bmpRect.height = m_bitmap.GetHeight();
        bmpRect.SetPosition(grid.AlignInRect(align, bestRect, bmpRect.GetSize()));
        bmpRect.y += bestRect.height - bmpRect.height;
        textRect.width -= bmpRect.width + 1;
        
        if ((align & wxALIGN_RIGHT) != 0)
        {
            
        }
        else
        {
            textRect.x = bmpRect.GetRight();
        }
    }
    
    wxSheetCellStringRendererRefData::DoDraw(grid, attr, dc, textRect, coords, isSelected);
    
    if (m_bitmap.Ok())
    {
        dc.SetClippingRegion(rect);
        dc.DrawBitmap(m_bitmap, bmpRect.x, bmpRect.y, TRUE);
        dc.DestroyClippingRegion();
    }
}

// ----------------------------------------------------------------------------
// wxSheetCellBoolRendererRefData
// ----------------------------------------------------------------------------

wxSize wxSheetCellBoolRendererRefData::ms_sizeCheckMark;

// FIXME these checkbox size calculations are really ugly...

// between checkmark and box
#define wxSHEET_CHECKMARK_MARGIN 2

wxSize wxSheetCellBoolRendererRefData::GetBestSize(wxSheet& grid,
                                                   const wxSheetCellAttr& WXUNUSED(attr),
                                                   wxDC& WXUNUSED(dc),
                                                   const wxSheetCoords& )
{
    // compute it only once (no locks for MT safeness in GUI thread...)
    if ( !ms_sizeCheckMark.x )
    {
        // get checkbox size
        wxCheckBox *checkbox = new wxCheckBox(&grid, -1, wxEmptyString);
        wxSize size = checkbox->GetBestSize();
        wxCoord checkSize = size.y + 2*wxSHEET_CHECKMARK_MARGIN;

        // FIXME wxGTK::wxCheckBox::GetBestSize() gives "wrong" result
#if defined(__WXGTK__) || defined(__WXMOTIF__)
        checkSize -= size.y / 2;
#endif // defined(__WXGTK__) || defined(__WXMOTIF__)

        delete checkbox;
        ms_sizeCheckMark.x = ms_sizeCheckMark.y = checkSize;
    }

    return ms_sizeCheckMark;
}

void wxSheetCellBoolRendererRefData::Draw(wxSheet& grid,
                                          const wxSheetCellAttr& attr,
                                          wxDC& dc, const wxRect& rect,
                                          const wxSheetCoords& coords,
                                          bool isSelected)
{
    wxSheetCellRendererRefData::Draw(grid, attr, dc, rect, coords, isSelected);

    // draw a check mark in the centre (ignoring alignment - TODO)
    wxSize size = GetBestSize(grid, attr, dc, coords);

    // don't draw outside the cell
    wxCoord minSize = wxMin(rect.width, rect.height);
    if ((size.x >= minSize) || (size.y >= minSize))
        size.x = size.y = minSize - 2; // leave (at least) 1 pixel margin

    // draw a border around checkmark
    int align = attr.GetAlignment();

    wxRect rectBorder(rect.GetPosition(), size);
    
    if ((align & wxALIGN_RIGHT) != 0)
        rectBorder.x += rect.width - size.x - 2;
    else if ((align & wxALIGN_CENTRE_HORIZONTAL) != 0)
        rectBorder.x += rect.width/2 - size.x/2;
    else // wxALIGN_LEFT
        rectBorder.x += 2;

    if ((align & wxALIGN_BOTTOM) != 0)
        rectBorder.y += rect.height - size.y - 2;
    else if ((align & wxALIGN_CENTRE_VERTICAL) != 0)
        rectBorder.y += rect.height/2 - size.y/2;
    else // wxALIGN_TOP
        rectBorder.y += 2;

    bool value;
    if ( grid.GetTable()->CanGetValueAs(coords, wxSHEET_VALUE_BOOL) )
        value = grid.GetTable()->GetValueAsBool(coords);
    else
    {
        wxString cellval( grid.GetTable()->GetValue(coords) );
        value = !( !cellval || (cellval == wxT("0")) ); // FIXME yikes!
    }

    if ( value )
    {
        wxRect rectMark = rectBorder;
#ifdef __WXMSW__
        // MSW DrawCheckMark() is weird (and should probably be changed...)
        rectMark.Inflate(-wxSHEET_CHECKMARK_MARGIN/2);
        rectMark.x++;
        rectMark.y++;
#else // !MSW
        rectMark.Inflate(-wxSHEET_CHECKMARK_MARGIN);
#endif // MSW/!MSW

        dc.SetTextForeground(attr.GetForegroundColour());
        dc.DrawCheckMark(rectMark);
    }

    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(wxPen(attr.GetForegroundColour(), 1, wxSOLID));
    dc.DrawRectangle(rectBorder);
}

// ----------------------------------------------------------------------------
// wxSheetCellDateTimeRendererRefData
// ----------------------------------------------------------------------------
#if wxUSE_DATETIME

// Enables a grid cell to display a formated date and or time
bool wxSheetCellDateTimeRendererRefData::Copy(const wxSheetCellDateTimeRendererRefData& other)
{
    m_outFormat = other.m_outFormat;
    m_inFormat  = other.m_inFormat;
    m_dateTime  = other.m_dateTime;
    m_tz        = other.m_tz;
    return wxSheetCellStringRendererRefData::Copy(other);
}

wxString wxSheetCellDateTimeRendererRefData::GetString(wxSheet& grid, const wxSheetCoords& coords)
{
    wxSheetTableBase *table = grid.GetTable();

    bool hasDatetime = FALSE;
    wxDateTime val;
    wxString text;
    if ( table && table->CanGetValueAs(coords, wxSHEET_VALUE_DATETIME) )
    {
        void* tempval = table->GetValueAsCustom(coords, wxSHEET_VALUE_DATETIME);

        if (tempval)
        {
            val = *((wxDateTime *)tempval);
            hasDatetime = TRUE;
            delete (wxDateTime *)tempval;
        }
    }

    if ( !hasDatetime )
    {
        text = grid.GetCellValue(coords);
        hasDatetime = val.ParseFormat(text, m_inFormat, m_dateTime) != (wxChar *)NULL ;
    }
    else
        text = val.Format( m_outFormat, m_tz );

    return text; //If we faild to parse string just show what we where given?
}

void wxSheetCellDateTimeRendererRefData::Draw(wxSheet& grid,
                                              wxSheetCellAttr& attr,
                                              wxDC& dc,
                                              const wxRect& rectCell,
                                              const wxSheetCoords& coords,
                                              bool isSelected)
{
    wxSheetCellRendererRefData::Draw(grid, attr, dc, rectCell, coords, isSelected);

    SetTextColoursAndFont(grid, attr, dc, isSelected);

    // draw the text right aligned by default FIXME why?
    int align = attr.GetAlignment(); // | wxRIGHT; 

    wxRect rect = rectCell;
    rect.Inflate(-1);

    grid.DrawTextRectangle(dc, GetString(grid, coords), rect, align);
}

wxSize wxSheetCellDateTimeRendererRefData::GetBestSize(wxSheet& grid,
                                                       wxSheetCellAttr& attr,
                                                       wxDC& dc,
                                                       const wxSheetCoords& coords)
{
    return DoGetBestSize(grid, attr, dc, GetString(grid, coords));
}

void wxSheetCellDateTimeRendererRefData::SetParameters(const wxString& params)
{
    if (!params.IsEmpty())
        m_outFormat = params;
}

#endif // wxUSE_DATETIME

// ----------------------------------------------------------------------------
// wxSheetCellEnumRendererRefData
// ----------------------------------------------------------------------------
// Renders a number as a textual equivalent.
// eg data in cell is 0,1,2 ... n the cell could be rendered as "John","Fred"..."Bob"

wxSheetCellEnumRendererRefData::wxSheetCellEnumRendererRefData(const wxString& choices)
{
    if (!choices.IsEmpty())
        SetParameters(choices);
}

bool wxSheetCellEnumRendererRefData::Copy(const wxSheetCellEnumRendererRefData& other)
{
    m_choices = other.m_choices;
    return wxSheetCellStringRendererRefData::Copy(other);
}

wxString wxSheetCellEnumRendererRefData::GetString( wxSheet& grid, 
                                                    const wxSheetCoords& coords )
{
    wxSheetTableBase *table = grid.GetTable();
    if ( table && table->CanGetValueAs(coords, wxSHEET_VALUE_NUMBER) )
    {
        int choiceno = table->GetValueAsLong(coords);
        return m_choices[choiceno];
    }

    return grid.GetCellValue(coords);
}

void wxSheetCellEnumRendererRefData::Draw( wxSheet& grid,
                                           wxSheetCellAttr& attr,
                                           wxDC& dc,
                                           const wxRect& rectCell,
                                           const wxSheetCoords& coords,
                                           bool isSelected )
{
    wxSheetCellRendererRefData::Draw(grid, attr, dc, rectCell, coords, isSelected);
    SetTextColoursAndFont(grid, attr, dc, isSelected);
    
    // draw the text right aligned by default FIXME why?
    int align = attr.GetAlignment(); // | wxRIGHT;     

    wxRect rect = rectCell;
    rect.Inflate(-1);

    grid.DrawTextRectangle(dc, GetString(grid, coords), rect, align);
}

wxSize wxSheetCellEnumRendererRefData::GetBestSize( wxSheet& grid,
                                                    wxSheetCellAttr& attr,
                                                    wxDC& dc,
                                                    const wxSheetCoords& coords)
{
    return DoGetBestSize(grid, attr, dc, GetString(grid, coords));
}

void wxSheetCellEnumRendererRefData::SetParameters(const wxString& params)
{
    if ( !params )
        return; // what can we do?

    m_choices.Empty();

    wxStringTokenizer tk(params, _T(','));
    while ( tk.HasMoreTokens() )
        m_choices.Add(tk.GetNextToken());
}

// ----------------------------------------------------------------------------
// wxSheetCellAutoWrapStringRendererRefData
// ----------------------------------------------------------------------------

void wxSheetCellAutoWrapStringRendererRefData::Draw(wxSheet& grid,
                                                    wxSheetCellAttr& attr,
                                                    wxDC& dc,
                                                    const wxRect& rectCell,
                                                    const wxSheetCoords& coords,
                                                    bool isSelected) 
{
    wxSheetCellRendererRefData::Draw(grid, attr, dc, rectCell, coords, isSelected);
    SetTextColoursAndFont(grid, attr, dc, isSelected);

    int align = attr.GetAlignment();

    wxRect rect = rectCell;
    rect.Inflate(-1);

    grid.DrawTextRectangle(dc, GetTextLines(grid, dc, attr, rect, coords),
                           rect, align);
}

wxArrayString
wxSheetCellAutoWrapStringRendererRefData::GetTextLines(wxSheet& grid,
                                                       wxDC& dc,
                                                       wxSheetCellAttr& attr,
                                                       const wxRect& rect,
                                                       const wxSheetCoords& coords)
{
    wxString data = grid.GetCellValue(coords);

    wxArrayString lines;
    dc.SetFont(attr.GetFont());

    //Taken from wxSheet again!
    wxCoord x = 0, y = 0, curr_x = 0;
    wxCoord max_x = rect.GetWidth();

    wxStringTokenizer tk(data, _T(" \n\t\r"));
    wxString thisline;

    while ( tk.HasMoreTokens() )
    {
        wxString tok = tk.GetNextToken();
        //FIXME: this causes us to print an extra unnecesary
        //       space at the end of the line. But it
        //       is invisible , simplifies the size calculation
        //       and ensures tokens are separated in the display
        tok += _T(" ");

        dc.GetTextExtent(tok, &x, &y);
        if ( curr_x + x > max_x) 
        {
            lines.Add( thisline );
            thisline = tok;
            curr_x = x;
        } 
        else 
        {
            thisline += tok;
            curr_x += x;
        }
    }
    
    lines.Add( thisline ); //Add last line

    return lines;
}

wxSize wxSheetCellAutoWrapStringRendererRefData::GetBestSize(wxSheet& grid,
                                                      wxSheetCellAttr& attr,
                                                      wxDC& dc,
                                                      const wxSheetCoords& coords)
{
    wxCoord x, y, height, width = grid.GetColWidth(coords.GetCol()) -10;
    int count = 250; //Limit iterations..

    wxRect rect(0, 0, width, 10);

    // M is a nice large character 'y' gives descender!.
    dc.SetFont(attr.GetFont());
    dc.GetTextExtent(wxT("My"), &x, &y);

    do     // Search for a shape no taller than the golden ratio.
    {
        width += 10;
        rect.SetWidth(width);
        height = y * GetTextLines(grid,dc,attr,rect,coords).GetCount();
        count--;
    } while (count && (width < (height*1.68)) );

    return wxSize(width, height);
}

// ----------------------------------------------------------------------------
// wxSheetCellRolColLabelRendererRefData
// ----------------------------------------------------------------------------

void wxSheetCellRolColLabelRendererRefData::Draw(wxSheet& grid,
                                            const wxSheetCellAttr& attr,
                                            wxDC& dc,
                                            const wxRect& rectCell,
                                            const wxSheetCoords& coords,
                                            bool isSelected)
{    
    // erase this cells background
    wxRect rect(rectCell);
    wxSheetCellRendererRefData::Draw(grid, attr, dc, rect, coords, isSelected);
    
    int left   = rectCell.x;
    int top    = rectCell.y;
    int right  = rectCell.GetRight();
    int bottom = rectCell.GetBottom();
    
    //dc.SetPen( wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW),1, wxSOLID) );
    // right side
    //dc.DrawLine( right, top, right, bottom );
    // left side
    //if (coords.m_col == -1)
    //    dc.DrawLine( left, top, left, bottom );
    // top side
    //if (coords.m_row == -1)
    //    dc.DrawLine( left, top, right, top );
    // bottom
    //dc.DrawLine( left, bottom, right, bottom );

    dc.SetPen( *wxWHITE_PEN );
    // left highlight
    dc.DrawLine( left, top, left, bottom );
    // top highlight
    dc.DrawLine( left, top, right, top );

    SetTextColoursAndFont(grid, attr, dc, isSelected);

    wxString value = grid.GetCellValue(coords);

    if (!value.IsEmpty())
    {
        int align  = attr.GetAlignment();
        int orient = attr.GetOrientation();
        rect.Deflate(2); // want margins
        grid.DrawTextRectangle(dc, value, rect, align, orient);    
    }
    
#if 0    
    // test code for sizing, draws corner tick marks    
    if (1)
    {
        rect = rectCell;
        dc.SetPen(*wxGREEN_PEN);
        dc.DrawLine(left, top, left+25, top);
        dc.DrawLine(right-25, bottom, right, bottom);
        dc.DrawLine(left, top, left, top+10);
        dc.DrawLine(right, bottom-10, right, bottom);
        wxRect r(rectCell);
        dc.SetPen(*wxCYAN_PEN);
        dc.DrawLine(r.x, r.y, r.x+25, r.y);
        dc.DrawLine(r.GetRight()-25, r.GetBottom(), r.GetRight(), r.GetBottom());
        dc.DrawLine(r.x, r.y, r.x, r.y+10);
        dc.DrawLine(r.GetRight(), r.GetBottom()-10, r.GetRight(), r.GetBottom());
    }
#endif // 0    
}

#endif // wxUSE_GRID
