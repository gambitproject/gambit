/////////////////////////////////////////////////////////////////////////////
// Name:        wx24defs.h
// Purpose:     Definitions to make a program using wxWidgets >= 2.5 work in 2.4
// Author:      John Labenski
// Modified by: 
// Created:     3/10/04
// RCS-ID:      
// Copyright:   (c) John Labenski
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_WX24DEFS_H__
#define __WX_WX24DEFS_H__

// Include any headers that we need to redefine macros for
#include "wx/defs.h"
#include "wx/object.h"
#include "wx/event.h"
#include "wx/dynarray.h"

//-----------------------------------------------------------------------------
// wxWidgets 2.4 compatibility with >= 2.5

#ifndef WXDLLIMPEXP_ADV
    #define WXDLLIMPEXP_ADV WXDLLEXPORT
#endif     

#ifndef DECLARE_NO_ASSIGN_CLASS
    #define DECLARE_NO_ASSIGN_CLASS(classname) \
        private:                               \
        classname& operator=(const classname&);
#endif        

#ifndef WX_DECLARE_OBJARRAY_WITH_DECL
    #define WX_DECLARE_OBJARRAY_WITH_DECL(T, name, expmode) WX_DECLARE_USER_EXPORTED_OBJARRAY(T, name, WXDLLEXPORT)
#endif

#ifndef WX_DEFINE_ARRAY_WITH_DECL_PTR
    #define WX_DEFINE_ARRAY_WITH_DECL_PTR(T, name, decl) WX_DEFINE_ARRAY(T, name)
#endif

#ifndef DECLARE_EXPORTED_EVENT_TYPE
    #define DECLARE_EXPORTED_EVENT_TYPE(expdecl, name, value) DECLARE_LOCAL_EVENT_TYPE(name, value)
#endif

#ifndef wxStaticCastEvent
    #define wxStaticCastEvent(type, val) (type) val
#endif      

#ifndef wxFULL_REPAINT_ON_RESIZE
    #define wxFULL_REPAINT_ON_RESIZE 0
#endif 

#endif //__WX_WX24DEFS_H__
