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
staticforward PyObject *mixed_str(mixedobject *);

PyTypeObject Mixedtype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "mixed",                         /* tp_name */
  sizeof(mixedobject),             /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  mixed_dealloc,     /* tp_dealloc  ref-count==0  */
  (printfunc)   0,       /* tp_print    "print x"     */
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
  (reprfunc)     mixed_str,         /* tp_str     "str(x)"  */
};  /* plus others: see Include/object.h */


/*****************************************************************************
 * INSTANCE METHODS
 *****************************************************************************/

static PyObject *
mixed_getcreator(mixedobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_profile->GetCreator());
}

static PyObject *
mixed_getgame(mixedobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  nfgobject *game = newnfgobject();
  game->m_nfg = new gbtNfgGame(self->m_profile->GetGame());
  return (PyObject *) game;
}

static PyObject *
mixed_getlabel(mixedobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_profile->GetLabel());
}

static PyObject *
mixed_getliapvalue(mixedobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("d", (double) self->m_profile->GetLiapValue());
}

static PyObject *
mixed_getpayoff(mixedobject *self, PyObject *args)
{
  PyObject *player;

  if (!PyArg_ParseTuple(args, "O", &player)) {
    return NULL;
  }

  if (!is_nfplayerobject(player)) {
    return NULL;
  }

  return Py_BuildValue("d",
		       (double) self->m_profile->GetPayoff(*((nfplayerobject *)player)->m_nfplayer));
}

static PyObject *
mixed_getqrelambda(mixedobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("d", (double) self->m_profile->QreLambda());
}

static PyObject *
mixed_getstrategyprob(mixedobject *self, PyObject *args)
{
  PyObject *strategy;
  
  if (!PyArg_ParseTuple(args, "O", &strategy)) {
    return NULL;
  }

  if (!is_strategyobject(strategy)) {
    return NULL;
  }

  return Py_BuildValue("d",
		       (double) self->m_profile->GetStrategyProb(*((strategyobject *) strategy)->m_strategy)); 
}

static PyObject *
mixed_getstrategyvalue(mixedobject *self, PyObject *args)
{
  PyObject *strategy;
  
  if (!PyArg_ParseTuple(args, "O", &strategy)) {
    return NULL;
  }

  if (!is_strategyobject(strategy)) {
    return NULL;
  }

  return Py_BuildValue("d",
		       (double) self->m_profile->GetStrategyValue(*((strategyobject *) strategy)->m_strategy)); 
}

static PyObject *
mixed_isnash(mixedobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  if (self->m_profile->IsNash() == GBT_TRISTATE_TRUE) {
    return Py_BuildValue("i", 1);
  }
  else if (self->m_profile->IsNash() == GBT_TRISTATE_FALSE) {
    return Py_BuildValue("i", 0);
  }
  else {
    return Py_BuildValue("i", -1);
  }
}

static PyObject *
mixed_isperfect(mixedobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  if (self->m_profile->IsPerfect() == GBT_TRISTATE_TRUE) {
    return Py_BuildValue("i", 1);
  }
  else if (self->m_profile->IsPerfect() == GBT_TRISTATE_FALSE) {
    return Py_BuildValue("i", 0);
  }
  else {
    return Py_BuildValue("i", -1);
  }
}

static PyObject *
mixed_isproper(mixedobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  if (self->m_profile->IsProper() == GBT_TRISTATE_TRUE) {
    return Py_BuildValue("i", 1);
  }
  else if (self->m_profile->IsProper() == GBT_TRISTATE_FALSE) {
    return Py_BuildValue("i", 0);
  }
  else {
    return Py_BuildValue("i", -1);
  }
}

static PyObject *
mixed_setlabel(mixedobject *self, PyObject *args)
{
  char *label;

  if (!PyArg_ParseTuple(args, "s", &label)) {
    return NULL;
  }

  self->m_profile->SetLabel(label);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
mixed_setstrategyprob(mixedobject *self, PyObject *args)
{
  PyObject *strategy;
  double prob;
  
  if (!PyArg_ParseTuple(args, "Od", &strategy, &prob)) {
    return NULL;
  }

  if (!is_strategyobject(strategy)) {
    return NULL;
  }

  self->m_profile->SetStrategyProb(*((strategyobject *) strategy)->m_strategy, 
				   prob);
  Py_INCREF(Py_None);
  return Py_None;
}

static struct PyMethodDef mixed_methods[] = {
  { "GetCreator", (PyCFunction) mixed_getcreator, 1 },
  { "GetGame", (PyCFunction) mixed_getgame, 1 },
  { "GetLabel", (PyCFunction) mixed_getlabel, 1 },
  { "GetLiapValue", (PyCFunction) mixed_getliapvalue, 1 },
  { "GetPayoff", (PyCFunction) mixed_getpayoff, 1 },
  { "GetQreLambda", (PyCFunction) mixed_getqrelambda, 1 },
  { "GetStrategyProb", (PyCFunction) mixed_getstrategyprob, 1 },
  { "GetStrategyValue", (PyCFunction) mixed_getstrategyvalue, 1 },
  { "IsNash", (PyCFunction) mixed_isnash, 1 },
  { "IsPerfect", (PyCFunction) mixed_isperfect, 1 },
  { "IsProper", (PyCFunction) mixed_isproper, 1 },
  { "SetLabel", (PyCFunction) mixed_setlabel, 1 },
  { "SetStrategyProb", (PyCFunction) mixed_setstrategyprob, 1 },
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

static PyObject *
mixed_str(mixedobject *self)
{
  return PyString_FromFormat("<{mixed} \"%s\">",
			     (char *) self->m_profile->GetLabel());
}

void
initmixed(void)
{
  Mixedtype.ob_type = &PyType_Type;
}
