//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Python extension type for player objects
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
#include "game/player.h"

// As with the underlying library, both extensive and normal form players
// appear in the same file, in the hopes of an eventual unification.

/*************************************************************************
 * EFPLAYER: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void efplayer_dealloc(efplayerobject *);
staticforward PyObject *efplayer_getattr(efplayerobject *, char *);
staticforward int efplayer_compare(efplayerobject *, efplayerobject *); 
staticforward int efplayer_print(efplayerobject *, FILE *, int);

PyTypeObject Efplayertype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "efplayer",                           /* tp_name */
  sizeof(efplayerobject),               /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  efplayer_dealloc,       /* tp_dealloc  ref-count==0  */
  (printfunc)   efplayer_print,         /* tp_print    "print x"     */
  (getattrfunc) efplayer_getattr,       /* tp_getattr  "x.attr"      */
  (setattrfunc) 0,                 /* tp_setattr  "x.attr=v"    */
  (cmpfunc)     efplayer_compare,   /* tp_compare  "x > y"       */
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
efplayer_getgame(efplayerobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  efgobject *game = newefgobject();
  game->m_efg = new gbtEfgGame(self->m_efplayer->GetGame());
  return (PyObject *) game;
}

static PyObject *
efplayer_getlabel(efplayerobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_efplayer->GetLabel());
}

static PyObject *
efplayer_ischance(efplayerobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("b", self->m_efplayer->IsChance());
}

static PyObject *
efplayer_numinfosets(efplayerobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("i", self->m_efplayer->NumInfosets());
}

static PyObject *
efplayer_setlabel(efplayerobject *self, PyObject *args)
{
  char *label;

  if (!PyArg_ParseTuple(args, "s", &label)) {
    return NULL;
  }

  self->m_efplayer->SetLabel(label);
  Py_INCREF(Py_None);
  return Py_None;
}

static struct PyMethodDef efplayer_methods[] = {
  { "GetGame", (PyCFunction) efplayer_getgame, 1 }, 
  { "GetLabel", (PyCFunction) efplayer_getlabel, 1 },
  { "IsChance", (PyCFunction) efplayer_ischance, 1 },
  { "NumInfosets", (PyCFunction) efplayer_numinfosets, 1 },
  { "SetLabel", (PyCFunction) efplayer_setlabel, 1 },
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

efplayerobject *
newefplayerobject(void)
{
  efplayerobject *self;
  self = PyObject_NEW(efplayerobject, &Efplayertype);
  if (self == NULL) {
    return NULL;
  }
  self->m_efplayer = new gbtEfgPlayer();
  return self;
}

static void                   
efplayer_dealloc(efplayerobject *self) 
{                            
  PyMem_DEL(self);           
}

static PyObject *
efplayer_getattr(efplayerobject *self, char *name)
{
  return Py_FindMethod(efplayer_methods, (PyObject *) self, name);
}

static int
efplayer_compare(efplayerobject *obj1, efplayerobject *obj2)
{
  // Implementation: If players are the game underlying object, return
  // equal; otherwise, order by their Python pointer addresses.
  if (*obj1->m_efplayer == *obj2->m_efplayer) {
    return 0;
  }
  else if (obj1->m_efplayer < obj2->m_efplayer) {
    return -1;
  }
  else {
    return 1;
  }
}

static int
efplayer_print(efplayerobject *self, FILE *fp, int flags)
{
  fprintf(fp, "<{efplayer} \"%s\">", (char *) self->m_efplayer->GetLabel());
  return 0;
}

/*************************************************************************
 * NFPLAYER: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void nfplayer_dealloc(nfplayerobject *);
staticforward PyObject *nfplayer_getattr(nfplayerobject *, char *);
staticforward int nfplayer_compare(nfplayerobject *, nfplayerobject *); 
staticforward int nfplayer_print(nfplayerobject *, FILE *, int);

PyTypeObject Nfplayertype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "nfplayer",                           /* tp_name */
  sizeof(nfplayerobject),               /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  nfplayer_dealloc,       /* tp_dealloc  ref-count==0  */
  (printfunc)   nfplayer_print,         /* tp_print    "print x"     */
  (getattrfunc) nfplayer_getattr,       /* tp_getattr  "x.attr"      */
  (setattrfunc) 0,                 /* tp_setattr  "x.attr=v"    */
  (cmpfunc)     nfplayer_compare,      /* tp_compare  "x > y"       */
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
nfplayer_getgame(nfplayerobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  nfgobject *game = newnfgobject();
  game->m_nfg = new gbtNfgGame(self->m_nfplayer->GetGame());
  return (PyObject *) game;
}

static PyObject *
nfplayer_getlabel(nfplayerobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_nfplayer->GetLabel());
}

static PyObject *
nfplayer_numstrategies(nfplayerobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("i", self->m_nfplayer->NumStrategies());
}

static PyObject *
nfplayer_setlabel(nfplayerobject *self, PyObject *args)
{
  char *label;

  if (!PyArg_ParseTuple(args, "s", &label)) {
    return NULL;
  }

  self->m_nfplayer->SetLabel(label);
  Py_INCREF(Py_None);
  return Py_None;
}

static struct PyMethodDef nfplayer_methods[] = {
  { "GetGame", (PyCFunction) nfplayer_getgame, 1 }, 
  { "GetLabel", (PyCFunction) nfplayer_getlabel, 1 },
  { "NumStrategies", (PyCFunction) nfplayer_numstrategies, 1 },
  { "SetLabel", (PyCFunction) nfplayer_setlabel, 1 },
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

nfplayerobject *
newnfplayerobject(void)
{
  nfplayerobject *self;
  self = PyObject_NEW(nfplayerobject, &Nfplayertype);
  if (self == NULL) {
    return NULL;
  }
  self->m_nfplayer = new gbtNfgPlayer();
  return self;
}

static void                   
nfplayer_dealloc(nfplayerobject *self) 
{                            
  PyMem_DEL(self);           
}

static PyObject *
nfplayer_getattr(nfplayerobject *self, char *name)
{
  return Py_FindMethod(nfplayer_methods, (PyObject *) self, name);
}

static int
nfplayer_compare(nfplayerobject *obj1, nfplayerobject *obj2)
{
  // Implementation: If players are the game underlying object, return
  // equal; otherwise, order by their Python pointer addresses.
  if (*obj1->m_nfplayer == *obj2->m_nfplayer) {
    return 0;
  }
  else if (obj1->m_nfplayer < obj2->m_nfplayer) {
    return -1;
  }
  else {
    return 1;
  }
}

static int
nfplayer_print(nfplayerobject *self, FILE *fp, int flags)
{
  fprintf(fp, "<{nfplayer} \"%s\">", (char *) self->m_nfplayer->GetLabel());
  return 0;
}

/************************************************************************
 * MODULE METHODS
 ************************************************************************/

void
initplayer(void)
{
  Efplayertype.ob_type = &PyType_Type;
  Nfplayertype.ob_type = &PyType_Type;
}
