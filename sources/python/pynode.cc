//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Python extension type for node objects
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
#include "game/nfg.h"

/*************************************************************************
 * NODE: TYPE DESCRIPTOR
 *************************************************************************/

staticforward void node_dealloc(nodeobject *);
staticforward PyObject *node_getattr(nodeobject *, char *);
staticforward int node_compare(nodeobject *, nodeobject *); 
staticforward int node_print(nodeobject *, FILE *, int);

PyTypeObject Nodetype = {      /* main python type-descriptor */
  /* type header */                    /* shared by all instances */
  PyObject_HEAD_INIT(0)
  0,                               /* ob_size */
  "node",                           /* tp_name */
  sizeof(nodeobject),               /* tp_basicsize */
  0,                               /* tp_itemsize */

  /* standard methods */
  (destructor)  node_dealloc,       /* tp_dealloc  ref-count==0  */
  (printfunc)   node_print,         /* tp_print    "print x"     */
  (getattrfunc) node_getattr,       /* tp_getattr  "x.attr"      */
  (setattrfunc) 0,                 /* tp_setattr  "x.attr=v"    */
  (cmpfunc)     node_compare,      /* tp_compare  "x > y"       */
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
node_getinfoset(nodeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  infosetobject *infoset = newinfosetobject();
  infoset->m_infoset = new gbtEfgInfoset(self->m_node->GetInfoset());
  return (PyObject *) infoset;
}

static PyObject *
node_getlabel(nodeobject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "")) {
    return NULL;
  }

  return Py_BuildValue("s", (char *) self->m_node->GetLabel());
}

static PyObject *
node_setlabel(nodeobject *self, PyObject *args)
{
  char *label;

  if (!PyArg_ParseTuple(args, "s", &label)) {
    return NULL;
  }

  self->m_node->SetLabel(label);
  Py_INCREF(Py_None);
  return Py_None;
}

static struct PyMethodDef node_methods[] = {
  { "GetInfoset", (PyCFunction) node_getinfoset, 1 }, 
  { "GetLabel", (PyCFunction) node_getlabel, 1 },
  { "SetLabel", (PyCFunction) node_setlabel, 1 },
  { NULL, NULL }
};

/*****************************************************************************
 * BASIC TYPE-OPERATIONS
 *****************************************************************************/

nodeobject *
newnodeobject(void)
{
  nodeobject *self;
  self = PyObject_NEW(nodeobject, &Nodetype);
  if (self == NULL) {
    return NULL;
  }
  self->m_node = new gbtEfgNode();
  return self;
}

static void                   
node_dealloc(nodeobject *self) 
{                            
  PyMem_DEL(self);           
}

static PyObject *
node_getattr(nodeobject *self, char *name)
{
  return Py_FindMethod(node_methods, (PyObject *) self, name);
}

static int
node_compare(nodeobject *obj1, nodeobject *obj2)
{
  // Implementation: If outcomes are the game underlying object, return
  // equal; otherwise, order by their Python pointer addresses.
  if (*obj1->m_node == *obj2->m_node) {
    return 0;
  }
  else if (obj1->m_node < obj2->m_node) {
    return -1;
  }
  else {
    return 1;
  }
}

static int
node_print(nodeobject *self, FILE *fp, int flags)
{
  fprintf(fp, "<{node} \"%s\">", (char *) self->m_node->GetLabel());
  return 0;
}

/************************************************************************
 * MODULE METHODS
 ************************************************************************/

void
initnode(void)
{
  Nodetype.ob_type = &PyType_Type;
}
