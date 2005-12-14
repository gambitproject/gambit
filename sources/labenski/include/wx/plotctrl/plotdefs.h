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

//#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
//    #pragma interface "plotdefs.h"
//#endif

#include "wx/defs.h"
//#include "wxthings/wx24defs.h"   // wx2.4 backwards compatibility

// ----------------------------------------------------------------------------
// DLLIMPEXP macros
// ----------------------------------------------------------------------------

// These are our DLL macros (see the contrib libs like wxPlot)
#ifdef WXMAKINGDLL_PLOTLIB
    #define WXDLLIMPEXP_PLOTLIB WXEXPORT
    #define WXDLLIMPEXP_DATA_PLOTLIB(type) WXEXPORT type
#elif defined(WXUSINGDLL)
    #define WXDLLIMPEXP_PLOTLIB WXIMPORT
    #define WXDLLIMPEXP_DATA_PLOTLIB(type) WXIMPORT type
#else // not making nor using DLL
    #define WXDLLIMPEXP_PLOTLIB
    #define WXDLLIMPEXP_DATA_PLOTLIB(type) type
#endif

#endif  // __WX_PLOTDEF_H__
