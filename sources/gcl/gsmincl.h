//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Miscellaneous GCL functions
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef GSMINCL_H
#define GSMINCL_H

#include "base/base.h"

class gOutput;

#define rcFAIL     0
#define rcSUCCESS -1
#define rcQUIT    -2


#define  porNULLSPEC  true

typedef enum {
  porBOOLEAN        = 0x00000001,
  porPRECISION      = 0x00000002,
  porINTEGER        = 0x00000004,
  porNUMBER         = 0x00000008,
  porTEXT           = 0x00000010,
  porNFOUTCOME      = 0x00000020,
  porNFG            = 0x00000040,
  porEFG            = 0x00000100,
  porMIXED          = 0x00000400,
  porBEHAV          = 0x00001000,
  porEFOUTCOME      = 0x00004000,
  porINFOSET        = 0x00010000,
  porNFSUPPORT      = 0x00040000,
  porEFSUPPORT      = 0x00100000,
  porEFBASIS        = 0x00200000,
  porNFPLAYER       = 0x00400000,
  porEFPLAYER       = 0x00800000,
  porNODE           = 0x01000000,
  porACTION         = 0x02000000,
  porREFERENCE      = 0x04000000,
  porOUTPUT         = 0x08000000,
  porINPUT          = 0x10000000,
  porSTRATEGY       = 0x20000000,
  porNULL           = 0x40000000,
  porUNDEFINED      = 0x80000000
};

const unsigned int porANYTYPE = 0xFFFFFFFF;

#define  NLIST 32767


class PortionSpec
{
public:
  unsigned long Type;
  unsigned int ListDepth;
  bool Null;

  PortionSpec(const PortionSpec& spec)
    : Type(spec.Type), ListDepth(spec.ListDepth), Null(spec.Null)
    {}
  PortionSpec(unsigned long type = 0,
	      unsigned int listdepth = 0,
	      bool null = false )
    : Type(type), ListDepth(listdepth), Null(null)
    {}
  ~PortionSpec()
  {}

  bool operator==(const PortionSpec& spec) const
  {
    return (Type==spec.Type &&
	    ListDepth==spec.ListDepth &&
	    Null==spec.Null);
  }
};


bool PortionSpecMatch( const PortionSpec& t1, const PortionSpec& t2 );


gText PortionSpecToText( const PortionSpec& type );
PortionSpec TextToPortionSpec( const gText& text );
void PrintPortionSpec( gOutput& s, PortionSpec type );
gOutput& operator << ( gOutput& s, PortionSpec spec );

#endif // GSMINCL_H

