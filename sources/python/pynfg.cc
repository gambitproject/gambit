//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Python extension type for normal form games
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
#include "game/nfg.h"
#include "pygambit.h"

/*************************************************************************
 * NFG: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void nfg_dealloc(nfgobject *);
staticforward PyObject *nfg_getattr(nfgobject *, char *);
staticforward int nfg_print(nfgobject *, FILE *, int);

PyTypeObject Nfgtype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "nfg",                           /* tp_name */
  sizeof(nfgobject),               /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  nfg_dealloc,       /* tp_dealloc  ref-count==0  */
  (printfunc)   nfg_print,         /* tp_print    "print x"     */
  (getattrfunc) nfg_getattr,       /* tp_getattr  "x.attr"      */
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
nfg_numoutcomes(nfgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("i", self->m_nfg.NumOutcomes());
}

static PyObject *
nfg_numplayers(nfgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("i", self->m_nfg.NumPlayers());
}

static PyObject *
nfg_writenfg(nfgobject *self, PyObject *args)
{
  char *filename;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  try {
    gFileOutput file(filename);
    self->m_nfg.WriteNfgFile(file, 6);
    Py_INCREF(Py_None);
    return Py_None;
  }
  catch (const gFileInput::OpenFailed &) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  catch (...) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  Py_INCREF(Py_None);
  return Py_None;
}

static struct PyMethodDef nfg_methods[] = {
  { "NumOutcomes", (PyCFunction) nfg_numoutcomes, 1 },
  { "NumPlayers", (PyCFunction) nfg_numplayers, 1 },
  { "WriteNfg", (PyCFunction) nfg_writenfg, 1 },
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

static nfgobject *
newnfgobject(void)
{
  nfgobject *self;
  self = PyObject_NEW(nfgobject, &Nfgtype);
  if (self == NULL) {
    return NULL;
  }
  self->m_nfg = 0;
  return self;
}

static void                   
nfg_dealloc(nfgobject *self) 
{                            
  PyMem_DEL(self);           
}

static PyObject *
nfg_getattr(nfgobject *self, char *name)
{
  return Py_FindMethod(nfg_methods, (PyObject *) self, name);
}

static int
nfg_print(nfgobject *self, FILE *fp, int flags)
{
  fprintf(fp, "<{nfg} \"%s\">", (char *) self->m_nfg.GetTitle());
  return 0;
}

/************************************************************************
 * MODULE METHODS
 ************************************************************************/

PyObject *
gbt_read_nfg(PyObject *self, PyObject *args)
{
  char *filename;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  nfgobject *nfg = newnfgobject();
  try {
    gFileInput file(filename);
    nfg->m_nfg = ReadNfgFile(file);
    return (PyObject *) nfg;
  }
  catch (const gFileInput::OpenFailed &) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  catch (...) {
    Py_INCREF(Py_None);
    return Py_None;
  }
}

void
initnfg(void)
{
  Nfgtype.ob_type = &PyType_Type;
}
