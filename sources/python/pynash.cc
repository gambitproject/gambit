//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Python extension module for Nash equilibrium algorithms
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
#include "base/gnullstatus.h"
#include "nash/nfgmixed.h"
#include "nash/efgpoly.h"
#include "nash/nfgpoly.h"
#include "nash/efgpure.h"
#include "nash/nfgpure.h"
#include "nash/efglcp.h"
#include "nash/nfglcp.h"
#include "nash/efgliap.h"
#include "nash/nfgliap.h"
#include "nash/efglogit.h"
#include "nash/nfglogit.h"
#include "nash/efglp.h"
#include "nash/nfglp.h"
#include "nash/nfgsimpdiv.h"
#include "nash/nfgyamamoto.h"
#include "pygambit.h"

/************************************************************************
 * MODULE METHODS
 ************************************************************************/

PyObject *
gbt_nash_enummixed(PyObject */*self*/, PyObject *args)
{
  PyObject *support;

  if (!PyArg_ParseTuple(args, "O", &support)) {
    return NULL;
  }

  if (is_nfsupportobject(support)) {
    gbtNfgNashEnumMixed<double> algorithm;
    algorithm.SetStopAfter(0);
    gbtNullStatus status;
    gbtList<MixedSolution> solutions = algorithm.Solve(*((nfsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      mixedobject *p = newmixedobject();
      p->m_profile = new MixedSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else if (is_efsupportobject(support)) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  else {
    return NULL;
  }
}

PyObject *
gbt_nash_enumpoly(PyObject */*self*/, PyObject *args)
{
  PyObject *support;

  if (!PyArg_ParseTuple(args, "O", &support)) {
    return NULL;
  }

  if (is_nfsupportobject(support)) {
    gbtNfgNashEnumPoly algorithm;
    gbtNullStatus status;
    gbtList<MixedSolution> solutions = algorithm.Solve(*((nfsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      mixedobject *p = newmixedobject();
      p->m_profile = new MixedSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else if (is_efsupportobject(support)) {
    gbtEfgNashEnumPoly algorithm;
    gbtNullStatus status;
    gbtList<BehavSolution> solutions = algorithm.Solve(*((efsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      behavobject *p = newbehavobject();
      p->m_profile = new BehavSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else {
    return NULL;
  }
}

PyObject *
gbt_nash_enumpure(PyObject */*self*/, PyObject *args)
{
  PyObject *support;

  if (!PyArg_ParseTuple(args, "O", &support)) {
    return NULL;
  }

  if (is_nfsupportobject(support)) {
    gbtNfgNashEnumPure algorithm;
    algorithm.SetStopAfter(0);
    gbtNullStatus status;
    gbtList<MixedSolution> solutions = algorithm.Solve(*((nfsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      mixedobject *p = newmixedobject();
      p->m_profile = new MixedSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else if (is_efsupportobject(support)) {
    gbtEfgNashEnumPure algorithm;
    algorithm.SetStopAfter(0);
    gbtNullStatus status;
    gbtList<BehavSolution> solutions = algorithm.Solve(*((efsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      behavobject *p = newbehavobject();
      p->m_profile = new BehavSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else {
    return NULL;
  }
}

PyObject *
gbt_nash_lcp(PyObject */*self*/, PyObject *args)
{
  PyObject *support;

  if (!PyArg_ParseTuple(args, "O", &support)) {
    return NULL;
  }

  if (is_nfsupportobject(support)) {
    gbtNfgNashLcp<double> algorithm;
    algorithm.SetStopAfter(0);
    gbtNullStatus status;
    gbtList<MixedSolution> solutions = algorithm.Solve(*((nfsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      mixedobject *p = newmixedobject();
      p->m_profile = new MixedSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else if (is_efsupportobject(support)) {
    gbtEfgNashLcp<double> algorithm;
    algorithm.SetStopAfter(0);
    gbtNullStatus status;
    gbtList<BehavSolution> solutions = algorithm.Solve(*((efsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      behavobject *p = newbehavobject();
      p->m_profile = new BehavSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else {
    return NULL;
  }
}

PyObject *
gbt_nash_liap(PyObject */*self*/, PyObject *args)
{
  PyObject *support;
  int stopAfter, nTries, maxIters;

  if (!PyArg_ParseTuple(args, "Oiii",
			&support, &stopAfter, &nTries, &maxIters)) {
    return NULL;
  }

  if (is_nfsupportobject(support)) {
    gbtNfgNashLiap algorithm;
    algorithm.SetStopAfter(stopAfter);
    algorithm.SetNumTries(nTries);
    algorithm.SetMaxitsN(maxIters);
    gbtNullStatus status;
    gbtList<MixedSolution> solutions = algorithm.Solve(*((nfsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      mixedobject *p = newmixedobject();
      p->m_profile = new MixedSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else if (is_efsupportobject(support)) {
    gbtEfgNashLiap algorithm;
    algorithm.SetStopAfter(stopAfter);
    algorithm.SetNumTries(nTries);
    algorithm.SetMaxitsN(maxIters);
    gbtNullStatus status;
    gbtList<BehavSolution> solutions = algorithm.Solve(*((efsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      behavobject *p = newbehavobject();
      p->m_profile = new BehavSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else {
    return NULL;
  }
}

PyObject *
gbt_nash_logit(PyObject */*self*/, PyObject *args)
{
  PyObject *support;
  double maxLam, stepSize;
  int fullGraph;

  if (!PyArg_ParseTuple(args, "Oddi",
			&support, &maxLam, &stepSize, &fullGraph)) {
    return NULL;
  }

  if (is_nfsupportobject(support)) {
    gbtNfgNashLogit algorithm;
    algorithm.SetMaxLambda(maxLam);
    algorithm.SetStepSize(stepSize);
    algorithm.SetFullGraph(fullGraph);
    gbtNullStatus status;
    gbtList<MixedSolution> solutions = algorithm.Solve(*((nfsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      mixedobject *p = newmixedobject();
      p->m_profile = new MixedSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else if (is_efsupportobject(support)) {
    gbtEfgNashLogit algorithm;
    algorithm.SetMaxLambda(maxLam);
    algorithm.SetStepSize(stepSize);
    algorithm.SetFullGraph(fullGraph);
    gbtNullStatus status;
    gbtList<BehavSolution> solutions = algorithm.Solve(*((efsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      behavobject *p = newbehavobject();
      p->m_profile = new BehavSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else {
    return NULL;
  }
}

PyObject *
gbt_nash_lp(PyObject */*self*/, PyObject *args)
{
  PyObject *support;

  if (!PyArg_ParseTuple(args, "O", &support)) {
    return NULL;
  }

  if (is_nfsupportobject(support)) {
    gbtNfgNashLp<double> algorithm;
    gbtNullStatus status;
    gbtList<MixedSolution> solutions = algorithm.Solve(*((nfsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      mixedobject *p = newmixedobject();
      p->m_profile = new MixedSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else if (is_efsupportobject(support)) {
    gbtEfgNashLp<double> algorithm;
    gbtNullStatus status;
    gbtList<BehavSolution> solutions = algorithm.Solve(*((efsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      behavobject *p = newbehavobject();
      p->m_profile = new BehavSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else {
    return NULL;
  }
}

PyObject *
gbt_nash_simpdiv(PyObject */*self*/, PyObject *args)
{
  PyObject *support;

  if (!PyArg_ParseTuple(args, "O", &support)) {
    return NULL;
  }

  if (is_nfsupportobject(support)) {
    gbtNfgNashSimpdiv<double> algorithm;
    gbtNullStatus status;
    gbtList<MixedSolution> solutions = algorithm.Solve(*((nfsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      mixedobject *p = newmixedobject();
      p->m_profile = new MixedSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else if (is_efsupportobject(support)) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  else {
    return NULL;
  }
}

PyObject *
gbt_nash_yamamoto(PyObject */*self*/, PyObject *args)
{
  PyObject *support;

  if (!PyArg_ParseTuple(args, "O", &support)) {
    return NULL;
  }

  if (is_nfsupportobject(support)) {
    gbtNfgNashYamamoto algorithm;
    gbtNullStatus status;
    gbtList<MixedSolution> solutions = algorithm.Solve(*((nfsupportobject *) support)->m_support, status);

    PyObject *list = PyList_New(0);

    for (int i = 1; i <= solutions.Length(); i++) {
      mixedobject *p = newmixedobject();
      p->m_profile = new MixedSolution(solutions[i]);
      PyList_Append(list, (PyObject *) p);
      Py_DECREF(p);
    }  

    return list;
  }
  else if (is_efsupportobject(support)) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  else {
    return NULL;
  }
}

void
initnash(void)
{
}
