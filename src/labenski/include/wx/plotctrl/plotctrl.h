/////////////////////////////////////////////////////////////////////////////
// Name:        plotctrl.h
// Purpose:     wxPlotCtrl
// Author:      John Labenski, Robert Roebling
// Modified by:
// Created:     6/5/2002
// Copyright:   (c) John Labenski, Robert Roebling
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_PLOTCTRL_H_
#define _WX_PLOTCTRL_H_

#include "wx/defs.h"
#include "wx/geometry.h"
#include "wx/bitmap.h"
#include "wx/window.h"
#include "wx/plotctrl/plotdefs.h"
#include "wx/plotctrl/plotcurv.h"
#include "wx/plotctrl/plotdata.h"
#include "wx/plotctrl/plotmark.h"
#include "wx/plotctrl/plotfunc.h"
#include "wx/wxthings/range.h"

class WXDLLEXPORT wxDC;
class WXDLLEXPORT wxNotifyEvent;
class WXDLLEXPORT wxPaintEvent;
class WXDLLEXPORT wxMouseEvent;
class WXDLLEXPORT wxKeyEvent;
class WXDLLEXPORT wxTimer;
class WXDLLEXPORT wxTimerEvent;
class WXDLLEXPORT wxEraseEvent;
class WXDLLEXPORT wxScrollBar;
class WXDLLEXPORT wxBitmap;
class WXDLLEXPORT wxTextCtrl;

class WXDLLIMPEXP_THINGS wxRangeIntSelection;
class WXDLLIMPEXP_THINGS wxRangeDoubleSelection;
class WXDLLIMPEXP_THINGS wxArrayRangeIntSelection;
class WXDLLIMPEXP_THINGS wxArrayRangeDoubleSelection;

class WXDLLIMPEXP_PLOTCTRL wxPlotArea;
class WXDLLIMPEXP_PLOTCTRL wxPlotAxis;
class WXDLLIMPEXP_PLOTCTRL wxPlotCtrl;
class WXDLLIMPEXP_PLOTCTRL wxPlotEvent;

class WXDLLIMPEXP_PLOTCTRL wxPlotCurve;
class WXDLLIMPEXP_PLOTCTRL wxPlotData;
class WXDLLIMPEXP_PLOTCTRL wxPlotFunction;

class WXDLLIMPEXP_PLOTCTRL wxPlotDrawerArea;
class WXDLLIMPEXP_PLOTCTRL wxPlotDrawerXAxis;
class WXDLLIMPEXP_PLOTCTRL wxPlotDrawerYAxis;
class WXDLLIMPEXP_PLOTCTRL wxPlotDrawerKey;
class WXDLLIMPEXP_PLOTCTRL wxPlotDrawerCurve;
class WXDLLIMPEXP_PLOTCTRL wxPlotDrawerDataCurve;
class WXDLLIMPEXP_PLOTCTRL wxPlotDrawerMarker;

//-----------------------------------------------------------------------------
// wxPlot Constants
//-----------------------------------------------------------------------------
#ifdef __VISUALC__
    #include <yvals.h>
    // disable warning for stl::numeric_limits,
    // C++ language change: to explicitly specialize
    #pragma warning(disable:4663)
#endif // __VISUALC__

#include <limits>
extern std::numeric_limits<wxDouble> wxDouble_limits;

extern const wxDouble wxPlot_MIN_DBL;   // = wxDouble_limits.min()*10
extern const wxDouble wxPlot_MAX_DBL;   // = wxDouble_limits.max()/10
extern const wxDouble wxPlot_MAX_RANGE; // = wxPlot_MAX_DBL*2

#define CURSOR_GRAB (wxCURSOR_MAX+100)  // A hand cursor with fingers closed

#include "wx/dynarray.h"
#ifndef WX_DECLARE_OBJARRAY_WITH_DECL // for wx2.4 backwards compatibility
    #define WX_DECLARE_OBJARRAY_WITH_DECL(T, name, expmode) WX_DECLARE_USER_EXPORTED_OBJARRAY(T, name, WXDLLIMPEXP_PLOTCTRL)
#endif
WX_DECLARE_OBJARRAY_WITH_DECL(wxPoint2DDouble, wxArrayPoint2DDouble, class WXDLLIMPEXP_PLOTCTRL);
WX_DECLARE_OBJARRAY_WITH_DECL(wxRect2DDouble,  wxArrayRect2DDouble, class WXDLLIMPEXP_PLOTCTRL);
WX_DECLARE_OBJARRAY_WITH_DECL(wxPlotCurve,     wxArrayPlotCurve, class WXDLLIMPEXP_PLOTCTRL);

// What type of axis for wxPlotAxis window
enum wxPlotAxis_Type
{
    wxPLOT_X_AXIS  = 0x0020,
    wxPLOT_Y_AXIS  = 0x0200,
    wxPLOT_DEFAULT = wxPLOT_X_AXIS|wxPLOT_Y_AXIS
};

// What is the function of the mouse during left down and dragging
enum wxPlotMouse_Type
{
    wxPLOT_MOUSE_NOTHING,   // do nothing
    wxPLOT_MOUSE_ZOOM,      // zoom into the plot
    wxPLOT_MOUSE_SELECT,    // select points in the active curve
    wxPLOT_MOUSE_DESELECT,  // deselect points in the active curve
    wxPLOT_MOUSE_PAN        // offset the origin
};

// What sort of marker should be drawn for mouse left down and dragging
enum wxPlotMarker_Type
{
    wxPLOT_MARKER_NONE,   // draw nothing
    wxPLOT_MARKER_RECT,   // draw a rectangle
    wxPLOT_MARKER_VERT,   // draw two vertical lines
    wxPLOT_MARKER_HORIZ   // draw two horizonal lines
};

// How does the selection mechanism act to selections
enum wxPlotSelection_Type
{
    wxPLOT_SELECT_NONE,         // no selections
    wxPLOT_SELECT_SINGLE,       // only one selection in one curve at a time
    wxPLOT_SELECT_SINGLE_CURVE, // only one curve may have selections at once
    wxPLOT_SELECT_SINGLE_PER_CURVE, // multiple curves may one have one selection each
    wxPLOT_SELECT_MULTIPLE      // multiple curves may have multiple selections
};

// Redraw parts or all of the windows
enum wxPlotRedraw_Type
{
    wxPLOT_REDRAW_NONE       = 0x000,  // do nothing
    wxPLOT_REDRAW_PLOT       = 0x001,  // redraw only the plot area
    wxPLOT_REDRAW_XAXIS      = 0x002,  // redraw x-axis, combine w/ redraw_plot
    wxPLOT_REDRAW_YAXIS      = 0x004,  // redraw y-axis, combine w/ redraw_plot
    wxPLOT_REDRAW_WINDOW     = 0x008,  // wxPlotCtrl container window
    wxPLOT_REDRAW_WHOLEPLOT  = wxPLOT_REDRAW_PLOT|wxPLOT_REDRAW_XAXIS|wxPLOT_REDRAW_YAXIS,
    wxPLOT_REDRAW_EVERYTHING = wxPLOT_REDRAW_WHOLEPLOT|wxPLOT_REDRAW_WINDOW,
    wxPLOT_REDRAW_BLOCKER    = 0x010   // don't let OnPaint redraw, used internally
};

// Styles for the different plot window components
enum wxPlotStyle_Type
{
    wxPLOT_STYLE_WINDOW,
    wxPLOT_STYLE_LABELS,
    wxPLOT_STYLE_TITLE,
    wxPLOT_STYLE_XAXIS_LABEL,
    wxPLOT_STYLE_YAXIS_LABEL,
    wxPLOT_STYLE_XAXIS_TICS,
    wxPLOT_STYLE_YAXIS_TICS,
    wxPLOT_STYLE_KEY,

    wxPLOT_STYLE_LAST
};

enum wxPlotStyleUse_Type
{
    wxPLOT_STYLEUSE_FORECOLOUR = 0x0001,
    wxPLOT_STYLEUSE_BACKCOLOUR = 0x0002,
    wxPLOT_STYLEUSE_FONT       = 0x0004,
    wxPLOT_STYLEUSE_LINEWIDTH  = 0x0008
};

//-----------------------------------------------------------------------------
// wxPlotArea - window where the plot is drawn (privately used in wxPlotCtrl)
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_PLOTCTRL wxPlotArea : public wxWindow
{
public:
    wxPlotArea( wxWindow *parent, wxWindowID win_id )
    {
        Init();
        (void)Create(parent, win_id);
    }

    bool Create( wxWindow *parent, wxWindowID win_id );
    virtual ~wxPlotArea() {}

    // Get the owner (parent) wxPlotCtrl
    wxPlotCtrl *GetOwner() const { return m_owner; }

    // Draw the area of the plot window in client coords bounded by rect
    //  resizes backing bitmap if necessary
    void CreateBitmap( const wxRect &rect );

    // implementation
    void OnEraseBackground( wxEraseEvent & ) { }
    void OnPaint( wxPaintEvent &event );
    void OnMouse( wxMouseEvent &event );
    void OnChar( wxKeyEvent &event );
    void OnKeyDown( wxKeyEvent &event );
    void OnKeyUp( wxKeyEvent &event );

    wxRect   m_mouseRect; // mouse drag rectangle, or 0,0,0,0 when not dragging
    wxPoint  m_mousePt;   // last mouse position
    wxBitmap m_bitmap;
    wxPlotCtrl *m_owner;

private:
    void Init();
    DECLARE_CLASS(wxPlotArea)
    DECLARE_EVENT_TABLE()
};

//-----------------------------------------------------------------------------
// wxPlotAxis - X or Y axis window (privately used in wxPlotCtrl)
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_PLOTCTRL wxPlotAxis : public wxWindow
{
public:
    wxPlotAxis( wxWindow *parent, wxWindowID win_id, wxPlotAxis_Type style )
    {
        Init();
        (void)Create( parent, win_id, style );
    }

    bool Create( wxWindow *parent, wxWindowID win_id, wxPlotAxis_Type style );
    virtual ~wxPlotAxis() {}

    // Create the backing bitmap of the window contents
    void CreateBitmap();
    // Get the owner (parent) wxPlotCtrl
    wxPlotCtrl *GetOwner() const { return m_owner; }

    bool IsXAxis() const { return (m_style & wxPLOT_X_AXIS) != 0; }

    // implementation
    void OnEraseBackground( wxEraseEvent & ) { }
    void OnPaint( wxPaintEvent &event );
    void OnMouse( wxMouseEvent &event );
    void OnChar( wxKeyEvent &event );

    wxPoint m_mousePt;  // last mouse position
    wxPlotAxis_Type m_style;
    wxBitmap m_bitmap;
    wxPlotCtrl *m_owner;

private:
    void Init();
    DECLARE_CLASS(wxPlotAxis)
    DECLARE_EVENT_TABLE()
};

//-----------------------------------------------------------------------------
// wxPlotCtrl - window to display wxPlotCurves, public interface
//
// notes:
//    call CalcBoundingRect() whenever you modify a curve's values so that
//    the default size of the plot is correct, see wxPlotCurve::GetBoundingRect
//
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_PLOTCTRL wxPlotCtrl : public wxWindow
{
public:
    wxPlotCtrl() : wxWindow() { Init(); }

    wxPlotCtrl( wxWindow *parent, wxWindowID win_id = wxID_ANY,
                  const wxPoint &pos = wxDefaultPosition,
                  const wxSize &size = wxDefaultSize,
                  wxPlotAxis_Type flags = wxPLOT_DEFAULT,
                  const wxString& name = wxT("wxPlotCtrl") )
    {
        Init();
        (void)Create(parent, win_id, pos, size, flags, name);
    }

    bool Create( wxWindow *parent, wxWindowID id = wxID_ANY,
                 const wxPoint &pos = wxDefaultPosition,
                 const wxSize &size = wxDefaultSize,
                 wxPlotAxis_Type flags = wxPLOT_DEFAULT,
                 const wxString& name = wxT("wxPlotCtrl") );

    virtual ~wxPlotCtrl();

    // ------------------------------------------------------------------------
    // Curve Accessors
    //
    // note: the curves are stored in an array casted to class wxPlotCurve
    //       in order to know the type and use it as a wxPlotFunction, wxPlotData
    //       or a class derived from one of these use
    //       wxPlotData *pd = wxDynamicCast( GetCurve(i), wxPlotData );
    //       pd will be NULL if GetCurve isn't a wxPlotData (or derived from it)
    // ------------------------------------------------------------------------

    // Add a curve to the plot, takes ownership of the curve and deletes it
    bool AddCurve( wxPlotCurve *curve, bool select=true, bool send_event=false );
    // Add a curve to the plot, increases ref count
    bool AddCurve( const wxPlotCurve &curve, bool select=true, bool send_event=false );
    // Delete this curve
    bool DeleteCurve( wxPlotCurve* curve, bool send_event=false );
    // Delete this curve, if curve_index = -1, delete all curves
    bool DeleteCurve( int curve_index, bool send_event=false );

    // Total number of curves associated with the plotctrl
    int GetCurveCount() const { return m_curves.GetCount(); }
    bool CurveIndexOk(int curve_index) const
        { return (curve_index>=0) && (curve_index < int(m_curves.GetCount())); }

    // Get the curve at this index
    wxPlotCurve *GetCurve( int curve_index ) const;
    // returns NULL if curve_index is not wxPlotData or derived from it
    wxPlotData *GetDataCurve( int curve_index ) const { return wxDynamicCast(GetCurve(curve_index), wxPlotData); }
    // returns NULL if curve_index is not wxPlotFunction or derived from it
    wxPlotFunction *GetFunctionCurve( int curve_index ) const { return wxDynamicCast(GetCurve(curve_index), wxPlotFunction); }
    // returns true if the curve is a wxPlotData curve
    bool IsDataCurve( int curve_index ) const { return GetDataCurve(curve_index) != NULL; }
    // returns true if the curve is a wxPlotFunction curve
    bool IsFunctionCurve( int curve_index ) const { return GetFunctionCurve(curve_index) != NULL; }
    // Else the function must be some sort of subclassed wxPlotCurve

    // Sets the currently active curve, NULL for none active
    void SetActiveCurve( wxPlotCurve* curve, bool send_event=false );
    // Gets the currently active curve, NULL if none
    wxPlotCurve *GetActiveCurve() const { return m_activeCurve; }
    // Gets the currently active curve as a wxPlotFunction
    //    returns NULL if its not a wxPlotFunction, even if a curve is active
    wxPlotFunction *GetActiveFuncCurve() const { return wxDynamicCast(m_activeCurve, wxPlotFunction); }
    // Gets the currently active curve as a wxPlotData
    //    returns NULL if its not a wxPlotData, even if a curve is active
    wxPlotData *GetActiveDataCurve() const { return wxDynamicCast(m_activeCurve, wxPlotData); }

    // Set the curve_index curve active, use -1 to have none selected
    void SetActiveIndex( int curve_index, bool send_event=false );
    // Get the index of the active curve, returns -1 if none active
    int GetActiveIndex() const { return m_active_index; }

    // Get an array of curve indexes that are of class wxPlotData,
    //   wxPlotFunction or derived from either
    wxArrayInt GetPlotDataIndexes() const;
    wxArrayInt GetPlotFunctionIndexes() const;

    //-------------------------------------------------------------------------
    // Markers
    //-------------------------------------------------------------------------

    // Add a marker to be displayed
    int AddMarker( const wxPlotMarker& marker );

    void RemoveMarker(int marker);
    void ClearMarkers();

    wxPlotMarker GetMarker(int marker) const;
    wxArrayPlotMarker& GetMarkerArray() { return m_plotMarkers; }

    //-------------------------------------------------------------------------
    // Cursor position - a single selected point in a curve
    //-------------------------------------------------------------------------

    // Hide the cursor
    void InvalidateCursor(bool send_event=false);
    // Does the cursor point to a valid curve and if a data curve a valid data index
    bool IsCursorValid();
    // Get the index of the curve that the cursor is associated with, -1 if none
    int GetCursorCurveIndex() const { return m_cursor_curve; }
    // Get the index into the wxPlotData curve of the cursor, -1 if not on a data curve
    int GetCursorDataIndex() const { return m_cursor_index; }
    // Get the location of the cursor, valid for all curve types if cursor valid
    wxPoint2DDouble GetCursorPoint();
    // Set the curve and the index into the wxPlotData of the cursor
    //   curve_index must point to a data curve and cursor_index valid in data
    bool SetCursorDataIndex(int curve_index, int cursor_index, bool send_event=false);
    // Set the curve and the x-value of the cursor, valid for all curve types
    //    if curve_index is a wxPlotData curve it finds nearest index
    bool SetCursorXPoint(int curve_index, double x, bool send_event=false);
    // The cursor must be valid, if center then it centers the plot on the cursor
    //    if !center then make the cursor just barely visible by shifting the view
    void MakeCursorVisible(bool center, bool send_event=false);

    //-------------------------------------------------------------------------
    // Selected points
    //-------------------------------------------------------------------------

    // Is anything selected in a particular curve or any curve if index = -1
    bool HasSelection(int curve_index = -1) const;

    // double valued selections can be made for wxPlotCurves and wxPlotFunctions
    //   for curves that are wxPlotData, the selection is empty, see GetDataCurveSelections
    const wxArrayRangeDoubleSelection& GetCurveSelections() const { return m_curveSelections; }
    // Get the particluar selection for the curve at index curve_index
    wxRangeDoubleSelection *GetCurveSelection(int curve_index) const;

    // the selections of wxPlotData curves are of the indexes of the data
    //   for curves that are wxPlotCurves or wxPlotFunctions the selection is empty
    const wxArrayRangeIntSelection& GetDataCurveSelections() const { return m_dataSelections; }
    // Get the particluar selection for the curve at index curve_index
    wxRangeIntSelection *GetDataCurveSelection(int curve_index) const;

    // Get the number of individual selections of this curve
    int GetSelectedRangeCount(int curve_index) const;

    // Selects points in a curve using a rectangular selection (see select range)
    //   this works for all plotcurve classes, for wxPlotData they're converted to the indexes however
    //   if there's nothing to select or already selected it returns false
    //   if curve_index == -1 then try to select points in all curves
    bool SelectRectangle( int curve_index, const wxRect2DDouble &rect, bool send_event = false)
        { return DoSelectRectangle(curve_index, rect, true, send_event); }
    bool DeselectRectangle( int curve_index, const wxRect2DDouble &rect, bool send_event = false)
        { return DoSelectRectangle(curve_index, rect, false, send_event); }

    // Select a single point wxRangeDouble(pt,pt) or a data range wxRangeDouble(pt1, pt2)
    //   this works for all plotcurve classes, for wxPlotData they're converted to the indexes however
    //   if there's nothing to select or already selected it returns false
    //   if curve_index == -1 then try to select points in all curves
    bool SelectXRange(int curve_index, const wxRangeDouble &range, bool send_event = false)
        { return DoSelectRectangle(curve_index, wxRect2DDouble(range.m_min, -wxPlot_MAX_DBL, range.GetRange(), wxPlot_MAX_RANGE), true, send_event); }
    bool DeselectXRange(int curve_index, const wxRangeDouble &range, bool send_event = false)
        { return DoSelectRectangle(curve_index, wxRect2DDouble(range.m_min, -wxPlot_MAX_DBL, range.GetRange(), wxPlot_MAX_RANGE), false, send_event); }
    bool SelectYRange(int curve_index, const wxRangeDouble &range, bool send_event = false)
        { return DoSelectRectangle(curve_index, wxRect2DDouble(-wxPlot_MAX_DBL, range.m_min, wxPlot_MAX_RANGE, range.GetRange()), true, send_event); }
    bool DeselectYRange(int curve_index, const wxRangeDouble &range, bool send_event = false)
        { return DoSelectRectangle(curve_index, wxRect2DDouble(-wxPlot_MAX_DBL, range.m_min, wxPlot_MAX_RANGE, range.GetRange()), false, send_event); }

    // Select a single point wxRangeInt(pt, pt) or a range of points wxRangeInt(pt1, pt2)
    //   if there's nothing to select or already selected it returns false,
    //   this ONLY works for wxPlotData curves
    bool SelectDataRange(int curve_index, const wxRangeInt &range, bool send_event = false)
        { return DoSelectDataRange(curve_index, range, true, send_event); }
    bool DeselectDataRange(int curve_index, const wxRangeInt &range, bool send_event = false)
        { return DoSelectDataRange(curve_index, range, false, send_event); }

    // Clear the ranges, if curve_index = -1 then clear them all
    bool ClearSelectedRanges(int curve_index, bool send_event = false);

    // internal use, or not...
    virtual bool DoSelectRectangle(int curve_index, const wxRect2DDouble &rect, bool select, bool send_event = false);
    virtual bool DoSelectDataRange(int curve_index, const wxRangeInt &range, bool select, bool send_event = false);
    // called from DoSelect... when selecting to ensure that the current selection
    // matches the SetSelectionType by unselecting as appropriate
    // The input curve_index implies that a selection will be made for that curve
    // This is not called for a deselection event.
    virtual bool UpdateSelectionState(int curve_index, bool send_event);

    // Set how the selections mechanism operates, see enum wxPlotSelection_Type
    //   You are responsible to clean up the selections if you change this,
    //   however it won't fail, but may be out of sync.
    void SetSelectionType(wxPlotSelection_Type type) { m_selection_type = type; }
    int GetSelectionType() const { return m_selection_type; }

    // ------------------------------------------------------------------------
    // Get/Set origin, size, and Zoom in/out of view, set scaling, size...
    // ------------------------------------------------------------------------

    // make this curve fully visible or -1 to make all curves visible
    //   uses wxPlotCurve::GetBoundingRect()
    //   data curves have known sizes, function curves use default rect, unless set
    bool MakeCurveVisible(int curve_index, bool send_event=false);

    // Set the origin of the plot window
    bool SetOrigin( double origin_x, double origin_y, bool send_event=false )
        { return SetZoom( m_zoom.m_x, m_zoom.m_y, origin_x, origin_y, send_event ); }

    // Get the bounds of the plot window view in plot coords
    const wxRect2DDouble& GetViewRect() const { return m_viewRect; }
    // Set the bounds of the plot window
    bool SetViewRect(const wxRect2DDouble &view, bool send_event=false);

    // Get the zoom factor = (pixel size of window)/(GetViewRect().m_width or height)
    const wxPoint2DDouble& GetZoom() const { return m_zoom; }

    // Zoom, if zoom_x or zoom_y <= 0 then fit that axis to window and center it
    bool SetZoom( const wxPoint2DDouble &zoom, bool around_center=true, bool send_event=false );
    virtual bool SetZoom( double zoom_x, double zoom_y,
                  double origin_x, double origin_y, bool send_event=false );

    // Zoom in client coordinates, window.[xy] is top left (unlike plot axis)
    bool SetZoom( const wxRect &window, bool send_event=false );

    // Set/Get the default size the plot should take when either no curves are
    //   loaded or only plot(curves/functions) that have no bounds are loaded
    //   The width and the height must both be > 0
    void SetDefaultBoundingRect( const wxRect2DDouble &rect, bool send_event = false );
    const wxRect2DDouble& GetDefaultBoundingRect() const { return m_defaultPlotRect; }

    // Get the bounding rect of all the curves,
    //    equals the default if no curves or no bounds on the curves
    const wxRect2DDouble& GetCurveBoundingRect() const { return m_curveBoundingRect; }

    // Get client rect of the wxPlotArea window, 0, 0, client_width, client_height
    const wxRect& GetPlotAreaRect() const { return m_areaClientRect; }

    // The history of mouse drag rects are saved (mouseFunc_zoom)
    void NextHistoryView(bool foward, bool send_event=false);
    int  GetHistoryViewCount() const { return m_historyViews.GetCount(); }
    int  GetHistoryViewIndex() const { return m_history_views_index; }

    // Fix the aspect ratio of the x and y axes, if set then when the zoom is
    //  set the smaller of the two (x or y) zooms is multiplied by the ratio
    //  to calculate the other.
    void SetFixAspectRatio(bool fix, double ratio = 1.0);
    void FixAspectRatio( double *zoom_x, double *zoom_y, double *origin_x, double *origin_y );

    // ------------------------------------------------------------------------
    // Mouse Functions for the area window
    // ------------------------------------------------------------------------

    // The current (last) pixel position of the mouse in the plotArea
    const wxPoint& GetAreaMouseCoord() const { return m_area->m_mousePt; }

    // The current plotArea position of the mouse cursor
    wxPoint2DDouble GetAreaMousePoint() const
        { return wxPoint2DDouble(GetPlotCoordFromClientX(m_area->m_mousePt.x),
                                 GetPlotCoordFromClientY(m_area->m_mousePt.y)); }

    // Get the rect during dragging mouse, else 0
    const wxRect& GetAreaMouseMarkedRect() const { return m_area->m_mouseRect; }

    // Set what the mouse will do for different actions
    void SetAreaMouseFunction(wxPlotMouse_Type func, bool send_event=false);
    wxPlotMouse_Type GetAreaMouseFunction() const { return m_area_mouse_func; }

    // Set what sort of marker should be drawn when dragging mouse
    void SetAreaMouseMarker(wxPlotMarker_Type type);
    wxPlotMarker_Type GetAreaMouseMarker() const { return m_area_mouse_marker; }

    // Set the mouse cursor wxCURSOR_XXX + CURSOR_GRAB for the plot area
    void SetAreaMouseCursor(int cursorid);

    // ------------------------------------------------------------------------
    // Options
    // ------------------------------------------------------------------------

    // Scroll the window only when the mouse button is released (for slow machines)
    bool GetScrollOnThumbRelease() const { return m_scroll_on_thumb_release; }
    void SetScrollOnThumbRelease( bool scrollOnThumbRelease = true )
        { m_scroll_on_thumb_release = scrollOnThumbRelease; }

    // Use a full width/height crosshair as a cursor
    bool GetCrossHairCursor() const { return m_crosshair_cursor; }
    void SetCrossHairCursor( bool useCrosshairCursor = false )
        { m_crosshair_cursor = useCrosshairCursor;
          m_area->m_mousePt = wxPoint(-1,-1); Redraw(wxPLOT_REDRAW_PLOT); }

    // Draw the data curve symbols on the plotctrl
    bool GetDrawSymbols() const { return m_draw_symbols; }
    void SetDrawSymbols( bool drawsymbols = true )
        { m_draw_symbols = drawsymbols; Redraw(wxPLOT_REDRAW_PLOT); }

    // Draw the interconnecting straight lines between data points
    bool GetDrawLines() const { return m_draw_lines; }
    void SetDrawLines( bool drawlines = true )
        { m_draw_lines = drawlines; Redraw(wxPLOT_REDRAW_PLOT); }

    // Draw the interconnecting splines between data points
    bool GetDrawSpline() const { return m_draw_spline; }
    void SetDrawSpline( bool drawspline = false )
        { m_draw_spline = drawspline; Redraw(wxPLOT_REDRAW_PLOT); }

    // Draw the plot grid over the whole window, else just tick marks at edge
    bool GetDrawGrid() const { return m_draw_grid; }
    void SetDrawGrid( bool drawgrid = true )
        { m_draw_grid = drawgrid; Redraw(wxPLOT_REDRAW_PLOT); }

    // Try to fit the window to show all curves when a new curve is added
    bool GetFitPlotOnNewCurve() const { return m_fit_on_new_curve; }
    void SetFitPlotOnNewCurve( bool fit = true ) { m_fit_on_new_curve = fit; }

    // Set the focus to this window if the mouse enters it, otherwise you have to click
    //   sometimes convenient, but often this is annoying
    bool GetGreedyFocus() const { return m_greedy_focus; }
    void SetGreedyFocus(bool grab_focus = false) { m_greedy_focus = grab_focus; }

    // turn on or off the Correct Ticks functions.  Turning this off allows a graph
    // that scrolls to scroll smoothly in the direction expected.  Turning this
    // on (default) gives better accuracy for 'mouse hover' information display
    bool GetCorrectTicks() const { return m_correct_ticks; }
    void SetCorrectTicks( bool correct = true ) { m_correct_ticks = correct; }

    // get/set the width of a 1 pixel pen in mm for printing
    double GetPrintingPenWidth(void) { return m_pen_print_width; }
    void SetPrintingPenWidth(double width) { m_pen_print_width = width; }

    // ------------------------------------------------------------------------
    // Colours & Fonts for windows, labels, title...
    // ------------------------------------------------------------------------

    // Get/Set the background colour of all the plot windows, default white
    wxColour     GetBackgroundColour() const { return m_area->GetBackgroundColour(); }
    virtual bool SetBackgroundColour( const wxColour &colour );

    // Get/Set the colour of the grid lines in the plot area, default grey
    wxColour GetGridColour() const { return m_area->GetForegroundColour(); }
    void     SetGridColour( const wxColour &colour );

    // Get/Set the colour of the border around the plot area, default black
    wxColour GetBorderColour() const { return m_borderColour; }
    void     SetBorderColour( const wxColour &colour );

    // Get/Set the colour of the cursor marker, default green
    wxColour GetCursorColour() const;
    void     SetCursorColour( const wxColour &colour );
    // Get/Set the cursor size, the size of the circle drawn for the cursor.
    //   set size to 0 to not have the cursor shown (default = 2)
    int GetCursorSize() const;
    void SetCursorSize(int size);

    // Get/Set the axis numbers font and colour, default normal & black
    wxFont   GetAxisFont() const;
    wxColour GetAxisColour() const;
    void     SetAxisFont( const wxFont &font );
    void     SetAxisColour( const wxColour &colour );

    // Get/Set axis label fonts and colour, default swiss and black
    wxFont   GetAxisLabelFont() const;
    wxColour GetAxisLabelColour() const;
    void     SetAxisLabelFont( const wxFont &font );
    void     SetAxisLabelColour( const wxColour &colour );

    // Get/Set the title font and colour, default swiss and black
    wxFont   GetPlotTitleFont() const   { return m_titleFont; }
    wxColour GetPlotTitleColour() const { return m_titleColour; }
    void     SetPlotTitleFont( const wxFont &font );
    void     SetPlotTitleColour( const wxColour &colour );

    // Get/Set the key font and colour
    wxFont   GetKeyFont() const;
    wxColour GetKeyColour() const;
    void     SetKeyFont( const wxFont &font );
    void     SetKeyColour( const wxColour & colour );

    // ------------------------------------------------------------------------
    // Title, axis labels, and key values and visibility
    // ------------------------------------------------------------------------

    // Get/Set showing x and/or y axes
    void SetShowXAxis(bool show) { m_show_xAxis = show; }
    void SetShowYAxis(bool show) { m_show_yAxis = show; }
    bool GetShowXAxis() { return m_show_xAxis; }
    bool GetShowYAxis() { return m_show_yAxis; }

    // Get/Set and show/hide the axis labels
    const wxString& GetXAxisLabel() const { return m_xLabel; }
    const wxString& GetYAxisLabel() const { return m_yLabel; }
    void SetXAxisLabel(const wxString &label);
    void SetYAxisLabel(const wxString &label);
    bool GetShowXAxisLabel() const { return m_show_xlabel; }
    bool GetShowYAxisLabel() const { return m_show_ylabel; }
    void SetShowXAxisLabel( bool show ) { m_show_xlabel = show; DoSize(); }
    void SetShowYAxisLabel( bool show ) { m_show_ylabel = show; DoSize(); }

    // Get/Set and show/hide the title
    const wxString& GetPlotTitle() const { return m_title; }
    void SetPlotTitle(const wxString &title);
    bool GetShowPlotTitle() const { return m_show_title; }
    void SetShowPlotTitle( bool show ) { m_show_title = show; DoSize(); }

    // Show a key with the function/data names, pos is %width and %height (0-100)
    const wxString& GetKeyString() const { return m_keyString; }
    bool GetShowKey() const { return m_show_key; }
    void SetShowKey(bool show) { m_show_key = show; Redraw(wxPLOT_REDRAW_PLOT); }
    wxPoint GetKeyPosition() const;
    bool GetKeyInside() const;
    void SetKeyPosition(const wxPoint &pos, bool stay_inside = true);

    // used internally to update the key string from the curve names
    virtual void CreateKeyString();

    // set the minimum value to be displayed as an exponential on the axes
    long GetMinExpValue() const { return m_min_exponential; }
    void SetMinExpValue( long min ) { m_min_exponential = min; }

    // ------------------------------------------------------------------------
    // Title, axis label editor control
    // ------------------------------------------------------------------------

    enum wxPlotCtrlTextCtrl_Type
    {
        wxPLOT_EDIT_TITLE = 1,
        wxPLOT_EDIT_XAXIS,
        wxPLOT_EDIT_YAXIS,
    };

    // Sends the wxEVT_PLOT_BEGIN_TITLE_(X/Y_LABEL)_EDIT event if send_event
    //  which can be vetoed
    void ShowTextCtrl(wxPlotCtrlTextCtrl_Type type, bool send_event = false);
    // Sends the wxEVT_PLOT_END_TITLE_(X/Y_LABEL)_EDIT event if send_event
    //  which can be vetoed
    void HideTextCtrl(bool save_value = true, bool send_event = false);
    bool IsTextCtrlShown() const;

    // ------------------------------------------------------------------------
    // Event processing
    // ------------------------------------------------------------------------

    // EVT_MOUSE_EVENTS from the area and axis windows are passed to these functions
    virtual void ProcessAreaEVT_MOUSE_EVENTS( wxMouseEvent &event );
    virtual void ProcessAxisEVT_MOUSE_EVENTS( wxMouseEvent &event );

    // EVT_CHAR from the area and axis windows are passed to these functions
    virtual void ProcessAreaEVT_CHAR( wxKeyEvent &event ) { OnChar(event); return; }
    virtual void ProcessAreaEVT_KEY_DOWN( wxKeyEvent &event );
    virtual void ProcessAreaEVT_KEY_UP( wxKeyEvent &event );
    virtual void ProcessAxisEVT_CHAR( wxKeyEvent &event ) { OnChar(event); return; }

    void OnChar( wxKeyEvent &event );
    void OnScroll( wxScrollEvent& event );
    void OnPaint( wxPaintEvent &event );
    void OnEraseBackground( wxEraseEvent &event ) { event.Skip(false); }
    void OnIdle( wxIdleEvent &event );
    void OnMouse( wxMouseEvent &event );
    void OnTextEnter( wxCommandEvent &event );

    // ------------------------------------------------------------------------
    // Drawing functions
    // ------------------------------------------------------------------------

    // call BeginBatch to disable redrawing EndBatch to reenable and refresh
    // when batchcount == 0, if !force_refresh then don't refresh when batch == 0
    void BeginBatch() { m_batch_count++; }
    void EndBatch(bool force_refresh = true);
    int  GetBatchCount() const { return m_batch_count; }

    // Redraw parts of the plotctrl using combinations of wxPlot_Redraw_Type
    void Redraw(int type);
    // Get/Set the redraw type variable (this is for internal use, see Redraw())
    int  GetRedrawType() const { return m_redraw_type; }
    void SetRedrawType(int type) { m_redraw_type = type; }

    // Draw a marker in lower right signifying that this has the focus
    virtual void DrawActiveBitmap( wxDC *dc );
    // Draw the wxPlotCtrl (this window)
    virtual void DrawPlotCtrl( wxDC *dc );
    // Draw the area window
    virtual void DrawAreaWindow( wxDC *dc, const wxRect& rect );
    // Draw a wxPlotData derived curve
    virtual void DrawDataCurve( wxDC *dc, wxPlotData *curve, int curve_index, const wxRect &rect );
    // Draw a generic wxPlotCurve curve
    virtual void DrawCurve( wxDC *dc, wxPlotCurve *curve, int curve_index, const wxRect &rect );
    // Draw the key
    virtual void DrawKey( wxDC *dc );
    // Draw the left click drag marker, type is wxPlot_Marker_Type
    virtual void DrawMouseMarker( wxDC *dc, int type, const wxRect &rect );
    // Draw a crosshair cursor at the point (mouse cursor)
    virtual void DrawCrosshairCursor( wxDC *dc, const wxPoint &pos );
    // Draw the cursor marking a single point in a curve wxPlotCtrl::GetCursorPoint
    virtual void DrawCurveCursor( wxDC *dc );
    // Draw the tick marks or grid lines
    virtual void DrawTickMarks( wxDC *dc, const wxRect& rect );
    // Draw markers
    virtual void DrawMarkers( wxDC *dc, const wxRect& rect );

    // redraw this wxPlotData between these two indexes (for (de)select redraw)
    virtual void RedrawDataCurve(int index, int min_index, int max_index);
    // redraw this wxPlotCurve between these two values (for (de)select redraw)
    virtual void RedrawCurve(int index, double min_x, double max_x);

    // Draw the X or Y axis onto the dc
    virtual void DrawXAxis( wxDC *dc, bool refresh );
    virtual void DrawYAxis( wxDC *dc, bool refresh );

    // Draw the plot axes and plotctrl on this wxDC for printing, sort of WYSIWYG
    //   the plot is drawn to fit inside the boundingRect (i.e. the margins)
    void DrawWholePlot( wxDC *dc, const wxRect &boundingRect, int dpi = 72 );

    // ------------------------------------------------------------------------
    // Axis tick calculations
    // ------------------------------------------------------------------------

    // find the optimal number of ticks, step size, and format string
    void AutoCalcTicks()      { AutoCalcXAxisTicks(); AutoCalcYAxisTicks(); }
    void AutoCalcXAxisTicks() { DoAutoCalcTicks(true); }
    void AutoCalcYAxisTicks() { DoAutoCalcTicks(false); }
    virtual void DoAutoCalcTicks(bool x_axis);
    // slightly correct the Zoom and origin to exactly match tick marks
    //  otherwise when the mouse is over '1' you may get 0.99999 or 1.000001
    void CorrectTicks() { CorrectXAxisTicks(); CorrectYAxisTicks(); }
    void CorrectXAxisTicks();
    void CorrectYAxisTicks();
    // Find the correct dc coords for the tick marks and label strings, internal use
    void CalcTickPositions() { CalcXAxisTickPositions(); CalcYAxisTickPositions(); }
    virtual void CalcXAxisTickPositions();
    virtual void CalcYAxisTickPositions();

    // ------------------------------------------------------------------------
    // Utilities
    // ------------------------------------------------------------------------

    // Find a curve at pt, in rect of size +- dxdyPt, starting with active curve
    // return sucess, setting curve_index, data_index if data curve, and if
    // curvePt fills the exact point in the curve.
    bool FindCurve(const wxPoint2DDouble &pt, const wxPoint2DDouble &dxdyPt,
                   int &curve_index, int &data_index, wxPoint2DDouble *curvePt = NULL) const;

    // if n is !finite send wxEVT_PLOT_ERROR if msg is not empty
    bool IsFinite(double n, const wxString &msg = wxEmptyString) const;

    // call this whenever you adjust the size of a data curve
    //    this necessary to know the default zoom to show them
    void CalcBoundingPlotRect();

    // Client (pixels) to/from plot (double) coords
    inline double GetPlotCoordFromClientX( int clientx ) const
        { return (clientx/m_zoom.m_x + m_viewRect.GetLeft()); }
    inline double GetPlotCoordFromClientY( int clienty ) const
        { return ((m_areaClientRect.height - clienty)/m_zoom.m_y + m_viewRect.GetTop());}
    inline wxRect2DDouble GetPlotRectFromClientRect( const wxRect &clientRect ) const
        {
            return wxRect2DDouble( GetPlotCoordFromClientX(clientRect.x),
                                   GetPlotCoordFromClientY(clientRect.GetBottom()),
                                   clientRect.width/m_zoom.m_x,
                                   clientRect.height/m_zoom.m_y );
        }

    inline int GetClientCoordFromPlotX( double plotx ) const
        { double x = m_zoom.m_x*(plotx - m_viewRect.GetLeft()) + 0.5; return x < INT_MAX ? int(x) : INT_MAX; }
    inline int GetClientCoordFromPlotY( double ploty ) const
        { double y = m_areaClientRect.height - m_zoom.m_y*(ploty - m_viewRect.GetTop()) + 0.5; return y < INT_MAX ? int(y) : INT_MAX; }
    inline wxRect GetClientRectFromPlotRect( const wxRect2DDouble &plotRect ) const
        {
            double w = plotRect.m_width*m_zoom.m_x + 0.5;
            double h = plotRect.m_height*m_zoom.m_y + 0.5;
            return wxRect( GetClientCoordFromPlotX(plotRect.m_x),
                           GetClientCoordFromPlotY(plotRect.GetBottom()),
                           w < INT_MAX ? int(w) : INT_MAX,
                           h < INT_MAX ? int(h) : INT_MAX );
        }

    // IDs for the children windows
    enum
    {
        ID_PLOTCTRL_X_AXIS = 100,
        ID_PLOTCTRL_Y_AXIS,
        ID_PLOTCTRL_AREA,
        ID_PLOTCTRL_X_SCROLLBAR,
        ID_PLOTCTRL_Y_SCROLLBAR
    };

    // Get the windows
    wxPlotArea* GetPlotArea()  const { return m_area; }
    wxPlotAxis* GetPlotXAxis() const { return m_xAxis; }
    wxPlotAxis* GetPlotYAxis() const { return m_yAxis; }

    // internal use size adjustment
    void AdjustScrollBars();
    void UpdateWindowSize();
    void DoSize(const wxRect &boundingRect = wxRect(0, 0, 0, 0));

    // who's got the focus, if this then draw the bitmap to show it, internal
    bool CheckFocus();

    // send event returning true if it's allowed
    bool DoSendEvent(wxPlotEvent &event) const;

    // Start the mouse timer with the win_id, stops old if for different id
    enum mouseTimerIDs
    {
        ID_AREA_TIMER = 10,
        ID_XAXIS_TIMER,
        ID_YAXIS_TIMER
    };
    void StartMouseTimer(wxWindowID win_id);
    void StopMouseTimer();
    bool IsTimerRunning();
    void OnTimer( wxTimerEvent &event );

    // A locker for the captured window, set to NULL to release
    void SetCaptureWindow( wxWindow *win );
    wxWindow *GetCaptureWindow() const { return m_winCapture; }

protected:
    void OnSize( wxSizeEvent& event );

    wxArrayPlotCurve  m_curves;         // all the curves
    wxPlotCurve*      m_activeCurve;    // currently active curve
    int               m_active_index;   // index in array of currently active curve

    wxPlotMarker m_cursorMarker;        // marker to draw for cursor
    int          m_cursor_curve;        // index into plot curve array
    int          m_cursor_index;        // if data curve, index in curve

    wxArrayRangeIntSelection    m_dataSelections;  // for wxPlotData
    wxArrayRangeDoubleSelection m_curveSelections; // for wxPlotCurve, wxPlotFunction
    int m_selection_type;

    wxArrayPlotMarker m_plotMarkers;    // extra markers to draw

    bool     m_show_key;                // show the key
    wxString m_keyString;               // the key string

    // title and label
    bool     m_show_title;
    wxString m_title;
    bool     m_show_xlabel, m_show_ylabel;
    wxString m_xLabel, m_yLabel;
    wxRect   m_titleRect, m_xLabelRect, m_yLabelRect;

    // fonts and colours
    wxFont   m_titleFont;
    wxColour m_titleColour;
    wxColour m_borderColour;

    // option variables
    bool m_scroll_on_thumb_release;
    bool m_crosshair_cursor;
    bool m_draw_symbols;
    bool m_draw_lines;
    bool m_draw_spline;
    bool m_draw_grid;
    bool m_fit_on_new_curve;
    bool m_show_xAxis;
    bool m_show_yAxis;

    // rects of the positions of each window - remember for DrawPlotCtrl
    wxRect m_xAxisRect, m_yAxisRect, m_areaRect, m_clientRect;

    // zooms
    wxPoint2DDouble     m_zoom;
    wxArrayRect2DDouble m_historyViews;
    int                 m_history_views_index;
    void AddHistoryView();              // maintains small list of views

    bool   m_fix_aspectratio;
    double m_aspectratio;

    // bounding rect, (GetLeft,GetTop) is lower left in screen coords
    wxRect2DDouble m_viewRect;          // part of the plot currently displayed
    wxRect2DDouble m_curveBoundingRect; // total extent of the plot - CalcBoundingPlotRect
    wxRect2DDouble m_defaultPlotRect;   // default extent of the plot, fallback
    wxRect         m_areaClientRect;    // rect of (wxPoint(0,0), PlotArea.GetClientSize())

    wxArrayInt m_xAxisTicks, m_yAxisTicks; // pixel coordinates of the tic marks
    wxArrayString m_xAxisTickLabels, m_yAxisTickLabels; // the tick labels
    wxString   m_xAxisTickFormat, m_yAxisTickFormat; // format %lg for example
    double     m_xAxisTick_step,  m_yAxisTick_step;  // step size between ticks
    int        m_xAxisTick_count, m_yAxisTick_count; // how many ticks fit?
    bool       m_correct_ticks;                      // tick correction

    // drawers
    wxPlotDrawerArea      *m_areaDrawer;
    wxPlotDrawerXAxis     *m_xAxisDrawer;
    wxPlotDrawerYAxis     *m_yAxisDrawer;
    wxPlotDrawerKey       *m_keyDrawer;
    wxPlotDrawerCurve     *m_curveDrawer;
    wxPlotDrawerDataCurve *m_dataCurveDrawer;
    wxPlotDrawerMarker    *m_markerDrawer;

    // windows
    wxPlotArea  *m_area;
    wxPlotAxis  *m_xAxis, *m_yAxis;
    wxScrollBar *m_xAxisScrollbar, *m_yAxisScrollbar;

    // textctrl for label/title editor, created and deleted as necessary
    wxTextCtrl *m_textCtrl;

    // focusing and bitmap to display focus
    wxBitmap *m_activeBitmap, *m_inactiveBitmap;
    bool m_focused;
    bool m_greedy_focus;

    // remember what needs to be repainted so unnecessary EVT_PAINTS are skipped
    int m_redraw_type;
    int m_batch_count;

    wxSize m_axisFontSize;      // pixel size of the number '5' for axis font
    int    m_y_axis_text_width; // size of "-5e+005" for max y axis width
    int    m_area_border_width; // width of area border pen (default 1)
    int    m_border;            // width of border between labels and axes
    long   m_min_exponential;   // minimum number displayed as an exponential
    double m_pen_print_width;   // width of a 1 pixel pen in mm when printed

    wxTimer  *m_timer;          // don't use, see accessor functions
    wxWindow *m_winCapture;     // don't use, see accessor functions

    wxPlotMouse_Type  m_area_mouse_func;
    wxPlotMarker_Type m_area_mouse_marker;
    int m_area_mouse_cursorid;

    void SetPlotWinMouseCursor(int cursorid);
    int m_mouse_cursorid;

private:
    void Init();
    DECLARE_ABSTRACT_CLASS(wxPlotCtrl)
    DECLARE_EVENT_TABLE()
};

//-----------------------------------------------------------------------------
// wxPlotEvent
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_PLOTCTRL wxPlotEvent : public wxNotifyEvent
{
public:
    wxPlotEvent(wxEventType commandType = wxEVT_NULL, int id = wxID_ANY,
                wxPlotCtrl *window = NULL);

    wxPlotEvent(const wxPlotEvent &event) : wxNotifyEvent(event),
        m_curve(event.m_curve), m_curveIndex(event.m_curveIndex),
        m_curveData_index(event.m_curveData_index), m_x(event.m_x), m_y(event.m_y) {}

    // position of the mouse cursor, double click, single point selection or 1st selected point
    double GetX() const { return m_x; }
    double GetY() const { return m_y; }
    void SetPosition( double x, double y ) { m_x = x; m_y = y; }

    int GetCurveDataIndex() const { return m_curveData_index; }
    void SetCurveDataIndex(int data_index) { m_curveData_index = data_index; }

    // pointer to the curve, NULL if not appropriate for the event type
    wxPlotCurve *GetCurve() const { return m_curve; }
    void SetCurve(wxPlotCurve *curve, int curve_index)
        { m_curve = curve; m_curveIndex = curve_index; }

    // index of the curve in wxPlotCtrl::GetCurve(index)
    int  GetCurveIndex() const { return m_curveIndex; }
    void SetCurveIndex( int curve_index ) { m_curveIndex = curve_index; }

    bool IsDataCurve() const { return wxDynamicCast(m_curve, wxPlotData) != NULL; }

    wxPlotCtrl *GetPlotCtrl() const
        { return wxDynamicCast(GetEventObject(),wxPlotCtrl); }

    int GetMouseFunction() const { return m_commandInt; }
    void SetMouseFunction(int func) { m_commandInt = func; }

    // implementation
    virtual wxEvent *Clone() const { return new wxPlotEvent(*this); }

    wxPlotCurve *m_curve;
    int m_curveIndex;
    int m_curveData_index;
    double m_x, m_y;

private:
    DECLARE_ABSTRACT_CLASS(wxPlotEvent);
};

//-----------------------------------------------------------------------------
// wxPlotEvent
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_PLOTCTRL wxPlotSelectionEvent : public wxPlotEvent
{
public:
    wxPlotSelectionEvent(wxEventType commandType = wxEVT_NULL, int id = wxID_ANY,
                         wxPlotCtrl *window = NULL);

    wxPlotSelectionEvent(const wxPlotSelectionEvent &event) : wxPlotEvent(event),
                                m_dataRange(event.m_dataRange),
                                m_curveRange(event.m_curveRange),
                                m_dataSelection(event.m_dataSelection),
                                m_selecting(event.m_selecting) {}

    // for SELection events the range specifies the new (de)selection range
    // note : for unordered data sets an event is only sent after all selections are made

    // the data selection range is used for wxPlotData curves these are the indexes
    //   for wxPlotCurves and wxPlotFunctions these are the pixel locations
    wxRangeInt GetDataSelectionRange() const { return m_dataRange; }
    void SetDataSelectionRange( const wxRangeInt &range, bool selecting )
        { m_dataRange = range; m_selecting = selecting; }

    // the curve selection range is the double valued start and stop position of the selection
    wxRangeDouble GetCurveSelectionRange() const { return m_curveRange; }
    void SetCurveSelectionRange( const wxRangeDouble &range, bool selecting )
        { m_curveRange = range; m_selecting = selecting; }

    // for a wxPlotData this is filled with the (de)selected ranges.
    //   there will only be more than one for unordered wxPlotDatas
    wxRangeIntSelection GetDataSelections() const { return m_dataSelection; }
    void SetDataSelections(const wxRangeIntSelection &ranges) { m_dataSelection = ranges; }

    // range is selected as opposed to being deselected
    bool IsSelecting() const { return m_selecting; }

    // implementation
    virtual wxEvent *Clone() const { return new wxPlotSelectionEvent(*this); }

    wxRangeInt m_dataRange;
    wxRangeDouble m_curveRange;
    wxRangeIntSelection m_dataSelection;
    bool m_selecting;

private:
    DECLARE_ABSTRACT_CLASS(wxPlotSelectionEvent);
};

// ----------------------------------------------------------------------------
// wxPlotEvent event types
// ----------------------------------------------------------------------------

BEGIN_DECLARE_EVENT_TYPES()

// wxPlotEvent
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_ADD_CURVE,          0) // a curve has been added
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_DELETING_CURVE,     0) // a curve is about to be deleted, event.Skip(false) to prevent
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_DELETED_CURVE,      0) // a curve has been deleted

DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_CURVE_SEL_CHANGING, 0) // curve selection changing, event.Skip(false) to prevent
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_CURVE_SEL_CHANGED,  0) // curve selection has changed

DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_MOUSE_MOTION,       0) // mouse moved
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_CLICKED,            0) // mouse left or right clicked
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_DOUBLECLICKED,      0) // mouse left or right doubleclicked
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_POINT_CLICKED,      0) // clicked on a plot point (+-2pixels)
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_POINT_DOUBLECLICKED,0) // dclicked on a plot point (+-2pixels)

DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_AREA_SEL_CREATING,  0) // mouse left down and drag begin
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_AREA_SEL_CHANGING,  0) // mouse left down and dragging
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_AREA_SEL_CREATED,   0) // mouse left down and drag end

DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_VIEW_CHANGING,      0) // zoom or origin of plotctrl is about to change
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_VIEW_CHANGED,       0) // zoom or origin of plotctrl has changed

DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_CURSOR_CHANGING,    0) // cursor point/curve is about to change, event.Skip(false) to prevent
                                                           // if the cursor is invalidated since
                                                           // the curve is gone you cannot prevent it
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_CURSOR_CHANGED,     0) // cursor point/curve changed

DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_ERROR,              0) // an error has occured, see event.GetString()
                                                           // usually nonfatal NaN overflow errors

DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_BEGIN_TITLE_EDIT,   0) // title is about to be edited, event.Skip(false) to prevent
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_END_TITLE_EDIT,     0) // title has been edited and changed, event.Skip(false) to prevent
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_BEGIN_X_LABEL_EDIT, 0) // x label is about to be edited, event.Skip(false) to prevent
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_END_X_LABEL_EDIT,   0) // x label has been edited and changed, event.Skip(false) to prevent
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_BEGIN_Y_LABEL_EDIT, 0) // y label is about to be edited, event.Skip(false) to prevent
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_END_Y_LABEL_EDIT,   0) // y label has been edited and changed, event.Skip(false) to prevent

DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_MOUSE_FUNC_CHANGING,0)
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_MOUSE_FUNC_CHANGED, 0)

// wxPlotSelectionEvent
//DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_RANGE_SEL_CREATING,0)
//DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_RANGE_SEL_CREATED, 0)
//DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_RANGE_SEL_CHANGING,0)
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_RANGE_SEL_CHANGED,   0)

// unused
/*
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_VALUE_SEL_CREATING, 0)
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_VALUE_SEL_CREATED,  0)
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_VALUE_SEL_CHANGING, 0)
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_VALUE_SEL_CHANGED,  0)
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_PLOTCTRL, wxEVT_PLOT_AREA_SEL_CHANGED,   0)
*/
END_DECLARE_EVENT_TYPES()
// ----------------------------------------------------------------------------
// wxPlotEvent macros
// ----------------------------------------------------------------------------

typedef void (wxEvtHandler::*wxPlotEventFunction)(wxPlotEvent&);
#define wxPlotEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxPlotEventFunction, &func)
#define wx__DECLARE_PLOTEVT(evt, id, fn) wx__DECLARE_EVT1( evt, id, wxPlotEventHandler(fn))

#define EVT_PLOT_ADD_CURVE(id, fn)           wx__DECLARE_PLOTEVT(wxEVT_PLOT_ADD_CURVE,           id, fn)
#define EVT_PLOT_DELETING_CURVE(id, fn)      wx__DECLARE_PLOTEVT(wxEVT_PLOT_DELETING_CURVE,      id, fn)
#define EVT_PLOT_DELETED_CURVE(id, fn)       wx__DECLARE_PLOTEVT(wxEVT_PLOT_DELETED_CURVE,       id, fn)

#define EVT_PLOT_CURVE_SEL_CHANGING(id, fn)  wx__DECLARE_PLOTEVT(wxEVT_PLOT_CURVE_SEL_CHANGING,  id, fn)
#define EVT_PLOT_CURVE_SEL_CHANGED(id, fn)   wx__DECLARE_PLOTEVT(wxEVT_PLOT_CURVE_SEL_CHANGED,   id, fn)

#define EVT_PLOT_MOUSE_MOTION(id, fn)        wx__DECLARE_PLOTEVT(wxEVT_PLOT_MOUSE_MOTION,        id, fn)
#define EVT_PLOT_CLICKED(id, fn)             wx__DECLARE_PLOTEVT(wxEVT_PLOT_CLICKED,             id, fn)
#define EVT_PLOT_DOUBLECLICKED(id, fn)       wx__DECLARE_PLOTEVT(wxEVT_PLOT_DOUBLECLICKED,       id, fn)
#define EVT_PLOT_POINT_CLICKED(id, fn)       wx__DECLARE_PLOTEVT(wxEVT_PLOT_POINT_CLICKED,       id, fn)
#define EVT_PLOT_POINT_DOUBLECLICKED(id, fn) wx__DECLARE_PLOTEVT(wxEVT_PLOT_POINT_DOUBLECLICKED, id, fn)

#define EVT_PLOT_AREA_SEL_CREATING(id, fn)   wx__DECLARE_PLOTEVT(wxEVT_PLOT_AREA_SEL_CREATING,   id, fn)
#define EVT_PLOT_AREA_SEL_CHANGING(id, fn)   wx__DECLARE_PLOTEVT(wxEVT_PLOT_AREA_SEL_CHANGING,   id, fn)
#define EVT_PLOT_AREA_SEL_CREATED(id, fn)    wx__DECLARE_PLOTEVT(wxEVT_PLOT_VIEW_CREATED,        id, fn)

#define EVT_PLOT_VIEW_CHANGING(id, fn)       wx__DECLARE_PLOTEVT(wxEVT_PLOT_VIEW_CHANGING,       id, fn)
#define EVT_PLOT_VIEW_CHANGED(id, fn)        wx__DECLARE_PLOTEVT(wxEVT_PLOT_VIEW_CHANGED,        id, fn)

#define EVT_PLOT_CURSOR_CHANGING(id, fn)     wx__DECLARE_PLOTEVT(wxEVT_PLOT_CURSOR_CHANGING,     id, fn)
#define EVT_PLOT_CURSOR_CHANGED(id, fn)      wx__DECLARE_PLOTEVT(wxEVT_PLOT_CURSOR_CHANGED,      id, fn)

#define EVT_PLOT_ERROR(id, fn)               wx__DECLARE_PLOTEVT(wxEVT_PLOT_ERROR,               id, fn)

#define EVT_PLOT_BEGIN_TITLE_EDIT(id, fn)    wx__DECLARE_PLOTEVT(wxEVT_PLOT_BEGIN_TITLE_EDIT,    id, fn)
#define EVT_PLOT_END_TITLE_EDIT(id, fn)      wx__DECLARE_PLOTEVT(wxEVT_PLOT_END_TITLE_EDIT,      id, fn)
#define EVT_PLOT_BEGIN_X_LABEL_EDIT(id, fn)  wx__DECLARE_PLOTEVT(wxEVT_PLOT_BEGIN_X_LABEL_EDIT,  id, fn)
#define EVT_PLOT_END_X_LABEL_EDIT(id, fn)    wx__DECLARE_PLOTEVT(wxEVT_PLOT_END_X_LABEL_EDIT,    id, fn)
#define EVT_PLOT_BEGIN_Y_LABEL_EDIT(id, fn)  wx__DECLARE_PLOTEVT(wxEVT_PLOT_BEGIN_Y_LABEL_EDIT,  id, fn)
#define EVT_PLOT_END_Y_LABEL_EDIT(id, fn)    wx__DECLARE_PLOTEVT(wxEVT_PLOT_END_Y_LABEL_EDIT,    id, fn)

#define EVT_PLOT_MOUSE_FUNC_CHANGING(id, fn) wx__DECLARE_PLOTEVT(wxEVT_PLOT_MOUSE_FUNC_CHANGING, id, fn)
#define EVT_PLOT_MOUSE_FUNC_CHANGED(id, fn)  wx__DECLARE_PLOTEVT(wxEVT_PLOT_MOUSE_FUNC_CHANGED,  id, fn)

typedef void (wxEvtHandler::*wxPlotSelectionEventFunction)(wxPlotSelectionEvent&);
#define wxPlotSelectionEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxPlotSelectionEventFunction, &func)
#define wx__DECLARE_PLOTSELECTIONEVT(evt, id, fn) wx__DECLARE_EVT1( evt, id, wxPlotSelectionEventHandler(fn))

//#define EVT_PLOT_RANGE_SEL_CREATING(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_PLOT_RANGE_SEL_CREATING, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent( wxPlotSelectionEventFunction, & fn ), (wxObject *) NULL ),
//#define EVT_PLOT_RANGE_SEL_CREATED(id, fn)  DECLARE_EVENT_TABLE_ENTRY(wxEVT_PLOT_RANGE_SEL_CREATED,  id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent( wxPlotSelectionEventFunction, & fn ), (wxObject *) NULL ),
//#define EVT_PLOT_RANGE_SEL_CHANGING(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_PLOT_RANGE_SEL_CHANGING, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent( wxPlotSelectionEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PLOT_RANGE_SEL_CHANGED(id, fn)   wx__DECLARE_PLOTSELECTIONEVT(wxEVT_PLOT_RANGE_SEL_CHANGED,  id, fn)

/*
#define EVT_PLOT_VALUE_SEL_CREATING(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_PLOT_VALUE_SEL_CREATING, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent( wxPlotEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PLOT_VALUE_SEL_CREATED(id, fn)  DECLARE_EVENT_TABLE_ENTRY(wxEVT_PLOT_VALUE_SEL_CREATED,  id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent( wxPlotEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PLOT_VALUE_SEL_CHANGING(id, fn) DECLARE_EVENT_TABLE_ENTRY(wxEVT_PLOT_VALUE_SEL_CHANGING, id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent( wxPlotEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PLOT_VALUE_SEL_CHANGED(id, fn)  DECLARE_EVENT_TABLE_ENTRY(wxEVT_PLOT_VALUE_SEL_CHANGED,  id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent( wxPlotEventFunction, & fn ), (wxObject *) NULL ),
#define EVT_PLOT_AREA_SEL_CHANGED(id, fn)   DECLARE_EVENT_TABLE_ENTRY(wxEVT_PLOT_AREA_SEL_CHANGED,   id, wxID_ANY, (wxObjectEventFunction) (wxEventFunction) wxStaticCastEvent( wxPlotEventFunction, & fn ), (wxObject *) NULL ),
*/

#endif // _WX_PLOTCTRL_H_
