//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Python extension type for outcome objects
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
#include "pygambit.h"
#include "game/outcome.h"

// As with the underlying library, both extensive and normal form outcomes
// appear in the same file, in the hopes of an eventual unification.

/*************************************************************************
 * EFOUTCOME: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void efoutcome_dealloc(efoutcomeobject *);
staticforward PyObject *efoutcome_getattr(efoutcomeobject *, char *);
staticforward int efoutcome_compare(efoutcomeobject *, efoutcomeobject *); 
staticforward PyObject *efoutcome_str(efoutcomeobject *);

PyTypeObject Efoutcometype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "efoutcome",                           /* tp_name */
  sizeof(efoutcomeobject),               /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  efoutcome_dealloc,       /* tp_dealloc  ref-count==0  */
  (printfunc)   0,         /* tp_print    "print x"     */
  (getattrfunc) efoutcome_getattr,       /* tp_getattr  "x.attr"      */
  (setattrfunc) 0,                 /* tp_setattr  "x.attr=v"    */
  (cmpfunc)     efoutcome_compare,   /* tp_compare  "x > y"       */
  (reprfunc)    0,                 /* tp_repr     `x`, print x  */

  /* type categories */
  0,                               /* tp_as_number   +,-,*,/,%,&,>>,pow...*/
  0,                               /* tp_as_sequence +,[i],[i:j],len, ...*/
  0,                               /* tp_as_mapping  [key], len, ...*/

  /* more methods */
  (hashfunc)     0,                /* tp_hash    "dict[x]" */
  (ternaryfunc)  0,                /* tp_call    "x()"     */
  (reprfunc)     efoutcome_str,                /* tp_str     "str(x)"  */
};  /* plus others: see Include/object.h */


/*****************************************************************************
 * INSTANCE METHODS
 *****************************************************************************/

static PyObject *
efoutcome_deleteoutcome(efoutcomeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  self->m_efoutcome->DeleteOutcome();
  Py_INCREF(Py_None);
  return Py_None;
}  

static PyObject *
efoutcome_getgame(efoutcomeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  efgobject *game = newefgobject();
  game->m_efg = new gbtGame(self->m_efoutcome->GetGame());
  return (PyObject *) game;
}

static PyObject *
efoutcome_getlabel(efoutcomeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_efoutcome->GetLabel());
}

static PyObject *
efoutcome_getpayoff(efoutcomeobject *self, PyObject *args)
{
  PyObject *player;

  if (!PyArg_ParseTuple(args, "O", &player)) {
    return NULL;
  }

  if (!is_efplayerobject(player)) {
    return NULL;
  }

  return Py_BuildValue("d",
		       (double) self->m_efoutcome->GetPayoff(*((efplayerobject *) player)->m_efplayer));
}

static PyObject *
efoutcome_setlabel(efoutcomeobject *self, PyObject *args)
{
  char *label;

  if (!PyArg_ParseTuple(args, "s", &label)) {
    return NULL;
  }

  self->m_efoutcome->SetLabel(label);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
efoutcome_setpayoff(efoutcomeobject *self, PyObject *args)
{
  PyObject *player;
  double payoff;

  if (!PyArg_ParseTuple(args, "Od", &player, &payoff)) {
    return NULL;
  }

  if (!is_efplayerobject(player)) {
    return NULL;
  }

  self->m_efoutcome->SetPayoff(*((efplayerobject *) player)->m_efplayer,
			       payoff);
  Py_INCREF(Py_None);
  return Py_None;
}

static struct PyMethodDef efoutcome_methods[] = {
  { "DeleteOutcome", (PyCFunction) efoutcome_deleteoutcome, 1 },
  { "GetGame", (PyCFunction) efoutcome_getgame, 1 }, 
  { "GetLabel", (PyCFunction) efoutcome_getlabel, 1 },
  { "GetPayoff", (PyCFunction) efoutcome_getpayoff, 1 },
  { "SetLabel", (PyCFunction) efoutcome_setlabel, 1 },
  { "SetPayoff", (PyCFunction) efoutcome_setpayoff, 1 },
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

efoutcomeobject *
newefoutcomeobject(void)
{
  efoutcomeobject *self;
  self = PyObject_NEW(efoutcomeobject, &Efoutcometype);
  if (self == NULL) {
    return NULL;
  }
  self->m_efoutcome = new gbtGameOutcome();
  return self;
}

static void                   
efoutcome_dealloc(efoutcomeobject *self) 
{                            
  delete self->m_efoutcome;
  PyMem_DEL(self);           
}

static PyObject *
efoutcome_getattr(efoutcomeobject *self, char *name)
{
  return Py_FindMethod(efoutcome_methods, (PyObject *) self, name);
}

static int
efoutcome_compare(efoutcomeobject *obj1, efoutcomeobject *obj2)
{
  // Implementation: If outcomes are the game underlying object, return
  // equal; otherwise, order by their Python pointer addresses.
  if (*obj1->m_efoutcome == *obj2->m_efoutcome) {
    return 0;
  }
  else if (obj1->m_efoutcome < obj2->m_efoutcome) {
    return -1;
  }
  else {
    return 1;
  }
}

static PyObject *
efoutcome_str(efoutcomeobject *self)
{
  return PyString_FromFormat("<{efoutcome} \"%s\">",
			     (char *) self->m_efoutcome->GetLabel());
}

/*************************************************************************
 * NFOUTCOME: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void nfoutcome_dealloc(nfoutcomeobject *);
staticforward PyObject *nfoutcome_getattr(nfoutcomeobject *, char *);
staticforward int nfoutcome_compare(nfoutcomeobject *, nfoutcomeobject *); 
staticforward PyObject *nfoutcome_str(nfoutcomeobject *);

PyTypeObject Nfoutcometype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "nfoutcome",                           /* tp_name */
  sizeof(nfoutcomeobject),               /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  nfoutcome_dealloc,       /* tp_dealloc  ref-count==0  */
  (printfunc)   0,         /* tp_print    "print x"     */
  (getattrfunc) nfoutcome_getattr,       /* tp_getattr  "x.attr"      */
  (setattrfunc) 0,                 /* tp_setattr  "x.attr=v"    */
  (cmpfunc)     nfoutcome_compare,      /* tp_compare  "x > y"       */
  (reprfunc)    0,                 /* tp_repr     `x`, print x  */

  /* type categories */
  0,                               /* tp_as_number   +,-,*,/,%,&,>>,pow...*/
  0,                               /* tp_as_sequence +,[i],[i:j],len, ...*/
  0,                               /* tp_as_mapping  [key], len, ...*/

  /* more methods */
  (hashfunc)     0,                /* tp_hash    "dict[x]" */
  (ternaryfunc)  0,                /* tp_call    "x()"     */
  (reprfunc)     nfoutcome_str,                /* tp_str     "str(x)"  */
};  /* plus others: see Include/object.h */


/*****************************************************************************
 * INSTANCE METHODS
 *****************************************************************************/

static PyObject *
nfoutcome_deleteoutcome(nfoutcomeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  self->m_nfoutcome->DeleteOutcome();
  Py_INCREF(Py_None);
  return Py_None;
}  

static PyObject *
nfoutcome_getgame(nfoutcomeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  nfgobject *game = newnfgobject();
  game->m_nfg = new gbtGame(self->m_nfoutcome->GetGame());
  return (PyObject *) game;
}

static PyObject *
nfoutcome_getlabel(nfoutcomeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_nfoutcome->GetLabel());
}

static PyObject *
nfoutcome_getpayoff(nfoutcomeobject *self, PyObject *args)
{
  PyObject *player;

  if (!PyArg_ParseTuple(args, "O", &player)) {
    return NULL;
  }

  if (!is_nfplayerobject(player)) {
    return NULL;
  }

  return Py_BuildValue("d",
		       (double) self->m_nfoutcome->GetPayoff(*((nfplayerobject *) player)->m_nfplayer));
}

static PyObject *
nfoutcome_setlabel(nfoutcomeobject *self, PyObject *args)
{
  char *label;

  if (!PyArg_ParseTuple(args, "s", &label)) {
    return NULL;
  }

  self->m_nfoutcome->SetLabel(label);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
nfoutcome_setpayoff(nfoutcomeobject *self, PyObject *args)
{
  PyObject *player;
  double payoff;

  if (!PyArg_ParseTuple(args, "Od", &player, &payoff)) {
    return NULL;
  }

  if (!is_nfplayerobject(player)) {
    return NULL;
  }

  self->m_nfoutcome->SetPayoff(*((nfplayerobject *) player)->m_nfplayer,
			       payoff);
  Py_INCREF(Py_None);
  return Py_None;
}

static struct PyMethodDef nfoutcome_methods[] = {
  { "DeleteOutcome", (PyCFunction) nfoutcome_deleteoutcome, 1 },
  { "GetGame", (PyCFunction) nfoutcome_getgame, 1 }, 
  { "GetLabel", (PyCFunction) nfoutcome_getlabel, 1 },
  { "GetPayoff", (PyCFunction) nfoutcome_getpayoff, 1 },
  { "SetLabel", (PyCFunction) nfoutcome_setlabel, 1 },
  { "SetPayoff", (PyCFunction) nfoutcome_setpayoff, 1 },
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

nfoutcomeobject *
newnfoutcomeobject(void)
{
  nfoutcomeobject *self;
  self = PyObject_NEW(nfoutcomeobject, &Nfoutcometype);
  if (self == NULL) {
    return NULL;
  }
  self->m_nfoutcome = new gbtGameOutcome();
  return self;
}

static void                   
nfoutcome_dealloc(nfoutcomeobject *self) 
{                            
  delete self->m_nfoutcome;
  PyMem_DEL(self);           
}

static PyObject *
nfoutcome_getattr(nfoutcomeobject *self, char *name)
{
  return Py_FindMethod(nfoutcome_methods, (PyObject *) self, name);
}

static int
nfoutcome_compare(nfoutcomeobject *obj1, nfoutcomeobject *obj2)
{
  // Implementation: If outcomes are the game underlying object, return
  // equal; otherwise, order by their Python pointer addresses.
  if (*obj1->m_nfoutcome == *obj2->m_nfoutcome) {
    return 0;
  }
  else if (obj1->m_nfoutcome < obj2->m_nfoutcome) {
    return -1;
  }
  else {
    return 1;
  }
}

static PyObject *
nfoutcome_str(nfoutcomeobject *self)
{
  return PyString_FromFormat("<{nfoutcome} \"%s\">",
			     (char *) self->m_nfoutcome->GetLabel());
}

/************************************************************************
 * MODULE METHODS
 ************************************************************************/

void
initoutcome(void)
{
  Efoutcometype.ob_type = &PyType_Type;
  Nfoutcometype.ob_type = &PyType_Type;
}
