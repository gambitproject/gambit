//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Python extension type for behavior strategy profiles
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
#include "nash/behavsol.h"
#include "pygambit.h"

/*************************************************************************
 * BEHAV PROFILE: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void behav_dealloc(behavobject *);
staticforward PyObject *behav_getattr(behavobject *, char *);
staticforward PyObject *behav_str(behavobject *);

PyTypeObject Behavtype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "behav",                         /* tp_name */
  sizeof(behavobject),             /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  behav_dealloc,     /* tp_dealloc  ref-count==0  */
  (printfunc)   0,       /* tp_print    "print x"     */
  (getattrfunc) behav_getattr,     /* tp_getattr  "x.attr"      */
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
  (reprfunc)     behav_str,                /* tp_str     "str(x)"  */
};  /* plus others: see Include/object.h */


/*****************************************************************************
 * INSTANCE METHODS
 *****************************************************************************/

static PyObject *
behav_getactionprob(behavobject *self, PyObject *args)
{
  PyObject *action;
  
  if (!PyArg_ParseTuple(args, "O", &action)) {
    return NULL;
  }

  if (!is_actionobject(action)) {
    return NULL;
  }

  return Py_BuildValue("d",
		       (double) self->m_profile->GetActionProb(*((actionobject *) action)->m_action)); 
}

static PyObject *
behav_getactionvalue(behavobject *self, PyObject *args)
{
  PyObject *action;
  
  if (!PyArg_ParseTuple(args, "O", &action)) {
    return NULL;
  }

  if (!is_actionobject(action)) {
    return NULL;
  }

  return Py_BuildValue("d",
		       (double) self->m_profile->GetActionValue(*((actionobject *) action)->m_action)); 
}

static PyObject *
behav_getbelief(behavobject *self, PyObject *args)
{
  PyObject *node;
  
  if (!PyArg_ParseTuple(args, "O", &node)) {
    return NULL;
  }

  if (!is_nodeobject(node)) {
    return NULL;
  }

  return Py_BuildValue("d",
		       (double) self->m_profile->GetBelief(*((nodeobject *) node)->m_node)); 
}

static PyObject *
behav_getcreator(behavobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_profile->GetCreator());
}

static PyObject *
behav_getgame(behavobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  efgobject *game = newefgobject();
  game->m_efg = new gbtGame(self->m_profile->GetGame());
  return (PyObject *) game;
}

static PyObject *
behav_getinfosetprob(behavobject *self, PyObject *args)
{
  PyObject *infoset;
  
  if (!PyArg_ParseTuple(args, "O", &infoset)) {
    return NULL;
  }

  if (!is_infosetobject(infoset)) {
    return NULL;
  }

  return Py_BuildValue("d",
		       (double) self->m_profile->GetInfosetProb(*((infosetobject *) infoset)->m_infoset)); 
}

static PyObject *
behav_getinfosetvalue(behavobject *self, PyObject *args)
{
  PyObject *infoset;
  
  if (!PyArg_ParseTuple(args, "O", &infoset)) {
    return NULL;
  }

  if (!is_infosetobject(infoset)) {
    return NULL;
  }

  return Py_BuildValue("d",
		       (double) self->m_profile->GetInfosetValue(*((infosetobject *) infoset)->m_infoset)); 
}

static PyObject *
behav_getlabel(behavobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_profile->GetLabel());
}

static PyObject *
behav_getliapvalue(behavobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("d", (double) self->m_profile->GetLiapValue());
}

static PyObject *
behav_getnodevalue(behavobject *self, PyObject *args)
{
  PyObject *node, *player;
  
  if (!PyArg_ParseTuple(args, "OO", &node, &player)) {
    return NULL;
  }

  if (!is_nodeobject(node) || !is_efplayerobject(player)) {
    return NULL;
  }

  return Py_BuildValue("d",
		       (double) self->m_profile->GetNodeValue(*((nodeobject *) node)->m_node,
							      *((efplayerobject *) player)->m_efplayer)); 
}

static PyObject *
behav_getpayoff(behavobject *self, PyObject *args)
{
  PyObject *player;

  if (!PyArg_ParseTuple(args, "O", &player)) {
    return NULL;
  }

  if (!is_efplayerobject(player)) {
    return NULL;
  }

  return Py_BuildValue("d",
		       (double) self->m_profile->GetPayoff(*((efplayerobject *)player)->m_efplayer));
}

static PyObject *
behav_getrealizprob(behavobject *self, PyObject *args)
{
  PyObject *node;
  
  if (!PyArg_ParseTuple(args, "O", &node)) {
    return NULL;
  }

  if (!is_nodeobject(node)) {
    return NULL;
  }

  return Py_BuildValue("d",
		       (double) self->m_profile->GetRealizProb(*((nodeobject *) node)->m_node)); 
}

static PyObject *
behav_getregret(behavobject *self, PyObject *args)
{
  PyObject *action;
  
  if (!PyArg_ParseTuple(args, "O", &action)) {
    return NULL;
  }

  if (!is_actionobject(action)) {
    return NULL;
  }

  return Py_BuildValue("d",
		       (double) self->m_profile->GetRegret(*((actionobject *) action)->m_action)); 
}

static PyObject *
behav_isnash(behavobject *self, PyObject *args)
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
behav_issequential(behavobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  if (self->m_profile->IsSequential() == GBT_TRISTATE_TRUE) {
    return Py_BuildValue("i", 1);
  }
  else if (self->m_profile->IsSequential() == GBT_TRISTATE_FALSE) {
    return Py_BuildValue("i", 0);
  }
  else {
    return Py_BuildValue("i", -1);
  }
}

static PyObject *
behav_issubgameperfect(behavobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  if (self->m_profile->IsSubgamePerfect() == GBT_TRISTATE_TRUE) {
    return Py_BuildValue("i", 1);
  }
  else if (self->m_profile->IsSubgamePerfect() == GBT_TRISTATE_FALSE) {
    return Py_BuildValue("i", 0);
  }
  else {
    return Py_BuildValue("i", -1);
  }
}

static PyObject *
behav_setactionprob(behavobject *self, PyObject *args)
{
  PyObject *action;
  double prob;
  
  if (!PyArg_ParseTuple(args, "Od", &action, &prob)) {
    return NULL;
  }

  if (!is_actionobject(action)) {
    return NULL;
  }

  self->m_profile->SetActionProb(*((actionobject *) action)->m_action, 
				 prob);
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *
behav_setlabel(behavobject *self, PyObject *args)
{
  char *label;

  if (!PyArg_ParseTuple(args, "s", &label)) {
    return NULL;
  }

  self->m_profile->SetLabel(label);
  Py_INCREF(Py_None);
  return Py_None;
}


static struct PyMethodDef behav_methods[] = {
  { "GetActionProb", (PyCFunction) behav_getactionprob, 1 },
  { "GetActionValue", (PyCFunction) behav_getactionvalue, 1 },
  { "GetBelief", (PyCFunction) behav_getbelief, 1 },
  { "GetCreator", (PyCFunction) behav_getcreator, 1 },
  { "GetGame", (PyCFunction) behav_getgame, 1 },
  { "GetInfosetProb", (PyCFunction) behav_getinfosetprob, 1 },
  { "GetInfosetValue", (PyCFunction) behav_getinfosetvalue, 1 },
  { "GetLabel", (PyCFunction) behav_getlabel, 1 },
  { "GetLiapValue", (PyCFunction) behav_getliapvalue, 1 },
  { "GetNodeValue", (PyCFunction) behav_getnodevalue, 1 },
  { "GetPayoff", (PyCFunction) behav_getpayoff, 1 },
  { "GetRealizProb", (PyCFunction) behav_getrealizprob, 1 },
  { "GetRegret", (PyCFunction) behav_getregret, 1 },
  { "IsNash", (PyCFunction) behav_isnash, 1 },
  { "IsSequential", (PyCFunction) behav_issequential, 1 },
  { "IsSubgamePerfect", (PyCFunction) behav_issubgameperfect, 1 },
  { "SetActionProb", (PyCFunction) behav_setactionprob, 1 },
  { "SetLabel", (PyCFunction) behav_setlabel, 1 },
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

behavobject *
newbehavobject(void)
{
  behavobject *self;
  self = PyObject_NEW(behavobject, &Behavtype);
  if (self == NULL) {
    return NULL;
  }
  self->m_profile = 0;
  return self;
}

static void                   
behav_dealloc(behavobject *self) 
{     
  delete self->m_profile;
  PyMem_DEL(self);           
}

static PyObject *
behav_getattr(behavobject *self, char *name)
{
  return Py_FindMethod(behav_methods, (PyObject *) self, name);
}

static PyObject *
behav_str(behavobject *self)
{
  return PyString_FromFormat("<{behav} \"%s\">",
			     (char *) self->m_profile->GetLabel());
}

void
initbehav(void)
{
  Behavtype.ob_type = &PyType_Type;
}
