///////////////////////////////////////////////////////////////////////////
// Name:        sheet.cpp
// Purpose:     wxSheet and related classes
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: John Labenski, Robin Dunn, Vadim Zeitlin
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, Michael Bedward (mbedward@ozemail.com.au)
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "sheet.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#include "wx/defs.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_GRID

#include "sheet.h"
#include "sheetsel.h"
#include "sheetctl.h"
#include "sheetspt.h" // only need wxEVT_SHEET_SPLIT_BEGIN

#ifndef WX_PRECOMP
    #include "wx/utils.h"
    #include "wx/dcclient.h"
    #include "wx/settings.h"
    #include "wx/log.h"
    #include "wx/dcscreen.h"
#endif

#include "wx/timer.h"
#include "wx/clipbrd.h"

// Required for wxIs... functions
#include <ctype.h>

#define PRINT_BLOCK(s, b) wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), s, b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());
#define PRINT_RECT(s, b)  wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), s, b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());

#define SPLIT_BUTTON_WIDTH 6

static bool wxRectIsEmpty(const wxRect& rect)
{
    return (rect.width < 1) || (rect.height < 1);
}

// ----------------------------------------------------------------------------
// conditional compilation
// ----------------------------------------------------------------------------

//#ifndef WXSHEET_DRAW_LINES
//#define WXSHEET_DRAW_LINES 1
//#endif

#define WXSHEET_DRAW_LINES 0

// ----------------------------------------------------------------------------
// globals
// ----------------------------------------------------------------------------

//#define DEBUG_ATTR_CACHE
#ifdef DEBUG_ATTR_CACHE
    static size_t gs_nAttrCacheHits = 0;
    static size_t gs_nAttrCacheMisses = 0;
#endif // DEBUG_ATTR_CACHE

// ----------------------------------------------------------------------------
// constants
// ----------------------------------------------------------------------------

// scroll line size
// TODO: this doesn't work at all, sheet cells have different sizes and approx
//       calculations don't work as because of the size mismatch scrollbars
//       sometimes fail to be shown when they should be or vice versa
//
//       The scroll bars may be a little flakey once in a while, but that is
//       surely much less horrible than having scroll lines of only 1!!!
//       -- Robin
//
//       Well, it's still seriously broken so it might be better but needs
//       fixing anyhow
//       -- Vadim
static const size_t SHEET_SCROLL_LINE_X = 15;  // 1;
static const size_t SHEET_SCROLL_LINE_Y = SHEET_SCROLL_LINE_X;

// the size of hash tables used a bit everywhere (the max number of elements
// in these hash tables is the number of rows/columns)
static const int SHEET_HASH_SIZE = 100;

const wxSheetCellAttr wxNullSheetCellAttr(FALSE);

#include "wx/arrimpl.cpp"
WX_DEFINE_OBJARRAY(wxArraySheetCellAttr)
WX_DEFINE_OBJARRAY(wxArrayPairArrayIntSheetString)
DEFINE_PAIRED_DATA_ARRAYS(wxSheetCoords, wxSheetCellAttr, wxPairArraySheetCoordsCellAttrBase)
DEFINE_PAIRED_DATA_ARRAYS(int, wxSheetCellAttr, wxPairArrayIntSheetCellAttr)
DEFINE_PAIRED_DATA_ARRAYS(int, wxString, wxPairArrayIntSheetString)
DEFINE_PAIRED_DATA_ARRAYS(int, wxPairArrayIntSheetString, wxPairArrayIntPairArraySheetStringBase)

// ----------------------------------------------------------------------------
// private functions
// ----------------------------------------------------------------------------

// Checks and returns a suitable horiz or vert alignment, if invalid replaces
//   with wxSheetCellAttr::NoHorizAlign/NoVertAlign
static int CheckAlignment(int align)
{
    // note: it's a shame that wxALIGN_LEFT = wxALIGN_TOP = 0
    
    int count = 0;
    if (align & wxSHEET_AttrAlignRight != 0) count++;
    if (align & wxSHEET_AttrAlignCenterHoriz != 0) count++;
    if (align & wxSHEET_AttrAlignHorizUnset != 0) count++;
    if (count > 1)
    {
        align &= ~wxSHEET_AttrAlignHoriz_Mask;      // clear whatever is there
        align |= wxSHEET_AttrAlignHorizUnset;  // set to no alignment
    }
    
    count = 0;
    if (align & wxSHEET_AttrAlignBottom != 0) count++;
    if (align & wxSHEET_AttrAlignCenterVert != 0) count++;
    if (align & wxSHEET_AttrAlignVertUnset != 0) count++;
    if (count > 1)
    {
        align &= ~wxSHEET_AttrAlignVert_Mask;      // clear whatever is there
        align |= wxSHEET_AttrAlignVertUnset;  // set to no alignment
    }
    
    return align;
}

static bool AlignHasAlignment(int align)
{
    return ((align & wxSHEET_AttrAlignHorizUnset) == 0) &&
           ((align & wxSHEET_AttrAlignVertUnset ) == 0);
}

int wxSheet::SetAlignment(int orig_align, int hAlign, int vAlign)
{
    if (hAlign != -1)
    {
        orig_align &= ~wxSHEET_AttrAlignHoriz_Mask;         // clear old
        orig_align |= hAlign & wxSHEET_AttrAlignHoriz_Mask; // set new
    }
    if (vAlign != -1)
    {
        orig_align &= ~wxSHEET_AttrAlignVert_Mask;
        orig_align |= vAlign & wxSHEET_AttrAlignVert_Mask;
    }
    
    return CheckAlignment(orig_align);
}

static bool OrientHasOrientation(int orientation)
{
    return ((orientation & wxSHEET_AttrOrientHoriz) != 0) || 
           ((orientation & wxSHEET_AttrOrientVert ) != 0);
}

// ----------------------------------------------------------------------------
// wxSheetCellAttrRefData
// ----------------------------------------------------------------------------
#define M_CELLATTRDATA ((wxSheetCellAttrRefData*)m_refData)

wxSheetCellAttrRefData::wxSheetCellAttrRefData() 
        : m_attrTypes(wxSHEET_AttrCell|wxSHEET_AttrAlignUnset),  // other unset are 0
          m_renderer(NULL), m_editor(NULL), m_defaultAttr(NULL) {}

wxSheetCellAttrRefData::~wxSheetCellAttrRefData()
{
    if (m_renderer)    delete m_renderer;
    if (m_editor)      delete m_editor;
    if (m_defaultAttr) delete m_defaultAttr;
}

// ----------------------------------------------------------------------------
// wxSheetCellAttr
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(wxSheetCellAttr, wxObject)

wxSheetCellAttr::wxSheetCellAttr(bool create)
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

bool wxSheetCellAttr::Copy(const wxSheetCellAttr& other) 
{ 
    wxCHECK_MSG(other.Ok(), FALSE, wxT("Attr to copy from is not created"));
    
    if (!Create())
        return FALSE;
    
    M_CELLATTRDATA->m_foreColour = ((wxSheetCellAttrRefData*)other.GetRefData())->m_foreColour;
    M_CELLATTRDATA->m_backColour = ((wxSheetCellAttrRefData*)other.GetRefData())->m_backColour;
    M_CELLATTRDATA->m_font       = ((wxSheetCellAttrRefData*)other.GetRefData())->m_font;
    M_CELLATTRDATA->m_attrTypes  = ((wxSheetCellAttrRefData*)other.GetRefData())->m_attrTypes;
    if (other.HasEditor())
        SetEditor(((wxSheetCellAttrRefData*)other.m_refData)->m_editor->Clone());
    if (other.HasRenderer())
        SetRenderer(((wxSheetCellAttrRefData*)other.m_refData)->m_renderer->Clone());
    
    SetDefaultAttr(other.GetDefaultAttr());
    return TRUE;
}

bool wxSheetCellAttr::UpdateWith(const wxSheetCellAttr& other) 
{ 
    wxCHECK_MSG(Ok() && other.Ok(), FALSE, wxT("this or Attr to UpdateWith from is not created"));

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
    
    // Directly access m_renderer/m_editor as GetRender/Editor may return different one

    // Maybe add support for merge of Render and Editor?
    if ( other.HasRenderer() )
        SetRenderer(((wxSheetCellAttrRefData*)other.m_refData)->m_renderer->Clone());
    if ( other.HasEditor() )
        SetEditor(((wxSheetCellAttrRefData*)other.m_refData)->m_editor->Clone());

    if ( other.HasDefaultAttr() )
        SetDefaultAttr(other.GetDefaultAttr());
    
    return TRUE;
}
    
bool wxSheetCellAttr::MergeWith(const wxSheetCellAttr &other)
{
    wxCHECK_MSG(Ok() && other.Ok(), FALSE, wxT("this or Attr to MergeWith from is not created"));
    
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
    
    // Directly access m_renderer/m_editor as GetRender/Editor may return different one

    // Maybe add support for merge of Render and Editor?
    if ( !HasRenderer() && other.HasRenderer() )
        SetRenderer(((wxSheetCellAttrRefData*)other.m_refData)->m_renderer->Clone());
    if ( !HasEditor() && other.HasEditor() )
        SetEditor(((wxSheetCellAttrRefData*)other.m_refData)->m_editor->Clone());

    if ( !HasDefaultAttr() && other.HasDefaultAttr() )
        SetDefaultAttr(other.GetDefaultAttr());
    
    return TRUE;
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
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));
    M_CELLATTRDATA->m_attrTypes &= ~wxSHEET_AttrAlignType_Mask; // clear old
    M_CELLATTRDATA->m_attrTypes |= (align & wxSHEET_AttrAlignType_Mask);  // set new
}
void wxSheetCellAttr::SetAlignment(int horizAlign, int vertAlign)
{
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));
    M_CELLATTRDATA->m_attrTypes &= ~wxSHEET_AttrAlignType_Mask; // clear old
    M_CELLATTRDATA->m_attrTypes |= wxSheet::SetAlignment(M_CELLATTRDATA->m_attrTypes, horizAlign, vertAlign);
}
void wxSheetCellAttr::SetOrientation(int orientation)
{
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));
    M_CELLATTRDATA->m_attrTypes &= ~wxSHEET_AttrOrientType_Mask;                 // clear old
    M_CELLATTRDATA->m_attrTypes |= (orientation & wxSHEET_AttrOrientType_Mask);  // set new
}
void wxSheetCellAttr::SetLevel(wxSheetAttrLevel_Type level)
{
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));
    M_CELLATTRDATA->m_attrTypes &= ~wxSHEET_AttrLevelType_Mask;           // clear old
    M_CELLATTRDATA->m_attrTypes |= (level & wxSHEET_AttrLevelType_Mask);  // set new
}
void wxSheetCellAttr::SetOverflow(bool allow)
{ 
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));
    M_CELLATTRDATA->m_attrTypes &= ~wxSHEET_AttrOverflowType_Mask;      // clear old
    M_CELLATTRDATA->m_attrTypes |= (allow ? wxSHEET_AttrOverflow : wxSHEET_AttrOverflowNot); 
}
void wxSheetCellAttr::SetOverflowMarker(bool draw_marker)
{ 
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));
    M_CELLATTRDATA->m_attrTypes &= ~wxSHEET_AttrOverflowMarkerType_Mask;      // clear old
    M_CELLATTRDATA->m_attrTypes |= (draw_marker ? wxSHEET_AttrOverflowMarker : wxSHEET_AttrOverflowMarkerNot); 
}
void wxSheetCellAttr::SetReadOnly(bool isReadOnly)
{ 
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));
    M_CELLATTRDATA->m_attrTypes &= ~wxSHEET_AttrReadType_Mask;          // clear old
    M_CELLATTRDATA->m_attrTypes |= (isReadOnly ? wxSHEET_AttrReadOnly : wxSHEET_AttrReadWrite); 
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
    wxCHECK_RET(m_refData, wxT("wxSheetCellAttr not initializied"));
    M_CELLATTRDATA->m_attrTypes &= ~wxSHEET_AttrAny;  // clear old
    M_CELLATTRDATA->m_attrTypes |= (kind & wxSHEET_AttrAny); 
}

bool wxSheetCellAttr::HasForegoundColour() const       
{ 
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
    return M_CELLATTRDATA->m_foreColour.Ok(); 
}
bool wxSheetCellAttr::HasBackgroundColour() const 
{ 
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
    return M_CELLATTRDATA->m_backColour.Ok(); 
}
bool wxSheetCellAttr::HasFont() const             
{ 
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
    return M_CELLATTRDATA->m_font.Ok(); 
}
bool wxSheetCellAttr::HasAlignment() const        
{ 
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
    return AlignHasAlignment(M_CELLATTRDATA->m_attrTypes);
}
bool wxSheetCellAttr::HasOrientation() const        
{ 
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
    return OrientHasOrientation(M_CELLATTRDATA->m_attrTypes);
}
bool wxSheetCellAttr::HasLevel() const        
{ 
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
    return (M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrLevelType_Mask) != 0;
}
bool wxSheetCellAttr::HasOverflowMode() const     
{ 
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
    return (M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrOverflowType_Mask) != 0; 
}
bool wxSheetCellAttr::HasOverflowMarkerMode() const     
{ 
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
    return (M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrOverflowMarkerType_Mask) != 0; 
}
bool wxSheetCellAttr::HasReadWriteMode() const    
{ 
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
    return (M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrReadType_Mask) != 0; 
}
bool wxSheetCellAttr::HasRenderer() const         
{ 
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
    return M_CELLATTRDATA->m_renderer && M_CELLATTRDATA->m_renderer->Ok(); 
}
bool wxSheetCellAttr::HasEditor() const           
{ 
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
    return M_CELLATTRDATA->m_editor && M_CELLATTRDATA->m_editor->Ok(); 
}
bool wxSheetCellAttr::HasDefaultAttr() const           
{ 
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
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
    else if (GetDefaultAttr().Ok() && GetDefaultAttr().HasForegoundColour())
        return GetDefaultAttr().GetForegroundColour();

    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return *wxBLACK;
}
const wxColour& wxSheetCellAttr::GetBackgroundColour() const
{
    wxCHECK_MSG(m_refData, *wxWHITE, wxT("wxSheetCellAttr not initializied"));
    if (HasBackgroundColour())
        return M_CELLATTRDATA->m_backColour;
    else if (GetDefaultAttr().Ok() && GetDefaultAttr().HasBackgroundColour())
        return GetDefaultAttr().GetBackgroundColour();

    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return *wxWHITE;
}
const wxFont& wxSheetCellAttr::GetFont() const
{
    wxCHECK_MSG(m_refData, *wxNORMAL_FONT, wxT("wxSheetCellAttr not initializied"));
    if (HasFont())
        return M_CELLATTRDATA->m_font;
    else if (GetDefaultAttr().Ok() && GetDefaultAttr().HasFont())
        return GetDefaultAttr().GetFont();

    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return *wxNORMAL_FONT;
}
int wxSheetCellAttr::GetAlignment() const
{
    wxCHECK_MSG(m_refData, wxSHEET_AttrAlignLeft|wxSHEET_AttrAlignCenterVert, wxT("wxSheetCellAttr not initializied"));
    if (HasAlignment())
        return M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrAlignType_Mask;
    else if (GetDefaultAttr().Ok() && GetDefaultAttr().HasAlignment())
        return GetDefaultAttr().GetAlignment();
    
    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return wxSHEET_AttrAlignLeft|wxSHEET_AttrAlignCenterVert;
}
wxOrientation wxSheetCellAttr::GetOrientation() const
{
    wxCHECK_MSG(m_refData, wxHORIZONTAL, wxT("wxSheetCellAttr not initializied"));
    if (HasOrientation())
        return wxOrientation(M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrOrientType_Mask);
    else if (GetDefaultAttr().Ok() && GetDefaultAttr().HasOrientation())
        return GetDefaultAttr().GetOrientation();
    
    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return wxHORIZONTAL;
}
wxSheetAttrLevel_Type wxSheetCellAttr::GetLevel() const
{
    wxCHECK_MSG(m_refData, wxSHEET_AttrLevelTop, wxT("wxSheetCellAttr not initializied"));
    if (HasLevel())
        return wxSheetAttrLevel_Type(M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrLevelType_Mask);
    else if (GetDefaultAttr().Ok() && GetDefaultAttr().HasLevel())
        return GetDefaultAttr().GetLevel();
    
    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return wxSHEET_AttrLevelTop;
}
bool wxSheetCellAttr::GetOverflow() const
{
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
    if (HasOverflowMode())
        return (M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrOverflow) != 0; 
    else if (GetDefaultAttr().Ok() && GetDefaultAttr().HasOverflowMode())
        return GetDefaultAttr().GetOverflow();
    
    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return FALSE;    
}
bool wxSheetCellAttr::GetOverflowMarker() const
{
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
    if (HasOverflowMarkerMode())
        return (M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrOverflowMarker) != 0; 
    else if (GetDefaultAttr().Ok() && GetDefaultAttr().HasOverflowMarkerMode())
        return GetDefaultAttr().GetOverflowMarker();
    
    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return FALSE;    
}
bool wxSheetCellAttr::GetReadOnly() const 
{ 
    wxCHECK_MSG(m_refData, FALSE, wxT("wxSheetCellAttr not initializied"));
    if (HasReadWriteMode())
        return (M_CELLATTRDATA->m_attrTypes & wxSHEET_AttrReadOnly) != 0; 
    else if (GetDefaultAttr().Ok() && GetDefaultAttr().HasReadWriteMode())
        return GetDefaultAttr().GetReadOnly();
    
    wxFAIL_MSG(wxT("Missing default cell attribute"));
    return FALSE;    
}

// Labenski - the logic is now different, I didn't understand what it was
// trying to do and setting the default editor/renderer was broken.

// Old logic which didn't allow you to set a default anything for the sheet

// GetRenderer and GetEditor use a slightly different decision path about
// which attribute to use.  If a non-default attr object has one then it is
// used, otherwise the default editor or renderer is fetched from the sheet and
// used.  It should be the default for the data type of the cell.  If it is
// NULL (because the table has a type that the sheet does not have in its
// registry,) then the sheet's default editor or renderer is used.

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
    if (defAttr.Ok() && (defAttr.m_refData != m_refData))
        M_CELLATTRDATA->m_defaultAttr = new wxSheetCellAttr(defAttr);
}

// ----------------------------------------------------------------------------
// wxPairArraySheetCoordsCellAttr
// ----------------------------------------------------------------------------

void wxPairArraySheetCoordsCellAttr::UpdateRows( size_t pos, int numRows )
{
    if (numRows == 0) return;
    
    int n, count = GetCount();
    for ( n = 0; n < count; n++ )
    {
        wxSheetCoords& coords = GetItemKey(n);
        if (coords.m_row >= int(pos))
        {
            if ((numRows < 0) && (coords.m_row < int(pos) - numRows))
            {
                RemoveAt(n); 
                count--;
                n--; 
            }
            else
                coords.m_row += numRows;
        }
    }
}
void wxPairArraySheetCoordsCellAttr::UpdateCols( size_t pos, int numCols )
{
    if (numCols == 0) return;
    
    int n, count = GetCount();
    for ( n = 0; n < count; n++ )
    {
        wxSheetCoords& coords = GetItemKey(n);
        if (coords.m_col >= int(pos))
        {
            if ((numCols < 0) && (coords.m_col < int(pos) - numCols))
            {
                RemoveAt(n);
                count--;
                n--; 
            }
            else
                coords.m_col += numCols;
        }
    }
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

                    // simple points system, greater the level you get added last
                    // if two levels match use ordering col, row, cell on top.
                    wxPairArrayIntSheetCellAttr data;
                    if (attrCol.Ok())
                        data.SetValue(attrCol.GetLevel()+2, attrCol);
                    if (attrRow.Ok())
                        data.SetValue(attrRow.GetLevel()+1, attrRow);
                    if (attrCell.Ok())
                        data.SetValue(attrCell.GetLevel()+0, attrCell);

                    // the attr are sorted from low priorty to high
                    int count = data.GetCount();
                
                    switch (count)
                    {
                        case 0  : return wxNullSheetCellAttr;
                        case 1  : return data.GetItemValue(0);
                        default :
                        {
                            wxSheetCellAttr attr;
                            attr.Copy(data.GetItemValue(0));
                            attr.MergeWith(data.GetItemValue(1));
                            if (count > 2)
                                attr.MergeWith(data.GetItemValue(2));
                            
                            // This is not a "real" attr but created
                            attr.SetKind(wxSHEET_AttrAny);
                            return attr;
                        }
                    }
                
                    return wxNullSheetCellAttr;
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
                                      const wxSheetCellAttr &attr_, 
                                      wxSheetAttr_Type type)
{
    wxSheetCellAttr attr(attr_); // make writeable
    
    switch (wxSheet::GetCellCoordsType(coords))
    {
        case wxSHEET_CELL_GRID :
        {
            switch (type)
            {   
                case wxSHEET_AttrCell :
                {
                    if (!attr.Ok())
                        m_cellAttrs.RemoveValue(coords);
                    else
                        m_cellAttrs.SetValue(coords, attr);
                    return;
                } 
                case wxSHEET_AttrRow :
                {
                    if (!attr.Ok())
                        m_rowAttrs.RemoveValue(coords.m_row);
                    else
                        m_rowAttrs.SetValue(coords.m_row, attr);
                    return;
                }
                case wxSHEET_AttrCol :
                {
                    if (!attr.Ok())
                        m_colAttrs.RemoveValue(coords.m_col);
                    else
                        m_colAttrs.SetValue(coords.m_col, attr);
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
                    if (!attr.Ok())
                        m_rowLabelAttrs.RemoveValue(coords.m_row);
                    else
                        m_rowLabelAttrs.SetValue(coords.m_row, attr);
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
                    if (!attr.Ok())
                        m_colLabelAttrs.RemoveValue(coords.m_col);
                    else
                        m_colLabelAttrs.SetValue(coords.m_col, attr);
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
void wxSheetCellAttrProvider::UpdateAttrRows( size_t pos, int numRows )
{
    if (numRows == 0) return;

    m_cellAttrs.UpdateRows( pos, numRows );
    
    if (numRows > 0)
    {
        m_rowAttrs.InsertPos( pos, numRows );
        m_rowLabelAttrs.InsertPos( pos, numRows );
    }
    else
    {
        m_rowAttrs.DeletePos( pos, -numRows );
        m_rowLabelAttrs.DeletePos( pos, -numRows );
    }
}
void wxSheetCellAttrProvider::UpdateAttrCols( size_t pos, int numCols )
{
    if (numCols == 0) return;

    m_cellAttrs.UpdateCols( pos, numCols );
    
    if (numCols > 0)
    {
        m_colAttrs.InsertPos( pos, numCols );
        m_colLabelAttrs.InsertPos( pos, numCols );
    }
    else
    {
        m_colAttrs.DeletePos( pos, -numCols );
        m_colLabelAttrs.DeletePos( pos, -numCols );
    }
}

// ----------------------------------------------------------------------------
// wxSheetDataTypeInfo: for the data type registry
// ----------------------------------------------------------------------------

class wxSheetDataTypeInfo
{
public:
    wxSheetDataTypeInfo(const wxString& typeName,
                        const wxSheetCellRenderer& renderer,
                        const wxSheetCellEditor& editor)
        : m_typeName(typeName), m_renderer(renderer), m_editor(editor) {}

    wxString            m_typeName;
    wxSheetCellRenderer m_renderer;
    wxSheetCellEditor   m_editor;
};

// ----------------------------------------------------------------------------
// wxSheetTypeRegistry
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_ADV wxSheetTypeRegistry
{
public:
    wxSheetTypeRegistry() {}
    ~wxSheetTypeRegistry();

    // register a new data type returning position
    int RegisterDataType(const wxString& typeName,
                         const wxSheetCellRenderer& renderer,
                         const wxSheetCellEditor& editor);

    // find one of already registered data types
    int FindRegisteredDataType(const wxString& typeName);

    // try to FindRegisteredDataType(), if this fails and typeName is one of
    // standard typenames, register it and return its index
    int FindDataType(const wxString& typeName);

    // try to FindDataType(), if it fails, see if it is not one of already
    // registered data types with some params in which case clone the
    // registered data type and set params for it
    int FindOrCloneDataType(const wxString& typeName);

    const wxSheetCellRenderer& GetRenderer(int index);
    const wxSheetCellEditor&   GetEditor(int index);

private:
    wxArrayPtrVoid m_typeInfo;
};

wxSheetTypeRegistry::~wxSheetTypeRegistry()
{    
    size_t i, count = m_typeInfo.Count();
    for ( i = 0; i < count; i++ )
        delete (wxSheetDataTypeInfo*)m_typeInfo[i];
}

int wxSheetTypeRegistry::RegisterDataType(const wxString& typeName,
                                          const wxSheetCellRenderer& renderer,
                                          const wxSheetCellEditor& editor)
{
    wxSheetDataTypeInfo* info = new wxSheetDataTypeInfo(typeName, renderer, editor);

    // is it already registered?
    int index = FindRegisteredDataType(typeName);
    if ( index != wxNOT_FOUND )
    {
        wxSheetDataTypeInfo *oldInfo = (wxSheetDataTypeInfo*)m_typeInfo[index];
        delete oldInfo;
        m_typeInfo[index] = info;
        return index;
    }

    m_typeInfo.Add(info);
    return m_typeInfo.GetCount() - 1;
}

int wxSheetTypeRegistry::FindRegisteredDataType(const wxString& typeName)
{
    if (typeName.IsEmpty())
        return wxNOT_FOUND;
    
    size_t i, count = m_typeInfo.GetCount();
    for ( i = 0; i < count; i++ )
    {
        if (typeName == ((wxSheetDataTypeInfo*)m_typeInfo[i])->m_typeName)
            return i;
    }

    return wxNOT_FOUND;
}

int wxSheetTypeRegistry::FindDataType(const wxString& typeName)
{
    int index = FindRegisteredDataType(typeName);
    if (index != wxNOT_FOUND)
        return index;
    
    // if one of the standard ones, register it "on the fly"
#if wxUSE_TEXTCTRL
    if ( typeName == wxSHEET_VALUE_STRING )
    {
        wxSheetCellRenderer renderer(new wxSheetCellStringRendererRefData());
        wxSheetCellEditor editor(new wxSheetCellTextEditorRefData());
        return RegisterDataType(wxSHEET_VALUE_STRING, renderer, editor);
    }
    else if ( typeName == wxSHEET_VALUE_NUMBER )
    {
        wxSheetCellRenderer renderer(new wxSheetCellNumberRendererRefData());
        wxSheetCellEditor editor(new wxSheetCellNumberEditorRefData());
        return RegisterDataType(wxSHEET_VALUE_NUMBER, renderer, editor);
    }
    else if ( typeName == wxSHEET_VALUE_FLOAT )
    {
        wxSheetCellRenderer renderer(new wxSheetCellFloatRendererRefData());
        wxSheetCellEditor editor(new wxSheetCellFloatEditorRefData());
        return RegisterDataType(wxSHEET_VALUE_FLOAT, renderer, editor);
    }
    else if ( typeName == wxSHEET_VALUE_LABEL )
    {
        wxSheetCellRenderer renderer(new wxSheetCellRolColLabelRendererRefData());
        wxSheetCellEditor editor(new wxSheetCellTextEditorRefData());
        return RegisterDataType(wxSHEET_VALUE_LABEL, renderer, editor);
    }
#endif // wxUSE_TEXTCTRL
#if wxUSE_CHECKBOX
    else if ( typeName == wxSHEET_VALUE_BOOL )
    {
        wxSheetCellRenderer renderer(new wxSheetCellBoolRendererRefData());
        wxSheetCellEditor editor(new wxSheetCellBoolEditorRefData());
        return RegisterDataType(wxSHEET_VALUE_BOOL, renderer, editor);
    }
#endif // wxUSE_CHECKBOX
#if wxUSE_COMBOBOX
    else if ( typeName == wxSHEET_VALUE_CHOICE )
    {
        wxSheetCellRenderer renderer(new wxSheetCellStringRendererRefData());
        wxSheetCellEditor editor(new wxSheetCellChoiceEditorRefData());
        return RegisterDataType(wxSHEET_VALUE_CHOICE, renderer, editor);
    }
#endif // wxUSE_COMBOBOX

    return wxNOT_FOUND;
}

int wxSheetTypeRegistry::FindOrCloneDataType(const wxString& typeName)
{
    if (typeName.IsEmpty())
        return wxNOT_FOUND;
    int index = FindDataType(typeName);
    if ( index != wxNOT_FOUND )
        return index;
    
    // the first part of the typename is the "real" type, anything after ':'
    // are the parameters for the renderer
    index = FindDataType(typeName.BeforeFirst(_T(':')));
    if ( index == wxNOT_FOUND )
        return wxNOT_FOUND;

    wxSheetCellRenderer renderer(GetRenderer(index).Clone());
    wxSheetCellEditor editor(GetEditor(index).Clone());
    
    // do it even if there are no parameters to reset them to defaults
    wxString params = typeName.AfterFirst(_T(':'));
    renderer.SetParameters(params);
    editor.SetParameters(params);

    return RegisterDataType(typeName, renderer, editor);
}

const wxSheetCellRenderer& wxSheetTypeRegistry::GetRenderer(int index)
{
    wxCHECK_MSG((index >= 0) && (index < int(m_typeInfo.GetCount())), wxNullSheetCellRenderer,
                wxT("Invalid index in wxSheetTypeRegistry::GetRenderer"));
    return ((wxSheetDataTypeInfo*)m_typeInfo[index])->m_renderer;
}

const wxSheetCellEditor& wxSheetTypeRegistry::GetEditor(int index)
{
    wxCHECK_MSG((index >= 0) && (index < int(m_typeInfo.GetCount())), wxNullSheetCellEditor,
                wxT("Invalid index in wxSheetTypeRegistry::GetEditor"));
    return ((wxSheetDataTypeInfo*)m_typeInfo[index])->m_editor;
}

// ----------------------------------------------------------------------------
// wxSheetTableBase
// ----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS( wxSheetTableBase, wxObject )

wxSheetTableBase::wxSheetTableBase(wxSheet *view) 
                 : wxObject(), m_view(view), m_attrProvider(NULL), 
                               m_own_attr_provider(TRUE)
{
}

wxSheetTableBase::~wxSheetTableBase()
{
    SetAttrProvider(NULL, TRUE);
}

void wxSheetTableBase::SetAttrProvider(wxSheetCellAttrProvider *attrProvider, 
                                       bool is_owner)
{
    if (m_attrProvider && m_own_attr_provider)
        delete m_attrProvider;
    
    m_attrProvider = attrProvider;
    m_own_attr_provider = is_owner;
}

bool wxSheetTableBase::CanHaveAttributes()
{
    // use the attr provider by default
    if ( !GetAttrProvider() )
        SetAttrProvider(new wxSheetCellAttrProvider, TRUE);
    
    return TRUE;
}

wxSheetCellAttr wxSheetTableBase::GetAttr( const wxSheetCoords& coords, 
                                                  wxSheetAttr_Type kind )
{  
    if ( GetAttrProvider() )
        return GetAttrProvider()->GetAttr(coords, kind);
    
    return wxNullSheetCellAttr;
}

void wxSheetTableBase::SetAttr( const wxSheetCoords& coords, 
                                const wxSheetCellAttr& attr, wxSheetAttr_Type kind)
{
    if ( GetAttrProvider() )
        GetAttrProvider()->SetAttr(coords, attr, kind);    
}

bool wxSheetTableBase::InsertRows( size_t WXUNUSED(pos), size_t WXUNUSED(numRows) )
{
    wxFAIL_MSG( wxT("Called grid table class function InsertRows\nbut your derived table class does not override this function") );

    return FALSE;
}
bool wxSheetTableBase::AppendRows( size_t WXUNUSED(numRows) )
{
    wxFAIL_MSG( wxT("Called grid table class function AppendRows\nbut your derived table class does not override this function"));

    return FALSE;
}
bool wxSheetTableBase::DeleteRows( size_t WXUNUSED(pos), size_t WXUNUSED(numRows) )
{
    wxFAIL_MSG( wxT("Called grid table class function DeleteRows\nbut your derived table class does not override this function"));
    return FALSE;
}
bool wxSheetTableBase::InsertCols( size_t WXUNUSED(pos), size_t WXUNUSED(numCols) )
{
    wxFAIL_MSG( wxT("Called grid table class function InsertCols\nbut your derived table class does not override this function"));
    return FALSE;
}
bool wxSheetTableBase::AppendCols( size_t WXUNUSED(numCols) )
{
    wxFAIL_MSG(wxT("Called grid table class function AppendCols\nbut your derived table class does not override this function"));
    return FALSE;
}
bool wxSheetTableBase::DeleteCols( size_t WXUNUSED(pos), size_t WXUNUSED(numCols) )
{
    wxFAIL_MSG( wxT("Called grid table class function DeleteCols\nbut your derived table class does not override this function"));
    return FALSE;
}

bool wxSheetTableBase::SendInsertRowsTableMsg(size_t pos, size_t numRows)
{
    if ( GetView() )
    {
        wxSheetTableMessage msg( this, wxSHEETTABLE_NOTIFY_ROWS_INSERTED,
                                 pos, numRows );

        GetView()->ProcessTableMessage( msg );
        return TRUE;
    }

    return FALSE;
}
bool wxSheetTableBase::SendAppendRowsTableMsg( size_t numRows )
{
    if ( GetView() )
    {
        wxSheetTableMessage msg( this, wxSHEETTABLE_NOTIFY_ROWS_APPENDED,
                                 -1, numRows );

        GetView()->ProcessTableMessage( msg );
        return TRUE;
    }
    return FALSE;
}

bool wxSheetTableBase::SendDeleteRowsTableMsg(size_t pos, size_t numRows)
{
    if ( GetView() )
    {
        wxSheetTableMessage msg( this, wxSHEETTABLE_NOTIFY_ROWS_DELETED,
                                 pos, numRows );

        GetView()->ProcessTableMessage( msg );
        return TRUE;
    }    
    return FALSE;
}
bool wxSheetTableBase::SendInsertColsTableMsg(size_t pos, size_t numCols)
{
    if ( GetView() )
    {
        wxSheetTableMessage msg( this, wxSHEETTABLE_NOTIFY_COLS_INSERTED,
                                 pos, numCols );

        GetView()->ProcessTableMessage( msg );
        return TRUE;
    }
    return FALSE;
}
bool wxSheetTableBase::SendAppendColsTableMsg( size_t numCols )
{
    if ( GetView() )
    {
        wxSheetTableMessage msg( this, wxSHEETTABLE_NOTIFY_COLS_APPENDED,
                                 -1, numCols );

        GetView()->ProcessTableMessage( msg );
        return TRUE;
    }
    return FALSE;
}
bool wxSheetTableBase::SendDeleteColsTableMsg(size_t pos, size_t numCols)
{
    if ( GetView() )
    {
        wxSheetTableMessage msg( this, wxSHEETTABLE_NOTIFY_COLS_DELETED,
                                 pos, numCols );

        GetView()->ProcessTableMessage( msg );
        return TRUE;
    }
    return FALSE;
}

bool wxSheetTableBase::SendUpdateTableMsg()
{
    if ( !GetView() )
        return FALSE;
    
    size_t tableRows = GetNumberRows();
    size_t tableCols = GetNumberCols();
    size_t sheetRows = GetView()->GetNumberRows();
    size_t sheetCols = GetView()->GetNumberCols();
    bool ret = FALSE;
    
    if (tableRows > sheetRows)
        ret |= SendAppendRowsTableMsg(tableRows-sheetRows);
    else if (tableRows < sheetRows)
        ret |= SendDeleteRowsTableMsg(sheetRows-1, sheetRows - tableRows );

    if (tableCols > sheetCols)
        ret |= SendAppendColsTableMsg(tableCols - sheetCols);
    else if (tableCols < sheetCols)
        ret |= SendDeleteColsTableMsg(sheetCols-1, sheetCols - tableCols);
    
    return ret;
}

wxString wxSheetTableBase::GetRowLabelValue( int row )
{
    return wxString::Format(wxT("%d"), row+1); // Starting at zero confuses users
}

wxString wxSheetTableBase::GetColLabelValue( int col )
{
    // default col labels are: cols [0-25]=[A-Z], cols [26-675]=[AA-ZZ]
    wxString s, s2;
    unsigned int i, n;
    for ( n = 1; ; n++ )
    {
        s += wxChar((_T('A')) + (wxChar)( col%26 ));
        col = col/26 - 1;
        if ( col < 0 ) break;
    }
    // reverse the string...
    for ( i = n; i > 0; i-- ) s2 += s[i-1]; 
    //for ( i = 0; i < n;  i++ )  s2 += s[n-i-1];

    return s2;
}

wxString wxSheetTableBase::GetValue( const wxSheetCoords& coords )
{
    if (wxSheet::IsRowLabelCell(coords))
    {
        return GetRowLabelValue(coords.m_row);
    }
    else if (wxSheet::IsColLabelCell(coords))
    {
        return GetColLabelValue(coords.m_col);
    }
    else if (wxSheet::IsCornerLabelCell(coords))
    {
        if (GetView()) 
            return GetView()->DoGetCornerLabelValue();
        
        return wxEmptyString;
    }

    wxFAIL_MSG( _T("invalid row or column index in wxSheetTableBase::GetValue") );
    return wxEmptyString;
}

void wxSheetTableBase::SetValue( const wxSheetCoords& coords, const wxString& value )
{
    if (wxSheet::IsRowLabelCell(coords))
    {
        SetRowLabelValue(coords.m_row, value);
    }
    else if (wxSheet::IsColLabelCell(coords))
    {
        SetColLabelValue(coords.m_col, value);
    }
    else if (wxSheet::IsCornerLabelCell(coords))
    {
        if (GetView())
            GetView()->DoSetCornerLabelValue(value);
    }
    else
        wxFAIL_MSG( _T("invalid row or column index in wxSheetTableBase::SetValue") );
}

wxString wxSheetTableBase::GetTypeName( const wxSheetCoords& )
{
    return wxEmptyString; //wxSHEET_VALUE_STRING;
}
bool wxSheetTableBase::CanGetValueAs( const wxSheetCoords& ,
                                      const wxString& typeName )
{
    return typeName == wxSHEET_VALUE_STRING;
}
bool wxSheetTableBase::CanSetValueAs( const wxSheetCoords& coords, const wxString& typeName )
{
    return CanGetValueAs(coords, typeName);
}
long wxSheetTableBase::GetValueAsLong( const wxSheetCoords& )
{
    return 0;
}
double wxSheetTableBase::GetValueAsDouble( const wxSheetCoords& )
{
    return 0.0;
}
bool wxSheetTableBase::GetValueAsBool( const wxSheetCoords& )
{
    return FALSE;
}
void wxSheetTableBase::SetValueAsLong( const wxSheetCoords& ,
                                       long WXUNUSED(value) )
{
}
void wxSheetTableBase::SetValueAsDouble( const wxSheetCoords& ,
                                         double WXUNUSED(value) )
{
}
void wxSheetTableBase::SetValueAsBool( const wxSheetCoords& ,
                                       bool WXUNUSED(value) )
{
}
void* wxSheetTableBase::GetValueAsCustom( const wxSheetCoords& ,
                                          const wxString& WXUNUSED(typeName) )
{
    return NULL;
}
void  wxSheetTableBase::SetValueAsCustom( const wxSheetCoords& ,
                                          const wxString& WXUNUSED(typeName),
                                          void* WXUNUSED(value) )
{
}

//-----------------------------------------------------------------------------
// wxSheetStringTable - sheet table for string data
//
// wxSheet uses this if you don't specify an alternative table class.
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( wxSheetStringTable, wxSheetTableBase )

wxSheetStringTable::wxSheetStringTable( int numRows, int numCols )
                   : wxSheetTableBase()
{
    m_data.Alloc( numRows );

    wxArrayString sa;
    sa.Alloc( numCols );
    sa.Add( wxEmptyString, numCols );

    m_data.Add( sa, numRows );
}

int wxSheetStringTable::GetNumberRows()
{
    return m_data.GetCount();
}
int wxSheetStringTable::GetNumberCols()
{
    return (m_data.GetCount() > 0) ? m_data[0].GetCount() : 0;
}

wxString wxSheetStringTable::GetValue( const wxSheetCoords& coords )
{
    if (!ContainsGridCell(coords))
        return wxSheetTableBase::GetValue( coords );

    return m_data[coords.m_row][coords.m_col];
}
void wxSheetStringTable::SetValue( const wxSheetCoords& coords, const wxString& value )
{
    if (!ContainsGridCell(coords))
    {
        wxSheetTableBase::SetValue( coords, value );
        return;
    }

    m_data[coords.m_row][coords.m_col] = value;
}
bool wxSheetStringTable::IsEmptyCell( const wxSheetCoords& coords )
{
    if (!ContainsGridCell(coords))
        return wxSheetTableBase::IsEmptyCell( coords );

    return m_data[coords.m_row][coords.m_col].IsEmpty();
}

void wxSheetStringTable::Clear()
{
    int row, col;
    int numRows, numCols;

    numRows = m_data.GetCount();
    if ( numRows > 0 )
    {
        numCols = m_data[0].GetCount();

        for ( row = 0; row < numRows; row++ )
        {
            for ( col = 0; col < numCols; col++ )
                m_data[row][col].Clear();
        }
    }
}

bool wxSheetStringTable::InsertRows( size_t pos, size_t numRows )
{
    size_t curNumRows = m_data.GetCount();
    size_t curNumCols = ( curNumRows > 0 ? m_data[0].GetCount() :
                          ( GetView() ? GetView()->GetNumberCols() : 0 ) );

    if ( pos >= curNumRows )
        return AppendRows( numRows );

    wxArrayString sa;
    sa.Alloc( curNumCols );
    sa.Add( wxEmptyString, curNumCols );
    m_data.Insert( sa, pos, numRows );
    
    return SendInsertRowsTableMsg( pos, numRows );
}

bool wxSheetStringTable::AppendRows( size_t numRows )
{
    size_t curNumRows = m_data.GetCount();
    size_t curNumCols = ( curNumRows > 0 ? m_data[0].GetCount() :
                          ( GetView() ? GetView()->GetNumberCols() : 0 ) );

    wxArrayString sa;
    if ( curNumCols > 0 )
    {
        sa.Alloc( curNumCols );
        sa.Add( wxEmptyString, curNumCols );
    }

    m_data.Add( sa, numRows );

    return SendAppendRowsTableMsg( numRows );
}

bool wxSheetStringTable::DeleteRows( size_t pos, size_t numRows )
{
    size_t curNumRows = m_data.GetCount();

    if ( pos >= curNumRows )
    {
        wxFAIL_MSG( wxString::Format
                    (
                        wxT("Called wxxGridStringTable::DeleteRows(pos=%lu, N=%lu)\nPos value is invalid for present table with %lu rows"),
                        (unsigned long)pos,
                        (unsigned long)numRows,
                        (unsigned long)curNumRows
                    ) );

        return FALSE;
    }

    if ( numRows > curNumRows - pos )
        numRows = curNumRows - pos;

    if ( numRows >= curNumRows )
        m_data.Clear();
    else
        m_data.RemoveAt( pos, numRows );
    
    return SendDeleteRowsTableMsg( pos, numRows );
}

bool wxSheetStringTable::InsertCols( size_t pos, size_t numCols )
{
    size_t curNumRows = m_data.GetCount();
    size_t curNumCols = ( curNumRows > 0 ? m_data[0].GetCount() :
                          ( GetView() ? GetView()->GetNumberCols() : 0 ) );

    if ( pos >= curNumCols )
        return AppendCols( numCols );

    size_t row, col;
    for ( row = 0; row < curNumRows; row++ )
    {
        for ( col = pos; col < pos + numCols; col++ )
            m_data[row].Insert( wxEmptyString, col );
    }
    
    return SendInsertColsTableMsg( pos, numCols );
}

bool wxSheetStringTable::AppendCols( size_t numCols )
{
    size_t curNumRows = m_data.GetCount();
#if 0
    if ( !curNumRows )
    {
        // TODO: something better than this ?
        //
        wxFAIL_MSG( wxT("Unable to append cols to a grid table with no rows.\nCall AppendRows() first") );
        return FALSE;
    }
#endif

    for ( size_t row = 0; row < curNumRows; row++ )
        m_data[row].Add( wxEmptyString, numCols );

    return SendAppendColsTableMsg( numCols );
}

bool wxSheetStringTable::DeleteCols( size_t pos, size_t numCols )
{
    size_t curNumRows = m_data.GetCount();
    size_t curNumCols = ( curNumRows > 0 ? m_data[0].GetCount() :
                          ( GetView() ? GetView()->GetNumberCols() : 0 ) );

    if ( pos >= curNumCols )
    {
        wxFAIL_MSG( wxString::Format
                    (
                        wxT("Called wxxGridStringTable::DeleteCols(pos=%lu, N=%lu)\nPos value is invalid for present table with %lu cols"),
                        (unsigned long)pos,
                        (unsigned long)numCols,
                        (unsigned long)curNumCols
                    ) );
        return FALSE;
    }

    if ( numCols > curNumCols - pos )
        numCols = curNumCols - pos;

    for ( size_t row = 0; row < curNumRows; row++ )
    {
        if ( numCols >= curNumCols )
            m_data[row].Clear();
        else
            m_data[row].RemoveAt( pos, numCols );
    }
    
    return SendDeleteColsTableMsg( pos, numCols );
}

wxString wxSheetStringTable::GetRowLabelValue( int row )
{
    if ( row > (int)(m_rowLabels.GetCount()) - 1 )
        return wxSheetTableBase::GetRowLabelValue( row );
    else
        return m_rowLabels[ row ];
}

wxString wxSheetStringTable::GetColLabelValue( int col )
{
    if ( col > (int)(m_colLabels.GetCount()) - 1 )
        return wxSheetTableBase::GetColLabelValue( col );
    else
        return m_colLabels[ col ];
}

void wxSheetStringTable::SetRowLabelValue( int row, const wxString& value )
{
    int count = m_rowLabels.GetCount();
    if ( row > count - 1 )
    {
        for ( int n = count; n <= row; n++ )
            m_rowLabels.Add( wxSheetTableBase::GetRowLabelValue(n) );
    }

    m_rowLabels[row] = value;
}

void wxSheetStringTable::SetColLabelValue( int col, const wxString& value )
{
    int count = m_colLabels.GetCount();
    if ( col > count - 1 )
    {
        for ( int n = count; n <= col; n++ )
            m_colLabels.Add( wxSheetTableBase::GetColLabelValue(n) );
    }

    m_colLabels[col] = value;
}

//-----------------------------------------------------------------------------
// wxPairArrayIntPairArraySheetString - string data
//-----------------------------------------------------------------------------

void wxPairArrayIntPairArraySheetString::RemoveEmptyRows()
{
    int nrows = GetCount();
    for (int r=0; r<nrows; r++)
    {
        if (GetItemValue(r).GetCount() == 0)
        {
            RemoveAt(r);
            r--;
            nrows--;
        }
    }
}

//-----------------------------------------------------------------------------
// wxSheetStringSparseTable - sheet table for string data
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( wxSheetStringSparseTable, wxSheetTableBase )

wxSheetStringSparseTable::wxSheetStringSparseTable( int numRows, int numCols )
                         : wxSheetTableBase(), m_numRows(numRows), m_numCols(numCols)
{
}

wxString wxSheetStringSparseTable::GetValue( const wxSheetCoords& coords )
{    
    if (!ContainsGridCell(coords))
        return wxSheetTableBase::GetValue(coords);
        
    int n = m_data.FindIndex(coords.m_row);
    if (n != wxNOT_FOUND)
        return m_data.GetItemValue(n).GetValue(coords.m_col);
    
    return wxEmptyString;
}

void wxSheetStringSparseTable::SetValue( const wxSheetCoords& coords, const wxString& value )
{
    if (!ContainsGridCell(coords))
    {
        wxSheetTableBase::SetValue(coords, value);
        return;
    }

    int n = m_data.FindIndex(coords.m_row);
    
    if (value.IsEmpty())
    {
        // remove the value if empty
        if (n != wxNOT_FOUND)
        {
            m_data.GetItemValue(n).RemoveValue(coords.m_col);
            // remove this row if empty
            if (m_data.GetItemValue(n).GetCount() == 0)
                m_data.RemoveAt(n);
        }
    }
    else
    {
        if (n == wxNOT_FOUND)
            m_data.GetOrCreateValue(coords.m_row).SetValue(coords.m_col, value);
        else
            m_data.GetItemValue(n).SetValue(coords.m_col, value);
    }
}

bool wxSheetStringSparseTable::IsEmptyCell( const wxSheetCoords& coords )
{
    if (!ContainsGridCell(coords))
        return wxSheetTableBase::IsEmptyCell(coords);
        
    int n = m_data.FindIndex(coords.m_row);
    if (n == wxNOT_FOUND)
        return TRUE;
    
    return m_data.GetItemValue(n).FindIndex(coords.m_col) == wxNOT_FOUND;
}

int wxSheetStringSparseTable::GetFirstNonEmptyColToLeft( const wxSheetCoords& coords )
{
    int rowPos = m_data.FindIndex(coords.m_row);
    if (rowPos != wxNOT_FOUND)
    {
        // pos == 0 meaning nothing to left, == count for col > last filled
        int colPos = m_data.GetItemValue(rowPos).FindInsertIndex(coords.m_col);
        if (colPos > 0)
            return m_data.GetItemValue(rowPos).GetItemKey(colPos-1);
    }

    return -1;    
}

void wxSheetStringSparseTable::Clear()
{
    m_data.Clear();    
}

bool wxSheetStringSparseTable::InsertRows( size_t pos, size_t numRows )
{
    if ( int(pos) >= m_numRows )
        return AppendRows( numRows );
    
    m_data.InsertPos(pos, numRows);
    m_rowLabels.InsertPos(pos, numRows);
    m_numRows += numRows;
    
    return SendInsertRowsTableMsg(pos, numRows);
}

bool wxSheetStringSparseTable::AppendRows( size_t numRows )
{
    m_numRows += numRows;    
    return SendAppendRowsTableMsg( numRows );
}

bool wxSheetStringSparseTable::DeleteRows( size_t pos, size_t numRows )
{
    wxCHECK_MSG(int(pos) < m_numRows-1, FALSE, wxString::Format(
        wxT("Called wxSheetStringTable::DeleteRows(pos=%lu, N=%lu)\nPos value is invalid for present table with %lu rows"),
        (unsigned long)pos, (unsigned long)numRows, (unsigned long)m_numRows));
    
    if ( numRows > m_numRows - pos )
        numRows = m_numRows - pos;
    
    if ( int(numRows) >= m_numRows )
    {
        Clear();
        m_numRows = 0;
    }
    else
    {
        m_data.DeletePos(pos, numRows);
        m_rowLabels.DeletePos(pos, numRows);
        m_numRows -= numRows;
    }

    return SendDeleteRowsTableMsg( pos, numRows );
}

bool wxSheetStringSparseTable::InsertCols( size_t pos, size_t numCols )
{
    if ( int(pos) >= m_numCols )
        return AppendCols( numCols );
    
    size_t n, count = m_data.GetCount();
    for (n=0; n<count; n++)
        m_data.GetItemValue(n).InsertPos(pos, numCols);

    m_colLabels.InsertPos(pos, numCols);
    m_numCols += numCols;
    
    return SendInsertColsTableMsg( pos, numCols );
}

bool wxSheetStringSparseTable::AppendCols( size_t numCols )
{
    m_numCols += numCols;
    return SendAppendColsTableMsg( numCols );
}

bool wxSheetStringSparseTable::DeleteCols( size_t pos, size_t numCols )
{
    wxCHECK_MSG(int(pos) < m_numCols-1, FALSE, wxString::Format(
        wxT("Called wxSheetStringTable::DeleteCols(pos=%lu, N=%lu)\nPos value is invalid for present table with %lu cols"),
        (unsigned long)pos, (unsigned long)numCols, (unsigned long)m_numCols));

    if ( numCols > m_numCols - pos )
        numCols = m_numCols - pos;

    if ( int(numCols) >= m_numCols )
    {
        Clear();
        m_numCols = 0;
    }
    else
    {
        size_t n, count = m_data.GetCount();
        for (n=0; n<count; n++)
            m_data.GetItemValue(n).DeletePos(pos, numCols);
        
        m_colLabels.DeletePos(pos, numCols);
        m_numCols -= numCols;
    }
    
    return SendDeleteColsTableMsg( pos, numCols );
}

wxString wxSheetStringSparseTable::GetRowLabelValue( int row )
{
    wxCHECK_MSG(row >= 0, wxEmptyString, wxT("Invalid row label index"));
    
    int pos = m_rowLabels.FindIndex(row);
    if (pos != wxNOT_FOUND)
        return m_rowLabels.GetItemValue(pos);
    else
        return wxSheetTableBase::GetRowLabelValue( row ); // using default label
}

wxString wxSheetStringSparseTable::GetColLabelValue( int col )
{
    wxCHECK_MSG(col >= 0, wxEmptyString, wxT("Invalid col label index"));
    
    int pos = m_colLabels.FindIndex(col);
    if (pos != wxNOT_FOUND)
        return m_colLabels.GetItemValue(pos);
    else
        return wxSheetTableBase::GetColLabelValue( col ); // using default label    
}

void wxSheetStringSparseTable::SetRowLabelValue( int row, const wxString& value )
{
    wxCHECK_RET(row >= 0, wxT("Invalid col label index"));    
    m_rowLabels.SetValue(row, value);    
}

void wxSheetStringSparseTable::SetColLabelValue( int col, const wxString& value )
{
    wxCHECK_RET(col >= 0, wxT("Invalid col label index"));
    m_colLabels.SetValue(col, value);    
}

// ----------------------------------------------------------------------------
// wxSheetRowLabelWindow
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( wxSheetRowLabelWindow, wxWindow )

BEGIN_EVENT_TABLE( wxSheetRowLabelWindow, wxWindow )
    EVT_PAINT           ( wxSheetRowLabelWindow::OnPaint )
    EVT_MOUSEWHEEL      ( wxSheetRowLabelWindow::OnMouseWheel)
    EVT_MOUSE_EVENTS    ( wxSheetRowLabelWindow::OnMouseEvent )
    EVT_KEY_DOWN        ( wxSheetRowLabelWindow::OnKeyDown )
    EVT_KEY_UP          ( wxSheetRowLabelWindow::OnKeyUp )
    EVT_CHAR            ( wxSheetRowLabelWindow::OnChar )
END_EVENT_TABLE()

wxSheetRowLabelWindow::wxSheetRowLabelWindow( wxSheet *parent, wxWindowID id,
                                              const wxPoint &pos, const wxSize &size )
    :wxWindow( parent, id, pos, size, wxWANTS_CHARS|wxBORDER_NONE) //|wxFULL_REPAINT_ON_RESIZE ) // FIXME
{
    m_owner = parent;
    m_mouseCursor = wxSheet::WXSHEET_CURSOR_SELECT_CELL;
}

void wxSheetRowLabelWindow::PrepareDC( wxDC& dc )
{
    m_owner->PrepareRowLabelDC( dc );
}

void wxSheetRowLabelWindow::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    wxPaintDC dc(this);
    PrepareDC( dc );
    m_owner->OnRowLabelWindowPaint(dc, GetUpdateRegion());
}

void wxSheetRowLabelWindow::OnMouseEvent( wxMouseEvent& event )
{
    m_owner->ProcessRowLabelMouseEvent( event );
}

void wxSheetRowLabelWindow::OnMouseWheel( wxMouseEvent& event )
{
    if (!m_owner->GetEventHandler()->ProcessEvent(event))
    {
        // GTK doesn't have good wheel events
        if (m_owner->GetNumberRows() < 1)
            return;
        
        wxPoint origin = m_owner->GetGridOrigin();
        wxSize size = m_owner->GetGridWindow()->GetClientSize();
        int dy = event.GetWheelRotation() < 0 ? 1 : -1;
        m_owner->SetGridOrigin( origin.x, origin.y + dy * size.y/4, FALSE, TRUE );
    }
}

// This seems to be required for wxMotif otherwise the mouse
// cursor must be in the cell edit control to get key events
void wxSheetRowLabelWindow::OnKeyDown( wxKeyEvent& event )
{
    if ( !m_owner->GetEventHandler()->ProcessEvent(event) ) 
        event.Skip();
}

void wxSheetRowLabelWindow::OnKeyUp( wxKeyEvent& event )
{
    if ( !m_owner->GetEventHandler()->ProcessEvent(event) ) 
        event.Skip();
}

void wxSheetRowLabelWindow::OnChar( wxKeyEvent& event )
{
    if ( !m_owner->GetEventHandler()->ProcessEvent(event) ) 
        event.Skip();
}

// ----------------------------------------------------------------------------
// wxSheetColLabelWindow
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( wxSheetColLabelWindow, wxWindow )

BEGIN_EVENT_TABLE( wxSheetColLabelWindow, wxWindow )
    EVT_PAINT           ( wxSheetColLabelWindow::OnPaint )
    EVT_MOUSEWHEEL      ( wxSheetColLabelWindow::OnMouseWheel)
    EVT_MOUSE_EVENTS    ( wxSheetColLabelWindow::OnMouseEvent )
    EVT_KEY_DOWN        ( wxSheetColLabelWindow::OnKeyDown )
    EVT_KEY_UP          ( wxSheetColLabelWindow::OnKeyUp )
    EVT_CHAR            ( wxSheetColLabelWindow::OnChar )
END_EVENT_TABLE()

wxSheetColLabelWindow::wxSheetColLabelWindow( wxSheet *parent, wxWindowID id,
                                              const wxPoint &pos, const wxSize &size )
    :wxWindow( parent, id, pos, size, wxWANTS_CHARS|wxBORDER_NONE) //|wxFULL_REPAINT_ON_RESIZE ) // FIXME flickertastic
{
    m_owner = parent;
    m_mouseCursor = wxSheet::WXSHEET_CURSOR_SELECT_CELL;
}

void wxSheetColLabelWindow::PrepareDC( wxDC& dc )
{
    m_owner->PrepareColLabelDC( dc );
}

void wxSheetColLabelWindow::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    wxPaintDC dc(this);
    PrepareDC( dc );
    m_owner->OnColLabelWindowPaint( dc, GetUpdateRegion() );
}

void wxSheetColLabelWindow::OnMouseEvent( wxMouseEvent& event )
{
    m_owner->ProcessColLabelMouseEvent( event );
}

void wxSheetColLabelWindow::OnMouseWheel( wxMouseEvent& event )
{
    if (!m_owner->GetEventHandler()->ProcessEvent(event))
    {
        // GTK doesn't have good wheel events
        if (m_owner->GetNumberCols() < 1)
            return;
        
        wxPoint origin = m_owner->GetGridOrigin();
        wxSize size = m_owner->GetGridWindow()->GetClientSize();
        int dx = event.GetWheelRotation() < 0 ? 1 : -1;
        m_owner->SetGridOrigin( origin.x + dx * size.x/4, origin.y, FALSE, TRUE );
    }
}

// This seems to be required for wxMotif otherwise the mouse
// cursor must be in the cell edit control to get key events
void wxSheetColLabelWindow::OnKeyDown( wxKeyEvent& event )
{
    if ( !m_owner->GetEventHandler()->ProcessEvent(event) ) 
        event.Skip();
}
void wxSheetColLabelWindow::OnKeyUp( wxKeyEvent& event )
{
    if ( !m_owner->GetEventHandler()->ProcessEvent(event) ) 
        event.Skip();
}
void wxSheetColLabelWindow::OnChar( wxKeyEvent& event )
{
    if ( !m_owner->GetEventHandler()->ProcessEvent(event) ) 
        event.Skip();
}

// ----------------------------------------------------------------------------
// wxSheetCornerLabelWindow
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( wxSheetCornerLabelWindow, wxWindow )

BEGIN_EVENT_TABLE( wxSheetCornerLabelWindow, wxWindow )
    EVT_MOUSEWHEEL      ( wxSheetCornerLabelWindow::OnMouseWheel)
    EVT_MOUSE_EVENTS    ( wxSheetCornerLabelWindow::OnMouseEvent )
    EVT_PAINT           ( wxSheetCornerLabelWindow::OnPaint)
    EVT_KEY_DOWN        ( wxSheetCornerLabelWindow::OnKeyDown )
    EVT_KEY_UP          ( wxSheetCornerLabelWindow::OnKeyUp )
    EVT_CHAR            ( wxSheetCornerLabelWindow::OnChar )
END_EVENT_TABLE()

wxSheetCornerLabelWindow::wxSheetCornerLabelWindow( wxSheet *parent, wxWindowID id,
                                                  const wxPoint &pos, const wxSize &size )
    :wxWindow( parent, id, pos, size, wxWANTS_CHARS|wxBORDER_NONE) //|wxFULL_REPAINT_ON_RESIZE ) // FIXME
{
    m_owner = parent;
    m_mouseCursor = wxSheet::WXSHEET_CURSOR_SELECT_CELL;
}

void wxSheetCornerLabelWindow::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    wxPaintDC dc(this);
    m_owner->OnCornerLabelWindowPaint( dc, GetUpdateRegion() );
}

void wxSheetCornerLabelWindow::OnMouseEvent( wxMouseEvent& event )
{
    m_owner->ProcessCornerLabelMouseEvent( event );
}

void wxSheetCornerLabelWindow::OnMouseWheel( wxMouseEvent& event )
{
    m_owner->GetEventHandler()->ProcessEvent(event);
}

// This seems to be required for wxMotif otherwise the mouse
// cursor must be in the cell edit control to get key events
void wxSheetCornerLabelWindow::OnKeyDown( wxKeyEvent& event )
{
    if ( !m_owner->GetEventHandler()->ProcessEvent(event) ) 
        event.Skip();
}
void wxSheetCornerLabelWindow::OnKeyUp( wxKeyEvent& event )
{
    if ( !m_owner->GetEventHandler()->ProcessEvent(event) ) 
        event.Skip();
}
void wxSheetCornerLabelWindow::OnChar( wxKeyEvent& event )
{
    if ( !m_owner->GetEventHandler()->ProcessEvent(event) ) 
        event.Skip();
}

// ----------------------------------------------------------------------------
// wxSheetGridWindow
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( wxSheetGridWindow, wxWindow )

BEGIN_EVENT_TABLE( wxSheetGridWindow, wxWindow )
    EVT_PAINT           ( wxSheetGridWindow::OnPaint )
    EVT_MOUSEWHEEL      ( wxSheetGridWindow::OnMouseWheel )
    EVT_MOUSE_EVENTS    ( wxSheetGridWindow::OnMouseEvent )
    EVT_KEY_DOWN        ( wxSheetGridWindow::OnKeyDown )
    EVT_KEY_UP          ( wxSheetGridWindow::OnKeyUp )
    EVT_CHAR            ( wxSheetGridWindow::OnChar )
    EVT_SET_FOCUS       ( wxSheetGridWindow::OnFocus )
    EVT_KILL_FOCUS      ( wxSheetGridWindow::OnFocus )          
    EVT_ERASE_BACKGROUND( wxSheetGridWindow::OnEraseBackground )
END_EVENT_TABLE()

wxSheetGridWindow::wxSheetGridWindow( wxSheet *parent, wxWindowID id, 
                                      const wxPoint &pos, const wxSize &size )
    :wxWindow( parent, id, pos, size, 
               wxWANTS_CHARS|wxBORDER_NONE, // |wxFULL_REPAINT_ON_RESIZE, // FIXME
               wxT("wxSheetGridWindow") )
{
    m_owner = parent;
    m_mouseCursor = wxSheet::WXSHEET_CURSOR_SELECT_CELL;
    SetBackgroundColour(*wxWHITE);
}

void wxSheetGridWindow::PrepareDC( wxDC& dc )
{
    m_owner->PrepareGridDC( dc );
}

void wxSheetGridWindow::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
    wxPaintDC dc( this );
    PrepareDC( dc );
    m_owner->OnGridWindowPaint( dc, GetUpdateRegion() );    
}

void wxSheetGridWindow::OnMouseEvent( wxMouseEvent& event )
{
    m_owner->ProcessGridCellMouseEvent( event );
}

void wxSheetGridWindow::OnMouseWheel( wxMouseEvent& event )
{
    // mouse wheel scrolls up/down same as row window
    m_owner->GetRowLabelWindow()->OnMouseWheel(event);
}

// This seems to be required for wxMotif/wxGTK otherwise the mouse
// cursor must be in the cell edit control to get key events
void wxSheetGridWindow::OnKeyDown( wxKeyEvent& event )
{
    if ( !m_owner->GetEventHandler()->ProcessEvent( event ) ) 
        event.Skip();
}
void wxSheetGridWindow::OnKeyUp( wxKeyEvent& event )
{
    if ( !m_owner->GetEventHandler()->ProcessEvent( event ) ) 
        event.Skip();
}
void wxSheetGridWindow::OnChar( wxKeyEvent& event )
{
    if ( !m_owner->GetEventHandler()->ProcessEvent( event ) ) 
        event.Skip();
}

void wxSheetGridWindow::OnFocus( wxFocusEvent& event )
{
    if ( !m_owner->GetEventHandler()->ProcessEvent( event ) )
        event.Skip();
}

// ----------------------------------------------------------------------------
// Internal Helper function for computing row or column from some
// (unscrolled) coordinate value, using either
// m_defaultRowHeight/m_defaultColWidth or binary search on array
// of m_rowBottoms/m_ColRights to speed up the search!
static int CoordToRowOrCol(int coord, int defaultDist, int minDist,
                           const wxArrayInt& BorderArray, int nMax,
                           bool clipToMinMax);
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// StyledTextEditorData_RefData
//-----------------------------------------------------------------------------
wxSheetRefData::wxSheetRefData()
{
    m_table    = NULL;
    m_ownTable = FALSE;

    m_typeRegistry = new wxSheetTypeRegistry;
    
    m_numRows = 0;
    m_numCols = 0;

    m_defaultRowHeight = WXSHEET_DEFAULT_ROW_HEIGHT;
    m_defaultColWidth  = WXSHEET_DEFAULT_COL_WIDTH;
    m_minAcceptableRowHeight = WXSHEET_MIN_ROW_HEIGHT;
    m_minAcceptableColWidth  = WXSHEET_MIN_COL_WIDTH;

    m_rowMinHeights = wxLongToLongHashMap(SHEET_HASH_SIZE);
    m_colMinWidths  = wxLongToLongHashMap(SHEET_HASH_SIZE);

    m_rowLabelWidth  = WXSHEET_DEFAULT_ROW_LABEL_WIDTH;
    m_colLabelHeight = WXSHEET_DEFAULT_COL_LABEL_HEIGHT;

    m_extraWidth  = 0;
    m_extraHeight = 0;
    
    m_equal_col_widths = 0;

    m_gridLineColour = wxColour( 192, 192, 192 );
    m_gridLinesEnabled = TRUE;
    m_cursorCellHighlightColour     = *wxBLACK;
    m_cursorCellHighlightPenWidth   = 2;
    m_cursorCellHighlightROPenWidth = 1;
    m_labelOutlineColour  = wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW);

    m_rowResizeCursor = wxCursor( wxCURSOR_SIZENS );
    m_colResizeCursor = wxCursor( wxCURSOR_SIZEWE );

    m_editable = TRUE;  // default for whole grid

    m_canDragRowSize  = TRUE; 
    m_canDragColSize  = TRUE; 
    m_canDragGridSize = TRUE;

    m_cursorCoords = wxNullSheetCoords;

    m_selectingAnchor = wxNullSheetCoords;
    m_selectionMode   = wxSHEET_SelectCells;
    m_selection = new wxSheetSelection();
    m_selectionBackground = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
    m_selectionForeground = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
    
    m_spannedCells = new wxSheetSelection;    
    
    m_cellEditorCoords = wxNullSheetCoords;

    m_pasting = FALSE;
}

wxSheetRefData::~wxSheetRefData() 
{
    if (m_ownTable && m_table)
        delete m_table;
    
    delete m_typeRegistry;
    delete m_spannedCells;   
    delete m_selection;    
}

int wxSheetRefData::FindSheet(wxSheet* sheet) const
{
    return m_sheets.Index(sheet);
}
void wxSheetRefData::AddSheet(wxSheet* sheet)
{
    wxCHECK_RET(sheet, wxT("Invalid sheet"));
    
    // not an error, just let them do it to avoid having to check
    if (!HasSheet(sheet))
        m_sheets.Add(sheet);
}
void wxSheetRefData::RemoveSheet(wxSheet* sheet)
{
    wxCHECK_RET(sheet, wxT("Invalid sheet"));

    // not an error, if not found allows for this to be called in destructor
    int index = FindSheet(sheet);
    if (index != wxNOT_FOUND)
        m_sheets.RemoveAt(index);
}

void wxSheetRefData::RefreshGridWindow(wxSheet* sheet, const wxRect* rect_)
{
    size_t n, count = m_sheets.GetCount();
    wxRect rect = rect_ ? (*rect_) : wxRect(0, 0, 1000000, 1000000);
    rect = sheet->CalcUnscrolledRect(rect);
    
    for (n=0; n<count; n++)
    {
        wxSheet* s = (wxSheet*)m_sheets.Item(n);
        if (s == sheet) 
            continue;
        
        wxRect scrolledRect(s->CalcScrolledRect(rect));
        wxRect refedRect(wxPoint(0,0), s->GetGridWindow()->GetSize());
        scrolledRect.Intersect(refedRect);
        if (!wxRectIsEmpty(scrolledRect))
        {
            s->GetGridWindow()->Refresh(FALSE, &scrolledRect);
        }
    }
}
void wxSheetRefData::RefreshRowLabelWindow(wxSheet* sheet, const wxRect* rect_)
{
    size_t n, count = m_sheets.GetCount();
    wxRect rect = rect_ ? (*rect_) : wxRect(0, 0, 1000000, 1000000);
    sheet->CalcUnscrolledPosition(0, rect.y, NULL, &rect.y);
    
    for (n=0; n<count; n++)
    {
        wxSheet* s = (wxSheet*)m_sheets.Item(n);
        if (s == sheet) 
            continue;
        
        wxRect scrolledRect(rect);
        s->CalcScrolledPosition(0, scrolledRect.y, NULL, &scrolledRect.y);
        wxRect refedRect(wxPoint(0,0), s->GetRowLabelWindow()->GetSize());
        scrolledRect.Intersect(refedRect);
        if (!wxRectIsEmpty(scrolledRect) && s->GetRowLabelWindow()->IsShown())
        {
            s->GetRowLabelWindow()->Refresh(FALSE, &scrolledRect);
        }
    }
}
void wxSheetRefData::RefreshColLabelWindow(wxSheet* sheet, const wxRect* rect_)
{
    size_t n, count = m_sheets.GetCount();
    wxRect rect = rect_ ? (*rect_) : wxRect(0, 0, 1000000, 1000000);
    sheet->CalcUnscrolledPosition(rect.x, 0, &rect.x, NULL);
    
    for (n=0; n<count; n++)
    {
        wxSheet* s = (wxSheet*)m_sheets.Item(n);
        if (s == sheet) 
            continue;
        
        wxRect scrolledRect(rect);
        s->CalcScrolledPosition(scrolledRect.x, 0, &scrolledRect.x, NULL);
        wxRect refedRect(wxPoint(0,0), s->GetColLabelWindow()->GetSize());
        scrolledRect.Intersect(refedRect);
        if (!wxRectIsEmpty(scrolledRect) && s->GetColLabelWindow()->IsShown())
        {
            s->GetColLabelWindow()->Refresh(FALSE, &scrolledRect);
        }
    }
}
void wxSheetRefData::RefreshCornerLabelWindow(wxSheet* sheet, const wxRect* WXUNUSED(rect_))
{
    size_t n, count = m_sheets.GetCount();
    
    for (n=0; n<count; n++)
    {
        wxSheet* s = (wxSheet*)m_sheets.Item(n);
        if (s == sheet) 
            continue;
        
        s->GetCornerLabelWindow()->Refresh(TRUE);
    }
}

//-----------------------------------------------------------------------------
// RTTI stuff for ? 
//-----------------------------------------------------------------------------

#if wxUSE_EXTENDED_RTTI
WX_DEFINE_FLAGS( wxSheetStyle )

wxBEGIN_FLAGS( wxSheetStyle )
    // new style border flags, we put them first to
    // use them for streaming out
    wxFLAGS_MEMBER(wxBORDER_SIMPLE)
    wxFLAGS_MEMBER(wxBORDER_SUNKEN)
    wxFLAGS_MEMBER(wxBORDER_DOUBLE)
    wxFLAGS_MEMBER(wxBORDER_RAISED)
    wxFLAGS_MEMBER(wxBORDER_STATIC)
    wxFLAGS_MEMBER(wxBORDER_NONE)
    
    // old style border flags
    wxFLAGS_MEMBER(wxSIMPLE_BORDER)
    wxFLAGS_MEMBER(wxSUNKEN_BORDER)
    wxFLAGS_MEMBER(wxDOUBLE_BORDER)
    wxFLAGS_MEMBER(wxRAISED_BORDER)
    wxFLAGS_MEMBER(wxSTATIC_BORDER)
    wxFLAGS_MEMBER(wxBORDER)

    // standard window styles
    wxFLAGS_MEMBER(wxTAB_TRAVERSAL)
    wxFLAGS_MEMBER(wxCLIP_CHILDREN)
    wxFLAGS_MEMBER(wxTRANSPARENT_WINDOW)
    wxFLAGS_MEMBER(wxWANTS_CHARS)
    wxFLAGS_MEMBER(wxFULL_REPAINT_ON_RESIZE)
    wxFLAGS_MEMBER(wxALWAYS_SHOW_SB )
    wxFLAGS_MEMBER(wxVSCROLL)
    wxFLAGS_MEMBER(wxHSCROLL)

wxEND_FLAGS( wxSheetStyle )

IMPLEMENT_DYNAMIC_CLASS_XTI(wxSheet, wxWindow,"sheet/sheet.h")

wxBEGIN_PROPERTIES_TABLE(wxSheet)
    wxHIDE_PROPERTY( Children )
    wxPROPERTY_FLAGS( WindowStyle , wxSheetStyle , long , SetWindowStyleFlag , GetWindowStyleFlag , , 0 /*flags*/ , wxT("Helpstring") , wxT("group")) // style
wxEND_PROPERTIES_TABLE()

wxBEGIN_HANDLERS_TABLE(wxSheet)
wxEND_HANDLERS_TABLE()

wxCONSTRUCTOR_5( wxSheet , wxWindow* , Parent , wxWindowID , Id , wxPoint , Position , wxSize , Size , long , WindowStyle ) 

/*
 TODO : Expose more information of a list's layout etc. via appropriate objects (à la NotebookPageInfo)
*/
#else
IMPLEMENT_DYNAMIC_CLASS( wxSheet, wxWindow )
#endif

BEGIN_EVENT_TABLE( wxSheet, wxWindow )
    EVT_PAINT           ( wxSheet::OnPaint )
    EVT_KEY_DOWN        ( wxSheet::OnKeyDown )
    EVT_KEY_UP          ( wxSheet::OnKeyUp )
    EVT_CHAR            ( wxSheet::OnChar )
    EVT_MOUSE_EVENTS    ( wxSheet::OnMouse )
    EVT_COMMAND_SCROLL  ( wxSheet::ID_HORIZ_SCROLLBAR,  wxSheet::OnScroll )
    EVT_COMMAND_SCROLL  ( wxSheet::ID_VERT_SCROLLBAR,   wxSheet::OnScroll )
    EVT_TIMER           ( wxSheet::ID_MOUSE_DRAG_TIMER, wxSheet::OnMouseTimer )
    EVT_ERASE_BACKGROUND( wxSheet::OnEraseBackground )
END_EVENT_TABLE()

void wxSheet::Init()
{    
    m_created = FALSE;    // set to TRUE by CreateGrid
    
    m_cornerLabelWin = NULL;
    m_rowLabelWin    = NULL;
    m_colLabelWin    = NULL;
    m_gridWin        = NULL;
    m_horizScrollBar = NULL;
    m_vertScrollBar  = NULL;
    
    m_scrollBarMode = wxSheet::SB_AS_NEEDED;

    m_keySelecting    = FALSE;

    // init attr cache
    m_cacheAttr       = wxNullSheetCellAttr;
    m_cacheAttrCoords = wxNullSheetCoords;
    m_cacheAttrType   = -1;
    
    m_inOnKeyDown = FALSE;
    m_batchCount  = 0;
    m_resizing = FALSE;
    
    m_mouseCursorMode = WXSHEET_CURSOR_SELECT_CELL;
    m_mouseCursor = WXSHEET_CURSOR_SELECT_CELL;

    m_winCapture = (wxWindow *)NULL;
    
    m_dragLastPos     = -1;
    m_dragRowOrCol    = -1;
    m_isDragging      = FALSE;
    m_startDragPos    = wxDefaultPosition;

    m_waitForSlowClick = FALSE;
    m_mouseTimer       = NULL;
    
    m_enable_split_vert  = FALSE;
    m_enable_split_horiz = FALSE;    
    
    m_refData = new wxSheetRefData; // ALWAYS CREATED and ALWAYS EXISTS!
}

bool wxSheet::Create( wxWindow *parent, wxWindowID id,
                      const wxPoint& pos, const wxSize& size,
                      long style, const wxString& name)
{
    if (!wxWindow::Create(parent, id, pos, size, style | wxWANTS_CHARS, name))
        return FALSE;
    
    // Set default cell attributes
    GetSheetRefData()->m_defaultGridCellAttr.Create();
    GetSheetRefData()->m_defaultGridCellAttr.SetKind(wxSHEET_AttrDefault);
    GetSheetRefData()->m_defaultGridCellAttr.SetFont(GetFont()); // need window created for this
    GetSheetRefData()->m_defaultGridCellAttr.SetAlignment(wxALIGN_LEFT | wxALIGN_TOP);
    GetSheetRefData()->m_defaultGridCellAttr.SetOrientation(wxHORIZONTAL);
    GetSheetRefData()->m_defaultGridCellAttr.SetLevel(wxSHEET_AttrLevelBottom);
    GetSheetRefData()->m_defaultGridCellAttr.SetOverflow(TRUE);
    GetSheetRefData()->m_defaultGridCellAttr.SetOverflowMarker(TRUE);
    GetSheetRefData()->m_defaultGridCellAttr.SetReadOnly(FALSE);
    GetSheetRefData()->m_defaultGridCellAttr.SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
    GetSheetRefData()->m_defaultGridCellAttr.SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    GetSheetRefData()->m_defaultGridCellAttr.SetRenderer(GetDefaultRendererForType(wxSHEET_VALUE_STRING));
    GetSheetRefData()->m_defaultGridCellAttr.SetEditor(GetDefaultEditorForType(wxSHEET_VALUE_STRING));

    wxFont labelFont = wxWindow::GetFont();
    labelFont.SetWeight( wxBOLD );
    
    // default row label attr
    GetSheetRefData()->m_defaultRowLabelAttr.Create();
    GetSheetRefData()->m_defaultRowLabelAttr.SetKind(wxSHEET_AttrDefault);
    GetSheetRefData()->m_defaultRowLabelAttr.SetFont(labelFont); // need window created for this
    GetSheetRefData()->m_defaultRowLabelAttr.SetAlignment(wxALIGN_CENTRE);
    GetSheetRefData()->m_defaultRowLabelAttr.SetOrientation(wxHORIZONTAL);
    GetSheetRefData()->m_defaultRowLabelAttr.SetLevel(wxSHEET_AttrLevelBottom);
    GetSheetRefData()->m_defaultRowLabelAttr.SetOverflow(FALSE);
    GetSheetRefData()->m_defaultRowLabelAttr.SetOverflowMarker(FALSE); // just cutoff, looks better?
    GetSheetRefData()->m_defaultRowLabelAttr.SetReadOnly(TRUE);
    GetSheetRefData()->m_defaultRowLabelAttr.SetForegroundColour(*wxBLACK);
    GetSheetRefData()->m_defaultRowLabelAttr.SetBackgroundColour(GetBackgroundColour());
    GetSheetRefData()->m_defaultRowLabelAttr.SetRenderer(GetDefaultRendererForType(wxSHEET_VALUE_LABEL));
    GetSheetRefData()->m_defaultRowLabelAttr.SetEditor(GetDefaultEditorForType(wxSHEET_VALUE_STRING));

    // default col and corner label attr are identical to row label attr
    GetSheetRefData()->m_defaultColLabelAttr    = GetSheetRefData()->m_defaultRowLabelAttr.Clone();
    GetSheetRefData()->m_defaultCornerLabelAttr = GetSheetRefData()->m_defaultRowLabelAttr.Clone();

    // subwindow components that make up the wxSheet
    m_gridWin        = new wxSheetGridWindow( this, ID_GRID_WINDOW );    
    m_rowLabelWin    = new wxSheetRowLabelWindow( this, ID_ROW_LABEL_WINDOW );
    m_colLabelWin    = new wxSheetColLabelWindow( this, ID_COL_LABEL_WINDOW );
    m_cornerLabelWin = new wxSheetCornerLabelWindow( this, ID_CORNER_LABEL_WINDOW );
    m_horizScrollBar = new wxScrollBar(this, ID_HORIZ_SCROLLBAR, wxDefaultPosition,
                                       wxDefaultSize, wxSB_HORIZONTAL);
    m_vertScrollBar  = new wxScrollBar(this, ID_VERT_SCROLLBAR, wxDefaultPosition,
                                       wxDefaultSize, wxSB_VERTICAL);

    GetSheetRefData()->m_defaultRowHeight = m_gridWin->GetCharHeight();

#if defined(__WXMOTIF__) || defined(__WXGTK__)  // see also text ctrl sizing in ShowCellEditControl()
    GetSheetRefData()->m_defaultRowHeight += 8;
#else
    GetSheetRefData()->m_defaultRowHeight += 4;
#endif

    GetSheetRefData()->AddSheet(this);
    return TRUE;
}

wxSheet::~wxSheet()
{
    StopMouseTimer();
    GetSheetRefData()->RemoveSheet(this);
    
    // destroy the edit control if this is the "parent"
    if (GetEditControl().Ok() && GetEditControl().GetControl())
    {
        wxWindowID win_id = GetEditControl().GetControl()->GetId(); 
        if (FindWindow(win_id) == GetEditControl().GetControl())
        {
            GetSheetRefData()->m_cellEditor.DestroyControl();
            GetSheetRefData()->m_cellEditor.Destroy();
            GetSheetRefData()->m_cellEditorCoords = wxNullSheetCoords;
        }
    }

#ifdef DEBUG_ATTR_CACHE
    size_t total = gs_nAttrCacheHits + gs_nAttrCacheMisses;
    wxPrintf(_T("wxSheet attribute cache statistics: "
                "total: %u, hits: %u (%u%%)\n"),
             total, gs_nAttrCacheHits,
             total ? (gs_nAttrCacheHits*100) / total : 0);
#endif
}

bool wxSheet::Destroy()
{
    GetSheetRefData()->RemoveSheet(this);
    return wxWindow::Destroy();
}

void wxSheet::RefSheet(wxSheet* sheet)
{
    wxCHECK_RET(sheet, wxT("Invalid wxSheet"));
    GetSheetRefData()->RemoveSheet(this);  // remove from old data
    UnRef();
    Ref(*sheet);
    GetSheetRefData()->AddSheet(this);
    CalcWindowSizes();
}

bool wxSheet::CreateGrid( int numRows, int numCols )
{
    wxCHECK_MSG( !m_created, FALSE, wxT("wxSheet::CreateGrid called more than once") );

    GetSheetRefData()->m_numRows = numRows;
    GetSheetRefData()->m_numCols = numCols;

    //m_table = new wxSheetStringSparseTable( m_numRows, m_numCols ); // FIXME test code
    GetSheetRefData()->m_table = new wxSheetStringTable( numRows, numCols );
    GetSheetRefData()->m_table->SetView( this );
    GetSheetRefData()->m_ownTable  = TRUE;

    CalcWindowSizes();

    m_created = TRUE;

    return m_created;
}

bool wxSheet::SetTable( wxSheetTableBase *table, bool takeOwnership )
{
    if ( m_created )
    {
        // stop all processing
        m_created = FALSE;

        if (GetSheetRefData()->m_ownTable && GetSheetRefData()->m_table)
        {
            wxSheetTableBase *t = GetSheetRefData()->m_table;
            GetSheetRefData()->m_table = NULL;
            delete t;
        }
        if (GetSelection())
            GetSelection()->Clear();

        GetSheetRefData()->m_table = NULL;
        GetSheetRefData()->m_numRows = 0;
        GetSheetRefData()->m_numCols = 0;
    }
    if (table)
    {
        GetSheetRefData()->m_numRows = table->GetNumberRows();
        GetSheetRefData()->m_numCols = table->GetNumberCols();

        GetSheetRefData()->m_table = table;
        GetSheetRefData()->m_table->SetView( this );
        GetSheetRefData()->m_ownTable = takeOwnership;
        
        CalcWindowSizes();

        m_created = TRUE;
    }

    return m_created;
}

// ------------------------------------------------------------------------
// Dimensions of the sheet

wxSheetCell_Type wxSheet::GetCellCoordsType(const wxSheetCoords& coords)
{
    if ((coords.m_row >= 0) && (coords.m_col >= 0))
    {
        return wxSHEET_CELL_GRID;
    }
    if (coords.m_row == -1)
    {
        if (coords.m_col == -1)
            return wxSHEET_CELL_CORNERLABEL;
        if (coords.m_col >= 0)
            return wxSHEET_CELL_COLLABEL;
        
        return wxSHEET_CELL_UNKNOWN;
    }
    if (coords.m_col == -1)
    {
        if (coords.m_row >= 0)
            return wxSHEET_CELL_ROWLABEL;
        
        return wxSHEET_CELL_UNKNOWN;
    }
    
    //if (IsGridCell(coords))        return wxSHEET_CELL_GRID;
    //if (IsRowLabelCell(coords))    return wxSHEET_CELL_ROWLABEL;
    //if (IsColLabelCell(coords))    return wxSHEET_CELL_COLLABEL;
    //if (IsCornerLabelCell(coords)) return wxSHEET_CELL_CORNERLABEL;
    
    return wxSHEET_CELL_UNKNOWN;
}

void wxSheet::ClearGrid()
{
    if ( GetTable() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(FALSE);

        GetTable()->Clear();
        RefreshGridWindow();
    }
}

bool wxSheet::DeleteRows( int pos, int numRows, bool WXUNUSED(updateLabels) )
{
    wxCHECK_MSG(m_created, FALSE, wxT("Called wxSheet::DeleteRows() before calling CreateGrid()"));
    // TODO: something with updateLabels flag

    if ( GetTable() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);

        // the table sends the results as a grid table message
        return GetTable()->DeleteRows( pos, numRows );
    }
    return FALSE;
}

bool wxSheet::DeleteCols( int pos, int numCols, bool WXUNUSED(updateLabels) )
{
    wxCHECK_MSG(m_created, FALSE, wxT("Called wxSheet::DeleteCols() before calling CreateGrid()"));
    // TODO: something with updateLabels flag

    if ( GetTable() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);

        // the table sends the results as a grid table message
        return GetTable()->DeleteCols( pos, numCols );
    }
    return FALSE;
}

bool wxSheet::InsertRows( int pos, int numRows, bool WXUNUSED(updateLabels) )
{
    wxCHECK_MSG(m_created, FALSE, wxT("Called wxSheet::InsertRows() before calling CreateGrid()"));
    // TODO: something with updateLabels flag

    if ( GetTable() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);

        // the table sends the results as a grid table message
        return GetTable()->InsertRows( pos, numRows );
    }
    return FALSE;
}

bool wxSheet::InsertCols( int pos, int numCols, bool WXUNUSED(updateLabels) )
{
    wxCHECK_MSG(m_created, FALSE, wxT("Called wxSheet::InsertCols() before calling CreateGrid()"));
    // TODO: something with updateLabels flag

    if ( GetTable() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);

        // the table sends the results as a grid table message
        return GetTable()->InsertCols( pos, numCols );
    }
    return FALSE;
}

bool wxSheet::AppendRows( int numRows, bool WXUNUSED(updateLabels) )
{
    wxCHECK_MSG(m_created, FALSE, wxT("Called wxSheet::AppendRows() before calling CreateGrid()"));
    // TODO: something with updateLabels flag

    if ( GetTable() )
    {
        // the table sends the results as a grid table message
        return GetTable()->AppendRows( numRows );
    }
    return FALSE;
}

bool wxSheet::AppendCols( int numCols, bool WXUNUSED(updateLabels) )
{
    wxCHECK_MSG(m_created, FALSE, wxT("Called wxSheet::AppendCols() before calling CreateGrid()"));
    // TODO: something with updateLabels flag

    if ( GetTable() )
    {
        // the table sends the results as a grid table message
        return GetTable()->AppendCols( numCols );
    }
    return FALSE;
}

bool wxSheet::SetNumberCols( int cols )
{
    wxCHECK_MSG(m_created, FALSE, wxT("Called wxSheet::SetNumberCols() before calling CreateGrid()") );
    wxCHECK_MSG(cols >= 0, FALSE, wxT("Called wxSheet::SetNumberCols() with negative cols") );

    if (GetNumberCols() > cols) 
        return DeleteCols(cols, GetNumberCols() - cols);
    else if (GetNumberCols() < cols) 
        return AppendCols(cols - GetNumberCols());
    
    return FALSE;
}

bool wxSheet::SetNumberRows( int rows )
{
    wxCHECK_MSG(m_created, FALSE, wxT("Called wxSheet::SetNumberRows() before calling CreateGrid()") );
    wxCHECK_MSG(rows >= 0, FALSE, wxT("Called wxSheet::SetNumberRows() with negative rows") );

    if (GetNumberRows() > rows) 
        return DeleteRows(rows, GetNumberRows() - rows);
    else if (GetNumberRows() < rows) 
        return AppendRows(rows - GetNumberRows());
    
    return FALSE;
}

bool wxSheet::SetNumberCells( int rows, int cols )
{
    return SetNumberRows(rows) || SetNumberCols(cols);
}

// ----------------------------------------------------------------------------
// Dimensions of the row and column sizes

void wxSheet::SetDefaultRowHeight( int height, bool resizeExistingRows )
{
    GetSheetRefData()->m_defaultRowHeight = wxMax( height, GetMinimalAcceptableRowHeight() );

    if ( resizeExistingRows )
    {
        // resizing all rows to the default row size, clear arrays 
        GetSheetRefData()->m_rowHeights.Empty();
        GetSheetRefData()->m_rowBottoms.Empty();
        if ( !GetBatchCount() )
            CalcWindowSizes();
    }
}

void wxSheet::SetDefaultColWidth( int width, bool resizeExistingCols )
{
    GetSheetRefData()->m_defaultColWidth = wxMax( width, GetMinimalAcceptableColWidth() );

    if ( resizeExistingCols )
    {
        // resizing all columns to the default column size, clear arrays 
        GetSheetRefData()->m_colWidths.Empty();
        GetSheetRefData()->m_colRights.Empty();
        if ( !GetBatchCount() )
            CalcWindowSizes();
    }
}

int wxSheet::GetMinimalColWidth(int col) const
{
    const wxLongToLongHashMap::const_iterator it = ((const wxLongToLongHashMap*)&GetSheetRefData()->m_colMinWidths)->find(col);
    return it != GetSheetRefData()->m_colMinWidths.end() ? (int)it->second : GetSheetRefData()->m_minAcceptableColWidth;
}
int wxSheet::GetMinimalRowHeight(int row) const
{
    const wxLongToLongHashMap::const_iterator it = ((const wxLongToLongHashMap*)&GetSheetRefData()->m_rowMinHeights)->find(row);
    return it != GetSheetRefData()->m_rowMinHeights.end() ? (int)it->second : GetSheetRefData()->m_minAcceptableRowHeight;
}
void wxSheet::SetMinimalColWidth( int col, int width )
{
    if (width > GetMinimalAcceptableColWidth()) 
        GetSheetRefData()->m_colMinWidths[col] = width;
    else
        GetSheetRefData()->m_colMinWidths.erase(col);   // remove width if it exists, using default  
}
void wxSheet::SetMinimalRowHeight( int row, int width )
{
    if (width > GetMinimalAcceptableRowHeight()) 
        GetSheetRefData()->m_rowMinHeights[row] = width;
    else
        GetSheetRefData()->m_rowMinHeights.erase(row);  // remove width if it exists, using default  
}

int wxSheet::GetColWidth(int col) const
{
    if (col == -1)
        return GetRowLabelWidth();
    if (GetSheetRefData()->m_colWidths.IsEmpty())
        return GetDefaultColWidth();
    
    wxCHECK_MSG(ContainsGridCol(col), GetDefaultColWidth(), _T("invalid column index") );
    return GetSheetRefData()->m_colWidths[col];
}
int wxSheet::GetColLeft(int col) const
{
    if (col == -1)  
        return 1; // 1 for border
    if (GetSheetRefData()->m_colRights.IsEmpty())
        return col * GetDefaultColWidth();
    
    wxCHECK_MSG(ContainsGridCol(col), col * GetDefaultColWidth(), _T("invalid column index") );
    return GetSheetRefData()->m_colRights[col] - GetSheetRefData()->m_colWidths[col];  
}
int wxSheet::GetColRight(int col) const
{
    if (col == -1)
        return GetRowLabelWidth();
    if (GetSheetRefData()->m_colRights.IsEmpty())
        return (col + 1) * GetDefaultColWidth();
    
    wxCHECK_MSG(ContainsGridCol(col), (col + 1) * GetDefaultColWidth(), _T("invalid column index") );
    return GetSheetRefData()->m_colRights[col];
}

int wxSheet::GetRowHeight(int row) const
{
    if (row == -1)
        return GetColLabelHeight();
    if (GetSheetRefData()->m_rowHeights.IsEmpty())
        return GetDefaultRowHeight();
    
    wxCHECK_MSG(ContainsGridRow(row), GetDefaultRowHeight(), _T("invalid row index") );
    return GetSheetRefData()->m_rowHeights[row];   
}
int wxSheet::GetRowTop(int row) const
{
    if (row == -1)
        return 1; // 1 for border
    if (GetSheetRefData()->m_rowBottoms.IsEmpty())
        return row * GetDefaultRowHeight();
    
    wxCHECK_MSG(ContainsGridRow(row), row * GetDefaultRowHeight(), _T("invalid row index") );
    return GetSheetRefData()->m_rowBottoms[row] - GetSheetRefData()->m_rowHeights[row];       
}
int wxSheet::GetRowBottom(int row) const
{
    if (row == -1)
        return GetColLabelHeight();
    
    if (GetSheetRefData()->m_rowBottoms.IsEmpty())
        return (row + 1) * GetDefaultRowHeight();
    
    wxCHECK_MSG(ContainsGridRow(row), (row + 1) * GetDefaultRowHeight(), _T("invalid row index") );
    return GetSheetRefData()->m_rowBottoms[row];           
}

wxSize wxSheet::GetCellSize(const wxSheetCoords& coords) const
{
    return wxSize(GetColWidth(coords.m_row), GetRowHeight(coords.m_row));
}

wxSize wxSheet::GetCellBestSize(const wxSheetCoords& coords, wxDC *dc) const
{
    wxCHECK_MSG(ContainsCell(coords), wxSize(GetDefaultColWidth(), GetDefaultRowHeight()), wxT("Invalid coords"));
    wxSheetCellAttr attr(GetAttr(coords));
    wxSheet *sheet = (wxSheet*)this;
    wxSheetCellRenderer renderer(attr.GetRenderer(sheet, coords));
    if (!dc)
    {
        wxClientDC cdc(sheet);
        return renderer.GetBestSize(*sheet, attr, cdc, coords);
    }
    
    return renderer.GetBestSize(*sheet, attr, *dc, coords);
}

bool wxSheet::IsCellShown( const wxSheetCoords& coords ) const
{
    wxCHECK_MSG(ContainsCell(coords), FALSE, wxT("Invalid coords in wxSheet::IsCellShown"));
    return (GetColWidth(coords.GetCol()) > 0) && (GetRowHeight(coords.GetRow()) > 0);
}

void wxSheet::SetRowHeight( int row, int height )
{
    if (row == -1)
    {
        SetColLabelHeight(height);
        return;
    }
    
    wxCHECK_RET(ContainsGridRow(row), _T("invalid row index") );

    // See comment in SetColWidth
    if ( height < GetMinimalAcceptableRowHeight() ) 
        return; 

    // need to really create the array
    if ( GetSheetRefData()->m_rowHeights.IsEmpty() )
        InitRowHeights();

    int h = wxMax( 0, height );
    int diff = h - GetSheetRefData()->m_rowHeights[row];
    GetSheetRefData()->m_rowHeights[row] = h;
    int numRows = GetNumberRows();
    
    for ( int i = row; i < numRows; i++ )
        GetSheetRefData()->m_rowBottoms[i] += diff;
    
    if ( !GetBatchCount() )
    {
        CalcWindowSizes();
        
        // Redraw everything below, for refed editors
        wxRect rect( CellToRect(wxSheetCoords(row, -1), TRUE) );
        rect.height = GetGridVirtualSize().y - rect.y;
        RefreshRowLabelWindow(TRUE, &rect);
        // draw this row and below
        wxSheetBlock block(row, 0, GetNumberRows()-row+1, GetNumberCols());
        RefreshGridCellBlock(block);
    }
}

void wxSheet::SetColWidth( int col, int width )
{
    if (col == -1)
    {
        SetRowLabelWidth(width);
        return;
    }

    wxCHECK_RET(ContainsGridCol(col), _T("invalid column index") );

    // should we check that it's bigger than GetMinimalColWidth(col) here?
    //                                                                 (VZ)
    // No, because it is reasonable to assume the library user know's
    // what he is doing. However whe should test against the weaker
    // constariant of minimalAcceptableWidth, as this breaks rendering
    //
    // This test then fixes sf.net bug #645734
    if ( width < GetMinimalAcceptableColWidth() ) 
        return; 

    if ( GetSheetRefData()->m_colWidths.IsEmpty() ) // need to really create the array
        InitColWidths();

    // if < 0 calc new width from label
    if ( width < 0 )
    {
        wxSize size(GetCellBestSize(wxSheetCoords(-1, col)));
        width = size.x + 6;
    }
    
    int w = wxMax( 0, width );
    int diff = w - GetSheetRefData()->m_colWidths[col];
    GetSheetRefData()->m_colWidths[col] = w;
    int numCols = GetNumberCols();
    
    for ( int i = col; i < numCols; i++ )
        GetSheetRefData()->m_colRights[i] += diff;
    
    if ( !GetBatchCount() )
    {
        CalcWindowSizes();
        
        // Redraw everything to right, for refed editors
        wxRect rect( CellToRect(wxSheetCoords(-1, col), TRUE) );
        rect.width = GetGridVirtualSize().x - rect.x;
        RefreshColLabelWindow(TRUE, &rect);
        // draw this col and to right
        wxSheetBlock block(0, col, GetNumberRows(), GetNumberCols()-col+1);
        RefreshGridCellBlock(block);
    }
}

// ----------------------------------------------------------------------------
// Auto sizing of the row/col widths/heights

void wxSheet::AutoSizeColOrRow( int colOrRow, bool setAsMin, bool column )
{
    wxSheetCoords coords( colOrRow, colOrRow );
    wxCHECK_RET((column && ContainsGridCol(colOrRow)) || (!column && ContainsGridRow(colOrRow)), 
                wxT("Invalid coords in wxSheet::AutoSizeColOrRow"));
    
    wxClientDC dc(m_gridWin);
    
    wxCoord extent, extentMax = 0;
    int max = column ? GetNumberRows() : GetNumberCols();
    for ( int rowOrCol = -1; rowOrCol < max; rowOrCol++ )
    {
        if ( column )
            coords.SetRow(rowOrCol);
        else
            coords.SetCol(rowOrCol);

        wxSize size(GetCellBestSize(coords, &dc));
        extent = column ? size.x : size.y;
        if ( extent > extentMax )
            extentMax = extent;
    }

    // if empty column - give default extent (notice that if extentMax is less
    // than default extent but != 0, it's ok)
    if ( !extentMax )
        extentMax = column ? GetDefaultColWidth() : GetDefaultRowHeight();
    else
        extentMax += column ? 10 : 6; // leave some space around text

    if ( column )
        SetColWidth(coords.GetCol(), extentMax);
    else
        SetRowHeight(coords.GetRow(), extentMax);
    
    if ( setAsMin )
    {
        if ( column )
            SetMinimalColWidth(colOrRow, extentMax);
        else
            SetMinimalRowHeight(colOrRow, extentMax);
    }
}

int wxSheet::SetOrCalcColumnSizes(bool calcOnly, bool setAsMin)
{
    int width = GetRowLabelWidth();

    if ( !calcOnly )
        BeginBatch();

    int numCols = GetNumberCols();
    for ( int col = 0; col < numCols; col++ )
    {
        if ( !calcOnly )
            AutoSizeColumn(col, setAsMin);

        width += GetColWidth(col);
    }

    if ( !calcOnly )
        EndBatch();

    return width;
}

int wxSheet::SetOrCalcRowSizes(bool calcOnly, bool setAsMin)
{
    int height = GetColLabelHeight();

    if ( !calcOnly )
        BeginBatch();

    int numRows = GetNumberRows();
    for ( int row = 0; row < numRows; row++ )
    {
        if ( !calcOnly )
            AutoSizeRow(row, setAsMin);

        height += GetRowHeight(row);
    }

    if ( !calcOnly )
        EndBatch();

    return height;
}

void wxSheet::AutoSize()
{
    BeginBatch();

    wxSize size(SetOrCalcColumnSizes(FALSE), SetOrCalcRowSizes(FALSE));
    
    // round up the size to a multiple of scroll step - this ensures that we
    // won't get the scrollbars if we're sized exactly to this width
    // CalcWindowSizes adds m_extraWidth + 1 etc. to calculate the necessary
    // scrollbar steps
    wxSize sizeFit(GetGridVirtualSize());
    sizeFit.x += GetRowLabelWidth();
    sizeFit.y += GetColLabelHeight();
    
    // distribute the extra space between the columns/rows to avoid having
    // extra white space

    int numCols = GetNumberCols();
    int numRows = GetNumberRows();
    
    // Remove the extra m_extraWidth + 1 added above
    wxCoord diff = sizeFit.x - size.x + (GetSheetRefData()->m_extraWidth + 1);
    if ( diff && numCols )
    {
        // try to resize the columns uniformly
        wxCoord diffPerCol = diff / numCols;
        if ( diffPerCol != 0 )
        {
            for ( int col = 0; col < numCols; col++ )
                SetColWidth(col, GetColWidth(col) + diffPerCol);
        }

        // add remaining amount to the last columns
        diff -= diffPerCol * numCols;
        if ( diff != 0 )
        {
            for ( int col = numCols - 1; col >= numCols - diff; col-- )
                SetColWidth(col, GetColWidth(col) + 1);
        }
    }

    // same for rows
    diff = sizeFit.y - size.y - (GetSheetRefData()->m_extraHeight + 1);
    if ( diff && numRows )
    {
        // try to resize the columns uniformly
        wxCoord diffPerRow = diff / numRows;
        if ( diffPerRow != 0 )
        {
            for ( int row = 0; row < numRows; row++ )
                SetRowHeight(row, GetRowHeight(row) + diffPerRow);
        }

        // add remaining amount to the last rows
        diff -= diffPerRow * numRows;
        if ( diff != 0 )
        {
            for ( int row = numRows - 1; row >= numRows - diff; row-- )
                SetRowHeight(row, GetRowHeight(row) + 1);
        }
    }

    sizeFit = GetGridVirtualSize();
    sizeFit.x += GetRowLabelWidth();
    sizeFit.y += GetColLabelHeight();
    
    SetClientSize(sizeFit + wxSize(2,2)); // FIXME ? Where's the extra 2 from in wxGTK
    EndBatch();
}

void wxSheet::AutoSizeRowLabelHeight( int row )
{
    wxCHECK_RET(ContainsGridRow(row), wxT("Invalid row in wxSheet::AutoSizeRowLabelHeight"));

    // Hide the edit control, so it won't interfer with drag-shrinking.
    if ( IsCellEditControlShown() )
        DisableCellEditControl(TRUE);

    // autosize row height depending on label text
    wxSheetCoords coords(row, -1);
    wxSize size(GetCellBestSize(coords));

    if (size.y < GetDefaultRowHeight())
        size.y = GetDefaultRowHeight();
    
    SetRowHeight(row, size.y);
}

void wxSheet::AutoSizeColLabelWidth( int col )
{
    wxCHECK_RET(ContainsGridCol(col), wxT("Invalid col in wxSheet::AutoSizeColLabelWidth"));

    // Hide the edit control, so it won't interfer with drag-shrinking.
    if ( IsCellEditControlShown() )
        DisableCellEditControl(TRUE);

    // autosize column width depending on label text
    wxSheetCoords coords(-1, col);
    wxSize size(GetCellBestSize(coords));
    
    if (size.x < GetDefaultColWidth())
        size.x = GetDefaultColWidth();
    
    SetColWidth(col, size.x);
}

void wxSheet::SetEqualColWidths(int min_width)
{
    GetSheetRefData()->m_equal_col_widths = wxMax(min_width, 0);

    // don't fail here, since EVT_SIZEs are generated before the grid is 
    if (m_created && (min_width > 0) && (GetNumberCols() > 0))
    {
        int colwidth = (GetClientSize().GetWidth() - GetRowLabelWidth())/GetNumberCols();
        SetDefaultColWidth( colwidth, TRUE );
    }
}

// ----------------------------------------------------------------------------
// the idea is to call these functions only when necessary because they create
// quite big arrays which eat memory mostly unnecessary - in particular, if
// default widths/heights are used for all rows/columns, we may not use these
// arrays at all
//
// with some extra code, it should be possible to only store the
// widths/heights different from default ones but this will be done later...
// ----------------------------------------------------------------------------

void InitRowColSizeEdge(wxArrayInt &sizes, wxArrayInt &edges, 
                        int default_size, int count)
{
    sizes.Empty();
    edges.Empty();
    sizes.Alloc( count );
    edges.Alloc( count );
    
    sizes.Add( default_size, count );
    int i, edge = 0;
    for ( i = 0; i < count; i++ )
    {
        edge += default_size;
        edges.Add( edge );
    }    
}
void wxSheet::InitRowHeights()
{
    InitRowColSizeEdge(GetSheetRefData()->m_rowHeights, GetSheetRefData()->m_rowBottoms, 
                       GetDefaultRowHeight(), GetNumberRows());
}
void wxSheet::InitColWidths()
{
    InitRowColSizeEdge(GetSheetRefData()->m_colWidths, GetSheetRefData()->m_colRights, 
                       GetDefaultColWidth(), GetNumberCols());
}

// given arrays of row/col widths/heights and their right/bottom edges it will 
// add to them if count > 0, remove if count < 0, 
// append if pos < 0, else insert if pos > 0
static void ResizeRowColSizeEdges( wxArrayInt &sizes, wxArrayInt &edges, 
                                   int default_size, int pos, int count )
{
    int old_count = sizes.GetCount();
    int start_pos = 0, end_pos = old_count + count;
    int edge = 0;
    
    if (count > 0)                               // adding rows/cols
    {
        if (pos < 0)                             // append 
        {
            sizes.Add( default_size, count );
            edges.Add( 0, count );
            edge = (old_count > 0) ? edges[old_count-1] : 0;
            start_pos = old_count;
        }
        else                                     // insert
        {
            sizes.Insert( default_size, pos, count );
            edges.Insert( 0, pos, count );
            edge      = (pos > 0) ? edges[pos-1] : 0;
            start_pos = (pos > 0) ? pos : 0;
        }
    }
    else if (count < 0)                          // deleting rows/cols
    {
        sizes.RemoveAt( pos, count );
        edges.RemoveAt( pos, count );
        edge      = (pos > 0) ? edges[pos-1] : 0;
        start_pos = (pos > 0) ? pos : 0;
    }
    else
        return;
        
    for ( int i = start_pos; i < end_pos; i++ )
    {
        edge += sizes[i];
        edges[i] = edge;
    }    
}

bool wxSheet::ProcessTableMessage( wxSheetTableMessage& msg )
{
    switch ( msg.GetId() )
    {
        case wxSHEETTABLE_REQUEST_VIEW_GET_VALUES:
            return GetModelValues();

        case wxSHEETTABLE_REQUEST_VIEW_SEND_VALUES:
            return SetModelValues();

        case wxSHEETTABLE_NOTIFY_ROWS_INSERTED :
        case wxSHEETTABLE_NOTIFY_ROWS_APPENDED :
        case wxSHEETTABLE_NOTIFY_ROWS_DELETED  :
        case wxSHEETTABLE_NOTIFY_COLS_INSERTED :
        case wxSHEETTABLE_NOTIFY_COLS_APPENDED :
        case wxSHEETTABLE_NOTIFY_COLS_DELETED  :
            return Redimension( msg );

        default:
            return FALSE;
    }
}

// the grid table sends a message to say that it has been redimensioned
bool wxSheet::Redimension( wxSheetTableMessage& msg )
{
    bool result = FALSE;

    // Clear the attribute cache as the attribute might refer to a different
    // cell than stored in the cache after adding/removing rows/columns.
    ClearAttrCache();
    // By the same reasoning, the editor should be dismissed if columns are
    // added or removed. And for consistency, it should IMHO always be
    // removed, not only if the cell "underneath" it actually changes.
    // For now, I intentionally do not save the editor's content as the
    // cell it might want to save that stuff to might no longer exist.
    if (IsCellEditControlCreated())
        DisableCellEditControl(FALSE);
    
    // FIXME - do I want to UpdateRows/Cols for selection or just clear them
    //         the event it would send would be confusing at best
    //         first a deselect and then a select... should sel follow insert?
    //         I don't think it's worth the confusion and since a click on 
    //         a cell is deselect all, the selection is not that precious.
    ClearSelection(TRUE);
    
    switch ( msg.GetId() )
    {
        case wxSHEETTABLE_NOTIFY_ROWS_INSERTED:
        {
            int pos = msg.GetPosition();
            int numRows = msg.GetRowsCols();
            wxCHECK_MSG((numRows > 0) && ((pos == 0)||(pos < GetNumberRows())), FALSE,
                        wxT("Invalid row insertion in wxSheet::Redimension"));

            //if ( m_selection )
                //m_selection->UpdateRows( pos, numRows );
            
            GetSheetRefData()->m_numRows += numRows;

            if ( !GetSheetRefData()->m_rowHeights.IsEmpty() )
            {
                ResizeRowColSizeEdges( GetSheetRefData()->m_rowHeights, 
                                       GetSheetRefData()->m_rowBottoms, 
                                       GetDefaultRowHeight(), pos, numRows );                
            }
            
            // if it was an empty grid the current cell is undefined...
            if ( !ContainsGridCell(GetGridCursorCell()) && ContainsGridCell(wxSheetCoords(0,0)) )
                SetGridCursorCell( wxSheetCoords(0, 0) );

            wxSheetCellAttrProvider *attrProvider = GetTable()->GetAttrProvider();
            
            if (attrProvider)
                attrProvider->UpdateAttrRows( pos, numRows );

            if ( !GetBatchCount() )
            {
                AdjustScrollbars();
                RefreshRowLabelWindow();
            }
            
            result = TRUE;
            break;
        }
        case wxSHEETTABLE_NOTIFY_ROWS_APPENDED:
        {
            int numRows = msg.GetRowsCols();
            wxCHECK_MSG(numRows > 0, FALSE, wxT("Invalid row appending in wxSheet::Redimension"));
            GetSheetRefData()->m_numRows += numRows;

            if ( !GetSheetRefData()->m_rowHeights.IsEmpty() )
            {
                ResizeRowColSizeEdges( GetSheetRefData()->m_rowHeights, 
                                       GetSheetRefData()->m_rowBottoms, 
                                       GetDefaultRowHeight(), -1, numRows );                
            }
            
            // if it was an empty grid the current cell is undefined...
            if ( !ContainsGridCell(GetGridCursorCell()) && ContainsGridCell(wxSheetCoords(0,0)) )
                SetGridCursorCell( wxSheetCoords(0, 0) );
            
            if ( !GetBatchCount() )
            {
                AdjustScrollbars();
                RefreshRowLabelWindow();
            }
            
            result = TRUE;
            break;
        }
        case wxSHEETTABLE_NOTIFY_ROWS_DELETED:
        {
            int pos = msg.GetPosition();
            int numRows = msg.GetRowsCols();
            wxCHECK_MSG((numRows > 0) && ContainsGridRow(pos) && (GetSheetRefData()->m_numRows - numRows >= 0), 
                        FALSE, wxT("Invalid row deletion in wxSheet::Redimension"));
            
            //if ( m_selection )
                //m_selection->UpdateRows( pos, -((int)numRows) );
            
            GetSheetRefData()->m_numRows -= numRows;

            if ( !GetSheetRefData()->m_rowHeights.IsEmpty() )
            {
                ResizeRowColSizeEdges( GetSheetRefData()->m_rowHeights, 
                                       GetSheetRefData()->m_rowBottoms, 
                                       GetDefaultRowHeight(), pos, -numRows );                
            }
            
            if ( !GetNumberRows() )
                GetSheetRefData()->m_cursorCoords = wxNullSheetCoords;
            else if ( GetGridCursorRow() >= GetNumberRows() )
                GetSheetRefData()->m_cursorCoords.m_row -= numRows;
            
            wxSheetCellAttrProvider * attrProvider = GetTable()->GetAttrProvider();
            
            if (attrProvider) 
            {
                attrProvider->UpdateAttrRows( pos, -((int)numRows) );
// ifdef'd out following patch from Paul Gammans
#if 0
                // No need to touch column attributes, unless we
                // removed _all_ rows, in this case, we remove
                // all column attributes.
                // I hate to do this here, but the
                // needed data is not available inside UpdateAttrRows.
                if ( !GetNumberRows() )
                    attrProvider->UpdateAttrCols( 0, -GetNumberCols() );
#endif
            }
            
            if ( !GetBatchCount() )
            {
                AdjustScrollbars();
                RefreshRowLabelWindow();
            }
            
            result = TRUE;
            break;
        }
        case wxSHEETTABLE_NOTIFY_COLS_INSERTED:
        {
            int pos = msg.GetPosition();
            int numCols = msg.GetRowsCols();
            wxCHECK_MSG((numCols > 0) && ((pos == 0)||(pos < GetSheetRefData()->m_numCols)), FALSE, 
                        wxT("Invalid col insertion in wxSheet::Redimension"));
            
            //if ( m_selection )
                //m_selection->UpdateCols( pos, numCols );
            
            GetSheetRefData()->m_numCols += numCols;

            if ( !GetSheetRefData()->m_colWidths.IsEmpty() )
            {
                ResizeRowColSizeEdges( GetSheetRefData()->m_colWidths, 
                                       GetSheetRefData()->m_colRights, 
                                       GetDefaultColWidth(), pos, numCols );                
            }
            
            // if it was an empty grid the current cell is undefined...
            if ( !ContainsGridCell(GetGridCursorCell()) && ContainsGridCell(wxSheetCoords(0,0)) )
                SetGridCursorCell( wxSheetCoords(0, 0) );
            
            wxSheetCellAttrProvider* attrProvider = GetTable()->GetAttrProvider();
            
            if (attrProvider)
                attrProvider->UpdateAttrCols( pos, numCols );
            
            if ( !GetBatchCount() )
            {
                AdjustScrollbars();
                RefreshColLabelWindow();
            }

            result = TRUE;
            break;
        }
        case wxSHEETTABLE_NOTIFY_COLS_APPENDED:
        {
            int numCols = msg.GetRowsCols();
            wxCHECK_MSG((numCols > 0), FALSE, wxT("Invalid col appending in wxSheet::Redimension"));
            GetSheetRefData()->m_numCols += numCols;
            
            if ( !GetSheetRefData()->m_colWidths.IsEmpty() )
            {
                ResizeRowColSizeEdges( GetSheetRefData()->m_colWidths, 
                                       GetSheetRefData()->m_colRights, 
                                       GetDefaultColWidth(), -1, numCols );                
            }
            
            // if it was an empty grid the current cell is undefined...
            if ( !ContainsGridCell(GetGridCursorCell()) && ContainsGridCell(wxSheetCoords(0,0)) )
                SetGridCursorCell( wxSheetCoords(0, 0) );
            
            if ( !GetBatchCount() )
            {
                AdjustScrollbars();
                RefreshColLabelWindow();
            }
            
            result = TRUE;
            break;
        }
        case wxSHEETTABLE_NOTIFY_COLS_DELETED:
        {
            int pos = msg.GetPosition();
            int numCols = msg.GetRowsCols();
            wxCHECK_MSG((numCols > 0) && ContainsGridCol(pos) && (GetSheetRefData()->m_numCols - numCols >= 0), 
                        FALSE, wxT("Invalid col deletion in wxSheet::Redimension"));

            //if ( m_selection )
                //m_selection->UpdateCols( pos, -((int)numCols) );
            
            GetSheetRefData()->m_numCols -= numCols;

            if ( !GetSheetRefData()->m_colWidths.IsEmpty() )
            {
                ResizeRowColSizeEdges( GetSheetRefData()->m_colWidths, 
                                       GetSheetRefData()->m_colRights, 
                                       GetDefaultColWidth(), pos, -numCols );                
            }
            
            if ( !GetNumberCols() )
                GetSheetRefData()->m_cursorCoords = wxNullSheetCoords;
            else if ( GetGridCursorCol() >= GetNumberCols() )
                GetSheetRefData()->m_cursorCoords.m_col -= numCols;
            
            wxSheetCellAttrProvider * attrProvider = GetTable()->GetAttrProvider();
            
            if (attrProvider) 
            {
                attrProvider->UpdateAttrCols( pos, -((int)numCols) );
// ifdef'd out following patch from Paul Gammans
#if 0
                // No need to touch row attributes, unless we
                // removed _all_ columns, in this case, we remove
                // all row attributes.
                // I hate to do this here, but the
                // needed data is not available inside UpdateAttrCols.
                if ( !GetNumberCols() )
                    attrProvider->UpdateAttrRows( 0, -GetNumberRows() );
#endif
            }
            
            if ( !GetBatchCount() )
            {
                AdjustScrollbars();
                RefreshColLabelWindow();
            }
            
            result = TRUE;
            break;
        }
    }

    if (result && !GetBatchCount())
        RefreshGridWindow();
    
    return result;
}

// ----------------------------------------------------------------------------
// Grid line and cell highlight colouring

void wxSheet::EnableGridLines( bool enable )
{
    if ( enable == GridLinesEnabled() )
        return;
    
    GetSheetRefData()->m_gridLinesEnabled = enable;
    RefreshGridWindow(FALSE);
}

void wxSheet::SetGridLineColour( const wxColour& colour )
{
    wxCHECK_RET(colour.Ok(), wxT("Invalid colour in wxSheet::SetGridLineColour"));   
    if ( GetSheetRefData()->m_gridLineColour == colour )
        return;

    GetSheetRefData()->m_gridLineColour = colour;
    RefreshGridWindow(FALSE);
}

void wxSheet::SetCursorCellHighlightColour( const wxColour& colour )
{
    wxCHECK_RET(colour.Ok(), wxT("Invalid colour in wxSheet::SetGridLineColour"));   
    if ( GetSheetRefData()->m_cursorCellHighlightColour == colour )
        return;
    
    GetSheetRefData()->m_cursorCellHighlightColour = colour;

    if (ContainsCell(GetGridCursorCell()))
        RefreshCell(GetGridCursorCell(), TRUE);
}

void wxSheet::SetCursorCellHighlightPenWidth(int width)
{
    wxCHECK_RET(width>0, wxT("Invalid pen width in wxSheet::SetGridLineColour"));   
    if (GetSheetRefData()->m_cursorCellHighlightPenWidth == width) 
        return;
    
    GetSheetRefData()->m_cursorCellHighlightPenWidth = width;

    if (ContainsCell(GetGridCursorCell()))
        RefreshCell(GetGridCursorCell(), TRUE);
}

void wxSheet::SetCursorCellHighlightROPenWidth(int width)
{
    //wxCHECK_RET(width>0, wxT("Invalid pen width in wxSheet::SetGridLineColour"));   
    if (GetSheetRefData()->m_cursorCellHighlightROPenWidth == width) 
        return;

    GetSheetRefData()->m_cursorCellHighlightROPenWidth = width;

    if (ContainsCell(GetGridCursorCell()))
        RefreshCell(GetGridCursorCell(), TRUE);
}

void wxSheet::SetLabelOutlineColour( const wxColour& colour )
{
    wxCHECK_RET(colour.Ok(), wxT("Invalid colour in wxSheet::SetLabelOutlineColour"));   
    if ( GetSheetRefData()->m_labelOutlineColour == colour )
        return;

    GetSheetRefData()->m_labelOutlineColour = colour;

    RefreshColLabelWindow();
    RefreshRowLabelWindow();
    RefreshCornerLabelWindow();
}

// ----------------------------------------------------------------------------
// Row/Col label size and attributes

int wxSheet::GetRowLabelWidth(bool zero_not_shown) const 
{ 
    if (zero_not_shown && !m_cornerLabelWin->IsShown() && !m_rowLabelWin->IsShown())
        return 0;
    
    return GetSheetRefData()->m_rowLabelWidth; 
}

int wxSheet::GetColLabelHeight(bool zero_not_shown) const 
{ 
    if (zero_not_shown && !m_cornerLabelWin->IsShown() && !m_colLabelWin->IsShown())
        return 0;
    
    return GetSheetRefData()->m_colLabelHeight; 
}    

void wxSheet::SetRowLabelWidth( int width )
{
    width = wxMax( width, 0 );
    if ( width == GetRowLabelWidth() )
        return;
    
    if ( width == 0 )
    {
        m_rowLabelWin->Show( FALSE );
        m_cornerLabelWin->Show( FALSE );
    }
    else if ( GetRowLabelWidth() > 0 )
    {
        m_rowLabelWin->Show( TRUE );
        if ( GetColLabelHeight() > 0 ) 
            m_cornerLabelWin->Show( TRUE );
    }

    GetSheetRefData()->m_rowLabelWidth = width;
    CalcWindowSizes();
}

void wxSheet::SetColLabelHeight( int height )
{
    height = wxMax( height, 0 );
    if ( height == GetColLabelHeight() )
        return;
    
    if ( height == 0 )
    {
        m_colLabelWin->Show( FALSE );
        m_cornerLabelWin->Show( FALSE );
    }
    else if ( GetColLabelHeight() > 0 )
    {
        m_colLabelWin->Show( TRUE );
        if ( GetRowLabelWidth() > 0 ) 
            m_cornerLabelWin->Show( TRUE );
    }

    GetSheetRefData()->m_colLabelHeight = height;
    CalcWindowSizes();
}

// ----------------------------------------------------------------------------
// Spanned cells

bool wxSheet::HasSpannedCells() const
{
    return GetSpannedBlocks()->GetCount() != 0u;
}

wxSheetCoords wxSheet::GetCellOwner( const wxSheetCoords& coords ) const
{
    int n = GetSpannedBlocks()->Index(coords);
    if (n != wxNOT_FOUND)
        return GetSpannedBlocks()->GetBlock(n).GetLeftTop();
    
    return coords;
}

wxSheetBlock wxSheet::GetCellBlock( const wxSheetCoords& coords ) const
{
    int n = GetSpannedBlocks()->Index(coords);
    if (n != wxNOT_FOUND)
        return GetSpannedBlocks()->GetBlock(n);
    
    return wxSheetBlock(coords, 1, 1);    
}

wxSheetCoords wxSheet::GetCellSpan( const wxSheetCoords& coords ) const
{
    int n = GetSpannedBlocks()->Index(coords);
    if (n != wxNOT_FOUND)
    {
        const wxSheetBlock &block = GetSpannedBlocks()->GetBlock(n);
        if (coords == block.GetLeftTop())
            return block.GetSize();
        else
            return block.GetLeftTop() - coords;
    }
    
    return wxSheetCoords(1, 1);
}

void wxSheet::SetCellSpan( const wxSheetBlock& block )
{
    wxCHECK_RET(!block.IsEmpty(), wxT("Cannot set cell size smaller than (1,1)"));
    wxCHECK_RET(ContainsGridCell(block.GetLeftTop()) && ContainsGridCell(block.GetRightBottom()), 
                  wxT("Cannot set cell size for cell out of grid"));

    wxSheetBlock bounds(block);
    const wxArraySheetBlock &arrBlock = GetSpannedBlocks()->GetBlockArray();
    int n, index = wxNOT_FOUND, intersections = 0, count = arrBlock.GetCount();
    
    // Check for multiple intersections, one is fine, but topleft corners must match
    for (n=0; n<count; n++)
    {
        if (arrBlock[n].Intersects(block))
        {
            intersections++;
            index = n;
        }
    }
    
    wxCHECK_RET( (intersections < 2) && 
                ((intersections == 0) || (arrBlock[index].GetLeftTop() == block.GetLeftTop())), 
                 wxT("Setting cell span for cells already spanned"));
    
    // delete old block and expand refresh bounds 
    if (index != wxNOT_FOUND)
    {
        bounds = bounds.Union(arrBlock[index]);
        GetSheetRefData()->m_spannedCells->DeselectBlock(arrBlock[index], FALSE);
    }

    // no need to add a 1x1 block
    if ((block.m_width > 1) || (block.m_height > 1))
    {
        // don't combine, but do sort
        GetSheetRefData()->m_spannedCells->SelectBlock(block, FALSE); 
        GetSheetRefData()->m_spannedCells->Sort();
    }
    
    RefreshGridCellBlock(bounds);
}

// ****************************************************************************

wxSheetCellAttr wxSheet::GetOrCreateAttr(const wxSheetCoords& coords, wxSheetAttr_Type type) const
{
    wxSheetCellAttr attr;
    
    wxCHECK_MSG( GetTable(), attr,
                 _T("we may only be called if CanHaveAttributes() returned TRUE and then m_table should be !NULL") );    
    wxCHECK_MSG( type != wxSHEET_AttrAny, attr, wxT("Cannot create attribute of type wxSHEET_AttrAny") );
    
    // if no attributes then you only have the default ones to work with
    if (!CanHaveAttributes())
        type = wxSHEET_AttrDefault;
    
    if (IsGridCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault : return DoGetDefaultGridAttr();
            case wxSHEET_AttrCell    :
            case wxSHEET_AttrRow     :
            case wxSHEET_AttrCol     :
            {
                wxCHECK_MSG(ContainsGridCell(coords), attr, wxT("Invalid attr coords"));
                attr = GetTable()->GetAttr(coords, type);
                if ( !attr.Ok() )
                {
                    attr.Create();
                    attr.SetKind(type);
                    attr.SetDefaultAttr(DoGetDefaultGridAttr());
                    GetTable()->SetAttr(coords, attr, type);
                }
                else
                    InitAttr(attr, DoGetDefaultGridAttr());
                
                return attr;
            } 
            default : 
            {
                wxFAIL_MSG(wxT("Grid cell attribute kind invalid for GetOrCreateAttr"));
                return attr;
            }
        }
    }
    else if (IsCornerLabelCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault : // only one of these, ever
            case wxSHEET_AttrCell    : return DoGetDefaultCornerLabelAttr();
            default :
            {
                wxFAIL_MSG(wxT("Grid cell attribute kind invalid for GetOrCreateAttr"));
                return attr;
            }
        }
    }
    else if (IsRowLabelCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault : return DoGetDefaultRowLabelAttr();
            case wxSHEET_AttrCell    :
            {
                wxCHECK_MSG(ContainsRowLabelCell(coords), attr, wxT("Invalid attr coords"));
                attr = GetTable()->GetAttr(coords, type);
                if ( !attr.Ok() )
                {
                    attr.Create();
                    attr.SetKind(type);
                    attr.SetDefaultAttr(DoGetDefaultRowLabelAttr());
                    GetTable()->SetAttr(coords, attr, type);
                }
                else
                    InitAttr(attr, DoGetDefaultRowLabelAttr());
                
                return attr;
            }
            default : 
            {
                wxFAIL_MSG(wxT("Grid cell attribute kind invalid for GetOrCreateAttr"));
                return attr;
            }
        }
    }
    else if (IsColLabelCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault : return DoGetDefaultColLabelAttr();
            case wxSHEET_AttrCell    :
            {
                wxCHECK_MSG(ContainsColLabelCell(coords), attr, wxT("Invalid attr coords"));
                attr = GetTable()->GetAttr(coords, type);
                if ( !attr.Ok() )
                {
                    attr.Create();
                    attr.SetKind(type);
                    attr.SetDefaultAttr(DoGetDefaultColLabelAttr());
                    GetTable()->SetAttr(coords, attr, type);
                }
                else
                    InitAttr(attr, DoGetDefaultColLabelAttr());
                
                return attr;
            }
            default : 
            {
                wxFAIL_MSG(wxT("Grid cell attribute kind invalid for GetOrCreateAttr"));
                return attr;
            }
        }
    }
    
    wxFAIL_MSG(wxString::Format(wxT("Unable to get or create attribute for cell(%d,%d)"), coords.m_row, coords.m_col));
    return wxSheetCellAttr();
}

wxSheetCellAttr wxSheet::GetAttr(const wxSheetCoords& coords, wxSheetAttr_Type type) const
{
    if (IsGridCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault : return DoGetDefaultGridAttr();
            case wxSHEET_AttrCell    :
            case wxSHEET_AttrRow     :
            case wxSHEET_AttrCol     :
            case wxSHEET_AttrAny     :
            default                  : 
            {
                wxCHECK_MSG(ContainsGridCell(coords), DoGetDefaultGridAttr(), wxT("Invalid attr coords"));
                wxSheetCellAttr attr;
                
                if ( !LookupAttr(coords, type, attr) )
                {
                    if (GetTable())
                    {
                        attr = GetTable()->GetAttr(coords, type);
                        if (attr.Ok())                    
                            CacheAttr(coords, attr, type);
                    }
                }
    
                if ((type == wxSHEET_AttrAny) && !attr.Ok())
                    attr = DoGetDefaultGridAttr();
        
                return attr;
            }
        }
    }
    else if (IsCornerLabelCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault : 
            case wxSHEET_AttrCell    : // only one of these, ever
            case wxSHEET_AttrAny     : return DoGetDefaultCornerLabelAttr();
            default : 
            {
                wxFAIL_MSG(wxT("Invalid attr type for corner label"));
            }
        }
        
        return wxNullSheetCellAttr;
    }
    else if (IsRowLabelCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault : return DoGetDefaultRowLabelAttr();
            case wxSHEET_AttrCell    :
            case wxSHEET_AttrAny     :
            {
                wxCHECK_MSG(ContainsRowLabelCell(coords), DoGetDefaultRowLabelAttr(), wxT("Invalid attr coords"));
                wxSheetCellAttr attr(GetTable()->GetAttr(coords, type));
                if ((type == wxSHEET_AttrAny) && !attr.Ok())
                    attr = DoGetDefaultRowLabelAttr();
                
                return attr;
            }
            default : 
            {
                wxFAIL_MSG(wxT("Invalid attr type for row label"));
            }
        }
    }
    else if (IsColLabelCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault : return DoGetDefaultColLabelAttr();
            case wxSHEET_AttrCell    :
            case wxSHEET_AttrAny     :
            {
                wxCHECK_MSG(ContainsColLabelCell(coords), DoGetDefaultColLabelAttr(), wxT("Invalid attr coords"));
                wxSheetCellAttr attr(GetTable()->GetAttr(coords, type));
                if ((type == wxSHEET_AttrAny) && !attr.Ok())
                    attr = DoGetDefaultColLabelAttr();
                
                return attr;
            }
            default : 
            {
                wxFAIL_MSG(wxT("Invalid attr type for col label"));
            }
        }
    }
    
    // return garbage, maybe they can hobble along for a bit this way
    wxFAIL_MSG(wxString::Format(wxT("Unable to get attribute for cell(%d,%d)"), coords.m_row, coords.m_col));
    return wxSheetCellAttr(TRUE);
}

void wxSheet::SetAttr(const wxSheetCoords& coords, 
                      const wxSheetCellAttr& attr_, wxSheetAttr_Type type)
{
    wxCHECK_RET(attr_.Ok(), wxT("Invalid attribute in wxSheet::SetDefaultAttr"));
    wxCHECK_RET( type != wxSHEET_AttrAny, wxT("Cannot create attribute of type wxSHEET_AttrAny") );
    wxSheetCellAttr attr(attr_);
    
    if (IsGridCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault : 
            {
                wxSheetCellAttr defAttr(DoGetDefaultGridAttr());
                defAttr.UpdateWith(attr); 
                // force good vals after update
                defAttr.SetDefaultAttr(wxNullSheetCellAttr);
                defAttr.SetKind(wxSHEET_AttrDefault);
                return;
            }
            case wxSHEET_AttrCell :
            case wxSHEET_AttrRow  :
            case wxSHEET_AttrCol  :
            {
                wxCHECK_RET(ContainsGridCell(coords), wxT("Invalid attr coords"));
                if ( CanHaveAttributes() && GetTable() )
                {
                    if (attr.Ok())
                        InitAttr(attr, DoGetDefaultGridAttr());
        
                    GetTable()->SetAttr(coords, attr, type);
                    ClearAttrCache();
                }
                return;
            }
            default : 
            {
                wxFAIL_MSG(wxT("Invalid attr type for grid cell"));
                return;
            }
        }
    }
    else if (IsCornerLabelCell(coords))
    {
        // only one attr for the corner window
        switch (type)
        {
            case wxSHEET_AttrDefault :
            case wxSHEET_AttrCell    :
            {
                wxSheetCellAttr defAttr(DoGetDefaultCornerLabelAttr());
                defAttr.UpdateWith(attr);
                // force good vals after update
                defAttr.SetDefaultAttr(wxNullSheetCellAttr);
                defAttr.SetKind(wxSHEET_AttrDefault);
                return;
            }
            default : 
            {
                wxFAIL_MSG(wxT("Invalid attr type for corner label"));
                return;
            }
        }
    }
    else if (IsRowLabelCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault :
            {    
                wxSheetCellAttr defAttr(DoGetDefaultRowLabelAttr());
                defAttr.UpdateWith(attr); 
                // force good vals after update
                defAttr.SetDefaultAttr(wxNullSheetCellAttr);
                defAttr.SetKind(wxSHEET_AttrDefault);
                return;
            }
            case wxSHEET_AttrCell :
            {
                wxCHECK_RET(ContainsRowLabelCell(coords), wxT("Invalid attr coords"));
                if ( CanHaveAttributes() && GetTable() )
                {
                    if (attr.Ok())
                        InitAttr(attr, DoGetDefaultRowLabelAttr());
        
                    GetTable()->SetAttr(coords, attr, type);
                }
                return;
            }
            default : 
            {
                wxFAIL_MSG(wxT("Invalid attr type for row label"));
                return;
            }
        }
    }
    else if (IsColLabelCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault :
            {    
                wxSheetCellAttr defAttr(DoGetDefaultRowLabelAttr());
                defAttr.UpdateWith(attr); 
                // force good vals after update
                defAttr.SetDefaultAttr(wxNullSheetCellAttr);
                defAttr.SetKind(wxSHEET_AttrDefault);
                return;
            }
            case wxSHEET_AttrCell :
            {
                wxCHECK_RET(ContainsColLabelCell(coords), wxT("Invalid attr coords"));
                if ( CanHaveAttributes() && GetTable() )
                {
                    if (attr.Ok())
                        InitAttr(attr, DoGetDefaultColLabelAttr());
        
                    GetTable()->SetAttr(coords, attr, type);
                }
                return;
            }
            default : 
            {
                wxFAIL_MSG(wxT("Invalid attr type for col label"));
                return;
            }
        }
    }
    
    wxFAIL_MSG(wxString::Format(wxT("Unable to set attribute for cell(%d,%d)"), coords.m_row, coords.m_col));
}

bool wxSheet::InitAttr( wxSheetCellAttr& initAttr, const wxSheetCellAttr& defAttr ) const
{
    if (initAttr == defAttr)
        return FALSE;
    
    wxSheetCellAttr attr(initAttr);
    wxSheetCellAttr attrDef(attr.GetDefaultAttr());
    // only 100000 def attr, should be enough?
    for (int n=0; n<100000; n++)
    {
        if (!attrDef.Ok())
        {
            attr.SetDefaultAttr(defAttr);
            return TRUE;
        }
        else if (attrDef == defAttr)
        {
            return FALSE; // already set
        }

        attr = attrDef;
        attrDef = attr.GetDefaultAttr();
    }
    
    wxFAIL_MSG(wxT("Unable to set default attribute for cell"));
    return FALSE;
}

const wxColour& wxSheet::GetAttrBackgroundColour( const wxSheetCoords& coords, wxSheetAttr_Type type ) const
{
    return GetAttr(coords, type).GetBackgroundColour();
}
const wxColour& wxSheet::GetAttrForegoundColour( const wxSheetCoords& coords, wxSheetAttr_Type type ) const
{
    return GetAttr(coords, type).GetForegroundColour();
}
const wxFont& wxSheet::GetAttrFont( const wxSheetCoords& coords, wxSheetAttr_Type type ) const
{
    return GetAttr(coords, type).GetFont();
}
int wxSheet::GetAttrAlignment( const wxSheetCoords& coords, wxSheetAttr_Type type ) const
{
    return GetAttr(coords, type).GetAlignment();
}
int wxSheet::GetAttrOrientation( const wxSheetCoords& coords, wxSheetAttr_Type type ) const
{
    return GetAttr(coords, type).GetOrientation();
}
int wxSheet::GetAttrLevel( const wxSheetCoords& coords, wxSheetAttr_Type type ) const
{
    return GetAttr(coords, type).GetLevel();
}
bool wxSheet::GetAttrOverflow( const wxSheetCoords& coords, wxSheetAttr_Type type ) const
{
    return GetAttr(coords, type).GetOverflow();
}
bool wxSheet::GetAttrOverflowMarker( const wxSheetCoords& coords, wxSheetAttr_Type type ) const
{
    return GetAttr(coords, type).GetOverflowMarker();
}
bool wxSheet::GetAttrReadOnly( const wxSheetCoords& coords, wxSheetAttr_Type type ) const
{
    return GetAttr(coords, type).GetReadOnly();
}
wxSheetCellRenderer wxSheet::GetAttrRenderer( const wxSheetCoords& coords, wxSheetAttr_Type type ) const
{
    return GetAttr(coords, type).GetRenderer((wxSheet*)this, coords);
}
wxSheetCellEditor wxSheet::GetAttrEditor( const wxSheetCoords& coords, wxSheetAttr_Type type ) const
{
    return GetAttr(coords, type).GetEditor((wxSheet*)this, coords);
}

void wxSheet::SetAttrBackgroundColour( const wxSheetCoords& coords, const wxColour& colour, wxSheetAttr_Type type )
{
    GetOrCreateAttr(coords, type).SetBackgroundColour(colour);
    RefreshAttrChange(coords, type);
}
void wxSheet::SetAttrForegoundColour( const wxSheetCoords& coords, const wxColour& colour, wxSheetAttr_Type type )
{
    GetOrCreateAttr(coords, type).SetForegroundColour(colour);
    RefreshAttrChange(coords, type);
}
void wxSheet::SetAttrFont( const wxSheetCoords& coords, const wxFont& font, wxSheetAttr_Type type )
{
    GetOrCreateAttr(coords, type).SetFont(font);
    RefreshAttrChange(coords, type);
}
void wxSheet::SetAttrAlignment( const wxSheetCoords& coords, int align, wxSheetAttr_Type type )
{
    GetOrCreateAttr(coords, type).SetAlignment(align);
    RefreshAttrChange(coords, type);
}
void wxSheet::SetAttrOrientation( const wxSheetCoords& coords, int orient, wxSheetAttr_Type type )
{
    GetOrCreateAttr(coords, type).SetOrientation(orient);
    RefreshAttrChange(coords, type);
}
void wxSheet::SetAttrLevel( const wxSheetCoords& coords, int level, wxSheetAttr_Type type )
{
    wxCHECK_RET(type != wxSHEET_AttrDefault, wxT("Unable to change level of default attribute"));
    GetOrCreateAttr(coords, type).SetLevel(wxSheetAttrLevel_Type(level));
    RefreshAttrChange(coords, type);
}
void wxSheet::SetAttrOverflow( const wxSheetCoords& coords, bool allow, wxSheetAttr_Type type )
{
    GetOrCreateAttr(coords, type).SetOverflow(allow);
    RefreshAttrChange(coords, type);
}
void wxSheet::SetAttrOverflowMarker( const wxSheetCoords& coords, bool draw_marker, wxSheetAttr_Type type )
{
    GetOrCreateAttr(coords, type).SetOverflowMarker(draw_marker);
    RefreshAttrChange(coords, type);
}
void wxSheet::SetAttrReadOnly( const wxSheetCoords& coords, bool isReadOnly, wxSheetAttr_Type type )
{
    GetOrCreateAttr(coords, type).SetReadOnly(isReadOnly);
    RefreshAttrChange(coords, type);
}
void wxSheet::SetAttrRenderer( const wxSheetCoords& coords, const wxSheetCellRenderer& renderer, wxSheetAttr_Type type )
{
    GetOrCreateAttr(coords, type).SetRenderer(renderer);
    RefreshAttrChange(coords, type);
}
void wxSheet::SetAttrEditor( const wxSheetCoords& coords, const wxSheetCellEditor& editor, wxSheetAttr_Type type )
{
    GetOrCreateAttr(coords, type).SetEditor(editor);
    RefreshAttrChange(coords, type);
}

// ----------------------------------------------------------------------------
// Get/Set the attributes for individual sheet cells/rows/cols

bool wxSheet::CanHaveAttributes() const
{
    return (GetTable() != NULL) && GetTable()->CanHaveAttributes();
}

void wxSheet::SetColFormatFloat(int col, int width, int precision)
{
    wxString typeName = wxSHEET_VALUE_FLOAT;
    if ( (width != -1) || (precision != -1) )
        typeName << _T(':') << width << _T(',') << precision;

    SetColFormatCustom(col, typeName);
}
void wxSheet::SetColFormatCustom(int col, const wxString& typeName)
{
    wxCHECK_RET(ContainsGridCol(col), wxT("Invalid col in wxSheet::SetColFormatCustom"));
    wxSheetCellRenderer ren(GetDefaultRendererForType(typeName));
    wxCHECK_RET(ren.Ok(), wxT("Invalid renderer in wxSheet::SetColFormatCustom"));
    SetAttrRenderer(wxSheetCoords(0,col), ren.Clone(), wxSHEET_AttrCol);
}

// ----------------------------------------------------------------------------
// Get/Set cell value

wxString wxSheet::GetCellValue( const wxSheetCoords& coords )
{
    if ( GetTable() )
        return GetTable()->GetValue( coords );
    
    if (IsRowLabelCell(coords))
        return wxString::Format(wxT("%d"), coords.m_row+1);
    if (IsColLabelCell(coords))
        wxString::Format(wxT("%d"), coords.m_col+1); // table base provides A-Z
    if (IsCornerLabelCell(coords))
        return GetSheetRefData()->m_cornerLabelValue; 
    
    return wxEmptyString;
}

void wxSheet::SetCellValue( const wxSheetCoords& coords, const wxString& s )
{
    if ( !GetTable() )
        return;
    
    GetTable()->SetValue( coords, s );

    // Note: If we are using IsCellEditControlEnabled,
    // this interacts badly with calling SetCellValue from
    // an EVT_SHEET_CELL_CHANGE handler.
    if ( (GetEditControlCoords() == coords) && IsCellEditControlShown())
    {
        HideCellEditControl();
        ShowCellEditControl(); // will reread data from table
    }   
    else
        RefreshCell(coords, FALSE);
}

bool wxSheet::IsCellEmpty( const wxSheetCoords& coords )
{
    if ( GetTable() )
        return GetTable()->IsEmptyCell( coords );
    
    // this tries the table too, but maybe they've overridden GetCellValue
    return GetCellValue(coords).IsEmpty();
}

// ----------------------------------------------------------------------------
// Register mapping between data types to Renderers/Editors

void wxSheet::RegisterDataType(const wxString& typeName,
                               const wxSheetCellRenderer& renderer,
                               const wxSheetCellEditor& editor)
{
    GetSheetRefData()->m_typeRegistry->RegisterDataType(typeName, renderer, editor);
}

wxSheetCellEditor wxSheet::GetDefaultEditorForType(const wxString& typeName) const
{
    int index = GetSheetRefData()->m_typeRegistry->FindOrCloneDataType(typeName);
    wxCHECK_MSG(index != wxNOT_FOUND, wxNullSheetCellEditor, wxT("Unknown data type name"));

    return GetSheetRefData()->m_typeRegistry->GetEditor(index);
}
wxSheetCellRenderer wxSheet::GetDefaultRendererForType(const wxString& typeName) const
{
    int index = GetSheetRefData()->m_typeRegistry->FindOrCloneDataType(typeName);
    wxCHECK_MSG(index != wxNOT_FOUND, wxNullSheetCellRenderer, wxT("Unknown data type name"));

    return GetSheetRefData()->m_typeRegistry->GetRenderer(index);
}

// FIXME GetDefaultEditorForCell - what is the point?
wxSheetCellEditor wxSheet::GetDefaultEditorForCell(const wxSheetCoords& coords) const
{
    wxCHECK_MSG(GetTable(), wxNullSheetCellEditor, wxT("Grid table is not created"));
    //return GetDefaultEditorForType(GetTable()->GetTypeName(coords));
    
    // Default is to NOT use this as a default, but the defAttr's editor
    int index = GetSheetRefData()->m_typeRegistry->FindOrCloneDataType(GetTable()->GetTypeName(coords));
    if (index == wxNOT_FOUND)
        return wxNullSheetCellEditor;

    return GetSheetRefData()->m_typeRegistry->GetEditor(index);
    
}
wxSheetCellRenderer wxSheet::GetDefaultRendererForCell(const wxSheetCoords& coords) const
{
    if (!GetTable()) return wxNullSheetCellRenderer; 
    //return GetDefaultRendererForType(GetTable()->GetTypeName(coords));

    // Default is to NOT use this as a default, but the defAttr's renderer
    int index = GetSheetRefData()->m_typeRegistry->FindOrCloneDataType(GetTable()->GetTypeName(coords));
    if (index == wxNOT_FOUND)
        return wxNullSheetCellRenderer;

    return GetSheetRefData()->m_typeRegistry->GetRenderer(index);
}

// ----------------------------------------------------------------------------
// Cursor movement and visibility functions

bool wxSheet::IsCellVisible( const wxSheetCoords& coords, bool wholeCellVisible ) const
{
    wxCHECK_MSG(ContainsCell(coords), FALSE, wxT("Invalid coords in wxSheet::IsCellVisible"));
    
    const wxRect clientRect(wxPoint(0,0), GetWindowForCoords(coords)->GetClientSize());
    const wxRect devRect(CellToRect(coords, TRUE));  // rect in device coords
    
    if ( wholeCellVisible )                     // is the whole cell visible?
    {
        wxRect intersectRect( devRect.Intersect(clientRect) );
        return intersectRect == devRect;
    }
    
    return devRect.Intersects(clientRect); // is the cell partly visible ?
}
bool wxSheet::IsRowVisible( int row, bool wholeRowVisible ) const
{
    if (row == -1)
        return m_colLabelWin && m_colLabelWin->IsShown();
    
    wxCHECK_MSG(ContainsGridRow(row), FALSE, wxT("Invalid row in wxSheet::IsRowVisible"));
    
    const wxRect clientRect(wxPoint(0,0), m_gridWin->GetClientSize());
    const wxRect logRect(0, GetRowTop(row), clientRect.width, GetRowHeight(row));
    const wxRect devRect(clientRect.x, CalcScrolledRect(logRect).y, logRect.width, logRect.height);
    
    if (wholeRowVisible)                  // is the whole row visible?
    {        
        wxRect intersectRect( devRect.Intersect(clientRect) );
        return intersectRect == devRect;
    }    
    
    return devRect.Intersects(clientRect); // is the cell partly visible ?
}
bool wxSheet::IsColVisible( int col, bool wholeColVisible ) const
{
    if (col == -1)
        return m_rowLabelWin && m_rowLabelWin->IsShown();
    
    wxCHECK_MSG(ContainsGridCol(col), FALSE, wxT("Invalid row in wxSheet::IsColVisible"));
    
    const wxRect clientRect(wxPoint(0,0), m_gridWin->GetClientSize());
    const wxRect logRect(GetColLeft(col), 0, GetColWidth(col), clientRect.height);
    const wxRect devRect(CalcScrolledRect(logRect).x, clientRect.y, logRect.width, logRect.height);
    
    if (wholeColVisible)                  // is the whole row visible?
    {        
        wxRect intersectRect( devRect.Intersect(clientRect) );
        return intersectRect == devRect;
    }    
    
    return devRect.Intersects(clientRect); // is the cell partly visible ?
}

void wxSheet::MakeCellVisible( const wxSheetCoords& coords )
{
    wxCHECK_RET(ContainsCell(coords), wxT("Invalid coords in wxSheet::MakeCellVisible"));
    
    if (IsCornerLabelCell(coords))
        return;
    
    int xpos = -1, ypos = -1;
    
    wxRect logRect( CellToRect(coords) );        // cell rect in logical coords
    wxRect devRect( CalcScrolledRect(logRect) ); // rect in device coords

    int x0, y0, cw, ch;
    m_gridWin->GetClientSize( &cw, &ch );
    CalcUnscrolledPosition(0, 0, &x0, &y0);

    if (!IsColLabelCell(coords))
    {
        if ( devRect.GetTop() < 0 )
        {
            ypos = logRect.GetTop();
            ypos -= SHEET_SCROLL_LINE_Y;  // add a little padding
            if (ypos < 0) ypos = 0;
        }
        else if ( devRect.GetBottom() > ch )
        {
            ypos = y0 + (devRect.GetBottom() - ch);
            ypos += SHEET_SCROLL_LINE_Y;
            if (ypos < 0) ypos = 0;
        }
    }

    if (!IsRowLabelCell(coords))
    {
        if ( devRect.GetLeft() < 0 )
        {
            xpos = logRect.GetLeft();
            xpos -= SHEET_SCROLL_LINE_X;
            if (xpos < 0) xpos = 0;
        }
        else if ( devRect.GetRight() > cw )
        {
            xpos = x0 + (devRect.GetRight() - cw);
            xpos += SHEET_SCROLL_LINE_X;
            if (xpos < 0) xpos = 0;
        }
        
    }
    
    // -1 means leave that postion the same, see if (x/ypos < 0) x/ypos = 0 above
    if ((xpos != -1) || (ypos != -1))
        SetGridOrigin( xpos, ypos, FALSE, TRUE );
}

void wxSheet::SetGridCursorCell( const wxSheetCoords& coords )
{   
    // Note: can set to invalid coords < 0 to hide cursor, but not out of range
    wxCHECK_RET((coords.GetRow() < GetSheetRefData()->m_numRows) && 
                (coords.GetCol() < GetSheetRefData()->m_numCols),
                wxT("Invalid coords in wxSheet::SetGridCursorCell"));
    
    if (GetGridCursorCell() == coords)
        return;
    
    m_waitForSlowClick = FALSE;
    
    // the event has been intercepted - do nothing
    if (SendEvent(wxEVT_SHEET_SELECTING_CELL, coords) != EVT_SKIPPED)
        return;

/*    
    wxClientDC dc(m_gridWin);
    PrepareGridDC(dc);
    
    if ( ContainsGridCell(GetGridCursorCell()) )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);

        if ( IsCellVisible(GetGridCursorCell(), FALSE) )
        {
            // only want cell owner in the block
            wxSheetBlock block(GetCellOwner(GetGridCursorCell()), 1, 1);

            GetSheetRefData()->m_cursorCoords = coords; // Otherwise refresh redraws the highlight!
            DrawGridCells(dc, wxSheetSelection(block));
            
            wxRect r = BlockToDeviceRect(block);
            if ( !GridLinesEnabled() )
                r.Inflate(1);
            
            DrawAllGridLines( dc, r );
        }
    }

    GetSheetRefData()->m_cursorCoords = coords;
    DrawCursorCellHighlight(dc, GetAttr(coords));
*/

    wxSheetCoords oldCursorCell = GetGridCursorCell();
    if ( ContainsGridCell(oldCursorCell) )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);
        
        GetSheetRefData()->m_cursorCoords = coords; // Otherwise refresh redraws the highlight!
        RefreshCell(oldCursorCell, TRUE);
    }    

    GetSheetRefData()->m_cursorCoords = coords;
    RefreshCell(GetGridCursorCell(), TRUE);

    SendEvent(wxEVT_SHEET_SELECTED_CELL, coords);
}

bool wxSheet::DoMoveCursor( const wxSheetCoords& relCoords, bool expandSelection )
{
    wxSheetCoords coords( GetGridCursorCell() + relCoords );
    if ( (relCoords == wxSheetCoords(0,0)) || 
         !ContainsGridCell(GetGridCursorCell()) || !ContainsGridCell(coords) )
        return FALSE;

    if ( expandSelection)
    {
        m_keySelecting = TRUE;
        if ( !ContainsGridCell(GetSelectingAnchor()) )
            SetSelectingAnchor(GetGridCursorCell());
    }
    else
        ClearSelection(TRUE);
    
    MakeCellVisible( coords );
    SetGridCursorCell( coords );
    if (expandSelection)
        HighlightSelectingBlock( GetSelectingAnchor(), GetGridCursorCell() );
        
    return TRUE;
}

bool wxSheet::DoMoveCursorBlock( const wxSheetCoords& relDir, bool expandSelection )
{
    wxSheetCoords coords(GetGridCursorCell() + relDir);
    if ( !ContainsGridCell(GetGridCursorCell()) || !ContainsGridCell(coords) )
        return FALSE;

    if ( IsCellEmpty(GetGridCursorCell()) )
    {
        // starting in an empty cell: find the next block of non-empty cells
        while ( ContainsGridCell(coords) )
        {
            if (!IsCellEmpty(coords) || !ContainsGridCell(coords + relDir))
                break;
                
            coords += relDir;
        }
    }
    else if ( IsCellEmpty(coords) )
    {
        // starting at the edge of a block: find the next block
        while ( ContainsGridCell(coords + relDir) )
        {
            coords += relDir;
            if ( !IsCellEmpty(coords) )
                break;
        }
    }
    else
    {
        // starting within a block with value: find the edge of the block
        while ( ContainsGridCell(coords + relDir) )
        {
            if ( IsCellEmpty(coords + relDir) )
                break;
            
            coords += relDir;
        }
    }

    return DoMoveCursor(coords-GetGridCursorCell(), expandSelection);
}

bool wxSheet::DoMoveCursorUpDownPage( bool page_up, bool expandSelection )
{
    if (!ContainsGridCell(GetGridCursorCell())) 
        return FALSE;

    int row = GetGridCursorRow();
    
    if ((page_up && (row < 1)) || (!page_up && (row >= GetNumberRows() - 1)))
        return FALSE;
    
    int cw, ch;
    m_gridWin->GetClientSize( &cw, &ch );

    int y = GetRowTop(row);
    y += page_up ? (-ch + 1) : ch;
    int newRow = YToGridRow( y, TRUE );

    wxSheetCoords relCoords(newRow - GetGridCursorRow(), 0);
    return DoMoveCursor(relCoords, expandSelection);
}

// ----------------------------------------------------------------------------
// Cell/Row/Col selection and deselection

bool wxSheet::HasSelection(bool selecting) const
{
    if (selecting)
        return (!GetSelectingBlock().IsEmpty() || 
                (GetSelection() && GetSelection()->HasSelection()));

    return GetSelection() && GetSelection()->HasSelection();
}
bool wxSheet::IsCellSelected( const wxSheetCoords& coords ) const
{
    return ( GetSelectingBlock().Contains(coords) || 
            (GetSelection() && GetSelection()->Contains(coords)) );
}
bool wxSheet::IsRowSelected( int row ) const
{
    wxSheetBlock rowBlock(row, 0, 1, GetNumberCols()); // not +1 
    return ( GetSelectingBlock().Contains(rowBlock) || 
            (GetSelection() && GetSelection()->Contains(rowBlock)) );
}
bool wxSheet::IsColSelected( int col ) const
{
    wxSheetBlock colBlock(0, col, GetNumberRows(), 1); // not +1 
    return ( GetSelectingBlock().Contains(colBlock) || 
            (GetSelection() && GetSelection()->Contains(colBlock)) );
}

void wxSheet::SetSelectionMode(wxSheetSelectionMode_Type selmode)
{
    wxCHECK_RET( GetSelection(),
                 wxT("Called wxSheet::SetSelectionMode() before calling CreateGrid()") );

    // yes we could patch up the selection, but does it really make sense?
    // like inserting/deleting rows/cols you'd have to send a slew of confusing
    // (de)selection events
    ClearSelection(TRUE);
    GetSheetRefData()->m_selectionMode = selmode;
}

bool wxSheet::SelectRow( int row, bool addToSelected, bool sendEvt )
{
    if (GetSelectionMode() == wxSHEET_SelectCols)
        return FALSE;
    
    // selection to # of cols means whole row is selected
    return SelectBlock( wxSheetBlock(row, 0, 1, GetNumberCols()+1), 
                        addToSelected, sendEvt );
}
bool wxSheet::SelectRows( int rowTop, int rowBottom, bool addToSelected, bool sendEvt )
{
    if ((GetSelectionMode() == wxSHEET_SelectCols) || (rowTop > rowBottom))
        return FALSE;
    
    // selection to # of cols means whole row is selected
    return SelectBlock( wxSheetBlock(rowTop, 0, rowBottom-rowTop+1, GetNumberCols()+1), 
                        addToSelected, sendEvt );
}
bool wxSheet::SelectCol( int col, bool addToSelected, bool sendEvt )
{
    if (GetSelectionMode() == wxSHEET_SelectRows)
        return FALSE;
    
    // selection to # of rows means whole col is selected
    return SelectBlock( wxSheetBlock(0, col, GetNumberRows()+1, 1), 
                        addToSelected, sendEvt );
}
bool wxSheet::SelectCols( int colLeft, int colRight, bool addToSelected, bool sendEvt )
{
    if ((GetSelectionMode() == wxSHEET_SelectRows) || (colLeft > colRight))
        return FALSE;
    
    // selection to # of rows means whole col is selected
    return SelectBlock( wxSheetBlock(0, colLeft, GetNumberRows()+1, colRight-colLeft+1), 
                        addToSelected, sendEvt );
}
bool wxSheet::SelectCell( const wxSheetCoords& coords, bool addToSelected, bool sendEvt )
{ 
    return SelectBlock(wxSheetBlock(coords, 1, 1), addToSelected, sendEvt); 
}
bool wxSheet::SelectBlock( const wxSheetBlock& block, bool addToSelected, bool sendEvt )
{
    if (!GetSelection())
        return FALSE;
    
    if ( !addToSelected )
        ClearSelection(sendEvt);

    wxArraySheetBlock addedBlocks;
    if (!GetSelection()->SelectBlock( block, TRUE, &addedBlocks ))
        return FALSE;

    if (!GetBatchCount())
    {
        wxSheetBlock bounds;
        
        for (size_t n=0; n<addedBlocks.GetCount(); n++)
            bounds = bounds.ExpandUnion(addedBlocks[n]);
        
        RefreshGridCellBlock(bounds);
    }
    
    if (sendEvt)
        SendRangeEvent(wxEVT_SHEET_RANGE_SELECTED, block, TRUE, addToSelected);
    
    return TRUE;
}

bool wxSheet::SelectAll(bool sendEvt)
{
    BeginBatch();
    ClearSelection(FALSE); // clear old and only have new, no event/refresh
    EndBatch(FALSE);
    // select to # rows/col means everything everywhere is selected
    return SelectBlock(wxSheetBlock(0, 0, GetNumberRows(), GetNumberCols()),
                       FALSE, sendEvt);
}

bool wxSheet::DeselectRow( int row, bool sendEvt )
{
    if (GetSelectionMode() == wxSHEET_SelectCols)
        return FALSE;
    
    // deselection to # of cols means whole row is deselected
    return DeselectBlock( wxSheetBlock(row, 0, 1, GetNumberCols()), sendEvt );
}
bool wxSheet::DeselectRows( int rowTop, int rowBottom, bool sendEvt )
{
    if ((GetSelectionMode() == wxSHEET_SelectCols) || (rowTop > rowBottom))
        return FALSE;
    
    // selection to # of cols means whole row is deselected
    return DeselectBlock( wxSheetBlock(rowTop, 0, rowBottom-rowTop+1, GetNumberCols()+1), 
                          sendEvt );
}
bool wxSheet::DeselectCol( int col, bool sendEvt )
{
    if (GetSelectionMode() == wxSHEET_SelectRows)
        return FALSE;
    
    // deselection to # of rows means whole col is deselected
    return DeselectBlock( wxSheetBlock(0, col, GetNumberRows(), 1), sendEvt );
}
bool wxSheet::DeselectCols( int colLeft, int colRight, bool sendEvt )
{
    if ((GetSelectionMode() == wxSHEET_SelectRows) || (colLeft > colRight))
        return FALSE;
    
    // selection to # of rows means whole col is deselected
    return DeselectBlock( wxSheetBlock(0, colLeft, GetNumberRows()+1, colRight-colLeft+1), 
                          sendEvt );
}
bool wxSheet::DeselectCell( const wxSheetCoords& coords, bool sendEvt )
{
    return DeselectBlock(wxSheetBlock(coords, 1, 1), sendEvt);
}

bool wxSheet::DeselectBlock( const wxSheetBlock& block, bool sendEvt )
{
    if (!GetSelection())
        return FALSE;
    
    // do they want to clear the whole grid
    wxSheetBlock gridBlock(0, 0, GetNumberRows(), GetNumberCols());
    if (gridBlock.Intersect(block) == gridBlock)
        return ClearSelection(sendEvt);
    
    wxArraySheetBlock deletedBlocks;
    if (!GetSelection()->DeselectBlock( block, TRUE, &deletedBlocks ))
        return FALSE;

    if (!GetBatchCount())
    {
        wxSheetBlock bounds;
        
        for (size_t n=0; n<deletedBlocks.GetCount(); n++)
            bounds = bounds.ExpandUnion(deletedBlocks[n]);
        
        RefreshGridCellBlock(bounds);
    }
    
    if (sendEvt)
        SendRangeEvent(wxEVT_SHEET_RANGE_SELECTED, block, FALSE, FALSE);
    
    return TRUE;
}

bool wxSheet::ClearSelection(bool send_event)
{
    if ( !HasSelection() )
        return FALSE;

    // no evt for clearing m_selectingBlock, it's not a real selection
    if ( !HasSelection(FALSE) )
        send_event = FALSE;
    
    wxSheetBlock bounds(GetSelectingBlock().ExpandUnion(GetSelection()->GetBoundingBlock()));
    SetSelectingBlock(wxNullSheetBlock);
    GetSelection()->Clear(); 
    
    RefreshGridCellBlock(bounds);
    
    if (send_event)
    {
        // One deselection event, indicating deselection of _all_ cells.
        SendRangeEvent( wxEVT_SHEET_RANGE_SELECTED, 
                        wxSheetBlock(0, 0, GetNumberRows(), GetNumberCols()),
                        FALSE, FALSE );
    }
    
    return TRUE;
}

bool wxSheet::ToggleCellSelection( const wxSheetCoords& coords, bool addToSelected, bool sendEvt )
{
    int sel = IsCellSelected(coords);
    if (!sel)
        return SelectCell(coords, addToSelected, sendEvt);

    return DeselectCell(coords, sendEvt );
}
bool wxSheet::ToggleRowSelection( int row, bool addToSelected, bool sendEvt )
{
    int sel = IsRowSelected(row);
    if (!sel)
        return SelectRow(row, addToSelected, sendEvt);

    return DeselectRow(row, sendEvt );
}
bool wxSheet::ToggleColSelection( int col, bool addToSelected, bool sendEvt )
{
    int sel = IsColSelected(col);
    if (!sel)
        return SelectCol(col, addToSelected, sendEvt);

    return DeselectCol(col, sendEvt );
}

// ----------------------------------------------------------------------------
// Copy/Paste

#include "wx/dataobj.h"

//#define wxDF_wxSHEET (wxDF_MAX+1001)  // works w/ GTK 1.2 non unicode
const wxChar* wxDF_wxSHEET = wxT("wxDF_wxSHEET");

class WXDLLEXPORT wxSheetDataObject : public wxDataObjectSimple
{
public:
    wxSheetDataObject() : wxDataObjectSimple(wxDF_wxSHEET) {}
    wxSheetDataObject(const wxPairArrayIntPairArraySheetString &values)
        : wxDataObjectSimple(wxDF_wxSHEET), m_values(values)
        {
            CreateDataString(); 
            m_values.Clear();
        }
        
    virtual size_t GetDataSize() const { return m_data.Len()+1; }
    virtual bool GetDataHere(void *buf) const;
    virtual bool SetData(size_t len, const void *buf);
    
    const wxPairArrayIntPairArraySheetString& GetValues() const { return m_values; }
 
    void CreateDataString();
    
protected:
    wxPairArrayIntPairArraySheetString m_values;    
    wxString m_data;
private:
    // I don't understand these? from include/wx/gtk/dataobj2.h
    // virtual function hiding supression
    size_t GetDataSize(const wxDataFormat& format) const
        { return(wxDataObjectSimple::GetDataSize(format)); }
    bool GetDataHere(const wxDataFormat& format, void* pBuf) const
        { return(wxDataObjectSimple::GetDataHere(format, pBuf)); }
    bool SetData(const wxDataFormat& format, size_t nLen, const void* pBuf)
        { return(wxDataObjectSimple::SetData(format, nLen, pBuf)); }
};

// FIXME! this is just a hack and kinda works
const wxString s_nextRow(wxT("\1\1"), 2); // just increment row number
const wxString s_nextCol(wxT("\1\2"), 2); // end of value marker
const wxString s_newRow(wxT("\1\3"), 2);  // specify a row number
const wxString s_newCol(wxT("\1\4"), 2);  // specify a col number

// Data string looks like this, start by specifying the row and col number
// then data and end data marker (s_nextCol) more data, if you skip cols
// then specify the col number with s_newCol.
// [s_newRow<row>:][s_newCol<col>:]hello[s_nextCol]text[s_nextCol][s_newCol<colNum>:]More[s_nextCol]

void wxSheetDataObject::CreateDataString()
{
    m_data.Clear();    
    m_values.RemoveEmptyRows();
    if (m_values.GetCount() == 0u)
        return;
    
    int c, ncols, r, nrows = m_values.GetCount();
    int row, col;
    int last_row = m_values.GetItemKey(0) - 10, last_col;
    
    for (r=0; r<nrows; r++)
    {
        row = m_values.GetItemKey(r);
        if (last_row+1 < row)
            m_data += s_newRow + wxString::Format(wxT("%d:"), row);
        else
            m_data += s_nextRow;
        
        col = m_values.GetItemValue(r).GetItemKey(0);
        m_data += s_newCol + wxString::Format(wxT("%d:"), col);
        last_col = col;
        
        ncols = m_values.GetItemValue(r).GetCount();
        for (c=0; c<ncols; c++)
        {
            col = m_values.GetItemValue(r).GetItemKey(c);
            if (last_col+1 < col)
                m_data += s_newCol + wxString::Format(wxT("%d:"), col);

            m_data += m_values.GetItemValue(r).GetItemValue(c) + s_nextCol;
            last_col = col;
        }
    }
}

bool wxSheetDataObject::GetDataHere(void *buf) const 
{
    memcpy( buf, m_data.mbc_str(), m_data.Len() + 1 );
    return TRUE;
}    

bool wxSheetDataObject::SetData(size_t len, const void *buf)
{
    m_values.Clear();
    m_data.Clear();
    
    if (len < 2u)
        return FALSE; // I guess?
    
    wxString strBuf(wxConvertMB2WX((const char *)buf), len); // probably not Unicode safe
    m_data = strBuf;
    
    //wxPrintf(wxT("Data len %d %d\n"), m_data.Len(), len);
    
    const wxChar *c = strBuf.GetData();
    size_t n, last_n = 0, str_len = strBuf.Len();
    int row=0, col=0;
    long long_val = 0;
    
    for ( n=0; n < str_len-1; n++, c++ )
    {
        // The next row, increment row counter
        if ( (*c == s_nextRow[0]) && (*(c+1) == s_nextRow[1]) ) // s_nextRow
        {
            //wxPrintf(wxT("Next row\n"));
            c++; n++; last_n = n+1;
            row++;
        }
        // a new row, read new row number
        else if ( (*c == s_newRow[0]) && (*(c+1) == s_newRow[1]) ) // s_newRow
        {
            c++; n++; last_n = n+1;
            for ( ; n < str_len-1; n++, c++ ) { if (*c == wxT(':')) break; }
            
            wxString strVal(strBuf.Mid(last_n, n-last_n));
            //wxPrintf(wxT("Getting rownum %d %d '%s'\n"), last_n, n, strVal.c_str());
            if ( strVal.ToLong(&long_val) )
                row = int(long_val);
            else
                return FALSE;
            
            last_n = n+1;
        }
        // read the next col value 
        else if ( (*c == s_nextCol[0]) && (*(c+1) == s_nextCol[1]) ) // s_nextCol
        {
            wxString strVal(strBuf.Mid(last_n, n-last_n));
            m_values.GetOrCreateValue(row).SetValue(col, strVal);
            //wxPrintf(wxT("Next col val %d '%s'\n"), strVal.Len(), strVal.c_str());
            c++; n++; last_n = n+1;
            col++;
        }
        // a some skipped cols, read new col number and value
        else if ( (*c == s_newCol[0]) && (*(c+1) == s_newCol[1]) ) // s_newCol
        {
            c++; n++; last_n = n+1;
            for ( ; n < str_len-1; n++, c++ ) { if (*c == wxT(':')) break; }

            wxString strVal(strBuf.Mid(last_n, n-last_n));
            //wxPrintf(wxT("Getting colnum %d %d '%s'\n"), last_n, n, strVal.c_str());
            if ( strVal.ToLong(&long_val) )
                col = int(long_val);
            else
                return FALSE;
            
            last_n = n+1;
        }
    }
    
    //wxPrintf(wxT("Got some data %d %d\n"), len, m_values.GetCount());
    return TRUE;
}

bool wxSheet::CopyInternalSelectionToClipboard(const wxChar& colSep)
{
    if (!wxTheClipboard->Open()) 
        return FALSE;
    
    // save data to clipboard, we want the internal data back if possible
    wxDataObjectComposite *data = new wxDataObjectComposite;
    data->Add(new wxSheetDataObject(GetSheetRefData()->m_copiedData), TRUE);
    data->Add(new wxTextDataObject(CopyInternalSelectionToString(colSep)), FALSE);
    wxTheClipboard->SetData( data );
    
    wxTheClipboard->Close();
    return TRUE;
}

bool wxSheet::CopyCurrentSelectionToClipboard(bool copy_cursor, const wxChar& colSep)
{
    CopyCurrentSelectionInternal(copy_cursor);
    return CopyInternalSelectionToClipboard(colSep);
}

bool wxSheet::PasteFromClipboard( const wxSheetCoords &topLeft,
                                  const wxChar& colSep )
{    
    if ( !wxTheClipboard->Open() )
        return FALSE;

    bool ret = FALSE;
    
    wxSheetDataObject sheetData;
    if (wxTheClipboard->IsSupported(wxDF_wxSHEET) && wxTheClipboard->GetData(sheetData))
    {
        GetSheetRefData()->m_copiedData = sheetData.GetValues();
        ret = PasteInternalCopiedSelection( topLeft );
    }
    else if (wxTheClipboard->IsSupported(wxDF_TEXT))
    {
        wxTextDataObject textData;
        if (wxTheClipboard->GetData(textData))
        {
            CopyStringToSelection(textData.GetText(), colSep);
            ret = PasteInternalCopiedSelection( topLeft );
        }
    }

    wxTheClipboard->Close();
    return ret;
}

bool wxSheet::CopyCurrentSelectionInternal(bool copy_cursor)
{
    wxPairArrayIntPairArraySheetString &copiedData = GetSheetRefData()->m_copiedData;
    copiedData.Clear();
    
    wxSheetSelection sel = *GetSelection();
    if (!GetSelectingBlock().IsEmpty())
        sel.SelectBlock(GetSelectingBlock());

    sel.SetBoundingBlock(wxSheetBlock(0, 0, GetNumberRows(), GetNumberCols()));
    
    if (!sel.HasSelection())
    {
        if (copy_cursor && ContainsGridCell(GetGridCursorCell()))
            sel.SelectBlock(wxSheetBlock(GetGridCursorCell(), 1, 1));
        else
            return FALSE;
    }
    
    wxSheetSelectionIterator selIter(sel);
    wxSheetCoords cell;
    while (selIter.GetNext(cell))
        copiedData.GetOrCreateValue(cell.m_row).SetValue(cell.m_col, GetCellValue(cell));
    
    return copiedData.GetCount() > 0;
}

wxString wxSheet::CopyInternalSelectionToString(const wxChar& colSep)
{
    wxPairArrayIntPairArraySheetString &copiedData = GetSheetRefData()->m_copiedData;
    wxString value;
    copiedData.RemoveEmptyRows();
    if (copiedData.GetCount() == 0)
        return value;

    int i, r, c;
    int row, col;
    int nrows = copiedData.GetCount();
    
    // find leftmost col
    int left_col = copiedData.GetItemValue(0).GetItemKey(0);
    for (r=1; r<nrows; r++)
    {
        if (left_col > copiedData.GetItemValue(r).GetItemKey(0))
            left_col = copiedData.GetItemValue(r).GetItemKey(0);
    }    
    
    int last_row=copiedData.GetItemKey(0), last_col=left_col;
    for (r=0; r<nrows; r++)
    {
        row = copiedData.GetItemKey(r);
        int ncols = copiedData.GetItemValue(r).GetCount();
        for (i = last_row; i<row; i++)
            value += wxT("\n");

        last_col = left_col;
        
        for (c=0; c<ncols; c++)
        {
            col = copiedData.GetItemValue(r).GetItemKey(c);
            for (i = last_col; i<col; i++)
                value += colSep;
            
            value += copiedData.GetItemValue(r).GetItemValue(c);
            last_col = col;
        }
        last_row = row;
    }
    
    return value;    
}

bool wxSheet::CopyStringToSelection(const wxString &string, const wxChar& colSep)
{
    wxPairArrayIntPairArraySheetString &copiedData = GetSheetRefData()->m_copiedData;
    copiedData.Clear();

    if (string.IsEmpty())
        return FALSE;
    
    const wxChar *c = string.GetData();
    int n, len = string.Length();
    
    int row = 0, col = 0;
    wxString buf;
    
    for (n=0; n<len; n++, c++)
    {
        if ((*c) == wxT('\n'))
        {
            copiedData.GetOrCreateValue(row).SetValue(col, buf);
            col = 0;
            row++;
            buf.Empty();
        }
        else if ((*c) == colSep)
        {
            copiedData.GetOrCreateValue(row).SetValue(col, buf);
            col++;
            buf.Empty();
        }
        else
            buf += *c;
    }    
    
    if (!buf.IsEmpty())
        copiedData.GetOrCreateValue(row).SetValue(col, buf);
    
    return TRUE;
}

bool wxSheet::PasteInternalCopiedSelection(const wxSheetCoords &topLeft_)
{
    wxPairArrayIntPairArraySheetString &copiedData = GetSheetRefData()->m_copiedData;
    // clean up, shouldn't happen but don't want to have to check it later
    copiedData.RemoveEmptyRows();
    if (copiedData.GetCount() == 0) return FALSE;

    wxSheetBlock currentBlock = GetSelection()->GetBoundingBlock().ExpandUnion(GetSelectingBlock());
    bool is_selection = !currentBlock.IsEmpty();
    
    wxSheetCoords topLeft(topLeft_);
    if (topLeft == wxNullSheetCoords)
    {
        if (is_selection)
            topLeft = currentBlock.GetLeftTop();
        else
            topLeft = GetGridCursorCell();
    }
    
    if (!ContainsGridCell(topLeft)) 
        return FALSE;
    
    wxSheetCoords copiedTopLeft( copiedData.GetItemKey(0), 
                                 copiedData.GetItemValue(0).GetItemKey(0) );
    
    wxSheetCoords shiftCell = topLeft - copiedTopLeft;
    
    GetSheetRefData()->m_pasting = TRUE;
    
    wxSheetCoords cell, pasteCell;
    size_t r, nrows = copiedData.GetCount();
    size_t c, ncols;
    for (r=0; r<nrows; r++)
    {
        cell.m_row = copiedData.GetItemKey(r);
        ncols = copiedData.GetItemValue(r).GetCount();

        // all done
        if (cell.m_row+shiftCell.m_row >= GetNumberRows())
            break;
        
        for (c=0; c<ncols; c++)
        {
            cell.m_col = copiedData.GetItemValue(r).GetItemKey(c);
            pasteCell = cell + shiftCell;
            if (pasteCell.m_col >= GetNumberCols())
                break;
            
            const wxString& value = copiedData.GetItemValue(r).GetItemValue(c);
            // only paste into selected cells if selection
            if (is_selection && IsCellSelected(pasteCell))
                SetCellValue(pasteCell, value);
            else
                SetCellValue(pasteCell, value);
        }
    }
    
    GetSheetRefData()->m_pasting = FALSE;
    return TRUE;
}

// ----------------------------------------------------------------------------
// Edit control functions (mostly used internally)

void wxSheet::EnableEditing( bool edit )
{
    // TODO: improve this ?
    if ( edit != IsEditable() )
    {
        if (!edit && IsCellEditControlCreated()) 
            DisableCellEditControl(TRUE);
        
        GetSheetRefData()->m_editable = edit;
    }
}

bool wxSheet::EnableCellEditControl( const wxSheetCoords& coords_ )
{
    // move to owner cell since that's where the editor is
    wxSheetCoords coords(GetCellOwner(coords_));
    wxCHECK_MSG(CanEnableCellControl(coords), FALSE, _T("can't enable editing for this cell!"));

    // already editing elsewhere, disable it
    if (IsCellEditControlCreated() && !DisableCellEditControl(TRUE))
        return FALSE;
    
    if (SendEvent( wxEVT_SHEET_EDITOR_ENABLED, coords ) == EVT_VETOED)
        return FALSE;

    // guarantee that it's visible
    MakeCellVisible(coords);
    GetSheetRefData()->m_cellEditorCoords = coords;
    
    // Get the appropriate editor for this cell
    wxSheetCellAttr attr(GetAttr(GetEditControlCoords()));
    GetSheetRefData()->m_cellEditor = attr.GetEditor(this, GetEditControlCoords());
    if (!GetEditControl().Ok())
    {
        GetSheetRefData()->m_cellEditorCoords = wxNullSheetCoords;
        wxFAIL_MSG(wxT("Unable to get cell edit control"));
        return FALSE;
    }        
    
    return ShowCellEditControl();
}

bool wxSheet::DisableCellEditControl( bool save_value ) 
{
    //wxCHECK_RET(, wxT("Edit control not enabled in wxSheet::DisableCellEditControl"));
    if (!IsCellEditControlCreated())
    {
        // just in case something failed earlier, make sure it's gone
        if (GetEditControl().Ok())
            GetSheetRefData()->m_cellEditor.Destroy();
        
        GetSheetRefData()->m_cellEditorCoords = wxNullSheetCoords;
        return TRUE;
    }

    if (SendEvent(wxEVT_SHEET_EDITOR_DISABLED, GetEditControlCoords()) == EVT_VETOED)
        return FALSE;
    
    if (save_value) 
        SaveEditControlValue(); 
    
    HideCellEditControl();
    GetSheetRefData()->m_cellEditor.DestroyControl(); // FIXME always destroy it else memory leak
    GetSheetRefData()->m_cellEditor.Destroy();
    GetSheetRefData()->m_cellEditorCoords = wxNullSheetCoords;
    return TRUE;
}

bool wxSheet::CanEnableCellControl(const wxSheetCoords& coords) const
{
    wxCHECK_MSG(ContainsCell(coords), FALSE, wxT("Invalid coords"));
    return IsEditable() && !GetAttr(coords).GetReadOnly();
}
bool wxSheet::IsCellEditControlCreated() const
{
    return GetEditControl().Ok() && GetEditControl().IsCreated();
}
bool wxSheet::IsCellEditControlShown() const
{
    return GetEditControl().Ok() && GetEditControl().IsCreated() && 
           GetEditControl().IsShown();
}

bool wxSheet::ShowCellEditControl()
{
    wxCHECK_MSG(CanEnableCellControl(GetEditControlCoords()), FALSE, // also checks coords
                wxT("Editor not enabled in wxSheet::ShowCellEditControl"));
    
    wxWindow *win = GetWindowForCoords(GetEditControlCoords());
    
    // If the control's parent is not correct we must recreate it 
    if (GetEditControl().GetControl() && (GetEditControl().GetControl()->GetParent() != win))
        GetSheetRefData()->m_cellEditor.DestroyControl();
    
    // Make sure the editor is created
    if (!GetEditControl().IsCreated())
    {
        GetSheetRefData()->m_cellEditor.CreateEditor(win, -1,
                               new wxSheetCellEditorEvtHandler(this, GetEditControl()));

        wxSheetEditorCreatedEvent evt(GetId(), wxEVT_SHEET_EDITOR_CREATED,
                                      this, GetEditControlCoords(), 
                                      GetSheetRefData()->m_cellEditor.GetControl());
        GetEventHandler()->ProcessEvent(evt);
        
        if (!GetEditControl().IsCreated())
        {
            // kill it all off, something went wrong
            if (GetEditControl().HasControl())
                GetSheetRefData()->m_cellEditor.DestroyControl();
            
            GetSheetRefData()->m_cellEditor.Destroy();
            GetSheetRefData()->m_cellEditorCoords = wxNullSheetCoords;
            wxFAIL_MSG(wxT("Unable to create edit control"));
            return FALSE;
        }
    }
    
    wxSheetCellAttr attr(GetAttr(GetEditControlCoords()));

    // the rectangle bounding the cell
    wxRect rect( CellToRect(GetEditControlCoords(), TRUE) );

    // cell is shifted by one pixel
    // However, don't allow x or y to become negative
    // since the SetSize() method interprets that as "don't change."
    if (rect.x > 0) rect.x--;
    if (rect.y > 0) rect.y--;

    // resize editor to overflow into righthand cells if wider than cell
    wxString value = GetCellValue(GetEditControlCoords());
    int clientWidth = win->GetClientSize().GetWidth();
    if (!value.IsEmpty() && attr.GetOverflow() &&
        (GetEditControlCoords().m_col < GetNumberCols()))
    {
        int w=rect.width, h=0;
        // expand width if text only if text is wider than cell width
        GetTextExtent(value, &w, &h, NULL, NULL, &attr.GetFont());

        if (w > rect.width)
        {
            wxSheetCoords cellSize(GetCellSpan(GetEditControlCoords()));
            wxSheetCoords c(GetEditControlCoords());
            wxSheetCoords oneCell(1,1);
            int numCols = GetNumberCols();
            for (c.m_col = c.m_col + cellSize.m_col; c.m_col < numCols; c.m_col++)
            {
                cellSize = GetCellSpan(c);
               // looks weird going over a spanned cell
                if ((rect.width < w) && (rect.GetRight() < clientWidth) && (cellSize == oneCell))
                    rect.width += GetColWidth(c.m_col);
                else
                    break;
            }
        }
    }
    
    // clip width to window size
    rect.width = wxMin(rect.width, clientWidth - rect.x);
    
    GetSheetRefData()->m_cellEditor.SetSize( rect, attr );
    GetSheetRefData()->m_cellEditor.Show( TRUE, attr );

    // recalc dimensions, maybe expand the scrolled window to account for editor
    CalcWindowSizes();

    GetSheetRefData()->m_cellEditor.BeginEdit(GetEditControlCoords(), this);
    
    // FIXME other spreadsheets don't clear cells to right, it looks weird? why not?
    // if this is empty, cell to right maybe overflowed into from left, clear it
    //if (value.IsEmpty() && (m_cellEditorCoords.m_col < m_numCols - 1))
    //    RefreshCell(m_cellEditorCoords+wxSheetCoords(0, 1));
    
    return TRUE;
}

bool wxSheet::HideCellEditControl()
{
    wxCHECK_MSG(IsCellEditControlShown(), FALSE, wxT("Edit control not shown in wxSheet::HideCellEditControl"));

    GetSheetRefData()->m_cellEditor.Show( FALSE, GetAttr(GetEditControlCoords()) );
    m_gridWin->SetFocus();
    RefreshCell(GetEditControlCoords(), FALSE);
    return TRUE;
}

void wxSheet::SaveEditControlValue()
{
    wxCHECK_RET(IsCellEditControlCreated(), wxT("Edit control not shown in wxSheet::HideCellEditControl"));

    wxString oldval = GetCellValue(GetEditControlCoords());

    if (GetSheetRefData()->m_cellEditor.EndEdit(GetEditControlCoords(), this))
    {
        // Event has been vetoed, set the data back.
        if ( SendEvent(wxEVT_SHEET_CELL_VALUE_CHANGED, GetEditControlCoords()) == EVT_VETOED ) 
            SetCellValue(GetEditControlCoords(), oldval);
    }
}

// ----------------------------------------------------------------------------
// Drawing functions

void wxSheet::EndBatch(bool refresh)
{
    if ( m_batchCount > 0 )
    {
        m_batchCount--;
        if ( !m_batchCount && refresh )
        {
            m_batchCount++;        // temp block refresh for window sizing
            AdjustScrollbars();
            m_batchCount--;
            RefreshRowLabelWindow();
            RefreshColLabelWindow();
            RefreshCornerLabelWindow();
            RefreshGridWindow(FALSE);
        }
    }
}

void wxSheet::Refresh(bool eraseb, const wxRect* rect_)
{
    // Don't do anything if between Begin/EndBatch...
    // EndBatch() will do all this on the last nested one anyway.
    if (GetBatchCount())
        return;

    if (rect_)
    {
        wxRect rect(rect_->Intersect(wxRect(wxPoint(0,0), GetClientSize())));
        if (wxRectIsEmpty(rect))
            return;
        
        int rect_x, rect_y, rectWidth, rectHeight;
        int width_label, width_cell, height_label, height_cell;
        int x, y;

        //Copy rectangle can get scroll offsets..
        rect_x = rect.GetX();
        rect_y = rect.GetY();
        rectWidth = rect.GetWidth();
        rectHeight = rect.GetHeight();

        width_label = GetRowLabelWidth() - rect_x;
        if (width_label > rectWidth) width_label = rectWidth;

        height_label = GetColLabelHeight() - rect_y;
        if (height_label > rectHeight) height_label = rectHeight;

        if (rect_x > GetRowLabelWidth())
        {
            x = rect_x - GetRowLabelWidth();
            width_cell = rectWidth;
        }
        else
        {
            x = 0;
            width_cell = rectWidth - (GetRowLabelWidth() - rect_x);
        }

        if (rect_y > GetColLabelHeight())
        {
            y = rect_y - GetColLabelHeight();
            height_cell = rectHeight;
        }
        else
        {
            y = 0;
            height_cell = rectHeight - (GetColLabelHeight() - rect_y);
        }

        // Paint corner label part intersecting rect.
        if ( (width_label > 0) && (height_label > 0) )
        {
            wxRect cornerRect(rect_x, rect_y, width_label, height_label);
            RefreshCornerLabelWindow(eraseb, &cornerRect);
        }

        // Paint col labels part intersecting rect.
        if ( (width_cell > 0) && (height_label > 0) )
        {
            wxRect colsRect(x, rect_y, width_cell, height_label);
            RefreshColLabelWindow(eraseb, &colsRect);
        }

        // Paint row labels part intersecting rect.
        if ( (width_label > 0) && (height_cell > 0) )
        {
            wxRect rowsRect(rect_x, y, width_label, height_cell);
            RefreshRowLabelWindow(eraseb, &rowsRect);
        }

        // Paint cell area part intersecting rect.
        if ( (width_cell > 0) && (height_cell > 0) )
        {
            wxRect areaRect(x, y, width_cell, height_cell);
            RefreshGridWindow(eraseb, &areaRect);
        }
    }
    else
    {
        RefreshCornerLabelWindow(eraseb, NULL);
        RefreshColLabelWindow(eraseb, NULL);
        RefreshRowLabelWindow(eraseb, NULL);
        RefreshGridWindow(eraseb, NULL);
    }
}

void wxSheet::RefreshGridWindow(bool eraseb, const wxRect* rect)
{
    if (GetBatchCount())
        return;
    
    if (rect)
    {
        wxRect r(rect->Intersect(wxRect(wxPoint(0,0), m_gridWin->GetClientSize())));
        if (!wxRectIsEmpty(r))
            m_gridWin->Refresh(eraseb, &r);
    }
    else
        m_gridWin->Refresh(eraseb, rect);
    
    GetSheetRefData()->RefreshGridWindow(this, rect);
}
void wxSheet::RefreshRowLabelWindow(bool eraseb, const wxRect* rect)
{
    if (GetBatchCount())
        return;

    if (rect)
    {
        wxRect r(rect->Intersect(wxRect(wxPoint(0,0), m_rowLabelWin->GetClientSize())));
        if (!wxRectIsEmpty(r) && m_rowLabelWin->IsShown())
            m_rowLabelWin->Refresh(eraseb, &r);
    }
    else
        m_rowLabelWin->Refresh(eraseb, rect);
    
    GetSheetRefData()->RefreshRowLabelWindow(this, rect);
}
void wxSheet::RefreshColLabelWindow(bool eraseb, const wxRect* rect)
{
    if (GetBatchCount())
        return;

    if (rect)
    {
        wxRect r(rect->Intersect(wxRect(wxPoint(0,0), m_colLabelWin->GetClientSize())));
        if (!wxRectIsEmpty(r) && m_colLabelWin->IsShown())
            m_colLabelWin->Refresh(eraseb, &r);
    }
    else
        m_colLabelWin->Refresh(eraseb, rect);
    
    GetSheetRefData()->RefreshColLabelWindow(this, rect);
}
void wxSheet::RefreshCornerLabelWindow(bool eraseb, const wxRect* rect)
{
    if (GetBatchCount())
        return;

    m_cornerLabelWin->Refresh(eraseb, rect);
    GetSheetRefData()->RefreshCornerLabelWindow(this, rect);
}

void wxSheet::RefreshCell(const wxSheetCoords& coords, bool single_cell)
{
    if ( GetBatchCount() )
        return;

    if (IsCornerLabelCell(coords))
    {
        RefreshCornerLabelWindow(TRUE);
        return;
    }
    else if (ContainsRowLabelCell(coords))
    {
        wxRect rect(CellToRect(coords, TRUE));
        RefreshRowLabelWindow( TRUE, &rect );
        return;
    }
    else if (ContainsColLabelCell(coords))
    {
        wxRect rect(CellToRect(coords, TRUE));
        RefreshColLabelWindow( TRUE, &rect );
        return;
    }
        
    wxCHECK_RET(ContainsGridCell(coords), wxT("Invalid coords in wxSheet::RefreshCell"));
    
    // refresh cell, but only if shown
    wxRect rect(CellToRect(coords, TRUE));
    
    if (!single_cell)
    {
        // hack to draw previous cell, if this cell turns empty and can be
        // overflowed into we need to erase the previous cell's |> cutoff marker
        if (coords.m_col > 0)
            rect.x = GetColLeft(coords.m_col-1);
        // we do have to draw the whole row right though
        rect.width = GetVirtualSize().x - rect.x; 
    }
    
    RefreshGridWindow( FALSE, &rect );
}

void wxSheet::RefreshRow( int row )
{
    if (GetBatchCount())
        return;
    
    if (row == -1)
    {
        RefreshColLabelWindow(TRUE);
        return;
    }
    
    wxCHECK_RET(ContainsGridRow(row), wxT("Invalid row"));

    // now use BlockToDeviceRect to include spanned cells 
    wxRect rect(BlockToDeviceRect(wxSheetBlock(row, 0, 1, GetNumberCols())));
    RefreshGridWindow( FALSE, &rect );
}

void wxSheet::RefreshCol( int col )
{
    if (GetBatchCount())
        return;
    
    if (col == -1)
    {
        RefreshRowLabelWindow(TRUE);
        return;
    }
    
    wxCHECK_RET(ContainsGridCol(col), wxT("Invalid col"));

    // now use BlockToDeviceRect to include spanned cells 
    wxRect rect(BlockToDeviceRect(wxSheetBlock(0, col, GetNumberRows(), 1)));
    RefreshGridWindow( FALSE, &rect );
}

void wxSheet::RefreshGridCellBlock( const wxSheetBlock& block_ )
{
    // no error msgs for this function, otherwise too many checks elsewhere
    if (block_.IsEmpty() || GetBatchCount())
        return;
    
    wxSheetBlock block(block_.Intersect(wxSheetBlock(0, 0, GetNumberRows(), GetNumberCols())));
    
    wxRect rect(BlockToDeviceRect( block ));
    RefreshGridWindow( FALSE, &rect );
}

void wxSheet::RefreshAttrChange(const wxSheetCoords& coords, wxSheetAttr_Type type)
{
    if (IsGridCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault : 
            {
                m_gridWin->SetBackgroundColour(DoGetDefaultGridAttr().GetBackgroundColour());
                RefreshGridWindow(FALSE);
                return;
            }
            case wxSHEET_AttrCol :
            {
                if (ContainsGridCol(coords.m_col))
                    RefreshCol(coords.m_col);
                    
                return;
            }
            case wxSHEET_AttrRow :
            {
                if (ContainsGridRow(coords.m_row))
                    RefreshRow(coords.m_row);
                
                return;
            }
            case wxSHEET_AttrCell :
            default               : 
            {
                if (ContainsGridCell(coords))
                    RefreshCell(coords, FALSE);
                return;
            }
        }
    }
    else if (IsCornerLabelCell(coords))
    {
        m_cornerLabelWin->SetBackgroundColour(DoGetDefaultCornerLabelAttr().GetBackgroundColour());
        RefreshCornerLabelWindow(TRUE);
        return;
    }
    else if (IsRowLabelCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault :
            {    
                m_rowLabelWin->SetBackgroundColour(DoGetDefaultRowLabelAttr().GetBackgroundColour());
                RefreshRowLabelWindow(TRUE);
                return;
            }
            case wxSHEET_AttrCell :
            default               : 
            {
                if (ContainsRowLabelCell(coords))
                    RefreshCell(coords);
                return;
            }
        }
    }
    else if (IsColLabelCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault :
            {    
                m_colLabelWin->SetBackgroundColour(DoGetDefaultColLabelAttr().GetBackgroundColour());
                RefreshColLabelWindow(TRUE);
                return;
            }
            case wxSHEET_AttrCell :
            default               : 
            {
                if (ContainsColLabelCell(coords))
                    RefreshCell(coords);
                return;
            }
        }
    }
}

void wxSheet::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    wxPaintDC dc(this);

    // draw the bottom right square area between the scrollbars 
    // since we trap EVT_ERASE_BACKGROUND
    if (m_vertScrollBar->IsShown() && m_horizScrollBar->IsShown())
    {
        dc.SetBrush(wxBrush(GetBackgroundColour(), wxSOLID));
        dc.SetPen(*wxTRANSPARENT_PEN);
        wxSize clientSize = GetClientSize();
        int top  = m_vertScrollBar->GetRect().GetBottom();
        int left = m_horizScrollBar->GetRect().GetRight() + m_horizSplitRect.GetWidth();
        dc.DrawRectangle(left, top, clientSize.x-left, clientSize.y-top);
    }
    
    if (!wxRectIsEmpty(m_vertSplitRect))
        DrawSplitterButton(dc, m_vertSplitRect);
    if (!wxRectIsEmpty(m_horizSplitRect))
        DrawSplitterButton(dc, m_horizSplitRect);
}

void wxSheet::DrawSplitterButton(wxDC &dc, const wxRect& rect)
{
    //dc.SetPen(*wxBLACK_PEN);
    //dc.SetBrush(*wxBLACK_BRUSH);
    //dc.DrawRectangle(rect);
    
    wxPen highlight(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNHIGHLIGHT), 1, wxSOLID);
    wxPen shadow(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW), 1, wxSOLID);
    wxPen darkShadow(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW), 1, wxSOLID);

    dc.SetPen(shadow);
    dc.SetBrush(wxBrush(GetBackgroundColour(), wxSOLID));
    dc.DrawRectangle(rect);
    dc.DrawLine(rect.GetRight()-1, rect.y+2, rect.GetRight()-1, rect.GetBottom()-1);
    dc.DrawLine(rect.x+2, rect.GetBottom()-1, rect.GetRight(), rect.GetBottom()-1);

    dc.SetPen(darkShadow);
    dc.DrawLine(rect.GetRight(), rect.y+1, rect.GetRight(), rect.GetBottom());
    dc.DrawLine(rect.x+1, rect.GetBottom(), rect.GetRight(), rect.GetBottom());
    
    dc.SetPen(highlight);
    dc.DrawLine(rect.x+1, rect.y+1, rect.GetRight()-1, rect.y+1);
    dc.DrawLine(rect.x+1, rect.y+1, rect.x+1, rect.GetBottom()-1);
}

void wxSheet::OnGridWindowPaint( wxDC& dc, const wxRegion& reg )
{
    wxSheetSelection dirtyBlocks;
    CalcCellsExposed( reg, dirtyBlocks );
    
    DrawGridCells( dc, dirtyBlocks ); 
#if WXSHEET_DRAW_LINES
    DrawAllGridLines( dc, reg );
#endif
    DrawGridSpace( dc );
    DrawCursorHighlight( dc, dirtyBlocks );   
}
void wxSheet::OnRowLabelWindowPaint( wxDC& dc, const wxRegion& reg )
{
    wxArrayInt rows;
    if (CalcRowLabelsExposed(reg, rows))
        DrawRowLabels( dc, rows );
}
void wxSheet::OnColLabelWindowPaint( wxDC& dc, const wxRegion& reg )
{
    wxArrayInt cols;
    if (CalcColLabelsExposed(reg, cols))
        DrawColLabels( dc, cols );
}
void wxSheet::OnCornerLabelWindowPaint( wxDC& dc, const wxRegion& WXUNUSED(reg) )
{
    DrawCornerLabel( dc );
}

// Note - this function only draws cells that are in the list of
// exposed cells (usually set from the update region by CalcExposedCells)
void wxSheet::DrawGridCells( wxDC& dc, const wxSheetSelection& blockSel_ )
{
    if ( !GetNumberRows() || !GetNumberCols() || GetBatchCount() ) 
        return;

    wxSheetSelection blockSel(blockSel_);       // modified to remove spanned cells
    wxSheetSelection removedSel;                // removed spanned
    wxSheetSelectionIterator selIter(blockSel); // forward iterator 
 
    wxSheetCoords coords;    
    wxSheetBlock  cellBlock;
    
    int check_overflow_row = -1;
    
    while (selIter.GetNext(coords))
    {
        if (removedSel.Contains(coords))
            continue;
        
        // trim out cells part of spanned cells, leaving only the owner cell
        cellBlock = GetCellBlock(coords);
        if (!cellBlock.IsOneCell())
        {
            // forget part of spanned cell, we only care about owner cell
            coords = cellBlock.GetLeftTop();  
            // remove whole spanned cell and add back the owner
            blockSel.DeselectBlock(cellBlock);
            blockSel.SelectBlock(wxSheetBlock(coords, 1, 1));
            // don't bother checking whole block again
            removedSel.SelectBlock(cellBlock);  
        }

        int bottom = cellBlock.GetBottom();

        // if empty find cell to left that might overflow into this one
        // only need to check left side of each block, hence check_overflow_row 
        if ((bottom > check_overflow_row) && GetTable() && GetTable()->IsEmptyCell(coords))
        {
            check_overflow_row = bottom;
            
            wxSheetCoords c;
            for ( c.m_row = coords.m_row; c.m_row <= bottom; c.m_row++ )
            {
                c.m_col = coords.m_col;
                while (c.m_col > 0) 
                {
                    c.m_col = GetTable()->GetFirstNonEmptyColToLeft(c);
                    if (c.m_col < 0)
                        break;
                    
                    // if !empty, stop, this is the only cell that could overflow
                    // Must check since default implementation of 
                    // GetFirstNonEmptyColToLeft just returns the previous col
                    if (!GetTable()->IsEmptyCell(c))
                    {
                        wxSheetCellAttr attr(GetAttr(c));
                        if (attr.GetOverflow())
                        {
                            // check if this cell actually does overflow into coords
                            int width = attr.GetRenderer(this, c).GetBestSize(*this, attr, dc, c).GetWidth();

                            if (GetColLeft(c.m_col)+width > GetColLeft(coords.m_col))
                                blockSel.SelectBlock(wxSheetBlock(c, 1, 1));
                        }
                        break;
                    }
                }
            }
        }
    }
    
    //static long counter = 0; ++counter;
    
    // paint cells in reverse order bottom to top, right to left for overflow
    wxSheetSelectionIterator revIter(blockSel, wxSSI_REVERSE);
    while (revIter.GetNext(coords))
    {
        //wxPrintf(wxT("%ld Drawing Cell %d %d\n"), counter, coords.m_row, coords.m_col); 
        DrawCell( dc, coords );
    }    
}

void wxSheet::DrawGridSpace( wxDC& dc )
{
    int cw, ch;
    m_gridWin->GetClientSize( &cw, &ch );

    int right, bottom;
    CalcUnscrolledPosition( cw, ch, &right, &bottom );

    int rightCol  = GetNumberCols() > 0 ? GetColRight(GetNumberCols() - 1)  : 0;
    int bottomRow = GetNumberRows() > 0 ? GetRowBottom(GetNumberRows() - 1) : 0;

    if ( (right > rightCol) || (bottom > bottomRow) )
    {
        int left, top;
        CalcUnscrolledPosition( 0, 0, &left, &top );

        dc.SetBrush( wxBrush(GetAttrBackgroundColour(wxSheetCoords(0,0), wxSHEET_AttrDefault), wxSOLID) );
        dc.SetPen( *wxTRANSPARENT_PEN );

        if ( right > rightCol )
            dc.DrawRectangle( rightCol, top, right - rightCol, ch);
        if ( bottom > bottomRow )
            dc.DrawRectangle( left, bottomRow, cw, bottom - bottomRow);
    }
}

void wxSheet::DrawCell( wxDC& dc, const wxSheetCoords& coords )
{
    wxRect rect(CellToRect(coords));
    if (wxRectIsEmpty(rect))  // !IsShown
        return;

    // we draw the cell border ourselves
#if !WXSHEET_DRAW_LINES
    if ( GetSheetRefData()->m_gridLinesEnabled )
        DrawCellBorder( dc, coords );
#endif

    wxSheetCellAttr attr(GetAttr(coords));
    
    // if the editor is shown, we should use it and not the renderer
    if ( (coords == GetEditControlCoords()) && IsCellEditControlShown() )
        GetSheetRefData()->m_cellEditor.PaintBackground(*this, attr, dc, rect, coords, IsCellSelected(coords));
    else
        attr.GetRenderer(this, coords).Draw(*this, attr, dc, rect, coords, IsCellSelected(coords));
}

void wxSheet::DrawCursorHighlight(wxDC& dc, const wxSheetSelection& blockSel)
{
    // This if block was previously in wxSheet::OnPaint but that doesn't
    // seem to get called under wxGTK - MB
    if ( !ContainsGridCell(GetGridCursorCell()) && GetNumberRows() && GetNumberCols() )
        GetSheetRefData()->m_cursorCoords.Set(0, 0);

    // don't show highlight when the edit control is shown
    if (!blockSel.Contains(GetGridCursorCell()) || IsCellEditControlShown())
        return;

    DrawCursorCellHighlight(dc, GetAttr(GetGridCursorCell()));
}

void wxSheet::DrawCursorCellHighlight( wxDC& dc, const wxSheetCellAttr& attr )
{
    if (!ContainsGridCell(GetGridCursorCell()))
        return;

    wxSheetCoords coords(GetCellOwner(GetGridCursorCell()));
    wxRect rect( CellToRect(coords) );
    if (wxRectIsEmpty(rect))  // !IsCellShown
        return;

    // hmmm... what could we do here to show that the cell is disabled?
    // for now, I just draw a thinner border than for the other ones, but
    // it doesn't look really good

    int penWidth = attr.GetReadOnly() ? GetCursorCellHighlightROPenWidth() : GetCursorCellHighlightPenWidth();

    if (penWidth > 0)
    {
        // The center of the drawn line is where the position/width/height of
        // the rectangle is actually at, (on wxMSW atr least,) so we will
        // reduce the size of the rectangle to compensate for the thickness of
        // the line.  If this is too strange on non wxMSW platforms then
        // please #ifdef this appropriately.
        rect.x += penWidth/2;
        rect.y += penWidth/2;
        rect.width -= penWidth-1;
        rect.height -= penWidth-1;

        // Now draw the rectangle
        // use the cellHighlightColour if the cell is inside a selection, this
        // will ensure the cell is always visible.
        dc.SetPen(wxPen(IsCellSelected(coords) ? GetSelectionForeground() :
                                                 GetCursorCellHighlightColour(), 
                          penWidth, wxSOLID));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle(rect);
    }

#if 0
        // VZ: my experiments with 3d borders...

        // how to properly set colours for arbitrary bg?
        wxCoord x1 = rect.x,
                y1 = rect.y,
                x2 = rect.x + rect.width -1,
                y2 = rect.y + rect.height -1;

        dc.SetPen(*wxWHITE_PEN);
        dc.DrawLine(x1, y1, x2, y1);
        dc.DrawLine(x1, y1, x1, y2);

        dc.DrawLine(x1 + 1, y2 - 1, x2 - 1, y2 - 1);
        dc.DrawLine(x2 - 1, y1 + 1, x2 - 1, y2 );

        dc.SetPen(*wxBLACK_PEN);
        dc.DrawLine(x1, y2, x2, y2);
        dc.DrawLine(x2, y1, x2, y2+1);
#endif // 0
}

void wxSheet::DrawCellBorder( wxDC& dc, const wxSheetCoords& coords )
{
    wxRect rect(CellToRect( coords ));
    if ( wxRectIsEmpty(rect) )  // !IsCellShown
        return;

    dc.SetPen( wxPen(GetGridLineColour(), 1, wxSOLID) );
    // right hand border
    dc.DrawLine( rect.x + rect.width, rect.y,
                 rect.x + rect.width, rect.y + rect.height + 1 );

    // bottom border
    dc.DrawLine( rect.x,              rect.y + rect.height,
                 rect.x + rect.width, rect.y + rect.height);
}

// TODO: remove this ???
// This is used to redraw all grid lines e.g. when the grid line colour
// has been changed
void wxSheet::DrawAllGridLines( wxDC& dc, const wxRegion & WXUNUSED(reg) )
{
#if !WXSHEET_DRAW_LINES
    return;
#endif

    if ( GetBatchCount() || !GridLinesEnabled() || !GetNumberRows() || !GetNumberCols() ) 
        return;

    int top, bottom, left, right;

#if 0  //#ifndef __WXGTK__
    if (reg.IsEmpty())
    {
        int cw, ch;
        m_gridWin->GetClientSize(&cw, &ch);

        // virtual coords of visible area
        CalcUnscrolledPosition( 0, 0, &left, &top );
        CalcUnscrolledPosition( cw, ch, &right, &bottom );
    }
    else
    {
        wxCoord x, y, w, h;
        reg.GetBox(x, y, w, h);
        CalcUnscrolledPosition( x, y, &left, &top );
        CalcUnscrolledPosition( x + w, y + h, &right, &bottom );
    }
#else
    int cw, ch;
    m_gridWin->GetClientSize(&cw, &ch);
    CalcUnscrolledPosition( 0, 0, &left, &top );
    CalcUnscrolledPosition( cw, ch, &right, &bottom );
#endif

    // avoid drawing grid lines past the last row and col
    right  = wxMin( right,  GetColRight(GetNumberCols() - 1) );
    bottom = wxMin( bottom, GetRowBottom(GetNumberRows() - 1) );
    
    // no gridlines inside spanned cells, clip them out
    int leftCol   = XToGridCol(left, TRUE);
    int topRow    = YToGridRow(top, TRUE);
    int rightCol  = XToGridCol(right, TRUE);
    int bottomRow = YToGridRow(bottom, TRUE);
    int i;

    if (HasSpannedCells())
    {
        wxRegion clippedcells(0, 0, cw, ch);
        wxSheetCoords cell;
        wxSheetBlock  cellBlock;
        for (cell.m_row = topRow; cell.m_row < bottomRow; cell.m_row++)
        {
            for (cell.m_col = leftCol; cell.m_col < rightCol; cell.m_col++)
            {
                cellBlock = GetCellBlock(cell);
                if (!cellBlock.IsOneCell())
                {
                    clippedcells.Subtract(CellToRect(cell, TRUE));
                    cell.m_col = cellBlock.GetRight();
                }
            }
        }
        dc.SetClippingRegion( clippedcells );
    }
    
    dc.SetPen( wxPen(GetGridLineColour(), 1, wxSOLID) );

    int numRows = GetNumberRows();
    int numCols = GetNumberCols();
    
    // horizontal grid lines
    for ( i = topRow; i < numRows; i++ )
    {
        int bot = GetRowBottom(i) - 1;
        if ( bot > bottom )
            break;

        if ( bot >= top )
            dc.DrawLine( left, bot, right, bot );
    }

    // vertical grid lines
    for ( i = leftCol; i < numCols; i++ )
    {
        int colRight = GetColRight(i) - 1;
        if ( colRight > right )
            break;

        if ( colRight >= left )
            dc.DrawLine( colRight, top, colRight, bottom );
    }
    
    dc.DestroyClippingRegion();
}

void wxSheet::DrawRowLabels( wxDC& dc, const wxArrayInt& rows )
{
    size_t i, numLabels = rows.GetCount();
    if ( !GetNumberRows() || !numLabels ) return;

    for ( i = 0; i < numLabels; i++ )
        DrawCell( dc, wxSheetCoords(rows[i], -1) );
    
    // Now draw the dividing lines
    dc.SetPen( wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW),1, wxSOLID) );
    int top, dummy;
    CalcUnscrolledPosition(0, 0, &dummy, &top);
    int bottom = GetRowBottom(rows[numLabels-1]);
    // left of row labels
    dc.DrawLine(0, top, 0, bottom);
    // right of row labels
    dc.DrawLine(GetRowLabelWidth()-1, top, GetRowLabelWidth()-1, bottom);
    // draw bottoms
    for ( i = 0; i < numLabels; i++ )
    {
        bottom = GetRowBottom(rows[i])-1;
        dc.DrawLine(0, bottom, GetRowLabelWidth()-1, bottom);
    }
}

void wxSheet::DrawColLabels( wxDC& dc, const wxArrayInt& cols )
{
    size_t i, numLabels = cols.GetCount();
    if ( !GetNumberCols() || !numLabels ) return;

    for ( i = 0; i < numLabels; i++ )
        DrawCell( dc, wxSheetCoords(-1, cols[i]) );

    // Now draw the dividing lines
    dc.SetPen( wxPen(GetLabelOutlineColour(), 1, wxSOLID) );
    int left, dummy;
    CalcUnscrolledPosition(0, 0, &left, &dummy);
    int right = GetColRight(cols[numLabels-1]); 
    // top of col labels
    dc.DrawLine(left, 0, right, 0);
    // bottom of col labels
    dc.DrawLine(left, GetColLabelHeight()-1, right, GetColLabelHeight()-1);
    // draw rights
    for ( i = 0; i < numLabels; i++ )
    {
        right = GetColRight(cols[i])-1;
        dc.DrawLine(right, 0, right, GetColLabelHeight()-1);
    }
}

void wxSheet::DrawCornerLabel( wxDC& dc )
{
    DrawCell( dc, wxSheetCoords(-1, -1) );

    // Now draw the dividing lines
    dc.SetPen( wxPen(GetLabelOutlineColour(), 1, wxSOLID) );
    wxSize size = m_cornerLabelWin->GetClientSize();
    dc.DrawLine(0, 0, size.x, 0);                   // top
    dc.DrawLine(0, size.y-1, size.x, size.y-1);     // bottom
    dc.DrawLine(0, 0, 0, size.y-1);                 // left
    dc.DrawLine(size.x-1, 0, size.x-1, size.y-1);   // right
}

void wxSheet::DrawRowColResizingMarker( int newDragPos )
{
    if ( ((m_mouseCursorMode & WXSHEET_CURSOR_RESIZING) == 0) ||
         ((m_dragLastPos == -1) && (newDragPos == -1)) )
        return;
    
    wxClientDC dc( m_gridWin );
    PrepareGridDC( dc );

    int cw, ch, left, top;
    m_gridWin->GetClientSize( &cw, &ch );
    CalcUnscrolledPosition( 0, 0, &left, &top );
    
    if (GridLinesEnabled())
    {
        dc.SetPen(wxPen(GetGridLineColour(), 2, wxSOLID));
    
        // Draw the anchor marker so you know what row/col you're resizing
        if (m_mouseCursorMode == WXSHEET_CURSOR_RESIZE_ROW)
        {
            int anchor = GetRowTop(m_dragRowOrCol);
            dc.DrawLine( left, anchor, left+cw, anchor );
        }
        else
        {
            int anchor = GetColLeft(m_dragRowOrCol);
            dc.DrawLine( anchor, top, anchor, top+ch );
        }
    }
    
    int log_fn = dc.GetLogicalFunction();
    dc.SetLogicalFunction(wxINVERT);

    if (m_dragLastPos >= 0)
    {
        if (m_mouseCursorMode == WXSHEET_CURSOR_RESIZE_ROW)
            dc.DrawLine( left, m_dragLastPos, left+cw, m_dragLastPos );
        else
            dc.DrawLine( m_dragLastPos, top, m_dragLastPos, top+ch );
    }

    if (newDragPos >= 0)
    {
        m_dragLastPos = newDragPos;
        
        if (m_mouseCursorMode == WXSHEET_CURSOR_RESIZE_ROW)
            dc.DrawLine( left, m_dragLastPos, left+cw, m_dragLastPos );
        else
            dc.DrawLine( m_dragLastPos, top, m_dragLastPos, top+ch );
    }
    
    dc.SetLogicalFunction(log_fn); // set it back since nobody else wants invert
}

bool wxSheet::CalcRowLabelsExposed( const wxRegion& reg, wxArrayInt& rowLabels ) const
{
    wxRegionIterator iter( reg );
    wxRect r;

    int top, bottom;
    while ( iter )
    {
        r = iter.GetRect();

        // TODO: remove this when we can...
        // There is a bug in wxMotif that gives garbage update
        // rectangles if you jump-scroll a long way by clicking the
        // scrollbar with middle button.  This is a work-around
        //
#if defined(__WXMOTIF__)
        int cw, ch;
        m_gridWin->GetClientSize( &cw, &ch );
        if ( r.GetTop() > ch ) r.SetTop( 0 );
        r.SetBottom( wxMin( r.GetBottom(), ch ) );
#endif

        // logical bounds of update region
        int dummy;
        CalcUnscrolledPosition( 0, r.GetTop(), &dummy, &top );
        CalcUnscrolledPosition( 0, r.GetBottom(), &dummy, &bottom );
        int numRows = GetNumberRows();
        
        // find the row labels within these bounds
        for ( int row = YToGridRow(top, TRUE);  row < numRows;  row++ )
        {
            if ( GetRowBottom(row) < top )
                continue;
            if ( GetRowTop(row) > bottom )
                break;

            rowLabels.Add( row );
        }

        iter++ ;
    }

    return rowLabels.GetCount() > 0u;
}

bool wxSheet::CalcColLabelsExposed( const wxRegion& reg, wxArrayInt& colLabels ) const
{
    wxRegionIterator iter( reg );
    wxRect r;

    int left, right;
    while ( iter )
    {
        r = iter.GetRect();

        // TODO: remove this when we can...
        // There is a bug in wxMotif that gives garbage update
        // rectangles if you jump-scroll a long way by clicking the
        // scrollbar with middle button.  This is a work-around
        //
#if defined(__WXMOTIF__)
        int cw, ch;
        m_gridWin->GetClientSize( &cw, &ch );
        if ( r.GetLeft() > cw ) r.SetLeft( 0 );
        r.SetRight( wxMin( r.GetRight(), cw ) );
#endif

        // logical bounds of update region
        int dummy;
        CalcUnscrolledPosition( r.GetLeft(), 0, &left, &dummy );
        CalcUnscrolledPosition( r.GetRight(), 0, &right, &dummy );
        int numCols = GetNumberCols();
        
        // find the cells within these bounds
        for ( int col = XToGridCol(left, TRUE);  col < numCols;  col++ )
        {
            if ( GetColRight(col) < left )
                continue;
            if ( GetColLeft(col) > right )
                break;

            colLabels.Add( col );
        }

        iter++ ;
    }
    return colLabels.GetCount() > 0u;
}

bool wxSheet::CalcCellsExposed( const wxRegion& reg, wxSheetSelection& blockSel ) const
{
    wxRegionIterator iter( reg );
    wxRect r;
    
    int left, top, right, bottom;
    while ( iter )
    {
        
        r = iter.GetRect();

        // TODO: remove this when we can...
        // There is a bug in wxMotif that gives garbage update
        // rectangles if you jump-scroll a long way by clicking the
        // scrollbar with middle button.  This is a work-around
        //
#if defined(__WXMOTIF__)
        int cw, ch;
        m_gridWin->GetClientSize( &cw, &ch );
        if ( r.GetTop() > ch ) r.SetTop( 0 );
        if ( r.GetLeft() > cw ) r.SetLeft( 0 );
        r.SetRight( wxMin( r.GetRight(), cw ) );
        r.SetBottom( wxMin( r.GetBottom(), ch ) );
#endif

        // logical bounds of update region
        CalcUnscrolledPosition( r.GetLeft(), r.GetTop(), &left, &top );
        CalcUnscrolledPosition( r.GetRight(), r.GetBottom(), &right, &bottom );

        int numRows = GetNumberRows();
        int numCols = GetNumberCols();
        
        // find the cells within these bounds
        wxSheetBlock block(YToGridRow(top, TRUE), XToGridCol(left, TRUE), 0, 0);
        int row, col;
        for ( row = block.GetTop(); row < numRows; row++ )
        {
            if ( GetRowBottom(row) <= top )
                continue;
            else if ( GetRowTop(row) > bottom )
                break;
        }
        
        for ( col = block.GetLeft(); col < numCols; col++ )
        {
            if ( GetColRight(col) <= left )
                continue;
            else if ( GetColLeft(col) > right )
                break;
        }

        block.SetRight(col - 1);
        block.SetBottom(row - 1);
        
        blockSel.SelectBlock(block);

        iter++;
    }
    
    return blockSel.GetCount() > 0;
}

void wxSheet::DrawTextRectangle( wxDC& dc, const wxString& value, 
                                 const wxRect& rect, int align, 
                                 int textOrientation )
{
    wxArrayString lines;
    if (StringToLines( value, lines ) > 0)
        DrawTextRectangle( dc, lines, rect, align, textOrientation );
}

void wxSheet::DrawTextRectangle( wxDC& dc, const wxArrayString& lines,
                                 const wxRect& rect, int align,
                                 int textOrientation )
{
    int nLines = lines.GetCount();
    if ( (nLines == 0) || ((nLines == 1) && lines[0].IsEmpty()) )
        return;
    
    dc.SetClippingRegion( rect );

    int l;
    float x = 0.0, y = 0.0;
    long textWidth=0, textHeight=0;
    long lineWidth=0, lineHeight=0;
    wxArrayInt lineWidths, lineHeights;
    
    // Measure the text extent once, Gtk2 is slow (takes 2sec off 23sec run)
    for ( l = 0; l < nLines; l++ )
    {
        dc.GetTextExtent(lines[l], &lineWidth, &lineHeight);
        lineWidths.Add(lineWidth);
        lineHeights.Add(lineHeight);
        textHeight += lineHeight;
        if (lineWidth > textWidth) 
            textWidth = lineWidth;
    }

    if ( textOrientation == wxVERTICAL )
    {
        long tmp = textHeight;
        textHeight = textWidth;
        textWidth = tmp;
    }
    
    if ((align & wxALIGN_BOTTOM) != 0)
    {
        if ( textOrientation == wxHORIZONTAL )
            y = rect.y + (rect.height - textHeight - 1);
        else // wxVERTICAL
            x = rect.x + rect.width - textWidth;
    }
    else if ((align & wxALIGN_CENTRE_VERTICAL) != 0)
    {
        if ( textOrientation == wxHORIZONTAL )
            y = rect.y + (rect.height - textHeight)/2;
        else // wxVERTICAL
            x = rect.x + (rect.width - textWidth)/2;
    }
    else // wxALIGN_TOP
    {
        if ( textOrientation == wxHORIZONTAL )
            y = rect.y + 1;
        else // wxVERTICAL
            x = rect.x + 1;
    }
    
    // Align each line of a multi-line label
    for ( l = 0; l < nLines; l++ )
    {
        lineWidth  = lineWidths[l];
        lineHeight = lineHeights[l];

        if ((align & wxALIGN_RIGHT) != 0)
        {
            if ( textOrientation == wxHORIZONTAL )
                x = rect.x + (rect.width - lineWidth - 1);
            else // wxVERTICAL
                y = rect.y + lineWidth + 1;
        }
        else if ((align & wxALIGN_CENTRE_HORIZONTAL) != 0)
        {
            if ( textOrientation == wxHORIZONTAL )
                x = rect.x + (rect.width - lineWidth)/2;
            else // wxVERTICAL
                y = rect.y + rect.height - (rect.height - lineWidth)/2;
        }
        else // wxALIGN_LEFT
        {
            if ( textOrientation == wxHORIZONTAL )
                x = rect.x + 1;
            else // wxVERTICAL
                y = rect.y + rect.height - 1;
        }

        if ( textOrientation == wxHORIZONTAL )
        {
            dc.DrawText( lines[l], (int)x, (int)y );
            y += lineHeight;
        }
        else // wxVERTICAL
        {
            dc.DrawRotatedText( lines[l], (int)x, (int)y, 90.0 );
            x += lineHeight;
        }
    }
    
    dc.DestroyClippingRegion();
}

int wxSheet::StringToLines( const wxString& value, wxArrayString& lines ) const
{
    size_t len = value.Length();
    if (!len)
        return 0;
    
    const wxChar *c = value.GetData();
    size_t pos, count = 0, start_pos = 0;
    
    for (pos = 0; pos < len; pos++, c++)
    {
        if ((*c == wxT('\n') || (*c == wxT('\r'))))
        {
            if (pos == start_pos)
                lines.Add(wxEmptyString);
            else
                lines.Add(value.Mid(start_pos, pos - start_pos));
            
            start_pos = pos + 1;
            count++;
            
            // Check for DOS line endings and skip them
            if ((*c == wxT('\r')) && (pos + 1 < len) && (c[1] == wxT('\n')))
            {
                c++;
                pos++;
                start_pos++;
            }
        }
    }
    
    if ( start_pos < len )
    {
        if (start_pos == 0)
            lines.Add(value);    
        else
            lines.Add(value.Mid(start_pos));    
        
        count++;
    }
    
    return count;
}

bool wxSheet::GetTextBoxSize( wxDC& dc, const wxArrayString& lines,
                              long *width, long *height ) const
{
    long w = 0, h = 0;
    long lineW, lineH;
    size_t i, count = lines.GetCount();
    for ( i = 0; i < count; i++ )
    {
        dc.GetTextExtent( lines[i], &lineW, &lineH );
        if (w < lineW) w = lineW;
        h += lineH;
    }

    if (width)  *width  = w;
    if (height) *height = h;
    
    return ((w > 0) && (h > 0));
}

// ----------------------------------------------------------------------------
// Geometry utility functions, pixel <-> grid etc

//  Note that all of these functions work with the logical coordinates of
//  grid cells and labels so you will need to convert from device
//  coordinates for mouse events etc.

// Internal Helper function for computing row or column from some
// (unscrolled) coordinate value, using either
// m_defaultRowHeight/m_defaultColWidth or binary search on array
// of m_rowBottoms/m_ColRights to speed up the search!

static int CoordToRowOrCol(int coord, int defaultDist, int minDist,
                           const wxArrayInt& BorderArray, int nMax,
                           bool clipToMinMax)
{
    if (coord < 0)
        return clipToMinMax && (nMax > 0) ? 0 : -1;

    if (!defaultDist)
        defaultDist = 1;

    size_t borderCount = BorderArray.GetCount();
    size_t i_max = coord / defaultDist,
           i_min = 0;

    if (borderCount == 0)
    {
        if ((int) i_max < nMax)
            return i_max;
        
        return clipToMinMax ? nMax - 1 : -1;
    }

    if ( coord >= BorderArray[borderCount-1])
        return clipToMinMax ? (int)borderCount-1 : -1;
    if ( coord <= BorderArray[0] )
        return 0;
    
    if ( i_max >= borderCount )
        i_max = borderCount - 1;
    else
    {
        // try to trim down i_max to a suitable max val
        if ( coord >= BorderArray[i_max] )
        {
            i_min = i_max;
            if (minDist)
            {
                i_max = coord / minDist;
                if ( i_max >= borderCount )
                    i_max = borderCount - 1;
            }
            else
                i_max = borderCount - 1;
        }
    }
    
    int median;
    while ( i_min < i_max )  
    { 
        // no check for == since it's not very likely, at most 1 extra iteration
        median = (i_min + i_max)/2;             
        if (BorderArray[median] > coord) 
            i_max = median;      
        else              
            i_min = median + 1;  
    } 
   
    return i_min;
}

wxSheetCoords wxSheet::XYToGridCell( int x, int y, bool clipToMinMax ) const
{
    return wxSheetCoords(YToGridRow(y, clipToMinMax), XToGridCol(x, clipToMinMax));
}

int wxSheet::YToGridRow( int y, bool clipToMinMax ) const
{
    return CoordToRowOrCol(y, GetDefaultRowHeight(), GetMinimalAcceptableRowHeight(), 
                           GetSheetRefData()->m_rowBottoms, GetNumberRows(), clipToMinMax);
}

int wxSheet::XToGridCol( int x, bool clipToMinMax ) const
{
    return CoordToRowOrCol(x, GetDefaultColWidth(), GetMinimalAcceptableColWidth(), 
                           GetSheetRefData()->m_colRights, GetNumberCols(), clipToMinMax);
}

int wxSheet::YToEdgeOfGridRow( int y ) const
{
    int i = YToGridRow(y, TRUE);

    if ( GetRowHeight(i) > WXSHEET_LABEL_EDGE_ZONE )
    {
        // We know that we are in row i, test whether we are
        // close enough to lower or upper border, respectively.
        if ( abs(GetRowBottom(i) - y) < WXSHEET_LABEL_EDGE_ZONE )
            return i;
        else if ( (i > 0) && (y - GetRowTop(i) < WXSHEET_LABEL_EDGE_ZONE) )
            return i - 1;
    }

    return -1;
}
int wxSheet::XToEdgeOfGridCol( int x ) const
{
    int i = XToGridCol(x, TRUE);

    if ( GetColWidth(i) > WXSHEET_LABEL_EDGE_ZONE )
    {
        // We know that we are in column i,  test whether we are
        // close enough to right or left border, respectively.
        if ( abs(GetColRight(i) - x) < WXSHEET_LABEL_EDGE_ZONE )
            return i;
        else if ( (i > 0) && (x - GetColLeft(i) < WXSHEET_LABEL_EDGE_ZONE) )
            return i - 1;
    }

    return -1;
}

wxRect wxSheet::CellToRect( const wxSheetCoords& coords, bool getDeviceRect ) const
{
    wxCHECK_MSG(ContainsCell(coords), wxSheetNoCellRect, wxT("Invalid coords"));
    wxSheetBlock block(GetCellBlock(coords));
    wxRect rect(GetColLeft(block.GetLeft()), GetRowTop(block.GetTop()), 0, 0);
    rect.width  = GetColRight( block.GetRight() ) - rect.x; // + 1;
    rect.height = GetRowBottom(block.GetBottom()) - rect.y; // + 1;
    
    // if grid lines are enabled, then the area of the cell is a bit smaller
    if (GridLinesEnabled()) 
    {
        rect.width--;
        rect.height--;
    }
    
    if (getDeviceRect && !IsCornerLabelCell(coords))
    {
        if (IsColLabelCell(coords))
        {
            int dummy;
            CalcScrolledPosition(rect.x, 0, &rect.x, &dummy);
        }
        else if (IsRowLabelCell(coords))
        {
            int dummy;
            CalcScrolledPosition(0, rect.y, &dummy, &rect.y);
        }
        else
            return CalcScrolledRect(rect);
    }
    
    return rect;
}
wxSheetBlock wxSheet::ExpandSpannedBlock(const wxSheetBlock& block_) const
{
    if (!HasSpannedCells() || block_.IsEmpty())
        return block_;
    
    wxSheetBlock block(block_);

    size_t n, count = GetSpannedBlocks()->GetCount();
    for (n=0; n<count; n++)
    {
        const wxSheetBlock& b = GetSpannedBlocks()->GetBlock(n);
        if (block_.Intersects(b)) // use original block
            block = block.Union(b);
    }
    
    return block;    
}

wxRect wxSheet::BlockToDeviceRect( const wxSheetBlock &block_) const
{   
    wxSheetBlock block(ExpandSpannedBlock(block_));
    if (block.IsEmpty())
        return wxSheetNoCellRect;
    
    wxRect rect(GetColLeft(block.GetLeft()), GetRowTop(block.GetTop()), 0, 0);
    rect.width  = GetColRight(block.GetRight())   - rect.x + 1;
    rect.height = GetRowBottom(block.GetBottom()) - rect.y + 1;
    
    // convert to scrolled coords
    rect = CalcScrolledRect(rect);

    return rect;
}

wxSheetBlock wxSheet::GetVisibleGridCellsBlock(bool wholeCellVisible) const
{
    wxRect rect(CalcUnscrolledRect(wxRect(wxPoint(0,0), m_gridWin->GetClientSize())));
    
    int leftCol   = XToGridCol(rect.GetLeft(),   TRUE);
    int topRow    = YToGridRow(rect.GetTop(),    TRUE);
    int rightCol  = XToGridCol(rect.GetRight(),  TRUE);
    int bottomRow = YToGridRow(rect.GetBottom(), TRUE);

    if (wholeCellVisible)
    {
        if (GetColLeft(leftCol) < rect.GetLeft())
            leftCol++;
        if (GetColRight(rightCol) > rect.GetRight())
            rightCol--;
        if (GetRowTop(topRow) < rect.GetTop())
            topRow++;
        if (GetRowBottom(bottomRow) > rect.GetBottom())
            bottomRow--;
    }
    
    return wxSheetBlock(topRow, leftCol, bottomRow-topRow+1, rightCol-leftCol+1);
}

wxPoint wxSheet::AlignInRect( int align, const wxRect& rect, const wxSize& size ) const
{
    wxPoint origin(rect.x, rect.y);
    
    if ((align & wxALIGN_RIGHT) != 0)
        origin.x += rect.width - size.x - 1;
    else if ((align & wxALIGN_CENTRE_HORIZONTAL) != 0)
        origin.x += (rect.width - size.x)/2;
    //else // wxALIGN_LEFT

    if ((align & wxALIGN_BOTTOM) != 0)
        origin.y += rect.height - size.y - 1;
    else if ((align & wxALIGN_CENTRE_VERTICAL) != 0)
        origin.y += (rect.height - size.y)/2;
    //else // wxALIGN_TOP

    return origin;    
}

// ----------------------------------------------------------------------------
// Scrolling functions

wxSize wxSheet::GetGridVirtualSize(bool add_extra) const
{
    wxSize size;
    if ((GetNumberCols() > 0) && (GetNumberRows() > 0))
    {
        size.x = GetColRight(GetNumberCols()-1);
        size.y = GetRowBottom(GetNumberRows()-1);
    }
    
    if (add_extra)
    {
        size.x += GetSheetRefData()->m_extraWidth;
        size.y += GetSheetRefData()->m_extraHeight;
    }
    
    return size;
}

void wxSheet::SetGridOrigin( int x, int y, bool adjustScrollBars, bool sendEvt )
{
    if (!m_gridWin)
        return;

    // normally -1 for don't change
    if (x == -1) x = m_gridOrigin.x;
    if (y == -1) y = m_gridOrigin.y;
    
    // during OnSize gridWin isn't resized yet
    int cw, ch;
    GetClientSize( &cw, &ch );       
    cw -= GetRowLabelWidth();
    ch -= GetColLabelHeight();
    if ( m_vertScrollBar->IsShown() )
        cw -= m_vertScrollBar->GetSize().x;
    if ( m_horizScrollBar->IsShown() )
        ch -= m_horizScrollBar->GetSize().y;
    
    // Force fitting, don't allow scrolling out of bounds
    wxSize virtSize = GetGridVirtualSize();
    if ((x < 0) || (virtSize.x < cw))
        x = 0; 
    else if (x > virtSize.x-cw) 
        x = virtSize.x - cw;
    if ((y < 0) || (virtSize.y < ch))
        y = 0; 
    else if (y > virtSize.y-ch) 
        y = virtSize.y - ch;
    
    int dx = m_gridOrigin.x - x;
    int dy = m_gridOrigin.y - y;
    
    if ((dx == 0) && (dy == 0))
        return;

    m_gridOrigin.x = x;
    m_gridOrigin.y = y;
    
/*
    wxRect rect( (dx >= 0) ? 0 : cw+dx,  
                 (dy >= 0) ? 0 : ch+dy,
                 dy != 0 ? cw : abs(dx), 
                 dx != 0 ? ch : abs(dy) );  
*/    
    
    if (adjustScrollBars)
        AdjustScrollbars();
    
    // FIXME - or at least check, GTK calcs rect for you, does MSW?
    m_gridWin->ScrollWindow( dx, dy ); //, &rect );
    if (dx != 0)
        m_colLabelWin->ScrollWindow( dx, 0 ); //, &rect );
    if (dy != 0)
        m_rowLabelWin->ScrollWindow( 0, dy ); //, &rect );
   
    // Let the windows refresh before next scroll event, otherwise windows
    //  don't line up
    //wxYieldIfNeeded();
    
    if (sendEvt)
        SendEvent(wxEVT_SHEET_VIEW_CHANGED, GetGridCursorCell());
}

void wxSheet::OnScroll( wxScrollEvent &event )
{
    if (!m_gridWin)
        return;

    event.Skip();
    
    int pos = event.GetPosition();
    //wxPrintf(wxT("Pos %d %d Length %d Left %d Right %d\n"), pos, m_horizScrollBar->GetThumbPosition(), m_horizScrollBar->GetThumbSize(), pos*m_horizScrollBar->GetThumbSize(), pos*m_horizScrollBar->GetThumbSize()+GetGridWindow()->GetClientSize().x);
    
    if (event.GetId() == ID_HORIZ_SCROLLBAR)
        SetGridOrigin( pos*15, -1, FALSE, TRUE );
        //SetGridOrigin( pos*m_horizScrollBar->GetThumbSize(), -1, FALSE, TRUE );
    else if (event.GetId() == ID_VERT_SCROLLBAR)
        SetGridOrigin( -1, pos*15, FALSE, TRUE );
        //SetGridOrigin( -1, pos*m_vertScrollBar->GetThumbSize(), FALSE, TRUE );
}

void wxSheet::AdjustScrollbars(bool calc_win_sizes)
{
    if (!m_gridWin || m_resizing)
        return;
    
    m_resizing = TRUE;

    bool horizSbShown = m_horizScrollBar->IsShown();
    bool vertSbShown  = m_vertScrollBar->IsShown();
    int  sb_width  = m_vertScrollBar->GetSize().x;
    int  sb_height = m_horizScrollBar->GetSize().y;
    
    int cw, ch;
    GetClientSize( &cw, &ch );

    // Grid window width and height, may be in OnSize so not sized yet
    int gw = cw - GetRowLabelWidth();
    int gh = ch - GetColLabelHeight();
    
    // grid total size
    wxSize virtSize(GetGridVirtualSize());
    
    // take into account editor if shown // FIXME what is this?
    if ( 0 && IsCellEditControlShown() ) 
    {
        int w2, h2;
        int r = GetGridCursorRow();
        int c = GetGridCursorCol();
        int x = GetColLeft(c);
        int y = GetRowTop(r);

        // how big is the editor
        GetEditControl().GetControl()->GetSize(&w2, &h2);
        w2 += x;
        h2 += y;
        if( w2 > virtSize.x ) virtSize.x = w2;
        if( h2 > virtSize.y ) virtSize.y = h2;
    }
    
    bool need_Xscroll = (m_scrollBarMode & SB_HORIZ_NEVER) != 0 ? FALSE : 
        (((m_scrollBarMode & SB_HORIZ_ALWAYS) != 0) ? TRUE : virtSize.x > gw);
    bool need_Yscroll = (m_scrollBarMode & SB_VERT_NEVER ) != 0 ? FALSE : 
        (((m_scrollBarMode & SB_VERT_ALWAYS ) != 0) ? TRUE : virtSize.y > gh);
    
    if (need_Xscroll) gh -= sb_height;
    if (need_Yscroll) gw -= sb_width;
    
    // resized grid size for scrollbars, now that it's smaller maybe need other
    need_Xscroll = (m_scrollBarMode & SB_HORIZ_NEVER) != 0 ? FALSE : 
        (((m_scrollBarMode & SB_HORIZ_ALWAYS) != 0) ? TRUE : virtSize.x > gw);
    need_Yscroll = (m_scrollBarMode & SB_VERT_NEVER ) != 0 ? FALSE : 
        (((m_scrollBarMode & SB_VERT_ALWAYS ) != 0) ? TRUE : virtSize.y > gh);
    
    //if (need_Xscroll) ch -= sb_height;
    //if (need_Yscroll) cw -= sb_width;
    //bool horiz_splitter = need_Xscroll && m_enable_split_horiz;
    //bool vert_splitter  = need_Yscroll && m_enable_split_vert;
    
    // width and height of the horiz and vert scrollbars
    //int sw = cw - (vert_splitter  ? SPLIT_BUTTON_WIDTH : 0);
    //int sh = ch - (horiz_splitter ? SPLIT_BUTTON_WIDTH : 0);   
    
    // set scrollbar parameters
    int thumbX = SHEET_SCROLL_LINE_X;
    int thumbY = SHEET_SCROLL_LINE_Y;

    //virtSize.x += sw - gw;
    //virtSize.y += sh - gh;
    
    // FIXME this is wrong for GTK and MSW, but why?
    int rangeX = !need_Xscroll ? 0 : 1 + (virtSize.x - gw + thumbX - 1)/thumbX;
    int rangeY = !need_Yscroll ? 0 : 1 + (virtSize.y - gh + thumbY - 1)/thumbY;
    
    int pageX = int((gw * 0.9) / thumbX);
    int pageY = int((gh * 0.9) / thumbY);

    int posX = m_gridOrigin.x/thumbX;
    int posY = m_gridOrigin.y/thumbY;

    thumbX = 1;
    thumbY = 1;

    //wxPrintf(wxT("Pos %d %d, virtSize %d %d, range %d %d, thumb %d %d page %d %d, win %d %d \n"), 
    //    posX, posY, virtSize.x, virtSize.y, rangeX, rangeY, thumbX, thumbY, pageX, pageY, gw, gh);
    
    if (need_Xscroll)
        m_horizScrollBar->SetScrollbar(posX, thumbX, rangeX, pageX);
    if (need_Yscroll)
        m_vertScrollBar->SetScrollbar(posY, thumbY, rangeY, pageY);
    
    //wxPrintf(wxT("Set pos %d range %d, thumb %d, page %d\n"), 
    //    m_horizScrollBar->GetThumbPosition(), m_horizScrollBar->GetRange(), m_horizScrollBar->GetThumbSize(), m_horizScrollBar->GetPageSize());
    
    bool calcSizes = FALSE;
    
    if (need_Xscroll != horizSbShown)
    {
        calcSizes = TRUE;
        m_horizScrollBar->Show(need_Xscroll);
    }
    if (need_Yscroll != vertSbShown)
    {
        calcSizes = TRUE;
        m_vertScrollBar->Show(need_Yscroll);
    }

    m_resizing = FALSE;
    
    if (calcSizes && calc_win_sizes)
        CalcWindowSizes(FALSE);
}

void wxSheet::PrepareGridDC( wxDC& dc )
{
    dc.SetDeviceOrigin( -m_gridOrigin.x, -m_gridOrigin.y );
}

void wxSheet::PrepareRowLabelDC( wxDC& dc )
{
    dc.SetDeviceOrigin( 0, -m_gridOrigin.y );
}

void wxSheet::PrepareColLabelDC( wxDC& dc )
{
    dc.SetDeviceOrigin( -m_gridOrigin.x, 0 );
}

// ----------------------------------------------------------------------------

bool wxSheet::HasFocus() const
{
    wxWindow *win = FindFocus();
    return win && ((win==(wxSheet*)this) || (win==m_gridWin) || (win==m_rowLabelWin) ||
           (win == m_colLabelWin) || (win == m_cornerLabelWin));
}

// ----------------------------------------------------------------------------
// Event handlers

void wxSheet::ProcessRowLabelMouseEvent( wxMouseEvent& event )
{
    int x, y;
    m_mousePos = event.GetPosition();
    CalcUnscrolledPosition( m_mousePos.x, m_mousePos.y, &x, &y );
    wxSheetCoords coords(YToGridRow(y), -1);

    if ( event.Entering() || event.Leaving() )
    {
        if (!event.Dragging() && !m_isDragging)
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_rowLabelWin);
        
        return;
    }
    
    if ( event.Dragging() && event.LeftIsDown() )
    {
        if (!m_isDragging)
        {
            m_isDragging = TRUE;
            SetCaptureWindow(m_rowLabelWin);
        }

        if ( event.LeftIsDown() )
        {
            bool can_scroll = TRUE;
            
            switch( m_mouseCursorMode )
            {
                case WXSHEET_CURSOR_RESIZE_ROW:
                {
                    y = wxMax( y, GetRowTop(m_dragRowOrCol) + 
                                  GetMinimalRowHeight(m_dragRowOrCol));
                    can_scroll = FALSE; // y != m_dragLastPos;
                    DrawRowColResizingMarker( y );
                    break;
                }
                case WXSHEET_CURSOR_SELECT_ROW:
                {
                    // check for clearing here since we didn't if editing allowed
                    bool add = event.ShiftDown() || event.ControlDown();
                    if (HasSelection(FALSE) && !add)
                        ClearSelection(TRUE);

                    if (ContainsGridCell(GetSelectingAnchor()) && ContainsRowLabelCell(coords))
                    {
                        HighlightSelectingBlock(GetSelectingAnchor(), 
                                                wxSheetCoords(coords.m_row, GetNumberCols()+1));
                    }
                    break;
                }
                default: 
                    break;
            }
            
            if (can_scroll && 
                ((m_mousePos.y < 0) || (m_mousePos.y > m_rowLabelWin->GetClientSize().GetHeight())))
            {
                if (!m_mouseTimer)
                    StartMouseTimer();
            }
            else
                StopMouseTimer();
        }
        return;
    }

    StopMouseTimer();        
    SetCaptureWindow(NULL);
    m_isDragging = FALSE;

    if ( event.LeftDown() )
    {
        if (IsCellEditControlShown())
            DisableCellEditControl(TRUE);
        
        // don't send a label click event for a hit on the edge of the row label
        // this is probably the user wanting to resize the row
        if ( YToEdgeOfGridRow(y) < 0 )
        {
            if ( (GetSelectionMode() != wxSHEET_SelectCols) &&
                 ContainsRowLabelCell(coords)  &&
                 (SendEvent(wxEVT_SHEET_LABEL_LEFT_DOWN, coords, &event) == EVT_SKIPPED))
            {
                bool add = event.ShiftDown() || event.ControlDown();
                wxSheetBlock block;
                
                if ( event.ShiftDown() )
                {
                    SetSelectingAnchor(wxSheetCoords(GetGridCursorRow(), 0));
                    block = wxSheetBlock(GetSelectingAnchor(), 
                                         wxSheetCoords(coords.m_row, GetNumberCols()+1));
                }
                else
                {
                    SetSelectingAnchor(wxSheetCoords(coords.m_row, 0));
                    block = wxSheetBlock(GetSelectingAnchor(), 1, GetNumberCols()+1);
                }
                    
                // if you can edit the row label then don't select row until drag
                bool can_edit = CanEnableCellControl(coords);
                if (!add && HasSelection() && 
                    (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING,
                                   block, FALSE, FALSE, &event) != EVT_VETOED))
                {   
                    ClearSelection(TRUE);
                }
                
                if (!can_edit)
                    HighlightSelectingBlock(block);

                SetMouseCursorMode(WXSHEET_CURSOR_SELECT_ROW, m_rowLabelWin);
            }
        }
        else
        {
            // starting to drag-resize a row
            if ( CanDragRowSize() )
            {
                SetMouseCursorMode(WXSHEET_CURSOR_RESIZE_ROW, m_rowLabelWin);
                SetCaptureWindow(m_rowLabelWin);
            }
        }
    }
    else if ( event.LeftDClick() )
    {
        int row = YToEdgeOfGridRow(y);
        if ( row < 0 )
        {
            if ( ContainsRowLabelCell(coords) &&
                 (SendEvent(wxEVT_SHEET_LABEL_LEFT_DCLICK, coords, &event) == EVT_SKIPPED))
            {
                if (CanEnableCellControl(coords))
                {
                    m_waitForSlowClick = FALSE;
                    ClearSelection(TRUE);
                    EnableCellEditControl(coords);
                    if ( IsCellEditControlCreated() )
                        GetSheetRefData()->m_cellEditor.StartingClick();
                }
            }
        }
        else
        {
            // adjust row height depending on label text
            if (CanDragRowSize())
                AutoSizeRowLabelHeight( row );
            
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_colLabelWin);
            m_dragLastPos  = -1;
        }
    }
    else if ( event.LeftUp() )
    {
        StopMouseTimer();
        SetCaptureWindow(NULL);

        if ( m_mouseCursorMode == WXSHEET_CURSOR_RESIZE_ROW )
        {
            // Note: send event *after* doing default processing in this case
            if (DoEndDragResizeRowCol())
                SendEvent( wxEVT_SHEET_ROW_SIZE, wxSheetCoords(m_dragRowOrCol, -1), &event );
        }
        else if ( m_mouseCursorMode == WXSHEET_CURSOR_SELECT_ROW )
        {
            if (!GetSelectingBlock().IsEmpty())
            {                
                bool add = event.ShiftDown() || event.ControlDown();

                if (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                             GetSelectingBlock(), TRUE, add, &event) != EVT_VETOED)
                {   
                    SelectRows(GetSelectingBlock().GetTop(), GetSelectingBlock().GetBottom(), add, TRUE);
                    SetSelectingBlock(wxNullSheetBlock);
                }
            }
        }

        SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_rowLabelWin);
        m_dragLastPos  = -1;
        if (!m_keySelecting)
            SetSelectingAnchor(wxNullSheetCoords);
        
        // send after default processing, they can use own evt handler 
        SendEvent(wxEVT_SHEET_LABEL_LEFT_UP, coords, &event);
    }
    else if ( event.RightDown() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);
        
        if ( ContainsRowLabelCell(coords) &&
	        (SendEvent(wxEVT_SHEET_LABEL_RIGHT_DOWN, coords, &event) == EVT_SKIPPED))
        {
            // no default action at the moment
        }
    }
    else if ( event.RightDClick() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);
        
        if ( ContainsRowLabelCell(coords) &&
	        (SendEvent(wxEVT_SHEET_LABEL_RIGHT_DCLICK, coords, &event) == EVT_SKIPPED))
        {
            // no default action at the moment
        }
    }
    else if ( event.RightUp() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);
        
        if ( ContainsRowLabelCell(coords) &&
	        (SendEvent(wxEVT_SHEET_LABEL_RIGHT_UP, coords, &event) == EVT_SKIPPED))
        {
            // no default action at the moment
        }
    }
    else if ( event.Moving() )
    {
        m_dragRowOrCol = YToEdgeOfGridRow( y );
        if ( m_dragRowOrCol >= 0 )
        {
            if ( m_mouseCursorMode == WXSHEET_CURSOR_SELECT_CELL )
            {
                // don't capture the mouse yet
                if ( CanDragRowSize() )
                    SetMouseCursorMode(WXSHEET_CURSOR_RESIZE_ROW, m_rowLabelWin);
            }
        }
        else if ( m_mouseCursorMode != WXSHEET_CURSOR_SELECT_CELL )
        {
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_rowLabelWin);
        }
    }
}

void wxSheet::ProcessColLabelMouseEvent( wxMouseEvent& event )
{
    int x, y;
    m_mousePos = event.GetPosition();
    CalcUnscrolledPosition( m_mousePos.x, m_mousePos.y, &x, &y );
    wxSheetCoords coords(-1, XToGridCol(x));

    if ( event.Entering() || event.Leaving() )
    {
        if (!event.Dragging() && !m_isDragging)
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_colLabelWin);
        
        return;
    }
    
    if ( event.Dragging() && event.LeftIsDown() )
    {
        if (!m_isDragging)
        {
            m_isDragging = TRUE;
            SetCaptureWindow(m_colLabelWin);
        }

        if ( event.LeftIsDown() )
        {
            bool can_scroll = TRUE;
            
            switch( m_mouseCursorMode )
            {
                case WXSHEET_CURSOR_RESIZE_COL :
                {
                    x = wxMax( x, GetColLeft(m_dragRowOrCol) +
                                  GetMinimalColWidth(m_dragRowOrCol));
                    can_scroll = FALSE; // x != m_dragLastPos;
                    DrawRowColResizingMarker( x );
                    break;
                }
                case WXSHEET_CURSOR_SELECT_COL :
                {
                    // check for clearing here since we didn't if editing allowed
                    bool add = event.ShiftDown() || event.ControlDown();
                    if (HasSelection(FALSE) && !add)
                        ClearSelection(TRUE);

                    if (ContainsGridCell(GetSelectingAnchor()) && ContainsColLabelCell(coords))
                    {
                        HighlightSelectingBlock(GetSelectingAnchor(), 
                                                wxSheetCoords(GetNumberRows()+1, coords.m_col));
                    }
                    break;
                }
                default:
                    break;
            }

            if (can_scroll &&
                ((m_mousePos.x < 0) || (m_mousePos.y > m_colLabelWin->GetClientSize().GetWidth())))
            {
                if (!m_mouseTimer)
                    StartMouseTimer();
            }
            else
                StopMouseTimer();
        }
        return;
    }

    StopMouseTimer();        
    SetCaptureWindow(NULL);
    m_isDragging = FALSE;

    if ( event.LeftDown() )
    {
        if (IsCellEditControlShown())
            DisableCellEditControl(TRUE);

        // don't send a label click event for a hit on the edge of the col label
        //  this is probably the user wanting to resize the col
        if ( XToEdgeOfGridCol(x) < 0 )
        {
            if ( (GetSelectionMode() != wxSHEET_SelectRows) &&
                 ContainsColLabelCell(coords) &&
                 (SendEvent(wxEVT_SHEET_LABEL_LEFT_DOWN, coords, &event) == EVT_SKIPPED))
            {
                bool add = event.ShiftDown() || event.ControlDown();
                wxSheetBlock block;
                
                if ( event.ShiftDown() )
                {
                    SetSelectingAnchor(wxSheetCoords(0, GetGridCursorCol()));
                    block = wxSheetBlock(GetSelectingAnchor(), 
                                         wxSheetCoords(GetNumberRows()+1, coords.m_col));
                }
                else
                {
                    SetSelectingAnchor(wxSheetCoords(0, coords.m_col));
                    block = wxSheetBlock(GetSelectingAnchor(), GetNumberRows()+1, 1);
                }

                // if you can edit the row label then don't select row
                bool can_edit = CanEnableCellControl(coords);
                if (!add && HasSelection() && 
                    (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING,
                                   block, FALSE, FALSE, &event) != EVT_VETOED))
                {   
                    ClearSelection(TRUE);
                }
                
                if (!can_edit)
                    HighlightSelectingBlock(block);
                
                SetMouseCursorMode(WXSHEET_CURSOR_SELECT_COL, m_colLabelWin);
            }
        }
        else
        {
            // starting to drag-resize a col
            if ( CanDragColSize() )
            {
                SetMouseCursorMode(WXSHEET_CURSOR_RESIZE_COL, m_colLabelWin);
                SetCaptureWindow(m_colLabelWin);
            }
        }
    }

    if ( event.LeftDClick() )
    {
        int col = XToEdgeOfGridCol(x);
        if ( col < 0 )
        {
            if ( ContainsColLabelCell(coords) &&
                 (SendEvent(wxEVT_SHEET_LABEL_LEFT_DCLICK, coords, &event) == EVT_SKIPPED))
            {
                if (CanEnableCellControl(coords))
                {
                    m_waitForSlowClick = FALSE;
                    ClearSelection(TRUE);
                    EnableCellEditControl(coords);
                    if ( IsCellEditControlCreated() )
                        GetSheetRefData()->m_cellEditor.StartingClick();
                }
            }
        }
        else
        {
            // adjust column width depending on label text
            if (CanDragColSize())
                AutoSizeColLabelWidth( col );
            
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_colLabelWin);
            m_dragLastPos  = -1;
        }
    }
    else if ( event.LeftUp() )
    {
        StopMouseTimer();
        SetCaptureWindow(NULL);
        
        if ( m_mouseCursorMode == WXSHEET_CURSOR_RESIZE_COL )
        {
            // Note: send event *after* doing default processing in this case
            if (DoEndDragResizeRowCol())
                SendEvent( wxEVT_SHEET_COL_SIZE, wxSheetCoords(-1, m_dragRowOrCol), &event );
        }
        else if ( m_mouseCursorMode == WXSHEET_CURSOR_SELECT_COL )
        {
            if (!GetSelectingBlock().IsEmpty())
            {
                bool add = event.ShiftDown() || event.ControlDown();

                if (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                             GetSelectingBlock(), TRUE, add, &event) != EVT_VETOED)
                {   
                    SelectCols(GetSelectingBlock().GetLeft(), GetSelectingBlock().GetRight(), add, TRUE);
                    SetSelectingBlock(wxNullSheetBlock);
                }
            }
        }
        
        SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_colLabelWin);
        m_dragLastPos  = -1;
        if (!m_keySelecting)
            SetSelectingAnchor(wxNullSheetCoords);
        
        // send after default processing, they can use own evt handler 
        SendEvent(wxEVT_SHEET_LABEL_LEFT_UP, coords, &event);
    }
    else if ( event.RightDown() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);
        
        if (ContainsColLabelCell(coords) && 
            (SendEvent(wxEVT_SHEET_LABEL_RIGHT_DOWN, coords, &event) == EVT_SKIPPED))
        {
            // no default action at the moment
        }
    }
    else if ( event.RightDClick() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);
        
        if (ContainsColLabelCell(coords) && 
            (SendEvent(wxEVT_SHEET_LABEL_RIGHT_DCLICK, coords, &event) == EVT_SKIPPED))
        {
            // no default action at the moment
        }
    }
    else if ( event.RightUp() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);
        
        if (ContainsColLabelCell(coords) && 
            (SendEvent(wxEVT_SHEET_LABEL_RIGHT_UP, coords, &event) == EVT_SKIPPED))
        {
            // no default action at the moment
        }
    }
    else if ( event.Moving() )
    {
        m_dragRowOrCol = XToEdgeOfGridCol( x );
        if ( m_dragRowOrCol >= 0 )
        {
            if ( m_mouseCursorMode == WXSHEET_CURSOR_SELECT_CELL )
            {
                // don't capture the cursor yet
                if ( CanDragColSize() )
                    SetMouseCursorMode(WXSHEET_CURSOR_RESIZE_COL, m_colLabelWin);
            }
        }
        else if ( m_mouseCursorMode != WXSHEET_CURSOR_SELECT_CELL )
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_colLabelWin);
    }
}

void wxSheet::ProcessCornerLabelMouseEvent( wxMouseEvent& event )
{
    wxSheetCoords coords(-1,-1);
    
    if ( event.Dragging() )
    {
        if ( event.LeftIsDown() )
        {
            if ((GetNumberRows() > 0) && (GetNumberCols() > 0))
            {
                SetSelectingAnchor(wxSheetCoords(0, 0));
                HighlightSelectingBlock(GetSelectingAnchor(), 
                                        wxSheetCoords(GetNumberRows()+1, GetNumberCols()+1));
            }
        }
        return;
    }
    
    if ( event.LeftDown() )
    {
        SetSelectingBlock(wxNullSheetBlock);
        
        if ( SendEvent(wxEVT_SHEET_LABEL_LEFT_DOWN, coords, &event) == EVT_SKIPPED)
        {
            wxSheetBlock block(0, 0, GetNumberRows()+1, GetNumberCols()+1);
            
            // if you can edit the corner label then don't select everything
            bool can_edit = CanEnableCellControl(coords);
            if (!can_edit && (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                               block, TRUE, FALSE, &event) != EVT_VETOED))
            {   
                SelectAll(TRUE);
            }   
        }
    }
    else if ( event.LeftDClick() )
    {
        if (SendEvent(wxEVT_SHEET_LABEL_LEFT_DCLICK, coords, &event) == EVT_SKIPPED)
        {
            if (CanEnableCellControl(coords))
            {
                ClearSelection(TRUE);
                EnableCellEditControl(coords);
                if ( IsCellEditControlCreated() )
                    GetSheetRefData()->m_cellEditor.StartingClick();
                
                m_waitForSlowClick = FALSE;
            }
        }
    }
    else if ( event.LeftUp() )
    {
        if (!GetSelectingBlock().IsEmpty())
        {
            if (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                           GetSelectingBlock(), TRUE, FALSE, &event) != EVT_VETOED)
            {   
                SelectAll(TRUE);
                SetSelectingBlock(wxNullSheetBlock);
            }
        }
        
        // send after default processing, they can use own evt handler 
        SendEvent(wxEVT_SHEET_LABEL_LEFT_UP, coords, &event);
    }
    else if ( event.RightDown() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);
        
        if (SendEvent(wxEVT_SHEET_LABEL_RIGHT_DOWN, coords, &event) == EVT_SKIPPED)
        {
            // no default action at the moment
        }
    }
    else if ( event.RightDClick() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);
        
        if (SendEvent(wxEVT_SHEET_LABEL_RIGHT_DCLICK, coords, &event) == EVT_SKIPPED)
        {
            // no default action at the moment
        }
    }
    else if ( event.RightUp() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);
        
        if (SendEvent(wxEVT_SHEET_LABEL_RIGHT_UP, coords, &event) == EVT_SKIPPED)
        {
            // no default action at the moment
        }
    }
}

void wxSheet::ProcessGridCellMouseEvent( wxMouseEvent& event )
{
    int x, y;
    m_mousePos = event.GetPosition();
    CalcUnscrolledPosition( m_mousePos.x, m_mousePos.y, &x, &y );
    wxSheetCoords coords(XYToGridCell( x, y ));
    
    //wxPrintf("Mouse %d %d, %d %d\n", x, y, m_mousePos.x, m_mousePos.y);

    // VZ: if we do this, the mode is reset to WXSHEET_CURSOR_SELECT_CELL
    //     immediately after it becomes WXSHEET_CURSOR_RESIZE_ROW/COL under wxGTK
    if ( event.Entering() || event.Leaving() )
    {
        //SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_gridWin);
        //m_gridWin->SetCursor( *wxSTANDARD_CURSOR );
        return;
    }
    
    if ( event.Dragging() && event.LeftIsDown() )
    {
        //wxLogDebug("pos(%d, %d) coords(%d, %d)", pos.x, pos.y, coords.GetRow(), coords.GetCol());

        // Don't start doing anything until the mouse has been dragged at
        // least 3 pixels in any direction...
        if (!m_isDragging)
        {
            SetCaptureWindow(m_gridWin);
            
            if (m_startDragPos == wxDefaultPosition)
            {
                m_startDragPos = m_mousePos;
                return;
            }
            if ((abs(m_startDragPos.x - m_mousePos.x) < 4) && 
                (abs(m_startDragPos.y - m_mousePos.y) < 4))
                return;
        }

        bool can_scroll = TRUE;
        m_isDragging = TRUE;
        if ( m_mouseCursorMode == WXSHEET_CURSOR_SELECT_CELL )
        {
            // Hide the edit control, so it won't interfer with drag-shrinking.
            if ( IsCellEditControlShown() )
                DisableCellEditControl(TRUE);

            if ( ContainsGridCell(coords) )
            {
                HighlightSelectingBlock( GetSelectingAnchor(), coords );
            }
        }
        else if ( m_mouseCursorMode == WXSHEET_CURSOR_RESIZE_ROW )
        {
            y = wxMax( y, GetRowTop(m_dragRowOrCol) +
                          GetMinimalRowHeight(m_dragRowOrCol) );
            can_scroll = FALSE; // y != m_dragLastPos;
            DrawRowColResizingMarker( y );
        }
        else if ( m_mouseCursorMode == WXSHEET_CURSOR_RESIZE_COL )
        {
            x = wxMax( x, GetColLeft(m_dragRowOrCol) +
                          GetMinimalColWidth(m_dragRowOrCol));
            can_scroll = FALSE; // x != m_dragLastPos;
            DrawRowColResizingMarker( x );
        }

        if (can_scroll && 
            !wxRect(wxPoint(0,0), m_gridWin->GetClientSize()).Inside(m_mousePos))
        {
            if (!m_mouseTimer)
                StartMouseTimer();
        }
        else
            StopMouseTimer();
        
        return;
    }

    StopMouseTimer();
    SetCaptureWindow(NULL);
    m_isDragging = FALSE;
    m_startDragPos = wxDefaultPosition;

    if ( event.LeftDown() && ContainsGridCell(coords) )
    {        
        if ( SendEvent(wxEVT_SHEET_CELL_LEFT_DOWN, coords, &event) == EVT_SKIPPED)
        {
            if ( !event.ControlDown() && HasSelection() )
            {
                wxSheetBlock block(0, 0, GetNumberRows()-1, GetNumberCols()-1);
                 
                if (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                                   block, FALSE, FALSE, &event) != EVT_VETOED)
                {   
                    ClearSelection(TRUE);
                }
            }
            
            if ( event.ShiftDown() )
            {
                wxSheetBlock block(GetGridCursorCell(), coords);
                
                if (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                                    block, TRUE, TRUE, &event) != EVT_VETOED)
                {   
                    SelectBlock(block, event.ControlDown(), TRUE);
                }
            }
            else if ( (XToEdgeOfGridCol(x) < 0) && (YToEdgeOfGridRow(y) < 0) )
            {
                if (IsCellEditControlCreated())
                    DisableCellEditControl(TRUE);
                
                MakeCellVisible( coords );

                if ( !ContainsGridCell(GetSelectingAnchor()) )
                    SetSelectingAnchor(coords);

                if ( event.ControlDown() )
                {
                    // toggle cell selection
                    int sel = IsCellSelected(coords);
                    wxSheetBlock block(coords, 1, 1);
                    if (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                                       block, !sel, !sel, &event) != EVT_VETOED)
                    {   
                        ToggleCellSelection(coords, !sel, TRUE);
                        SetSelectingBlock(wxNullSheetBlock);
                    }
                }
                else
                {
                    if (GetGridCursorCell() == coords)
                        m_waitForSlowClick = TRUE;

                    SetGridCursorCell( coords );
                    // FIXME weird? Highlight a whole row/col when not in select cells?
                    if ( GetSelectionMode() != wxSHEET_SelectCells )
                        HighlightSelectingBlock( coords, coords );
                }
            }
        }
    }
    else if ( event.LeftDClick() && ContainsGridCell(coords) )
    {
        if ((XToEdgeOfGridCol(x) < 0) && (YToEdgeOfGridRow(y) < 0) &&
            (SendEvent(wxEVT_SHEET_CELL_LEFT_DCLICK, coords, &event) == EVT_SKIPPED))
        {
            if ((coords == GetGridCursorCell()) && CanEnableCellControl(GetGridCursorCell()))
            {
                ClearSelection(TRUE);
                EnableCellEditControl(GetGridCursorCell());
                if ( IsCellEditControlCreated() )
                    GetSheetRefData()->m_cellEditor.StartingClick();

                m_waitForSlowClick = FALSE;
            }
        }
    }
    else if ( event.LeftUp() )
    {
        StopMouseTimer();
        SetCaptureWindow(NULL);

        if ( m_mouseCursorMode == WXSHEET_CURSOR_SELECT_CELL )
        {
            if ((coords == GetGridCursorCell()) && m_waitForSlowClick && 
                CanEnableCellControl(GetGridCursorCell()))
            {
                ClearSelection(TRUE);
                EnableCellEditControl(GetGridCursorCell());
                if ( IsCellEditControlCreated() )
                    GetSheetRefData()->m_cellEditor.StartingClick();

                m_waitForSlowClick = FALSE;
            }
            else if ( !GetSelectingBlock().IsEmpty() )
            {
                if (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                                   GetSelectingBlock(), TRUE, TRUE, &event) != EVT_VETOED)
                {   
                    SelectBlock(GetSelectingBlock(), TRUE, TRUE);
                    SetSelectingBlock(wxNullSheetBlock);
                }
            }
        }
        else if ( m_mouseCursorMode == WXSHEET_CURSOR_RESIZE_ROW )
        {
            // Note: send event *after* doing default processing in this case
            if (DoEndDragResizeRowCol())
                SendEvent( wxEVT_SHEET_ROW_SIZE, wxSheetCoords(m_dragRowOrCol, -1), &event );
            
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_gridWin);
        }
        else if ( m_mouseCursorMode == WXSHEET_CURSOR_RESIZE_COL )
        {
            // Note: send event *after* doing default processing in this case
            if (DoEndDragResizeRowCol())
                SendEvent( wxEVT_SHEET_COL_SIZE, wxSheetCoords(-1, m_dragRowOrCol), &event );
            
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_gridWin);
        }

        // Show edit control, if it has been hidden for drag-shrinking.
        if (IsCellEditControlCreated() && !IsCellEditControlShown())
            ShowCellEditControl();
        
        if (!m_keySelecting)
            SetSelectingAnchor(wxNullSheetCoords);

        m_dragLastPos = -1;
        
        // send after default processing, they can use own evt handler 
        SendEvent(wxEVT_SHEET_CELL_LEFT_UP, coords, &event);
    }
    else if ( event.RightDown() && ContainsGridCell(coords) )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);
        
        if (SendEvent(wxEVT_SHEET_CELL_RIGHT_DOWN, coords, &event) == EVT_SKIPPED)
        {
            // no default action at the moment
        }
    }
    else if ( event.RightDClick() && ContainsGridCell(coords) )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);
        
        if (SendEvent(wxEVT_SHEET_CELL_RIGHT_DCLICK, coords, &event) == EVT_SKIPPED)
        {
            // no default action at the moment
        }
    }
    else if ( event.RightUp() && ContainsGridCell(coords) )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(TRUE);
        
        if (SendEvent(wxEVT_SHEET_CELL_RIGHT_UP, coords, &event) == EVT_SKIPPED)
        {
            // no default action at the moment
        }
    }
    else if ( event.Moving() && !event.IsButton() )
    {
        if ( (coords.GetRow() < 0) || (coords.GetCol() < 0) )
        {
            // out of grid cell area
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_gridWin);
            return;
        }

        int dragRow = YToEdgeOfGridRow( y );
        int dragCol = XToEdgeOfGridCol( x );

        // Dragging on the corner of a cell to resize in both
        // directions is not implemented yet...
        if ( (dragRow >= 0) && (dragCol >= 0) )
        {
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_gridWin);
            return;
        }

        if ( dragRow >= 0 )
        {
            m_dragRowOrCol = dragRow;

            if ( m_mouseCursorMode == WXSHEET_CURSOR_SELECT_CELL )
            {
                if ( CanDragRowSize() && CanDragGridSize() )
                {
                    SetMouseCursorMode(WXSHEET_CURSOR_RESIZE_ROW, m_gridWin);
                    SetCaptureWindow(m_gridWin);
                }
            }

            if ( dragCol >= 0 )
                m_dragRowOrCol = dragCol;

            return;
        }

        if ( dragCol >= 0 )
        {
            m_dragRowOrCol = dragCol;

            if ( m_mouseCursorMode == WXSHEET_CURSOR_SELECT_CELL )
            {
                if ( CanDragColSize() && CanDragGridSize() )
                {
                    SetMouseCursorMode(WXSHEET_CURSOR_RESIZE_COL, m_gridWin);
                    SetCaptureWindow(m_gridWin);
                }
            }

            return;
        }

        // Neither on a row or col edge
        if ( m_mouseCursorMode != WXSHEET_CURSOR_SELECT_CELL )
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_gridWin);
    }
}

void wxSheet::OnKeyDown( wxKeyEvent& event )
{
    //wxCHECK_RET(!m_inOnKeyDown, wxT("wxSheet::OnKeyDown called while already active"));
    // yield called from SetGridOrigin causes this in MSW
    
    if (m_inOnKeyDown)
        return;

    m_inOnKeyDown = TRUE;
    
    // FIXME - is this really a good idea? probably not, should make own "key" event
    // propagate the event up and see if it gets processed
    //wxWindow *parent = GetParent();
    //wxKeyEvent keyEvt( event );
    //keyEvt.SetEventObject( parent );
    //!parent->GetEventHandler()->ProcessEvent(keyEvt) &&
    
    if ( GetNumberRows() && GetNumberCols() )
    {
        // try local handlers
        switch ( event.GetKeyCode() )
        {
            case WXK_RETURN:
            case WXK_NUMPAD_ENTER:
            {
                if ( event.ControlDown() )
                    event.Skip();  // to let the edit control have the return
                else
                {
                    if ( IsCellEditControlCreated() )
                        DisableCellEditControl(TRUE);
                    else if ( GetGridCursorRow() < GetNumberRows()-1 )
                        MoveCursorDown( event.ShiftDown() );
                }
                break;
            }
            case WXK_ESCAPE:
            {
                ClearSelection();
                break;
            }
            case WXK_TAB:
            {
                if (event.ShiftDown())
                {
                    if ( IsCellEditControlCreated() )
                        DisableCellEditControl(TRUE);   
                    else if ( GetGridCursorCol() > 0 )
                        MoveCursorLeft( FALSE );
                }
                else
                {
                    if ( IsCellEditControlCreated() )
                        DisableCellEditControl(TRUE);
                    else if ( GetGridCursorCol() < GetNumberCols()-1 )
                        MoveCursorRight( FALSE );
                }
                break;
            }
            case WXK_SPACE:
            {
                if ( event.ShiftDown() && ContainsGridCell(GetGridCursorCell()) )
                {
                    wxSheetBlock block(GetGridCursorRow(), 0, 1, GetNumberCols()+1);
                    
                    if (!GetSelectingBlock().IsEmpty() && 
                        GetSelectingBlock().Contains(GetGridCursorCell()))
                    {
                        block.SetTop(GetSelectingBlock().GetTop());
                        block.SetBottom(GetSelectingBlock().GetBottom());
                    }
                    
                    if (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                                       block, TRUE, FALSE, &event) != EVT_VETOED)
                    {   
                        SelectRows(block.GetTop(), block.GetBottom(), FALSE, TRUE);
                    }
                    
                    break;
                }
                if ( event.ControlDown() && ContainsGridCell(GetGridCursorCell()) )
                {
                    wxSheetBlock block(0, GetGridCursorCol(), GetNumberRows()+1, 1);
                    
                    if (!GetSelectingBlock().IsEmpty() && 
                        GetSelectingBlock().Contains(GetGridCursorCell()))
                    {
                        block.SetLeft(GetSelectingBlock().GetLeft());
                        block.SetRight(GetSelectingBlock().GetRight());
                    }
                    
                    if (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                                       block, TRUE, FALSE, &event) != EVT_VETOED)
                    {   
                        SelectCols(block.GetLeft(), block.GetRight(), FALSE, TRUE);
                    }
                    
                    break;
                }
                if ( event.AltDown() && ContainsGridCell(GetGridCursorCell()) )
                {                    
                    // Toggle cell selection FIXME not sure how useful this is
                    bool sel = IsCellSelected(GetGridCursorCell());
                    wxSheetBlock block(GetGridCursorCell(), 1, 1);
                    if (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                                       block, !sel, !sel, &event) != EVT_VETOED)
                    {   
                        ToggleCellSelection(GetGridCursorCell(), !sel, TRUE);
                    }
                    
                    break;
                }
                if ( !IsEditable() )
                {
                    MoveCursorRight( FALSE );
                    break;
                }
                // Otherwise fall through to default
            }
            default:
            {
                // is it possible to edit the current cell at all?
                if ( !IsCellEditControlCreated() && CanEnableCellControl(GetGridCursorCell()) )
                {
                    // yes, now check whether the cells editor accepts the key
                    wxSheetCellEditor editor(GetAttr(GetGridCursorCell()).GetEditor(this, GetGridCursorCell()));

                    // <F2> is special and will always start editing, for
                    // other keys - ask the editor itself
                    if ( ((event.GetKeyCode() == WXK_F2) && !event.HasModifiers())
                         || editor.IsAcceptedKey(event) )
                    {
                        // ensure cell is visble
                        MakeCellVisible(GetGridCursorCell());
                        EnableCellEditControl(GetGridCursorCell());

                        // a problem can arise if the cell is not completely
                        // visible (even after calling MakeCellVisible the
                        // control is not created and calling StartingKey will
                        // crash the app
                        if ( IsCellEditControlCreated() )
                            GetSheetRefData()->m_cellEditor.StartingKey(event);
                    }
                    else
                        event.Skip();
                }
                else
                {
                    // let others process char events with modifiers or all
                    // char events for readonly cells
                    event.Skip();
                }
                break;
            }
        }
    }

    m_inOnKeyDown = FALSE;
}

void wxSheet::OnKeyUp( wxKeyEvent& event )
{
    if ( m_keySelecting && (event.GetKeyCode() == WXK_SHIFT) )
    {
        if ( !GetSelectingBlock().IsEmpty() )
        {
            if (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                            GetSelectingBlock(), TRUE, TRUE, &event) != EVT_VETOED)
            {   
                SelectBlock(GetSelectingBlock(), TRUE, TRUE);
                SetSelectingBlock(wxNullSheetBlock);
            }
            
            SetSelectingAnchor(wxNullSheetCoords);
        }

        m_keySelecting = FALSE;
    }
    
    event.Skip();
}

void wxSheet::OnChar( wxKeyEvent& event )
{
    //wxCHECK_RET(!m_inOnKeyDown, wxT("wxSheet::OnKeyDown called while already active"));
    // yield called from SetGridOrigin causes this in MSW
    if (m_inOnKeyDown)
        return;

    if ( !GetNumberRows() || !GetNumberCols() )
    {
        event.Skip();
        return;
    }

    m_inOnKeyDown = TRUE;
    int keyMods = GetKeyModifiers(&event);
    
    // try local handlers
    switch ( event.GetKeyCode() )
    {
        case WXK_UP:
        {
            if (!ContainsGridCell(GetGridCursorCell())) break;
            if ( event.ControlDown() )
                MoveCursorUpBlock( event.ShiftDown() );
            else if ( keyMods == ALT_DOWN )
                SetRowHeight(GetGridCursorRow(), GetRowHeight(GetGridCursorRow())-5);
            else
                MoveCursorUp( event.ShiftDown() );
            
            break;
        }
        case WXK_DOWN:
        {
            if (!ContainsGridCell(GetGridCursorCell())) break;
            if ( event.ControlDown() )
                MoveCursorDownBlock( event.ShiftDown() );
            else if ( keyMods == ALT_DOWN )
                SetRowHeight(GetGridCursorRow(), GetRowHeight(GetGridCursorRow())+5);
            else
                MoveCursorDown( event.ShiftDown() );
            
            break;
        }
        case WXK_LEFT:
        {
            if (!ContainsGridCell(GetGridCursorCell())) break;
            if ( event.ControlDown() )
                MoveCursorLeftBlock( event.ShiftDown() );
            else if ( keyMods == ALT_DOWN )
                SetColWidth(GetGridCursorCol(), GetColWidth(GetGridCursorCol())-5);
            else
                MoveCursorLeft( event.ShiftDown() );
            
            break;
        }
        case WXK_RIGHT:
        {
            if (!ContainsGridCell(GetGridCursorCell())) break;
            if ( event.ControlDown() )
                MoveCursorRightBlock( event.ShiftDown() );
            else if ( keyMods == ALT_DOWN )
                SetColWidth(GetGridCursorCol(), GetColWidth(GetGridCursorCol())+5);
            else
                MoveCursorRight( event.ShiftDown() );
            
            break;
        }
        case WXK_PRIOR:
        {
            MoveCursorUpPage( event.ShiftDown() );
            break;
        }
        case WXK_NEXT:
        {
            MoveCursorDownPage( event.ShiftDown() );
            break;
        }
        case WXK_HOME :
        {
            if ( event.ControlDown() )
            {
                wxSheetCoords coords( 0, 0 );
                if ( ContainsGridCell(coords) )
                {
                    wxSheetCoords lastCoords(GetGridCursorCell());
                    MakeCellVisible( coords );
                    SetGridCursorCell( coords );
                    if ( event.ShiftDown() && ContainsGridCell(lastCoords) )
                    {
                        m_keySelecting = TRUE;
                        if ( !ContainsGridCell(GetSelectingAnchor()) )
                            SetSelectingAnchor(lastCoords);

                        HighlightSelectingBlock(GetSelectingAnchor(), GetGridCursorCell());
                    }
                }
            }
            else
                event.Skip();
            
            break;
        }
        case WXK_END:
        {
            if ( event.ControlDown() )
            {
                wxSheetCoords coords(GetNumberRows()-1, GetNumberCols()-1);
                if ( ContainsGridCell(coords) )
                {
                    wxSheetCoords lastCoords(GetGridCursorCell());
                    MakeCellVisible( coords );
                    SetGridCursorCell( coords );
                    if ( event.ShiftDown() && ContainsGridCell(lastCoords) )
                    {
                        m_keySelecting = TRUE;
                        if ( !ContainsGridCell(GetSelectingAnchor()) )
                            SetSelectingAnchor(lastCoords);

                        HighlightSelectingBlock(GetSelectingAnchor(), GetGridCursorCell());
                    }
                }
            }
            else
                event.Skip();
            
            break;
        }
        default : 
            event.Skip();
    }
    
    m_inOnKeyDown = FALSE;
}

void wxSheet::OnMouse( wxMouseEvent& event )
{
    if (!m_enable_split_vert && !m_enable_split_horiz)
        return;

    wxPoint mousePos = event.GetPosition();
    
    if (event.LeftDown())
    {
        if (m_vertSplitRect.Inside(mousePos) || m_horizSplitRect.Inside(mousePos))
            SetCaptureWindow(this);
    }
    else if (event.LeftUp())
    {
        SetCaptureWindow(NULL);
    }
    else if (event.Dragging() && HasCapture())// && 
             //(m_mouseCursorMode & WXSHEET_CURSOR_SPLITTING != 0))
    {
        wxSheetSplitterEvent splitEvent(GetId(), wxEVT_SHEET_SPLIT_BEGIN);
        splitEvent.SetEventObject(this);
        splitEvent.m_vert_split = (m_mouseCursorMode == WXSHEET_CURSOR_SPLIT_VERTICAL);
        
        SetCaptureWindow(NULL);
        SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, this);
        GetEventHandler()->ProcessEvent(splitEvent);
    }
    else if ((event.Leaving() || event.Entering()) && !HasCapture())
    {
        SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, this);
    }
    else if (event.Moving() && !HasCapture())
    {
        if (m_vertSplitRect.Inside(mousePos))
            SetMouseCursorMode(WXSHEET_CURSOR_SPLIT_VERTICAL, this);
        else if (m_horizSplitRect.Inside(mousePos))
            SetMouseCursorMode(WXSHEET_CURSOR_SPLIT_HORIZONTAL, this);
        else
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, this);
    }
}

void wxSheet::StopMouseTimer()
{
    if (m_mouseTimer)
    {
        if (m_mouseTimer->IsRunning())
            m_mouseTimer->Stop();
    
        delete m_mouseTimer;
        m_mouseTimer = NULL;
    }
}
void wxSheet::StartMouseTimer()
{
    if (!m_mouseTimer) 
        m_mouseTimer = new wxTimer(this, ID_MOUSE_DRAG_TIMER);
                
    if (!m_mouseTimer->IsRunning())
        m_mouseTimer->Start(100, FALSE);
}

void wxSheet::OnMouseTimer( wxTimerEvent &WXUNUSED(event) )
{
    // the window must be captured and thus m_mousePos is for that window
    wxWindow *win = GetCaptureWindow();
    
    if (!win || (GetNumberCols() < 1) || (GetNumberRows() < 1))
    {
        StopMouseTimer();
        return;
    }
    
    wxSize clientSize(win->GetClientSize());
    int dx = (m_mousePos.x < 0) ? -1 : ((m_mousePos.x > clientSize.x) ? 1 : 0);
    int dy = (m_mousePos.y < 0) ? -1 : ((m_mousePos.y > clientSize.y) ? 1 : 0);

    if (win == m_rowLabelWin)
        dx = 0;
    else if (win == m_colLabelWin)
        dy = 0;
    
    if ((dx == 0) && (dy == 0)) // mouse is back in the window
    {
        StopMouseTimer();
        return;
    }
    
    wxSize cSize = m_gridWin->GetClientSize();
    SetGridOrigin( m_gridOrigin.x + dx*SHEET_SCROLL_LINE_X, 
                   m_gridOrigin.y + dy*SHEET_SCROLL_LINE_Y, TRUE, TRUE );
    
    // send fake mouse event to process, assume left down and we're dragging
    wxMouseEvent mEvt(wxEVT_MOTION);
    mEvt.SetEventObject(win);
    mEvt.m_leftDown = TRUE;
    mEvt.m_x = m_mousePos.x;
    mEvt.m_y = m_mousePos.y;
    
    win->ProcessEvent(mEvt);
}

bool wxSheet::DoEndDragResizeRowCol()
{
    if ( m_dragLastPos < 0 )
        return FALSE;

    if (IsCellEditControlCreated())
        DisableCellEditControl(TRUE);
    
    // erase the last line and resize the row/col
    DrawRowColResizingMarker();

    if (m_mouseCursorMode == WXSHEET_CURSOR_RESIZE_ROW)
    {    
        int rowTop = GetRowTop(m_dragRowOrCol);
        int height = wxMax(m_dragLastPos - rowTop, GetMinimalRowHeight(m_dragRowOrCol));
        if (height != GetRowHeight(m_dragRowOrCol))
        {
            SetRowHeight( m_dragRowOrCol, height );
            return TRUE;
        }
    }
    else if (m_mouseCursorMode == WXSHEET_CURSOR_RESIZE_COL)
    {
        int colLeft = GetColLeft(m_dragRowOrCol);
        int width = wxMax(m_dragLastPos - colLeft, GetMinimalColWidth(m_dragRowOrCol));
        if (width != GetColWidth(m_dragRowOrCol))
        {
            SetColWidth( m_dragRowOrCol, width );
            return TRUE;
        }
    }
    
    return FALSE;
}

void wxSheet::SetMouseCursorMode(MouseCursorMode mode, wxWindow *win)
{
    wxCHECK_RET(win, wxT("Invalid window"));

    m_mouseCursorMode = mode;
    
    if (win == m_gridWin)
    {
        if (m_gridWin->m_mouseCursor == mode)
            return;
        
        m_gridWin->m_mouseCursor = mode;
    }
    else if (win == m_cornerLabelWin)
    {
        if (m_cornerLabelWin->m_mouseCursor == mode)
            return;
        
        m_cornerLabelWin->m_mouseCursor = mode;
    }
    else if (win == m_rowLabelWin)
    {
        if (m_rowLabelWin->m_mouseCursor == mode)
            return;
        
        m_rowLabelWin->m_mouseCursor = mode;
    }
    else if (win == m_colLabelWin)
    {
        if (m_colLabelWin->m_mouseCursor == mode)
            return;
        
        m_colLabelWin->m_mouseCursor = mode;
    }
    else if (win == this)
    {
        if (m_mouseCursor == mode)
            return;
        
        m_mouseCursor = mode;
    }

    switch ( mode )
    {
        case WXSHEET_CURSOR_RESIZE_ROW : 
        case WXSHEET_CURSOR_SPLIT_VERTICAL :
        {
            win->SetCursor( GetSheetRefData()->m_rowResizeCursor );
            break;
        }
        case WXSHEET_CURSOR_RESIZE_COL :
        case WXSHEET_CURSOR_SPLIT_HORIZONTAL :
        {
            win->SetCursor( GetSheetRefData()->m_colResizeCursor );
            break;
        }
        default:
            win->SetCursor( *wxSTANDARD_CURSOR );
    }
}

void wxSheet::SetCaptureWindow(wxWindow *win)
{   
    if (m_winCapture && (m_winCapture != win) && m_winCapture->HasCapture())
        m_winCapture->ReleaseMouse();

    m_winCapture = win;
    
    if (m_winCapture && (!m_winCapture->HasCapture()))
        m_winCapture->CaptureMouse();
}

wxWindow* wxSheet::GetWindowForCoords( const wxSheetCoords& coords ) const
{
    if (IsGridCell(coords))
        return m_gridWin;
    if (IsRowLabelCell(coords))
        return m_rowLabelWin;
    if (IsColLabelCell(coords))
        return m_colLabelWin;
    if (IsCornerLabelCell(coords))
        return m_cornerLabelWin;
    
    wxFAIL_MSG(wxString::Format(wxT("Unable to get window for coords (%d,%d)"), coords.m_row, coords.m_col));
    return NULL;
}

// ----- event handlers

// Generate a grid event based on a mouse/key event and
// return the result of ProcessEvent()
int wxSheet::SendEvent( const wxEventType type, const wxSheetCoords& coords, 
                        wxEvent *mouseOrKeyEvt )
{
    //wxMouseEvent *mouseEvt = wxDynamicCast(mouseOrKeyEvt, wxMouseEvent);    
    //wxPoint pos = mouseEvt ? mouseEvt->GetPosition() : wxPoint(-1, -1);   
    //pos += wxPoint(GetRowLabelWidth(), GetColLabelHeight());       
    
    wxSheetEvent sheetEvt(GetId(), type, this, coords, wxPoint(-1,-1), IsSelecting());
    sheetEvt.SetKeysDownMousePos(mouseOrKeyEvt);
    return DoSendEvent(&sheetEvt);
}

int wxSheet::SendRangeEvent( const wxEventType type, const wxSheetBlock& block, 
                             bool selecting, bool add, wxEvent *mouseOrKeyEvt )
{
    if ( type == wxEVT_SHEET_RANGE_SELECTED )
    {
        wxSheetRangeSelectEvent sheetEvt(GetId(), type, this, block, selecting, add );
        
        sheetEvt.SetKeysDownMousePos(mouseOrKeyEvt);
        sheetEvt.m_coords = GetGridCursorCell();
        return DoSendEvent(&sheetEvt);
    }
    
    return 0;
}

int wxSheet::DoSendEvent(wxSheetEvent *event)
{
    wxCHECK_MSG(event, 0, wxT("invalid event in wxSheet::DoSendEvent"));
    bool claimed = GetEventHandler()->ProcessEvent(*event);
    bool vetoed  = !event->IsAllowed();
  
    // A Veto'd event may not be claimed, test this first
    if (vetoed) return EVT_VETOED; 
    return claimed ? EVT_CLAIMED : EVT_SKIPPED;
}

void wxSheet::HighlightSelectingBlock( const wxSheetBlock &block_ )
{
    wxSheetBlock block(block_.GetAligned());
    
    if (block == GetSelectingBlock())
        return;
    
    if ( GetSelection() && !block.IsEmpty() )
    {
        // make sure block that's selected goes full width/height
        if ( GetSelectionMode() == wxSHEET_SelectRows )
        {
            block.m_col = 0;
            block.m_width = GetNumberCols() - 1;
        }
        else if ( GetSelectionMode() == wxSHEET_SelectCols )
        {
            block.m_row = 0;
            block.m_height = GetNumberRows() - 1;
        }
    }

    wxSheetBlock oldSelBlock(GetSelectingBlock());
    SetSelectingBlock(block.IsEmpty() ? wxNullSheetBlock : block);
    
    // First the case that we selected a completely new area
    if ( oldSelBlock.IsEmpty() )
    {
        RefreshGridCellBlock(block);
    }
    // New selection is empty, erase old one
    else if ( block.IsEmpty() )
    {
        RefreshGridCellBlock(oldSelBlock);
    }
    // two selections don't intersect at all, not expected, but ok I guess
    else if ( !block.Intersects(oldSelBlock) )      
    {
        RefreshGridCellBlock(block.Union(oldSelBlock));
    }
    // Now handle changing an existing selection area.
    else if ( oldSelBlock != block )
    {
        // FIXME - this is not great
        wxSheetBlock changed[8];
        wxSheetBlock bounds;
        
        oldSelBlock.Delete(block, changed[0], changed[1], changed[2], changed[3]);
        block.Delete(oldSelBlock, changed[4], changed[5], changed[6], changed[7]);
        
        {
            for (int n=0; n<8; n++)
                bounds = bounds.ExpandUnion(changed[n]);
            
            RefreshGridCellBlock(bounds);
        }
    }
}

// ------ functions to get/send data (see also public functions)

bool wxSheet::GetModelValues()
{
    // Hide the editor, so it won't hide a changed value.
    if (IsCellEditControlShown())
        HideCellEditControl();

    if ( GetTable() )
    {
        RefreshGridWindow(); // all we need to do is repaint the grid
        return TRUE;
    }

    return FALSE;
}

bool wxSheet::SetModelValues()
{
    // Disable the editor, so it won't hide a changed value.
    // FIXME: Do we also want to save the current value of the editor first? yes?
    if (IsCellEditControlCreated())
        DisableCellEditControl(TRUE);

    if ( GetTable() )
    {
        wxSheetCoords coords;
        int numRows = GetNumberRows();
        int numCols = GetNumberCols();
        for ( coords.m_row = 0; coords.m_row < numRows; coords.m_row++ )
        {
            for ( coords.m_col = 0; coords.m_col < numCols; coords.m_col++ )
                GetTable()->SetValue( coords, GetCellValue(coords) );
        }

        return TRUE;
    }

    return FALSE;
}

// ----------------------------------------------------------------------------
// Attrbitute cache 

void wxSheet::ClearAttrCache()
{
    if ( m_cacheAttrType != -1 )
    {
        m_cacheAttr.Destroy();
        m_cacheAttrCoords = wxNullSheetCoords;
        m_cacheAttrType = -1;
    }
}

void wxSheet::CacheAttr(const wxSheetCoords& coords, const wxSheetCellAttr &attr,
                        wxSheetAttr_Type type ) const
{
    if ( attr.Ok() )
    {
        wxSheet *self = (wxSheet *)this;  // const_cast
        self->m_cacheAttr = attr;
        self->m_cacheAttrCoords = coords;
        self->m_cacheAttrType = type;
    }
}

bool wxSheet::LookupAttr(const wxSheetCoords& coords, wxSheetAttr_Type type, 
                         wxSheetCellAttr &attr ) const
{
    if ( (type == m_cacheAttrType) && (coords == m_cacheAttrCoords) )
    {
        attr = m_cacheAttr;

#ifdef DEBUG_ATTR_CACHE
        gs_nAttrCacheHits++;
#endif

        return TRUE;
    }
    
#ifdef DEBUG_ATTR_CACHE
    gs_nAttrCacheMisses++;
#endif
    return FALSE;
}

// ----------------------------------------------------------------------------
// Gui Sizing functions

void wxSheet::CalcWindowSizes(bool adjustScrollBars)
{
    if (!m_gridWin || m_resizing)
        return;

    if (adjustScrollBars)
        AdjustScrollbars(FALSE);

    m_resizing = TRUE;
    
    int cw, ch;
    GetClientSize( &cw, &ch );
    wxRect rect;

    int  sb_width  = m_vertScrollBar->GetSize().x;
    int  sb_height = m_horizScrollBar->GetSize().y;
    bool horiz_sb  = m_horizScrollBar->IsShown();
    bool vert_sb   = m_vertScrollBar->IsShown();
    if (horiz_sb) ch -= sb_width;
    if (vert_sb ) cw -= sb_width;
    
    bool horiz_splitter = horiz_sb && m_enable_split_horiz;
    bool vert_splitter  = vert_sb  && m_enable_split_vert;
    
    if ( horiz_sb )
    {
        rect = wxRect(0, ch, cw, sb_height);
        if (horiz_splitter)
        {
            rect.width -= SPLIT_BUTTON_WIDTH;
            m_horizSplitRect = wxRect(rect.GetRight(), rect.GetTop(), SPLIT_BUTTON_WIDTH, rect.GetHeight());
        }
        else
            m_horizSplitRect = wxRect(0,0,0,0);
        
        if (rect != m_horizScrollBar->GetRect())
            m_horizScrollBar->SetSize( rect );
    }
    if ( vert_sb )
    {
        rect = wxRect(cw, 0, sb_width, ch);
        if (vert_splitter)
        {
            rect.height -= SPLIT_BUTTON_WIDTH;
            m_vertSplitRect = wxRect(rect.GetLeft(), rect.GetTop(), rect.GetWidth(), SPLIT_BUTTON_WIDTH);
            rect.y += SPLIT_BUTTON_WIDTH;
        }
        else
            m_vertSplitRect = wxRect(0,0,0,0);
        
        if (rect != m_vertScrollBar->GetRect())
            m_vertScrollBar->SetSize( rect );
    }
    if ( m_cornerLabelWin->IsShown() )
    {
        rect = wxRect(0, 0, GetRowLabelWidth(), GetColLabelHeight());
        if (rect != m_cornerLabelWin->GetRect())
            m_cornerLabelWin->SetSize( rect );
    }
    if ( m_colLabelWin->IsShown() )
    {
        rect = wxRect(GetRowLabelWidth(), 0, cw-GetRowLabelWidth(), GetColLabelHeight());
        if (rect != m_colLabelWin->GetRect())
            m_colLabelWin->SetSize( rect );
    }
    if ( m_rowLabelWin->IsShown() )
    {
        rect = wxRect(0, GetColLabelHeight(), GetRowLabelWidth(), ch-GetColLabelHeight());
        if (rect != m_rowLabelWin->GetRect())
            m_rowLabelWin->SetSize( rect );
    }
    if ( m_gridWin->IsShown() )
    {
        rect = wxRect(GetRowLabelWidth(), GetColLabelHeight(), cw-GetRowLabelWidth(), ch-GetColLabelHeight());
        if (rect != m_gridWin->GetRect())
            m_gridWin->SetSize( rect );
        
        //PRINT_RECT(wxT("Set grid rect "), rect);
        //PRINT_RECT(wxT("Get grid rect "), wxRect(wxPoint(0,0), m_gridWin->GetSize()));
    }
    
    m_resizing = FALSE;    
}

void wxSheet::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
    wxWindow::DoSetSize(x, y, width, height, sizeFlags);
    CalcWindowSizes();
    SetGridOrigin(m_gridOrigin.x, m_gridOrigin.y, TRUE, TRUE);
    SetEqualColWidths(GetSheetRefData()->m_equal_col_widths);
}

wxSize wxSheet::DoGetBestSize() const
{
    // don't set sizes, only calculate them
    wxSheet *self = (wxSheet *)this;  // const_cast

    int width = self->SetOrCalcColumnSizes(TRUE);
    int height = self->SetOrCalcRowSizes(TRUE);

    int maxwidth, maxheight;
    wxDisplaySize( &maxwidth, &maxheight );

    if ( width > maxwidth ) 
        width = maxwidth;
    if ( height > maxheight ) 
        height = maxheight;

    return wxSize( width, height );
}


int wxSheet::GetKeyModifiers(wxEvent *mouseOrKeyEvent) const
{
    // In GTK meta is the numLock key
    int mods = NO_MODIFIERS;
    wxMouseEvent *mouseEvt = wxDynamicCast(mouseOrKeyEvent, wxMouseEvent);
    
    if (mouseEvt)
    {
        if (mouseEvt->ControlDown())
            mods |= CTRL_DOWN; 
        if (mouseEvt->ShiftDown())
            mods |= SHIFT_DOWN; 
        if (mouseEvt->AltDown())     
            mods |= ALT_DOWN; 
        //if (mouseEvt->MetaDown())    // meta is numlock in GTK
        //    mods |= META_DOWN; 
    }
    else 
    {
        wxKeyEvent *keyEvt = wxDynamicCast(mouseOrKeyEvent, wxKeyEvent);
        if (keyEvt)
        {
            if (keyEvt->ControlDown())
                mods |= CTRL_DOWN; 
            if (keyEvt->ShiftDown())
                mods |= SHIFT_DOWN; 
            if (keyEvt->AltDown())     
                mods |= ALT_DOWN; 
            //if (keyEvt->MetaDown())    
            //    mods |= META_DOWN; 
        }
    }

    //wxPrintf("Mods c%d s%d a%d m%d, %d\n", CTRL_DOWN, SHIFT_DOWN, ALT_DOWN, META_DOWN, mods); 
    
    return mods;    
}

// ----------------------------------------------------------------------------
// events
// ----------------------------------------------------------------------------

DEFINE_EVENT_TYPE(wxEVT_SHEET_VIEW_CHANGED)
DEFINE_EVENT_TYPE(wxEVT_SHEET_SELECTING_CELL)
DEFINE_EVENT_TYPE(wxEVT_SHEET_SELECTED_CELL)
DEFINE_EVENT_TYPE(wxEVT_SHEET_CELL_LEFT_DOWN)
DEFINE_EVENT_TYPE(wxEVT_SHEET_CELL_RIGHT_DOWN)
DEFINE_EVENT_TYPE(wxEVT_SHEET_CELL_LEFT_UP)
DEFINE_EVENT_TYPE(wxEVT_SHEET_CELL_RIGHT_UP)
DEFINE_EVENT_TYPE(wxEVT_SHEET_CELL_LEFT_DCLICK)
DEFINE_EVENT_TYPE(wxEVT_SHEET_CELL_RIGHT_DCLICK)
DEFINE_EVENT_TYPE(wxEVT_SHEET_LABEL_LEFT_DOWN)
DEFINE_EVENT_TYPE(wxEVT_SHEET_LABEL_RIGHT_DOWN)
DEFINE_EVENT_TYPE(wxEVT_SHEET_LABEL_LEFT_UP)
DEFINE_EVENT_TYPE(wxEVT_SHEET_LABEL_RIGHT_UP)
DEFINE_EVENT_TYPE(wxEVT_SHEET_LABEL_LEFT_DCLICK)
DEFINE_EVENT_TYPE(wxEVT_SHEET_LABEL_RIGHT_DCLICK)
DEFINE_EVENT_TYPE(wxEVT_SHEET_ROW_SIZE)
DEFINE_EVENT_TYPE(wxEVT_SHEET_COL_SIZE)
DEFINE_EVENT_TYPE(wxEVT_SHEET_RANGE_SELECTING)
DEFINE_EVENT_TYPE(wxEVT_SHEET_RANGE_SELECTED)
DEFINE_EVENT_TYPE(wxEVT_SHEET_CELL_VALUE_CHANGING)
DEFINE_EVENT_TYPE(wxEVT_SHEET_CELL_VALUE_CHANGED)
DEFINE_EVENT_TYPE(wxEVT_SHEET_EDITOR_ENABLED)
DEFINE_EVENT_TYPE(wxEVT_SHEET_EDITOR_DISABLED)
DEFINE_EVENT_TYPE(wxEVT_SHEET_EDITOR_CREATED)

// ----------------------------------------------------------------------------
// wxSheetEvent
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( wxSheetEvent, wxNotifyEvent )

wxSheetEvent::wxSheetEvent(int id, wxEventType type, wxObject* obj,
                           const wxSheetCoords& coords, const wxPoint &pos,
                           bool sel )
    : wxNotifyEvent(type, id), m_coords(coords), m_pos(pos), m_selecting(sel), 
                  m_control(FALSE), m_shift(FALSE), m_alt(FALSE), m_meta(FALSE)
{
    SetEventObject(obj);
}

void wxSheetEvent::SetKeysDownMousePos(wxEvent *mouseOrKeyEvent)
{
    wxMouseEvent *mouseEvt = wxDynamicCast(mouseOrKeyEvent, wxMouseEvent);
    if (mouseEvt)
    {
        m_control = mouseEvt->ControlDown(); 
        m_shift   = mouseEvt->ShiftDown();   
        m_alt     = mouseEvt->AltDown();     
        m_meta    = mouseEvt->MetaDown();    
        m_pos     = mouseEvt->GetPosition();
    }
    else 
    {
        wxKeyEvent *keyEvt = wxDynamicCast(mouseOrKeyEvent, wxKeyEvent);
        if (keyEvt)
        {
            m_control = keyEvt->ControlDown(); 
            m_shift   = keyEvt->ShiftDown();   
            m_alt     = keyEvt->AltDown();     
            m_meta    = keyEvt->MetaDown();    
            m_pos     = keyEvt->GetPosition();
        }
        else
            return;  // neither mouse nor key event
    }
    
    // FIXME - do I really want to scroll the position? or leave it as is
    // we've set the position from the event, now scroll it
    wxSheet *sheet = wxDynamicCast(GetEventObject(), wxSheet);
    wxWindow *win = wxDynamicCast(mouseOrKeyEvent->GetEventObject(), wxWindow);
    if (sheet && win)
    {
        if ( win == sheet->GetGridWindow())
            m_pos = sheet->CalcUnscrolledPosition(m_pos);
        else if (win == sheet->GetRowLabelWindow())
            sheet->CalcUnscrolledPosition(0, m_pos.y, NULL, &m_pos.y);
        else if (win == sheet->GetColLabelWindow())
            sheet->CalcUnscrolledPosition(m_pos.x, 0, &m_pos.x, NULL);
    }
}

// ----------------------------------------------------------------------------
// wxSheetRangeSelectEvent
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS( wxSheetRangeSelectEvent, wxSheetEvent )

wxSheetRangeSelectEvent::wxSheetRangeSelectEvent(int id, wxEventType type, wxObject* obj,
                                                 const wxSheetBlock& block,
                                                 bool sel, bool add )
        : wxSheetEvent(id, type, obj, wxNullSheetCoords, wxPoint(-1, -1), sel), 
          m_block(block), m_add(add)
{
}

// ----------------------------------------------------------------------------
// wxSheetEditorCreatedEvent
// ----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(wxSheetEditorCreatedEvent, wxCommandEvent)

wxSheetEditorCreatedEvent::wxSheetEditorCreatedEvent(int id, wxEventType type,
                                                     wxObject* obj, 
                                                     const wxSheetCoords& coords,
                                                     wxWindow* ctrl)
    : wxCommandEvent(type, id), m_coords(coords), m_ctrl(ctrl)
{
    SetEventObject(obj);
}

#endif // wxUSE_GRID
