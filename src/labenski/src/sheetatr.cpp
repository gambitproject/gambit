///////////////////////////////////////////////////////////////////////////////
// Name:        sheetatr.cpp
// Purpose:     wxSheet and related classes
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: John Labenski, Robin Dunn, Vadim Zeitlin
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, Michael Bedward (mbedward@ozemail.com.au)
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/sheet/sheetatr.h"
#include "wx/sheet/sheet.h"

#ifndef WX_PRECOMP
#endif // WX_PRECOMP

#define PRINT_BLOCK(s, b) wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), wxT(s), b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());
#define PRINT_RECT(s, b)  wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), wxT(s), b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

const wxSheetCellAttr wxNullSheetCellAttr(false);

#include "wx/arrimpl.cpp"
WX_DEFINE_OBJARRAY(wxArraySheetCellAttr)
DEFINE_PAIREDSHEETCOORDS_DATA_ARRAYS(wxSheetCellAttr, wxPairArraySheetCoordsCellAttr)
DEFINE_PAIRED_INT_DATA_ARRAYS(wxSheetCellAttr, wxPairArrayIntSheetCellAttr)

// ----------------------------------------------------------------------------
// wxSheetCellAttrRefData
// ----------------------------------------------------------------------------
#define M_CELLATTRDATA ((wxSheetCellAttrRefData*)m_refData)

wxSheetCellAttrRefData::wxSheetCellAttrRefData() 
                       : wxObjectRefData(),
                         // other m_attrTypes unset values are 0
                         m_attrTypes(wxSHEET_AttrCell|wxSHEET_AttrAlignUnset),  
                         m_renderer(NULL), m_editor(NULL), m_defaultAttr(NULL) 
{
}

wxSheetCellAttrRefData::wxSheetCellAttrRefData( const wxSheetCellAttrRefData& data ) 
                       : wxObjectRefData(),
                         m_foreColour(data.m_foreColour), 
                         m_backColour(data.m_backColour),
                         m_font(data.m_font), 
                         m_attrTypes(data.m_attrTypes)
{
    m_renderer    = !data.m_renderer    ? NULL : new wxSheetCellRenderer(*data.m_renderer);
    m_editor      = !data.m_editor      ? NULL : new wxSheetCellEditor(*data.m_editor);
    m_defaultAttr = !data.m_defaultAttr ? NULL : new wxSheetCellAttr(*data.m_defaultAttr);
}

wxSheetCellAttrRefData::~wxSheetCellAttrRefData()
{
    delete m_renderer;
    delete m_editor;
    delete m_defaultAttr;
}

// ----------------------------------------------------------------------------
// wxSheetCellAttr
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellAttr, wxObject)

wxSheetCellAttr::wxSheetCellAttr(bool create) : wxObject()
{
    if (create)
        m_refData = new wxSheetCellAttrRefData;
}

bool wxSheetCellAttr::Create()
{
    UnRef();
    m_refData = new wxSheetCellAttrRefData;    
    return m_refData != NULL;
}

wxObjectRefData *wxSheetCellAttr::CreateRefData() const 
{ 
    return new wxSheetCellAttrRefData; 
}
wxObjectRefData *wxSheetCellAttr::CloneRefData(const wxObjectRefData *data) const
{ 
    wxCHECK_MSG(data, new wxSheetCellAttrRefData, wxT("Invalid data for CloneRefData"));
    return new wxSheetCellAttrRefData(*(const wxSheetCellAttrRefData *)data); 
}

bool wxSheetCellAttr::Copy(const wxSheetCellAttr& other) 
{ 
    wxCHECK_MSG(other.Ok(), false, wxT("Attr to copy from is not created"));
    
    if (!Create())
        return false;
    
    M_CELLATTRDATA->m_foreColour = ((wxSheetCellAttrRefData*)other.GetRefData())->m_foreColour;
    M_CELLATTRDATA->m_backColour = ((wxSheetCellAttrRefData*)other.GetRefData())->m_backColour;
    M_CELLATTRDATA->m_font       = ((wxSheetCellAttrRefData*)other.GetRefData())->m_font;
    M_CELLATTRDATA->m_attrTypes  = ((wxSheetCellAttrRefData*)other.GetRefData())->m_attrTypes;
    if (other.HasEditor())
        SetEditor(((wxSheetCellAttrRefData*)other.m_refData)->m_editor->Clone());
    if (other.HasRenderer())
        SetRenderer(((wxSheetCellAttrRefData*)other.m_refData)->m_renderer->Clone());
    
    SetDefaultAttr(other.GetDefaultAttr());
    return true;
}

bool wxSheetCellAttr::UpdateWith(const wxSheetCellAttr& other) 
{ 
    wxCHECK_MSG(Ok() && other.Ok(), false, wxT("this or Attr to UpdateWith from is not created"));

    if ( other.HasForegoundColour() )
        SetForegroundColour(other.GetForegroundColour());
    if ( other.HasBackgroundColour() )
        SetBackgroundColour(other.GetBackgroundColour());
    if ( other.HasFont() )
        SetFont(other.GetFont());
    if ( other.HasAlignment() )
        SetAlignment(other.GetAlignment());
    if ( other.HasOrientation() )
        SetOrientation(other.GetOrientation());
    if ( other.HasLevel() )
        SetLevel(other.GetLevel());
    if ( other.HasReadWriteMode() )
        SetReadOnly(other.GetReadOnly());
    if ( other.HasOverflowMode() )
        SetOverflow(other.GetOverflow());
    if ( other.HasOverflowMarkerMode() )
        SetOverflowMarker(other.GetOverflowMarker());
    if ( other.HasShowEditorMode() )
        SetShowEditor(other.GetShowEditor());
    
    // Directly access m_renderer/m_editor as GetRender/Editor may return different one

    // Maybe add support for merge of Render and Editor?
    if ( other.HasRenderer() )
        SetRenderer(((wxSheetCellAttrRefData*)other.m_refData)->m_renderer->Clone());
    if ( other.HasEditor() )
        SetEditor(((wxSheetCellAttrRefData*)other.m_refData)->m_editor->Clone());

    if ( other.HasDefaultAttr() )
        SetDefaultAttr(other.GetDefaultAttr());
    
    return true;
}
    
bool wxSheetCellAttr::MergeWith(const wxSheetCellAttr &other)
{
    wxCHECK_MSG(Ok() && other.Ok(), false, wxT("this or Attr to MergeWith from is not created"));
    
    if ( !HasForegoundColour() && other.HasForegoundColour() )
        SetForegroundColour(other.GetForegroundColour());
    if ( !HasBackgroundColour() && other.HasBackgroundColour() )
        SetBackgroundColour(other.GetBackgroundColour());
    if ( !HasFont() && other.HasFont() )
        SetFont(other.GetFont());
    if ( !HasAlignment() && other.HasAlignment() )
        SetAlignment(other.GetAlignment());
    if ( !HasOrientation() && other.HasOrientation() )
        SetOrientation(other.GetOrientation());
    if ( !HasLevel() && other.HasLevel() )
        SetLevel(other.GetLevel());
    if ( !HasReadWriteMode() && other.HasReadWriteMode() )
        SetReadOnly(other.GetReadOnly());
    if ( !HasOverflowMode() && other.HasOverflowMode() )
        SetOverflow(other.GetOverflow());
    if ( !HasOverflowMarkerMode() && other.HasOverflowMarkerMode() )
        SetOverflowMarker(other.GetOverflowMarker());
    if ( !HasShowEditorMode() && other.HasShowEditorMode() )
        SetShowEditor(other.GetShowEditor());
    
    // Directly access m_renderer/m_editor as GetRender/Editor may return different one

    // Maybe add support for merge of Render and Editor?
    if ( !HasRenderer() && other.HasRenderer() )
        SetRenderer(((wxSheetCellAttrRefData*)other.m_refData)->m_renderer->Clone());
    if ( !HasEditor() && other.HasEditor() )
        SetEditor(((wxSheetCellAttrRefData*)other.m_refData)->m_editor->Clone());

    if ( !HasDefaultAttr() && other.HasDefaultAttr() )
        SetDefaultAttr(other.GetDefaultAttr());
    
    return true;
}

void wxSheetCellAttr::SetForegroundColour(const wxColour& foreColour) 
{ 
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));
    M_CELLATTRDATA->m_foreColour = foreColour; 
}
void wxSheetCellAttr::SetBackgroundColour(const wxColour& backColour) 
{ 
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));
    M_CELLATTRDATA->m_backColour = backColour; 
}
void wxSheetCellAttr::SetFont(const wxFont& font) 
{ 
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));
    M_CELLATTRDATA->m_font = font; 
}
void wxSheetCellAttr::SetAlignment(int align)
{
    SetType(align, wxSHEET_AttrAlignType_Mask);
}
void wxSheetCellAttr::SetAlignment(int horizAlign, int vertAlign)
{
    SetType(wxSheet::SetAlignment(M_CELLATTRDATA->m_attrTypes, horizAlign, vertAlign), 
            wxSHEET_AttrAlignType_Mask);
}
void wxSheetCellAttr::SetOrientation(int orientation)
{
    SetType(orientation, wxSHEET_AttrOrientType_Mask);
}
void wxSheetCellAttr::SetLevel(wxSheetAttrLevel_Type level)
{
    SetType(level, wxSHEET_AttrLevelType_Mask);
}
void wxSheetCellAttr::SetOverflow(bool allow)
{ 
    SetType(allow ? wxSHEET_AttrOverflow : wxSHEET_AttrOverflowNot,
            wxSHEET_AttrOverflowType_Mask);
}
void wxSheetCellAttr::SetOverflowMarker(bool draw_marker)
{ 
    SetType(draw_marker ? wxSHEET_AttrOverflowMarker : wxSHEET_AttrOverflowMarkerNot,
            wxSHEET_AttrOverflowMarkerType_Mask);
}
void wxSheetCellAttr::SetShowEditor(bool show_editor)
{ 
    SetType(show_editor ? wxSHEET_AttrShowEditor : wxSHEET_AttrShowEditorNot, 
            wxSHEET_AttrShowEditorType_Mask);
}
void wxSheetCellAttr::SetReadOnly(bool isReadOnly)
{ 
    SetType(isReadOnly ? wxSHEET_AttrReadOnly : wxSHEET_AttrReadWrite, 
            wxSHEET_AttrReadType_Mask);
}
void wxSheetCellAttr::SetRenderer(const wxSheetCellRenderer& renderer)
{ 
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));
    if (M_CELLATTRDATA->m_renderer)
    {
        delete M_CELLATTRDATA->m_renderer;
        M_CELLATTRDATA->m_renderer = NULL;
    }
    if (renderer.Ok())
        M_CELLATTRDATA->m_renderer = new wxSheetCellRenderer(renderer); 
}
void wxSheetCellAttr::SetEditor(const wxSheetCellEditor& editor)
{ 
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));
    if (M_CELLATTRDATA->m_editor)
    {
        delete M_CELLATTRDATA->m_editor;
        M_CELLATTRDATA->m_editor = NULL;
    }
    if (editor.Ok())
        M_CELLATTRDATA->m_editor = new wxSheetCellEditor(editor); 
}
void wxSheetCellAttr::SetKind(wxSheetAttr_Type kind) 
{ 
    SetType(kind, wxSHEET_AttrAny);
}

bool wxSheetCellAttr::HasForegoundColour() const       
{ 
    wxCHECK_MSG(m_refData, false, wxT("wxSheetCellAttr not initializied"));
    return M_CELLATTRDATA->m_foreColour.Ok(); 
}
bool wxSheetCellAttr::HasBackgroundColour() const 
{ 
    wxCHECK_MSG(m_refData, false, wxT("wxSheetCellAttr not initializied"));
    return M_CELLATTRDATA->m_backColour.Ok(); 
}
bool wxSheetCellAttr::HasFont() const             
{ 
    wxCHECK_MSG(m_refData, false, wxT("wxSheetCellAttr not initializied"));
    return M_CELLATTRDATA->m_font.Ok(); 
}
bool wxSheetCellAttr::HasAlignment() const        
{ 
    wxCHECK_MSG(m_refData, false, wxT("wxSheetCellAttr not initializied"));
    // note: Left and Top is 0 
    return (((M_CELLATTRDATA->m_attrTypes) & wxSHEET_AttrAlignHorizUnset) == 0) && 
           (((M_CELLATTRDATA->m_attrTypes) & wxSHEET_AttrAlignVertUnset ) == 0);
}
bool wxSheetCellAttr::HasOrientation() const        
{ 
    wxCHECK_MSG(m_refData, false, wxT("wxSheetCellAttr not initializied"));
    return ((M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrOrientHoriz) != 0) || 
           ((M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrOrientVert ) != 0);    
}
bool wxSheetCellAttr::HasLevel() const        
{ 
    return HasType(wxSHEET_AttrLevelType_Mask);
}
bool wxSheetCellAttr::HasOverflowMode() const     
{ 
    return HasType(wxSHEET_AttrOverflowType_Mask);
}
bool wxSheetCellAttr::HasOverflowMarkerMode() const     
{ 
    return HasType(wxSHEET_AttrOverflowMarkerType_Mask);
}
bool wxSheetCellAttr::HasShowEditorMode() const     
{ 
    return HasType(wxSHEET_AttrShowEditorType_Mask);
}
bool wxSheetCellAttr::HasReadWriteMode() const    
{ 
    return HasType(wxSHEET_AttrReadType_Mask);
}
bool wxSheetCellAttr::HasRenderer() const         
{ 
    wxCHECK_MSG(m_refData, false, wxT("wxSheetCellAttr not initializied"));
    return M_CELLATTRDATA->m_renderer && M_CELLATTRDATA->m_renderer->Ok(); 
}
bool wxSheetCellAttr::HasEditor() const           
{ 
    wxCHECK_MSG(m_refData, false, wxT("wxSheetCellAttr not initializied"));
    return M_CELLATTRDATA->m_editor && M_CELLATTRDATA->m_editor->Ok(); 
}
bool wxSheetCellAttr::HasDefaultAttr() const           
{ 
    wxCHECK_MSG(m_refData, false, wxT("wxSheetCellAttr not initializied"));
    return M_CELLATTRDATA->m_defaultAttr && M_CELLATTRDATA->m_defaultAttr->Ok(); 
}
bool wxSheetCellAttr::IsComplete() const 
{ 
    return Ok() && HasForegoundColour() && HasBackgroundColour() && HasFont() &&
           HasAlignment() && HasOverflowMode() && HasLevel() && 
           HasReadWriteMode() && HasRenderer() && HasEditor();
}

const wxColour& wxSheetCellAttr::GetForegroundColour() const
{
    wxCHECK_MSG(m_refData, *wxBLACK, wxT("wxSheetCellAttr not initializied"));
    if (HasForegoundColour())
        return M_CELLATTRDATA->m_foreColour;
    else if (GetDefaultAttr().Ok()) // && GetDefaultAttr().HasForegoundColour())
        return GetDefaultAttr().GetForegroundColour();

    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return *wxBLACK;
}
const wxColour& wxSheetCellAttr::GetBackgroundColour() const
{
    wxCHECK_MSG(m_refData, *wxWHITE, wxT("wxSheetCellAttr not initializied"));
    if (HasBackgroundColour())
        return M_CELLATTRDATA->m_backColour;
    else if (GetDefaultAttr().Ok()) // && GetDefaultAttr().HasBackgroundColour())
        return GetDefaultAttr().GetBackgroundColour();

    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return *wxWHITE;
}
const wxFont& wxSheetCellAttr::GetFont() const
{
    wxCHECK_MSG(m_refData, *wxNORMAL_FONT, wxT("wxSheetCellAttr not initializied"));
    if (HasFont())
        return M_CELLATTRDATA->m_font;
    else if (GetDefaultAttr().Ok()) // && GetDefaultAttr().HasFont())
        return GetDefaultAttr().GetFont();

    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return *wxNORMAL_FONT;
}
int wxSheetCellAttr::GetAlignment() const
{
    wxCHECK_MSG(m_refData, wxSHEET_AttrAlignLeft|wxSHEET_AttrAlignCenterVert, wxT("wxSheetCellAttr not initializied"));
    if (HasAlignment())
        return M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrAlignType_Mask;
    else if (GetDefaultAttr().Ok()) // && GetDefaultAttr().HasAlignment())
        return GetDefaultAttr().GetAlignment();
    
    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return wxSHEET_AttrAlignLeft|wxSHEET_AttrAlignCenterVert;
}
wxOrientation wxSheetCellAttr::GetOrientation() const
{
    wxCHECK_MSG(m_refData, wxHORIZONTAL, wxT("wxSheetCellAttr not initializied"));
    if (HasOrientation())
        return wxOrientation(M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrOrientType_Mask);
    else if (GetDefaultAttr().Ok()) // && GetDefaultAttr().HasOrientation())
        return GetDefaultAttr().GetOrientation();
    
    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return wxHORIZONTAL;
}
wxSheetAttrLevel_Type wxSheetCellAttr::GetLevel() const
{
    wxCHECK_MSG(m_refData, wxSHEET_AttrLevelTop, wxT("wxSheetCellAttr not initializied"));
    if (HasLevel())
        return wxSheetAttrLevel_Type(M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrLevelType_Mask);
    else if (GetDefaultAttr().Ok()) // && GetDefaultAttr().HasLevel())
        return GetDefaultAttr().GetLevel();
    
    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return wxSHEET_AttrLevelTop;
}
bool wxSheetCellAttr::GetOverflow() const
{
    wxCHECK_MSG(m_refData, false, wxT("wxSheetCellAttr not initializied"));
    if (HasOverflowMode())
        return (M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrOverflow) != 0; 
    else if (GetDefaultAttr().Ok()) // && GetDefaultAttr().HasOverflowMode())
        return GetDefaultAttr().GetOverflow();
    
    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return false;    
}
bool wxSheetCellAttr::GetOverflowMarker() const
{
    wxCHECK_MSG(m_refData, false, wxT("wxSheetCellAttr not initializied"));
    if (HasOverflowMarkerMode())
        return (M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrOverflowMarker) != 0; 
    else if (GetDefaultAttr().Ok()) // && GetDefaultAttr().HasOverflowMarkerMode())
        return GetDefaultAttr().GetOverflowMarker();
    
    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return false;    
}
bool wxSheetCellAttr::GetShowEditor() const
{
    wxCHECK_MSG(m_refData, false, wxT("wxSheetCellAttr not initializied"));
    if (HasShowEditorMode())
        return (M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrShowEditor) != 0; 
    else if (GetDefaultAttr().Ok()) // && GetDefaultAttr().HasShowEditorMode())
        return GetDefaultAttr().GetShowEditor();
    
    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return false;    
}
bool wxSheetCellAttr::GetReadOnly() const 
{ 
    wxCHECK_MSG(m_refData, false, wxT("wxSheetCellAttr not initializied"));
    if (HasReadWriteMode())
        return (M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrReadOnly) != 0; 
    else if (GetDefaultAttr().Ok()) // && GetDefaultAttr().HasReadWriteMode())
        return GetDefaultAttr().GetReadOnly();
    
    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return false;    
}

// Labenski - the logic is now different, I didn't understand what it was
// trying to do and setting the default editor/renderer was broken.

// Old wxGrid logic which didn't allow you to set a default anything

// "GetRenderer and GetEditor use a slightly different decision path about
// which attribute to use.  If a non-default attr object has one then it is
// used, otherwise the default editor or renderer is fetched from the sheet and
// used.  It should be the default for the data type of the cell.  If it is
// NULL (because the table has a type that the sheet does not have in its
// registry,) then the sheet's default editor or renderer is used."

wxSheetCellRenderer wxSheetCellAttr::GetRenderer(wxSheet* sheet, const wxSheetCoords& coords) const
{
    wxCHECK_MSG(m_refData, wxNullSheetCellRenderer, wxT("wxSheetCellAttr not initializied"));
    wxSheetCellRenderer renderer;

    // first try to get the renderer for the cell from the sheet, if this is default
    if (sheet && (sheet->GetDefaultAttr(coords) == *this))
        renderer = sheet->GetDefaultRendererForCell(coords);
    
    // then see if this has a renderer and return it if valid
    if ( !renderer.Ok() )
    {
        if ( M_CELLATTRDATA->m_renderer && M_CELLATTRDATA->m_renderer->Ok() )
            renderer = *M_CELLATTRDATA->m_renderer;
        else 
        {
            // couldn't get it from the sheet try attr default
            wxSheetCellAttr defAttr(GetDefaultAttr());
            if ( defAttr.Ok() )
                renderer = defAttr.GetRenderer(sheet, coords);
        }
    }

    // we're supposed to always find something
    wxASSERT_MSG(renderer.Ok(), wxT("Missing default cell renderer"));
    return renderer;
}
wxSheetCellEditor wxSheetCellAttr::GetEditor(wxSheet* sheet, const wxSheetCoords& coords) const
{
    wxCHECK_MSG(m_refData, wxNullSheetCellEditor, wxT("wxSheetCellAttr not initializied"));
    wxSheetCellEditor editor;

    // first try to get the editor for the cell from the sheet, if this is default
    if (sheet && (sheet->GetDefaultAttr(coords) == *this))
        editor = sheet->GetDefaultEditorForCell(coords);
    
    // then see if this has a editor and return it if valid
    if ( !editor.Ok() )
    {
        if ( M_CELLATTRDATA->m_editor && M_CELLATTRDATA->m_editor->Ok() )
            editor = *M_CELLATTRDATA->m_editor;
        else 
        {
            // couldn't get it from the sheet try attr default
            wxSheetCellAttr defAttr(GetDefaultAttr());
            if ( defAttr.Ok() )
                editor = defAttr.GetEditor(sheet, coords);
        }
    }

    // we're supposed to always find something
    wxASSERT_MSG(editor.Ok(), wxT("Missing default cell editor"));
    return editor;    
}
wxSheetAttr_Type wxSheetCellAttr::GetKind() const
{ 
    wxCHECK_MSG(m_refData, wxSHEET_AttrCell, wxT("wxSheetCellAttr not initializied"));
    return wxSheetAttr_Type(M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrAny); 
}

const wxSheetCellAttr& wxSheetCellAttr::GetDefaultAttr() const
{
    wxCHECK_MSG(m_refData, wxNullSheetCellAttr, wxT("wxSheetCellAttr not initializied"));
    if (M_CELLATTRDATA->m_defaultAttr != NULL)
        return *M_CELLATTRDATA->m_defaultAttr;
    
    return wxNullSheetCellAttr;
}
void wxSheetCellAttr::SetDefaultAttr(const wxSheetCellAttr& defAttr) 
{ 
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));    
    if (M_CELLATTRDATA->m_defaultAttr)
    {
        delete M_CELLATTRDATA->m_defaultAttr;
        M_CELLATTRDATA->m_defaultAttr = NULL;
    }
    // don't set defAttr to this, you don't gain anything, but a memory leak
    //if (defAttr.Ok() && (defAttr.m_refData != m_refData))
    //    M_CELLATTRDATA->m_defaultAttr = new wxSheetCellAttr(defAttr);
    
    if (defAttr.Ok())
    {
        wxSheetCellAttr attr(defAttr);
        while (attr.Ok())
        {
            wxCHECK_RET((*this) != attr, wxT("wxSheet attributes are linked recursively"));
            attr = attr.GetDefaultAttr();
        }

        M_CELLATTRDATA->m_defaultAttr = new wxSheetCellAttr(defAttr);
    }
}

void wxSheetCellAttr::SetType(int type, int mask)
{
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));
    M_CELLATTRDATA->m_attrTypes &= ~mask;          // clear old
    M_CELLATTRDATA->m_attrTypes |= (mask ? (type & mask) : type);  // set new
}
int wxSheetCellAttr::GetType(int mask) const
{
    wxCHECK_MSG(m_refData, 0, wxT("wxSheetCellAttr not initializied"));
    return (M_CELLATTRDATA->m_attrTypes & mask);
}

// ----------------------------------------------------------------------------
// wxSheetCellAttrProvider
// ----------------------------------------------------------------------------
wxSheetCellAttr wxSheetCellAttrProvider::GetAttr(const wxSheetCoords& coords,
                                                 wxSheetAttr_Type type )
{   
    switch (wxSheet::GetCellCoordsType(coords))
    {
        case wxSHEET_CELL_GRID :
        {
            switch (type)
            {
                case wxSHEET_AttrAny:
                {
                    wxSheetCellAttr attrCell(m_cellAttrs.GetValue(coords));
                    wxSheetCellAttr attrRow(m_rowAttrs.GetValue(coords.GetRow()));
                    wxSheetCellAttr attrCol(m_colAttrs.GetValue(coords.GetCol()));

                    // points system, greater the level you get added last
                    // if two levels match use ordering col, row, cell on top.
                    wxPairArrayIntSheetCellAttr data;
                    if (attrCol.Ok())  data.SetValue(attrCol.GetLevel()+2, attrCol);
                    if (attrRow.Ok())  data.SetValue(attrRow.GetLevel()+1, attrRow);
                    if (attrCell.Ok()) data.SetValue(attrCell.GetLevel()+0, attrCell);

                    // the attr are sorted from low priority to high
                    const int count = data.GetCount();

                    switch (count)
                    {
                        case 0  : return wxNullSheetCellAttr;
                        case 1  : return data.GetItemValue(0);
                        default : break;
                    }

                    // This is the preferred way - link them together
                    // but you must make sure that it doesn't loop
                    // see check in wxSheetCellAttr::SetDefaultAttr
                    wxSheetCellAttr attr;
                    attr.Copy(data.GetItemValue(0));
                    attr.SetDefaultAttr(data.GetItemValue(1));
                    if (count > 2)
                        data.GetItemValue(1).SetDefaultAttr(data.GetItemValue(2));
/*
                    // This is the old way of copying the values to a single attr
                    wxSheetCellAttr attr;
                    attr.Copy(data.GetItemValue(0));
                    attr.MergeWith(data.GetItemValue(1));
                    if (count > 2)
                       attr.MergeWith(data.GetItemValue(2));
*/
                    // This is not a "real" attr but created
                    attr.SetKind(wxSHEET_AttrAny);
                    
                    return attr;
                }
                case wxSHEET_AttrCell : return m_cellAttrs.GetValue(coords);
                case wxSHEET_AttrRow  : return m_rowAttrs.GetValue(coords.GetRow());
                case wxSHEET_AttrCol  : return m_colAttrs.GetValue(coords.GetCol());
                default:
                    // unused as yet...
                    // (wxSheetCellAttr::Default):
                    // (wxSheetCellAttr::Merged):
                    break;
            }
            break;
        }
        case wxSHEET_CELL_ROWLABEL : return m_rowLabelAttrs.GetValue(coords.m_row);
        case wxSHEET_CELL_COLLABEL : return m_colLabelAttrs.GetValue(coords.m_col);
        default :
            wxFAIL_MSG(wxString::Format(wxT("Unhandled coords (%d, %d) in wxSheetCellAttrProvider::GetAttr"), coords.m_row, coords.m_col));
            break;
    }

    return wxNullSheetCellAttr;
}

void wxSheetCellAttrProvider::SetAttr(const wxSheetCoords& coords, 
                                      const wxSheetCellAttr &attr, 
                                      wxSheetAttr_Type type)
{
    switch (wxSheet::GetCellCoordsType(coords))
    {
        case wxSHEET_CELL_GRID :
        {
            switch (type)
            {   
                case wxSHEET_AttrCell :
                {
                    if (attr.Ok()) m_cellAttrs.SetValue(coords, attr);
                    else           m_cellAttrs.RemoveValue(coords);            
                    return;
                } 
                case wxSHEET_AttrRow :
                {
                    if (attr.Ok()) m_rowAttrs.SetValue(coords.m_row, attr);
                    else           m_rowAttrs.RemoveValue(coords.m_row);
                    return;
                }
                case wxSHEET_AttrCol :
                {
                    if (attr.Ok()) m_colAttrs.SetValue(coords.m_col, attr);
                    else           m_colAttrs.RemoveValue(coords.m_col);
                    return;
                }
                default :
                {
                    wxFAIL_MSG(wxT("Unhandled attr type for grid coords"));
                    return;
                }
            }
            break;
        }
        case wxSHEET_CELL_ROWLABEL :
        {
            switch (type)
            {
                case wxSHEET_AttrCell :
                {
                    if (attr.Ok()) m_rowLabelAttrs.SetValue(coords.m_row, attr);
                    else           m_rowLabelAttrs.RemoveValue(coords.m_row);
                    return;
                } 
                default : 
                {
                    wxFAIL_MSG(wxT("Unhandled attr type for row label coords"));
                    return;
                }
            }
            break;
        }
        case wxSHEET_CELL_COLLABEL :
        {
            switch (type)
            {
                case wxSHEET_AttrCell :
                {
                    if (attr.Ok()) m_colLabelAttrs.SetValue(coords.m_col, attr);
                    else           m_colLabelAttrs.RemoveValue(coords.m_col);
                    return;
                } 
                default : 
                {
                    wxFAIL_MSG(wxT("Unhandled attr type for col label coords"));
                    return;
                }
            }
            break;
        }
        default :
            wxFAIL_MSG(wxString::Format(wxT("Unhandled coords (%d, %d) in wxSheetCellAttrProvider::SetAttr"), coords.m_row, coords.m_col));
            break;
    }
}
void wxSheetCellAttrProvider::UpdateRows( size_t row, int numRows, int update )
{
    // no errors here since the pair arrays don't need to know about size
    if ((update & wxSHEET_UpdateGridCellAttrs) != 0)
        m_rowAttrs.UpdatePos( row, numRows );
    if ((update & wxSHEET_UpdateGridCellAttrs) != 0)
        m_cellAttrs.UpdateRows( row, numRows );    
    if ((update & wxSHEET_UpdateRowLabelAttrs) != 0)
        m_rowLabelAttrs.UpdatePos( row, numRows );
}

void wxSheetCellAttrProvider::UpdateCols( size_t col, int numCols, int update )
{
    if ((update & wxSHEET_UpdateGridCellAttrs) != 0)
        m_colAttrs.UpdatePos( col, numCols );
    if ((update & wxSHEET_UpdateGridCellAttrs) != 0)
        m_cellAttrs.UpdateCols( col, numCols );
    if ((update & wxSHEET_UpdateColLabelAttrs) != 0)
        m_colLabelAttrs.UpdatePos( col, numCols );
}
