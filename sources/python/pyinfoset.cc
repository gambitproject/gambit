//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Python extension type for infoset objects
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
#include "game/efg.h"

// This file provides implementations for both action and information set,
// since they're closely related

/*************************************************************************
 * ACTION: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void action_dealloc(actionobject *);
staticforward PyObject *action_getattr(actionobject *, char *);
staticforward int action_compare(actionobject *, actionobject *); 
staticforward int action_print(actionobject *, FILE *, int);

PyTypeObject Actiontype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "action",                           /* tp_name */
  sizeof(actionobject),               /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  action_dealloc,       /* tp_dealloc  ref-count==0  */
  (printfunc)   action_print,         /* tp_print    "print x"     */
  (getattrfunc) action_getattr,       /* tp_getattr  "x.attr"      */
  (setattrfunc) 0,                 /* tp_setattr  "x.attr=v"    */
  (cmpfunc)     action_compare,      /* tp_compare  "x > y"       */
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
action_getinfoset(actionobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  infosetobject *infoset = newinfosetobject();
  infoset->m_infoset = new gbtEfgInfoset(self->m_action->GetInfoset());
  return (PyObject *) infoset;
}

static PyObject *
action_getlabel(actionobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_action->GetLabel());
}

static PyObject *
action_setlabel(actionobject *self, PyObject *args)
{
  char *label;

  if (!PyArg_ParseTuple(args, "s", &label)) {
    return NULL;
  }

  self->m_action->SetLabel(label);
  Py_INCREF(Py_None);
  return Py_None;
}

static struct PyMethodDef action_methods[] = {
  { "GetInfoset", (PyCFunction) action_getinfoset, 1 },
  { "GetLabel", (PyCFunction) action_getlabel, 1 },
  { "SetLabel", (PyCFunction) action_setlabel, 1 },
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

actionobject *
newactionobject(void)
{
  actionobject *self;
  self = PyObject_NEW(actionobject, &Actiontype);
  if (self == NULL) {
    return NULL;
  }
  self->m_action = new gbtEfgAction();
  return self;
}

static void                   
action_dealloc(actionobject *self) 
{                            
  PyMem_DEL(self);           
}

static PyObject *
action_getattr(actionobject *self, char *name)
{
  return Py_FindMethod(action_methods, (PyObject *) self, name);
}

static int
action_compare(actionobject *obj1, actionobject *obj2)
{
  // Implementation: If outcomes are the game underlying object, return
  // equal; otherwise, order by their Python pointer addresses.
  if (*obj1->m_action == *obj2->m_action) {
    return 0;
  }
  else if (obj1->m_action < obj2->m_action) {
    return -1;
  }
  else {
    return 1;
  }
}

static int
action_print(actionobject *self, FILE *fp, int flags)
{
  fprintf(fp, "<{action} \"%s\">", (char *) self->m_action->GetLabel());
  return 0;
}

/*************************************************************************
 * INFOSET: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void infoset_dealloc(infosetobject *);
staticforward PyObject *infoset_getattr(infosetobject *, char *);
staticforward int infoset_compare(infosetobject *, infosetobject *); 
staticforward int infoset_print(infosetobject *, FILE *, int);

PyTypeObject Infosettype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "infoset",                           /* tp_name */
  sizeof(infosetobject),               /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  infoset_dealloc,       /* tp_dealloc  ref-count==0  */
  (printfunc)   infoset_print,         /* tp_print    "print x"     */
  (getattrfunc) infoset_getattr,       /* tp_getattr  "x.attr"      */
  (setattrfunc) 0,                 /* tp_setattr  "x.attr=v"    */
  (cmpfunc)     infoset_compare,      /* tp_compare  "x > y"       */
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
infoset_getlabel(infosetobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_infoset->GetLabel());
}

static PyObject *
infoset_getplayer(infosetobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  efplayerobject *player = newefplayerobject();
  player->m_efplayer = new gbtEfgPlayer(self->m_infoset->GetPlayer());
  return (PyObject *) player;
}

static PyObject *
infoset_setlabel(infosetobject *self, PyObject *args)
{
  char *label;

  if (!PyArg_ParseTuple(args, "s", &label)) {
    return NULL;
  }

  self->m_infoset->SetLabel(label);
  Py_INCREF(Py_None);
  return Py_None;
}

static struct PyMethodDef infoset_methods[] = {
  { "GetLabel", (PyCFunction) infoset_getlabel, 1 },
  { "GetPlayer", (PyCFunction) infoset_getplayer, 1 }, 
  { "SetLabel", (PyCFunction) infoset_setlabel, 1 },
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

infosetobject *
newinfosetobject(void)
{
  infosetobject *self;
  self = PyObject_NEW(infosetobject, &Infosettype);
  if (self == NULL) {
    return NULL;
  }
  self->m_infoset = new gbtEfgInfoset();
  return self;
}

static void                   
infoset_dealloc(infosetobject *self) 
{                            
  PyMem_DEL(self);           
}

static PyObject *
infoset_getattr(infosetobject *self, char *name)
{
  return Py_FindMethod(infoset_methods, (PyObject *) self, name);
}

static int
infoset_compare(infosetobject *obj1, infosetobject *obj2)
{
  // Implementation: If outcomes are the game underlying object, return
  // equal; otherwise, order by their Python pointer addresses.
  if (*obj1->m_infoset == *obj2->m_infoset) {
    return 0;
  }
  else if (obj1->m_infoset < obj2->m_infoset) {
    return -1;
  }
  else {
    return 1;
  }
}

static int
infoset_print(infosetobject *self, FILE *fp, int flags)
{
  fprintf(fp, "<{infoset} \"%s\">", (char *) self->m_infoset->GetLabel());
  return 0;
}

/************************************************************************
 * MODULE METHODS
 ************************************************************************/

void
initinfoset(void)
{
  Actiontype.ob_type = &PyType_Type;
  Infosettype.ob_type = &PyType_Type;
}
