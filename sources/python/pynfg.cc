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
staticforward int nfg_compare(nfgobject *, nfgobject *);
staticforward PyObject *nfg_str(nfgobject *);

PyTypeObject Nfgtype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "nfg",                           /* tp_name */
  sizeof(nfgobject),               /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  nfg_dealloc,       /* tp_dealloc  ref-count==0  */
  (printfunc)   0,         /* tp_print    "print x"     */
  (getattrfunc) nfg_getattr,       /* tp_getattr  "x.attr"      */
  (setattrfunc) 0,                 /* tp_setattr  "x.attr=v"    */
  (cmpfunc)     nfg_compare,       /* tp_compare  "x > y"       */
  (reprfunc)    0,                 /* tp_repr     `x`, print x  */

  /* type categories */
  0,                               /* tp_as_number   +,-,*,/,%,&,>>,pow...*/
  0,                               /* tp_as_sequence +,[i],[i:j],len, ...*/
  0,                               /* tp_as_mapping  [key], len, ...*/

  /* more methods */
  (hashfunc)     0,                /* tp_hash    "dict[x]" */
  (ternaryfunc)  0,                /* tp_call    "x()"     */
  (reprfunc)     nfg_str,               /* tp_str     "str(x)"  */
};  /* plus others: see Include/object.h */


/*****************************************************************************
 * INSTANCE METHODS
 *****************************************************************************/

static PyObject *
nfg_getcomment(nfgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_nfg->GetComment());
}

static PyObject *
nfg_getlabel(nfgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_nfg->GetLabel());
}

static PyObject *
nfg_getoutcome(nfgobject *self, PyObject *args)
{
  int index;

  if (!PyArg_ParseTuple(args, "i", &index)) {
    return NULL;
  }

  if (index < 1 || index > self->m_nfg->NumOutcomes()) {
    return NULL;
  }

  nfoutcomeobject *outcome = newnfoutcomeobject();
  *outcome->m_nfoutcome = self->m_nfg->GetOutcome(index);
  return (PyObject *) outcome;
}

static PyObject *
nfg_getplayer(nfgobject *self, PyObject *args)
{
  int index;

  if (!PyArg_ParseTuple(args, "i", &index)) {
    return NULL;
  }

  if (index < 1 || index > self->m_nfg->NumPlayers()) {
    return NULL;
  }

  nfplayerobject *player = newnfplayerobject();
  *player->m_nfplayer = self->m_nfg->GetPlayer(index);
  return (PyObject *) player;
}

static PyObject *
nfg_isconstsum(nfgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("b", self->m_nfg->IsConstSum());
}

static PyObject *
nfg_newcontingency(nfgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  nfgcontingencyobject *contingency = newnfgcontingencyobject();
  contingency->m_contingency = new gbtNfgContingency(*self->m_nfg);
  return (PyObject *) contingency;
}

static PyObject *
nfg_newoutcome(nfgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  nfoutcomeobject *outcome = newnfoutcomeobject();
  *outcome->m_nfoutcome = self->m_nfg->NewOutcome();
  return (PyObject *) outcome;
}

static PyObject *
nfg_newsupport(nfgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  nfsupportobject *support = newnfsupportobject();
  support->m_support = new gbtNfgSupport(*self->m_nfg);
  return (PyObject *) support;
}

static PyObject *
nfg_numoutcomes(nfgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("i", self->m_nfg->NumOutcomes());
}

static PyObject *
nfg_numplayers(nfgobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("i", self->m_nfg->NumPlayers());
}

static PyObject *
nfg_setlabel(nfgobject *self, PyObject *args)
{
  char *label;

  if (!PyArg_ParseTuple(args, "s", &label)) {
    return NULL;
  }

  self->m_nfg->SetLabel(label);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
nfg_writenfg(nfgobject *self, PyObject *args)
{
  char *filename;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  try {
    gbtFileOutput file(filename);
    self->m_nfg->WriteNfg(file);
    Py_INCREF(Py_None);
    return Py_None;
  }
  catch (const gbtFileInput::OpenFailed &) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  catch (...) {
    Py_INCREF(Py_None);
    return Py_None;
  }
}

static struct PyMethodDef nfg_methods[] = {
  { "GetComment", (PyCFunction) nfg_getcomment, 1 },
  { "GetLabel", (PyCFunction) nfg_getlabel, 1 },
  { "GetOutcome", (PyCFunction) nfg_getoutcome, 1 },
  { "GetPlayer", (PyCFunction) nfg_getplayer, 1 },
  { "IsConstSum", (PyCFunction) nfg_isconstsum, 1 },
  { "NewContingency", (PyCFunction) nfg_newcontingency, 1 },
  { "NewOutcome", (PyCFunction) nfg_newoutcome, 1 },
  { "NewSupport", (PyCFunction) nfg_newsupport, 1 },
  { "NumOutcomes", (PyCFunction) nfg_numoutcomes, 1 },
  { "NumPlayers", (PyCFunction) nfg_numplayers, 1 },
  { "SetLabel", (PyCFunction) nfg_setlabel, 1 }, 
  { "WriteNfg", (PyCFunction) nfg_writenfg, 1 },
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

nfgobject *
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
  delete self->m_nfg;
  PyMem_DEL(self);           
}

static PyObject *
nfg_getattr(nfgobject *self, char *name)
{
  return Py_FindMethod(nfg_methods, (PyObject *) self, name);
}

static int
nfg_compare(nfgobject *obj1, nfgobject *obj2)
{
  // Implementation: If games are the game underlying object, return
  // equal; otherwise, order by their Python pointer addresses.
  if (*obj1->m_nfg == *obj2->m_nfg) {
    return 0;
  }
  else if (obj1->m_nfg < obj2->m_nfg) {
    return -1;
  }
  else {
    return 1;
  }
}

static PyObject *
nfg_str(nfgobject *self)
{
  return PyString_FromFormat("<{nfg} \"%s\">",
			     (char *) self->m_nfg->GetLabel());
}

/************************************************************************
 * MODULE METHODS
 ************************************************************************/

PyObject *
gbt_new_nfg(PyObject */*self*/, PyObject *args)
{
  PyObject *list;

  if (!PyArg_ParseTuple(args, "O", &list)) {
    return NULL;
  }

  if (!PyList_Check(list)) {
    return NULL;
  }

  gbtArray<int> dim(PyList_Size(list));
  for (int pl = 1; pl <= dim.Length(); pl++) {
    if (!PyInt_Check(PyList_GetItem(list, pl - 1))) {
      return NULL;
    }
    else {
      dim[pl] = PyInt_AsLong(PyList_GetItem(list, pl - 1));
    }
  }

  nfgobject *nfg = newnfgobject();
  nfg->m_nfg = new gbtNfgGame(dim);
  return (PyObject *) nfg;
}

PyObject *
gbt_read_nfg(PyObject */*self*/, PyObject *args)
{
  char *filename;

  if (!PyArg_ParseTuple(args, "s", &filename)) {
    return NULL;
  }

  nfgobject *nfg = newnfgobject();
  try {
    gbtFileInput file(filename);
    nfg->m_nfg = new gbtNfgGame(ReadNfg(file));
    return (PyObject *) nfg;
  }
  catch (const gbtFileInput::OpenFailed &) {
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
