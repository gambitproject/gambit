//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Python extension type for mixed strategy profiles
//
// This file is part of Gambit
// Copyright (c) 2003, The Gambit Project
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

#include <Python.h>
#include "nash/mixedsol.h"
#include "pygambit.h"

/*************************************************************************
 * MIXED PROFILE: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void mixed_dealloc(mixedobject *);
staticforward PyObject *mixed_getattr(mixedobject *, char *);
staticforward int mixed_print(mixedobject *, FILE *, int);

PyTypeObject Mixedtype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "mixed",                         /* tp_name */
  sizeof(mixedobject),             /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  mixed_dealloc,     /* tp_dealloc  ref-count==0  */
  (printfunc)   mixed_print,       /* tp_print    "print x"     */
  (getattrfunc) mixed_getattr,     /* tp_getattr  "x.attr"      */
  (setattrfunc) 0,                 /* tp_setattr  "x.attr=v"    */
  (cmpfunc)     0,                 /* tp_compare  "x > y"       */
  (reprfunc)    0,                 /* tp_repr     `x`, print x  */

  /* type categories */
  0,                               /* tp_as_number   +,-,*,/,%,&,>>,pow...*/
  0,                               /* tp_as_sequence +,[i],[i:j],len, ...*/
  0,                               /* tp_as_mapping  [key], len, ...*/

  /* more methods */
  (hashfunc)     0,                /* tp_hash    "dict[x]" */
  (ternaryfunc)  0,                /* tp_call    "x()"     */
  (reprfunc)     0,                /* tp_str     "str(x)"  */
};  /* plus others: see Include/object.h */


/*****************************************************************************
 * INSTANCE METHODS
 *****************************************************************************/

static struct PyMethodDef mixed_methods[] = {
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

mixedobject *
newmixedobject(void)
{
  mixedobject *self;
  self = PyObject_NEW(mixedobject, &Mixedtype);
  if (self == NULL) {
    return NULL;
  }
  self->m_profile = 0;
  return self;
}

static void                   
mixed_dealloc(mixedobject *self) 
{     
  delete self->m_profile;
  PyMem_DEL(self);           
}

static PyObject *
mixed_getattr(mixedobject *self, char *name)
{
  return Py_FindMethod(mixed_methods, (PyObject *) self, name);
}

static int
mixed_print(mixedobject *self, FILE *fp, int flags)
{
  fprintf(fp, "<{mixed} ");

  for (int pl = 1; pl <= self->m_profile->GetGame().NumPlayers(); pl++)  {
    fprintf(fp, "{ ");
    gbtNfgPlayer player = self->m_profile->GetGame().GetPlayer(pl);
    for (int st = 1; st <= player.NumStrategies(); st++) {
      gbtNfgStrategy strategy = player.GetStrategy(st);
      fprintf(fp, "%s ", (char *) ToText((*self->m_profile)(strategy)));
    }
    fprintf(fp, "}");
  }
  fprintf(fp, ">");
  return 0;
}

void
initmixed(void)
{
  Mixedtype.ob_type = &PyType_Type;
}

