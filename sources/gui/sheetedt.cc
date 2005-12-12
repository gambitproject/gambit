///////////////////////////////////////////////////////////////////////////////
// Name:        sheetedt.cpp
// Purpose:     wxSheet controls
// Author:      John Labenski, Paul Gammans, Roger Gammans
// Modified by: John Labenski
// Created:     11/04/2001
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, The Computer Surgery (paul@compsurg.co.uk)
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "sheetedt.h"
#endif

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

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
    #include "wx/valtext.h"
    #include "wx/settings.h"    
    #include "wx/intl.h"
#endif // WX_PRECOMP

#include "sheet.h"
#include "sheetedt.h"
#include "wx/tokenzr.h"
#include "wx/spinctrl.h"
// Required for wxIs... functions
#include <ctype.h>

#define PRINT_RECT(s, b) wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), wxT(s), b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());


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

IMPLEMENT_DYNAMIC_CLASS(wxSheetCellEditor, wxObject)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellEditorRefData, wxObject)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellTextEditorRefData, wxSheetCellEditorRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellNumberEditorRefData, wxSheetCellTextEditorRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellFloatEditorRefData, wxSheetCellTextEditorRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellBoolEditorRefData, wxSheetCellEditorRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellChoiceEditorRefData, wxSheetCellEditorRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellEnumEditorRefData, wxSheetCellChoiceEditorRefData)
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellAutoWrapStringEditorRefData, wxSheetCellTextEditorRefData)

// ----------------------------------------------------------------------------
// wxSheetCellEditorEvtHandler
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( wxSheetCellEditorEvtHandler, wxEvtHandler )

BEGIN_EVENT_TABLE( wxSheetCellEditorEvtHandler, wxEvtHandler )
    EVT_KEY_DOWN       ( wxSheetCellEditorEvtHandler::OnKeyDown )
    EVT_CHAR           ( wxSheetCellEditorEvtHandler::OnChar )
    EVT_WINDOW_DESTROY ( wxSheetCellEditorEvtHandler::OnDestroy )
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
    wxCHECK_RET(m_sheet && m_editor, wxT("Invalid sheet or editor in wxSheetCellEditorEvtHandler"));

    if (!m_editor->OnKeyDown(event))
        return;
    
    switch ( event.GetKeyCode() )
    {
        case WXK_ESCAPE:
        {
            m_editor->ResetValue();
            m_sheet->DisableCellEditControl(false);
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
    wxCHECK_RET(m_sheet && m_editor, wxT("Invalid sheet or editor in wxSheetCellEditorEvtHandler"));

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

void wxSheetCellEditorEvtHandler::OnDestroy(wxWindowDestroyEvent& event)
{
    wxCHECK_RET(m_sheet && m_editor, wxT("Invalid sheet or editor in wxSheetCellEditorEvtHandler"));

    event.Skip(); 
    
    // when parent window is destroyed pop event handler and NULL the control 
    if (m_editor->m_control && (m_editor->m_control->GetEventHandler() == this))
    {
        // see if anyone else want's to process the event before we delete ourself
        if (!GetNextHandler()->ProcessEvent(event))
        {
            wxWindow *win = m_editor->m_control;
            m_editor->m_control = NULL;
            win->PopEventHandler(true);
        }
        
        event.Skip(false);
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
    wxCHECK_MSG(Ok(), false, wxT("wxSheetCellEditor not created"));
    return M_CELLEDITORDATA->IsCreated();
}
bool wxSheetCellEditor::IsShown() const
{
    wxCHECK_MSG(Ok(), false, wxT("wxSheetCellEditor not created"));
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
void wxSheetCellEditor::DestroyControl()
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->DestroyControl();
}
void wxSheetCellEditor::CreateEditor(wxWindow* parent, wxWindowID id, 
                                     wxEvtHandler* evtHandler, wxSheet* sheet)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->CreateEditor(parent, id, evtHandler, sheet);
}
void wxSheetCellEditor::SetSize(const wxRect& rect, const wxSheetCellAttr &attr)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->SetSize(rect, attr);
}
wxSize wxSheetCellEditor::GetBestSize(wxSheet& sheet, const wxSheetCellAttr& attr,
                                      const wxSheetCoords& coords) const
{
    wxCHECK_MSG(Ok(), sheet.GetCellSize(coords), wxT("wxSheetCellEditor not created"));
    return M_CELLEDITORDATA->GetBestSize(sheet, attr, coords);
}
void wxSheetCellEditor::Show(bool show, const wxSheetCellAttr &attr)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->Show(show, attr);
}
void wxSheetCellEditor::PaintBackground(wxSheet& sheet, const wxSheetCellAttr& attr, 
                                        wxDC& dc, const wxRect& rect, 
                                        const wxSheetCoords& coords, bool isSelected)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->PaintBackground(sheet, attr, dc, rect, coords, isSelected);
}
void wxSheetCellEditor::InitEditor(const wxSheetCoords& coords, const wxSheetCellAttr& attr, 
                                   wxSheet* sheet)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->InitEditor(coords, attr, sheet);
}
void wxSheetCellEditor::BeginEdit(const wxSheetCoords& coords, wxSheet* sheet)
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->BeginEdit(coords, sheet);
}
bool wxSheetCellEditor::EndEdit(const wxSheetCoords& coords, wxSheet* sheet)
{
    wxCHECK_MSG(Ok(), false, wxT("wxSheetCellEditor not created"));
    return M_CELLEDITORDATA->EndEdit(coords, sheet);
}
void wxSheetCellEditor::ResetValue()
{
    wxCHECK_RET(Ok(), wxT("wxSheetCellEditor not created"));
    M_CELLEDITORDATA->ResetValue();
}
bool wxSheetCellEditor::IsAcceptedKey(wxKeyEvent& event)
{
    wxCHECK_MSG(Ok(), false, wxT("wxSheetCellEditor not created"));
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
    wxCHECK_MSG(Ok(), true, wxT("wxSheetCellEditor not created"));
    return M_CELLEDITORDATA->OnKeyDown(event);
}
bool wxSheetCellEditor::OnChar(wxKeyEvent& event)
{
    wxCHECK_MSG(Ok(), true, wxT("wxSheetCellEditor not created"));
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
    wxCHECK_MSG(other.Ok(), false, wxT("wxSheetCellEditor not created"));
    
    UnRef();
    m_refData = ((wxSheetCellEditorRefData*)other.m_refData)->Clone();
    return true;
}

// ----------------------------------------------------------------------------
// wxSheetCellEditorRefData
// ----------------------------------------------------------------------------
wxSheetCellEditorRefData::~wxSheetCellEditorRefData() 
{
    if (GetControl())
        DestroyControl(); 
}

bool wxSheetCellEditorRefData::IsShown() const
{
    return GetControl() && GetControl()->IsShown();
}

void wxSheetCellEditorRefData::SetControl(wxWindow* control) 
{ 
    DestroyControl();
    m_control = control; 
}

void wxSheetCellEditorRefData::DestroyControl() 
{ 
    if (m_control)
    {
        wxWindow *win = m_control;
        m_control = NULL;
        
        // if pushed event handler, pop and delete the handler
        if (win != win->GetEventHandler())
            win->PopEventHandler(true); 
        
        win->Destroy();
    }
}

void wxSheetCellEditorRefData::CreateEditor(wxWindow* WXUNUSED(parent),
                                            wxWindowID WXUNUSED(id),
                                            wxEvtHandler* evtHandler,
                                            wxSheet* WXUNUSED(sheet))
{
    if (evtHandler && GetControl())
        GetControl()->PushEventHandler(evtHandler);
}

void wxSheetCellEditorRefData::SetSize(const wxRect& rect, const wxSheetCellAttr &)
{
    wxCHECK_RET(GetControl(), wxT("wxSheetCellEditor not created"));
    GetControl()->SetSize(rect, wxSIZE_ALLOW_MINUS_ONE);
}

wxSize wxSheetCellEditorRefData::GetBestSize(wxSheet& sheet, const wxSheetCellAttr& ,
                                             const wxSheetCoords& coords)
{
    wxCHECK_MSG(GetControl(), sheet.GetCellSize(coords), wxT("wxSheetCellEditor not created"));
    return GetControl()->GetBestSize();
}

void wxSheetCellEditorRefData::Show(bool show, const wxSheetCellAttr &)
{
    wxCHECK_RET(GetControl(), wxT("wxSheetCellEditor not created"));
    GetControl()->Show(show);   
}

void wxSheetCellEditorRefData::PaintBackground(wxSheet& , const wxSheetCellAttr& attr, 
                                               wxDC& dc, const wxRect& rect, 
                                               const wxSheetCoords& , bool )
{
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
    if (GetControl())
        GetControl()->Refresh();
}


void wxSheetCellEditorRefData::InitEditor(const wxSheetCoords& , const wxSheetCellAttr& attr, 
                                          wxSheet* )
{
    wxCHECK_RET(GetControl(), wxT("wxSheetCellEditor not created"));
    // set the colours/fonts if we have any
    if ( attr.Ok() )
    {
        GetControl()->SetForegroundColour(attr.GetForegroundColour());
        
        // FIXME this is weird, in GTK you can't set the background of a textctrl
        // to the default window background colour, ie. label textctrls
        // this doesn't happen anymore in > 2.5.3 needs test in 2.4 maybe
        //wxColour c = attr.GetBackgroundColour();
        //c.Set(wxMin(c.Red()+1, 255), c.Green(), c.Blue());
        //GetControl()->SetBackgroundColour(c);
        
        GetControl()->SetBackgroundColour(attr.GetBackgroundColour());
        GetControl()->SetFont(attr.GetFont());
    }
}

bool wxSheetCellEditorRefData::IsAcceptedKey(wxKeyEvent& event)
{
    // accept the simple key presses, not anything with Ctrl/Alt or just shift
    //return !(event.ControlDown() || event.AltDown() || (event.GetKeyCode() == WXK_SHIFT));

    bool ctrl = event.ControlDown();
    bool alt  = event.AltDown();
#ifdef __WXMAC__
    // On the Mac the Alt key is more like shift and is used for entry of
    // valid characters, so check for Ctrl and Meta instead.
    alt = event.MetaDown();
#endif

    // Assume it's not a valid char if ctrl or alt is down, but if both are
    // down then it may be because of an AltGr key combination, so let them
    // through in that case.
    if ((ctrl || alt) && !(ctrl && alt))
        return false;
    
#if wxUSE_UNICODE
    int key = event.GetUnicodeKey();
    bool keyOk = true;

    // if the unicode key code is not really a unicode character (it may
    // be a function key or etc., the platforms appear to always give us a
    // small value in this case) then fallback to the ascii key code but
    // don't do anything for function keys or etc.
    if (key <= 127)
    {
        key = event.GetKeyCode();
        keyOk = (key <= 127);
    }
    return keyOk;
#else // !wxUSE_UNICODE
    int key = event.GetKeyCode();
    if (key <= 255) 
        return true;
    return false;
#endif // wxUSE_UNICODE/!wxUSE_UNICODE
}

// ----------------------------------------------------------------------------
// wxSheetCellTextEditorRefData
// ----------------------------------------------------------------------------
#if wxUSE_TEXTCTRL

void wxSheetCellTextEditorRefData::CreateEditor(wxWindow* parent,
                                                wxWindowID id,
                                                wxEvtHandler* evtHandler,
                                                wxSheet* sheet)
{
    SetControl(new wxTextCtrl(parent, id, wxEmptyString,
                              wxDefaultPosition, wxDefaultSize //,wxBORDER_NONE
#if defined(__WXMSW__)
                              , wxTE_PROCESS_TAB | wxTE_AUTO_SCROLL
#endif // defined(__WXMSW__)
                              ));

    // set max length allowed in the textctrl, if the parameter was set
    if (m_maxChars != 0)
        GetTextCtrl()->SetMaxLength(m_maxChars);

    wxSheetCellEditorRefData::CreateEditor(parent, id, evtHandler, sheet);
}

void wxSheetCellTextEditorRefData::SetSize(const wxRect& rectOrig, const wxSheetCellAttr &attr)
{
    wxCHECK_RET(IsCreated(), wxT("The wxSheetCellEditor must be Created first!"));
    wxRect rect(rectOrig);

    // Put the control inside the grid lines on all sides.
    
#if defined(__WXGTK__)
    // these are good for GTK 1.2, wx ver 2.5
    //if (rect.x != 0) rect.x += 1;  
    //if (rect.y != 0) rect.y += 1;
        
#elif defined(__WXMSW__) 
    // This works for wxMSW ver 2.5
    //if (rect.x != 0) rect.x += 1;  
    //if (rect.y != 0) rect.y += 1;

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

wxSize wxSheetCellTextEditorRefData::GetBestSize(wxSheet& sheet, const wxSheetCellAttr& attr,
                                                 const wxSheetCoords& coords)
{
    wxCHECK_MSG(GetTextCtrl(), sheet.GetCellSize(coords), wxT("wxSheetCellEditor not created"));
    // in GTK and MSW the wxTextCtrl returns a fixed width, ignoring contents
    wxString value = sheet.GetCellValue(coords);
    int w = 0, h = 0;
    GetTextCtrl()->GetTextExtent(value, &w, &h, NULL, NULL, &attr.GetFont());
    wxSize textCtrlSize(GetTextCtrl()->GetBestSize());
    return wxSize(w + 4, textCtrlSize.y);
}

void wxSheetCellTextEditorRefData::BeginEdit(const wxSheetCoords& coords, wxSheet* sheet)
{
    wxCHECK_RET(IsCreated() && sheet, wxT("The wxSheetCellEditor must be Created first!"));

    m_startValue = sheet->GetCellValue(coords);
    m_maxLength = 0;
    DoBeginEdit(m_startValue);
}

void wxSheetCellTextEditorRefData::DoBeginEdit(const wxString& startValue)
{
    wxCHECK_RET(IsCreated(), wxT("The wxSheetCellEditor must be Created first!"));
    
    // In MSW, SetValue has the effect of pushing the value off the left side, 
    //   even if it fits within width. AppendText seems to work better.
    //GetTextCtrl()->SetValue(startValue);
    //GetTextCtrl()->SetInsertionPointEnd();

    GetTextCtrl()->Clear();
    GetTextCtrl()->AppendText(startValue);
    GetTextCtrl()->SetSelection(-1,-1);
    GetTextCtrl()->SetFocus();
}

bool wxSheetCellTextEditorRefData::EndEdit(const wxSheetCoords& coords, wxSheet* sheet)
{
    wxCHECK_MSG(IsCreated() && sheet, false, wxT("The wxSheetCellEditor must be Created first!"));

    wxString value = GetTextCtrl()->GetValue();
    bool changed = (value != m_startValue);

    if (changed)
    {
        if (sheet->SendEvent(wxEVT_SHEET_CELL_VALUE_CHANGING, coords, NULL, value) == wxSheet::EVT_VETOED) 
            changed = false;
        else
            sheet->SetCellValue(coords, value);
    }

    m_startValue.Clear();
    m_maxLength = 0;

    return changed;
}

void wxSheetCellTextEditorRefData::ResetValue()
{
    wxCHECK_RET(IsCreated(), wxT("The wxSheetCellEditor must be Created first!"));
    DoResetValue(m_startValue);
}

void wxSheetCellTextEditorRefData::DoResetValue(const wxString& startValue)
{
    wxCHECK_RET(IsCreated(), wxT("The wxSheetCellEditor must be Created first!"));
    GetTextCtrl()->SetValue(startValue);
    GetTextCtrl()->SetInsertionPointEnd();
}

bool wxSheetCellTextEditorRefData::IsAcceptedKey(wxKeyEvent& event)
{
    return wxSheetCellEditorRefData::IsAcceptedKey(event);
}

void wxSheetCellTextEditorRefData::StartingKey(wxKeyEvent& event)
{
    wxCHECK_RET(IsCreated(), wxT("The wxSheetCellEditor must be Created first!"));

    // Since this is now happening in the EVT_CHAR event EmulateKeyPress is no
    // longer an appropriate way to get the character into the text control.
    // Do it ourselves instead.  We know that if we get this far that we have
    // a valid character, so not a whole lot of testing needs to be done.

    wxTextCtrl* tc = GetTextCtrl();
    wxChar ch;
    long pos;
    
#if wxUSE_UNICODE
    ch = event.GetUnicodeKey();
    if (ch <= 127)
        ch = (wxChar)event.GetKeyCode();
#else
    ch = (wxChar)event.GetKeyCode();
#endif
    switch (ch)
    {
        case WXK_DELETE:
            // delete the character at the cursor
            pos = tc->GetInsertionPoint();
            if (pos < tc->GetLastPosition())
                tc->Remove(pos, pos+1);
            break;

        case WXK_BACK:
            // delete the character before the cursor
            pos = tc->GetInsertionPoint();
            if (pos > 0)
                tc->Remove(pos-1, pos);
            break;

        default:
            tc->WriteText(ch);
            break;
    }
}

void wxSheetCellTextEditorRefData::HandleReturn( wxKeyEvent&
                                         WXUNUSED_GTK(WXUNUSED_MOTIF(event)) )
{
    wxCHECK_RET(IsCreated(), wxT("The wxSheetCellEditor must be Created first!"));
    
#if defined(__WXMOTIF__) || defined(__WXGTK__)
    // wxMotif needs a little extra help...
    size_t pos = (size_t)( GetTextCtrl()->GetInsertionPoint() );
    wxString s( GetTextCtrl()->GetValue() );
    s = s.Left(pos) + wxT("\n") + s.Mid(pos);
    GetTextCtrl()->SetValue(s);
    GetTextCtrl()->SetInsertionPoint( pos );
#else
    // the other ports can handle a Return key press
    event.Skip();
#endif // defined(__WXMOTIF__) || defined(__WXGTK__)
}

bool wxSheetCellTextEditorRefData::OnChar( wxKeyEvent& WXUNUSED(event) )
{  
    wxCHECK_MSG(IsCreated(), true, wxT("The wxSheetCellEditor must be Created first!"));
    // This function is used to expand the textctrl as you type
    // it doesn't stop the event, just looks at it
    wxString value = GetTextCtrl()->GetValue();
    if (int(value.Length()) <= m_maxLength)
        return true;

    m_maxLength = value.Length();
    
    wxRect rect(GetTextCtrl()->GetRect());
    int clientWidth = GetTextCtrl()->GetParent()->GetClientSize().x;
    // already at edge of window, maybe the win should should scroll?
    if (rect.GetRight() >= clientWidth)
        return true;
    
    int w, h;
    wxFont font = GetTextCtrl()->GetFont();
    GetTextCtrl()->GetTextExtent(value, &w, &h, NULL, NULL, &font);
    
    // can't just use w/value.Len since it might be "iii", too small for "WWW"
    int chW;
    GetTextCtrl()->GetTextExtent(wxT("W"), &chW, &h, NULL, NULL, &font);
    int extra = int(chW*1.5);
    //if (rect.width < w+extra)
        //GetTextCtrl()->SetSize(wxMin(w+extra, clientWidth-rect.x), rect.height);

    w += extra;
    
    wxSheet* sheet = wxDynamicCast(GetTextCtrl()->GetParent()->GetParent(), wxSheet);
    if (!sheet)
        return true;
    
    wxSheetCoords c(sheet->GetEditControlCoords());
    if (!sheet->GetAttrOverflow(c))
        return true;
    
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
    GetTextCtrl()->SetSize(rect.width, rect.height);
    
    return true;
}

void wxSheetCellTextEditorRefData::SetParameters(const wxString& params)
{
    if ( params.IsEmpty() )
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
    return GetTextCtrl()->GetValue();
}

#endif // wxUSE_TEXTCTRL

// ----------------------------------------------------------------------------
// wxSheetCellAutoWrapStringEditorRefData
// ----------------------------------------------------------------------------
#if wxUSE_TEXTCTRL

void wxSheetCellAutoWrapStringEditorRefData::CreateEditor(wxWindow* parent,
                                                          wxWindowID id,
                                                          wxEvtHandler* evtHandler,
                                                          wxSheet* sheet)
{
    SetControl( new wxTextCtrl(parent, id, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE | wxTE_RICH) );

    wxSheetCellEditorRefData::CreateEditor(parent, id, evtHandler, sheet);
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
                                                  wxEvtHandler* evtHandler,
                                                  wxSheet* sheet)
{
    if ( HasRange() )
    {
        // create a spin ctrl
        SetControl( new wxSpinCtrl(parent, wxID_ANY, wxEmptyString,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_ARROW_KEYS, m_min, m_max) );

        wxSheetCellEditorRefData::CreateEditor(parent, id, evtHandler, sheet);
    }
    else
    {
        // just a text control
        wxSheetCellTextEditorRefData::CreateEditor(parent, id, evtHandler, sheet);

#if wxUSE_VALIDATORS
        GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
#endif // wxUSE_VALIDATORS
    }
}

void wxSheetCellNumberEditorRefData::BeginEdit(const wxSheetCoords& coords, wxSheet* sheet)
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    // first get the value
    wxSheetTable *table = sheet->GetTable();
    if ( table && table->CanGetValueAs(coords, wxSHEET_VALUE_NUMBER) )
    {
        m_startValue = table->GetValueAsLong(coords);
    }
    else
    {
        m_startValue = 0;
        wxString sValue = sheet->GetCellValue(coords);
        wxCHECK_RET(sValue.IsEmpty() || sValue.ToLong(&m_startValue),
                    _T("this cell doesn't have numeric value") );
    }

    if ( HasRange() )
    {
        GetSpinCtrl()->SetValue((int)m_startValue);
        GetSpinCtrl()->SetFocus();
    }
    else
    {
        DoBeginEdit(GetInitValue());
    }
}

bool wxSheetCellNumberEditorRefData::EndEdit(const wxSheetCoords& coords, wxSheet* sheet)
{
    wxCHECK_MSG(IsCreated() && sheet, false, wxT("The wxSheetCellEditor must be Created first!"));
    
    bool changed;
    long value = 0;
    long oldValue = m_startValue;
    wxString text;
  
    if ( HasRange() )
    {
        value = GetSpinCtrl()->GetValue();
        changed = value != oldValue;
        if (changed)
            text = wxString::Format(wxT("%ld"), value);
    }
    else
    {
        text = GetTextCtrl()->GetValue();
        changed = (text.IsEmpty() || text.ToLong(&value)) && (value != oldValue);
    }

    if ( changed )
    {
        if (sheet->SendEvent(wxEVT_SHEET_CELL_VALUE_CHANGING, coords, NULL, text, value) == wxSheet::EVT_VETOED) 
            return false;

        wxSheetTable *table = sheet->GetTable();       
        if (table && table->CanSetValueAs(coords, wxSHEET_VALUE_NUMBER))
            table->SetValueAsLong(coords, value);
        else
            sheet->SetCellValue(coords, text);
    }

    return changed;
}

void wxSheetCellNumberEditorRefData::ResetValue()
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    if ( HasRange() )
        GetSpinCtrl()->SetValue((int)m_startValue);
    else
        DoResetValue(GetInitValue());
}

bool wxSheetCellNumberEditorRefData::IsAcceptedKey(wxKeyEvent& event)
{
    if ( wxSheetCellEditorRefData::IsAcceptedKey(event) )
    {
        int keycode = event.GetKeyCode();
        if ( (keycode < 128) &&
             (wxIsdigit(keycode) || keycode == '+' || keycode == '-'))
        {
            return true;
        }
    }

    return false;
}

void wxSheetCellNumberEditorRefData::StartingKey(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    if ( !HasRange() )
    {
        if ( wxIsdigit(keycode) || keycode == '+' || keycode == '-')
        {
            wxSheetCellEditorRefData::StartingKey(event);

            // skip Skip() below
            return;
        }
    }
#if wxUSE_SPINCTRL
    else
    {
        if ( wxIsdigit(keycode) )
        {
            wxSpinCtrl* spin = (wxSpinCtrl*)m_control;
            spin->SetValue(keycode - '0');
            spin->SetSelection(1,1);
            return;
        }
    }
#endif // wxUSE_SPINCTRL

    event.Skip();
}

void wxSheetCellNumberEditorRefData::SetParameters(const wxString& params)
{
    if ( params.IsEmpty() )
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
        long value = GetSpinCtrl()->GetValue();
        s.Printf(wxT("%ld"), value);
    }
    else
    {
        s = GetTextCtrl()->GetValue();
    }
    return s;
}

wxString wxSheetCellNumberEditorRefData::GetInitValue() const
{ 
    return wxString::Format(_T("%ld"), m_startValue); 
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
                                                 wxEvtHandler* evtHandler,
                                                 wxSheet* sheet)
{
    wxSheetCellTextEditorRefData::CreateEditor(parent, id, evtHandler, sheet);

#if wxUSE_VALIDATORS
    GetTextCtrl()->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
#endif // wxUSE_VALIDATORS
}

void wxSheetCellFloatEditorRefData::BeginEdit(const wxSheetCoords& coords, wxSheet* sheet)
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    // first get the value
    wxSheetTable *table = sheet->GetTable();
    if ( table && table->CanGetValueAs(coords, wxSHEET_VALUE_FLOAT) )
    {
        m_startValue = table->GetValueAsDouble(coords);
    }
    else
    {
        m_startValue = 0.0;
        wxString sValue = sheet->GetCellValue(coords);
        if (!sValue.ToDouble(&m_startValue) && !sValue.IsEmpty())
        {
            wxFAIL_MSG( _T("this cell doesn't have float value") );
            return;
        }
    }

    DoBeginEdit(GetInitValue());
}

bool wxSheetCellFloatEditorRefData::EndEdit(const wxSheetCoords& coords, wxSheet* sheet)
{
    wxCHECK_MSG(IsCreated() && sheet, false, wxT("The wxSheetCellEditor must be Created first!"));
    
    double value = 0.0;
    wxString text(GetTextCtrl()->GetValue());

    if ( (text.IsEmpty() || text.ToDouble(&value)) && (value != m_startValue) )
    {
        if (sheet->SendEvent(wxEVT_SHEET_CELL_VALUE_CHANGING, coords, NULL, GetStringValue(value)) == wxSheet::EVT_VETOED) 
            return false;

        wxSheetTable *table = sheet->GetTable();
        if (table && table->CanSetValueAs(coords, wxSHEET_VALUE_FLOAT))
            table->SetValueAsDouble(coords, value);
        else
            sheet->SetCellValue(coords, text.IsEmpty() ? GetStringValue(value) : text);

        return true;
    }
    return false;
}

void wxSheetCellFloatEditorRefData::ResetValue()
{
    DoResetValue(GetInitValue());
}

void wxSheetCellFloatEditorRefData::StartingKey(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    char tmpbuf[2];
    tmpbuf[0] = (char) keycode;
    tmpbuf[1] = '\0';
    wxString strbuf(tmpbuf, *wxConvCurrent);
#if wxUSE_INTL        
    bool is_decimal_point = ( strbuf ==
       wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT, wxLOCALE_CAT_NUMBER) );
#else
    bool is_decimal_point = ( strbuf == _T(".") );
#endif
    if ( wxIsdigit(keycode) || keycode == '+' || keycode == '-'
         || is_decimal_point )
    {
        wxSheetCellEditorRefData::StartingKey(event);

        // skip Skip() below
        return;
    }

    event.Skip();
}

void wxSheetCellFloatEditorRefData::SetParameters(const wxString& params)
{
    if ( params.IsEmpty() )
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

wxString wxSheetCellFloatEditorRefData::GetStringValue(double value) const
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

    return wxString::Format(fmt, value);
}

bool wxSheetCellFloatEditorRefData::IsAcceptedKey(wxKeyEvent& event)
{
    if ( wxSheetCellEditorRefData::IsAcceptedKey(event) )
    {
        int keycode = event.GetKeyCode();
        //printf("%d\n", keycode);
        // accept digits, 'e' as in '1e+6', also '-', '+', and '.'
        char tmpbuf[2];
        tmpbuf[0] = (char) keycode;
        tmpbuf[1] = '\0';
        wxString strbuf(tmpbuf, *wxConvCurrent);
#if wxUSE_INTL        
        bool is_decimal_point =
            ( strbuf == wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT,
                                          wxLOCALE_CAT_NUMBER) );
#else
        bool is_decimal_point = ( strbuf == _T(".") );
#endif
        if ( (keycode < 128) && 
             (wxIsdigit(keycode) || tolower(keycode) == 'e' ||
              is_decimal_point || keycode == '+' || keycode == '-') )
            return true;
    }

    return false;
}

#endif // wxUSE_TEXTCTRL

// ----------------------------------------------------------------------------
// wxSheetCellBoolEditorRefData
// ----------------------------------------------------------------------------
#if wxUSE_CHECKBOX

void wxSheetCellBoolEditorRefData::CreateEditor(wxWindow* parent,
                                                wxWindowID id,
                                                wxEvtHandler* evtHandler,
                                                wxSheet* sheet)
{
    SetControl( new wxCheckBox(parent, id, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize,
                               wxNO_BORDER) );

    wxSheetCellEditorRefData::CreateEditor(parent, id, evtHandler, sheet);
}

void wxSheetCellBoolEditorRefData::SetSize(const wxRect& r, const wxSheetCellAttr &attr)
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    bool resize = false;
    wxSize size = GetControl()->GetSize();
    wxCoord minSize = wxMin(r.width, r.height);

    // check if the checkbox is not too big/small for this cell
    wxSize sizeBest = GetControl()->GetBestSize();
    if ( !(size == sizeBest) )
    {
        // reset to default size if it had been made smaller
        size = sizeBest;
        resize = true;
    }

    if ( (size.x >= minSize) || (size.y >= minSize) )
    {
        // leave 1 pixel margin
        size.x = size.y = minSize - 2;
        resize = true;
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
        GetCheckBox()->SetBackgroundColour(colBg);
    }
}

void wxSheetCellBoolEditorRefData::BeginEdit(const wxSheetCoords& coords, wxSheet* sheet)
{
    wxCHECK_RET(sheet && GetControl(), wxT("The wxSheetCellEditor must be Created first!"));

    wxSheetTable *table = sheet->GetTable();
    if (table && table->CanGetValueAs(coords, wxSHEET_VALUE_BOOL))
        m_startValue = table->GetValueAsBool(coords);
    else
    {
        wxString cellval( sheet->GetCellValue(coords) );
        m_startValue = !( cellval.IsEmpty() || (cellval == wxT("0")) );
    }
    GetCheckBox()->SetValue(m_startValue);
    GetCheckBox()->SetFocus();
}

bool wxSheetCellBoolEditorRefData::EndEdit(const wxSheetCoords& coords, wxSheet* sheet)
{
    wxCHECK_MSG(GetControl(), false, wxT("The wxSheetCellEditor must be Created first!"));
    
    bool value = GetCheckBox()->GetValue();
    wxString strValue = value ? wxT("1") : wxT("0");
    bool changed = ( value != m_startValue );

    if ( changed )
    {
        if (sheet->SendEvent(wxEVT_SHEET_CELL_VALUE_CHANGING, coords, NULL, strValue, value ? 1 : 0) == wxSheet::EVT_VETOED) 
            return false;
        
        wxSheetTable *table = sheet->GetTable();
        if (table && table->CanSetValueAs(coords, wxSHEET_VALUE_BOOL))
            table->SetValueAsBool(coords, value);
        else
            sheet->SetCellValue(coords, strValue);
    }

    return changed;
}

void wxSheetCellBoolEditorRefData::ResetValue()
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    GetCheckBox()->SetValue(m_startValue);
}

void wxSheetCellBoolEditorRefData::StartingClick()
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    GetCheckBox()->SetValue(!GetCheckBox()->GetValue());
}

bool wxSheetCellBoolEditorRefData::IsAcceptedKey(wxKeyEvent& event)
{
    if ( wxSheetCellEditorRefData::IsAcceptedKey(event) )
    {
        int keycode = event.GetKeyCode();
        switch ( keycode )
        {
            case WXK_SPACE:
            case '+':
            case '-':
                return true;
        }
    }

    return false;
}

void wxSheetCellBoolEditorRefData::StartingKey(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    switch ( keycode )
    {
        case WXK_SPACE:
            GetCheckBox()->SetValue(!GetCheckBox()->GetValue());
            break;
            
        case '+':
            GetCheckBox()->SetValue(true);
            break;
                
        case '-':
            GetCheckBox()->SetValue(false);
            break;
    }
}

// return the value as "1" for true and the empty string for false
wxString wxSheetCellBoolEditorRefData::GetValue() const
{
    wxCHECK_MSG(GetControl(), wxEmptyString, wxT("The wxSheetCellEditor must be Created first!"));
    bool bSet = GetCheckBox()->GetValue();
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
                                                  wxEvtHandler* evtHandler,
                                                  wxSheet* sheet)
{
#if wxCHECK_VERSION(2,5,0)
    SetControl( new wxComboBox(parent, id, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize,
                               m_choices,
                               m_allowOthers ? 0 : wxCB_READONLY) );
#else 
    const size_t count = m_choices.GetCount();
    wxString *choices = new wxString[count];
    for ( size_t n = 0; n < count; n++ )
        choices[n] = m_choices[n];

    SetControl( new wxComboBox(parent, id, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize,
                               count, choices,
                               m_allowOthers ? 0 : wxCB_READONLY) );

    delete []choices;
#endif // wxCHECK_VERSION(2,5,0)
    wxSheetCellEditorRefData::CreateEditor(parent, id, evtHandler, sheet);
}

void wxSheetCellChoiceEditorRefData::PaintBackground(wxSheet& sheet, const wxSheetCellAttr& attr, 
                                                     wxDC& dc, const wxRect& rect, 
                                                     const wxSheetCoords& coords, bool isSelected)
{
    // as we fill the entire client area, don't do anything here to minimize
    // flicker

    // TODO: It doesn't actually fill the client area since the height of a
    // combo always defaults to the standard...  Until someone has time to
    // figure out the right rectangle to paint, just do it the normal way...
    wxSheetCellEditorRefData::PaintBackground(sheet, attr, dc, rect, coords, isSelected);
}

void wxSheetCellChoiceEditorRefData::BeginEdit(const wxSheetCoords& coords, wxSheet* sheet)
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));

    wxString m_startValue = sheet->GetCellValue(coords);

    if (m_allowOthers)
        GetComboBox()->SetValue(m_startValue);
    else
    {
        // find the right position, or default to the first if not found
        int pos = GetComboBox()->FindString(m_startValue);
        if (pos == -1)
            pos = 0;
        if (GetComboBox()->GetCount() > pos)
            GetComboBox()->SetSelection(pos);
    }
    GetComboBox()->SetInsertionPointEnd();
    GetComboBox()->SetFocus();
}

bool wxSheetCellChoiceEditorRefData::EndEdit(const wxSheetCoords& coords, wxSheet* sheet)
{
    wxCHECK_MSG(GetControl(), false, wxT("The wxSheetCellEditor must be Created first!"));
    wxString value = GetComboBox()->GetValue();
    if (value == m_startValue)
        return false;

    if (sheet->SendEvent(wxEVT_SHEET_CELL_VALUE_CHANGING, coords, NULL, value) == wxSheet::EVT_VETOED) 
        return false;
        
    sheet->GetTable()->SetValue(coords, value);
    return true;
}

void wxSheetCellChoiceEditorRefData::ResetValue()
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEditor must be Created first!"));
    GetComboBox()->SetValue(m_startValue);
    GetComboBox()->SetInsertionPointEnd();
}

void wxSheetCellChoiceEditorRefData::SetParameters(const wxString& params)
{
    // what can we do?
    if ( params.IsEmpty() )
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
    return GetComboBox()->GetValue();
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

void wxSheetCellEnumEditorRefData::BeginEdit(const wxSheetCoords& coords, wxSheet* sheet)
{
    wxCHECK_RET(GetControl(), wxT("The wxSheetCellEnumEditor must be Created first!"));
    wxSheetTable *table = sheet->GetTable();

    if ( table && table->CanGetValueAs(coords, wxSHEET_VALUE_NUMBER) )
        m_startint = table->GetValueAsLong(coords);
    else
    {
        wxString startValue = sheet->GetCellValue(coords);
        if (startValue.IsEmpty() || !startValue.ToLong(&m_startint))
            m_startint = -1;
    }

    GetComboBox()->SetSelection(m_startint);
    GetComboBox()->SetInsertionPointEnd();
    GetComboBox()->SetFocus();
}

bool wxSheetCellEnumEditorRefData::EndEdit(const wxSheetCoords& coords, wxSheet* sheet)
{
    wxCHECK_MSG(GetControl(), false, wxT("The wxSheetCellEditor must be Created first!"));
    int value = GetComboBox()->GetSelection();
    wxString strValue = wxString::Format(wxT("%i"), value);
    bool changed = (value != m_startint);
    if (changed)
    {
        if (sheet->SendEvent(wxEVT_SHEET_CELL_VALUE_CHANGING, coords, NULL, strValue, value) == wxSheet::EVT_VETOED) 
            return false;
        
        if (sheet->GetTable() && sheet->GetTable()->CanSetValueAs(coords, wxSHEET_VALUE_NUMBER))
            sheet->GetTable()->SetValueAsLong(coords, value);
        else
            sheet->SetCellValue(coords, strValue);
    }

    return changed;
}

#endif // wxUSE_COMBOBOX
