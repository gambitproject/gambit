/////////////////////////////////////////////////////////////////////////////
// Name:        thingdef.h
// Purpose:     Definitions for wxThings
// Author:      John Labenski
// Modified by:
// Created:     1/08/1999
// RCS-ID:      $Id$
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
/////////////////////////////////////////////////////////////////////////////

#ifndef __WX_THINGDEF_H__
#define __WX_THINGDEF_H__

#include "wx/defs.h"

// ----------------------------------------------------------------------------
// DLLIMPEXP macros
// ----------------------------------------------------------------------------

// These are our DLL macros (see the contrib libs like wxPlot)
/*
#ifdef WXMAKINGDLL_THINGS
    #define WXDLLIMPEXP_THINGS WXEXPORT
    #define WXDLLIMPEXP_DATA_THINGS(type) WXEXPORT type
#elif defined(WXUSINGDLL)
    #define WXDLLIMPEXP_THINGS WXIMPORT
    #define WXDLLIMPEXP_DATA_THINGS(type) WXIMPORT type
#else // not making nor using DLL
*/
    #define WXDLLIMPEXP_THINGS
    #define WXDLLIMPEXP_DATA_THINGS(type) type
/*
#endif
*/

#endif  // __WX_THINGDEF_H__
