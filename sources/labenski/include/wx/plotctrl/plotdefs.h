/////////////////////////////////////////////////////////////////////////////
// Name:        plotdefs.h
// Purpose:     Definitions for wxPlotLib
// Author:      John Labenski
// Modified by:
// Created:     1/08/2005
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_PLOTDEF_H__
#define __WX_PLOTDEF_H__

#include "wx/defs.h"

// ----------------------------------------------------------------------------
// DLLIMPEXP macros
// ----------------------------------------------------------------------------

// These are our DLL macros (see the contrib libs like wxPlot)
/*
#ifdef WXMAKINGDLL_PLOTCTRL
    #define WXDLLIMPEXP_PLOTCTRL WXEXPORT
    #define WXDLLIMPEXP_DATA_PLOTCTRL(type) WXEXPORT type
#elif defined(WXUSINGDLL)
    #define WXDLLIMPEXP_PLOTCTRL WXIMPORT
    #define WXDLLIMPEXP_DATA_PLOTCTRL(type) WXIMPORT type
#else // not making nor using DLL
*/
    #define WXDLLIMPEXP_PLOTCTRL
    #define WXDLLIMPEXP_DATA_PLOTCTRL(type) type
/*
#endif
*/

// ----------------------------------------------------------------------------
// Convenience macros
// ----------------------------------------------------------------------------

// Check if value is >= min_val and <= max_val
#define wxPCHECK_MINMAX_RET(val, min_val, max_val, msg) \
    wxCHECK_RET((int(val)>=int(min_val))&&(int(val)<=int(max_val)), msg)

#define wxPCHECK_MINMAX_MSG(val, min_val, max_val, ret, msg) \
    wxCHECK_MSG((int(val)>=int(min_val))&&(int(val)<=int(max_val)), ret, msg)

#endif  // __WX_PLOTDEF_H__
