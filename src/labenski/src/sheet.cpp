///////////////////////////////////////////////////////////////////////////////
// Name:        sheet.cpp
// Purpose:     wxSheet and related classes
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: John Labenski, Robin Dunn, Vadim Zeitlin
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, Michael Bedward (mbedward@ozemail.com.au)
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

// wxSheet code is based on wxGrid, to ensure that it stays up to date changes
// in the wxGrid files should be noted and incorporated if applicable.
// Note: very little code remains, update colours and sizing...
// wx/include/wx/generic/grid.h         1.140
// wx/include/wx/generic/gridctrl.h     1.15
// wx/include/wx/generic/gridsel.h      1.18
// wx/src/generic/grid.cpp              1.318
// wx/src/generic/gridctrl.cpp          1.15
// wx/src/generic/gridsel.cpp           1.20

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "sheet.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/defs.h"
    #include "wx/utils.h"
    #include "wx/dcclient.h"
    #include "wx/settings.h"
    #include "wx/log.h"
    #include "wx/dcscreen.h"
#endif // WX_PRECOMP

#include "wx/sheet/sheet.h"
#include "wx/sheet/sheetspt.h" // for wxSheetSplitterEvent, wxEVT_SHEET_SPLIT_BEGIN

#include "wx/timer.h"
#include "wx/clipbrd.h"
#include "wx/renderer.h"

// Required for wxIs... functions
#include <ctype.h>

#define PRINT_BLOCK(s, b) wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), wxT(s), b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());
#define PRINT_RECT(s, b)  wxPrintf(wxT("%s %d %d %d %d - w%d h%d\n"), wxT(s), b.GetTop(), b.GetLeft(), b.GetBottom(), b.GetRight(), b.GetWidth(), b.GetHeight());

#define SPLIT_BUTTON_WIDTH 6

#define wxStrF wxString::Format

#ifndef wxSL_INVERSE // !wxCHECK_VERSION(2,5,3) // FIXME temp fix until 2.5.4
    #define SetOwnBackgroundColour SetBackgroundColour
    #define SetOwnForegroundColour SetForegroundColour
#endif

#ifdef __wxGTK__
    //#define USE_RENDERNATIVE 1
#endif // __wxGTK__

static inline bool wxRectIsEmpty(const wxRect& rect)
{
    return (rect.width < 1) || (rect.height < 1);
}

// ----------------------------------------------------------------------------
// conditional compilation
// ----------------------------------------------------------------------------

// Gambit modification for custom border drawing
#define WXSHEET_DRAW_LINES 0
#ifndef WXSHEET_DRAW_LINES
    #define WXSHEET_DRAW_LINES 1
#endif

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
#define SHEET_SCROLL_LINE_X 15  // 1
#define SHEET_SCROLL_LINE_Y SHEET_SCROLL_LINE_X

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
class WXDLLIMPEXP_SHEET wxSheetTypeRegistry
{
public:
    wxSheetTypeRegistry() {}
    virtual ~wxSheetTypeRegistry();

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
// wxSheetChildWindow
// ----------------------------------------------------------------------------
IMPLEMENT_ABSTRACT_CLASS( wxSheetChildWindow, wxWindow )

BEGIN_EVENT_TABLE( wxSheetChildWindow, wxWindow )
    EVT_PAINT        ( wxSheetChildWindow::OnPaint )
    EVT_MOUSEWHEEL   ( wxSheetChildWindow::OnMouse )
    EVT_MOUSE_EVENTS ( wxSheetChildWindow::OnMouse )
    EVT_KEY_DOWN     ( wxSheetChildWindow::OnKeyAndChar )
    EVT_KEY_UP       ( wxSheetChildWindow::OnKeyAndChar )
    EVT_CHAR         ( wxSheetChildWindow::OnKeyAndChar )
    EVT_SET_FOCUS    ( wxSheetChildWindow::OnFocus )
    EVT_KILL_FOCUS   ( wxSheetChildWindow::OnFocus )          
END_EVENT_TABLE()

wxSheetChildWindow::wxSheetChildWindow( wxSheet *parent, wxWindowID id,
                                        const wxPoint &pos, const wxSize &size,
                                        long style, const wxString& name )
                   :wxWindow( parent, id, pos, size, style, name ),
                    m_owner(parent), m_mouseCursor(wxSheet::WXSHEET_CURSOR_SELECT_CELL)
{
}

void wxSheetChildWindow::OnPaint( wxPaintEvent& event )
{
    if (!m_owner || !m_owner->GetEventHandler()->ProcessEvent(event))
    {
        wxPaintDC dc(this); // MSW at least requires this
    }
}

void wxSheetChildWindow::OnMouse( wxMouseEvent& event )
{
    if (!m_owner || !m_owner->GetEventHandler()->ProcessEvent(event))
        event.Skip();    
}

// This seems to be required for wxMotif otherwise the mouse
// cursor must be in the cell edit control to get key events
void wxSheetChildWindow::OnKeyAndChar( wxKeyEvent& event )
{
    if (!m_owner || !m_owner->GetEventHandler()->ProcessEvent(event)) 
        event.Skip();
}

void wxSheetChildWindow::OnFocus( wxFocusEvent& event )
{
    if (!m_owner || !m_owner->GetEventHandler()->ProcessEvent(event))
        event.Skip();
}

void wxSheetChildWindow::OnEraseBackground( wxEraseEvent& event)
{
    if (!m_owner || !m_owner->GetEventHandler()->ProcessEvent(event))
        event.Skip(false);
}

//-----------------------------------------------------------------------------
// wxSheetRefData
//-----------------------------------------------------------------------------
wxSheetRefData::wxSheetRefData()
{
    m_table    = NULL;
    m_ownTable = false;

    m_typeRegistry = new wxSheetTypeRegistry;
    
    m_rowEdges.SetDefaultSize(WXSHEET_DEFAULT_ROW_HEIGHT);
    m_colEdges.SetDefaultSize(WXSHEET_DEFAULT_COL_WIDTH);
    m_rowEdges.SetMinAllowedSize(WXSHEET_MIN_ROW_HEIGHT);
    m_colEdges.SetMinAllowedSize(WXSHEET_MIN_COL_WIDTH);

    m_rowLabelWidth  = WXSHEET_DEFAULT_ROW_LABEL_WIDTH;
    m_colLabelHeight = WXSHEET_DEFAULT_COL_LABEL_HEIGHT;
   
    m_equal_col_widths = 0;

    m_gridLineColour   = wxColour( 192, 192, 192 );
    m_gridLinesEnabled = wxBOTH;
    m_cursorCellHighlightColour     = *wxBLACK;
    m_cursorCellHighlightPenWidth   = 2;
    m_cursorCellHighlightROPenWidth = 1;
    m_labelOutlineColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW);

    m_rowResizeCursor = wxCursor( wxCURSOR_SIZENS );
    m_colResizeCursor = wxCursor( wxCURSOR_SIZEWE );

    m_editable = true;  // default for whole grid

    m_canDrag = wxSHEET_DragCellSize;

    m_cursorCoords = wxNullSheetCoords;

    m_selectingAnchor = wxNullSheetCoords;
    m_selectionMode   = wxSHEET_SelectCells;
    m_selection       = new wxSheetSelection;
    m_selectionBackground = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
    m_selectionForeground = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
        
    m_cellEditorCoords = wxNullSheetCoords;

    m_pasting = false;
}

wxSheetRefData::~wxSheetRefData() 
{
    if (m_ownTable && m_table)
        delete m_table;
    
    delete m_typeRegistry;
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
    const int index = FindSheet(sheet);
    if (index != wxNOT_FOUND)
        m_sheets.RemoveAt(index);
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

IMPLEMENT_DYNAMIC_CLASS_XTI(wxSheet, wxWindow,"wx/sheet/sheet.h")

wxBEGIN_PROPERTIES_TABLE(wxSheet)
    wxHIDE_PROPERTY( Children )
    wxPROPERTY_FLAGS( WindowStyle , wxSheetStyle , long , SetWindowStyleFlag , GetWindowStyleFlag , EMPTY_MACROVALUE, 0 /*flags*/ , wxT("Helpstring") , wxT("group")) // style
wxEND_PROPERTIES_TABLE()

wxBEGIN_HANDLERS_TABLE(wxSheet)
wxEND_HANDLERS_TABLE()

wxCONSTRUCTOR_5( wxSheet , wxWindow* , Parent , wxWindowID , Id , wxPoint , Position , wxSize , Size , long , WindowStyle ) 

/*
 TODO : Expose more information of a list's layout etc. via appropriate objects (a la NotebookPageInfo)
*/
#else
IMPLEMENT_DYNAMIC_CLASS( wxSheet, wxWindow )
#endif

BEGIN_EVENT_TABLE( wxSheet, wxWindow )
    EVT_PAINT           ( wxSheet::OnPaint )
    EVT_SIZE            ( wxSheet::OnSize )
    //EVT_ERASE_BACKGROUND( wxSheet::OnEraseBackground )
    EVT_KEY_DOWN        ( wxSheet::OnKeyDown )
    EVT_KEY_UP          ( wxSheet::OnKeyUp )
    EVT_CHAR            ( wxSheet::OnChar )
    EVT_MOUSEWHEEL      ( wxSheet::OnMouseWheel )
    EVT_MOUSE_EVENTS    ( wxSheet::OnMouse )
    EVT_COMMAND_SCROLL  ( wxSheet::ID_HORIZ_SCROLLBAR,  wxSheet::OnScroll )
    EVT_COMMAND_SCROLL  ( wxSheet::ID_VERT_SCROLLBAR,   wxSheet::OnScroll )
    EVT_TIMER           ( wxSheet::ID_MOUSE_DRAG_TIMER, wxSheet::OnMouseTimer )
END_EVENT_TABLE()

void wxSheet::Init()
{    
    m_cornerLabelWin = NULL;
    m_rowLabelWin    = NULL;
    m_colLabelWin    = NULL;
    m_gridWin        = NULL;
    m_horizScrollBar = NULL;
    m_vertScrollBar  = NULL;
    
    m_scrollBarMode  = wxSheet::SB_AS_NEEDED;

    m_keySelecting    = false;

    // init attr cache
    m_cacheAttr       = wxNullSheetCellAttr;
    m_cacheAttrCoords = wxNullSheetCoords;
    m_cacheAttrType   = -1;
    
    m_inOnKeyDown = false;
    m_batchCount  = 0;
    m_resizing    = false;
    
    m_mouseCursorMode = WXSHEET_CURSOR_SELECT_CELL;
    m_mouseCursor     = WXSHEET_CURSOR_SELECT_CELL;

    m_winCapture = (wxWindow *)NULL;
    
    m_dragLastPos     = -1;
    m_dragRowOrCol    = -1;
    m_isDragging      = false;
    m_startDragPos    = wxDefaultPosition;

    m_waitForSlowClick = false;
    m_mouseTimer       = NULL;
    
    m_enable_split_vert  = false;
    m_enable_split_horiz = false;    
    
    m_refData = new wxSheetRefData; // ALWAYS CREATED and ALWAYS EXISTS!
}

// NOTE: If using the default visual attributes works everywhere then this can
// be removed as well as the #else cases below.
#define _USE_VISATTR 0

#if _USE_VISATTR
#include "wx/listbox.h"
#endif

bool wxSheet::Create( wxWindow *parent, wxWindowID id,
                      const wxPoint& pos, const wxSize& size,
                      long style, const wxString& name )
{
    if (!wxWindow::Create(parent, id, pos, size, style | wxWANTS_CHARS, name))
        return false;
    
#if wxCHECK_VERSION(2,5,2) && _USE_VISATTR
    wxVisualAttributes gva = wxListBox::GetClassDefaultAttributes();
    wxVisualAttributes lva = wxPanel::GetClassDefaultAttributes();

    wxColour gfg = gva.colFg;  // grid foreground
    wxColour gbg = gva.colBg;  // grid background
    wxColour lfg = lva.colFg;  // label foreground
    wxColour lbg = lva.colBg;  // label background
#else // 2.4.x     
    wxColour gfg = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT );
    wxColour gbg = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOW );
    wxColour lfg = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT );
    wxColour lbg = wxSystemSettings::GetColour( wxSYS_COLOUR_BTNFACE );
#endif //wxCHECK_VERSION(2,5,2)   
    
    // Set default cell attributes
    wxSheetCellAttr *attr = &GetSheetRefData()->m_defaultGridCellAttr;
    attr->Create();
    attr->SetKind(wxSHEET_AttrDefault);
    attr->SetFont(GetFont()); // need window created for this
    attr->SetAlignment(wxALIGN_LEFT | wxALIGN_TOP);
    attr->SetOrientation(wxHORIZONTAL);
    attr->SetLevel(wxSHEET_AttrLevelBottom);
    attr->SetOverflow(true);
    attr->SetOverflowMarker(true);
    attr->SetShowEditor(false);
    attr->SetReadOnly(false);
    attr->SetForegroundColour(gfg);
    attr->SetBackgroundColour(gbg);
    attr->SetRenderer(GetDefaultRendererForType(wxSHEET_VALUE_STRING));
    attr->SetEditor(GetDefaultEditorForType(wxSHEET_VALUE_STRING));

    wxFont labelFont = wxWindow::GetFont();
    labelFont.SetWeight( wxBOLD );
    
    // default row label attr
    attr = &GetSheetRefData()->m_defaultRowLabelAttr;
    attr->Create();
    attr->SetKind(wxSHEET_AttrDefault);
    attr->SetFont(labelFont); // need window created for this
    attr->SetAlignment(wxALIGN_CENTRE);
    attr->SetOrientation(wxHORIZONTAL);
    attr->SetLevel(wxSHEET_AttrLevelBottom);
    attr->SetOverflow(false);
    attr->SetOverflowMarker(false); // just cutoff, looks better?
    attr->SetShowEditor(false);
    attr->SetReadOnly(true);
    attr->SetForegroundColour(lfg);
    attr->SetBackgroundColour(lbg);
    attr->SetRenderer(GetDefaultRendererForType(wxSHEET_VALUE_LABEL));
    attr->SetEditor(GetDefaultEditorForType(wxSHEET_VALUE_STRING));

    // default col and corner label attr are identical to row label attr
    GetSheetRefData()->m_defaultColLabelAttr    = attr->Clone();
    GetSheetRefData()->m_defaultCornerLabelAttr = attr->Clone();

    // subwindow components that make up the wxSheet
    m_gridWin        = new wxSheetChildWindow( this, ID_GRID_WINDOW );    
    m_rowLabelWin    = new wxSheetChildWindow( this, ID_ROW_LABEL_WINDOW );
    m_colLabelWin    = new wxSheetChildWindow( this, ID_COL_LABEL_WINDOW );
    m_cornerLabelWin = new wxSheetChildWindow( this, ID_CORNER_LABEL_WINDOW );
    m_horizScrollBar = new wxScrollBar(this, ID_HORIZ_SCROLLBAR, wxDefaultPosition,
                                       wxDefaultSize, wxSB_HORIZONTAL);
    m_vertScrollBar  = new wxScrollBar(this, ID_VERT_SCROLLBAR, wxDefaultPosition,
                                       wxDefaultSize, wxSB_VERTICAL);

    // det the default colours for each window 
    m_cornerLabelWin->SetOwnForegroundColour(lfg);
    m_cornerLabelWin->SetOwnBackgroundColour(lbg);
    m_rowLabelWin->SetOwnForegroundColour(lfg);
    m_rowLabelWin->SetOwnBackgroundColour(lbg);
    m_colLabelWin->SetOwnForegroundColour(lfg);
    m_colLabelWin->SetOwnBackgroundColour(lbg);

    m_gridWin->SetOwnForegroundColour(gfg);
    m_gridWin->SetOwnBackgroundColour(gbg);

    // figure out what a reasonable size would be 
    int char_w = 5, char_h = 10;
    GetTextExtent(wxT("W1!(jJ"), &char_w, &char_h, NULL, NULL, &labelFont);
    
#if defined(__WXMOTIF__) || defined(__WXGTK__)  // see also text ctrl sizing in ShowCellEditControl()
    char_h += 8;
#else
    char_h += 6; // test in wx24
#endif

    GetSheetRefData()->m_rowEdges.SetDefaultSize(char_h);

    GetSheetRefData()->AddSheet(this);

#if wxCHECK_VERSION(2,7,2)
    SetInitialSize(size);
#elif wxCHECK_VERSION(2,5,2)
    SetBestFittingSize(size);
#endif // wxCHECK_VERSION(2,5,2)

    return true;
}

wxSheet::~wxSheet()
{
    SetCaptureWindow(NULL);
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
    SetCaptureWindow(NULL);
    StopMouseTimer();
    GetSheetRefData()->RemoveSheet(this);
    return wxWindow::Destroy();
}

void wxSheet::RefSheet(wxSheet* sheet)
{
    wxCHECK_RET(sheet, wxT("Invalid wxSheet"));
    SetCaptureWindow(NULL);
    StopMouseTimer();
    GetSheetRefData()->RemoveSheet(this);  // remove from old data
    UnRef();
    Ref(*sheet);
    GetSheetRefData()->AddSheet(this);
    CalcWindowSizes();
}

wxSheet* wxSheet::Clone(wxWindowID id)
{
    wxSheet *sheet = (wxSheet*)GetClassInfo()->CreateObject();
    sheet->Create(GetParent(), id);
    return sheet;
}

bool wxSheet::Enable(bool enable)
{
    if ( !wxWindow::Enable(enable) )
  	    return false;
  	 
  	// redraw in the new state
  	Refresh();
  	 
  	return true;
}

// ------------------------------------------------------------------------

bool wxSheet::SetTable( wxSheetTable *table, bool takeOwnership )
{
    if ( GetSheetRefData()->m_table )
    {
        if (GetSheetRefData()->m_ownTable)
            delete GetSheetRefData()->m_table;

        if (GetSelection())
            GetSelection()->Clear();

        GetSheetRefData()->m_table = NULL;
        GetSheetRefData()->m_rowEdges.Clear();
        GetSheetRefData()->m_colEdges.Clear();
    }
    
    if (table)
    {
        GetSheetRefData()->m_rowEdges.UpdatePos(0, table->GetNumberRows());
        GetSheetRefData()->m_colEdges.UpdatePos(0, table->GetNumberCols());

        GetSheetRefData()->m_table = table;
        GetSheetRefData()->m_table->SetView( this );
        GetSheetRefData()->m_ownTable = takeOwnership;
        
        CalcWindowSizes();
    }

    return true;
}

bool wxSheet::CreateGrid( int numRows, int numCols, int options )
{
    // Create a table and assign it's data structures
    wxSheetTable *table = new wxSheetTable( this );
    table->SetGridCellValueProvider(new wxSheetValueProviderString(numRows, numCols, options), true);
    //table->SetGridCellValueProvider(new wxSheetValueProviderSparseString(numRows, numCols, options), true);
    
    // label data containers are optional and needed only if you Set values for them
    // we create both of them.
    // NB. They're the same as the grid data but only have one row or col each. 
    // NB. wxSHEET_ValueProviderColPref is used for the col labels since 
    //     we expect more cols than rows
    table->SetRowLabelValueProvider(new wxSheetValueProviderString(numRows, 1), true);
    table->SetColLabelValueProvider(new wxSheetValueProviderString(1, numCols, wxSHEET_ValueProviderColPref), true);    
    // Set the attribute provider if you plan to "Set" attributes for particular
    //  cells/rows/cols, otherwise the default attributes will suffice
    table->SetAttrProvider(new wxSheetCellAttrProvider, true);

    return SetTable(table, true);
}

// ------------------------------------------------------------------------
// Dimensions of the sheet

wxSheetCell_Type wxSheet::GetCellCoordsType(const wxSheetCoords& coords)
{
    if ((coords.m_row >= 0) && (coords.m_col >= 0))
    {
        return wxSHEET_CELL_GRID;
    }
    else if (coords.m_row == -1)
    {
        if (coords.m_col == -1)
            return wxSHEET_CELL_CORNERLABEL;
        if (coords.m_col >= 0)
            return wxSHEET_CELL_COLLABEL;
    }
    else if (coords.m_col == -1)
    {
        if (coords.m_row >= 0)
            return wxSHEET_CELL_ROWLABEL;
    }

    return wxSHEET_CELL_UNKNOWN;
}

void wxSheet::ClearValues(int update)
{
    if ( GetTable() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(false);

        GetTable()->ClearValues(update);
        RefreshGridWindow();
    }
}

bool wxSheet::UpdateRows( size_t row, int numRows, int update )
{
    wxCHECK_MSG(GetTable(), false, wxT("Called wxSheet::UpdateRows() before calling CreateGrid()"));
    // the table should then call DoUpdateRows back on the sheet before returning
    return GetTable()->UpdateRows(row, numRows, update);
}
bool wxSheet::UpdateCols( size_t col, int numCols, int update )
{
    wxCHECK_MSG(GetTable(), false, wxT("Called wxSheet::UpdateCols() before calling CreateGrid()"));
    // the table should then call DoUpdateRows back on the sheet before returning
    return GetTable()->UpdateCols(col, numCols, update);
}

bool wxSheet::SetNumberCols( size_t cols, int update )
{
    const int n_cols = int(cols) - GetNumberCols();
    if (n_cols > 0)
        return AppendCols(n_cols, update);
    else if (n_cols < 0)
        return DeleteCols(cols, -n_cols, update);
    
    return false;
}

bool wxSheet::SetNumberRows( size_t rows, int update )
{
    const int n_rows = int(rows) - GetNumberRows();
    if (n_rows > 0) 
        return AppendRows(n_rows, update);
    else if (n_rows < 0) 
        return DeleteRows(rows, -n_rows, update);
    
    return false;
}

bool wxSheet::DoUpdateRows( size_t row, int numRows, int update)
{    
    // Clear the attribute cache as the attribute might refer to a invalid cell
    ClearAttrCache();
    if (numRows == 0) return false;
    
    // By the same reasoning, the editor should be dismissed if columns are
    // added or removed. And for consistency, it should IMHO always be
    // removed, not only if the cell "underneath" it actually changes.
    // For now, I intentionally do not save the editor's content as the
    // cell it might want to save that stuff to might no longer exist.
    if (IsCellEditControlCreated())
        DisableCellEditControl(false);
    
    // FIXME - do I want to UpdateRows/Cols for selection or just clear them
    //         the event it would send would be confusing at best
    //         first a deselect and then a select... should sel follow insert?
    //         I don't think it's worth the confusion and since a click on 
    //         a cell is deselect all, the selection is not that precious.
    //ClearSelection(true);

    GetSheetRefData()->m_rowEdges.UpdatePos(row, numRows);

    // Update any data structures
    if ( ((update & wxSHEET_UpdateSelection) != 0) && GetSelection() )
        GetSelection()->UpdateRows( row, numRows );

    GetSheetRefData()->m_cursorCoords.UpdateRows(row, numRows);    
    // try to force the cursor to a valid cell
    if ( !ContainsGridCell(GetGridCursorCell()) && ContainsGridCell(wxSheetCoords(0,0)) )
        SetGridCursorCell( wxSheetCoords(0, 0) );
    else
        GetSheetRefData()->m_cursorCoords = wxNullSheetCoords;

    AdjustScrollbars();
    RefreshRowLabelWindow(true);
    RefreshGridWindow();    
    return true;
}

bool wxSheet::DoUpdateCols( size_t col, int numCols, int update)
{
    ClearAttrCache();
    if (numCols == 0) return false;
    
    if (IsCellEditControlCreated())
        DisableCellEditControl(false);
       
    GetSheetRefData()->m_colEdges.UpdatePos(col, numCols);

    if ( ((update & wxSHEET_UpdateSelection) != 0) && GetSelection() )
        GetSelection()->UpdateCols( col, numCols );   

    GetSheetRefData()->m_cursorCoords.UpdateCols(col, numCols);    
    if ( !ContainsGridCell(GetGridCursorCell()) && ContainsGridCell(wxSheetCoords(0,0)) )
        SetGridCursorCell( wxSheetCoords(0, 0) );
    else
        GetSheetRefData()->m_cursorCoords = wxNullSheetCoords;

    AdjustScrollbars();
    RefreshColLabelWindow(true);
    RefreshGridWindow();   
    return true;
}

// ----------------------------------------------------------------------------
// Dimensions of the row and column sizes

void wxSheet::SetDefaultRowHeight( int height, bool resizeExistingRows )
{
    GetSheetRefData()->m_rowEdges.SetDefaultSize(height, resizeExistingRows);
    if ( resizeExistingRows && !GetBatchCount() )
        CalcWindowSizes(true);
}

void wxSheet::SetDefaultColWidth( int width, bool resizeExistingCols )
{
    GetSheetRefData()->m_colEdges.SetDefaultSize(width, resizeExistingCols);
    if ( resizeExistingCols && !GetBatchCount() )
        CalcWindowSizes(true);
}

void wxSheet::SetRowHeight( int row, int height )
{
    if (row == -1)
    {
        SetColLabelHeight(height);
        return;
    }
    
    wxCHECK_RET(ContainsGridRow(row), _T("invalid row index") );

    const int old_height = GetRowHeight(row);

    // if < 0 calc new height from label
    if ( height < 0 )
        height = GetCellBestSize(wxSheetCoords(row, -1)).y + 6; // FIXME test this
    
    // should we check that it's bigger than GetMinimalRowHeight(row) here?
    //                                                                 (VZ)
    // No, because it is reasonable to assume the library user know's
    // what he is doing. However whe should test against the weaker
    // constariant of minimalAcceptableHeight, as this breaks rendering
    if ( height == old_height )
        return; 
    if (height < GetMinimalAcceptableRowHeight()) 
        height = GetMinimalAcceptableRowHeight();
    
    GetSheetRefData()->m_rowEdges.SetSize(row, height);
    
    if ( !GetBatchCount() )
    {
        CalcWindowSizes(true);
        SetGridOrigin(GetGridOrigin(), true);
        RefreshBlock(wxSheetBlock(row, -1, GetNumberRows()-row+1, GetNumberCols()+1));
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

    const int old_width = GetColWidth(col);
    
    if ( width < 0 )
        width = GetCellBestSize(wxSheetCoords(-1, col)).x + 6;
    
    if ( width == old_width )
        return; 
    if ( width < GetMinimalAcceptableColWidth() ) 
        width = GetMinimalAcceptableColWidth();

    GetSheetRefData()->m_colEdges.SetSize(col, width);

    if ( !GetBatchCount() )
    {
        CalcWindowSizes(true);
        SetGridOrigin(GetGridOrigin(), true);
        RefreshBlock(wxSheetBlock(-1, col, GetNumberRows()+1, GetNumberCols()-col+1));
    }
}

int wxSheet::GetColWidth(int col) const
{
    if (col == -1)
        return GetRowLabelWidth();

    return GetSheetRefData()->m_colEdges.GetSize(col);
}
int wxSheet::GetColLeft(int col) const
{
    if (col == -1)  
        return 1; // 1 for border

    return GetSheetRefData()->m_colEdges.GetMin(col);
}
int wxSheet::GetColRight(int col) const
{
    if (col == -1)
        return GetRowLabelWidth();

    return GetSheetRefData()->m_colEdges.GetMax(col);
}

int wxSheet::GetRowHeight(int row) const
{
    if (row == -1)
        return GetColLabelHeight();

    return GetSheetRefData()->m_rowEdges.GetSize(row);
}
int wxSheet::GetRowTop(int row) const
{
    if (row == -1)
        return 1; // 1 for border
    
    return GetSheetRefData()->m_rowEdges.GetMin(row);
}
int wxSheet::GetRowBottom(int row) const
{
    if (row == -1)
        return GetColLabelHeight();
    
    return GetSheetRefData()->m_rowEdges.GetMax(row);
}

wxSize wxSheet::GetCellSize(const wxSheetCoords& coords) const
{
    return wxSize(GetColWidth(coords.m_row), GetRowHeight(coords.m_row));
}

bool wxSheet::IsCellShown( const wxSheetCoords& coords ) const
{
    wxCHECK_MSG(ContainsCell(coords), false, wxT("Invalid coords in wxSheet::IsCellShown"));
    return (GetColWidth(coords.GetCol()) > 0) && (GetRowHeight(coords.GetRow()) > 0);
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

int wxSheet::GetRowBestHeight(int row) const
{
    wxCHECK_MSG(ContainsGridRow(row), GetDefaultRowHeight(), wxT("Invalid coords in wxSheet::GetRowBestHeight"));
    wxClientDC dc(m_gridWin);
    wxCoord bestHeight = 0;
    const int numCols = GetNumberCols();
    wxSheetCoords coords( row, row );
    for ( coords.m_col = -1; coords.m_col < numCols; coords.m_col++ )
    {
        const int height = GetCellBestSize(coords, &dc).GetHeight();
        if ( height > bestHeight )
            bestHeight = height;
    }

    return bestHeight;
}

int wxSheet::GetColBestWidth(int col) const
{
    wxCHECK_MSG(ContainsGridCol(col), GetDefaultColWidth(), wxT("Invalid col in wxSheet::GetColBestWidth"));
    wxClientDC dc(m_gridWin);
    wxCoord bestWidth = 0;
    const int numRows = GetNumberRows();
    wxSheetCoords coords( col, col );
    for ( coords.m_row = -1; coords.m_row < numRows; coords.m_row++ )
    {
        const int width = GetCellBestSize(coords, &dc).GetWidth();
        if ( width > bestWidth )
            bestWidth = width;
    }

    return bestWidth;
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
    
    if ( width == 0 )
    {
        m_rowLabelWin->Show( false );
        m_cornerLabelWin->Show( false );
    }
    else if ( GetRowLabelWidth(false) > 0 )
    {
        GetSheetRefData()->m_rowLabelWidth = width;
        
        m_rowLabelWin->Show( true );
        if ( GetColLabelHeight() > 0 ) 
            m_cornerLabelWin->Show( true );
    }

    CalcWindowSizes();
}

void wxSheet::SetColLabelHeight( int height )
{
    height = wxMax( height, 0 );
    
    if ( height == 0 )
    {
        m_colLabelWin->Show( false );
        m_cornerLabelWin->Show( false );
    }
    else if ( GetColLabelHeight(false) > 0 )
    {
        GetSheetRefData()->m_colLabelHeight = height;
        
        m_colLabelWin->Show( true );
        if ( GetRowLabelWidth() > 0 ) 
            m_cornerLabelWin->Show( true );
    }

    CalcWindowSizes();
}

// ----------------------------------------------------------------------------
// Auto sizing of the row/col widths/heights

int wxSheet::AutoSizeRow( int row, bool setAsMin )
{
    wxCHECK_MSG(ContainsGridRow(row), 0, wxT("Invalid coords in wxSheet::AutoSizeRow"));
    int bestHeight = GetRowBestHeight(row);

    // if empty row - give default extent (notice that if extentMax is less
    // than default extent but != 0, it's ok)
    if ( bestHeight == 0 )
        bestHeight = GetDefaultRowHeight();
    else
        bestHeight += 6; // leave some space around text

    if (GetRowHeight(row) != bestHeight)
        SetRowHeight(row, bestHeight);
    if ( setAsMin )
        SetMinimalRowHeight(row, bestHeight);

    return bestHeight;
}
int wxSheet::AutoSizeCol( int col, bool setAsMin )
{
    wxCHECK_MSG(ContainsGridCol(col), 0, wxT("Invalid col in wxSheet::AutoSizeCol"));
    int bestWidth = GetColBestWidth(col);

    if ( bestWidth == 0 )
        bestWidth = GetDefaultColWidth();
    else
        bestWidth += 10; // leave some space around text

    if (GetColWidth(col) != bestWidth)
        SetColWidth(col, bestWidth);
    if ( setAsMin )
        SetMinimalColWidth(col, bestWidth);

    return bestWidth;
}

void wxSheet::AutoSizeRows(bool setAsMin)
{
    BeginBatch();

    const int numRows = GetNumberRows();
    for ( int row = 0; row < numRows; row++ )
        AutoSizeRow(row, setAsMin);

    EndBatch();
}
void wxSheet::AutoSizeCols(bool setAsMin)
{
    BeginBatch();

    const int numCols = GetNumberCols();
    for ( int col = 0; col < numCols; col++ )
        AutoSizeCol(col, setAsMin);

    EndBatch();
}

void wxSheet::AutoSize(bool setAsMin)
{
    BeginBatch();

    SetMargins(0, 0);
    AutoSizeRows(setAsMin);
    AutoSizeCols(setAsMin);
    
    wxSize sizeFit(GetGridVirtualSize());
    sizeFit.x += GetRowLabelWidth();
    sizeFit.y += GetColLabelHeight();
    
    wxSize winSize(GetClientSize());

    // distribute the extra space between the columns/rows to avoid having
    // extra white space
    const int numCols = GetNumberCols();
    const int numRows = GetNumberRows();
    
    // Remove the extra m_extraWidth + 1 added above
    wxCoord diff = winSize.x - sizeFit.x;
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
    diff = winSize.y - sizeFit.y;
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
        DisableCellEditControl(true);

    // autosize row height depending on label text
    const wxSheetCoords coords(row, -1);
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
        DisableCellEditControl(true);

    // autosize column width depending on label text
    const wxSheetCoords coords(-1, col);
    wxSize size(GetCellBestSize(coords));
    
    if (size.x < GetDefaultColWidth())
        size.x = GetDefaultColWidth();
    
    SetColWidth(col, size.x);
}

void wxSheet::SetEqualColWidths(int min_width)
{
    GetSheetRefData()->m_equal_col_widths = wxMax(min_width, 0);
    const int numCols = GetNumberCols();
    
    // don't fail here, since EVT_SIZEs are generated before the grid is 
    if ((min_width > 0) && (numCols > 0) && GetTable() && m_gridWin)
    {
        const int colwidth = m_gridWin->GetClientSize().GetWidth()/numCols;
        SetDefaultColWidth( colwidth, true );
    }
}

// ----------------------------------------------------------------------------
// Grid line and cell highlight colouring

void wxSheet::EnableGridLines( int dir )
{
    GetSheetRefData()->m_gridLinesEnabled = dir;
    RefreshGridWindow(false);
}

void wxSheet::SetGridLineColour( const wxColour& colour )
{
    wxCHECK_RET(colour.Ok(), wxT("Invalid colour in wxSheet::SetGridLineColour"));   
    GetSheetRefData()->m_gridLineColour = colour;
    RefreshGridWindow(false);
}

void wxSheet::SetCursorCellHighlightColour( const wxColour& colour )
{
    wxCHECK_RET(colour.Ok(), wxT("Invalid colour in wxSheet::SetGridLineColour"));      
    GetSheetRefData()->m_cursorCellHighlightColour = colour;

    if (ContainsCell(GetGridCursorCell()))
        RefreshCell(GetGridCursorCell(), true);
}

void wxSheet::SetCursorCellHighlightPenWidth(int width)
{
    wxCHECK_RET(width >= 0, wxT("Invalid pen width in wxSheet::SetGridLineColour"));       
    GetSheetRefData()->m_cursorCellHighlightPenWidth = width;

    if (ContainsCell(GetGridCursorCell()))
        RefreshCell(GetGridCursorCell(), true);
}

void wxSheet::SetCursorCellHighlightROPenWidth(int width)
{
    wxCHECK_RET(width >= 0, wxT("Invalid pen width in wxSheet::SetCursorCellHighlightROPenWidth"));   
    GetSheetRefData()->m_cursorCellHighlightROPenWidth = width;

    if (ContainsCell(GetGridCursorCell()))
        RefreshCell(GetGridCursorCell(), true);
}

void wxSheet::SetLabelOutlineColour( const wxColour& colour )
{
    wxCHECK_RET(colour.Ok(), wxT("Invalid colour in wxSheet::SetLabelOutlineColour"));   
    GetSheetRefData()->m_labelOutlineColour = colour;

    RefreshColLabelWindow(true);
    RefreshRowLabelWindow(true);
    RefreshCornerLabelWindow(true);
}

// ----------------------------------------------------------------------------
// Spanned cells

bool wxSheet::HasSpannedCells() const
{
    return GetTable() && GetTable()->HasSpannedCells(); 
}
wxSheetCoords wxSheet::GetCellOwner( const wxSheetCoords& coords ) const
{
    return GetCellBlock(coords).GetLeftTop();
}
wxSheetBlock wxSheet::GetCellBlock( const wxSheetCoords& coords ) const
{
    return GetTable() ? GetTable()->GetCellBlock(coords) : wxSheetBlock(coords, 1, 1);
}
wxSheetCoords wxSheet::GetCellSpan( const wxSheetCoords& coords ) const
{
    const wxSheetBlock block(GetCellBlock(coords));
    if (coords == block.GetLeftTop())
        return block.GetSize();
    else
        return block.GetLeftTop() - coords;
}
void wxSheet::SetCellSpan( const wxSheetBlock& block )
{
    wxCHECK_RET(GetTable(), wxT("The wxSheetTable doesn't exist to SetCellSpan"));
    GetTable()->SetCellSpan(block);
}

// ****************************************************************************

wxSheetCellAttr wxSheet::GetOrCreateAttr(const wxSheetCoords& coords, wxSheetAttr_Type type) const
{
    wxSheetCellAttr attr;
    wxCHECK_MSG( type != wxSHEET_AttrAny, attr, wxT("Cannot create attribute of type wxSHEET_AttrAny") );
    
    if (IsGridCell(coords))
    {
        switch (type)
        {
            case wxSHEET_AttrDefault : return DoGetDefaultGridAttr();
            case wxSHEET_AttrCell    :
            case wxSHEET_AttrRow     :
            case wxSHEET_AttrCol     :
            {
                wxCHECK_MSG(GetTable() && ContainsGridCell(coords), attr, wxT("Invalid table or attr coords"));
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
                wxCHECK_MSG(GetTable() && ContainsRowLabelCell(coords), attr, wxT("Invalid table or attr coords"));
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
                wxCHECK_MSG(GetTable() && ContainsColLabelCell(coords), attr, wxT("Invalid table or attr coords"));
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
                wxSheetCellAttr attr;
                if (GetTable())
                    attr = GetTable()->GetAttr(coords, type);
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
                wxSheetCellAttr attr;
                if (GetTable())
                    attr = GetTable()->GetAttr(coords, type);
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
    return wxSheetCellAttr(true);
}

void wxSheet::SetAttr(const wxSheetCoords& coords, 
                      const wxSheetCellAttr& attr_, wxSheetAttr_Type type)
{
    wxCHECK_RET(attr_.Ok(), wxT("Invalid attribute in wxSheet::SetDefaultAttr"));
    wxCHECK_RET( type != wxSHEET_AttrAny, wxT("Cannot create attribute of type wxSHEET_AttrAny") );
    wxSheetCellAttr attr(attr_);
    ClearAttrCache();

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
                wxCHECK_RET(GetTable() && ContainsGridCell(coords), wxT("Invalid table or attr coords"));
                if (attr.Ok())
                    InitAttr(attr, DoGetDefaultGridAttr());
     
                GetTable()->SetAttr(coords, attr, type);
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
                wxCHECK_RET(GetTable() && ContainsRowLabelCell(coords), wxT("Invalid table or attr coords"));
                if (attr.Ok())
                    InitAttr(attr, DoGetDefaultRowLabelAttr());
        
                GetTable()->SetAttr(coords, attr, type);
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
                wxCHECK_RET(GetTable() && ContainsColLabelCell(coords), wxT("Invalid table or attr coords"));
                if (attr.Ok())
                    InitAttr(attr, DoGetDefaultColLabelAttr());
      
                GetTable()->SetAttr(coords, attr, type);
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
        return false;
    
    wxSheetCellAttr attr(initAttr);
    wxSheetCellAttr attrDef(attr.GetDefaultAttr());
    // only 100000 def attr, should be enough?
    for (int n=0; n<100000; n++)
    {
        if (!attrDef.Ok())
        {
            attr.SetDefaultAttr(defAttr);
            return true;
        }
        else if (attrDef == defAttr)
        {
            return false; // already set
        }

        attr = attrDef;
        attrDef = attr.GetDefaultAttr();
    }
    
    wxFAIL_MSG(wxT("Unable to set default attribute for cell"));
    return false;
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
bool wxSheet::GetAttrShowEditor( const wxSheetCoords& coords, wxSheetAttr_Type type ) const
{
    return GetAttr(coords, type).GetShowEditor();
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
void wxSheet::SetAttrForegroundColour( const wxSheetCoords& coords, const wxColour& colour, wxSheetAttr_Type type )
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
void wxSheet::SetAttrShowEditor( const wxSheetCoords& coords, bool show_editor, wxSheetAttr_Type type )
{
    GetOrCreateAttr(coords, type).SetShowEditor(show_editor);
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
    
    return wxEmptyString;
}

void wxSheet::SetCellValue( const wxSheetCoords& coords, const wxString& s )
{
    wxCHECK_RET( GetTable(), wxT("Invalid sheet table for SetCellValue"));
    GetTable()->SetValue( coords, s );

    // Note: If we are using IsCellEditControlEnabled,
    // this interacts badly with calling SetCellValue from
    // an EVT_SHEET_CELL_CHANGE handler.
    if ((GetEditControlCoords() == coords) && IsCellEditControlShown())
    {
        HideCellEditControl();
        ShowCellEditControl(); // will reread data from table
    }   
    else
        RefreshCell(coords, false);
}

bool wxSheet::HasCellValue( const wxSheetCoords& coords )
{
    if ( GetTable() )
        return GetTable()->HasValue( coords );
    
    // this tries the table too, but maybe they've only overridden GetCellValue
    return !GetCellValue(coords).IsEmpty();
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
    wxCHECK_MSG(ContainsCell(coords), false, wxT("Invalid coords in wxSheet::IsCellVisible"));
    
    const wxRect clientRect(wxPoint(0,0), GetWindowForCoords(coords)->GetClientSize());
    const wxRect devRect(CellToRect(coords, true));  // rect in device coords
    
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
    
    wxCHECK_MSG(ContainsGridRow(row), false, wxT("Invalid row in wxSheet::IsRowVisible"));
    
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
    
    wxCHECK_MSG(ContainsGridCol(col), false, wxT("Invalid row in wxSheet::IsColVisible"));
    
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
    
    const wxRect logRect( CellToRect(coords) );        // cell rect in logical coords
    const wxRect devRect( CalcScrolledRect(logRect) ); // rect in device coords

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
        SetGridOrigin( xpos, ypos, true, true );
}

void wxSheet::SetGridCursorCell( const wxSheetCoords& coords )
{   
    // Note: can set to invalid coords < 0 to hide cursor, but not out of range
    wxCHECK_RET((coords.GetRow() < GetNumberRows()) && 
                (coords.GetCol() < GetNumberCols()),
                wxT("Invalid coords in wxSheet::SetGridCursorCell"));
    
    if (GetGridCursorCell() == coords)
        return;
    
    m_waitForSlowClick = false;
    
    // the event has been intercepted - do nothing
    if (SendEvent(wxEVT_SHEET_SELECTING_CELL, coords) != EVT_SKIPPED)
        return;

    const wxSheetCoords oldCursorCell(GetGridCursorCell());
    if ( ContainsGridCell(oldCursorCell) )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(true);
        
        // Otherwise refresh redraws the highlight!
        GetSheetRefData()->m_cursorCoords = coords; 
        RefreshCell(oldCursorCell, true);
    }    

    GetSheetRefData()->m_cursorCoords = coords;
    
    if (ContainsGridCell(GetGridCursorCell()))
        RefreshCell(GetGridCursorCell(), true);
    
    SendEvent(wxEVT_SHEET_SELECTED_CELL, coords);
}

bool wxSheet::DoMoveCursor( const wxSheetCoords& relCoords, bool expandSelection )
{
    const wxSheetCoords cursorCoords(GetGridCursorCell());
    const wxSheetCoords coords( cursorCoords + relCoords );
    if ( (relCoords == wxSheetCoords(0,0)) || 
         !ContainsGridCell(cursorCoords) || !ContainsGridCell(coords) )
        return false;

    if ( expandSelection)
    {
        m_keySelecting = true;
        if ( !ContainsGridCell(GetSelectingAnchor()) )
            SetSelectingAnchor(cursorCoords);
    }
    else
        ClearSelection(true);
    
    MakeCellVisible( coords );
    SetGridCursorCell( coords );
    if (expandSelection && !HasSelectionMode(wxSHEET_SelectNone))
        HighlightSelectingBlock( GetSelectingAnchor(), GetGridCursorCell() );
        
    return true;
}

bool wxSheet::DoMoveCursorBlock( const wxSheetCoords& relDir, bool expandSelection )
{
    const wxSheetCoords cursorCoords(GetGridCursorCell());
    wxSheetCoords coords(cursorCoords + relDir);
    if ( !ContainsGridCell(cursorCoords) || !ContainsGridCell(coords) )
        return false;

    if ( !HasCellValue(cursorCoords) )
    {
        // starting in an empty cell: find the next block of non-empty cells
        while ( ContainsGridCell(coords) )
        {
            if (HasCellValue(coords) || !ContainsGridCell(coords + relDir))
                break;
                
            coords += relDir;
        }
    }
    else if ( !HasCellValue(coords) )
    {
        // starting at the edge of a block: find the next block
        while ( ContainsGridCell(coords + relDir) )
        {
            coords += relDir;
            if ( HasCellValue(coords) )
                break;
        }
    }
    else
    {
        // starting within a block with value: find the edge of the block
        while ( ContainsGridCell(coords + relDir) )
        {
            if ( !HasCellValue(coords + relDir) )
                break;
            
            coords += relDir;
        }
    }

    return DoMoveCursor(coords-cursorCoords, expandSelection);
}

bool wxSheet::DoMoveCursorUpDownPage( bool page_up, bool expandSelection )
{
    if (!ContainsGridCell(GetGridCursorCell())) 
        return false;

    const int row = GetGridCursorRow();
    
    if ((page_up && (row < 1)) || (!page_up && (row >= GetNumberRows() - 1)))
        return false;
    
    int cw, ch;
    m_gridWin->GetClientSize( &cw, &ch );

    int y = GetRowTop(row) + (page_up ? (-ch + 1) : ch);
    const int newRow = YToGridRow( y, true );
    return DoMoveCursor(wxSheetCoords(newRow - row, 0), expandSelection);
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
    // like you'd have to send a slew of confusing (de)selection events
    ClearSelection(true);
    GetSheetRefData()->m_selectionMode = selmode;
}

bool wxSheet::SelectRow( int row, bool addToSelected, bool sendEvt )
{
    if (HasSelectionMode(wxSHEET_SelectCols))
        return false;
    
    // selection to # of cols means whole row is selected
    return SelectBlock( wxSheetBlock(row, 0, 1, GetNumberCols()+1), 
                        addToSelected, sendEvt );
}
bool wxSheet::SelectRows( int rowTop, int rowBottom, bool addToSelected, bool sendEvt )
{
    if (HasSelectionMode(wxSHEET_SelectCols) || (rowTop > rowBottom))
        return false;
    
    // selection to # of cols means whole row is selected
    return SelectBlock( wxSheetBlock(rowTop, 0, rowBottom-rowTop+1, GetNumberCols()+1), 
                        addToSelected, sendEvt );
}
bool wxSheet::SelectCol( int col, bool addToSelected, bool sendEvt )
{
    if (HasSelectionMode(wxSHEET_SelectRows))
        return false;
    
    // selection to # of rows means whole col is selected
    return SelectBlock( wxSheetBlock(0, col, GetNumberRows()+1, 1), 
                        addToSelected, sendEvt );
}
bool wxSheet::SelectCols( int colLeft, int colRight, bool addToSelected, bool sendEvt )
{
    if (HasSelectionMode(wxSHEET_SelectRows) || (colLeft > colRight))
        return false;
    
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
        return false;
    
    if ( !addToSelected )
        ClearSelection(sendEvt);

    wxArraySheetBlock addedBlocks;
    if (!GetSelection()->SelectBlock( block, true, &addedBlocks ))
        return false;

    if (!GetBatchCount())
    {
        wxSheetBlock bounds;
        
        for (size_t n=0; n<addedBlocks.GetCount(); n++)
            bounds = bounds.ExpandUnion(addedBlocks[n]);
        
        RefreshGridCellBlock(bounds);
    }
    
    if (sendEvt)
        SendRangeEvent(wxEVT_SHEET_RANGE_SELECTED, block, true, addToSelected);
    
    return true;
}

bool wxSheet::SelectAll(bool sendEvt)
{
    BeginBatch();
    ClearSelection(false); // clear old and only have new, no event/refresh
    EndBatch(false);
    // select to # rows/col means everything everywhere is selected
    return SelectBlock(wxSheetBlock(0, 0, GetNumberRows(), GetNumberCols()),
                       false, sendEvt);
}

bool wxSheet::DeselectRow( int row, bool sendEvt )
{
    if (HasSelectionMode(wxSHEET_SelectCols))
        return false;
    
    // deselection to # of cols means whole row is deselected
    return DeselectBlock( wxSheetBlock(row, 0, 1, GetNumberCols()), sendEvt );
}
bool wxSheet::DeselectRows( int rowTop, int rowBottom, bool sendEvt )
{
    if (HasSelectionMode(wxSHEET_SelectCols) || (rowTop > rowBottom))
        return false;
    
    // selection to # of cols means whole row is deselected
    return DeselectBlock( wxSheetBlock(rowTop, 0, rowBottom-rowTop+1, GetNumberCols()+1), 
                          sendEvt );
}
bool wxSheet::DeselectCol( int col, bool sendEvt )
{
    if (HasSelectionMode(wxSHEET_SelectRows))
        return false;
    
    // deselection to # of rows means whole col is deselected
    return DeselectBlock( wxSheetBlock(0, col, GetNumberRows(), 1), sendEvt );
}
bool wxSheet::DeselectCols( int colLeft, int colRight, bool sendEvt )
{
    if (HasSelectionMode(wxSHEET_SelectRows) || (colLeft > colRight))
        return false;
    
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
        return false;
    
    // do they want to clear the whole grid
    wxSheetBlock gridBlock(0, 0, GetNumberRows(), GetNumberCols());
    if (gridBlock.Intersect(block) == gridBlock)
        return ClearSelection(sendEvt);
    
    wxArraySheetBlock deletedBlocks;
    if (!GetSelection()->DeselectBlock( block, true, &deletedBlocks ))
        return false;

    if (!GetBatchCount())
    {
        wxSheetBlock bounds;
        
        for (size_t n=0; n<deletedBlocks.GetCount(); n++)
            bounds = bounds.ExpandUnion(deletedBlocks[n]);
        
        RefreshGridCellBlock(bounds);
    }
    
    if (sendEvt)
        SendRangeEvent(wxEVT_SHEET_RANGE_SELECTED, block, false, false);
    
    return true;
}

bool wxSheet::ClearSelection(bool send_event)
{
    if ( !HasSelection() )
        return false;

    // no evt for clearing m_selectingBlock, it's not a real selection
    if ( !HasSelection(false) )
        send_event = false;
    
    wxSheetBlock bounds(GetSelectingBlock().ExpandUnion(GetSelection()->GetBoundingBlock()));
    SetSelectingBlock(wxNullSheetBlock);
    GetSelection()->Clear(); 
    
    RefreshGridCellBlock(bounds);
    
    if (send_event)
    {
        // One deselection event, indicating deselection of _all_ cells.
        SendRangeEvent( wxEVT_SHEET_RANGE_SELECTED, 
                        wxSheetBlock(0, 0, GetNumberRows(), GetNumberCols()),
                        false, false );
    }
    
    return true;
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

class wxSheetDataObject : public wxDataObjectSimple
{
public:
    wxSheetDataObject() : wxDataObjectSimple(wxDF_wxSHEET) {}
    wxSheetDataObject(const wxSheetValueProviderSparseString &values)
        : wxDataObjectSimple(wxDF_wxSHEET), m_values(values)
        {
            CreateDataString(); 
            m_values.ClearValues();
        }
        
    virtual size_t GetDataSize() const { return (m_data.Len()+1)*sizeof(wxChar); }
    virtual bool GetDataHere(void *buf) const;
    virtual bool SetData(size_t len, const void *buf);
    
    const wxSheetValueProviderSparseString& GetValues() const { return m_values; }
 
    void CreateDataString();
    
protected:
    wxSheetValueProviderSparseString m_values;    
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

//const wxString s_nextRow(wxT("s")); // just increment row number
//const wxString s_nextCol(wxT("d")); // end of value marker
//const wxString s_newRow(wxT("r"));  // specify a row number
//const wxString s_newCol(wxT("c"));  // specify a col number
// Data string looks like this: `r23c12:size:some data string`
//  'r' followed by a number specifies a row #
//      without a number means next row
//  'c' followed by a number specifies a col #
//      without a number means next col
//  size is the number of chars of the data string

// Data string looks like this: start by specifying the row and col number
// then data and end data marker (s_nextCol) more data, if you skip cols
// then specify the col number with s_newCol.
// [s_newRow<row>:][s_newCol<col>:]hello[s_nextCol]text[s_nextCol][s_newCol<colNum>:]More[s_nextCol]

void wxSheetDataObject::CreateDataString()
{
    m_data.Clear();    
    wxPairArrayIntPairArraySheetString &values = m_values.GetData();
    m_values.RemoveEmpty();

    if (values.GetCount() == 0u)
        return;
    
    int c, ncols, r, nrows = values.GetCount();
    int row, col;
    int last_row = values.GetItemKey(0) - 10, last_col;
    
    for (r=0; r<nrows; r++)
    {
        row = values.GetItemKey(r);
        if (last_row+1 < row)
            m_data += s_newRow + wxString::Format(wxT("%d:"), row);
        else
            m_data += s_nextRow;
        
        col = values.GetItemValue(r).GetItemKey(0);
        m_data += s_newCol + wxString::Format(wxT("%d:"), col);
        last_col = col;
        
        ncols = values.GetItemValue(r).GetCount();
        for (c=0; c<ncols; c++)
        {
            col = values.GetItemValue(r).GetItemKey(c);
            if (last_col+1 < col)
                m_data += s_newCol + wxString::Format(wxT("%d:"), col);

            m_data += values.GetItemValue(r).GetItemValue(c) + s_nextCol;
            last_col = col;
        }
    }
}

bool wxSheetDataObject::GetDataHere(void *buf) const 
{
    size_t len = GetDataSize();
    memset( (char *)buf, 0, len );
    memcpy( buf, m_data.c_str(), len );
    return true;
}    

bool wxSheetDataObject::SetData(size_t len, const void *buf)
{
    wxPairArrayIntPairArraySheetString &values = m_values.GetData();
    values.Clear();
    m_data.Clear();
    
    if (len < 2u)
        return false; // I guess?
    
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
                return false;
            
            last_n = n+1;
        }
        // read the next col value 
        else if ( (*c == s_nextCol[0]) && (*(c+1) == s_nextCol[1]) ) // s_nextCol
        {
            wxString strVal(strBuf.Mid(last_n, n-last_n));
            values.GetOrCreateValue(row).SetValue(col, strVal);
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
                return false;
            
            last_n = n+1;
        }
    }
    
    //wxPrintf(wxT("Got some data %d %d\n"), len, m_values.GetCount());
    return true;
}

bool wxSheet::CopyInternalSelectionToClipboard(const wxChar& colSep)
{
    if (!wxTheClipboard->Open()) 
        return false;
    
    // save data to clipboard, we want the internal data back if possible
    wxDataObjectComposite *data = new wxDataObjectComposite;
    data->Add(new wxSheetDataObject(GetSheetRefData()->m_copiedData), true);
    data->Add(new wxTextDataObject(CopyInternalSelectionToString(colSep)), false);
    wxTheClipboard->SetData( data );
    
    wxTheClipboard->Close();
    return true;
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
        return false;

    bool ret = false;
    
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
    wxPairArrayIntPairArraySheetString &copiedData = GetSheetRefData()->m_copiedData.GetData();
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
            return false;
    }
    
    wxSheetSelectionIterator selIter(sel);
    wxSheetCoords cell;
    while (selIter.GetNext(cell))
        copiedData.GetOrCreateValue(cell.m_row).SetValue(cell.m_col, GetCellValue(cell));
    
    return copiedData.GetCount() > 0;
}

wxString wxSheet::CopyInternalSelectionToString(const wxChar& colSep)
{
    wxPairArrayIntPairArraySheetString &copiedData = GetSheetRefData()->m_copiedData.GetData();
    wxString value;
    GetSheetRefData()->m_copiedData.RemoveEmpty();
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
    wxPairArrayIntPairArraySheetString &copiedData = GetSheetRefData()->m_copiedData.GetData();
    copiedData.Clear();

    if (string.IsEmpty())
        return false;
    
    const wxChar *c = string.GetData();
    int n, len = string.Length();
    
    int row = 0, col = 0;
    wxString buf;
    
    for (n=0; n<len; n++, c++)
    {
        if (((*c) == wxT('\r')) || ((*c) == wxT('\n')))
        {
            // check for DOS's \r\n and discard the trailing \n
            if (((*c) == wxT('\r')) && (n < len - 1) && (c[1] == wxT('\n')))
            {
                c++;   
                n++;
            }
            
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
    
    return true;
}

bool wxSheet::PasteInternalCopiedSelection(const wxSheetCoords &topLeft_)
{
    wxPairArrayIntPairArraySheetString &copiedData = GetSheetRefData()->m_copiedData.GetData();
    // clean up, shouldn't happen but don't want to have to check it later
    GetSheetRefData()->m_copiedData.RemoveEmpty();
    if (copiedData.GetCount() == 0) return false;

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
        return false;
    
    wxSheetCoords copiedTopLeft( copiedData.GetItemKey(0), 
                                 copiedData.GetItemValue(0).GetItemKey(0) );
    
    wxSheetCoords shiftCell = topLeft - copiedTopLeft;
    
    GetSheetRefData()->m_pasting = true;
    
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
    
    GetSheetRefData()->m_pasting = false;
    return true;
}

// ----------------------------------------------------------------------------
// Edit control functions (mostly used internally)

void wxSheet::EnableEditing( bool edit )
{
    // TODO: improve this ?
    if ( edit != IsEditable() )
    {
        if (!edit && IsCellEditControlCreated()) 
            DisableCellEditControl(true);
        
        GetSheetRefData()->m_editable = edit;
    }
}

bool wxSheet::EnableCellEditControl( const wxSheetCoords& coords_ )
{
    // move to owner cell since that's where the editor is
    const wxSheetCoords coords(GetCellOwner(coords_));
    wxCHECK_MSG(CanEnableCellControl(coords), false, _T("can't enable editing for this cell!"));

    // already editing elsewhere, disable it
    if (IsCellEditControlCreated() && !DisableCellEditControl(true))
        return false;
    
    if (SendEvent( wxEVT_SHEET_EDITOR_ENABLED, coords ) == EVT_VETOED)
        return false;

    // guarantee that it's visible
    MakeCellVisible(coords);
    GetSheetRefData()->m_cellEditorCoords = coords;
    
    // Get the appropriate editor for this cell
    wxSheetCellAttr attr(GetAttr(coords));
    GetSheetRefData()->m_cellEditor = attr.GetEditor(this, coords);
    if (!GetEditControl().Ok())
    {
        GetSheetRefData()->m_cellEditorCoords = wxNullSheetCoords;
        wxFAIL_MSG(wxT("Unable to get cell edit control"));
        return false;
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
        return true;
    }

    if (SendEvent(wxEVT_SHEET_EDITOR_DISABLED, GetEditControlCoords()) == EVT_VETOED)
        return false;
    
    if (save_value) 
        SaveEditControlValue(); 
    
    HideCellEditControl();
    // FIXME always destroy it else memory leak, maybe fixed with handler OnDestroy
    GetSheetRefData()->m_cellEditor.DestroyControl(); 
    GetSheetRefData()->m_cellEditor.Destroy();
    GetSheetRefData()->m_cellEditorCoords = wxNullSheetCoords;
    return true;
}

bool wxSheet::CanEnableCellControl(const wxSheetCoords& coords) const
{
    wxCHECK_MSG(ContainsCell(coords), false, wxT("Invalid coords"));
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
    wxCHECK_MSG(CanEnableCellControl(GetEditControlCoords()), false, // also checks coords
                wxT("Editor not enabled in wxSheet::ShowCellEditControl"));
    
    wxWindow *win = GetWindowForCoords(GetEditControlCoords());
    
    // If the control's parent is not correct we must recreate it 
    if (GetEditControl().GetControl() && (GetEditControl().GetControl()->GetParent() != win))
        GetSheetRefData()->m_cellEditor.DestroyControl();
    
    // Make sure the editor is created
    if (!GetEditControl().IsCreated())
    {
        GetSheetRefData()->m_cellEditor.CreateEditor(win, -1,
                               new wxSheetCellEditorEvtHandler(this, GetEditControl()),
                               this);

        wxSheetEditorCreatedEvent evt(GetId(), wxEVT_SHEET_EDITOR_CREATED,
                                      this, GetEditControlCoords(), 
                                      GetSheetRefData()->m_cellEditor.GetControl());
        GetEventHandler()->ProcessEvent(evt);
        
        if (!GetEditControl().IsCreated())
        {
            // kill it all off, something went wrong
            if (GetEditControl().GetControl())
                GetSheetRefData()->m_cellEditor.DestroyControl();
            
            GetSheetRefData()->m_cellEditor.Destroy();
            GetSheetRefData()->m_cellEditorCoords = wxNullSheetCoords;
            wxFAIL_MSG(wxT("Unable to create edit control"));
            return false;
        }
    }
    
    wxSheetCellAttr attr(GetAttr(GetEditControlCoords()));

    // the rectangle bounding the cell
    wxRect rect( CellToRect(GetEditControlCoords(), true) );

    // FIXME use GetBestSize

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
    GetSheetRefData()->m_cellEditor.Show( true, attr );

    // recalc dimensions, maybe expand the scrolled window to account for editor
    CalcWindowSizes();

    GetSheetRefData()->m_cellEditor.BeginEdit(GetEditControlCoords(), this);
    
    // FIXME other spreadsheets don't clear cells to right, it looks weird? why not?
    // if this is empty, cell to right maybe overflowed into from left, clear it
    //if (value.IsEmpty() && (m_cellEditorCoords.m_col < m_numCols - 1))
    //    RefreshCell(m_cellEditorCoords+wxSheetCoords(0, 1));
    
    return true;
}

bool wxSheet::HideCellEditControl()
{
    wxCHECK_MSG(IsCellEditControlShown(), false, wxT("Edit control not shown in wxSheet::HideCellEditControl"));

    GetSheetRefData()->m_cellEditor.Show( false, GetAttr(GetEditControlCoords()) );
    m_gridWin->SetFocus();
    RefreshCell(GetEditControlCoords(), false);
    return true;
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
            Refresh(true);
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
        const wxRect rect(*rect_);
        const int rowLabelWidth  = GetRowLabelWidth();
        const int colLabelHeight = GetColLabelHeight();
        const wxSize extentSize(GetGridExtent());

        wxRect cornerRect(0, 0, rowLabelWidth, colLabelHeight);
        cornerRect.Intersect(rect);

        wxRect rowLabelRect(0, colLabelHeight, rowLabelWidth, extentSize.y);
        rowLabelRect.Intersect(rect);
        rowLabelRect.y -= colLabelHeight;

        wxRect colLabelRect(rowLabelWidth, 0, extentSize.x, colLabelHeight);
        colLabelRect.Intersect(rect);
        colLabelRect.x -= rowLabelWidth;

        wxRect gridRect(rowLabelWidth, colLabelHeight, extentSize.x, extentSize.y);
        gridRect.Intersect(rect);
        gridRect.x -= rowLabelWidth;
        gridRect.y -= colLabelHeight;

        if ( !wxRectIsEmpty(cornerRect) ) 
            RefreshCornerLabelWindow(eraseb, &cornerRect);
        if ( !wxRectIsEmpty(rowLabelRect) )
            RefreshRowLabelWindow(eraseb, &rowLabelRect);
        if ( !wxRectIsEmpty(colLabelRect) ) 
            RefreshColLabelWindow(eraseb, &colLabelRect);
        if ( !wxRectIsEmpty(gridRect) )
            RefreshGridWindow(eraseb, &gridRect);
    }
    else
    {
        RefreshCornerLabelWindow(eraseb, NULL);
        RefreshRowLabelWindow(eraseb, NULL);
        RefreshColLabelWindow(eraseb, NULL);
        RefreshGridWindow(eraseb, NULL);
    }
}
void wxSheet::RefreshGridWindow(bool eraseb, const wxRect* rect)
{
    if (GetBatchCount())
        return;

    size_t n, count = GetSheetRefData()->GetSheetCount();
    for (n=0; n<count; n++)
    {
        wxSheet* s = GetSheetRefData()->GetSheet(n);
        wxWindow *win = (wxWindow*)s->GetGridWindow();
        if (win && win->IsShown())
        {
            if (rect)
            {
                wxRect scrolledRect(s->CalcScrolledRect(*rect));
                const wxRect refedRect(wxPoint(0,0), win->GetSize());
                scrolledRect.Intersect(refedRect);
                if (!wxRectIsEmpty(scrolledRect))
                    win->Refresh(eraseb, &scrolledRect);
            }
            else
                win->Refresh(eraseb, NULL);
        }
    }
}
void wxSheet::RefreshRowLabelWindow(bool eraseb, const wxRect* rect)
{
    if (GetBatchCount())
        return;

    size_t n, count = GetSheetRefData()->GetSheetCount();
    for (n=0; n<count; n++)
    {
        wxSheet* s = GetSheetRefData()->GetSheet(n);
        wxWindow *win = (wxWindow*)s->GetRowLabelWindow();
        if (win && win->IsShown())
        {
            if (rect)
            {
                wxRect scrolledRect(*rect);
                s->CalcScrolledPosition(0, scrolledRect.y, NULL, &scrolledRect.y);
                const wxRect refedRect(wxPoint(0,0), win->GetSize());
                scrolledRect.Intersect(refedRect);
                if (!wxRectIsEmpty(scrolledRect))
                    win->Refresh(eraseb, &scrolledRect);
            }
            else
                win->Refresh(eraseb, NULL);
        }
    }    
}
void wxSheet::RefreshColLabelWindow(bool eraseb, const wxRect* rect)
{
    if (GetBatchCount())
        return;

    size_t n, count = GetSheetRefData()->GetSheetCount();
    for (n=0; n<count; n++)
    {
        wxSheet* s = GetSheetRefData()->GetSheet(n);
        wxWindow *win = (wxWindow*)s->GetColLabelWindow();
        if (win && win->IsShown())
        {
            if (rect)
            {
                wxRect scrolledRect(*rect);
                s->CalcScrolledPosition(scrolledRect.x, 0, &scrolledRect.x, NULL);
                const wxRect refedRect(wxPoint(0,0), win->GetSize());
                scrolledRect.Intersect(refedRect);
                if (!wxRectIsEmpty(scrolledRect))
                    win->Refresh(eraseb, &scrolledRect);
            }
            else
                win->Refresh(eraseb, NULL);
        }
    }    
}
void wxSheet::RefreshCornerLabelWindow(bool eraseb, const wxRect* rect)
{
    if (GetBatchCount())
        return;

    size_t n, count = GetSheetRefData()->GetSheetCount();
    for (n=0; n<count; n++)
    {
        wxSheet* s = GetSheetRefData()->GetSheet(n);
        wxWindow *win = (wxWindow*)s->GetCornerLabelWindow();
        if (win && win->IsShown())
            win->Refresh(eraseb, rect);
    }
}

void wxSheet::RefreshCell(const wxSheetCoords& coords, bool single_cell)
{
    if ( GetBatchCount() )
        return;

    //wxPrintf(wxT("RefreshCell %d %d\n"), coords.m_row, coords.m_col);
    
    if (IsCornerLabelCell(coords))
    {
        RefreshCornerLabelWindow(true);
        return;
    }
    
    wxRect rect(CellToRect(coords, false));
    
    if (ContainsRowLabelCell(coords))
    {
        RefreshRowLabelWindow( true, &rect );
        return;
    }
    else if (ContainsColLabelCell(coords))
    {
        RefreshColLabelWindow( true, &rect );
        return;
    }
    else if (ContainsGridCell(coords))
    {   
        if (!single_cell)
        {
            // hack to draw previous cell, if this cell turns empty and can be
            // overflowed into we need to erase the previous cell's |> cutoff marker
            if (coords.m_col > 0)
                rect.x = GetColLeft(coords.m_col-1);
            // we do have to draw the whole row right though
            rect.width = GetVirtualSize().x - rect.x; 
        }
    
        RefreshGridWindow( false, &rect );
    }
    else
        wxFAIL_MSG(wxT("Invalid coords in wxSheet::RefreshCell"));
}
void wxSheet::RefreshBlock(const wxSheetBlock& block)
{
    if ( GetBatchCount() || block.IsEmpty() )
        return;

    //PRINT_BLOCK(wxT("RefreshBlock "), block)
    
    // This function also refreshes beyond the labels and grid since if a 
    //   cell is resized smaller that may have to be repainted

    wxSheetBlock b;
    const wxSheetCoords coords(block.GetLeftTop());
    const wxSize winSize(GetGridExtent());
    const int numRows = GetNumberRows();
    const int numCols = GetNumberCols();

    // Corner Labels
    if (IsCornerLabelCell(coords))
    {
        RefreshCornerLabelWindow(true);
    }
    // Row Labels
    b = block.Intersect(wxSheetBlock(0, -1, numRows, 1));
    if (!b.IsEmpty())
    {
        wxRect rect(BlockToLogicalRect(b));
        if (b.GetBottom() == numRows - 1)
            rect.height = winSize.y - rect.y;
        RefreshRowLabelWindow( true, &rect );
    }
    // Col Labels
    b = block.Intersect(wxSheetBlock(-1, 0, 1, numCols));
    if (!b.IsEmpty())
    {
        wxRect rect(BlockToLogicalRect(b));
        if (b.GetRight() == numCols - 1)
            rect.width = winSize.x - rect.x;
        RefreshColLabelWindow( true, &rect );
    }
    // Grid cells
    b = block.Intersect(wxSheetBlock(0, 0, numRows, numCols));
    if (!b.IsEmpty())
    {
        wxRect rect(BlockToLogicalRect(b));
        if (b.GetBottom() == numRows - 1)
            rect.height = winSize.y - rect.y;
        if (b.GetRight() == numCols - 1)
            rect.width = winSize.x - rect.x;
        RefreshGridWindow( false, &rect );
    }
}
void wxSheet::RefreshRow( int row )
{
    RefreshBlock(wxSheetBlock(row, 0, 1, GetNumberCols()));
}
void wxSheet::RefreshCol( int col )
{
    RefreshBlock(wxSheetBlock(0, col, GetNumberRows(), 1));
}
void wxSheet::RefreshGridCellBlock( const wxSheetBlock& block )
{
    RefreshBlock(block);
}

void wxSheet::RefreshAttrChange(const wxSheetCoords& coords, wxSheetAttr_Type type)
{
    switch (GetCellCoordsType(coords))
    {
        case wxSHEET_CELL_GRID :
        {
            switch (type)
            {
                case wxSHEET_AttrDefault : 
                {
                    m_gridWin->SetBackgroundColour(DoGetDefaultGridAttr().GetBackgroundColour());
                    RefreshGridWindow(false);
                    break;
                }
                case wxSHEET_AttrCol :
                {
                    if (ContainsGridCol(coords.m_col)) RefreshCol(coords.m_col);
                    break;
                }
                case wxSHEET_AttrRow :
                {
                    if (ContainsGridRow(coords.m_row)) RefreshRow(coords.m_row);
                    break;
                }
                case wxSHEET_AttrCell :
                default               : 
                {
                    if (ContainsGridCell(coords)) RefreshCell(coords, false);
                    break;
                }
            }
            break;
        }
        case wxSHEET_CELL_CORNERLABEL :
        {
            m_cornerLabelWin->SetBackgroundColour(DoGetDefaultCornerLabelAttr().GetBackgroundColour());
            RefreshCornerLabelWindow(true);
            break;
        }
        case wxSHEET_CELL_ROWLABEL :
        {
            switch (type)
            {
                case wxSHEET_AttrDefault :
                {    
                    m_rowLabelWin->SetBackgroundColour(DoGetDefaultRowLabelAttr().GetBackgroundColour());
                    RefreshRowLabelWindow(true);
                    break;
                }
                case wxSHEET_AttrCell :
                default               : 
                {
                    if (ContainsRowLabelCell(coords))
                        RefreshCell(coords);
                    break;
                }
            }
            break;
        }
        case wxSHEET_CELL_COLLABEL :
        {
            switch (type)
            {
                case wxSHEET_AttrDefault :
                {    
                    m_colLabelWin->SetBackgroundColour(DoGetDefaultColLabelAttr().GetBackgroundColour());
                    RefreshColLabelWindow(true);
                    break;
                }
                case wxSHEET_AttrCell :
                default               : 
                {
                    if (ContainsColLabelCell(coords))
                        RefreshCell(coords);
                    break;
                }
            }
            break;
        }
        default : break;
    }
}

void wxSheet::OnPaint( wxPaintEvent& event )
{
    wxWindow *win = (wxWindow*)event.GetEventObject();
    wxPaintDC dc(win);
    wxRegion reg(win->GetUpdateRegion());
    
    if (!IsShown()) return;
    
    if (win == this)
    {
        PaintSheetWindow( dc, reg );
    }
    else if (win == m_cornerLabelWin)
    {
        if (m_cornerLabelWin->IsShown())
        {
            PaintCornerLabelWindow( dc, reg );
        }
    }
    else if (win == m_rowLabelWin)
    {
        if (m_rowLabelWin->IsShown())
        {
            PrepareRowLabelDC( dc );
            PaintRowLabelWindow( dc, reg );
        }
    }
    else if (win == m_colLabelWin)
    {
        if (m_colLabelWin->IsShown())
        {
            PrepareColLabelDC( dc );
            PaintColLabelWindow( dc, reg );
        }
    }
    else if (win == m_gridWin)
    {
        if (m_gridWin->IsShown())
        {
            PrepareGridDC( dc );
            PaintGridWindow( dc, reg );
        }
    }
}

void wxSheet::OnEraseBackground( wxEraseEvent& event )
{
    wxWindow *win = (wxWindow*)event.GetEventObject();
   
    if (win == this)
        event.Skip(false);
    else if ((win == m_cornerLabelWin) || (win == m_rowLabelWin) || (win == m_colLabelWin))
        event.Skip(false);
    else if (win == m_gridWin)
        event.Skip(false);
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

void wxSheet::PaintSheetWindow( wxDC& dc, const wxRegion& WXUNUSED(reg) )
{
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

void wxSheet::PaintGridWindow( wxDC& dc, const wxRegion& reg )
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
void wxSheet::PaintRowLabelWindow( wxDC& dc, const wxRegion& reg )
{
    const int numRows = GetNumberRows();
    if (0 && numRows)
    {

        wxRect r(0, GetRowBottom(numRows-1), GetRowLabelWidth(), GetGridVirtualSize(true).y);
        r = CalcScrolledRect(r);
        r.Intersect(reg.GetBox());
        dc.DrawRectangle(r);
    }

    wxArrayInt rows;
    if (CalcRowLabelsExposed(reg, rows))
        DrawRowLabels( dc, rows );

}
void wxSheet::PaintColLabelWindow( wxDC& dc, const wxRegion& reg )
{
    const int numCols = GetNumberCols();
    if (0 && numCols)
    {
        wxRect r(GetColRight(numCols-1), 0, GetGridVirtualSize(true).x, GetColLabelHeight());
        r = CalcScrolledRect(r);
        r.Intersect(reg.GetBox());
        dc.DrawRectangle(r);
    }

    wxArrayInt cols;
    if (CalcColLabelsExposed(reg, cols))
        DrawColLabels( dc, cols );
}
void wxSheet::PaintCornerLabelWindow( wxDC& dc, const wxRegion& WXUNUSED(reg) )
{
    DrawCornerLabel( dc );
}

int wxSheet::FindOverflowCell( const wxSheetCoords& coords, wxDC& dc )
{    
    wxSheetCoords c(coords);
    while (c.m_col > 0) 
    {
        c.m_col = GetTable()->GetFirstNonEmptyColToLeft(c);
        if (c.m_col < 0)
            return -1;

        // if !empty, stop, this is the only cell that could overflow
        // Must check since default implementation of 
        // GetFirstNonEmptyColToLeft just returns the previous col
        if (HasCellValue(c))
        {
            wxSheetCellAttr attr(GetAttr(c));
            if (attr.GetOverflow())
            {
                // check if this cell actually does overflow into coords
                int width = attr.GetRenderer(this, c).GetBestSize(*this, attr, dc, c).GetWidth();
                if (GetColLeft(c.m_col)+width > GetColLeft(coords.m_col))
                    return c.m_col;
            }
            break;
        }
    }    
    return -1;
}

// Note - this function only draws cells that are in the list of
// exposed cells (usually set from the update region by CalcExposedCells)
void wxSheet::DrawGridCells( wxDC& dc, const wxSheetSelection& blockSel )
{
    if ( !GetNumberRows() || !GetNumberCols() || GetBatchCount() ) 
        return;

    //static long counter = 0; ++counter;      // for testing only    
    
    wxSheetSelection removedSel;                // removed spanned
    wxSheetCoords coords;    
    wxSheetCoords lastCoords(-1,-1);
    wxSheetBlock  cellBlock;
    wxPairArrayIntInt leftCols;
    
    // paint cells in reverse order bottom to top, right to left for overflow
    wxSheetSelectionIterator revIter(blockSel, wxSSI_REVERSE);
    while (revIter.GetNext(coords) != wxSHEET_SELECTIONITER_GET_END)
    {
        // already painted this cell
        if (removedSel.Contains(coords))
        {
            lastCoords = coords;
            continue;
        }
        
        // trim out cells part of spanned cells, leaving only the owner cell
        cellBlock = GetCellBlock(coords);
        if (!cellBlock.IsOneCell())
        {
            // forget part of spanned cell, we only care about owner cell
            coords = cellBlock.GetLeftTop();  
            // don't bother checking whole block again
            removedSel.SelectBlock(cellBlock, false);  
        }
        
        // if empty find cell to left that might overflow into this one
        // only need to check left side of each block, when row increments
        if ((lastCoords.m_row > coords.m_row) && !HasCellValue(lastCoords))
        {
            int overflow_col = FindOverflowCell(lastCoords, dc);
            if ((overflow_col >= 0) && !blockSel.Contains(lastCoords.m_row, overflow_col))
                leftCols.SetValue(lastCoords.m_row, overflow_col);
        }

        //wxPrintf(wxT("%ld Drawing Cell %d %d - has %d %d\n"), counter, coords.m_row, coords.m_col, HasCellValue(coords), HasCellValue(lastCoords)); fflush(stdout);
        DrawCell( dc, coords );
        lastCoords = coords;        
    }

    // check the top left cell of the last block too
    if (ContainsGridCell(lastCoords) && !HasCellValue(lastCoords))
    {
        int overflow_col = FindOverflowCell(lastCoords, dc);
        if ((overflow_col >= 0) && !blockSel.Contains(lastCoords.m_row, overflow_col))
            leftCols.SetValue(lastCoords.m_row, overflow_col);
    }
    
    // now redraw the cells that could have overflowed into these 
    int n, count = leftCols.GetCount();
    for (n = 0; n < count; n++)
    {
        coords.Set(leftCols.GetItemKey(n), leftCols.GetItemValue(n));
        DrawCell( dc, coords );
        //wxPrintf(wxT("%ld Overflow Drawing Cell %d %d\n"), counter, coords.m_row, coords.m_col); fflush(stdout);
    }
}

/*
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
            blockSel.DeselectBlock(cellBlock, false);
            blockSel.SelectBlock(wxSheetBlock(coords, 1, 1), false);
            // don't bother checking whole block again
            removedSel.SelectBlock(cellBlock, false);  
        }

        int bottom = cellBlock.GetBottom();

        // if empty find cell to left that might overflow into this one
        // only need to check left side of each block, hence check_overflow_row 
        if ((bottom > check_overflow_row) && !HasValue()) //GetTable() && !GetTable()->HasValue(coords))
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
                    if (GetTable()->HasValue(c))
                    {
                        wxSheetCellAttr attr(GetAttr(c));
                        if (attr.GetOverflow())
                        {
                            // check if this cell actually does overflow into coords
                            int width = attr.GetRenderer(this, c).GetBestSize(*this, attr, dc, c).GetWidth();

                            if (GetColLeft(c.m_col)+width > GetColLeft(coords.m_col))
                                blockSel.SelectBlock(wxSheetBlock(c, 1, 1), false);
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
*/

void wxSheet::DrawGridSpace( wxDC& dc )
{
    int cw, ch;
    m_gridWin->GetClientSize( &cw, &ch );

    int right, bottom;
    CalcUnscrolledPosition( cw, ch, &right, &bottom );

    const int numRows = GetNumberRows();
    const int numCols = GetNumberCols();
    const int bottomRow = numRows > 0 ? GetRowBottom(numRows - 1) + 1: 0;
    const int rightCol  = numCols > 0 ? GetColRight(numCols - 1)  + 1: 0;

    if ( (right > rightCol) || (bottom > bottomRow) )
    {
        int left, top;
        CalcUnscrolledPosition( 0, 0, &left, &top );

        dc.SetBrush( wxBrush(GetAttrBackgroundColour(wxSheetCoords(0,0), wxSHEET_AttrDefault), wxSOLID) );
        dc.SetPen( *wxTRANSPARENT_PEN );

        if ( right > rightCol )
            dc.DrawRectangle( rightCol, top, right - rightCol, ch );
        if ( bottom > bottomRow )
            dc.DrawRectangle( left, bottomRow, cw, bottom - bottomRow );
    }
}

void wxSheet::DrawCell( wxDC& dc, const wxSheetCoords& coords )
{
    wxRect rect(CellToRect(coords));
    if (wxRectIsEmpty(rect))  // !IsShown
        return;

    // we draw the cell border ourselves
#if !WXSHEET_DRAW_LINES
    if ( GridLinesEnabled() != 0 )
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
    // Make sure that the cursor is valid (just in case)
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

    int penWidth = attr.GetReadOnly() ? GetCursorCellHighlightROPenWidth() : 
                                        GetCursorCellHighlightPenWidth();

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
    if ((GridLinesEnabled() & wxVERTICAL) != 0)
    {
        // right hand border
        dc.DrawLine( rect.x + rect.width, rect.y,
                     rect.x + rect.width, rect.y + rect.height + 1 );
    }
    if ((GridLinesEnabled() & wxHORIZONTAL) != 0)
    {
        // bottom border
        dc.DrawLine( rect.x,              rect.y + rect.height,
                     rect.x + rect.width, rect.y + rect.height);
    }
}

// TODO: remove this ???
// This is used to redraw all grid lines e.g. when the grid line colour
// has been changed
void wxSheet::DrawAllGridLines( wxDC& dc, const wxRegion & WXUNUSED(reg) )
{
#if !WXSHEET_DRAW_LINES
    return;
#endif

    const int numRows = GetNumberRows();
    const int numCols = GetNumberCols();
    
    if ( GetBatchCount() || (GridLinesEnabled() == 0) || !numRows || !numCols ) 
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
    right  = wxMin( right,  GetColRight(numCols - 1) );
    bottom = wxMin( bottom, GetRowBottom(numRows - 1) );
    
    // no gridlines inside spanned cells, clip them out
    int leftCol   = XToGridCol(left, true);
    int topRow    = YToGridRow(top, true);
    int rightCol  = XToGridCol(right, true);
    int bottomRow = YToGridRow(bottom, true);
    int i;

    if (HasSpannedCells())
    {
        const wxSheetBlock block(topRow, leftCol, bottomRow-topRow+1, rightCol-leftCol+1);
        wxRegion clippedcells(0, 0, cw, ch);
        bool done = false;
        
        if (GetSpannedBlocks())
        {
            const wxSheetSelection* spannedBlocks = GetSpannedBlocks();
            const int count = spannedBlocks->GetCount();
        
            for (i=spannedBlocks->FindTopRow(topRow); i<count; i++)
            {
                const wxSheetBlock &b = spannedBlocks->GetBlock(i);
                if (block.Intersects(b))
                {
                    clippedcells.Subtract(BlockToRect(b, true));
                    done = true;
                }
                else if (bottomRow < b.GetTop())
                    break;
            }
        }
        else // grind through it 
        {
            wxSheetCoords c;
            for (c.m_row = topRow; c.m_row <= bottomRow; c.m_row++)
            {
                for (c.m_col = leftCol; c.m_col <= rightCol; c.m_col++)
                {
                    const wxSheetBlock b(GetCellBlock(c));
                    if (!b.IsOneCell())
                    {
                        done = true;
                        clippedcells.Subtract(BlockToRect(b, true));
                    }
                }
            }
        }
        
        if (done)
            dc.SetClippingRegion( clippedcells );
    }
    
    dc.SetPen( wxPen(GetGridLineColour(), 1, wxSOLID) );

    if ((GridLinesEnabled() & wxHORIZONTAL) != 0)
    {
        for ( i = topRow; i < numRows; i++ )
        {
            int rowBottom = GetRowBottom(i);
            if ( rowBottom > bottom )
                break;

            if ( rowBottom >= top )
                dc.DrawLine( left, rowBottom, right, rowBottom );
        }
    }

    if ((GridLinesEnabled() & wxVERTICAL) != 0)
    {
        for ( i = leftCol; i < numCols; i++ )
        {
            int colRight = GetColRight(i);
            if ( colRight > right )
                break;

            if ( colRight >= left )
                dc.DrawLine( colRight, top, colRight, bottom );
        }
    }
    
    dc.DestroyClippingRegion();
}

void wxSheet::DrawRowLabels( wxDC& dc, const wxArrayInt& rows )
{
    size_t i, numLabels = rows.GetCount();
    if ( !GetNumberRows() || !numLabels ) return;

    for ( i = 0; i < numLabels; i++ )
        DrawCell( dc, wxSheetCoords(rows[i], -1) );
    
/*  // sample native rendernative code - FIXME  
    rect.SetX( 1 );
    rect.SetY( GetRowTop(row) + 1 );
    rect.SetWidth( m_rowLabelWidth - 2 );
    rect.SetHeight( GetRowHeight(row) - 2 );
    CalcScrolledPosition( 0, rect.y, NULL, &rect.y );
    wxWindowDC *win_dc = (wxWindowDC*) &dc;
    wxRendererNative::Get().DrawHeaderButton( win_dc->m_owner, dc, rect, 0 );
*/
    
    // Now draw the dividing lines
    dc.SetPen( wxPen(GetLabelOutlineColour(), 1, wxSOLID) );
    int top;
    CalcUnscrolledPosition(0, 0, NULL, &top);
    //int top = GetRowTop(rows[0]);
    int bottom = top + m_gridWin->GetSize().y; //GetRowBottom(rows[numLabels-1]);
    int width = GetRowLabelWidth();
    // left of row labels
    dc.DrawLine(0, top, 0, bottom);
    // right of row labels
    dc.DrawLine(width-1, top, width-1, bottom);
    // draw bottoms
    wxSheetCoords coords(0, -1);
    for ( i = 0; i < numLabels; i++ )
    {
        coords.m_row = rows[i];
        bottom = GetRowBottom(GetCellBlock(coords).GetBottom());
        dc.DrawLine(0, bottom, width-1, bottom);
    }
}

void wxSheet::DrawColLabels( wxDC& dc, const wxArrayInt& cols )
{
    size_t i, numLabels = cols.GetCount();
    if ( !GetNumberCols() || !numLabels ) return;

    for ( i = 0; i < numLabels; i++ )
        DrawCell( dc, wxSheetCoords(-1, cols[i]) );

/*  // sample native rendernative code - FIXME  
    rect.SetX( colLeft + 1 );
    rect.SetY( 1 );
    rect.SetWidth( GetColWidth(col) - 2 );
    rect.SetHeight( m_colLabelHeight - 2 );
    wxWindowDC *win_dc = (wxWindowDC*) &dc;
    wxRendererNative::Get().DrawHeaderButton( win_dc->m_owner, dc, rect, 0 );   
*/
    
    // Now draw the dividing lines
    dc.SetPen( wxPen(GetLabelOutlineColour(), 1, wxSOLID) );
    int left; 
    CalcUnscrolledPosition(0, 0, &left, NULL);
    //int left = GetColLeft(cols[0]); 
    int right = left + m_gridWin->GetSize().x; //GetColRight(cols[numLabels-1]); 
    int height = GetColLabelHeight();
    // top of col labels
    dc.DrawLine(left, 0, right, 0);
    // bottom of col labels
    dc.DrawLine(left, height-1, right, height-1);
    // draw rights
    wxSheetCoords coords(-1, 0);
    for ( i = 0; i < numLabels; i++ )
    {
        coords.m_col = cols[i];
        right = GetColRight(GetCellBlock(coords).GetRight());
        dc.DrawLine(right, 0, right, height-1);
    }
}

void wxSheet::DrawCornerLabel( wxDC& dc )
{
    DrawCell( dc, wxSheetCoords(-1, -1) );
    wxSize size(m_cornerLabelWin->GetClientSize());

#ifdef USE_RENDERNATIVE
    wxRect rect(1, 1, size.x-2, size.y-2);
    //rect.SetX( 1 );
    //rect.SetY( 1 );
    //rect.SetWidth( client_width - 2 );
    //rect.SetHeight( client_height - 2 );
    wxRendererNative::Get().DrawHeaderButton( this, dc, rect, 0 );
#else          
    // Now draw the dividing lines
    dc.SetPen( wxPen(GetLabelOutlineColour(), 1, wxSOLID) );
    dc.DrawLine(0, 0, size.x, 0);                   // top
    dc.DrawLine(0, size.y-1, size.x, size.y-1);     // bottom
    dc.DrawLine(0, 0, 0, size.y-1);                 // left
    dc.DrawLine(size.x-1, 0, size.x-1, size.y-1);   // right
#endif     
}

void wxSheet::DrawRowColResizingMarker( int newDragPos )
{
    if ( !HasMouseCursorMode(WXSHEET_CURSOR_RESIZING) ||
         ((m_dragLastPos == -1) && (newDragPos == -1)) )
        return;
    
    wxClientDC dc( m_gridWin );
    PrepareGridDC( dc );

    int left = 0, top = 0;
    CalcUnscrolledPosition( 0, 0, &left, &top );

    int right = GetColRight(GetNumberCols()-1);
    int bottom = GetRowBottom(GetNumberRows()-1);
    int cw = 0, ch = 0;
    m_gridWin->GetClientSize( &cw, &ch );
    right = wxMin(left+cw, right);
    bottom = wxMin(top+ch, bottom);

    const bool resizingRow = HasMouseCursorMode(WXSHEET_CURSOR_RESIZE_ROW);
    
    if (GridLinesEnabled())
    {
        dc.SetPen(wxPen(GetGridLineColour(), 3, wxSOLID));
    
        // Draw the anchor marker so you know what row/col you're resizing
        if (resizingRow)
        {
            int anchor = GetRowTop(m_dragRowOrCol);
            dc.DrawLine( left, anchor, right, anchor );
        }
        else // resizing col
        {
            int anchor = GetColLeft(m_dragRowOrCol);
            dc.DrawLine( anchor, top, anchor, bottom );
        }
    }
    
    int log_fn = dc.GetLogicalFunction();
    dc.SetLogicalFunction(wxINVERT);

    if (m_dragLastPos >= 0)
    {
        if (resizingRow)
            dc.DrawLine( left, m_dragLastPos, right, m_dragLastPos );
        else // resizing col
            dc.DrawLine( m_dragLastPos, top, m_dragLastPos, bottom );
    }

    if (newDragPos >= 0)
    {
        m_dragLastPos = newDragPos;
        
        if (resizingRow)
            dc.DrawLine( left, m_dragLastPos, right, m_dragLastPos );
        else // resizing col
            dc.DrawLine( m_dragLastPos, top, m_dragLastPos, bottom );
    }
    
    dc.SetLogicalFunction(log_fn); // set it back since nobody else wants invert
}

bool wxSheet::CalcRowLabelsExposed( const wxRegion& reg, wxArrayInt& rowLabels ) const
{
    const int numRows = GetNumberRows();
    if (!numRows) 
        return false;

    const bool spanned = HasSpannedCells();
    wxRegionIterator iter( reg );

    while ( iter )
    {
        wxRect r(iter.GetRect());

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
        int top;
        CalcUnscrolledPosition( 0, r.GetTop(), NULL, &top );
        int bottom = top + r.GetHeight();
        //CalcUnscrolledPosition( 0, r.GetBottom(), &dummy, &bottom );
        
        // find the row labels within these bounds
        for ( int row = YToGridRow(top, true); row < numRows;  row++ )
        {
            if ( GetRowBottom(row) < top )
                continue;
            if ( GetRowTop(row) > bottom )
                break;

            if (spanned)
            {
                const wxSheetBlock block(GetCellBlock(wxSheetCoords(row, -1)));
                    
                if (block.GetHeight() >= 1)
                {
                    rowLabels.Add( block.GetTop() );
                    row = block.GetBottom();
                }
                else if (block.GetHeight() < 1)
                    row = block.GetBottom();
            }
            else
                rowLabels.Add( row );
        }

        iter++ ;
    }

    return rowLabels.GetCount() > 0u;
}

bool wxSheet::CalcColLabelsExposed( const wxRegion& reg, wxArrayInt& colLabels ) const
{
    const int numCols = GetNumberCols();
    if (!numCols)
        return false;

    const bool spanned = HasSpannedCells();
    wxRegionIterator iter( reg );

    while ( iter )
    {
        wxRect r(iter.GetRect());

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
        int left;
        CalcUnscrolledPosition( r.GetLeft(), 0, &left, NULL );
        int right = left + r.GetWidth();
        //CalcUnscrolledPosition( r.GetRight(), 0, &right, &dummy );
        
        // find the cells within these bounds
        for ( int col = XToGridCol(left, true); col < numCols;  col++ )
        {
            if ( GetColRight(col) < left )
                continue;
            if ( GetColLeft(col) > right )
                break;

            if (spanned)
            {
                const wxSheetBlock block(GetCellBlock(wxSheetCoords(-1, col)));
                    
                if (block.GetWidth() >= 1)
                {
                    colLabels.Add( block.GetLeft() );
                    col = block.GetRight();
                }
                else if (block.GetWidth() < 1)
                    col = block.GetRight();
            }
            else
                colLabels.Add( col );
        }

        iter++ ;
    }
    return colLabels.GetCount() > 0u;
}

bool wxSheet::CalcCellsExposed( const wxRegion& reg, wxSheetSelection& blockSel ) const
{
    const int numRows = GetNumberRows();
    const int numCols = GetNumberCols();
    if (!numRows || !numCols)
        return false;

    wxRegionIterator iter( reg );
    while ( iter )
    {
        wxRect r(iter.GetRect());

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
        int left, top, right, bottom;
        CalcUnscrolledPosition( r.GetLeft(), r.GetTop(), &left, &top );
        CalcUnscrolledPosition( r.GetRight(), r.GetBottom(), &right, &bottom );
        
        // find the cells within these bounds
        wxSheetBlock block(YToGridRow(top, true), XToGridCol(left, true), 0, 0);
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
        
        blockSel.SelectBlock(block, false);

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

    // swap width and height if vertically orientated
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
    
    return (w > 0) && (h > 0);
}

// ----------------------------------------------------------------------------

wxSheetCoords wxSheet::XYToGridCell( int x, int y, bool clipToMinMax ) const
{
    return wxSheetCoords(YToGridRow(y, clipToMinMax), XToGridCol(x, clipToMinMax));
}
int wxSheet::YToGridRow( int y, bool clipToMinMax ) const
{
    return GetSheetRefData()->m_rowEdges.FindIndex(y, clipToMinMax);
}
int wxSheet::XToGridCol( int x, bool clipToMinMax ) const
{
    return GetSheetRefData()->m_colEdges.FindIndex(x, clipToMinMax);
}
int wxSheet::YToEdgeOfGridRow( int y ) const
{
    return GetSheetRefData()->m_rowEdges.FindMaxEdgeIndex(y);
}
int wxSheet::XToEdgeOfGridCol( int x ) const
{
    return GetSheetRefData()->m_colEdges.FindMaxEdgeIndex(x);
}

wxRect wxSheet::CellToRect( const wxSheetCoords& coords, bool getDeviceRect ) const
{
    wxCHECK_MSG(ContainsCell(coords), wxRect(0,0,0,0), wxT("Invalid coords"));
    return BlockToRect(GetCellBlock(coords), getDeviceRect);
}

wxRect wxSheet::BlockToRect( const wxSheetBlock& block, bool getDeviceRect ) const
{   
    wxRect rect(BlockToLogicalRect(block, false));
    
    // if grid lines are enabled, then the area of the cell is a bit smaller
    if ((GridLinesEnabled() & wxHORIZONTAL) != 0) 
        rect.height--;
    if ((GridLinesEnabled() & wxVERTICAL) != 0) 
        rect.width--;

    if (getDeviceRect)
    {
        switch (GetCellCoordsType(block.GetLeftTop()))
        {
            case wxSHEET_CELL_GRID     : return CalcScrolledRect(rect);
            case wxSHEET_CELL_ROWLABEL : CalcScrolledPosition(0, rect.y, NULL, &rect.y); break;
            case wxSHEET_CELL_COLLABEL : CalcScrolledPosition(rect.x, 0, &rect.x, NULL); break;
            default : break;
        }
    }
    
    return rect;
}

wxSheetBlock wxSheet::ExpandSpannedBlock(const wxSheetBlock& block_) const
{
    if (!HasSpannedCells() || block_.IsEmpty())
        return block_;
    
    wxSheetBlock block(block_);

    if (GetSpannedBlocks())
    {
        const wxSheetSelection* spannedBlocks = GetSpannedBlocks();
        size_t n, count = spannedBlocks->GetCount();
        for (n=0; n<count; n++)
        {
            const wxSheetBlock& b = spannedBlocks->GetBlock(n);
            if (block_.Intersects(b)) // use original block so it doesn't keep growing
                block = block.Union(b);
        }
    }
    else // brute force expansion
    {
        wxSheetCoords c;
        int row_bottom = block_.GetBottom();
        int col_right  = block_.GetRight();
        for (c.m_row = block_.GetTop(); c.m_row <= row_bottom; c.m_row++)
        {
            for (c.m_col = block_.GetLeft(); c.m_col <= col_right; c.m_col++)
            {
                block = block.Union(GetCellBlock(c));
            }
        }
    }
    
    return block;    
}

wxRect wxSheet::BlockToLogicalRect( const wxSheetBlock& block_, bool expand_spanned ) const
{
    wxSheetBlock block(expand_spanned ? ExpandSpannedBlock(block_) : block_);

    wxRect rect(GetColLeft(block.GetLeft()), GetRowTop(block.GetTop()), 0, 0);
    rect.width  = GetColRight(block.GetRight())   - rect.x + 1;
    rect.height = GetRowBottom(block.GetBottom()) - rect.y + 1;
    return rect;    
}

wxRect wxSheet::BlockToDeviceRect( const wxSheetBlock &block_, bool expand_spanned) const
{   
    // merely convert to scrolled coords
    return CalcScrolledRect(BlockToLogicalRect(block_, expand_spanned));
}

wxSheetBlock wxSheet::LogicalGridRectToBlock(const wxRect &rect, bool wholeCell) const
{
    const int numRows = GetNumberRows();
    const int numCols = GetNumberCols();
    if (!numRows || !numCols) return wxNullSheetBlock;

    int leftCol   = XToGridCol(rect.GetLeft(),   true);
    int topRow    = YToGridRow(rect.GetTop(),    true);
    int rightCol  = XToGridCol(rect.GetRight(),  true);
    int bottomRow = YToGridRow(rect.GetBottom(), true);
    
    if (wholeCell)
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
    
    wxSheetBlock block(topRow, leftCol, bottomRow-topRow+1, rightCol-leftCol+1);
    return block.Intersect(wxSheetBlock(0, 0, numRows, numCols));
}

wxSheetBlock wxSheet::GetVisibleGridCellsBlock(bool wholeCellVisible) const
{
    wxRect rect(CalcUnscrolledRect(wxRect(wxPoint(0,0), m_gridWin->GetClientSize())));
    return LogicalGridRectToBlock(rect, wholeCellVisible);
}

wxPoint wxSheet::AlignInRect( int align, const wxRect& rect, const wxSize& size, bool inside ) const
{
    wxPoint origin(rect.x, rect.y);
    
    // if it won't fit horizontally, then it must be aligned left
    if (inside && (size.x > rect.width))
    {
        align &= ~wxALIGN_RIGHT;
        align &= ~wxALIGN_CENTRE_HORIZONTAL;
    }
    // if it won't fit vertically, then it must be aligned to the top
    if (inside && (size.y > rect.height))
    {
        align &= ~wxALIGN_BOTTOM;
        align &= ~wxALIGN_CENTRE_VERTICAL;
    }
    
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

wxSize wxSheet::GetGridVirtualSize(bool add_margin) const
{
    wxSize size;
    const int numRows = GetNumberRows();
    const int numCols = GetNumberCols();
    if ((numCols > 0) && (numRows > 0))
    {
        size.x = GetColRight(numCols - 1);
        size.y = GetRowBottom(numRows - 1);
    }
    
    if (add_margin)
    {
        size.x += GetSheetRefData()->m_marginSize.x;
        size.y += GetSheetRefData()->m_marginSize.y;
    }
    
    return size;
}

wxSize wxSheet::GetGridExtent() const
{
    wxSize s(GetGridVirtualSize(true));
    if (GetGridWindow())
    {
        wxSize winSize(GetGridWindow()->GetSize());
        if (winSize.x > s.x) s.x = winSize.x;
        if (winSize.y > s.y) s.y = winSize.y;
    }
    return s;
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
    wxSize virtSize(GetGridVirtualSize());
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
        SetGridOrigin( pos*15, -1, false, true );
        //SetGridOrigin( pos*m_horizScrollBar->GetThumbSize(), -1, false, true );
    else if (event.GetId() == ID_VERT_SCROLLBAR)
        SetGridOrigin( -1, pos*15, false, true );
        //SetGridOrigin( -1, pos*m_vertScrollBar->GetThumbSize(), false, true );
}

void wxSheet::AdjustScrollbars(bool calc_win_sizes)
{
    if (!m_gridWin || m_resizing)
        return;
    
    m_resizing = true;

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
/*
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
*/    
    // Figure out if we need the scrollbars at all
    bool need_Xscroll = (m_scrollBarMode & SB_HORIZ_NEVER) != 0 ? false : 
        (((m_scrollBarMode & SB_HORIZ_ALWAYS) != 0) ? true : virtSize.x > gw);
    bool need_Yscroll = (m_scrollBarMode & SB_VERT_NEVER ) != 0 ? false : 
        (((m_scrollBarMode & SB_VERT_ALWAYS ) != 0) ? true : virtSize.y > gh);
    
    // Now cut down size due to the scrollbars if shown
    if (need_Xscroll) gh -= sb_height;
    if (need_Yscroll) gw -= sb_width;
    
    // Maybe now that it's smaller we need the other scrollbar
    need_Xscroll = (m_scrollBarMode & SB_HORIZ_NEVER) != 0 ? false : 
        (((m_scrollBarMode & SB_HORIZ_ALWAYS) != 0) ? true : virtSize.x > gw);
    need_Yscroll = (m_scrollBarMode & SB_VERT_NEVER ) != 0 ? false : 
        (((m_scrollBarMode & SB_VERT_ALWAYS ) != 0) ? true : virtSize.y > gh);
    
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
    
    // recalculate the windows sizes and positions if we added/removed scrollbar
    bool calcSizes = false;
    
    if (need_Xscroll != horizSbShown)
    {
        calcSizes = true;
        m_horizScrollBar->Show(need_Xscroll);
    }
    if (need_Yscroll != vertSbShown)
    {
        calcSizes = true;
        m_vertScrollBar->Show(need_Yscroll);
    }

    m_resizing = false;
    
    if (calcSizes && calc_win_sizes)
        CalcWindowSizes(false);
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

// Checks and returns a suitable horiz or vert alignment, if invalid replaces
//   with wxSheetCellAttr::NoHorizAlign/NoVertAlign
static int CheckAlignment(int align)
{
    // note: it's a shame that wxALIGN_LEFT = wxALIGN_TOP = 0
    
    int count = 0;
    if ((align & wxSHEET_AttrAlignRight)       != 0) count++;
    if ((align & wxSHEET_AttrAlignCenterHoriz) != 0) count++;
    if ((align & wxSHEET_AttrAlignHorizUnset)  != 0) count++;
    if (count > 1)
    {
        align &= ~wxSHEET_AttrAlignHoriz_Mask; // clear whatever is there
        align |= wxSHEET_AttrAlignHorizUnset;  // set to no alignment
    }
    
    count = 0;
    if ((align & wxSHEET_AttrAlignBottom)     != 0) count++;
    if ((align & wxSHEET_AttrAlignCenterVert) != 0) count++;
    if ((align & wxSHEET_AttrAlignVertUnset)  != 0) count++;
    if (count > 1)
    {
        align &= ~wxSHEET_AttrAlignVert_Mask; // clear whatever is there
        align |= wxSHEET_AttrAlignVertUnset;  // set to no alignment
    }
    
    return align;
}

int wxSheet::SetAlignment(int orig_align, int hAlign, int vAlign)
{
    if (hAlign != -1)
    {
        orig_align &= ~wxSHEET_AttrAlignHoriz_Mask;           // clear old
        orig_align |= (hAlign & wxSHEET_AttrAlignHoriz_Mask); // set new
    }
    if (vAlign != -1)
    {
        orig_align &= ~wxSHEET_AttrAlignVert_Mask;
        orig_align |= (vAlign & wxSHEET_AttrAlignVert_Mask);
    }
    
    return CheckAlignment(orig_align);
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
void wxSheet::OnMouse( wxMouseEvent& event )
{
    wxWindow *win = (wxWindow*)event.GetEventObject();
    
    if (win == this)
        ProcessSheetMouseEvent(event);
    else if (win == m_rowLabelWin)
        ProcessRowLabelMouseEvent(event);
    else if (win == m_colLabelWin)
        ProcessColLabelMouseEvent(event);
    else if (win == m_cornerLabelWin)
        ProcessCornerLabelMouseEvent(event);
    else if (win == m_gridWin)
        ProcessGridCellMouseEvent(event);
    else
        event.Skip();
}

void wxSheet::OnMouseWheel( wxMouseEvent& event )
{
    wxWindow *win = (wxWindow*)event.GetEventObject();
    
    // Scroll up and down by a 1/3 of the height of the window
    if ((win == m_rowLabelWin) || (win == m_gridWin))
    {
        // GTK doesn't have good wheel events
        if (GetNumberRows() > 0)
        {
            wxPoint origin(GetGridOrigin());
            wxSize size(GetGridWindow()->GetClientSize());
            int dy = event.GetWheelRotation() < 0 ? 1 : -1;
            SetGridOrigin( origin.x, origin.y + dy * size.y/3, true, true );
        }
    }
    // Scroll sideways by a 1/3 of the width of the window
    else if (win == m_colLabelWin)
    {
        if (GetNumberCols() > 0)
        {
            wxPoint origin(GetGridOrigin());
            wxSize size(GetGridWindow()->GetClientSize());
            int dx = event.GetWheelRotation() < 0 ? 1 : -1;
            SetGridOrigin( origin.x + dx * size.x/3, origin.y, true, true );
        }
    }

    event.Skip();
}

void wxSheet::ProcessSheetMouseEvent( wxMouseEvent& event )
{
    if (!m_enable_split_vert && !m_enable_split_horiz)
        return;

    wxPoint mousePos = event.GetPosition();
    
    if (event.LeftDown())
    {
#if wxCHECK_VERSION(2,7,0)
        if (m_vertSplitRect.Contains(mousePos) || m_horizSplitRect.Contains(mousePos))
#else
        if (m_vertSplitRect.Inside(mousePos) || m_horizSplitRect.Inside(mousePos))
#endif
            SetCaptureWindow(this);
    }
    else if (event.LeftUp())
    {
        SetCaptureWindow(NULL);
    }
    else if (event.Dragging() && HasCapture() && HasMouseCursorMode(WXSHEET_CURSOR_SPLITTING))
    {
        wxSheetSplitterEvent splitEvent(GetId(), wxEVT_SHEET_SPLIT_BEGIN);
        splitEvent.SetEventObject(this);
        splitEvent.m_vert_split = HasMouseCursorMode(WXSHEET_CURSOR_SPLIT_VERTICAL);
        
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
#if wxCHECK_VERSION(2,7,0)
        if (m_vertSplitRect.Contains(mousePos))
            SetMouseCursorMode(WXSHEET_CURSOR_SPLIT_VERTICAL, this);
        else if (m_horizSplitRect.Contains(mousePos))
            SetMouseCursorMode(WXSHEET_CURSOR_SPLIT_HORIZONTAL, this);
        else
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, this);
#else
        if (m_vertSplitRect.Inside(mousePos))
            SetMouseCursorMode(WXSHEET_CURSOR_SPLIT_VERTICAL, this);
        else if (m_horizSplitRect.Inside(mousePos))
            SetMouseCursorMode(WXSHEET_CURSOR_SPLIT_HORIZONTAL, this);
        else
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, this);
#endif
    }
}

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
            m_isDragging = true;
            SetCaptureWindow(m_rowLabelWin);
        }

        if ( event.LeftIsDown() )
        {
            bool can_scroll = true;
            
            switch ( GetMouseCursorMode() )
            {
                case WXSHEET_CURSOR_RESIZE_ROW:
                {
                    y = wxMax( y, GetRowTop(m_dragRowOrCol) + 
                                  GetMinimalRowHeight(m_dragRowOrCol));
                    can_scroll = false; // y != m_dragLastPos;
                    DrawRowColResizingMarker( y );
                    break;
                }
                case WXSHEET_CURSOR_SELECT_ROW:
                {
                    if (HasSelectionMode(wxSHEET_SelectNone|wxSHEET_SelectCols))
                        break;
                    
                    // check for clearing here since we didn't if editing allowed
                    bool add = event.ShiftDown() || event.ControlDown();
                    if (HasSelection(false) && !add)
                        ClearSelection(true);

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
    m_isDragging = false;

    if ( event.LeftDown() )
    {
        if (IsCellEditControlShown())
            DisableCellEditControl(true);
        
        // don't send a label click event for a hit on the edge of the row label
        // this is probably the user wanting to resize the row
        if ( YToEdgeOfGridRow(y) < 0 )
        {
            if ( ContainsRowLabelCell(coords)  &&
                 (SendEvent(wxEVT_SHEET_LABEL_LEFT_DOWN, coords, &event) == EVT_SKIPPED) &&
                 !HasSelectionMode(wxSHEET_SelectNone|wxSHEET_SelectCols) )
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
                                   block, false, false, &event) != EVT_VETOED))
                {   
                    ClearSelection(true);
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
                    m_waitForSlowClick = false;
                    ClearSelection(true);
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

        if ( HasMouseCursorMode(WXSHEET_CURSOR_RESIZE_ROW) )
        {
            // Note: send event *after* doing default processing in this case
            if (DoEndDragResizeRowCol())
                SendEvent( wxEVT_SHEET_ROW_SIZE, wxSheetCoords(m_dragRowOrCol, -1), &event );
        }
        else if ( HasMouseCursorMode(WXSHEET_CURSOR_SELECT_ROW) )
        {
            if (!GetSelectingBlock().IsEmpty())
            {                
                bool add = event.ShiftDown() || event.ControlDown();

                if (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                             GetSelectingBlock(), true, add, &event) != EVT_VETOED)
                {   
                    SelectRows(GetSelectingBlock().GetTop(), GetSelectingBlock().GetBottom(), add, true);
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
            DisableCellEditControl(true);
        
        if ( ContainsRowLabelCell(coords) &&
	        (SendEvent(wxEVT_SHEET_LABEL_RIGHT_DOWN, coords, &event) == EVT_SKIPPED))
        {
            // no default action at the moment
        }
    }
    else if ( event.RightDClick() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(true);
        
        if ( ContainsRowLabelCell(coords) &&
	        (SendEvent(wxEVT_SHEET_LABEL_RIGHT_DCLICK, coords, &event) == EVT_SKIPPED))
        {
            // no default action at the moment
        }
    }
    else if ( event.RightUp() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(true);
        
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
            if ( HasMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL) )
            {
                // don't capture the mouse yet
                if ( CanDragRowSize() )
                {
                    SetMouseCursorMode(WXSHEET_CURSOR_RESIZE_ROW, m_rowLabelWin);
                }
            }
        }
        else if ( !HasMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL) )
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
            m_isDragging = true;
            SetCaptureWindow(m_colLabelWin);
        }

        if ( event.LeftIsDown() )
        {
            bool can_scroll = true;
            
            switch( GetMouseCursorMode() )
            {
                case WXSHEET_CURSOR_RESIZE_COL :
                {
                    x = wxMax( x, GetColLeft(m_dragRowOrCol) +
                                  GetMinimalColWidth(m_dragRowOrCol));
                    can_scroll = false; // x != m_dragLastPos;
                    DrawRowColResizingMarker( x );
                    break;
                }
                case WXSHEET_CURSOR_SELECT_COL :
                {
                    if (HasSelectionMode(wxSHEET_SelectNone|wxSHEET_SelectRows))
                        break;
                    
                    // check for clearing here since we didn't if editing allowed
                    bool add = event.ShiftDown() || event.ControlDown();
                    if (HasSelection(false) && !add)
                        ClearSelection(true);

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
                ((m_mousePos.x < 0) || (m_mousePos.x > m_colLabelWin->GetClientSize().GetWidth())))
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
    m_isDragging = false;

    if ( event.LeftDown() )
    {
        if (IsCellEditControlShown())
            DisableCellEditControl(true);

        // don't send a label click event for a hit on the edge of the col label
        //  this is probably the user wanting to resize the col
        if ( XToEdgeOfGridCol(x) < 0 )
        {
            if ( ContainsColLabelCell(coords) &&
                 (SendEvent(wxEVT_SHEET_LABEL_LEFT_DOWN, coords, &event) == EVT_SKIPPED) &&
                 !HasSelectionMode(wxSHEET_SelectNone|wxSHEET_SelectRows) )
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
                                   block, false, false, &event) != EVT_VETOED))
                {   
                    ClearSelection(true);
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
                    m_waitForSlowClick = false;
                    ClearSelection(true);
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
        
        if ( HasMouseCursorMode(WXSHEET_CURSOR_RESIZE_COL) )
        {
            // Note: send event *after* doing default processing in this case
            if (DoEndDragResizeRowCol())
                SendEvent( wxEVT_SHEET_COL_SIZE, wxSheetCoords(-1, m_dragRowOrCol), &event );
        }
        else if ( HasMouseCursorMode(WXSHEET_CURSOR_SELECT_COL) )
        {
            if (!GetSelectingBlock().IsEmpty())
            {
                bool add = event.ShiftDown() || event.ControlDown();

                if (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                             GetSelectingBlock(), true, add, &event) != EVT_VETOED)
                {   
                    SelectCols(GetSelectingBlock().GetLeft(), GetSelectingBlock().GetRight(), add, true);
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
            DisableCellEditControl(true);
        
        if (ContainsColLabelCell(coords) && 
            (SendEvent(wxEVT_SHEET_LABEL_RIGHT_DOWN, coords, &event) == EVT_SKIPPED))
        {
            // no default action at the moment
        }
    }
    else if ( event.RightDClick() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(true);
        
        if (ContainsColLabelCell(coords) && 
            (SendEvent(wxEVT_SHEET_LABEL_RIGHT_DCLICK, coords, &event) == EVT_SKIPPED))
        {
            // no default action at the moment
        }
    }
    else if ( event.RightUp() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(true);
        
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
            if ( HasMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL) )
            {
                // don't capture the cursor yet
                if ( CanDragColSize() )
                    SetMouseCursorMode(WXSHEET_CURSOR_RESIZE_COL, m_colLabelWin);
            }
        }
        else if ( !HasMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL) )
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
            if ((GetNumberRows() > 0) && (GetNumberCols() > 0) && !HasSelectionMode(wxSHEET_SelectNone))
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
            if (!can_edit && !HasSelectionMode(wxSHEET_SelectNone) &&
                (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                               block, true, false, &event) != EVT_VETOED))
            {   
                SelectAll(true);
            }   
        }
    }
    else if ( event.LeftDClick() )
    {
        if (SendEvent(wxEVT_SHEET_LABEL_LEFT_DCLICK, coords, &event) == EVT_SKIPPED)
        {
            if (CanEnableCellControl(coords))
            {
                ClearSelection(true);
                EnableCellEditControl(coords);
                if ( IsCellEditControlCreated() )
                    GetSheetRefData()->m_cellEditor.StartingClick();
                
                m_waitForSlowClick = false;
            }
        }
    }
    else if ( event.LeftUp() )
    {
        if (!GetSelectingBlock().IsEmpty())
        {
            if (SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                           GetSelectingBlock(), true, false, &event) != EVT_VETOED)
            {   
                SelectAll(true);
                SetSelectingBlock(wxNullSheetBlock);
            }
        }
        
        // send after default processing, they can use own evt handler 
        SendEvent(wxEVT_SHEET_LABEL_LEFT_UP, coords, &event);
    }
    else if ( event.RightDown() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(true);
        
        if (SendEvent(wxEVT_SHEET_LABEL_RIGHT_DOWN, coords, &event) == EVT_SKIPPED)
        {
            // no default action at the moment
        }
    }
    else if ( event.RightDClick() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(true);
        
        if (SendEvent(wxEVT_SHEET_LABEL_RIGHT_DCLICK, coords, &event) == EVT_SKIPPED)
        {
            // no default action at the moment
        }
    }
    else if ( event.RightUp() )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(true);
        
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

        bool can_scroll = true;
        m_isDragging = true;
        if ( HasMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL) )
        {
            // Hide the edit control, so it won't interfer with drag-shrinking.
            if ( IsCellEditControlShown() )
                DisableCellEditControl(true);

            if ( ContainsGridCell(coords) && !HasSelectionMode(wxSHEET_SelectNone) )
            {
                HighlightSelectingBlock( GetSelectingAnchor(), coords );
            }
        }
        else if ( HasMouseCursorMode(WXSHEET_CURSOR_RESIZE_ROW) )
        {
            y = wxMax( y, GetRowTop(m_dragRowOrCol) +
                          GetMinimalRowHeight(m_dragRowOrCol) );
            can_scroll = false; // y != m_dragLastPos;
            DrawRowColResizingMarker( y );
        }
        else if ( HasMouseCursorMode(WXSHEET_CURSOR_RESIZE_COL) )
        {
            x = wxMax( x, GetColLeft(m_dragRowOrCol) +
                          GetMinimalColWidth(m_dragRowOrCol));
            can_scroll = false; // x != m_dragLastPos;
            DrawRowColResizingMarker( x );
        }

        if (can_scroll && 
#if wxCHECK_VERSION(2,7,0)
            !wxRect(wxPoint(0,0), m_gridWin->GetClientSize()).Contains(m_mousePos))
#else
            !wxRect(wxPoint(0,0), m_gridWin->GetClientSize()).Inside(m_mousePos))
#endif
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
    m_isDragging = false;
    m_startDragPos = wxDefaultPosition;

    if ( event.LeftDown() && ContainsGridCell(coords) )
    {        
        if ( SendEvent(wxEVT_SHEET_CELL_LEFT_DOWN, coords, &event) == EVT_SKIPPED)
        {
            if ( !event.ControlDown() && HasSelection() )
            {
                wxSheetBlock block(0, 0, GetNumberRows()-1, GetNumberCols()-1);
                 
                if (!HasSelectionMode(wxSHEET_SelectNone) &&
                    SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                                   block, false, false, &event) != EVT_VETOED)
                {   
                    ClearSelection(true);
                }
            }
            
            if ( event.ShiftDown() )
            {
                wxSheetBlock block(GetGridCursorCell(), coords);
                
                if (!HasSelectionMode(wxSHEET_SelectNone) &&
                    SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                                    block, true, true, &event) != EVT_VETOED)
                {   
                    SelectBlock(block, event.ControlDown(), true);
                }
            }
            else if ( (XToEdgeOfGridCol(x) < 0) && (YToEdgeOfGridRow(y) < 0) )
            {
                if (IsCellEditControlCreated())
                    DisableCellEditControl(true);
                
                MakeCellVisible( coords );

                if ( !ContainsGridCell(GetSelectingAnchor()) )
                    SetSelectingAnchor(coords);

                if ( event.ControlDown() )
                {
                    // toggle cell selection
                    int sel = IsCellSelected(coords);
                    wxSheetBlock block(coords, 1, 1);
                    if (!HasSelectionMode(wxSHEET_SelectNone) &&
                        SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                                       block, !sel, !sel, &event) != EVT_VETOED)
                    {   
                        ToggleCellSelection(coords, !sel, true);
                        SetSelectingBlock(wxNullSheetBlock);
                    }
                }
                else
                {
                    if (GetGridCursorCell() == coords)
                        m_waitForSlowClick = true;

                    SetGridCursorCell( coords );
                    // FIXME weird? Highlight a whole row/col when not in select cells?
                    if ( !HasSelectionMode(wxSHEET_SelectNone|wxSHEET_SelectCells) )
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
                ClearSelection(true);
                EnableCellEditControl(GetGridCursorCell());
                if ( IsCellEditControlCreated() )
                    GetSheetRefData()->m_cellEditor.StartingClick();

                m_waitForSlowClick = false;
            }
        }
    }
    else if ( event.LeftUp() )
    {
        StopMouseTimer();
        SetCaptureWindow(NULL);

        if ( HasMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL) )
        {
            if ((coords == GetGridCursorCell()) && m_waitForSlowClick && 
                CanEnableCellControl(GetGridCursorCell()))
            {
                ClearSelection(true);
                EnableCellEditControl(GetGridCursorCell());
                if ( IsCellEditControlCreated() )
                    GetSheetRefData()->m_cellEditor.StartingClick();

                m_waitForSlowClick = false;
            }
            else if ( !GetSelectingBlock().IsEmpty() )
            {
                if (!HasSelectionMode(wxSHEET_SelectNone) &&
                    SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                                   GetSelectingBlock(), true, true, &event) != EVT_VETOED)
                {   
                    SelectBlock(GetSelectingBlock(), true, true);
                    SetSelectingBlock(wxNullSheetBlock);
                }
            }
        }
        else if ( HasMouseCursorMode(WXSHEET_CURSOR_RESIZE_ROW) )
        {
            // Note: send event *after* doing default processing in this case
            if (DoEndDragResizeRowCol())
                SendEvent( wxEVT_SHEET_ROW_SIZE, wxSheetCoords(m_dragRowOrCol, -1), &event );
            
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_gridWin);
        }
        else if ( HasMouseCursorMode(WXSHEET_CURSOR_RESIZE_COL) )
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
            DisableCellEditControl(true);
        
        if (SendEvent(wxEVT_SHEET_CELL_RIGHT_DOWN, coords, &event) == EVT_SKIPPED)
        {
            // no default action at the moment
        }
    }
    else if ( event.RightDClick() && ContainsGridCell(coords) )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(true);
        
        if (SendEvent(wxEVT_SHEET_CELL_RIGHT_DCLICK, coords, &event) == EVT_SKIPPED)
        {
            // no default action at the moment
        }
    }
    else if ( event.RightUp() && ContainsGridCell(coords) )
    {
        if (IsCellEditControlCreated())
            DisableCellEditControl(true);
        
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

        // check if this is inside a spanned cell, if so don't drag
        if ( (dragRow >= 0) || (dragCol >= 0) )
        {
            const wxSheetBlock cellBlock(GetCellBlock(coords));
            if (!cellBlock.IsOneCell() && 
                ((dragRow != cellBlock.GetBottom()) || (dragCol != cellBlock.GetRight())))
                dragRow = dragCol = -1;
        }

        // Dragging on the corner of a cell to resize in both
        // directions is not implemented yet...
        if ( (dragRow >= 0) && (dragCol >= 0) )
        {
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_gridWin);
        }
        else if ( dragRow >= 0 )
        {
            m_dragRowOrCol = dragRow;

            if ( HasMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL) )
            {
                if ( CanDragRowSize() && CanDragGridSize() )
                {
                    SetMouseCursorMode(WXSHEET_CURSOR_RESIZE_ROW, m_gridWin);
                    SetCaptureWindow(m_gridWin);
                }
            }

            if ( dragCol >= 0 )
                m_dragRowOrCol = dragCol;
        }
        else if ( dragCol >= 0 )
        {
            m_dragRowOrCol = dragCol;

            if ( HasMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL) )
            {
                if ( CanDragColSize() && CanDragGridSize() )
                {
                    SetMouseCursorMode(WXSHEET_CURSOR_RESIZE_COL, m_gridWin);
                    SetCaptureWindow(m_gridWin);
                }
            }
        }
        // Neither on a row or col edge
        else if ( !HasMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL) )
            SetMouseCursorMode(WXSHEET_CURSOR_SELECT_CELL, m_gridWin);
    }
}

void wxSheet::OnKeyDown( wxKeyEvent& event )
{
    //wxCHECK_RET(!m_inOnKeyDown, wxT("wxSheet::OnKeyDown called while already active"));
    // yield called from SetGridOrigin causes this in MSW
    
    if (m_inOnKeyDown)
        return;

    m_inOnKeyDown = true;
    
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
                        DisableCellEditControl(true);
                    if ( GetGridCursorRow() < GetNumberRows()-1 )
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
                        DisableCellEditControl(true);   
                    if ( GetGridCursorCol() > 0 )
                        MoveCursorLeft( false );
                }
                else
                {
                    if ( IsCellEditControlCreated() )
                        DisableCellEditControl(true);
                    if ( GetGridCursorCol() < GetNumberCols()-1 )
                        MoveCursorRight( false );
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
                    
                    if (!HasSelectionMode(wxSHEET_SelectNone) &&
                        SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                                       block, true, false, &event) != EVT_VETOED)
                    {   
                        SelectRows(block.GetTop(), block.GetBottom(), false, true);
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
                    
                    if (!HasSelectionMode(wxSHEET_SelectNone) &&
                        SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                                       block, true, false, &event) != EVT_VETOED)
                    {   
                        SelectCols(block.GetLeft(), block.GetRight(), false, true);
                    }
                    
                    break;
                }
                if ( event.AltDown() && ContainsGridCell(GetGridCursorCell()) )
                {                    
                    // Toggle cell selection FIXME not sure how useful this is
                    bool sel = IsCellSelected(GetGridCursorCell());
                    wxSheetBlock block(GetGridCursorCell(), 1, 1);
                    if (!HasSelectionMode(wxSHEET_SelectNone) &&
                        SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                                       block, !sel, !sel, &event) != EVT_VETOED)
                    {   
                        ToggleCellSelection(GetGridCursorCell(), !sel, true);
                    }
                    
                    break;
                }
                if ( !IsEditable() )
                {
                    MoveCursorRight( false );
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

    m_inOnKeyDown = false;
}

void wxSheet::OnKeyUp( wxKeyEvent& event )
{
    if ( m_keySelecting && (event.GetKeyCode() == WXK_SHIFT) )
    {
        if ( !GetSelectingBlock().IsEmpty() )
        {
            if (!HasSelectionMode(wxSHEET_SelectNone) &&
                SendRangeEvent(wxEVT_SHEET_RANGE_SELECTING, 
                            GetSelectingBlock(), true, true, &event) != EVT_VETOED)
            {   
                SelectBlock(GetSelectingBlock(), true, true);
                SetSelectingBlock(wxNullSheetBlock);
            }
            
            SetSelectingAnchor(wxNullSheetCoords);
        }

        m_keySelecting = false;
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

    m_inOnKeyDown = true;
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
        case WXK_PAGEUP:
        {
            MoveCursorUpPage( event.ShiftDown() );
            break;
        }
        case WXK_PAGEDOWN:
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
                        m_keySelecting = true;
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
                        m_keySelecting = true;
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
    
    m_inOnKeyDown = false;
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
        m_mouseTimer->Start(100, true); // one shot
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
                   m_gridOrigin.y + dy*SHEET_SCROLL_LINE_Y, true, true );
    
    // send fake mouse event to process, assume left down and we're dragging
    wxMouseEvent mEvt(wxEVT_MOTION);
    mEvt.SetEventObject(win);
    mEvt.m_leftDown = true;
    mEvt.m_x = m_mousePos.x;
    mEvt.m_y = m_mousePos.y;
    
    win->ProcessEvent(mEvt);
    StartMouseTimer();
}

bool wxSheet::DoEndDragResizeRowCol()
{
    if ( m_dragLastPos < 0 )
        return false;

    if (IsCellEditControlCreated())
        DisableCellEditControl(true);
    
    // erase the last line and resize the row/col
    DrawRowColResizingMarker();

    if (HasMouseCursorMode(WXSHEET_CURSOR_RESIZE_ROW))
    {    
        int height = m_dragLastPos - GetRowTop(m_dragRowOrCol);
        const int minHeight = GetMinimalRowHeight(m_dragRowOrCol);
        if (minHeight > height) height = minHeight;
        if (height != GetRowHeight(m_dragRowOrCol))
        {
            SetRowHeight( m_dragRowOrCol, height );
            return true;
        }
    }
    else if (HasMouseCursorMode(WXSHEET_CURSOR_RESIZE_COL))
    {
        int width = m_dragLastPos - GetColLeft(m_dragRowOrCol);
        const int minWidth = GetMinimalColWidth(m_dragRowOrCol);
        if (minWidth > width) width = minWidth;
        if (width != GetColWidth(m_dragRowOrCol))
        {
            SetColWidth( m_dragRowOrCol, width );
            return true;
        }
    }
    
    return false;
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
        if ( HasSelectionMode(wxSHEET_SelectRows) )
        {
            block.SetLeft(0);
            block.SetWidth(GetNumberCols() - 1);
        }
        else if ( HasSelectionMode(wxSHEET_SelectCols) )
        {
            block.SetTop(0);
            block.SetHeight(GetNumberRows() - 1);
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
        return true;
    }

    return false;
}

bool wxSheet::SetModelValues()
{
    // Disable the editor, so it won't hide a changed value.
    // FIXME: Do we also want to save the current value of the editor first? yes?
    if (IsCellEditControlCreated())
        DisableCellEditControl(true);

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

        return true;
    }

    return false;
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

        return true;
    }
    
#ifdef DEBUG_ATTR_CACHE
    gs_nAttrCacheMisses++;
#endif
    return false;
}

// ----------------------------------------------------------------------------
// Gui Sizing functions

void wxSheet::CalcWindowSizes(bool adjustScrollBars)
{
    if (!m_gridWin || m_resizing)
        return;

    if (adjustScrollBars)
        AdjustScrollbars(false);

    m_resizing = true;
    
    int cw, ch;
    GetClientSize( &cw, &ch );
    wxRect rect;

    const int rowLabelWidth  = GetRowLabelWidth();
    const int colLabelHeight = GetColLabelHeight();
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
        rect = wxRect(0, 0, rowLabelWidth, colLabelHeight);
        if (rect != m_cornerLabelWin->GetRect())
            m_cornerLabelWin->SetSize( rect );
    }
    if ( m_colLabelWin->IsShown() )
    {
        rect = wxRect(rowLabelWidth, 0, cw-rowLabelWidth, colLabelHeight);
        if (rect != m_colLabelWin->GetRect())
            m_colLabelWin->SetSize( rect );
    }
    if ( m_rowLabelWin->IsShown() )
    {
        rect = wxRect(0, colLabelHeight, rowLabelWidth, ch-colLabelHeight);
        if (rect != m_rowLabelWin->GetRect())
            m_rowLabelWin->SetSize( rect );
    }
    if ( m_gridWin->IsShown() )
    {
        rect = wxRect(rowLabelWidth, colLabelHeight, cw-rowLabelWidth, ch-colLabelHeight);
        if (rect != m_gridWin->GetRect())
            m_gridWin->SetSize( rect );
        
        //PRINT_RECT("Set grid rect ", rect);
        //PRINT_RECT("Get grid rect ", wxRect(wxPoint(0,0), m_gridWin->GetSize()));
    }

    m_resizing = false;    

#ifdef __WXMSW__
    // MSW at least needs a little help making sure that the corner and 
    //  the splitter rects get painted
    wxClientDC dc(this);
    PaintSheetWindow(dc, wxRect(0, 0, cw, ch));
#endif // __WXMSW__
}            

void wxSheet::OnSize(wxSizeEvent& event)
{
    //PRINT_RECT(wxT("SheetRect"), GetRect());
    CalcWindowSizes(true);
    SetGridOrigin(m_gridOrigin.x, m_gridOrigin.y, true, true);
    SetEqualColWidths(GetSheetRefData()->m_equal_col_widths);    

    event.Skip();
}

wxSize wxSheet::DoGetBestSize() const
{
    wxSize displaySize(wxGetClientDisplayRect().GetSize());
    wxSize size(GetGridVirtualSize(true));
    size.x += GetRowLabelWidth();
    size.y += GetColLabelHeight();

    if ( size.x > displaySize.x/2 ) size.x = displaySize.x/2;
    if ( size.y > displaySize.y/2 ) size.y = displaySize.y/2;
    
    // NOTE: This size should be cached, but first we need to add calls to
    // InvalidateBestSize everywhere that could change the results of this
    // calculation.
    // CacheBestSize(size);

    return size;
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
                  m_control(false), m_shift(false), m_alt(false), m_meta(false)
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


// Notes:
//   Regex to CSV 
//      http://dotnetjunkies.com/WebLog/chris.taylor/archive/2004/04/09/11039.aspx
//      ((?<field>[^\",\\r\\n]+)|\"(?<field>([^\"]|\"\")+)\")(,|(?<rowbreak>\\r\\n|\\n|$))
//
//      http://www.codeguru.com/Cpp/Cpp/string/net/article.php/c8153/
//      String* pattern = S",(?=(?:[^\"]*\"[^\"]*\")*(?![^\"]*\"))"

/*
 http://www.xbeat.net/vbspeed/c_ParseCSV.php
Private Sub Class_Initialize()
    Set regEx = New VBScript_RegExp_55.RegExp
    regEx.Global = True
    regEx.IgnoreCase = True
    regEx.Pattern = "(\s*""[^""]*""\s*,)|(\s*[^,]*\s*,)" 'The magic...
End Sub


Friend Function ParseCSV02(sExpr$, arVals$()) As Long
    Dim lCnt&
    Set mc = regEx.Execute(sExpr & ",")
    ReDim arVals(mc.Count - 1) As String
    For Each m In mc
        arVals(lCnt) = Trim$(Left$(m.Value, m.Length - 1))
        If Left$(arVals(lCnt), 1) = """" And Right$(arVals(lCnt), 1) = """" Then
            arVals(lCnt) = Mid$(arVals(lCnt), 2, Len(arVals(lCnt)) - 2)
        End If
        lCnt = lCnt + 1
    Next m
    ParseCSV02 = mc.Count
End Function
*/

#include "wx/regex.h"



class wxCSV_IO
{
public :
    wxCSV_IO( bool t=false );
    
   
    wxArrayString ParseLine(const wxString& line);
    bool Test();

    wxRegEx m_regex;
};

wxCSV_IO::wxCSV_IO(bool t)
{
        

//                      "(\s*""[^""]*""\s*,)|(\s*[^,]*\s*,)" 'The magic...
//    m_regex.Compile(wxT("(\\s*\"[^\"]*\"\\s*,)|(\\s*[^,]*\\s*,)"), wxRE_ICASE);
    m_regex.Compile(wxT("(\\s*\"[^\"]*\"\\s*,)|(\\s*[^,]*\\s*,)"), wxRE_ICASE);
    //m_regex.Compile(wxT(",(?=([^\"]*"[^\"]*\")*(?![^\"]*\"))"),  wxRE_ICASE); 
    //m_regex.Compile(wxT(",(?=(?:[^\"]*\"[^\"]*\")*(?![^\"]*\"))"), wxRE_ICASE); 
    
    if (t) Test();
}

wxArrayString wxCSV_IO::ParseLine(const wxString& line_)
{
    wxArrayString values;
    wxString line(line_ + wxT(",")); // add trailing ',' for "a,b" case

    // See if there is a match at all
    while ( m_regex.Matches(line) )
    {
        // Get the first match (BUG? can't use GetMatch(line, n))
        wxString val(m_regex.GetMatch(line, 0));
        size_t len = val.Length();
        if ((len > 0u) && val.Last() == wxT(',')) // GetMatch returns "val,"
            val = val.Mid(0, len-1);
        
        //wxPrintf(wxT("VALUE '")+val+wxT("' LINE '")+line+wxT("'\n"));
        
        line = line.Mid(len);              // remove the value from the line
        val = val.Strip(wxString::both);   // strip blank spaces from value 
                                           // should've quoted ie. "  a a "
        len = val.Length();
        // remove the leading and trailing quotes
        if ((len > 1u) && (val.GetChar(0) == wxT('\"')) && (val.GetChar(len-1) == wxT('\"')))
            val = val.AfterFirst(wxT('\"')).BeforeLast(wxT('\"'));

        values.Add(val);
    }

    return values;
}


wxString Joint(const wxArrayString& a) 
{ 
    if (a.GetCount() == 0u) return wxEmptyString;
        
    wxString s = wxT("'")+ a[0] + wxT("'\t "); 
    for (size_t n = 1u; n < a.GetCount(); n++) s += wxT("'")+ a[n] + wxT("'\t ");
    return s;
}

void CSV_TEST(const wxString& instr, const wxArrayString& ans, const wxArrayString &res)
{
    wxString msg(wxT("'") + instr + wxT("'\t ") + Joint(ans) + wxT(" ** \t ") + Joint(res));
    
    if ( ans.GetCount() != res.GetCount() )
        wxPrintf(wxT("COUNT MISMATCH ERROR! \n"));

    for (size_t n = 0; n < wxMin(ans.GetCount(), res.GetCount()); n++)
        if (ans[n] != res[n]) wxPrintf(wxT("Error in item %u\n"), n);
    
    wxPrintf(msg + wxT("\n"));
}

#define CSVT1(str, a1) { wxArrayString ar; ar.Add(wxT(a1)); CSV_TEST(wxT(str), ar, ParseLine(wxT(str))); }
#define CSVT2(str, a1, a2) { wxArrayString ar; ar.Add(wxT(a1)); ar.Add(wxT(a2)); CSV_TEST(wxT(str), ar, ParseLine(wxT(str))); }
#define CSVT3(str, a1, a2, a3) { wxArrayString ar; ar.Add(wxT(a1)); ar.Add(wxT(a2)); ar.Add(wxT(a3)); CSV_TEST(wxT(str), ar, ParseLine(wxT(str))); }

bool wxCSV_IO::Test()
{  
    CSVT3("a,b,c", "a", "b", "c")
    CSVT3("\"a\",b,c", "a", "b", "c")
    CSVT3("'a',b,c", "'a'", "b", "c")
    CSVT3("  a  ,  b  ,  c  ", "a", "b", "c")  
    CSVT2("aa,bb;cc", "aa", "bb;cc")
    CSVT1("", "")
    CSVT1("a", "a")
    CSVT3(",b,", "", "b", "")
    CSVT3(",,c", "", "", "c")
    CSVT3(",,", "", "", "")
    CSVT2("\"\",b", "", "b")
    CSVT2("\" \",b", " ", "b")
    CSVT1("\"a,b\"", "a,b")
    CSVT2("\"a,b\",c", "a,b", "c")
    CSVT2("\" a , b \", c", " a , b ", "c")
    CSVT2("a b,c", "a b", "c")
    CSVT2("a\"b,c", "a\"b", "c")
    CSVT2("\"a\"\"b\",c", "a\"b", "c")
    CSVT2("a\"\"b,c", "a\"\"b", "c")       // BROKEN
    CSVT3("a,b\",c", "a", "b\"", "c")
    CSVT3("a,b\"\",c", "a", "b\"\"", "c")
    CSVT3("a,\"B: \"\"Hi, I'm B\"\"\",c", "a", "B: \"Hi, I'm B\"", "c")
   
    return true;
}

//wxCSV_IO csvTest(true);
