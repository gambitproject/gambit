//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Python extension type for extensive form supports
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
#include "game/efg.h"
#include "game/efstrat.h"
#include "pygambit.h"

/*************************************************************************
 * EFSUPPORT: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void efsupport_dealloc(efsupportobject *);
staticforward PyObject *efsupport_getattr(efsupportobject *, char *);
staticforward int efsupport_print(efsupportobject *, FILE *, int);

PyTypeObject Efsupporttype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "efsupport",                         /* tp_name */
  sizeof(efsupportobject),             /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  efsupport_dealloc,     /* tp_dealloc  ref-count==0  */
  (printfunc)   efsupport_print,       /* tp_print    "print x"     */
  (getattrfunc) efsupport_getattr,     /* tp_getattr  "x.attr"      */
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

static PyObject *
efsupport_getlabel(efsupportobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_support->GetLabel());
}

static PyObject *
efsupport_setlabel(efsupportobject *self, PyObject *args)
{
  char *label;

  if (!PyArg_ParseTuple(args, "s", &label)) {
    return NULL;
  }

  self->m_support->SetLabel(label);
  Py_INCREF(Py_None);
  return Py_None;
}

static struct PyMethodDef efsupport_methods[] = {
  { "GetLabel", (PyCFunction) efsupport_getlabel, 1 },
  { "SetLabel", (PyCFunction) efsupport_setlabel, 1 },
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

efsupportobject *
newefsupportobject(void)
{
  efsupportobject *self;
  self = PyObject_NEW(efsupportobject, &Efsupporttype);
  if (self == NULL) {
    return NULL;
  }
  self->m_support = 0;
  return self;
}

static void                   
efsupport_dealloc(efsupportobject *self) 
{     
  delete self->m_support;
  PyMem_DEL(self);           
}

static PyObject *
efsupport_getattr(efsupportobject *self, char *name)
{
  return Py_FindMethod(efsupport_methods, (PyObject *) self, name);
}

static int
efsupport_print(efsupportobject *self, FILE *fp, int flags)
{
  fprintf(fp, "<{efsupport}>");
  return 0;
}

void
initefgsupport(void)
{
  Efsupporttype.ob_type = &PyType_Type;
}

