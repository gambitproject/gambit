///////////////////////////////////////////////////////////////////////////////
// Name:        sheetatr.h
// Purpose:     wxSheetAttribute and related classes
// Author:      Michael Bedward (based on code by Julian Smart, Robin Dunn)
// Modified by: John Labenski
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski, Michael Bedward
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

#ifndef __WX_SHEETATR_H__
#define __WX_SHEETATR_H__

#include "wx/sheet/sheetdef.h"
#include "wx/font.h"
#include "wx/colour.h"

// ----------------------------------------------------------------------------
// wxSheetCellAttr : contains all the attributes for a wxSheet cell
//
// Note: When created all the HasXXX return false, use Copy, Merge, or SetXXX
//
// The default attr for the different wxSheet areas must be complete so that 
// when a new attr is assigned you need only set the values you want to be 
// different than the default's. Unset values are retrieved from the default
// attr which gets them from it's def attr and so on, they're chained together.
// ----------------------------------------------------------------------------

class WXDLLIMPEXP_SHEET wxSheetCellAttr : public wxObject
{
public:
    // if create then create with ref data
    wxSheetCellAttr( bool create = false );
    // make a refed copy of the other attribute
    wxSheetCellAttr( const wxSheetCellAttr& attr ) : wxObject() { Ref(attr); }

    // Recreate the ref data, unrefing the old
    bool Create();
    void Destroy() { UnRef(); }
    inline bool Ok() const { return m_refData != NULL; }

    // Makes a full new unrefed copy of the other, this doesn't have to be created
    bool Copy(const wxSheetCellAttr& other);
    // Copies the values from the other, but only if the other has them, this must be created
    bool UpdateWith(const wxSheetCellAttr& other);
    // Merges this with the other, copy values of other only this doesn't have them
    bool MergeWith(const wxSheetCellAttr &mergefrom);
    
    // setters
    void SetForegroundColour(const wxColour& foreColour);
    void SetBackgroundColour(const wxColour& backColour);
    void SetFont(const wxFont& font);
    // wxSheetAttrAlign_Type
    void SetAlignment(int align);
    void SetAlignment(int horzAlign, int vertAlign);
    // wxSheetAttrOrientation_Type
    void SetOrientation(int orientation);
    void SetLevel(wxSheetAttrLevel_Type level);
    void SetOverflow(bool allow);
    void SetOverflowMarker(bool draw_marker);
    void SetShowEditor(bool show_editor);
    void SetReadOnly(bool isReadOnly);
    void SetRenderer(const wxSheetCellRenderer& renderer);
    void SetEditor(const wxSheetCellEditor& editor);
    void SetKind(wxSheetAttr_Type kind);

    // validation
    bool HasForegoundColour() const;
    bool HasBackgroundColour() const;
    bool HasFont() const;
    bool HasAlignment() const;
    bool HasOrientation() const;
    bool HasLevel() const;
    bool HasOverflowMode() const;
    bool HasOverflowMarkerMode() const;
    bool HasShowEditorMode() const;
    bool HasReadWriteMode() const;
    bool HasRenderer() const;
    bool HasEditor() const;
    bool HasDefaultAttr() const;
    // bool HasKind() const - always has kind, default is wxSHEET_AttrCell

    // does this attr define all the HasXXX properties, except DefaultAttr
    //   if this is true, it's a suitable default attr for an area
    bool IsComplete() const;  

    // accessors
    const wxColour& GetForegroundColour() const;
    const wxColour& GetBackgroundColour() const;
    const wxFont& GetFont() const;
    int GetAlignment() const;
    wxOrientation GetOrientation() const;
    wxSheetAttrLevel_Type GetLevel() const;
    bool GetOverflow() const;
    bool GetOverflowMarker() const;
    bool GetShowEditor() const;
    bool GetReadOnly() const;
    wxSheetCellRenderer GetRenderer(wxSheet* grid, const wxSheetCoords& coords) const;
    wxSheetCellEditor GetEditor(wxSheet* grid, const wxSheetCoords& coords) const;
    wxSheetAttr_Type GetKind() const;

    // any unset values of this attr are retrieved from the default attr
    // if you try to set the def attr to this, it's ignored
    // don't bother to link multiple attributes together in a loop, obviously.
    const wxSheetCellAttr& GetDefaultAttr() const;
    void SetDefaultAttr(const wxSheetCellAttr& defaultAttr);

    // operators
    bool operator == (const wxSheetCellAttr& obj) const { return m_refData == obj.m_refData; }
    bool operator != (const wxSheetCellAttr& obj) const { return m_refData != obj.m_refData; }
    wxSheetCellAttr& operator = (const wxSheetCellAttr& obj)
    {   
        if ( (*this) != obj ) Ref(obj);
        return *this;   
    }
    
    wxSheetCellAttr Clone() const     { wxSheetCellAttr obj; obj.Copy(*this); return obj; }
    wxSheetCellAttr* NewClone() const { return new wxSheetCellAttr(Clone()); }

    // implementation
    void SetType(int type, int mask);
    int  GetType(int mask = ~0) const;
    bool HasType(int type) const { return GetType(type) != 0; }

protected:
    // override wxObject's create a new m_refData
    virtual wxObjectRefData *CreateRefData() const;
    // override wxObject's create a new m_refData initialized with the given one
    virtual wxObjectRefData *CloneRefData(const wxObjectRefData *data) const;
    
    DECLARE_DYNAMIC_CLASS(wxSheetCellAttr)
};

// ----------------------------------------------------------------------------
// wxSheetCellAttrRefData : data for the wxSheetCellAttr
// 
// only use this as a LAST resort for overriding the behavior
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_SHEET wxSheetCellAttrRefData : public wxObjectRefData
{
public:    
    wxSheetCellAttrRefData();
    wxSheetCellAttrRefData( const wxSheetCellAttrRefData& data );
    virtual ~wxSheetCellAttrRefData();

    wxColour m_foreColour,
             m_backColour;
    wxFont   m_font;
    // stores wxSheetAttr_Type, align, orient, overflow, show edit, read, level
    wxUint32 m_attrTypes; 
    
    // these are pointers since we can't define the edit/ren/attr all at once
    wxSheetCellRenderer *m_renderer;
    wxSheetCellEditor   *m_editor;
    wxSheetCellAttr     *m_defaultAttr;
};

// ----------------------------------------------------------------------------
// wxNullSheetCellAttr - an uncreated wxSheetCellAttr for use when there's no attribute
// ----------------------------------------------------------------------------

WXDLLIMPEXP_DATA_SHEET(extern const wxSheetCellAttr) wxNullSheetCellAttr;

// ----------------------------------------------------------------------------
// wxArraySheetCellAttr - wxArray of wxSheetCellAttr
// wxPairArrayIntSheetCellAttr - int key, wxSheetCellAttr value pairs 
// wxPairArraySheetCoordsCellAttr - wxSheetCoords key, wxSheetCellAttr value pairs 
// ----------------------------------------------------------------------------

// Create a 1-D array of wxArraySheetCellAttr for row/col labels
WX_DECLARE_OBJARRAY_WITH_DECL(wxSheetCellAttr, wxArraySheetCellAttr,
                              class WXDLLIMPEXP_SHEET);
// Create the paired array of attrs for row/col labels
DECLARE_PAIRED_INT_DATA_ARRAYS( wxSheetCellAttr, wxArraySheetCellAttr, 
                                wxPairArrayIntSheetCellAttr, class WXDLLIMPEXP_SHEET)

// Create wxPairArraySheetCoordsCellAttr for storing coords keys and attr values.
DECLARE_PAIREDSHEETCOORDS_DATA_ARRAYS(wxSheetCellAttr, wxArraySheetCellAttr, 
                                      wxPairArraySheetCoordsCellAttr, class WXDLLIMPEXP_SHEET)

// ----------------------------------------------------------------------------
// wxSheetCellAttrProvider : for wxSheetTable to retrieve/store cell attr
//
// implementation note: we separate it from wxSheetTable because we wish to
// avoid deriving a new table class if possible, and sometimes it will be
// enough to just derive another wxSheetCellAttrProvider instead
//
// the default implementation is reasonably efficient for the generic case,
// but you might still wish to implement your own for some specific situations
// if you have performance problems with the stock one or the attribute 
// properties lend themselves to be calculated on the fly
// ----------------------------------------------------------------------------
class WXDLLIMPEXP_SHEET wxSheetCellAttrProvider : public wxClientDataContainer
{
public:
    wxSheetCellAttrProvider() {}
    virtual ~wxSheetCellAttrProvider() {}

    // Get the attribute for the coords of type wxSheetAttr_Type
    // you must implement these cell coords for grid, row/col/corner labels
    // see wxSheet::IsGridCell/IsRowLabelCell/IsColLabelCell/IsCornerLabelCell
    // and all types except wxSHEET_AttrDefault, which sheet should have handled
    // If type is wxSHEET_AttrCell/Row/Col return wxNullSheetCellAttr if one is
    // not set. Type wxSHEET_AttrAny is used to merge row/col/cell attr
    // based on their level.
    // return wxNullSheetCellAttr if none set for coords and type
    virtual wxSheetCellAttr GetAttr( const wxSheetCoords& coords,
                                     wxSheetAttr_Type type );
    
    // Set the attribute for the coords, see GetAttr for coords and type
    //  if the !attr.Ok() the attr is removed w/o error even if it didn't exist
    // use wxNullSheetCellAttr to remove attr for coords and type
    virtual void SetAttr( const wxSheetCoords& coords, 
                          const wxSheetCellAttr& attr,
                          wxSheetAttr_Type type );

    // Update internal data whenever # rows/cols change (must be called)
    //  this shifts rows/cols and deletes them as appropriate
    //  you can specificly update only some of the attributes by ORing 
    //  enum wxSheetUpdate_Type for the attributes.
    virtual void UpdateRows( size_t pos, int numRows, int update = wxSHEET_UpdateAttributes );
    virtual void UpdateCols( size_t pos, int numCols, int update = wxSHEET_UpdateAttributes );

protected:
    wxPairArraySheetCoordsCellAttr m_cellAttrs;

    wxPairArrayIntSheetCellAttr m_rowAttrs,
                                m_colAttrs,
                                m_rowLabelAttrs,
                                m_colLabelAttrs;

    DECLARE_NO_COPY_CLASS(wxSheetCellAttrProvider)
};

#endif  // __WX_SHEETATR_H__
